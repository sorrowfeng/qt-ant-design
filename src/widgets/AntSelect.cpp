#include "AntSelect.h"

#include <QApplication>
#include <QFocusEvent>
#include <QFrame>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

#include "../styles/AntSelectStyle.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"

class AntSelectPopup : public QFrame
{
public:
    explicit AntSelectPopup(AntSelect* owner)
        : QFrame(nullptr, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
          m_owner(owner)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setObjectName(QStringLiteral("AntSelectPopup"));
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)

        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        antTheme->drawEffectShadow(&painter, rect().adjusted(2, 2, -2, -2), 10, token.borderRadiusLG, 0.55);

        QRectF panel = rect().adjusted(8, 4, -8, -8);
        painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter.setBrush(token.colorBgElevated);
        painter.drawRoundedRect(panel, token.borderRadiusLG, token.borderRadiusLG);
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
    AntSelect* m_owner = nullptr;
};

class AntSelectOptionWidget : public QWidget
{
public:
    AntSelectOptionWidget(AntSelect* select, int index, QWidget* parent)
        : QWidget(parent),
          m_select(select),
          m_index(index)
    {
        setAttribute(Qt::WA_Hover, true);
        setMouseTracking(true);
        setFixedHeight(antTheme->tokens().controlHeight);
        setCursor(Qt::PointingHandCursor);
    }

    void setIndex(int index)
    {
        m_index = index;
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        if (!m_select || m_index < 0 || m_index >= m_select->m_options.size())
        {
            return;
        }

        const auto& token = antTheme->tokens();
        const AntSelectOption option = m_select->m_options.at(m_index);
        const bool selected = m_select->m_currentIndex == m_index;
        const bool highlighted = m_select->m_highlightedIndex == m_index || m_hovered;
        const bool disabled = option.disabled;

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        QRectF bg = rect().adjusted(4, 2, -4, -2);
        if (selected)
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(AntPalette::alpha(token.colorPrimary, antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.24 : 0.12));
            painter.drawRoundedRect(bg, token.borderRadiusSM, token.borderRadiusSM);
        }
        else if (highlighted && !disabled)
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(token.colorFillQuaternary);
            painter.drawRoundedRect(bg, token.borderRadiusSM, token.borderRadiusSM);
        }

        QFont f = painter.font();
        f.setPixelSize(token.fontSize);
        f.setWeight(selected ? QFont::DemiBold : QFont::Normal);
        painter.setFont(f);
        painter.setPen(disabled ? token.colorTextDisabled : token.colorText);

        QRect textRect = rect().adjusted(14, 0, selected ? -34 : -14, 0);
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, option.label);

        if (selected)
        {
            QPen checkPen(disabled ? token.colorTextDisabled : token.colorPrimary, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter.setPen(checkPen);
            const qreal cx = width() - 20;
            const qreal cy = height() / 2.0;
            QPainterPath check;
            check.moveTo(cx - 6, cy);
            check.lineTo(cx - 2, cy + 4);
            check.lineTo(cx + 7, cy - 6);
            painter.drawPath(check);
        }
    }

    void enterEvent(QEnterEvent* event) override
    {
        m_hovered = true;
        if (m_select)
        {
            m_select->setHighlightedIndex(m_index);
        }
        update();
        QWidget::enterEvent(event);
    }

    void leaveEvent(QEvent* event) override
    {
        m_hovered = false;
        update();
        QWidget::leaveEvent(event);
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton && m_select && m_index >= 0 && m_index < m_select->m_options.size() && !m_select->m_options.at(m_index).disabled)
        {
            m_select->selectOptionFromPopup(m_index);
            event->accept();
            return;
        }
        QWidget::mousePressEvent(event);
    }

private:
    AntSelect* m_select = nullptr;
    int m_index = -1;
    bool m_hovered = false;
};

