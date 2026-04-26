#include "AntTimePicker.h"

#include <QFocusEvent>
#include <QFrame>
#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStringList>
#include <QWheelEvent>

#include <algorithm>

#include "../styles/AntTimePickerStyle.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"

class AntTimePickerPopup : public QFrame
{
public:
    explicit AntTimePickerPopup(AntTimePicker* owner)
        : QFrame(nullptr, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
          m_owner(owner)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setMouseTracking(true);
        resize(248, 286);
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

        drawColumns(painter, panel);
        drawFooter(painter, panel);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        m_hoveredColumn = columnAt(event->position());
        m_hoveredRow = rowAt(event->position());
        const bool clickable = (m_hoveredColumn >= 0 && m_hoveredRow >= 0) || nowRect().contains(event->position()) || okRect().contains(event->position());
        setCursor(clickable ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
        QFrame::mouseMoveEvent(event);
    }

    void leaveEvent(QEvent* event) override
    {
        m_hoveredColumn = -1;
        m_hoveredRow = -1;
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

        if (m_owner->m_showNow && nowRect().contains(event->position()))
        {
            m_owner->setPanelTime(QTime::currentTime());
            m_owner->setSelectedTime(m_owner->m_panelTime);
            update();
            event->accept();
            return;
        }
        if (okRect().contains(event->position()))
        {
            m_owner->acceptPanelTime();
            event->accept();
            return;
        }

        const int col = columnAt(event->position());
        const int row = rowAt(event->position());
        if (col >= 0 && row >= 0)
        {
            QTime next = m_owner->m_panelTime;
            if (!next.isValid())
            {
                next = QTime::currentTime();
            }
            const int value = valueForRow(col, row);
            if (col == 0)
            {
                next.setHMS(value, next.minute(), next.second());
            }
            else if (col == 1)
            {
                next.setHMS(next.hour(), value, next.second());
            }
            else
            {
                next.setHMS(next.hour(), next.minute(), value);
            }
            m_owner->setPanelTime(next);
            m_owner->setSelectedTime(next);
            update();
            event->accept();
            return;
        }

        QFrame::mousePressEvent(event);
    }

    void wheelEvent(QWheelEvent* event) override
    {
        const int col = columnAt(event->position());
        if (!m_owner || col < 0)
        {
            QFrame::wheelEvent(event);
            return;
        }

        QTime next = m_owner->m_panelTime.isValid() ? m_owner->m_panelTime : QTime::currentTime();
        const int direction = event->angleDelta().y() > 0 ? -1 : 1;
        if (col == 0)
        {
            next = next.addSecs(direction * m_owner->m_hourStep * 3600);
        }
        else if (col == 1)
        {
            next = next.addSecs(direction * m_owner->m_minuteStep * 60);
        }
        else
        {
            next = next.addSecs(direction * m_owner->m_secondStep);
        }
        m_owner->setPanelTime(next);
        m_owner->setSelectedTime(next);
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
    QRectF panelRect() const
    {
        return rect().adjusted(8, 4, -8, -8);
    }

    QRectF columnsRect() const
    {
        const QRectF panel = panelRect();
        return QRectF(panel.left() + 10, panel.top() + 12, panel.width() - 20, 7 * 28);
    }

    QRectF footerRect() const
    {
        const QRectF panel = panelRect();
        return QRectF(panel.left(), panel.bottom() - 48, panel.width(), 48);
    }

    QRectF nowRect() const
    {
        const QRectF footer = footerRect();
        return QRectF(footer.left() + 12, footer.top() + 9, 64, 30);
    }

    QRectF okRect() const
    {
        const QRectF footer = footerRect();
        return QRectF(footer.right() - 70, footer.top() + 9, 56, 30);
    }

    int columnAt(const QPointF& pos) const
    {
        const QRectF columns = columnsRect();
        if (!columns.contains(pos))
        {
            return -1;
        }
        const int col = static_cast<int>((pos.x() - columns.left()) / (columns.width() / 3.0));
        return std::clamp(col, 0, 2);
    }

    int rowAt(const QPointF& pos) const
    {
        const QRectF columns = columnsRect();
        if (!columns.contains(pos))
        {
            return -1;
        }
        const int row = static_cast<int>((pos.y() - columns.top()) / 28.0);
        return row >= 0 && row < 7 ? row : -1;
    }

    int valueForRow(int column, int row) const
    {
        const QTime time = m_owner->m_panelTime.isValid() ? m_owner->m_panelTime : QTime::currentTime();
        const int center = column == 0 ? time.hour() : (column == 1 ? time.minute() : time.second());
        const int step = column == 0 ? m_owner->m_hourStep : (column == 1 ? m_owner->m_minuteStep : m_owner->m_secondStep);
        const int max = column == 0 ? 24 : 60;
        int value = center + (row - 3) * step;
        while (value < 0)
        {
            value += max;
        }
        return value % max;
    }

    void drawColumns(QPainter& painter, const QRectF& panel) const
    {
        Q_UNUSED(panel)
        const auto& token = antTheme->tokens();
        const QRectF columns = columnsRect();
        const qreal colW = columns.width() / 3.0;

        QFont headerFont = painter.font();
        headerFont.setPixelSize(token.fontSizeSM);
        headerFont.setWeight(QFont::DemiBold);
        painter.setFont(headerFont);
        painter.setPen(token.colorTextSecondary);
        const QStringList headers = {QStringLiteral("Hour"), QStringLiteral("Minute"), QStringLiteral("Second")};
        for (int c = 0; c < 3; ++c)
        {
            painter.drawText(QRectF(columns.left() + c * colW, columns.top() - 2, colW, 20), Qt::AlignCenter, headers.at(c));
        }

        QFont valueFont = painter.font();
        valueFont.setPixelSize(token.fontSize);
        painter.setFont(valueFont);

        for (int c = 0; c < 3; ++c)
        {
            const QRectF colRect(columns.left() + c * colW, columns.top() + 20, colW, columns.height() - 20);
            if (c > 0)
            {
                painter.setPen(QPen(token.colorSplit, token.lineWidth));
                painter.drawLine(QPointF(colRect.left(), colRect.top()), QPointF(colRect.left(), colRect.bottom()));
            }

            for (int r = 0; r < 7; ++r)
            {
                const QRectF rowRect(colRect.left() + 5, colRect.top() + r * 24, colRect.width() - 10, 22);
                const bool selected = r == 3;
                const bool hovered = c == m_hoveredColumn && r == m_hoveredRow;
                if (selected)
                {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(AntPalette::alpha(token.colorPrimary, antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.26 : 0.12));
                    painter.drawRoundedRect(rowRect, token.borderRadiusSM, token.borderRadiusSM);
                }
                else if (hovered)
                {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(token.colorFillQuaternary);
                    painter.drawRoundedRect(rowRect, token.borderRadiusSM, token.borderRadiusSM);
                }

                painter.setPen(selected ? token.colorPrimary : token.colorText);
                painter.drawText(rowRect, Qt::AlignCenter, QStringLiteral("%1").arg(valueForRow(c, r), 2, 10, QLatin1Char('0')));
            }
        }
    }

    void drawFooter(QPainter& painter, const QRectF& panel) const
    {
        const auto& token = antTheme->tokens();
        const QRectF footer = footerRect();
        painter.setPen(QPen(token.colorSplit, token.lineWidth));
        painter.drawLine(QPointF(panel.left(), footer.top()), QPointF(panel.right(), footer.top()));

        QFont font = painter.font();
        font.setPixelSize(token.fontSize);
        font.setWeight(QFont::DemiBold);
        painter.setFont(font);

        if (m_owner->m_showNow)
        {
            painter.setPen(token.colorPrimary);
            painter.drawText(nowRect(), Qt::AlignCenter, QStringLiteral("Now"));
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(token.colorPrimary);
        painter.drawRoundedRect(okRect(), token.borderRadiusSM, token.borderRadiusSM);
        painter.setPen(token.colorTextLightSolid);
        painter.drawText(okRect(), Qt::AlignCenter, QStringLiteral("OK"));
    }

    AntTimePicker* m_owner = nullptr;
    int m_hoveredColumn = -1;
    int m_hoveredRow = -1;
};

AntTimePicker::AntTimePicker(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntTimePickerStyle(style()));
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_panelTime = QTime::currentTime();
    m_popup = new AntTimePickerPopup(this);

    updateCursor();
}

AntTimePicker::~AntTimePicker()
{
    if (m_popup)
    {
        m_popup->deleteLater();
    }
}

QTime AntTimePicker::selectedTime() const { return m_selectedTime; }

void AntTimePicker::setSelectedTime(const QTime& time)
{
    if (m_selectedTime == time)
    {
        return;
    }
    m_selectedTime = time;
    if (m_selectedTime.isValid())
    {
        m_panelTime = m_selectedTime;
    }
    update();
    if (m_popup)
    {
        m_popup->update();
    }
    Q_EMIT selectedTimeChanged(m_selectedTime);
    Q_EMIT timeStringChanged(timeString());
}

bool AntTimePicker::hasSelectedTime() const { return m_selectedTime.isValid(); }

void AntTimePicker::clear()
{
    if (!m_selectedTime.isValid())
    {
        return;
    }
    setSelectedTime(QTime());
    Q_EMIT cleared();
}

QString AntTimePicker::displayFormat() const { return m_displayFormat; }

void AntTimePicker::setDisplayFormat(const QString& format)
{
    if (format.isEmpty() || m_displayFormat == format)
    {
        return;
    }
    m_displayFormat = format;
    update();
    Q_EMIT displayFormatChanged(m_displayFormat);
    Q_EMIT timeStringChanged(timeString());
}

QString AntTimePicker::placeholderText() const { return m_placeholderText; }

void AntTimePicker::setPlaceholderText(const QString& text)
{
    if (m_placeholderText == text)
    {
        return;
    }
    m_placeholderText = text;
    update();
    Q_EMIT placeholderTextChanged(m_placeholderText);
}

Ant::Size AntTimePicker::pickerSize() const { return m_pickerSize; }

void AntTimePicker::setPickerSize(Ant::Size size)
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

Ant::Status AntTimePicker::status() const { return m_status; }

void AntTimePicker::setStatus(Ant::Status status)
{
    if (m_status == status)
    {
        return;
    }
    m_status = status;
    update();
    Q_EMIT statusChanged(m_status);
}

Ant::Variant AntTimePicker::variant() const { return m_variant; }

void AntTimePicker::setVariant(Ant::Variant variant)
{
    if (m_variant == variant)
    {
        return;
    }
    m_variant = variant;
    update();
    Q_EMIT variantChanged(m_variant);
}

bool AntTimePicker::allowClear() const { return m_allowClear; }

void AntTimePicker::setAllowClear(bool allowClear)
{
    if (m_allowClear == allowClear)
    {
        return;
    }
    m_allowClear = allowClear;
    update();
    Q_EMIT allowClearChanged(m_allowClear);
}

bool AntTimePicker::isOpen() const { return m_open; }

void AntTimePicker::setOpen(bool open)
{
    if (m_open == open)
    {
        return;
    }
    m_open = open;
    if (m_open)
    {
        if (m_selectedTime.isValid())
        {
            m_panelTime = m_selectedTime;
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

int AntTimePicker::hourStep() const { return m_hourStep; }

void AntTimePicker::setHourStep(int step)
{
    step = normalizeStep(step, 23);
    if (m_hourStep == step)
    {
        return;
    }
    m_hourStep = step;
    Q_EMIT hourStepChanged(m_hourStep);
}

int AntTimePicker::minuteStep() const { return m_minuteStep; }

void AntTimePicker::setMinuteStep(int step)
{
    step = normalizeStep(step, 59);
    if (m_minuteStep == step)
    {
        return;
    }
    m_minuteStep = step;
    Q_EMIT minuteStepChanged(m_minuteStep);
}

int AntTimePicker::secondStep() const { return m_secondStep; }

void AntTimePicker::setSecondStep(int step)
{
    step = normalizeStep(step, 59);
    if (m_secondStep == step)
    {
        return;
    }
    m_secondStep = step;
    Q_EMIT secondStepChanged(m_secondStep);
}

bool AntTimePicker::showNow() const { return m_showNow; }

void AntTimePicker::setShowNow(bool show)
{
    if (m_showNow == show)
    {
        return;
    }
    m_showNow = show;
    if (m_popup)
    {
        m_popup->update();
    }
    Q_EMIT showNowChanged(m_showNow);
}

QString AntTimePicker::timeString() const
{
    return m_selectedTime.isValid() ? m_selectedTime.toString(m_displayFormat) : QString();
}

QSize AntTimePicker::sizeHint() const
{
    return QSize(160, metrics().height);
}

QSize AntTimePicker::minimumSizeHint() const
{
    return QSize(120, metrics().height);
}

void AntTimePicker::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntTimePicker::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void AntTimePicker::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
    QWidget::leaveEvent(event);
}

void AntTimePicker::mousePressEvent(QMouseEvent* event)
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

void AntTimePicker::keyPressEvent(QKeyEvent* event)
{
    if (!isEnabled())
    {
        QWidget::keyPressEvent(event);
        return;
    }
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        if (m_open)
        {
            acceptPanelTime();
        }
        else
        {
            setOpen(true);
        }
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Escape)
    {
        setOpen(false);
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void AntTimePicker::focusInEvent(QFocusEvent* event)
{
    update();
    QWidget::focusInEvent(event);
}

void AntTimePicker::focusOutEvent(QFocusEvent* event)
{
    if (!m_popup->isVisible())
    {
        setOpen(false);
    }
    update();
    QWidget::focusOutEvent(event);
}

void AntTimePicker::changeEvent(QEvent* event)
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

AntTimePicker::Metrics AntTimePicker::metrics() const
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

QRectF AntTimePicker::controlRect() const
{
    const Metrics m = metrics();
    return QRectF(1, (height() - m.height) / 2.0, width() - 2, m.height);
}

QRectF AntTimePicker::iconRect(const Metrics& metrics) const
{
    const QRectF control = controlRect();
    return QRectF(control.right() - metrics.iconWidth, control.top(), metrics.iconWidth, control.height());
}

QColor AntTimePicker::borderColor() const
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

QColor AntTimePicker::backgroundColor() const
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

bool AntTimePicker::canClear() const
{
    return isEnabled() && m_allowClear && m_hovered && m_selectedTime.isValid();
}

void AntTimePicker::updatePopupGeometry()
{
    if (!m_popup)
    {
        return;
    }
    m_popup->move(mapToGlobal(QPoint(0, height() + 4)));
}

void AntTimePicker::setPanelTime(const QTime& time)
{
    if (!time.isValid())
    {
        return;
    }
    m_panelTime = QTime(time.hour(), time.minute(), time.second());
    if (m_popup)
    {
        m_popup->update();
    }
}

void AntTimePicker::acceptPanelTime()
{
    if (!m_panelTime.isValid())
    {
        m_panelTime = QTime::currentTime();
    }
    if (m_rangeMode)
    {
        if (!m_pickingEnd || !m_startTime.isValid())
        {
            m_startTime = m_panelTime;
            m_pickingEnd = true;
            Q_EMIT startTimeChanged(m_startTime);
        }
        else
        {
            m_endTime = m_panelTime;
            m_pickingEnd = false;
            setSelectedTime(m_startTime);
            setOpen(false);
            Q_EMIT startTimeChanged(m_startTime);
            Q_EMIT endTimeChanged(m_endTime);
            Q_EMIT accepted(m_selectedTime);
        }
    }
    else
    {
        setSelectedTime(m_panelTime);
        setOpen(false);
        Q_EMIT accepted(m_selectedTime);
    }
}

bool AntTimePicker::isRangeMode() const { return m_rangeMode; }
void AntTimePicker::setRangeMode(bool rangeMode)
{
    if (m_rangeMode == rangeMode) return;
    m_rangeMode = rangeMode;
    m_pickingEnd = false;
    Q_EMIT rangeModeChanged(m_rangeMode);
}

QTime AntTimePicker::startTime() const { return m_startTime; }
void AntTimePicker::setStartTime(const QTime& time)
{
    if (m_startTime == time) return;
    m_startTime = time;
    Q_EMIT startTimeChanged(m_startTime);
}

QTime AntTimePicker::endTime() const { return m_endTime; }
void AntTimePicker::setEndTime(const QTime& time)
{
    if (m_endTime == time) return;
    m_endTime = time;
    Q_EMIT endTimeChanged(m_endTime);
}

void AntTimePicker::updateCursor()
{
    setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

int AntTimePicker::normalizeStep(int step, int maximum) const
{
    return std::clamp(step, 1, maximum);
}
