#include "AntPopover.h"

#include <QGuiApplication>
#include <QFontMetrics>
#include <QHideEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QScreen>
#include <QTimer>

#include "core/AntPopupMotion.h"
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

QPolygonF arrowPolygonFor(const QRect& bubble, int arrowSize, Ant::TooltipPlacement placement, bool arrowVisible)
{
    if (!arrowVisible)
    {
        return {};
    }

    constexpr qreal joinOverlap = 1.0;
    switch (placement)
    {
    case Ant::TooltipPlacement::Top:
        return QPolygonF()
            << QPointF(bubble.center().x() - arrowSize, bubble.bottom() - joinOverlap)
            << QPointF(bubble.center().x() + arrowSize, bubble.bottom() - joinOverlap)
            << QPointF(bubble.center().x(), bubble.bottom() + arrowSize);
    case Ant::TooltipPlacement::TopLeft:
        return QPolygonF()
            << QPointF(bubble.left() + 24 - arrowSize, bubble.bottom() - joinOverlap)
            << QPointF(bubble.left() + 24 + arrowSize, bubble.bottom() - joinOverlap)
            << QPointF(bubble.left() + 24, bubble.bottom() + arrowSize);
    case Ant::TooltipPlacement::TopRight:
        return QPolygonF()
            << QPointF(bubble.right() - 24 - arrowSize, bubble.bottom() - joinOverlap)
            << QPointF(bubble.right() - 24 + arrowSize, bubble.bottom() - joinOverlap)
            << QPointF(bubble.right() - 24, bubble.bottom() + arrowSize);
    case Ant::TooltipPlacement::Bottom:
        return QPolygonF()
            << QPointF(bubble.center().x() - arrowSize, bubble.top() + joinOverlap)
            << QPointF(bubble.center().x() + arrowSize, bubble.top() + joinOverlap)
            << QPointF(bubble.center().x(), bubble.top() - arrowSize);
    case Ant::TooltipPlacement::BottomLeft:
        return QPolygonF()
            << QPointF(bubble.left() + 24 - arrowSize, bubble.top() + joinOverlap)
            << QPointF(bubble.left() + 24 + arrowSize, bubble.top() + joinOverlap)
            << QPointF(bubble.left() + 24, bubble.top() - arrowSize);
    case Ant::TooltipPlacement::BottomRight:
        return QPolygonF()
            << QPointF(bubble.right() - 24 - arrowSize, bubble.top() + joinOverlap)
            << QPointF(bubble.right() - 24 + arrowSize, bubble.top() + joinOverlap)
            << QPointF(bubble.right() - 24, bubble.top() - arrowSize);
    case Ant::TooltipPlacement::Left:
        return QPolygonF()
            << QPointF(bubble.right() - joinOverlap, bubble.center().y() - arrowSize)
            << QPointF(bubble.right() - joinOverlap, bubble.center().y() + arrowSize)
            << QPointF(bubble.right() + arrowSize, bubble.center().y());
    case Ant::TooltipPlacement::Right:
    default:
        return QPolygonF()
            << QPointF(bubble.left() + joinOverlap, bubble.center().y() - arrowSize)
            << QPointF(bubble.left() + joinOverlap, bubble.center().y() + arrowSize)
            << QPointF(bubble.left() - arrowSize, bubble.center().y());
    }
}
}

AntPopover::AntPopover(QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
{
    installAntStyle<AntPopoverStyle>(this);
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
            if (!isHoveringInteractiveArea())
            {
                setOpen(false);
            }
        }
    });

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidatePopoverLayout();
        applyPopoverSizeHint();
        requestPopoverUpdate(rect(), QStringLiteral("theme"));
    });
    syncPopoverPerfCounters();
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
    invalidatePopoverLayout();
    applyPopoverSizeHint();
    requestPopoverUpdate(rect(), QStringLiteral("title"));
    Q_EMIT titleChanged(m_title);
}

Ant::IconType AntPopover::titleIconType() const { return m_titleIconType; }