AntSelect::AntSelect(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_popup = new AntSelectPopup(this);
    m_popupLayout = new QVBoxLayout(m_popup);
    m_popupLayout->setContentsMargins(12, 8, 12, 12);
    m_popupLayout->setSpacing(0);

    m_arrowAnimation = new QPropertyAnimation(this, "arrowRotation", this);
    m_arrowAnimation->setDuration(160);
    m_arrowAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_loadingTimer = new QTimer(this);
    connect(m_loadingTimer, &QTimer::timeout, this, [this]() {
        m_loadingAngle = (m_loadingAngle + 30) % 360;
        update();
    });

    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        rebuildPopup();
        updateGeometry();
        update();
    });

    auto* selectStyle = new AntSelectStyle(style());
    selectStyle->setParent(this);
    setStyle(selectStyle);

    updateCursor();
}

AntSelect::~AntSelect()
{
    if (m_popup)
    {
        m_popup->deleteLater();
    }
}

Ant::SelectSize AntSelect::selectSize() const { return m_selectSize; }

void AntSelect::setSelectSize(Ant::SelectSize size)
{
    if (m_selectSize == size)
    {
        return;
    }
    m_selectSize = size;
    rebuildPopup();
    updateGeometry();
    update();
    Q_EMIT selectSizeChanged(m_selectSize);
}

Ant::SelectStatus AntSelect::status() const { return m_status; }

void AntSelect::setStatus(Ant::SelectStatus status)
{
    if (m_status == status)
    {
        return;
    }
    m_status = status;
    update();
    Q_EMIT statusChanged(m_status);
}

Ant::SelectVariant AntSelect::variant() const { return m_variant; }

void AntSelect::setVariant(Ant::SelectVariant variant)
{
    if (m_variant == variant)
    {
        return;
    }
    m_variant = variant;
    update();
    Q_EMIT variantChanged(m_variant);
}

QString AntSelect::placeholderText() const { return m_placeholderText; }

void AntSelect::setPlaceholderText(const QString& text)
{
    if (m_placeholderText == text)
    {
        return;
    }
    m_placeholderText = text;
    update();
    Q_EMIT placeholderTextChanged(m_placeholderText);
}

bool AntSelect::allowClear() const { return m_allowClear; }

void AntSelect::setAllowClear(bool allowClear)
{
    if (m_allowClear == allowClear)
    {
        return;
    }
    m_allowClear = allowClear;
    update();
    Q_EMIT allowClearChanged(m_allowClear);
}

bool AntSelect::isEditable() const { return m_editable; }

void AntSelect::setEditable(bool editable)
{
    if (m_editable == editable) return;
    m_editable = editable;
    if (editable)
    {
        if (!m_editField)
        {
            m_editField = new QLineEdit(this);
            m_editField->setFrame(false);
            m_editField->setStyleSheet(QStringLiteral("background:transparent; border:none;"));
            m_editField->setVisible(false);
            connect(m_editField, &QLineEdit::textChanged, this, [this]() {
                if (!m_open) setOpen(true);
                rebuildPopup();
            });
        }
    }
    else if (m_editField)
    {
        m_editField->hide();
    }
    update();
    Q_EMIT editableChanged(m_editable);
}

bool AntSelect::isLoading() const { return m_loading; }

void AntSelect::setLoading(bool loading)
{
    if (m_loading == loading)
    {
        return;
    }
    m_loading = loading;
    m_loading ? m_loadingTimer->start(80) : m_loadingTimer->stop();
    update();
    Q_EMIT loadingChanged(m_loading);
}

bool AntSelect::isOpen() const { return m_open; }

void AntSelect::setOpen(bool open)
{
    if (m_open == open)
    {
        return;
    }

    m_open = open;
    animateArrow(m_open);

    if (m_open)
    {
        rebuildPopup();
        updatePopupGeometry();
        m_highlightedIndex = m_currentIndex >= 0 ? m_currentIndex : nextEnabledIndex(-1, 1);
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
    Q_EMIT openChanged(m_open);
}

int AntSelect::currentIndex() const { return m_currentIndex; }

void AntSelect::setCurrentIndex(int index)
{
    if (index < -1 || index >= m_options.size())
    {
        return;
    }
    if (index >= 0 && m_options.at(index).disabled)
    {
        return;
    }
    if (m_currentIndex == index)
    {
        return;
    }

    m_currentIndex = index;
    m_highlightedIndex = index;
    rebuildPopup();
    update();
    Q_EMIT currentIndexChanged(m_currentIndex);
    Q_EMIT currentTextChanged(currentText());
    Q_EMIT currentValueChanged(currentValue());
}

QString AntSelect::currentText() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_options.size())
    {
        return QString();
    }
    return m_options.at(m_currentIndex).label;
}

