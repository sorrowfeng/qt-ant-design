#include "AntTooltip.h"

#include <QApplication>
#include <QFocusEvent>
#include <QGuiApplication>
#include <QHideEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QTimer>

#include "core/AntPopupMotion.h"
#include "core/AntTheme.h"
#include "styles/AntTooltipStyle.h"

namespace
{
QRect availableScreenGeometryFor(const QWidget* widget)
{
    if (widget)
    {
        if (QScreen* screen = widget->screen())
        {
            return screen->availableGeometry();
        }
    }
    if (QScreen* screen = QGuiApplication::primaryScreen())
    {
        return screen->availableGeometry();
    }
    return QRect(0, 0, 1280, 720);
}

bool isTopPlacement(Ant::TooltipPlacement placement)
{
    return placement == Ant::TooltipPlacement::Top
        || placement == Ant::TooltipPlacement::TopLeft
        || placement == Ant::TooltipPlacement::TopRight;
}

bool isBottomPlacement(Ant::TooltipPlacement placement)
{
    return placement == Ant::TooltipPlacement::Bottom
        || placement == Ant::TooltipPlacement::BottomLeft
        || placement == Ant::TooltipPlacement::BottomRight;
}
}

AntTooltip::AntTooltip(QWidget* parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
{
    installAntStyle<AntTooltipStyle>(this);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setAttribute(Qt::WA_DeleteOnClose, false);

    m_openTimer = new QTimer(this);
    m_openTimer->setSingleShot(true);
    connect(m_openTimer, &QTimer::timeout, this, [this]() {
        if (!m_target || m_title.trimmed().isEmpty())
        {
            return;
        }
        updatePosition();
        AntPopupMotion::show(this, AntPopupMotion::fromTooltipPlacement(m_renderPlacement));
    });
}

AntTooltip::~AntTooltip()
{
    uninstallTarget();
}

QString AntTooltip::title() const
{
    return m_title;
}

void AntTooltip::setTitle(const QString& title)
{
    if (m_title == title)
    {
        return;
    }
    m_title = title;
    adjustSize();
    update();
    Q_EMIT titleChanged(m_title);
}

Ant::TooltipPlacement AntTooltip::placement() const
{
    return m_placement;
}

Ant::TooltipPlacement AntTooltip::renderPlacement() const
{
    return m_renderPlacement;
}

void AntTooltip::setPlacement(Ant::TooltipPlacement placement)
{
    if (m_placement == placement)
    {
        return;
    }
    m_placement = placement;
    m_renderPlacement = placement;
    if (isVisible())
    {
        updatePosition();
    }
    update();
    Q_EMIT placementChanged(m_placement);
}

QColor AntTooltip::color() const
{
    return m_color;
}

void AntTooltip::setColor(const QColor& color)
{
    if (m_color == color)
    {
        return;
    }
    m_color = color;
    update();
    Q_EMIT colorChanged(m_color);
}

bool AntTooltip::arrowVisible() const
{
    return m_arrowVisible;
}

void AntTooltip::setArrowVisible(bool visible)
{
    if (m_arrowVisible == visible)
    {
        return;
    }
    m_arrowVisible = visible;
    adjustSize();
    update();
    Q_EMIT arrowVisibleChanged(m_arrowVisible);
}

int AntTooltip::openDelay() const
{
    return m_openDelay;
}

void AntTooltip::setOpenDelay(int delayMs)
{
    delayMs = qMax(0, delayMs);
    if (m_openDelay == delayMs)
    {
        return;
    }
    m_openDelay = delayMs;
    Q_EMIT openDelayChanged(m_openDelay);
}

QWidget* AntTooltip::target() const
{
    return m_target.data();
}

void AntTooltip::setTarget(QWidget* target)
{
    if (m_target == target)
    {
        return;
    }
    uninstallTarget();
    installTarget(target);
}

void AntTooltip::showTooltip()
{
    if (!m_target || m_title.trimmed().isEmpty())
    {
        return;
    }
    m_openTimer->stop();
    updatePosition();
    AntPopupMotion::show(this, AntPopupMotion::fromTooltipPlacement(m_renderPlacement));
}

void AntTooltip::hideTooltip()
{
    m_openTimer->stop();
    AntPopupMotion::hide(this, AntPopupMotion::fromTooltipPlacement(m_renderPlacement));
}

QSize AntTooltip::sizeHint() const
{
    const Metrics m = metrics();
    const auto& token = antTheme->tokens();
    QFont textFont = font();
    textFont.setPixelSize(token.fontSizeSM);
    QFontMetrics fm(textFont);
    const QRect textRect = fm.boundingRect(QRect(0, 0, m.maxWidth, 400),
                                           Qt::TextWordWrap,
                                           m_title);
    const int arrow = m_arrowVisible ? m.arrowSize : 0;
    if (m_placement == Ant::TooltipPlacement::Left || m_placement == Ant::TooltipPlacement::Right)
    {
        return QSize(textRect.width() + m.paddingX * 2 + arrow, textRect.height() + m.paddingY * 2);
    }
    return QSize(textRect.width() + m.paddingX * 2, textRect.height() + m.paddingY * 2 + arrow);
}

QSize AntTooltip::minimumSizeHint() const
{
    return QSize(48, 28);
}

bool AntTooltip::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_target)
    {
        switch (event->type())
        {
        case QEvent::Enter:
        case QEvent::FocusIn:
            if (!m_title.trimmed().isEmpty())
            {
                m_openTimer->start(m_openDelay);
            }
            break;
        case QEvent::Leave:
        case QEvent::FocusOut:
        case QEvent::Hide:
            hideTooltip();
            break;
        case QEvent::Move:
        case QEvent::Resize:
            if (isVisible())
            {
                updatePosition();
            }
            break;
        case QEvent::MouseMove:
            if (isVisible())
            {
                updatePosition();
            }
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntTooltip::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntTooltip::hideEvent(QHideEvent* event)
{
    m_openTimer->stop();
    QWidget::hideEvent(event);
}

void AntTooltip::mousePressEvent(QMouseEvent* event)
{
    hideTooltip();
    QWidget::mousePressEvent(event);
}

AntTooltip::Metrics AntTooltip::metrics() const
{
    Metrics m;
    return m;
}

QRect AntTooltip::bubbleRect() const
{
    const Metrics m = metrics();
    if (!m_arrowVisible)
    {
        return rect();
    }
    if (isTopPlacement(m_renderPlacement))
    {
        return rect().adjusted(0, 0, 0, -m.arrowSize);
    }
    if (isBottomPlacement(m_renderPlacement))
    {
        return rect().adjusted(0, m.arrowSize, 0, 0);
    }
    if (m_renderPlacement == Ant::TooltipPlacement::Left)
    {
        return rect().adjusted(0, 0, -m.arrowSize, 0);
    }
    return rect().adjusted(m.arrowSize, 0, 0, 0);
}

QPolygonF AntTooltip::arrowPolygon() const
{
    const Metrics m = metrics();
    if (!m_arrowVisible)
    {
        return {};
    }

    const QRect bubble = bubbleRect();
    switch (m_renderPlacement)
    {
    case Ant::TooltipPlacement::Top:
        return QPolygonF({QPointF(width() / 2.0 - m.arrowSize, bubble.bottom()),
                          QPointF(width() / 2.0 + m.arrowSize, bubble.bottom()),
                          QPointF(width() / 2.0, rect().bottom())});
    case Ant::TooltipPlacement::TopLeft:
        return QPolygonF({QPointF(bubble.left() + 18 - m.arrowSize, bubble.bottom()),
                          QPointF(bubble.left() + 18 + m.arrowSize, bubble.bottom()),
                          QPointF(bubble.left() + 18, rect().bottom())});
    case Ant::TooltipPlacement::TopRight:
        return QPolygonF({QPointF(bubble.right() - 18 - m.arrowSize, bubble.bottom()),
                          QPointF(bubble.right() - 18 + m.arrowSize, bubble.bottom()),
                          QPointF(bubble.right() - 18, rect().bottom())});
    case Ant::TooltipPlacement::Bottom:
        return QPolygonF({QPointF(width() / 2.0 - m.arrowSize, bubble.top()),
                          QPointF(width() / 2.0 + m.arrowSize, bubble.top()),
                          QPointF(width() / 2.0, rect().top())});
    case Ant::TooltipPlacement::BottomLeft:
        return QPolygonF({QPointF(bubble.left() + 18 - m.arrowSize, bubble.top()),
                          QPointF(bubble.left() + 18 + m.arrowSize, bubble.top()),
                          QPointF(bubble.left() + 18, rect().top())});
    case Ant::TooltipPlacement::BottomRight:
        return QPolygonF({QPointF(bubble.right() - 18 - m.arrowSize, bubble.top()),
                          QPointF(bubble.right() - 18 + m.arrowSize, bubble.top()),
                          QPointF(bubble.right() - 18, rect().top())});
    case Ant::TooltipPlacement::Left:
        return QPolygonF({QPointF(bubble.right(), height() / 2.0 - m.arrowSize),
                          QPointF(bubble.right(), height() / 2.0 + m.arrowSize),
                          QPointF(rect().right(), height() / 2.0)});
    case Ant::TooltipPlacement::Right:
    default:
        return QPolygonF({QPointF(bubble.left(), height() / 2.0 - m.arrowSize),
                          QPointF(bubble.left(), height() / 2.0 + m.arrowSize),
                          QPointF(rect().left(), height() / 2.0)});
    }
}

QColor AntTooltip::bubbleColor() const
{
    if (m_color.isValid())
    {
        return m_color;
    }
    return antTheme->themeMode() == Ant::ThemeMode::Dark ? QColor("#424242") : QColor("#262626");
}

QColor AntTooltip::textColor() const
{
    if (m_color.isValid())
    {
        return m_color.lightnessF() > 0.6 ? QColor(Qt::black) : QColor(Qt::white);
    }
    return QColor(Qt::white);
}

Ant::TooltipPlacement AntTooltip::resolvedPlacement(const QRect& targetRect, const QRect& screenRect) const
{
    const QSize tipSize = sizeHint();
    const int gap = metrics().gap;

    switch (m_placement)
    {
    case Ant::TooltipPlacement::Top:
    case Ant::TooltipPlacement::TopLeft:
    case Ant::TooltipPlacement::TopRight:
        if (targetRect.top() - gap - tipSize.height() < screenRect.top())
        {
            return m_placement == Ant::TooltipPlacement::TopLeft ? Ant::TooltipPlacement::BottomLeft :
                   m_placement == Ant::TooltipPlacement::TopRight ? Ant::TooltipPlacement::BottomRight :
                                                                    Ant::TooltipPlacement::Bottom;
        }
        break;
    case Ant::TooltipPlacement::Bottom:
    case Ant::TooltipPlacement::BottomLeft:
    case Ant::TooltipPlacement::BottomRight:
        if (targetRect.bottom() + gap + tipSize.height() > screenRect.bottom())
        {
            return m_placement == Ant::TooltipPlacement::BottomLeft ? Ant::TooltipPlacement::TopLeft :
                   m_placement == Ant::TooltipPlacement::BottomRight ? Ant::TooltipPlacement::TopRight :
                                                                       Ant::TooltipPlacement::Top;
        }
        break;
    case Ant::TooltipPlacement::Left:
        if (targetRect.left() - gap - tipSize.width() < screenRect.left())
        {
            return Ant::TooltipPlacement::Right;
        }
        break;
    case Ant::TooltipPlacement::Right:
        if (targetRect.right() + gap + tipSize.width() > screenRect.right())
        {
            return Ant::TooltipPlacement::Left;
        }
        break;
    }
    return m_placement;
}

QPoint AntTooltip::tooltipTopLeft(const QRect& targetRect,
                                  const QSize& tooltipSize,
                                  Ant::TooltipPlacement placement) const
{
    const int gap = metrics().gap;
    switch (placement)
    {
    case Ant::TooltipPlacement::Top:
        return QPoint(targetRect.center().x() - tooltipSize.width() / 2, targetRect.top() - tooltipSize.height() - gap);
    case Ant::TooltipPlacement::TopLeft:
        return QPoint(targetRect.left(), targetRect.top() - tooltipSize.height() - gap);
    case Ant::TooltipPlacement::TopRight:
        return QPoint(targetRect.right() - tooltipSize.width(), targetRect.top() - tooltipSize.height() - gap);
    case Ant::TooltipPlacement::Bottom:
        return QPoint(targetRect.center().x() - tooltipSize.width() / 2, targetRect.bottom() + gap);
    case Ant::TooltipPlacement::BottomLeft:
        return QPoint(targetRect.left(), targetRect.bottom() + gap);
    case Ant::TooltipPlacement::BottomRight:
        return QPoint(targetRect.right() - tooltipSize.width(), targetRect.bottom() + gap);
    case Ant::TooltipPlacement::Left:
        return QPoint(targetRect.left() - tooltipSize.width() - gap, targetRect.center().y() - tooltipSize.height() / 2);
    case Ant::TooltipPlacement::Right:
    default:
        return QPoint(targetRect.right() + gap, targetRect.center().y() - tooltipSize.height() / 2);
    }
}

void AntTooltip::updatePosition()
{
    if (!m_target)
    {
        return;
    }

    adjustSize();
    const QRect targetRect(m_target->mapToGlobal(QPoint(0, 0)), m_target->size());
    const QRect screenRect = availableScreenGeometryFor(m_target);
    const Ant::TooltipPlacement placement = resolvedPlacement(targetRect, screenRect);
    m_renderPlacement = placement;
    QPoint topLeft = tooltipTopLeft(targetRect, sizeHint(), placement);

    topLeft.setX(qBound(screenRect.left() + 4, topLeft.x(), screenRect.right() - width() - 4));
    topLeft.setY(qBound(screenRect.top() + 4, topLeft.y(), screenRect.bottom() - height() - 4));
    move(topLeft);
}

void AntTooltip::installTarget(QWidget* target)
{
    m_target = target;
    if (m_target)
    {
        m_target->installEventFilter(this);
        m_target->setMouseTracking(true);
    }
}

void AntTooltip::uninstallTarget()
{
    if (m_target)
    {
        m_target->removeEventFilter(this);
    }
    m_target = nullptr;
}