void AntPopover::setTitleIconType(Ant::IconType iconType)
{
    if (m_titleIconType == iconType)
    {
        return;
    }
    m_titleIconType = iconType;
    invalidatePopoverLayout();
    applyPopoverSizeHint();
    requestPopoverUpdate(rect(), QStringLiteral("titleIcon"));
}

QString AntPopover::content() const { return m_content; }

void AntPopover::setContent(const QString& content)
{
    if (m_content == content)
    {
        return;
    }
    m_content = content;
    invalidatePopoverLayout();
    applyPopoverSizeHint();
    requestPopoverUpdate(rect(), QStringLiteral("content"));
    Q_EMIT contentChanged(m_content);
}

Ant::TooltipPlacement AntPopover::placement() const { return m_placement; }

Ant::TooltipPlacement AntPopover::renderPlacement() const { return m_renderPlacement; }

void AntPopover::setPlacement(Ant::TooltipPlacement placement)
{
    if (m_placement == placement)
    {
        return;
    }
    m_placement = placement;
    m_renderPlacement = placement;
    invalidatePopoverLayout();
    invalidatePopoverPlacementCache();
    if (isVisible())
    {
        updatePosition();
    }
    requestPopoverUpdate(rect(), QStringLiteral("placement"));
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
    invalidatePopoverLayout();
    applyPopoverSizeHint();
    requestPopoverUpdate(rect(), QStringLiteral("arrow"));
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
        AntPopupMotion::show(this, AntPopupMotion::fromTooltipPlacement(m_renderPlacement));
    }
    else
    {
        AntPopupMotion::hide(this, AntPopupMotion::fromTooltipPlacement(m_renderPlacement));
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
        m_actionWidget->hide();
        m_actionWidget->deleteLater();
    }
    m_actionWidget = widget;
    if (m_actionWidget)
    {
        m_actionWidget->setParent(this);
        m_actionWidget->show();
    }
    invalidatePopoverLayout();
    applyPopoverSizeHint();
    syncActionGeometry();
    requestPopoverUpdate(rect(), QStringLiteral("action"));
}

QSize AntPopover::sizeHint() const
{
    return popoverLayout().sizeHint;
}

QSize AntPopover::minimumSizeHint() const
{
    return popoverLayout().minimumSizeHint;
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
                if (!m_open && !m_openTimer->isActive())
                {
                    m_openTimer->start(120);
                }
            }
            break;
        case QEvent::Leave:
            if (m_trigger == Ant::PopoverTrigger::Hover)
            {
                m_openTimer->stop();
                if (m_open && !m_closeTimer->isActive())
                {
                    m_closeTimer->start(140);
                }
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
        if (m_open && !m_closeTimer->isActive())
        {
            m_closeTimer->start(140);
        }
    }
    QWidget::leaveEvent(event);
}

AntPopover::Metrics AntPopover::metrics() const
{
    Metrics m;
    return m;
}

