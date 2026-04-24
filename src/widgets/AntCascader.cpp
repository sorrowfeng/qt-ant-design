#include "AntCascader.h"

#include <QApplication>
#include <QFocusEvent>
#include <QFrame>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QScreen>
#include <QScrollBar>

#include <algorithm>

#include "../styles/AntCascaderStyle.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"

namespace
{
constexpr int kColumnWidth = 180;
constexpr int kMaxVisibleItems = 8;
constexpr int kPopupHPadding = 8;
constexpr int kPopupVPadding = 4;
constexpr int kPopupShadowMargin = 8;
constexpr int kOptionHPadding = 12;
constexpr int kArrowSize = 16;
constexpr int kCheckSize = 14;

struct ColumnState
{
    const QVector<AntCascaderOption>* options = nullptr;
    int highlightedIndex = -1;
    int selectedIndex = -1;
};
} // namespace

class CascaderPopup : public QFrame
{
public:
    explicit CascaderPopup(AntCascader* owner)
        : QFrame(nullptr, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
          m_owner(owner)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setObjectName(QStringLiteral("CascaderPopup"));
        setMouseTracking(true);
    }

    void rebuildColumns()
    {
        m_columns.clear();
        m_hoveredColumn = -1;
        m_hoveredRow = -1;

        const QVector<AntCascaderOption>* current = &m_owner->m_options;
        const QStringList& valPath = m_owner->m_value;

        for (int depth = 0; depth <= valPath.size(); ++depth)
        {
            if (!current || current->isEmpty())
            {
                break;
            }

            ColumnState col;
            col.options = current;
            col.highlightedIndex = -1;
            col.selectedIndex = -1;

            if (depth < valPath.size())
            {
                for (int i = 0; i < current->size(); ++i)
                {
                    if (current->at(i).value.toString() == valPath.at(depth))
                    {
                        col.selectedIndex = i;
                        break;
                    }
                }
            }

            m_columns.push_back(col);

            if (depth < valPath.size() && col.selectedIndex >= 0)
            {
                current = &current->at(col.selectedIndex).children;
            }
            else
            {
                current = nullptr;
            }
        }
    }

    void updateSizeAndPosition()
    {
        int columnCount = std::max(1, static_cast<int>(m_columns.size()));
        int maxItemsInAnyCol = 0;
        for (const auto& col : m_columns)
        {
            if (col.options)
            {
                maxItemsInAnyCol = std::max(maxItemsInAnyCol, static_cast<int>(col.options->size()));
            }
        }
        maxItemsInAnyCol = std::min(std::max(1, maxItemsInAnyCol), kMaxVisibleItems);

        const auto& token = antTheme->tokens();
        int optionHeight = token.controlHeight;
        if (m_owner->cascaderSize() == Ant::SelectSize::Large)
        {
            optionHeight = token.controlHeightLG;
        }
        else if (m_owner->cascaderSize() == Ant::SelectSize::Small)
        {
            optionHeight = token.controlHeightSM;
        }

        int popupWidth = columnCount * kColumnWidth + kPopupHPadding * 2 + kPopupShadowMargin * 2;
        int popupHeight = maxItemsInAnyCol * optionHeight + kPopupVPadding * 2 + kPopupShadowMargin * 2;
        popupHeight = std::max(popupHeight, optionHeight + kPopupVPadding * 2 + kPopupShadowMargin * 2);

        QPoint globalPos = m_owner->mapToGlobal(QPoint(0, m_owner->height() + 4));

        QScreen* screen = QApplication::screenAt(globalPos);
        if (!screen)
        {
            screen = QApplication::primaryScreen();
        }
        QRect screenGeom = screen->availableGeometry();

        if (globalPos.x() + popupWidth > screenGeom.right())
        {
            globalPos.setX(screenGeom.right() - popupWidth - 4);
        }
        if (globalPos.x() < screenGeom.left())
        {
            globalPos.setX(screenGeom.left() + 4);
        }
        if (globalPos.y() + popupHeight > screenGeom.bottom())
        {
            globalPos.setY(m_owner->mapToGlobal(QPoint(0, -popupHeight - 4)).y());
        }
        if (globalPos.y() < screenGeom.top())
        {
            globalPos.setY(screenGeom.top() + 4);
        }

        setGeometry(globalPos.x(), globalPos.y(), popupWidth, popupHeight);
    }