QVariant AntSelect::currentValue() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_options.size())
    {
        return QVariant();
    }
    return m_options.at(m_currentIndex).value;
}

int AntSelect::count() const { return m_options.size(); }

AntSelectOption AntSelect::optionAt(int index) const
{
    if (index < 0 || index >= m_options.size())
    {
        return {};
    }
    return m_options.at(index);
}

void AntSelect::addOption(const QString& label, const QVariant& value, bool disabled)
{
    m_options.push_back({label, value.isValid() ? value : QVariant(label), disabled});
    if (m_currentIndex == -1 && !disabled)
    {
        m_highlightedIndex = m_options.size() - 1;
    }
    rebuildPopup();
    update();
}

void AntSelect::addOptions(const QStringList& labels)
{
    for (const QString& label : labels)
    {
        addOption(label, label);
    }
}

void AntSelect::clearOptions()
{
    m_options.clear();
    m_currentIndex = -1;
    m_highlightedIndex = -1;
    rebuildPopup();
    update();
    Q_EMIT currentIndexChanged(m_currentIndex);
    Q_EMIT currentTextChanged(QString());
    Q_EMIT currentValueChanged(QVariant());
}

int AntSelect::maxVisibleItems() const { return m_maxVisibleItems; }

void AntSelect::setMaxVisibleItems(int count)
{
    count = std::max(1, count);
    if (m_maxVisibleItems == count)
    {
        return;
    }
    m_maxVisibleItems = count;
    updatePopupGeometry();
    Q_EMIT maxVisibleItemsChanged(m_maxVisibleItems);
}

qreal AntSelect::arrowRotation() const { return m_arrowRotation; }

void AntSelect::setArrowRotation(qreal rotation)
{
    m_arrowRotation = rotation;
    update();
}

bool AntSelect::isHoveredState() const { return m_hovered; }

bool AntSelect::isPressedState() const { return m_pressed; }

int AntSelect::loadingAngle() const { return m_loadingAngle; }

QSize AntSelect::sizeHint() const
{
    const Metrics m = metrics();
    return QSize(220, m.height);
}

QSize AntSelect::minimumSizeHint() const
{
    const Metrics m = metrics();
    return QSize(96, m.height);
}

void AntSelect::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void AntSelect::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    update();
    QWidget::leaveEvent(event);
}