const AntPopover::PopoverLayoutCache& AntPopover::popoverLayout() const
{
    const auto& token = antTheme->tokens();
    const QSize actionSize = m_actionWidget ? m_actionWidget->sizeHint() : QSize();
    const bool hasActionWidget = m_actionWidget != nullptr;

    if (m_layoutCache.valid &&
        m_layoutCache.widgetSize == size() &&
        m_layoutCache.title == m_title &&
        m_layoutCache.content == m_content &&
        m_layoutCache.titleIconType == m_titleIconType &&
        m_layoutCache.renderPlacement == m_renderPlacement &&
        m_layoutCache.arrowVisible == m_arrowVisible &&
        m_layoutCache.hasActionWidget == hasActionWidget &&
        m_layoutCache.actionSize == actionSize &&
        m_layoutCache.font == font() &&
        m_layoutCache.themeMode == antTheme->themeMode() &&
        m_layoutCache.tokenFontSize == token.fontSize &&
        qFuzzyCompare(m_layoutCache.tokenLineHeight, token.lineHeight))
    {
        ++m_layoutCacheHitCount;
        syncPopoverPerfCounters();
        return m_layoutCache;
    }

    ++m_layoutBuildCount;
    PopoverLayoutCache cache;
    cache.valid = true;
    cache.widgetSize = size();
    cache.title = m_title;
    cache.content = m_content;
    cache.titleIconType = m_titleIconType;
    cache.renderPlacement = m_renderPlacement;
    cache.arrowVisible = m_arrowVisible;
    cache.hasActionWidget = hasActionWidget;
    cache.actionSize = actionSize;
    cache.font = font();
    cache.themeMode = antTheme->themeMode();
    cache.tokenFontSize = token.fontSize;
    cache.tokenLineHeight = token.lineHeight;
    cache.metrics = metrics();

    QFont titleFont = font();
    titleFont.setPixelSize(token.fontSize);
    titleFont.setWeight(QFont::DemiBold);
    const QFontMetrics titleFm(titleFont);

    QFont bodyFont = font();
    bodyFont.setPixelSize(token.fontSize);
    const QFontMetrics bodyFm(bodyFont);

    const Metrics& m = cache.metrics;
    const int textWidth = m.maxWidth;
    const int titleIconExtra = (!m_title.isEmpty() && m_titleIconType != Ant::IconType::None) ? 26 : 0;
    const QRect titleBounds = m_title.isEmpty()
        ? QRect()
        : titleFm.boundingRect(QRect(0, 0, textWidth - titleIconExtra, 120), Qt::TextWordWrap, m_title);
    const QRect bodyBounds = m_content.isEmpty()
        ? QRect()
        : bodyFm.boundingRect(QRect(0, 0, textWidth, 240), Qt::TextWordWrap, m_content);
    const int actionHeight = hasActionWidget ? qMax(28, actionSize.height()) + 10 : 0;
    const int arrow = m_arrowVisible ? m.arrowSize : 0;
    const int lineHeight = qRound(token.fontSize * token.lineHeight);
    const int titleHeight = titleBounds.isNull() ? 0 : qMax(qMax(titleBounds.height(), lineHeight), titleIconExtra > 0 ? 18 : 0);
    const int bodyHeight = bodyBounds.isNull() ? 0 : qMax(bodyBounds.height(), lineHeight);
    const int bubbleWidth = qMax(m.titleMinWidth, qMax(titleBounds.width() + titleIconExtra, bodyBounds.width())) + m.paddingX * 2;
    const int bubbleHeight = m.paddingY * 2 +
        titleHeight +
        (titleBounds.isNull() || bodyBounds.isNull() ? 0 : m.titleBodyGap) +
        bodyHeight +
        actionHeight;
    const bool sideArrow = m_arrowVisible &&
        (m_renderPlacement == Ant::TooltipPlacement::Left || m_renderPlacement == Ant::TooltipPlacement::Right);
    const bool verticalArrow = m_arrowVisible && !sideArrow;
    const int width = bubbleWidth + m.shadowMargin * 2 + (sideArrow ? arrow : 0);
    const int height = bubbleHeight + m.shadowMargin * 2 + (verticalArrow ? arrow : 0);
    cache.sizeHint = QSize(width, qMax(56, height));
    cache.minimumSizeHint = QSize(180, 64);

    if (!m_arrowVisible)
    {
        cache.bubbleRect = rect().adjusted(m.shadowMargin, m.shadowMargin, -m.shadowMargin, -m.shadowMargin);
    }
    else if (isTopPlacement(m_renderPlacement))
    {
        cache.bubbleRect = rect().adjusted(m.shadowMargin, m.shadowMargin, -m.shadowMargin, -(m.shadowMargin + m.arrowSize));
    }
    else if (isBottomPlacement(m_renderPlacement))
    {
        cache.bubbleRect = rect().adjusted(m.shadowMargin, m.shadowMargin + m.arrowSize, -m.shadowMargin, -m.shadowMargin);
    }
    else if (m_renderPlacement == Ant::TooltipPlacement::Left)
    {
        cache.bubbleRect = rect().adjusted(m.shadowMargin, m.shadowMargin, -(m.shadowMargin + m.arrowSize), -m.shadowMargin);
    }
    else
    {
        cache.bubbleRect = rect().adjusted(m.shadowMargin + m.arrowSize, m.shadowMargin, -m.shadowMargin, -m.shadowMargin);
    }
    cache.arrowPolygon = arrowPolygonFor(cache.bubbleRect, m.arrowSize, m_renderPlacement, m_arrowVisible);

    const QRect inner = cache.bubbleRect.adjusted(m.paddingX, m.paddingY, -m.paddingX, -m.paddingY);
    int headerHeight = 0;
    if (!m_title.isEmpty())
    {
        const int iconExtra = m_titleIconType != Ant::IconType::None ? 26 : 0;
        headerHeight = titleFm.boundingRect(QRect(0, 0, inner.width() - iconExtra, 120), Qt::TextWordWrap, m_title).height();
        headerHeight = qMax(headerHeight, iconExtra > 0 ? 18 : 0);
    }
    cache.headerRect = QRect(inner.left(), inner.top(), inner.width(), headerHeight);

    if (hasActionWidget)
    {
        cache.actionRect = QRect(inner.right() - actionSize.width(),
                                 inner.bottom() - actionSize.height(),
                                 actionSize.width(),
                                 actionSize.height());
    }

    const int bodyTop = cache.headerRect.height() <= 0
        ? inner.top()
        : cache.headerRect.top() + cache.headerRect.height() + m.titleBodyGap;
    const int bodyBottom = hasActionWidget ? cache.actionRect.top() - 10 : inner.bottom();
    cache.bodyRect = QRect(inner.left(), bodyTop, inner.width(), qMax(24, bodyBottom - bodyTop));

    m_layoutCache = cache;
    syncPopoverPerfCounters();
    return m_layoutCache;
}