    void setHoveredCell(int col, int row)
    {
        if (m_hoveredColumn == col && m_hoveredRow == row)
        {
            return;
        }

        m_hoveredColumn = col;
        m_hoveredRow = row;

        if (col >= 0 && col < m_columns.size() && row >= 0 && m_columns[col].options && row < m_columns[col].options->size())
        {
            m_columns[col].highlightedIndex = row;
        }

        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)

        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

        QRect panelRect = rect().adjusted(kPopupShadowMargin, kPopupShadowMargin, -kPopupShadowMargin, -kPopupShadowMargin);
        antTheme->drawEffectShadow(&painter, panelRect, 10, token.borderRadiusLG, 0.55);

        painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter.setBrush(token.colorBgElevated);
        painter.drawRoundedRect(panelRect, token.borderRadiusLG, token.borderRadiusLG);

        QRect contentRect = panelRect.adjusted(kPopupHPadding, kPopupVPadding, -kPopupHPadding, -kPopupVPadding);

        int optionHeight = token.controlHeight;
        int fontSize = token.fontSize;
        if (m_owner->cascaderSize() == Ant::SelectSize::Large)
        {
            optionHeight = token.controlHeightLG;
            fontSize = token.fontSizeLG;
        }
        else if (m_owner->cascaderSize() == Ant::SelectSize::Small)
        {
            optionHeight = token.controlHeightSM;
            fontSize = token.fontSizeSM;
        }

