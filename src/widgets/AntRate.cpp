#include "AntRate.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QStyle>
#include <QToolTip>

#include <algorithm>
#include <cmath>

#include "../styles/AntRateStyle.h"
#include "core/AntTheme.h"

AntRate::AntRate(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);

    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        style()->unpolish(this);
        style()->polish(this);
        updateGeometry();
        update();
    });

    auto* rateStyle = new AntRateStyle(style());
    rateStyle->setParent(this);
    setStyle(rateStyle);
}

double AntRate::value() const
{
    return m_value;
}

void AntRate::setValue(double value)
{
    double newValue = std::clamp(value, 0.0, static_cast<double>(m_count));
    if (m_allowHalf)
    {
        newValue = std::round(newValue * 2.0) / 2.0;
    }
    else
    {
        newValue = std::round(newValue);
    }

    if (qFuzzyCompare(m_value, newValue))
    {
        return;
    }

    m_value = newValue;
    update();
    Q_EMIT valueChanged(m_value);
}

int AntRate::count() const
{
    return m_count;
}

void AntRate::setCount(int count)
{
    if (count < 1 || m_count == count)
    {
        return;
    }

    m_count = count;
    setValue(m_value);
    updateGeometry();
    update();
    Q_EMIT countChanged(m_count);
}

bool AntRate::allowHalf() const
{
    return m_allowHalf;
}

void AntRate::setAllowHalf(bool allow)
{
    if (m_allowHalf == allow)
    {
        return;
    }

    m_allowHalf = allow;
    setValue(m_value);
    update();
    Q_EMIT allowHalfChanged(m_allowHalf);
}

bool AntRate::allowClear() const
{
    return m_allowClear;
}

void AntRate::setAllowClear(bool allow)
{
    if (m_allowClear == allow)
    {
        return;
    }

    m_allowClear = allow;
    Q_EMIT allowClearChanged(m_allowClear);
}

bool AntRate::isDisabled() const
{
    return m_disabled;
}

void AntRate::setDisabled(bool disabled)
{
    if (m_disabled == disabled)
    {
        return;
    }

    m_disabled = disabled;
    setCursor(m_disabled ? Qt::ArrowCursor : Qt::PointingHandCursor);
    update();
    Q_EMIT disabledChanged(m_disabled);
}

Ant::Size AntRate::rateSize() const
{
    return m_rateSize;
}

void AntRate::setRateSize(Ant::Size size)
{
    if (m_rateSize == size)
    {
        return;
    }

    m_rateSize = size;
    updateGeometry();
    update();
    Q_EMIT rateSizeChanged(m_rateSize);
}

double AntRate::hoverValue() const
{
    return m_hoverValue;
}

bool AntRate::isHoveredState() const
{
    return m_hovered;
}

QSize AntRate::sizeHint() const
{
    const auto& token = antTheme->tokens();
    int starSize = static_cast<int>(std::round(token.controlHeight * 0.625));
    switch (m_rateSize)
    {
        case Ant::Size::Small:
            starSize = static_cast<int>(std::round(token.controlHeightSM * 0.625));
            break;
        case Ant::Size::Large:
            starSize = static_cast<int>(std::round(token.controlHeightLG * 0.625));
            break;
        default:
            break;
    }
    const int totalWidth = m_count * starSize + (m_count - 1) * token.marginXS;
    return QSize(totalWidth, starSize + 4);
}

QSize AntRate::minimumSizeHint() const
{
    return sizeHint();
}

void AntRate::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void AntRate::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_hoverValue = -1.0;
    Q_EMIT hoverChanged(m_hoverValue);
    update();
    QWidget::leaveEvent(event);
}

void AntRate::mouseMoveEvent(QMouseEvent* event)
{
    if (m_disabled || !isEnabled())
    {
        QWidget::mouseMoveEvent(event);
        return;
    }

    updateHoverValue(event->pos());
    QWidget::mouseMoveEvent(event);
}

void AntRate::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && !m_disabled && isEnabled())
    {
        m_pressed = true;
        updateHoverValue(event->pos());
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntRate::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed)
    {
        m_pressed = false;
        if (rect().contains(event->pos()))
        {
            updateHoverValue(event->pos());
            const double clickedValue = m_hoverValue >= 0.0 ? m_hoverValue : m_value;
            if (m_allowClear && qFuzzyCompare(clickedValue, m_value) && m_value > 0.0)
            {
                setValue(0.0);
            }
            else
            {
                setValue(clickedValue);
            }
        }
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void AntRate::keyPressEvent(QKeyEvent* event)
{
    if (m_disabled || !isEnabled())
    {
        QWidget::keyPressEvent(event);
        return;
    }

    const double step = m_allowHalf ? 0.5 : 1.0;
    switch (event->key())
    {
        case Qt::Key_Left:
            setValue(m_value - step);
            event->accept();
            return;
        case Qt::Key_Right:
            setValue(m_value + step);
            event->accept();
            return;
        case Qt::Key_Home:
            setValue(0.0);
            event->accept();
            return;
        case Qt::Key_End:
            setValue(static_cast<double>(m_count));
            event->accept();
            return;
        default:
            break;
    }
    QWidget::keyPressEvent(event);
}

void AntRate::focusInEvent(QFocusEvent* event)
{
    m_focused = true;
    update();
    QWidget::focusInEvent(event);
}

void AntRate::focusOutEvent(QFocusEvent* event)
{
    m_focused = false;
    update();
    QWidget::focusOutEvent(event);
}

void AntRate::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        setCursor(isEnabled() && !m_disabled ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
    QWidget::changeEvent(event);
}

double AntRate::starValueAt(const QPoint& pos) const
{
    const auto& token = antTheme->tokens();
    int starSize = static_cast<int>(std::round(token.controlHeight * 0.625));
    switch (m_rateSize)
    {
        case Ant::Size::Small:
            starSize = static_cast<int>(std::round(token.controlHeightSM * 0.625));
            break;
        case Ant::Size::Large:
            starSize = static_cast<int>(std::round(token.controlHeightLG * 0.625));
            break;
        default:
            break;
    }
    const int margin = token.marginXS;
    const int totalWidth = m_count * starSize + (m_count - 1) * margin;
    const int startX = (width() - totalWidth) / 2;
    const int startY = (height() - starSize) / 2;

    if (pos.y() < startY - 4 || pos.y() > startY + starSize + 4)
    {
        return -1.0;
    }

    const int relativeX = pos.x() - startX;
    if (relativeX < 0)
    {
        return 0.0;
    }

    const int starWithMargin = starSize + margin;
    int starIndex = relativeX / starWithMargin;
    if (starIndex >= m_count)
    {
        starIndex = m_count - 1;
    }

    const int withinStar = relativeX - starIndex * starWithMargin;
    if (withinStar > starSize)
    {
        return static_cast<double>(starIndex + 1);
    }

    double value = static_cast<double>(starIndex);
    if (m_allowHalf)
    {
        value += (withinStar >= starSize / 2) ? 1.0 : 0.5;
    }
    else
    {
        value += 1.0;
    }

    return std::clamp(value, 0.0, static_cast<double>(m_count));
}

void AntRate::updateHoverValue(const QPoint& pos)
{
    const double newHover = starValueAt(pos);
    if (!qFuzzyCompare(m_hoverValue, newHover))
    {
        m_hoverValue = newHover;
        Q_EMIT hoverChanged(m_hoverValue);
        update();
    }
}
