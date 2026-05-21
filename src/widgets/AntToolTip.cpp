#include "AntToolTip.h"

#include <QApplication>
#include <QFocusEvent>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QHideEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QTimer>

#include "core/AntPopupMotion.h"
#include "core/AntTheme.h"
#include "styles/AntToolTipStyle.h"

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

AntToolTip::AntToolTip(QWidget* parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
{
    installAntStyle<AntToolTipStyle>(this);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

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

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateToolTipLayout();
        invalidateToolTipPosition();
        adjustSize();
        requestToolTipUpdate(rect(), QStringLiteral("theme"));
    });

    syncToolTipPerfCounters();
}

AntToolTip::~AntToolTip()
{
    uninstallTarget();
}

QString AntToolTip::title() const
{
    return m_title;
}

void AntToolTip::setTitle(const QString& title)
{
    if (m_title == title)
    {
        return;
    }
    m_title = title;
    invalidateToolTipLayout();
    invalidateToolTipPosition();
    adjustSize();
    requestToolTipUpdate(rect(), QStringLiteral("title"));
    Q_EMIT titleChanged(m_title);
}

Ant::TooltipPlacement AntToolTip::placement() const
{
    return m_placement;
}

Ant::TooltipPlacement AntToolTip::renderPlacement() const
{
    return m_renderPlacement;
}

void AntToolTip::setPlacement(Ant::TooltipPlacement placement)
{
    if (m_placement == placement)
    {
        return;
    }
    m_placement = placement;
    m_renderPlacement = placement;
    invalidateToolTipLayout();
    invalidateToolTipPosition();
    if (isVisible())
    {
        updatePosition();
    }
    requestToolTipUpdate(rect(), QStringLiteral("placement"));
    Q_EMIT placementChanged(m_placement);
}

QColor AntToolTip::color() const
{
    return m_color;
}

void AntToolTip::setColor(const QColor& color)
{
    if (m_color == color)
    {
        return;
    }
    m_color = color;
    invalidateToolTipLayout();
    requestToolTipUpdate(bubbleRect(), QStringLiteral("color"));
    Q_EMIT colorChanged(m_color);
}

bool AntToolTip::arrowVisible() const
{
    return m_arrowVisible;
}

void AntToolTip::setArrowVisible(bool visible)
{
    if (m_arrowVisible == visible)
    {
        return;
    }
    m_arrowVisible = visible;
    invalidateToolTipLayout();
    invalidateToolTipPosition();
    adjustSize();
    requestToolTipUpdate(rect(), QStringLiteral("arrow"));
    Q_EMIT arrowVisibleChanged(m_arrowVisible);
}

int AntToolTip::openDelay() const
{
    return m_openDelay;
}

void AntToolTip::setOpenDelay(int delayMs)
{
    delayMs = qMax(0, delayMs);
    if (m_openDelay == delayMs)
    {
        return;
    }
    m_openDelay = delayMs;
    Q_EMIT openDelayChanged(m_openDelay);
}

QWidget* AntToolTip::target() const
{
    return m_target.data();
}

void AntToolTip::setTarget(QWidget* target)
{
    if (m_target == target)
    {
        return;
    }
    uninstallTarget();
    installTarget(target);
}

void AntToolTip::showTooltip()
{
    if (!m_target || m_title.trimmed().isEmpty())
    {
        return;
    }
    m_openTimer->stop();
    syncToolTipPerfCounters();
    updatePosition();
    AntPopupMotion::show(this, AntPopupMotion::fromTooltipPlacement(m_renderPlacement));
}

void AntToolTip::hideTooltip()
{
    m_openTimer->stop();
    syncToolTipPerfCounters();
    AntPopupMotion::hide(this, AntPopupMotion::fromTooltipPlacement(m_renderPlacement));
}

QSize AntToolTip::sizeHint() const
{
    return tooltipLayout().sizeHint;
}

QSize AntToolTip::minimumSizeHint() const
{
    return tooltipLayout().minimumSizeHint;
}