        int colX = contentRect.left();
        for (int colIdx = 0; colIdx < m_columns.size(); ++colIdx)
        {
            const ColumnState& col = m_columns.at(colIdx);
            if (!col.options || col.options->isEmpty())
            {
                break;
            }

            int colRight = colX + kColumnWidth;

            if (colIdx > 0)
            {
                painter.setPen(QPen(token.colorSplit, token.lineWidth));
                painter.drawLine(QPointF(colX - 0.5, contentRect.top()),
                                 QPointF(colX - 0.5, contentRect.bottom()));
            }

            for (int rowIdx = 0; rowIdx < col.options->size(); ++rowIdx)
            {
                const AntCascaderOption& opt = col.options->at(rowIdx);
                QRect itemRect(colX, contentRect.top() + rowIdx * optionHeight, kColumnWidth, optionHeight);

                bool isSelected = col.selectedIndex == rowIdx;
                bool isHighlighted = col.highlightedIndex == rowIdx;
                bool isDisabled = opt.disabled;

                QRectF bgRect = QRectF(itemRect).adjusted(4, 2, -4, -2);

                if (isSelected)
                {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(AntPalette::alpha(token.colorPrimary,
                                                       antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.24 : 0.12));
                    painter.drawRoundedRect(bgRect, token.borderRadiusSM, token.borderRadiusSM);
                }
                else if (isHighlighted && !isDisabled)
                {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(token.colorFillQuaternary);
                    painter.drawRoundedRect(bgRect, token.borderRadiusSM, token.borderRadiusSM);
                }

                QFont f = painter.font();
                f.setPixelSize(fontSize);
                f.setWeight(isSelected ? QFont::DemiBold : QFont::Normal);
                painter.setFont(f);

                QColor textColor = isDisabled ? token.colorTextDisabled : token.colorText;
                painter.setPen(textColor);

                bool hasChildren = !opt.isLeaf && !opt.children.isEmpty();
                int rightPad = kOptionHPadding;
                if (hasChildren)
                {
                    rightPad = kArrowSize + kOptionHPadding;
                }
                else if (isSelected)
                {
                    rightPad = kCheckSize + kOptionHPadding;
                }

                QRect textRect = itemRect.adjusted(kOptionHPadding, 0, -rightPad, 0);
                painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, opt.label);

                if (hasChildren)
                {
                    painter.setPen(QPen(isDisabled ? token.colorTextDisabled : token.colorTextTertiary,
                                        1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    qreal cx = itemRect.right() - kOptionHPadding - kArrowSize / 2.0;
                    qreal cy = itemRect.center().y();
                    QPainterPath arrow;
                    arrow.moveTo(cx - 3, cy - 4);
                    arrow.lineTo(cx + 3, cy);
                    arrow.lineTo(cx - 3, cy + 4);
                    painter.drawPath(arrow);
                }
                else if (isSelected)
                {
                    QPen checkPen(isDisabled ? token.colorTextDisabled : token.colorPrimary,
                                  2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                    painter.setPen(checkPen);
                    qreal cx = itemRect.right() - kOptionHPadding - kCheckSize / 2.0;
                    qreal cy = itemRect.center().y();
                    QPainterPath check;
                    check.moveTo(cx - 6, cy);
                    check.lineTo(cx - 2, cy + 4);
                    check.lineTo(cx + 7, cy - 6);
                    painter.drawPath(check);
                }
            }

            colX = colRight;
        }
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        const auto& token = antTheme->tokens();
        int optionHeight = token.controlHeight;
        if (m_owner->cascaderSize() == Ant::SelectSize::Large)
        {
            optionHeight = token.controlHeightLG;
        }
        else if (m_owner->cascaderSize() == Ant::SelectSize::Small)
        {
            optionHeight = token.controlHeightSM;
        }

        QRect panelRect = rect().adjusted(kPopupShadowMargin, kPopupShadowMargin, -kPopupShadowMargin, -kPopupShadowMargin);
        QRect contentRect = panelRect.adjusted(kPopupHPadding, kPopupVPadding, -kPopupHPadding, -kPopupVPadding);

        QPoint pos = event->pos();

        if (!contentRect.contains(pos))
        {
            setHoveredCell(-1, -1);
            return;
        }

        int relX = pos.x() - contentRect.left();
        int relY = pos.y() - contentRect.top();
        int colIdx = relX / kColumnWidth;
        int rowIdx = relY / optionHeight;

        if (colIdx < 0 || colIdx >= m_columns.size())
        {
            setHoveredCell(-1, -1);
            return;
        }

        const ColumnState& col = m_columns.at(colIdx);
        if (!col.options || rowIdx < 0 || rowIdx >= col.options->size())
        {
            setHoveredCell(-1, -1);
            return;
        }

        setHoveredCell(colIdx, rowIdx);

        if (m_owner->expandTrigger() == Ant::CascaderExpandTrigger::Hover)
        {
            handleColumnExpand(colIdx, rowIdx);
        }

        QFrame::mouseMoveEvent(event);
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() != Qt::LeftButton)
        {
            QFrame::mousePressEvent(event);
            return;
        }

        const auto& token = antTheme->tokens();
        int optionHeight = token.controlHeight;
        if (m_owner->cascaderSize() == Ant::SelectSize::Large)
        {
            optionHeight = token.controlHeightLG;
        }
        else if (m_owner->cascaderSize() == Ant::SelectSize::Small)
        {
            optionHeight = token.controlHeightSM;
        }

        QRect panelRect = rect().adjusted(kPopupShadowMargin, kPopupShadowMargin, -kPopupShadowMargin, -kPopupShadowMargin);
        QRect contentRect = panelRect.adjusted(kPopupHPadding, kPopupVPadding, -kPopupHPadding, -kPopupVPadding);

        QPoint pos = event->pos();
        if (!contentRect.contains(pos))
        {
            return;
        }

        int relX = pos.x() - contentRect.left();
        int relY = pos.y() - contentRect.top();
        int colIdx = relX / kColumnWidth;
        int rowIdx = relY / optionHeight;

        if (colIdx < 0 || colIdx >= m_columns.size())
        {
            return;
        }

        const ColumnState& col = m_columns.at(colIdx);
        if (!col.options || rowIdx < 0 || rowIdx >= col.options->size())
        {
            return;
        }

        const AntCascaderOption& opt = col.options->at(rowIdx);
        if (opt.disabled)
        {
            return;
        }

        handleColumnClick(colIdx, rowIdx);
        event->accept();
    }