void AntSelect::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isEnabled())
    {
        const Metrics m = metrics();
        if (canClear() && clearButtonRect(m).contains(event->position()))
        {
            setCurrentIndex(-1);
            Q_EMIT cleared();
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

void AntSelect::keyPressEvent(QKeyEvent* event)
{
    if (!isEnabled())
    {
        QWidget::keyPressEvent(event);
        return;
    }

    switch (event->key())
    {
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (m_open && m_highlightedIndex >= 0)
        {
            selectOptionFromPopup(m_highlightedIndex);
        }
        else
        {
            setOpen(true);
        }
        event->accept();
        return;
    case Qt::Key_Escape:
        setOpen(false);
        event->accept();
        return;
    case Qt::Key_Down:
        if (!m_open)
        {
            setOpen(true);
        }
        setHighlightedIndex(nextEnabledIndex(m_highlightedIndex, 1));
        event->accept();
        return;
    case Qt::Key_Up:
        if (!m_open)
        {
            setOpen(true);
        }
        setHighlightedIndex(nextEnabledIndex(m_highlightedIndex, -1));
        event->accept();
        return;
    default:
        QWidget::keyPressEvent(event);
        return;
    }
}

void AntSelect::focusInEvent(QFocusEvent* event)
{
    update();
    QWidget::focusInEvent(event);
}

void AntSelect::focusOutEvent(QFocusEvent* event)
{
    if (!m_popup->isVisible())
    {
        setOpen(false);
    }
    update();
    QWidget::focusOutEvent(event);
}

void AntSelect::changeEvent(QEvent* event)
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

AntSelect::Metrics AntSelect::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    m.height = token.controlHeight;
    m.fontSize = token.fontSize;
    m.radius = token.borderRadius;
    m.paddingX = token.paddingSM - token.lineWidth;
    m.arrowWidth = token.fontSize + token.paddingXS * 2;
    m.optionHeight = token.controlHeight;

    if (m_selectSize == Ant::SelectSize::Large)
    {
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
        m.optionHeight = token.controlHeightLG;
    }
    else if (m_selectSize == Ant::SelectSize::Small)
    {
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSizeSM;
        m.radius = token.borderRadiusSM;
        m.paddingX = token.paddingXS;
        m.optionHeight = token.controlHeightSM;
    }
    return m;
}

QRectF AntSelect::controlRect() const
{
    const Metrics m = metrics();
    return QRectF(1, (height() - m.height) / 2.0, width() - 2, m.height);
}

QRectF AntSelect::clearButtonRect(const Metrics& metrics) const
{
    const QRectF control = controlRect();
    const qreal size = std::min<qreal>(18, control.height() - 8);
    return QRectF(control.right() - metrics.paddingX - size, control.center().y() - size / 2.0, size, size);
}

QColor AntSelect::borderColor() const
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

QColor AntSelect::backgroundColor() const
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

void AntSelect::rebuildPopup()
{
    if (!m_popupLayout)
    {
        return;
    }

    // Filter for editable mode
    QString filterText;
    if (m_editable && m_editField)
        filterText = m_editField->text().toLower();

    while (QLayoutItem* item = m_popupLayout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
        {
            widget->deleteLater();
        }
        delete item;
    }

    const Metrics m = metrics();
    for (int i = 0; i < m_options.size(); ++i)
    {
        if (!filterText.isEmpty() && !m_options.at(i).label.toLower().contains(filterText))
            continue;
        auto* option = new AntSelectOptionWidget(this, i, m_popup);
        option->setFixedHeight(m.optionHeight);
        option->setEnabled(!m_options.at(i).disabled);
        m_popupLayout->addWidget(option);
    }

    updatePopupGeometry();
}

void AntSelect::updatePopupGeometry()
{
    if (!m_popup)
    {
        return;
    }

    const Metrics m = metrics();
    const int optionCount = static_cast<int>(m_options.size());
    const int visibleCount = std::min(std::max(1, optionCount), m_maxVisibleItems);
    const int popupWidth = width();
    const int popupHeight = visibleCount * m.optionHeight + 20;
    const QPoint globalPos = mapToGlobal(QPoint(0, height() + 4));
    m_popup->setGeometry(globalPos.x(), globalPos.y(), popupWidth, popupHeight);
}

void AntSelect::selectOptionFromPopup(int index)
{
    if (index < 0 || index >= m_options.size() || m_options.at(index).disabled)
    {
        return;
    }

    setCurrentIndex(index);
    setOpen(false);
    Q_EMIT optionSelected(m_currentIndex, currentValue());
}

void AntSelect::setHighlightedIndex(int index)
{
    if (m_highlightedIndex == index || index < -1 || index >= m_options.size())
    {
        return;
    }
    if (index >= 0 && m_options.at(index).disabled)
    {
        return;
    }

    m_highlightedIndex = index;
    if (m_popup)
    {
        m_popup->update();
    }
}

int AntSelect::nextEnabledIndex(int start, int direction) const
{
    if (m_options.isEmpty())
    {
        return -1;
    }

    int index = start;
    for (int i = 0; i < m_options.size(); ++i)
    {
        index += direction;
        if (index < 0)
        {
            index = m_options.size() - 1;
        }
        else if (index >= m_options.size())
        {
            index = 0;
        }
        if (!m_options.at(index).disabled)
        {
            return index;
        }
    }
    return -1;
}

void AntSelect::animateArrow(bool open)
{
    m_arrowAnimation->stop();
    m_arrowAnimation->setStartValue(m_arrowRotation);
    m_arrowAnimation->setEndValue(open ? 180.0 : 0.0);
    m_arrowAnimation->start();
}

void AntSelect::updateCursor()
{
    setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

bool AntSelect::canClear() const
{
    return isEnabled() && m_allowClear && m_hovered && !m_loading && m_currentIndex >= 0;
}