void AntPopover::invalidatePopoverLayout() const
{
    m_layoutCache.valid = false;
    invalidatePopoverPlacementCache();
}

void AntPopover::invalidatePopoverPlacementCache() const
{
    m_positionCacheValid = false;
}

void AntPopover::applyPopoverSizeHint()
{
    const QSize targetSize = sizeHint();
    if (size() == targetSize)
    {
        ++m_sizeSkipCount;
        syncPopoverPerfCounters();
        return;
    }

    ++m_sizeApplyCount;
    resize(targetSize);
    syncPopoverPerfCounters();
}

void AntPopover::requestPopoverUpdate(const QRect& region, const QString& mode)
{
    m_lastUpdateMode = mode;
    ++m_regionUpdateCount;
    syncPopoverPerfCounters();
    if (region.isValid() && !region.isEmpty())
    {
        update(region);
        return;
    }
    update();
}

void AntPopover::syncPopoverPerfCounters() const
{
    auto* self = const_cast<AntPopover*>(this);
    self->setProperty("antPopoverLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antPopoverLayoutCacheHitCount", m_layoutCacheHitCount);
    self->setProperty("antPopoverPositionResolveCount", m_positionResolveCount);
    self->setProperty("antPopoverPositionResolveSkipCount", m_positionResolveSkipCount);
    self->setProperty("antPopoverSizeApplyCount", m_sizeApplyCount);
    self->setProperty("antPopoverSizeSkipCount", m_sizeSkipCount);
    self->setProperty("antPopoverActionGeometryApplyCount", m_actionGeometryApplyCount);
    self->setProperty("antPopoverActionGeometrySkipCount", m_actionGeometrySkipCount);
    self->setProperty("antPopoverPositionApplyCount", m_positionApplyCount);
    self->setProperty("antPopoverPositionSkipCount", m_positionSkipCount);
    self->setProperty("antPopoverRegionUpdateCount", m_regionUpdateCount);
    self->setProperty("antPopoverLastUpdateMode", m_lastUpdateMode);
}