    void hideEvent(QHideEvent* event) override
    {
        if (m_owner && m_owner->isOpen())
        {
            m_owner->setOpen(false);
        }
        QFrame::hideEvent(event);
    }

private:
    void handleColumnClick(int colIdx, int rowIdx)
    {
        if (colIdx < 0 || colIdx >= m_columns.size())
        {
            return;
        }

        const ColumnState& col = m_columns.at(colIdx);
        if (!col.options || rowIdx < 0 || rowIdx >= col.options->size())
        {
            return;
        }

        const AntCascaderOption& opt = col.options->at(rowIdx);

        QStringList newPath;
        for (int i = 0; i < colIdx; ++i)
        {
            const ColumnState& prevCol = m_columns.at(i);
            if (prevCol.selectedIndex >= 0 && prevCol.options)
            {
                newPath.push_back(prevCol.options->at(prevCol.selectedIndex).value.toString());
            }
        }
        newPath.push_back(opt.value.toString());

        if (opt.isLeaf || opt.children.isEmpty())
        {
            m_owner->setValue(newPath);
            m_owner->setOpen(false);
        }
        else
        {
            m_columns.resize(colIdx + 1);
            m_columns[colIdx].selectedIndex = rowIdx;
            m_columns[colIdx].highlightedIndex = -1;

            ColumnState newCol;
            newCol.options = &opt.children;
            newCol.highlightedIndex = -1;
            newCol.selectedIndex = -1;
            m_columns.push_back(newCol);

            m_owner->m_value = newPath;
            emit m_owner->valueChanged(m_owner->m_value);

            updateSizeAndPosition();
            update();
        }
    }

    void handleColumnExpand(int colIdx, int rowIdx)
    {
        if (colIdx < 0 || colIdx >= m_columns.size())
        {
            return;
        }

        const ColumnState& col = m_columns.at(colIdx);
        if (!col.options || rowIdx < 0 || rowIdx >= col.options->size())
        {
            return;
        }

        if (col.selectedIndex == rowIdx)
        {
            return;
        }

        const AntCascaderOption& opt = col.options->at(rowIdx);
        if (opt.disabled || opt.isLeaf || opt.children.isEmpty())
        {
            return;
        }

        handleColumnClick(colIdx, rowIdx);
    }

    AntCascader* m_owner = nullptr;
    QVector<ColumnState> m_columns;
    int m_hoveredColumn = -1;
    int m_hoveredRow = -1;
};

AntCascader::AntCascader(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_popup = new CascaderPopup(this);

    m_arrowAnimation = new QPropertyAnimation(this, "arrowRotation", this);
    m_arrowAnimation->setDuration(160);
    m_arrowAnimation->setEasingCurve(QEasingCurve::OutCubic);

    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        updateGeometry();
        update();
    });

    auto* cascaderStyle = new AntCascaderStyle(style());
    cascaderStyle->setParent(this);
    setStyle(cascaderStyle);

    updateCursor();
}

AntCascader::~AntCascader()
{
    if (m_popup)
    {
        m_popup->deleteLater();
    }
}

QVector<AntCascaderOption> AntCascader::options() const
{
    return m_options;
}

void AntCascader::setOptions(const QVector<AntCascaderOption>& options)
{
    m_options = options;
    resolveLeafFlags(m_options);
    update();
    emit optionsChanged();
}

QStringList AntCascader::value() const
{
    return m_value;
}

void AntCascader::setValue(const QStringList& path)
{
    if (m_value == path)
    {
        return;
    }

    m_value = path;
    update();
    emit valueChanged(m_value);
}

QString AntCascader::placeholder() const
{
    return m_placeholder;
}

void AntCascader::setPlaceholder(const QString& placeholder)
{
    if (m_placeholder == placeholder)
    {
        return;
    }
    m_placeholder = placeholder;
    update();
    emit placeholderChanged(m_placeholder);
}

bool AntCascader::allowClear() const
{
    return m_allowClear;
}

void AntCascader::setAllowClear(bool allowClear)
{
    if (m_allowClear == allowClear)
    {
        return;
    }
    m_allowClear = allowClear;
    update();
    emit allowClearChanged(m_allowClear);
}

