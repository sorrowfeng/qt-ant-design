#include "AntDatePicker.h"

#include <QFocusEvent>
#include <QFrame>
#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStringList>

#include "../styles/AntDatePickerStyle.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"

class AntDatePickerPopup : public QFrame
{
public:
    explicit AntDatePickerPopup(AntDatePicker* owner)
        : QFrame(owner, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
          m_owner(owner)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setMouseTracking(true);
        resize(304, 344);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        if (!m_owner)
        {
            return;
        }

        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

        antTheme->drawEffectShadow(&painter, rect().adjusted(2, 2, -2, -2), 12, token.borderRadiusLG, 0.55);
        const QRectF panel = panelRect();
        painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter.setBrush(token.colorBgElevated);
        painter.drawRoundedRect(panel, token.borderRadiusLG, token.borderRadiusLG);

        drawHeader(painter, panel);
        drawWeekdays(painter, panel);
        drawDays(painter, panel);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        m_hoveredDate = dateAt(event->position());
        const bool overButton = previousRect().contains(event->position()) || nextRect().contains(event->position());
        setCursor((m_hoveredDate.isValid() || overButton) ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
        QFrame::mouseMoveEvent(event);
    }

    void leaveEvent(QEvent* event) override
    {
        m_hoveredDate = QDate();
        update();
        QFrame::leaveEvent(event);
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (!m_owner || event->button() != Qt::LeftButton)
        {
            QFrame::mousePressEvent(event);
            return;
        }

        if (previousRect().contains(event->position()))
        {
            m_owner->setPanelDate(m_owner->m_panelDate.addMonths(-1));
            event->accept();
            return;
        }
        if (nextRect().contains(event->position()))
        {
            m_owner->setPanelDate(m_owner->m_panelDate.addMonths(1));
            event->accept();
            return;
        }

        const QDate date = dateAt(event->position());
        if (date.isValid())
        {
            m_owner->selectDateFromPopup(date);
            event->accept();
            return;
        }

        QFrame::mousePressEvent(event);
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
    QRectF panelRect() const
    {
        return rect().adjusted(8, 4, -8, -8);
    }

    QRectF previousRect() const
    {
        const QRectF panel = panelRect();
        return QRectF(panel.left() + 12, panel.top() + 12, 28, 28);
    }

    QRectF nextRect() const
    {
        const QRectF panel = panelRect();
        return QRectF(panel.right() - 40, panel.top() + 12, 28, 28);
    }

    QRectF gridRect() const
    {
        const QRectF panel = panelRect();
        return QRectF(panel.left() + 14, panel.top() + 88, panel.width() - 28, 6 * 34);
    }

    QDate firstGridDate() const
    {
        const QDate first(m_owner->m_panelDate.year(), m_owner->m_panelDate.month(), 1);
        const int sundayBasedOffset = first.dayOfWeek() % 7;
        return first.addDays(-sundayBasedOffset);
    }

    QDate dateAt(const QPointF& pos) const
    {
        const QRectF grid = gridRect();
        if (!grid.contains(pos))
        {
            return QDate();
        }
        const qreal cellW = grid.width() / 7.0;
        const qreal cellH = grid.height() / 6.0;
        const int col = static_cast<int>((pos.x() - grid.left()) / cellW);
        const int row = static_cast<int>((pos.y() - grid.top()) / cellH);
        if (col < 0 || col > 6 || row < 0 || row > 5)
        {
            return QDate();
        }
        return firstGridDate().addDays(row * 7 + col);
    }

    void drawHeader(QPainter& painter, const QRectF& panel) const
    {
        const auto& token = antTheme->tokens();
        QFont font = painter.font();
        font.setPixelSize(token.fontSize);
        font.setWeight(QFont::DemiBold);
        painter.setFont(font);
        painter.setPen(token.colorText);
        painter.drawText(QRectF(panel.left() + 48, panel.top() + 10, panel.width() - 96, 32),
                         Qt::AlignCenter,
                         m_owner->m_panelDate.toString(QStringLiteral("MMMM yyyy")));

        painter.setPen(QPen(token.colorTextSecondary, 1.7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        drawChevron(painter, previousRect(), true);
        drawChevron(painter, nextRect(), false);

        painter.setPen(QPen(token.colorSplit, token.lineWidth));
        painter.drawLine(QPointF(panel.left(), panel.top() + 54), QPointF(panel.right(), panel.top() + 54));
    }

    void drawWeekdays(QPainter& painter, const QRectF& panel) const
    {
        const auto& token = antTheme->tokens();
        static const QStringList weekdays = {
            QStringLiteral("Su"),
            QStringLiteral("Mo"),
            QStringLiteral("Tu"),
            QStringLiteral("We"),
            QStringLiteral("Th"),
            QStringLiteral("Fr"),
            QStringLiteral("Sa"),
        };

        QFont font = painter.font();
        font.setPixelSize(token.fontSizeSM);
        font.setWeight(QFont::DemiBold);
        painter.setFont(font);
        painter.setPen(token.colorTextSecondary);

        const QRectF grid = gridRect();
        const qreal cellW = grid.width() / 7.0;
        for (int i = 0; i < weekdays.size(); ++i)
        {
            painter.drawText(QRectF(grid.left() + i * cellW, panel.top() + 60, cellW, 24), Qt::AlignCenter, weekdays.at(i));
        }
    }

    void drawDays(QPainter& painter, const QRectF& panel) const
    {
        Q_UNUSED(panel)
        const auto& token = antTheme->tokens();
        const QRectF grid = gridRect();
        const qreal cellW = grid.width() / 7.0;
        const qreal cellH = grid.height() / 6.0;
        const QDate today = QDate::currentDate();
        const QDate first = firstGridDate();

        QFont font = painter.font();
        font.setPixelSize(token.fontSize);
        font.setWeight(QFont::Normal);
        painter.setFont(font);

        for (int i = 0; i < 42; ++i)
        {
            const QDate date = first.addDays(i);
            const int row = i / 7;
            const int col = i % 7;
            const QRectF cell(grid.left() + col * cellW, grid.top() + row * cellH, cellW, cellH);
            const QRectF inner(cell.center().x() - 14, cell.center().y() - 14, 28, 28);
            const bool inMonth = date.month() == m_owner->m_panelDate.month();
            const bool selected = m_owner->m_selectedDate.isValid() && date == m_owner->m_selectedDate;
            const bool isToday = date == today;
            const bool hovered = date == m_hoveredDate;

            if (hovered && !selected)
            {
                painter.setPen(Qt::NoPen);
                painter.setBrush(token.colorFillQuaternary);
                painter.drawRoundedRect(inner, token.borderRadiusSM, token.borderRadiusSM);
            }
            if (selected)
            {
                painter.setPen(Qt::NoPen);
                painter.setBrush(token.colorPrimary);
                painter.drawRoundedRect(inner, token.borderRadiusSM, token.borderRadiusSM);
            }
            else if (isToday)
            {
                painter.setPen(QPen(token.colorPrimary, token.lineWidth));
                painter.setBrush(Qt::NoBrush);
                painter.drawRoundedRect(inner.adjusted(0.5, 0.5, -0.5, -0.5), token.borderRadiusSM, token.borderRadiusSM);
            }

            painter.setPen(selected ? token.colorTextLightSolid : (inMonth ? token.colorText : token.colorTextDisabled));
            painter.drawText(cell, Qt::AlignCenter, QString::number(date.day()));
        }
    }

    void drawChevron(QPainter& painter, const QRectF& rect, bool left) const
    {
        const QPointF center = rect.center();
        QPainterPath path;
        if (left)
        {
            path.moveTo(center.x() + 4, center.y() - 6);
            path.lineTo(center.x() - 3, center.y());
            path.lineTo(center.x() + 4, center.y() + 6);
        }
        else
        {
            path.moveTo(center.x() - 4, center.y() - 6);
            path.lineTo(center.x() + 3, center.y());
            path.lineTo(center.x() - 4, center.y() + 6);
        }
        painter.drawPath(path);
    }

    AntDatePicker* m_owner = nullptr;
    QDate m_hoveredDate;
};

AntDatePicker::AntDatePicker(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntDatePickerStyle(style()));
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_panelDate = QDate::currentDate();
    m_popup = new AntDatePickerPopup(this);

    updateCursor();
}

AntDatePicker::~AntDatePicker()
{
    delete m_popup;
    m_popup = nullptr;
}

QDate AntDatePicker::selectedDate() const { return m_selectedDate; }

void AntDatePicker::setSelectedDate(const QDate& date)
{
    if (m_selectedDate == date)
    {
        return;
    }
    m_selectedDate = date;
    if (m_selectedDate.isValid())
    {
        m_panelDate = m_selectedDate;
    }
    update();
    if (m_popup)
    {
        m_popup->update();
    }
    Q_EMIT selectedDateChanged(m_selectedDate);
    Q_EMIT dateStringChanged(dateString());
}

bool AntDatePicker::hasSelectedDate() const
{
    if (m_rangeMode)
    {
        return m_startDate.isValid() || m_endDate.isValid();
    }
    return m_selectedDate.isValid();
}

void AntDatePicker::clear()
{
    if (!hasSelectedDate())
    {
        return;
    }
    const bool hadSelectedDate = m_selectedDate.isValid();
    m_selectedDate = QDate();
    if (m_rangeMode)
    {
        m_startDate = QDate();
        m_endDate = QDate();
        m_pickingEnd = false;
        Q_EMIT startDateChanged(m_startDate);
        Q_EMIT endDateChanged(m_endDate);
    }
    if (hadSelectedDate)
    {
        Q_EMIT selectedDateChanged(m_selectedDate);
    }
    update();
    if (m_popup)
    {
        m_popup->update();
    }
    Q_EMIT dateStringChanged(dateString());
    Q_EMIT cleared();
}

QString AntDatePicker::displayFormat() const { return m_displayFormat; }

void AntDatePicker::setDisplayFormat(const QString& format)
{
    if (format.isEmpty() || m_displayFormat == format)
    {
        return;
    }
    m_displayFormat = format;
    update();
    Q_EMIT displayFormatChanged(m_displayFormat);
    Q_EMIT dateStringChanged(dateString());
}

QString AntDatePicker::placeholderText() const { return m_placeholderText; }

void AntDatePicker::setPlaceholderText(const QString& text)
{
    if (m_placeholderText == text)
    {
        return;
    }
    m_placeholderText = text;
    update();
    Q_EMIT placeholderTextChanged(m_placeholderText);
}

Ant::Size AntDatePicker::pickerSize() const { return m_pickerSize; }

void AntDatePicker::setPickerSize(Ant::Size size)
{
    if (m_pickerSize == size)
    {
        return;
    }
    m_pickerSize = size;
    updateGeometry();
    update();
    Q_EMIT pickerSizeChanged(m_pickerSize);
}

Ant::Status AntDatePicker::status() const { return m_status; }

void AntDatePicker::setStatus(Ant::Status status)
{
    if (m_status == status)
    {
        return;
    }
    m_status = status;
    update();
    Q_EMIT statusChanged(m_status);
}

Ant::Variant AntDatePicker::variant() const { return m_variant; }

void AntDatePicker::setVariant(Ant::Variant variant)
{
    if (m_variant == variant)
    {
        return;
    }
    m_variant = variant;
    update();
    Q_EMIT variantChanged(m_variant);
}

bool AntDatePicker::allowClear() const { return m_allowClear; }

void AntDatePicker::setAllowClear(bool allowClear)
{
    if (m_allowClear == allowClear)
    {
        return;
    }
    m_allowClear = allowClear;
    update();
    Q_EMIT allowClearChanged(m_allowClear);
}

bool AntDatePicker::isOpen() const { return m_open; }

void AntDatePicker::setOpen(bool open)
{
    if (m_open == open)
    {
        return;
    }
    m_open = open;
    if (m_open)
    {
        if (m_selectedDate.isValid())
        {
            m_panelDate = m_selectedDate;
        }
        updatePopupGeometry();
        m_popup->show();
        m_popup->raise();
    }
    else if (m_popup->isVisible())
    {
        m_popup->hide();
    }
    update();
    Q_EMIT openChanged(m_open);
}

QString AntDatePicker::dateString() const
{
    if (m_rangeMode)
    {
        const QString startText = m_startDate.isValid() ? m_startDate.toString(m_displayFormat) : QString();
        const QString endText = m_endDate.isValid() ? m_endDate.toString(m_displayFormat) : QString();
        if (startText.isEmpty() && endText.isEmpty())
        {
            return QString();
        }
        return QStringLiteral("%1 - %2").arg(startText, endText);
    }
    return m_selectedDate.isValid() ? m_selectedDate.toString(m_displayFormat) : QString();
}

bool AntDatePicker::isHoveredState() const { return m_hovered; }

QSize AntDatePicker::sizeHint() const
{
    return QSize(m_rangeMode ? 280 : 180, metrics().height);
}

QSize AntDatePicker::minimumSizeHint() const
{
    return QSize(120, metrics().height);
}

void AntDatePicker::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntDatePicker::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void AntDatePicker::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    update();
    QWidget::leaveEvent(event);
}

void AntDatePicker::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isEnabled())
    {
        if (canClear() && iconRect(metrics()).contains(event->position()))
        {
            clear();
            event->accept();
            return;
        }
        setFocus(Qt::MouseFocusReason);
        setOpen(!m_open);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntDatePicker::keyPressEvent(QKeyEvent* event)
{
    if (!isEnabled())
    {
        QWidget::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        setOpen(!m_open);
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Escape)
    {
        setOpen(false);
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)
    {
        setPanelDate(m_panelDate.addMonths(event->key() == Qt::Key_Left ? -1 : 1));
        setOpen(true);
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void AntDatePicker::focusInEvent(QFocusEvent* event)
{
    update();
    QWidget::focusInEvent(event);
}

void AntDatePicker::focusOutEvent(QFocusEvent* event)
{
    if (!m_popup->isVisible())
    {
        setOpen(false);
    }
    update();
    QWidget::focusOutEvent(event);
}

void AntDatePicker::changeEvent(QEvent* event)
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

AntDatePicker::Metrics AntDatePicker::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    m.height = token.controlHeight;
    m.fontSize = token.fontSize;
    m.radius = token.borderRadius;
    m.paddingX = token.paddingSM - token.lineWidth;
    m.iconWidth = token.fontSize + token.paddingXS * 2;
    if (m_pickerSize == Ant::Size::Large)
    {
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
    }
    else if (m_pickerSize == Ant::Size::Small)
    {
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSizeSM;
        m.radius = token.borderRadiusSM;
        m.paddingX = token.paddingXS;
    }
    return m;
}

QRectF AntDatePicker::controlRect() const
{
    const Metrics m = metrics();
    return QRectF(1, (height() - m.height) / 2.0, width() - 2, m.height);
}

QRectF AntDatePicker::iconRect(const Metrics& metrics) const
{
    const QRectF control = controlRect();
    return QRectF(control.right() - metrics.iconWidth, control.top(), metrics.iconWidth, control.height());
}

QColor AntDatePicker::borderColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorBorderDisabled;
    }
    if (m_status == Ant::Status::Error)
    {
        return (m_hovered || hasFocus() || m_open) ? token.colorErrorHover : token.colorError;
    }
    if (m_status == Ant::Status::Warning)
    {
        return (m_hovered || hasFocus() || m_open) ? token.colorWarningHover : token.colorWarning;
    }
    if (m_hovered || hasFocus() || m_open)
    {
        return token.colorPrimaryHover;
    }
    return token.colorBorder;
}

QColor AntDatePicker::backgroundColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorBgContainerDisabled;
    }
    if (m_variant == Ant::Variant::Filled)
    {
        return m_hovered ? token.colorFillTertiary : token.colorFillQuaternary;
    }
    if (m_variant == Ant::Variant::Borderless || m_variant == Ant::Variant::Underlined)
    {
        return QColor(0, 0, 0, 0);
    }
    return token.colorBgContainer;
}