QRect AntPopover::bubbleRect() const
{
    return popoverLayout().bubbleRect;
}

QPolygonF AntPopover::arrowPolygon() const
{
    return popoverLayout().arrowPolygon;
}

QRect AntPopover::headerRect() const
{
    return popoverLayout().headerRect;
}

QRect AntPopover::bodyRect() const
{
    return popoverLayout().bodyRect;
}

QRect AntPopover::actionRect() const
{
    return popoverLayout().actionRect;
}

void AntPopover::updatePosition()
{
    if (!m_target)
    {
        return;
    }
    applyPopoverSizeHint();
    const QRect targetRect(m_target->mapToGlobal(QPoint(0, 0)), m_target->size());
    const QRect screenRect = availableScreenGeometryFor(m_target);
    QSize popupSize = sizeHint();
    Ant::TooltipPlacement placement = m_cachedResolvedPlacement;
    if (m_positionCacheValid &&
        m_cachedTargetRect == targetRect &&
        m_cachedScreenRect == screenRect &&
        m_cachedPopupSize == popupSize &&
        m_cachedPlacementRequest == m_placement)
    {
        ++m_positionResolveSkipCount;
    }
    else
    {
        placement = resolvedPlacement(targetRect, screenRect, popupSize);
        m_positionCacheValid = true;
        m_cachedTargetRect = targetRect;
        m_cachedScreenRect = screenRect;
        m_cachedPopupSize = popupSize;
        m_cachedPlacementRequest = m_placement;
        m_cachedResolvedPlacement = placement;
        ++m_positionResolveCount;
    }

    if (m_renderPlacement != placement)
    {
        m_renderPlacement = placement;
        m_layoutCache.valid = false;
        applyPopoverSizeHint();
        popupSize = sizeHint();
    }

    QPoint topLeft = popupTopLeft(targetRect, popupSize, placement);
    topLeft.setX(qBound(screenRect.left() + 4, topLeft.x(), screenRect.right() - popupSize.width() - 4));
    topLeft.setY(qBound(screenRect.top() + 4, topLeft.y(), screenRect.bottom() - popupSize.height() - 4));
    if (pos() == topLeft)
    {
        ++m_positionSkipCount;
        syncPopoverPerfCounters();
        return;
    }

    ++m_positionApplyCount;
    move(topLeft);
    syncPopoverPerfCounters();
}

void AntPopover::syncActionGeometry()
{
    if (m_actionWidget)
    {
        const QRect targetGeometry = actionRect();
        if (m_actionWidget->geometry() == targetGeometry && m_actionWidget->isVisible())
        {
            ++m_actionGeometrySkipCount;
            syncPopoverPerfCounters();
            return;
        }

        ++m_actionGeometryApplyCount;
        m_actionWidget->setGeometry(targetGeometry);
        m_actionWidget->show();
        syncPopoverPerfCounters();
    }
}

void AntPopover::installTarget(QWidget* target)
{
    m_target = target;
    invalidatePopoverPlacementCache();
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
    invalidatePopoverPlacementCache();
}

bool AntPopover::isHoveringInteractiveArea() const
{
    const QPoint globalPos = QCursor::pos();
    if (m_target)
    {
        const QRect targetRect(m_target->mapToGlobal(QPoint(0, 0)), m_target->size());
        if (targetRect.contains(globalPos))
        {
            return true;
        }
    }
    return isVisible() && geometry().contains(globalPos);
}

Ant::TooltipPlacement AntPopover::resolvedPlacement(const QRect& targetRect, const QRect& screenRect, const QSize& popupSize) const
{
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
    if (m_placement == Ant::TooltipPlacement::Left &&
        targetRect.left() - popupSize.width() - metrics().gap < screenRect.left())
    {
        return Ant::TooltipPlacement::Right;
    }
    if (m_placement == Ant::TooltipPlacement::Right &&
        targetRect.right() + popupSize.width() + metrics().gap > screenRect.right())
    {
        return Ant::TooltipPlacement::Left;
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