Ant::SelectSize AntCascader::cascaderSize() const
{
    return m_cascaderSize;
}

void AntCascader::setCascaderSize(Ant::SelectSize size)
{
    if (m_cascaderSize == size)
    {
        return;
    }
    m_cascaderSize = size;
    updateGeometry();
    update();
    emit cascaderSizeChanged(m_cascaderSize);
}

Ant::SelectStatus AntCascader::status() const
{
    return m_status;
}

void AntCascader::setStatus(Ant::SelectStatus status)
{
    if (m_status == status)
    {
        return;
    }
    m_status = status;
    update();
    emit statusChanged(m_status);
}

Ant::SelectVariant AntCascader::variant() const
{
    return m_variant;
}

void AntCascader::setVariant(Ant::SelectVariant variant)
{
    if (m_variant == variant)
    {
        return;
    }
    m_variant = variant;
    update();
    emit variantChanged(m_variant);
}

Ant::CascaderExpandTrigger AntCascader::expandTrigger() const
{
    return m_expandTrigger;
}

void AntCascader::setExpandTrigger(Ant::CascaderExpandTrigger trigger)
{
    if (m_expandTrigger == trigger)
    {
        return;
    }
    m_expandTrigger = trigger;
    emit expandTriggerChanged(m_expandTrigger);
}

bool AntCascader::isOpen() const
{
    return m_open;
}

void AntCascader::setOpen(bool open)
{
    if (m_open == open)
    {
        return;
    }

    m_open = open;
    animateArrow(m_open);

    if (m_open)
    {
        auto* popup = static_cast<CascaderPopup*>(m_popup);
        popup->rebuildColumns();
        popup->updateSizeAndPosition();
        m_popup->show();
        m_popup->raise();
    }
    else
    {
        if (m_popup->isVisible())
        {
            m_popup->hide();
        }
    }

    update();
    emit openChanged(m_open);
}

qreal AntCascader::arrowRotation() const
{
    return m_arrowRotation;
}

void AntCascader::setArrowRotation(qreal rotation)
{
    m_arrowRotation = rotation;
    update();
}

QString AntCascader::displayText() const
{
    if (m_value.isEmpty())
    {
        return QString();
    }

    QStringList labels;
    if (buildLabelPath(m_options, m_value, labels, 0))
    {
        return labels.join(QStringLiteral(" / "));
    }

    return m_value.join(QStringLiteral(" / "));
}

bool AntCascader::isHoveredState() const
{
    return m_hovered;
}

bool AntCascader::isPressedState() const
{
    return m_pressed;
}

QSize AntCascader::sizeHint() const
{
    const Metrics m = metrics();
    return QSize(220, m.height);
}

QSize AntCascader::minimumSizeHint() const
{
    const Metrics m = metrics();
    return QSize(120, m.height);
}

void AntCascader::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void AntCascader::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    update();
    QWidget::leaveEvent(event);
}

void AntCascader::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isEnabled())
    {
        const Metrics m = metrics();
        if (canClear() && clearButtonRect(m).contains(event->position()))
        {
            setValue(QStringList());
            event->accept();
            return;
        }

        m_pressed = true;
        setFocus(Qt::MouseFocusReason);
        setOpen(!m_open);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntCascader::focusInEvent(QFocusEvent* event)
{
    update();
    QWidget::focusInEvent(event);
}

void AntCascader::focusOutEvent(QFocusEvent* event)
{
    if (!m_popup->isVisible())
    {
        setOpen(false);
    }
    update();
    QWidget::focusOutEvent(event);
}

void AntCascader::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        updateCursor();
        if (!isEnabled())
        {
            setOpen(false);
        }
        update();
    }
    QWidget::changeEvent(event);
}

AntCascader::Metrics AntCascader::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    m.height = token.controlHeight;
    m.fontSize = token.fontSize;
    m.radius = token.borderRadius;
    m.paddingX = token.paddingSM - token.lineWidth;
    m.arrowWidth = token.fontSize + token.paddingXS * 2;

    if (m_cascaderSize == Ant::SelectSize::Large)
    {
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
    }
    else if (m_cascaderSize == Ant::SelectSize::Small)
    {
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSizeSM;
        m.radius = token.borderRadiusSM;
        m.paddingX = token.paddingXS;
    }
    return m;
}

