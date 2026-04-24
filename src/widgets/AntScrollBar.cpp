#include "AntScrollBar.h"

#include <QMouseEvent>
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
    update();
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
    m_hovered = true;
    update();
    QScrollBar::enterEvent(event);
}

void AntScrollBar::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
    QScrollBar::leaveEvent(event);
}

void AntScrollBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_pressed = true;
        update();
    }
    QScrollBar::mousePressEvent(event);
}

void AntScrollBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_pressed = false;
        update();
    }
    QScrollBar::mouseReleaseEvent(event);
}

void AntScrollBar::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        update();
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
        style()->unpolish(this);
        style()->polish(this);
        updateGeometry();
        update();
    });
}