bool AntDatePicker::canClear() const
{
    return isEnabled() && m_allowClear && m_hovered && m_selectedDate.isValid();
}

void AntDatePicker::updatePopupGeometry()
{
    if (!m_popup)
    {
        return;
    }
    const QPoint globalPos = mapToGlobal(QPoint(0, height() + 4));
    m_popup->move(globalPos);
}

void AntDatePicker::setPanelDate(const QDate& date)
{
    if (!date.isValid())
    {
        return;
    }
    m_panelDate = QDate(date.year(), date.month(), 1);
    if (m_popup)
    {
        m_popup->update();
    }
}

void AntDatePicker::selectDateFromPopup(const QDate& date)
{
    if (m_rangeMode)
    {
        if (!m_pickingEnd || !m_startDate.isValid())
        {
            m_startDate = date;
            m_endDate = QDate();
            m_pickingEnd = true;
            update();
            Q_EMIT startDateChanged(m_startDate);
            Q_EMIT endDateChanged(m_endDate);
            Q_EMIT dateStringChanged(dateString());
        }
        else
        {
            m_endDate = (date >= m_startDate) ? date : m_startDate;
            if (date < m_startDate) m_startDate = date;
            m_pickingEnd = false;
            setSelectedDate(m_startDate); // update display
            setOpen(false);
            Q_EMIT startDateChanged(m_startDate);
            Q_EMIT endDateChanged(m_endDate);
        }
    }
    else
    {
        setSelectedDate(date);
        setOpen(false);
    }
}

bool AntDatePicker::isRangeMode() const { return m_rangeMode; }
void AntDatePicker::setRangeMode(bool rangeMode)
{
    if (m_rangeMode == rangeMode) return;
    m_rangeMode = rangeMode;
    m_pickingEnd = false;
    updateGeometry();
    update();
    Q_EMIT rangeModeChanged(m_rangeMode);
    Q_EMIT dateStringChanged(dateString());
}

QDate AntDatePicker::startDate() const { return m_startDate; }
void AntDatePicker::setStartDate(const QDate& date)
{
    if (m_startDate == date) return;
    m_startDate = date;
    updateGeometry();
    update();
    Q_EMIT startDateChanged(m_startDate);
    Q_EMIT dateStringChanged(dateString());
}

QDate AntDatePicker::endDate() const { return m_endDate; }
void AntDatePicker::setEndDate(const QDate& date)
{
    if (m_endDate == date) return;
    m_endDate = date;
    updateGeometry();
    update();
    Q_EMIT endDateChanged(m_endDate);
    Q_EMIT dateStringChanged(dateString());
}

void AntDatePicker::updateCursor()
{
    setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
}