QRectF AntCascader::controlRect() const
{
    const Metrics m = metrics();
    return QRectF(1, (height() - m.height) / 2.0, width() - 2, m.height);
}

QRectF AntCascader::clearButtonRect(const Metrics& m) const
{
    const QRectF control = controlRect();
    const qreal size = std::min<qreal>(18, control.height() - 8);
    return QRectF(control.right() - m.paddingX - size,
                  control.center().y() - size / 2.0,
                  size,
                  size);
}

QColor AntCascader::borderColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorBorderDisabled;
    }
    if (m_status == Ant::SelectStatus::Error)
    {
        return (m_hovered || hasFocus() || m_open) ? token.colorErrorHover : token.colorError;
    }
    if (m_status == Ant::SelectStatus::Warning)
    {
        return (m_hovered || hasFocus() || m_open) ? token.colorWarningHover : token.colorWarning;
    }
    if (m_hovered || hasFocus() || m_open)
    {
        return token.colorPrimaryHover;
    }
    return token.colorBorder;
}

QColor AntCascader::backgroundColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorBgContainerDisabled;
    }
    if (m_variant == Ant::SelectVariant::Filled)
    {
        return m_hovered ? token.colorFillTertiary : token.colorFillQuaternary;
    }
    if (m_variant == Ant::SelectVariant::Borderless || m_variant == Ant::SelectVariant::Underlined)
    {
        return QColor(0, 0, 0, 0);
    }
    return token.colorBgContainer;
}

bool AntCascader::canClear() const
{
    return isEnabled() && m_allowClear && m_hovered && !m_value.isEmpty();
}

void AntCascader::updateCursor()
{
    setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

void AntCascader::animateArrow(bool open)
{
    m_arrowAnimation->stop();
    m_arrowAnimation->setStartValue(m_arrowRotation);
    m_arrowAnimation->setEndValue(open ? 180.0 : 0.0);
    m_arrowAnimation->start();
}

void AntCascader::updatePopupPosition()
{
    if (m_popup && m_popup->isVisible())
    {
        auto* popup = static_cast<CascaderPopup*>(m_popup);
        popup->rebuildColumns();
        popup->updateSizeAndPosition();
    }
}

void AntCascader::resolveLeafFlags(QVector<AntCascaderOption>& options)
{
    for (auto& opt : options)
    {
        if (opt.children.isEmpty())
        {
            opt.isLeaf = true;
        }
        else
        {
            opt.isLeaf = false;
            resolveLeafFlags(opt.children);
        }
    }
}

const AntCascaderOption* AntCascader::findOptionByValue(const QVector<AntCascaderOption>& options, const QVariant& value)
{
    for (const auto& opt : options)
    {
        if (opt.value == value)
        {
            return &opt;
        }
    }
    return nullptr;
}

bool AntCascader::buildLabelPath(const QVector<AntCascaderOption>& options, const QStringList& valuePath, QStringList& labelPath, int depth)
{
    if (depth >= valuePath.size())
    {
        return true;
    }

    const QString& val = valuePath.at(depth);
    for (const auto& opt : options)
    {
        if (opt.value.toString() == val)
        {
            labelPath.push_back(opt.label);
            if (depth + 1 == valuePath.size())
            {
                return true;
            }
            return buildLabelPath(opt.children, valuePath, labelPath, depth + 1);
        }
    }
    return false;
}

bool AntCascader::buildValuePathFromLabels(const QVector<AntCascaderOption>& options, const QStringList& labelPath, QStringList& valuePath, int depth)
{
    if (depth >= labelPath.size())
    {
        return true;
    }

    const QString& lbl = labelPath.at(depth);
    for (const auto& opt : options)
    {
        if (opt.label == lbl)
        {
            valuePath.push_back(opt.value.toString());
            if (depth + 1 == labelPath.size())
            {
                return true;
            }
            return buildValuePathFromLabels(opt.children, labelPath, valuePath, depth + 1);
        }
    }
    return false;
}
