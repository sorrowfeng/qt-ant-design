#include "AntScrollBar.h"

#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionSlider>

#include "../styles/AntScrollBarStyle.h"
#include "core/AntTheme.h"

AntScrollBar::AntScrollBar(QWidget* parent)
    : QScrollBar(parent)
{
    initStyle();
}

AntScrollBar::AntScrollBar(Qt::Orientation orientation, QWidget* parent)
    : QScrollBar(parent)
{
    setOrientation(orientation);
    initStyle();
}

bool AntScrollBar::autoHide() const
{
    return m_autoHide;
}

void AntScrollBar::setAutoHide(bool autoHide)
{
    if (m_autoHide == autoHide)
    {
        return;
    }

    m_autoHide = autoHide;
    updateSliderRegion();
    Q_EMIT autoHideChanged(m_autoHide);
}

bool AntScrollBar::isHoveredState() const
{
    return m_hovered;
}

bool AntScrollBar::isPressedState() const
{
    return m_pressed;
}

QSize AntScrollBar::sizeHint() const
{
    if (orientation() == Qt::Vertical)
    {
        return QSize(8, 120);
    }
    return QSize(120, 8);
}

QSize AntScrollBar::minimumSizeHint() const
{
    if (orientation() == Qt::Vertical)
    {
        return QSize(8, 32);
    }
    return QSize(32, 8);
}

void AntScrollBar::enterEvent(QEnterEvent* event)
{
    if (!m_hovered)
    {
        m_hovered = true;
        updateSliderRegion();
    }
    QScrollBar::enterEvent(event);
}

void AntScrollBar::leaveEvent(QEvent* event)
{
    if (m_hovered)
    {
        m_hovered = false;
        updateSliderRegion();
    }
    QScrollBar::leaveEvent(event);
}

void AntScrollBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (!m_pressed)
        {
            m_pressed = true;
            updateSliderRegion();
        }
    }
    QScrollBar::mousePressEvent(event);
}

void AntScrollBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (m_pressed)
        {
            m_pressed = false;
            updateSliderRegion();
        }
    }
    QScrollBar::mouseReleaseEvent(event);
}

void AntScrollBar::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        updateSliderRegion();
    }
    QScrollBar::changeEvent(event);
}

void AntScrollBar::initStyle()
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);

    auto* scrollBarStyle = new AntScrollBarStyle(style());
    scrollBarStyle->setParent(this);
    setStyle(scrollBarStyle);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        ++m_themeRefreshCount;
        updateSliderRegion();
    });
    syncScrollBarPerfCounters();
}

QRect AntScrollBar::sliderHandleRect() const
{
    QStyleOptionSlider option;
    initStyleOption(&option);
    return style()->subControlRect(QStyle::CC_ScrollBar, &option, QStyle::SC_ScrollBarSlider, this);
}

void AntScrollBar::updateSliderRegion(const QRect& previousHandle)
{
    QRect dirty = sliderHandleRect();
    if (previousHandle.isValid())
    {
        dirty = dirty.united(previousHandle);
    }
    if (!dirty.isValid())
    {
        dirty = rect();
    }

    update(dirty.adjusted(-2, -2, 2, 2).intersected(rect()));
    ++m_sliderRegionUpdateCount;
    syncScrollBarPerfCounters();
}

void AntScrollBar::syncScrollBarPerfCounters() const
{
    auto* self = const_cast<AntScrollBar*>(this);
    self->setProperty("antScrollBarSliderRegionUpdateCount", m_sliderRegionUpdateCount);
    self->setProperty("antScrollBarThemeRefreshCount", m_themeRefreshCount);
}
