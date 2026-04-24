#include "AntPopover.h"

#include <QGuiApplication>
#include <QHideEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QTimer>

#include "core/AntTheme.h"
#include "styles/AntPopoverStyle.h"
#include "styles/AntPalette.h"

namespace
{
QRect availableScreenGeometryFor(const QWidget* widget)
{
    if (widget && widget->screen())
    {
        return widget->screen()->availableGeometry();
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

AntPopover::AntPopover(QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
{
    setStyle(new AntPopoverStyle(style()));
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);

    m_openTimer = new QTimer(this);
    m_openTimer->setSingleShot(true);
    connect(m_openTimer, &QTimer::timeout, this, [this]() { setOpen(true); });

    m_closeTimer = new QTimer(this);
    m_closeTimer->setSingleShot(true);
    connect(m_closeTimer, &QTimer::timeout, this, [this]() {
        if (m_trigger == Ant::PopoverTrigger::Hover)
        {
            setOpen(false);
        }
    });
}

AntPopover::~AntPopover()
{
    uninstallTarget();
}

QString AntPopover::title() const { return m_title; }

void AntPopover::setTitle(const QString& title)
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

QString AntPopover::content() const { return m_content; }

void AntPopover::setContent(const QString& content)
{
    if (m_content == content)
    {
        return;
    }
    m_content = content;
    adjustSize();
    update();
    Q_EMIT contentChanged(m_content);
}

Ant::TooltipPlacement AntPopover::placement() const { return m_placement; }

void AntPopover::setPlacement(Ant::TooltipPlacement placement)
{
    if (m_placement == placement)
    {
        return;
    }
    m_placement = placement;
    if (isVisible())
    {
        updatePosition();
    }
    update();
    Q_EMIT placementChanged(m_placement);
}

Ant::PopoverTrigger AntPopover::trigger() const { return m_trigger; }

void AntPopover::setTrigger(Ant::PopoverTrigger trigger)
{
    if (m_trigger == trigger)
    {
        return;
    }
    m_trigger = trigger;
    Q_EMIT triggerChanged(m_trigger);
}

bool AntPopover::arrowVisible() const { return m_arrowVisible; }

void AntPopover::setArrowVisible(bool visible)
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

bool AntPopover::isOpen() const { return m_open; }

void AntPopover::setOpen(bool open)
{
    if (m_open == open)
    {
        return;
    }
    m_open = open;
    m_openTimer->stop();
    m_closeTimer->stop();

    if (m_open && m_target)
    {
        updatePosition();
        show();
        raise();
    }
    else
    {
        hide();
    }
    Q_EMIT openChanged(m_open);
}

QWidget* AntPopover::target() const { return m_target.data(); }

void AntPopover::setTarget(QWidget* target)
{
    if (m_target == target)
    {
        return;
    }
    uninstallTarget();
    installTarget(target);
}

QWidget* AntPopover::actionWidget() const { return m_actionWidget.data(); }

void AntPopover::setActionWidget(QWidget* widget)
{
    if (m_actionWidget == widget)
    {
        return;
    }
    if (m_actionWidget && m_actionWidget->parent() == this)
    {
        m_actionWidget->deleteLater();
    }
    m_actionWidget = widget;
    if (m_actionWidget)
    {
        m_actionWidget->setParent(this);
        m_actionWidget->show();
    }
    syncActionGeometry();
    adjustSize();
    update();
}

QSize AntPopover::sizeHint() const
{
    const auto& token = antTheme->tokens();
    const Metrics m = metrics();
    QFont titleFont = font();
    titleFont.setPixelSize(token.fontSize);
    titleFont.setWeight(QFont::DemiBold);
    QFontMetrics titleFm(titleFont);

    QFont bodyFont = font();
    bodyFont.setPixelSize(token.fontSizeSM);
    QFontMetrics bodyFm(bodyFont);

    const int textWidth = m.maxWidth;
    const QRect titleBounds = m_title.isEmpty() ? QRect() : titleFm.boundingRect(QRect(0, 0, textWidth, 120), Qt::TextWordWrap, m_title);
    const QRect bodyBounds = m_content.isEmpty() ? QRect() : bodyFm.boundingRect(QRect(0, 0, textWidth, 240), Qt::TextWordWrap, m_content);
    const int actionHeight = m_actionWidget ? qMax(28, m_actionWidget->sizeHint().height()) + 10 : 0;
    const int arrow = m_arrowVisible ? m.arrowSize : 0;
    const int width = qMax(160, qMax(titleBounds.width(), bodyBounds.width()) + m.paddingX * 2);
    const int height = m.paddingY * 2 + titleBounds.height() + (titleBounds.isNull() || bodyBounds.isNull() ? 0 : 8) + bodyBounds.height() + actionHeight + arrow;
    return QSize(width, qMax(56, height));
}

QSize AntPopover::minimumSizeHint() const
{
    return QSize(180, 64);
}

bool AntPopover::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_target)
    {
        switch (event->type())
        {
        case QEvent::Enter:
            if (m_trigger == Ant::PopoverTrigger::Hover)
            {
                m_closeTimer->stop();
                m_openTimer->start(120);
            }
            break;
        case QEvent::Leave:
            if (m_trigger == Ant::PopoverTrigger::Hover)
            {
                m_openTimer->stop();
                m_closeTimer->start(140);
            }
            break;
        case QEvent::MouseButtonPress:
            if (m_trigger == Ant::PopoverTrigger::Click)
            {
                setOpen(!m_open);
                return true;
            }
            break;
        case QEvent::Move:
        case QEvent::Resize:
            if (m_open)
            {
                updatePosition();
            }
            break;
        case QEvent::Hide:
            setOpen(false);
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntPopover::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntPopover::resizeEvent(QResizeEvent* event)
{
    syncActionGeometry();
    QWidget::resizeEvent(event);
}

void AntPopover::hideEvent(QHideEvent* event)
{
    m_openTimer->stop();
    m_closeTimer->stop();
    m_open = false;
    QWidget::hideEvent(event);
}

void AntPopover::enterEvent(QEnterEvent* event)
{
    if (m_trigger == Ant::PopoverTrigger::Hover)
    {
        m_closeTimer->stop();
    }
    QWidget::enterEvent(event);
}

void AntPopover::leaveEvent(QEvent* event)
{
    if (m_trigger == Ant::PopoverTrigger::Hover)
    {
        m_closeTimer->start(140);
    }
    QWidget::leaveEvent(event);
}

AntPopover::Metrics AntPopover::metrics() const
{
    Metrics m;
    return m;
}

QRect AntPopover::bubbleRect() const
{
    const Metrics m = metrics();
    if (!m_arrowVisible)
    {
        return rect().adjusted(8, 8, -8, -8);
    }
    if (isTopPlacement(m_placement))
    {
        return rect().adjusted(8, 8, -8, -(8 + m.arrowSize));
    }
    if (isBottomPlacement(m_placement))
    {
        return rect().adjusted(8, 8 + m.arrowSize, -8, -8);
    }
    if (m_placement == Ant::TooltipPlacement::Left)
    {
        return rect().adjusted(8, 8, -(8 + m.arrowSize), -8);
    }
    return rect().adjusted(8 + m.arrowSize, 8, -8, -8);
}

QPolygonF AntPopover::arrowPolygon() const
{
    const Metrics m = metrics();
    const QRect bubble = bubbleRect();
    if (!m_arrowVisible)
    {
        return {};
    }
    switch (m_placement)
    {
    case Ant::TooltipPlacement::Top:
        return QPolygonF()
            << QPointF(bubble.center().x() - m.arrowSize, bubble.bottom())
            << QPointF(bubble.center().x() + m.arrowSize, bubble.bottom())
            << QPointF(bubble.center().x(), bubble.bottom() + m.arrowSize);
    case Ant::TooltipPlacement::TopLeft:
        return QPolygonF()
            << QPointF(bubble.left() + 24 - m.arrowSize, bubble.bottom())
            << QPointF(bubble.left() + 24 + m.arrowSize, bubble.bottom())
            << QPointF(bubble.left() + 24, bubble.bottom() + m.arrowSize);
    case Ant::TooltipPlacement::TopRight:
        return QPolygonF()
            << QPointF(bubble.right() - 24 - m.arrowSize, bubble.bottom())
            << QPointF(bubble.right() - 24 + m.arrowSize, bubble.bottom())
            << QPointF(bubble.right() - 24, bubble.bottom() + m.arrowSize);
    case Ant::TooltipPlacement::Bottom:
        return QPolygonF()
            << QPointF(bubble.center().x() - m.arrowSize, bubble.top())
            << QPointF(bubble.center().x() + m.arrowSize, bubble.top())
            << QPointF(bubble.center().x(), bubble.top() - m.arrowSize);
    case Ant::TooltipPlacement::BottomLeft:
        return QPolygonF()
            << QPointF(bubble.left() + 24 - m.arrowSize, bubble.top())
            << QPointF(bubble.left() + 24 + m.arrowSize, bubble.top())
            << QPointF(bubble.left() + 24, bubble.top() - m.arrowSize);
    case Ant::TooltipPlacement::BottomRight:
        return QPolygonF()
            << QPointF(bubble.right() - 24 - m.arrowSize, bubble.top())
            << QPointF(bubble.right() - 24 + m.arrowSize, bubble.top())
            << QPointF(bubble.right() - 24, bubble.top() - m.arrowSize);
    case Ant::TooltipPlacement::Left:
        return QPolygonF()
            << QPointF(bubble.right(), bubble.center().y() - m.arrowSize)
            << QPointF(bubble.right(), bubble.center().y() + m.arrowSize)
            << QPointF(bubble.right() + m.arrowSize, bubble.center().y());
    case Ant::TooltipPlacement::Right:
    default:
        return QPolygonF()
            << QPointF(bubble.left(), bubble.center().y() - m.arrowSize)
            << QPointF(bubble.left(), bubble.center().y() + m.arrowSize)
            << QPointF(bubble.left() - m.arrowSize, bubble.center().y());
    }
}

QRect AntPopover::headerRect() const
{
    const Metrics m = metrics();
    QRect bubble = bubbleRect().adjusted(m.paddingX, m.paddingY, -m.paddingX, -m.paddingY);
    int headerHeight = 0;
    if (!m_title.isEmpty())
    {
        QFont fontMetricsFont = font();
        fontMetricsFont.setPixelSize(antTheme->tokens().fontSize);
        fontMetricsFont.setWeight(QFont::DemiBold);
        headerHeight = QFontMetrics(fontMetricsFont).boundingRect(QRect(0, 0, bubble.width(), 120), Qt::TextWordWrap, m_title).height();
    }
    return QRect(bubble.left(), bubble.top(), bubble.width(), headerHeight);
}

QRect AntPopover::bodyRect() const
{
    const Metrics m = metrics();
    QRect bubble = bubbleRect().adjusted(m.paddingX, m.paddingY, -m.paddingX, -m.paddingY);
    QRect header = headerRect();
    int top = header.isNull() ? bubble.top() : header.bottom() + 8;
    int bottom = m_actionWidget ? actionRect().top() - 10 : bubble.bottom();
    return QRect(bubble.left(), top, bubble.width(), qMax(24, bottom - top));
}

QRect AntPopover::actionRect() const
{
    if (!m_actionWidget)
    {
        return {};
    }
    const Metrics m = metrics();
    QRect bubble = bubbleRect().adjusted(m.paddingX, m.paddingY, -m.paddingX, -m.paddingY);
    QSize size = m_actionWidget->sizeHint();
    return QRect(bubble.right() - size.width(),
                 bubble.bottom() - size.height(),
                 size.width(),
                 size.height());
}

void AntPopover::updatePosition()
{
    if (!m_target)
    {
        return;
    }
    adjustSize();
    const QRect targetRect(m_target->mapToGlobal(QPoint(0, 0)), m_target->size());
    const QRect screenRect = availableScreenGeometryFor(m_target);
    const Ant::TooltipPlacement placement = resolvedPlacement(targetRect, screenRect);
    QPoint topLeft = popupTopLeft(targetRect, sizeHint(), placement);
    topLeft.setX(qBound(screenRect.left() + 4, topLeft.x(), screenRect.right() - width() - 4));
    topLeft.setY(qBound(screenRect.top() + 4, topLeft.y(), screenRect.bottom() - height() - 4));
    move(topLeft);
}

void AntPopover::syncActionGeometry()
{
    if (m_actionWidget)
    {
        m_actionWidget->setGeometry(actionRect());
        m_actionWidget->show();
    }
}

void AntPopover::installTarget(QWidget* target)
{
    m_target = target;
    if (m_target)
    {
        m_target->installEventFilter(this);
        m_target->setMouseTracking(true);
    }
}

void AntPopover::uninstallTarget()
{
    if (m_target)
    {
        m_target->removeEventFilter(this);
    }
    m_target = nullptr;
}

Ant::TooltipPlacement AntPopover::resolvedPlacement(const QRect& targetRect, const QRect& screenRect) const
{
    const QSize popupSize = sizeHint();
    if (isTopPlacement(m_placement) && targetRect.top() - popupSize.height() - metrics().gap < screenRect.top())
    {
        return m_placement == Ant::TooltipPlacement::TopLeft ? Ant::TooltipPlacement::BottomLeft :
               m_placement == Ant::TooltipPlacement::TopRight ? Ant::TooltipPlacement::BottomRight :
                                                                Ant::TooltipPlacement::Bottom;
    }
    if (isBottomPlacement(m_placement) && targetRect.bottom() + popupSize.height() + metrics().gap > screenRect.bottom())
    {
        return m_placement == Ant::TooltipPlacement::BottomLeft ? Ant::TooltipPlacement::TopLeft :
               m_placement == Ant::TooltipPlacement::BottomRight ? Ant::TooltipPlacement::TopRight :
                                                                   Ant::TooltipPlacement::Top;
    }
    return m_placement;
}

QPoint AntPopover::popupTopLeft(const QRect& targetRect, const QSize& popupSize, Ant::TooltipPlacement placement) const
{
    const int gap = metrics().gap;
    switch (placement)
    {
    case Ant::TooltipPlacement::Top:
        return {targetRect.center().x() - popupSize.width() / 2, targetRect.top() - popupSize.height() - gap};
    case Ant::TooltipPlacement::TopLeft:
        return {targetRect.left(), targetRect.top() - popupSize.height() - gap};
    case Ant::TooltipPlacement::TopRight:
        return {targetRect.right() - popupSize.width(), targetRect.top() - popupSize.height() - gap};
    case Ant::TooltipPlacement::Bottom:
        return {targetRect.center().x() - popupSize.width() / 2, targetRect.bottom() + gap};
    case Ant::TooltipPlacement::BottomLeft:
        return {targetRect.left(), targetRect.bottom() + gap};
    case Ant::TooltipPlacement::BottomRight:
        return {targetRect.right() - popupSize.width(), targetRect.bottom() + gap};
    case Ant::TooltipPlacement::Left:
        return {targetRect.left() - popupSize.width() - gap, targetRect.center().y() - popupSize.height() / 2};
    case Ant::TooltipPlacement::Right:
    default:
        return {targetRect.right() + gap, targetRect.center().y() - popupSize.height() / 2};
    }
}