bool AntToolTip::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_target)
    {
        switch (event->type())
        {
        case QEvent::Enter:
        case QEvent::FocusIn:
            maybeStartOpenTimer();
            break;
        case QEvent::Leave:
        case QEvent::FocusOut:
        case QEvent::Hide:
            hideTooltip();
            break;
        case QEvent::Destroy:
            m_openTimer->stop();
            m_target = nullptr;
            m_positionCacheValid = false;
            hide();
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

void AntToolTip::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntToolTip::hideEvent(QHideEvent* event)
{
    m_openTimer->stop();
    syncToolTipPerfCounters();
    QWidget::hideEvent(event);
}

void AntToolTip::mousePressEvent(QMouseEvent* event)
{
    hideTooltip();
    QWidget::mousePressEvent(event);
}

AntToolTip::Metrics AntToolTip::metrics() const
{
    Metrics m;
    return m;
}

const AntToolTip::ToolTipLayoutCache& AntToolTip::tooltipLayout() const
{
    const auto& token = antTheme->tokens();
    if (m_layoutCache.valid &&
        m_layoutCache.widgetSize == size() &&
        m_layoutCache.font == font() &&
        m_layoutCache.themeMode == antTheme->themeMode() &&
        m_layoutCache.tokenFontSizeSM == token.fontSizeSM &&
        m_layoutCache.tokenBorderRadiusSM == token.borderRadiusSM &&
        m_layoutCache.title == m_title &&
        m_layoutCache.customColor == m_color &&
        m_layoutCache.arrowVisible == m_arrowVisible &&
        m_layoutCache.renderPlacement == m_renderPlacement)
    {
        ++m_layoutCacheHitCount;
        syncToolTipPerfCounters();
        return m_layoutCache;
    }

    ++m_layoutBuildCount;
    ToolTipLayoutCache cache;
    cache.valid = true;
    cache.widgetSize = size();
    cache.font = font();
    cache.themeMode = antTheme->themeMode();
    cache.tokenFontSizeSM = token.fontSizeSM;
    cache.tokenBorderRadiusSM = token.borderRadiusSM;
    cache.title = m_title;
    cache.customColor = m_color;
    cache.arrowVisible = m_arrowVisible;
    cache.renderPlacement = m_renderPlacement;
    cache.metrics = metrics();
    cache.minimumSizeHint = QSize(48, 28);

    QFont textFont = font();
    textFont.setPixelSize(token.fontSizeSM);
    QFontMetrics fm(textFont);
    const QRect measuredText = fm.boundingRect(QRect(0, 0, cache.metrics.maxWidth, 400),
                                               Qt::TextWordWrap,
                                               m_title);
    const int arrow = m_arrowVisible ? cache.metrics.arrowSize : 0;
    if (m_renderPlacement == Ant::TooltipPlacement::Left || m_renderPlacement == Ant::TooltipPlacement::Right)
    {
        cache.sizeHint = QSize(measuredText.width() + cache.metrics.paddingX * 2 + arrow,
                               measuredText.height() + cache.metrics.paddingY * 2);
    }
    else
    {
        cache.sizeHint = QSize(measuredText.width() + cache.metrics.paddingX * 2,
                               measuredText.height() + cache.metrics.paddingY * 2 + arrow);
    }

    const QRect fullRect(QPoint(0, 0), size().isEmpty() ? cache.sizeHint : size());
    if (!m_arrowVisible)
    {
        cache.bubbleRect = fullRect;
    }
    else if (isTopPlacement(m_renderPlacement))
    {
        cache.bubbleRect = fullRect.adjusted(0, 0, 0, -cache.metrics.arrowSize);
    }
    else if (isBottomPlacement(m_renderPlacement))
    {
        cache.bubbleRect = fullRect.adjusted(0, cache.metrics.arrowSize, 0, 0);
    }
    else if (m_renderPlacement == Ant::TooltipPlacement::Left)
    {
        cache.bubbleRect = fullRect.adjusted(0, 0, -cache.metrics.arrowSize, 0);
    }
    else
    {
        cache.bubbleRect = fullRect.adjusted(cache.metrics.arrowSize, 0, 0, 0);
    }

    const QRect& bubble = cache.bubbleRect;
    if (m_arrowVisible)
    {
        const int arrowSize = cache.metrics.arrowSize;
        switch (m_renderPlacement)
        {
        case Ant::TooltipPlacement::Top:
            cache.arrowPolygon = QPolygonF({QPointF(fullRect.width() / 2.0 - arrowSize, bubble.bottom()),
                                            QPointF(fullRect.width() / 2.0 + arrowSize, bubble.bottom()),
                                            QPointF(fullRect.width() / 2.0, fullRect.bottom())});
            break;
        case Ant::TooltipPlacement::TopLeft:
            cache.arrowPolygon = QPolygonF({QPointF(bubble.left() + 18 - arrowSize, bubble.bottom()),
                                            QPointF(bubble.left() + 18 + arrowSize, bubble.bottom()),
                                            QPointF(bubble.left() + 18, fullRect.bottom())});
            break;
        case Ant::TooltipPlacement::TopRight:
            cache.arrowPolygon = QPolygonF({QPointF(bubble.right() - 18 - arrowSize, bubble.bottom()),
                                            QPointF(bubble.right() - 18 + arrowSize, bubble.bottom()),
                                            QPointF(bubble.right() - 18, fullRect.bottom())});
            break;
        case Ant::TooltipPlacement::Bottom:
            cache.arrowPolygon = QPolygonF({QPointF(fullRect.width() / 2.0 - arrowSize, bubble.top()),
                                            QPointF(fullRect.width() / 2.0 + arrowSize, bubble.top()),
                                            QPointF(fullRect.width() / 2.0, fullRect.top())});
            break;
        case Ant::TooltipPlacement::BottomLeft:
            cache.arrowPolygon = QPolygonF({QPointF(bubble.left() + 18 - arrowSize, bubble.top()),
                                            QPointF(bubble.left() + 18 + arrowSize, bubble.top()),
                                            QPointF(bubble.left() + 18, fullRect.top())});
            break;
        case Ant::TooltipPlacement::BottomRight:
            cache.arrowPolygon = QPolygonF({QPointF(bubble.right() - 18 - arrowSize, bubble.top()),
                                            QPointF(bubble.right() - 18 + arrowSize, bubble.top()),
                                            QPointF(bubble.right() - 18, fullRect.top())});
            break;
        case Ant::TooltipPlacement::Left:
            cache.arrowPolygon = QPolygonF({QPointF(bubble.right(), fullRect.height() / 2.0 - arrowSize),
                                            QPointF(bubble.right(), fullRect.height() / 2.0 + arrowSize),
                                            QPointF(fullRect.right(), fullRect.height() / 2.0)});
            break;
        case Ant::TooltipPlacement::Right:
        default:
            cache.arrowPolygon = QPolygonF({QPointF(bubble.left(), fullRect.height() / 2.0 - arrowSize),
                                            QPointF(bubble.left(), fullRect.height() / 2.0 + arrowSize),
                                            QPointF(fullRect.left(), fullRect.height() / 2.0)});
            break;
        }
    }

    cache.textRect = cache.bubbleRect.adjusted(cache.metrics.paddingX,
                                               cache.metrics.paddingY,
                                               -cache.metrics.paddingX,
                                               -cache.metrics.paddingY);
    cache.bubbleColor = bubbleColor();
    cache.textColor = textColor();

    m_layoutCache = cache;
    syncToolTipPerfCounters();
    return m_layoutCache;
}

void AntToolTip::invalidateToolTipLayout() const
{
    m_layoutCache.valid = false;
}

void AntToolTip::invalidateToolTipPosition()
{
    m_positionCacheValid = false;
}

QRect AntToolTip::bubbleRect() const
{
    return tooltipLayout().bubbleRect;
}

QPolygonF AntToolTip::arrowPolygon() const
{
    return tooltipLayout().arrowPolygon;
}

QColor AntToolTip::bubbleColor() const
{
    if (m_color.isValid())
    {
        return m_color;
    }
    return antTheme->themeMode() == Ant::ThemeMode::Dark ? QColor("#424242") : QColor("#262626");
}

QColor AntToolTip::textColor() const
{
    if (m_color.isValid())
    {
        return m_color.lightnessF() > 0.6 ? QColor(Qt::black) : QColor(Qt::white);
    }
    return QColor(Qt::white);
}

Ant::TooltipPlacement AntToolTip::resolvedPlacement(const QRect& targetRect, const QRect& screenRect, const QSize& tipSize) const
{
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

QPoint AntToolTip::tooltipTopLeft(const QRect& targetRect,
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

void AntToolTip::updatePosition()
{
    if (!m_target)
    {
        return;
    }

    const QRect targetRect(m_target->mapToGlobal(QPoint(0, 0)), m_target->size());
    const QRect screenRect = availableScreenGeometryFor(m_target);
    const QSize tooltipSize = sizeHint();
    const Ant::TooltipPlacement placement = resolvedPlacement(targetRect, screenRect, tooltipSize);
    QPoint topLeft = tooltipTopLeft(targetRect, tooltipSize, placement);

    topLeft.setX(qBound(screenRect.left() + 4, topLeft.x(), screenRect.right() - tooltipSize.width() - 4));
    topLeft.setY(qBound(screenRect.top() + 4, topLeft.y(), screenRect.bottom() - tooltipSize.height() - 4));

    if (m_positionCacheValid &&
        m_lastTargetRect == targetRect &&
        m_lastScreenRect == screenRect &&
        m_lastTooltipSize == tooltipSize &&
        m_lastTooltipTopLeft == topLeft &&
        m_lastPositionPlacement == placement)
    {
        ++m_positionSkipCount;
        syncToolTipPerfCounters();
        return;
    }

    const bool placementChanged = m_renderPlacement != placement;
    m_renderPlacement = placement;
    if (placementChanged)
    {
        invalidateToolTipLayout();
    }

    m_lastTargetRect = targetRect;
    m_lastScreenRect = screenRect;
    m_lastTooltipSize = tooltipSize;
    m_lastTooltipTopLeft = topLeft;
    m_lastPositionPlacement = placement;
    m_positionCacheValid = true;
    ++m_positionApplyCount;
    setGeometry(QRect(topLeft, tooltipSize));
    syncToolTipPerfCounters();
}

void AntToolTip::requestToolTipUpdate(const QRect& region, const QString& mode)
{
    m_lastUpdateMode = mode;
    ++m_regionUpdateCount;
    syncToolTipPerfCounters();

    if (region.isValid() && !region.isEmpty())
    {
        update(region);
        return;
    }
    update();
}

void AntToolTip::maybeStartOpenTimer()
{
    if (m_title.trimmed().isEmpty())
    {
        ++m_openTimerSkipCount;
        syncToolTipPerfCounters();
        return;
    }
    if (isVisible() || m_openTimer->isActive())
    {
        ++m_openTimerSkipCount;
        syncToolTipPerfCounters();
        return;
    }
    ++m_openTimerStartCount;
    m_openTimer->start(m_openDelay);
    syncToolTipPerfCounters();
}

void AntToolTip::syncToolTipPerfCounters() const
{
    auto* self = const_cast<AntToolTip*>(this);
    self->setProperty("antToolTipLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antToolTipLayoutCacheHitCount", m_layoutCacheHitCount);
    self->setProperty("antToolTipPositionApplyCount", m_positionApplyCount);
    self->setProperty("antToolTipPositionSkipCount", m_positionSkipCount);
    self->setProperty("antToolTipOpenTimerStartCount", m_openTimerStartCount);
    self->setProperty("antToolTipOpenTimerSkipCount", m_openTimerSkipCount);
    self->setProperty("antToolTipOpenTimerActive", m_openTimer && m_openTimer->isActive());
    self->setProperty("antToolTipRegionUpdateCount", m_regionUpdateCount);
    self->setProperty("antToolTipLastUpdateMode", m_lastUpdateMode);
}

void AntToolTip::installTarget(QWidget* target)
{
    m_target = target;
    if (m_target)
    {
        m_target->installEventFilter(this);
        m_target->setMouseTracking(true);
        m_targetDestroyedConnection = connect(m_target.data(), &QObject::destroyed, this, [this]() {
            m_openTimer->stop();
            m_target = nullptr;
            invalidateToolTipPosition();
            hide();
            syncToolTipPerfCounters();
        });
    }
    invalidateToolTipPosition();
    syncToolTipPerfCounters();
}

void AntToolTip::uninstallTarget()
{
    if (m_target)
    {
        m_target->removeEventFilter(this);
    }
    QObject::disconnect(m_targetDestroyedConnection);
    m_targetDestroyedConnection = {};
    m_target = nullptr;
    invalidateToolTipPosition();
    syncToolTipPerfCounters();
}
