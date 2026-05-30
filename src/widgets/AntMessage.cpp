#include "AntMessage.h"

#include <QApplication>
#include <QEnterEvent>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QScreen>
#include <QShowEvent>
#include <QTimer>

#include <algorithm>

#include "core/AntPopupMotion.h"
#include "core/AntTheme.h"
#include "styles/AntMessageStyle.h"

namespace
{
constexpr int MessageMotionDistance = 24;

bool isBottomMessagePlacement(Ant::Placement placement)
{
    return placement == Ant::Placement::Bottom ||
           placement == Ant::Placement::BottomLeft ||
           placement == Ant::Placement::BottomRight;
}

bool anchorCanHostMessage(QWidget* anchor)
{
    if (!anchor)
    {
        return true;
    }

    QWidget* window = anchor->window();
    return anchor->isVisible() &&
           (!window || window->isVisible()) &&
           !anchor->size().isEmpty() &&
           (!window || !window->size().isEmpty());
}

AntPopupMotion::Placement messageMotionPlacement(Ant::Placement placement)
{
    return isBottomMessagePlacement(placement)
               ? AntPopupMotion::Placement::Top
               : AntPopupMotion::Placement::Bottom;
}

QWidget* widgetBelowMessageAt(QWidget* anchor, QWidget* message, const QPoint& globalPos)
{
    auto widgetAtInWindow = [globalPos](QWidget* window) -> QWidget* {
        if (!window || !window->isVisible() || !window->geometry().contains(globalPos))
        {
            return nullptr;
        }

        QWidget* target = window->childAt(window->mapFromGlobal(globalPos));
        return target ? target : window;
    };

    if (QWidget* target = widgetAtInWindow(anchor ? anchor->window() : nullptr))
    {
        if (target != message && target->window() != message)
        {
            return target;
        }
    }

    const auto topLevels = QApplication::topLevelWidgets();
    for (QWidget* window : topLevels)
    {
        if (!window || window == message || window->window() == message)
        {
            continue;
        }
        if (QWidget* target = widgetAtInWindow(window))
        {
            return target;
        }
    }

    return nullptr;
}

void sendMouseClick(QWidget* target, const QPoint& globalPos, Qt::KeyboardModifiers modifiers)
{
    if (!target || !target->isEnabled())
    {
        return;
    }

    QPointer<QWidget> guard(target);
    const QPoint localPos = target->mapFromGlobal(globalPos);
    QMouseEvent pressEvent(QEvent::MouseButtonPress,
                           QPointF(localPos),
                           QPointF(globalPos),
                           Qt::LeftButton,
                           Qt::LeftButton,
                           modifiers);
    QApplication::sendEvent(target, &pressEvent);

    if (!guard)
    {
        return;
    }

    QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
                             QPointF(localPos),
                             QPointF(globalPos),
                             Qt::LeftButton,
                             Qt::NoButton,
                             modifiers);
    QApplication::sendEvent(target, &releaseEvent);
}

void drawMessageShadow(QPainter& painter, const QRectF& bubble, qreal radius)
{
    painter.save();
    const bool dark = antTheme->themeMode() == Ant::ThemeMode::Dark;
    constexpr int ShadowLayers = 14;
    const qreal maxAlpha = dark ? 0.040 : 0.024;
    for (int i = ShadowLayers; i >= 1; --i)
    {
        const qreal progress = 1.0 - static_cast<qreal>(i) / ShadowLayers;
        QColor shadow = antTheme->tokens().colorShadow;
        shadow.setAlphaF(maxAlpha * progress * progress);

        const QRectF layer = bubble.adjusted(-i * 0.26, -i * 0.08 + 1.2, i * 0.26, i * 0.32 + 2.4);
        QPainterPath outer;
        outer.addRoundedRect(layer, radius + i * 0.25, radius + i * 0.25);
        QPainterPath inner;
        inner.addRoundedRect(bubble, radius, radius);
        painter.fillPath(outer.subtracted(inner), shadow);
    }

    painter.restore();
}
} // namespace

AntMessage::AntMessage(QWidget* parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
{
    installAntStyle<AntMessageStyle>(this);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setMouseTracking(true);

    m_closeTimer = new QTimer(this);
    m_closeTimer->setSingleShot(true);
    connect(m_closeTimer, &QTimer::timeout, this, [this]() {
        AntPopupMotion::close(this, messageMotionPlacement(m_placement), MessageMotionDistance);
    });

    m_loadingTimer = new QTimer(this);
    connect(m_loadingTimer, &QTimer::timeout, this, [this]() {
        m_loadingAngle = (m_loadingAngle + 30) % 360;
        requestMessageUpdate(messageLayout().iconRect.toAlignedRect().adjusted(-3, -3, 3, 3),
                             QStringLiteral("loading"),
                             true);
    });

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateMessageLayout();
        clearShadowCache();
        applyMessageSizeHint();
        requestMessageUpdate(rect(), QStringLiteral("theme"));
    });
    syncMessagePerfCounters();
}

AntMessage::~AntMessage()
{
    uninstallAnchorWatcher();
}

AntMessage* AntMessage::open(const QString& text, Ant::MessageType type, QWidget* anchor, int durationMs, Ant::Placement placement)
{
    auto* message = new AntMessage();
    message->installAnchorWatcher(anchor);
    message->m_placement = placement;
    message->setText(text);
    message->setMessageType(type);
    message->setDuration(durationMs);
    message->applyMessageSizeHint();

    if (!message->anchorReady())
    {
        message->setProperty("antMessageSuppressedForHiddenAnchor", true);
        QPointer<AntMessage> guard(message);
        QTimer::singleShot(0, message, [guard]() {
            if (guard)
            {
                guard->deleteLater();
            }
        });
        return message;
    }

    activeMessages().append(message);

    QPointer<QWidget> anchorGuard(anchor);
    QObject::connect(message, &QObject::destroyed, qApp, [message, anchorGuard]() {
        activeMessages().removeAll(message);
        relayoutMessages(anchorGuard.data());
    });

    relayoutMessages(anchor);
    AntPopupMotion::show(message, messageMotionPlacement(placement), MessageMotionDistance);
    return message;
}

AntMessage* AntMessage::info(const QString& text, QWidget* anchor, int durationMs, Ant::Placement placement)
{
    return open(text, Ant::MessageType::Info, anchor, durationMs, placement);
}

AntMessage* AntMessage::success(const QString& text, QWidget* anchor, int durationMs, Ant::Placement placement)
{
    return open(text, Ant::MessageType::Success, anchor, durationMs, placement);
}

AntMessage* AntMessage::warning(const QString& text, QWidget* anchor, int durationMs, Ant::Placement placement)
{
    return open(text, Ant::MessageType::Warning, anchor, durationMs, placement);
}

AntMessage* AntMessage::error(const QString& text, QWidget* anchor, int durationMs, Ant::Placement placement)
{
    return open(text, Ant::MessageType::Error, anchor, durationMs, placement);
}

AntMessage* AntMessage::loading(const QString& text, QWidget* anchor, int durationMs, Ant::Placement placement)
{
    return open(text, Ant::MessageType::Loading, anchor, durationMs, placement);
}

QString AntMessage::text() const { return m_text; }

void AntMessage::setText(const QString& text)
{
    if (m_text == text)
    {
        return;
    }
    m_text = text;
    invalidateMessageLayout();
    clearShadowCache();
    applyMessageSizeHint();
    requestMessageUpdate(rect(), QStringLiteral("text"));
    Q_EMIT textChanged(m_text);
}

Ant::MessageType AntMessage::messageType() const { return m_messageType; }

void AntMessage::setMessageType(Ant::MessageType type)
{
    if (m_messageType == type)
    {
        return;
    }
    m_messageType = type;
    invalidateMessageLayout();
    updateLoadingState();
    requestMessageUpdate(messageLayout().iconRect.toAlignedRect().adjusted(-3, -3, 3, 3),
                         QStringLiteral("type"));
    Q_EMIT messageTypeChanged(m_messageType);
}

int AntMessage::duration() const { return m_duration; }

void AntMessage::setDuration(int durationMs)
{
    durationMs = std::max(0, durationMs);
    if (m_duration == durationMs)
    {
        return;
    }
    m_duration = durationMs;
    startTimers();
    Q_EMIT durationChanged(m_duration);
}

bool AntMessage::pauseOnHover() const { return m_pauseOnHover; }

void AntMessage::setPauseOnHover(bool pause)
{
    if (m_pauseOnHover == pause)
    {
        return;
    }
    m_pauseOnHover = pause;
    Q_EMIT pauseOnHoverChanged(m_pauseOnHover);
}

int AntMessage::loadingAngle() const { return m_loadingAngle; }

QSize AntMessage::sizeHint() const
{
    return messageLayout().sizeHint;
}

QSize AntMessage::minimumSizeHint() const
{
    return messageLayout().minimumSizeHint;
}

void AntMessage::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

bool AntMessage::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == m_anchor.data() || watched == m_anchorWindow.data()) && event)
    {
        switch (event->type())
        {
        case QEvent::Move:
        case QEvent::Resize:
        case QEvent::Show:
        case QEvent::Hide:
        case QEvent::WindowStateChange:
            handleAnchorChanged(event->type());
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntMessage::showEvent(QShowEvent* event)
{
    startTimers();
    updateLoadingState();
    QWidget::showEvent(event);
}

void AntMessage::hideEvent(QHideEvent* event)
{
    m_closeTimer->stop();
    m_loadingTimer->stop();
    Q_EMIT closed();
    QWidget::hideEvent(event);
}

void AntMessage::enterEvent(AntEnterEvent* event)
{
    m_hovered = true;
    if (m_pauseOnHover)
    {
        m_closeTimer->stop();
    }
    QWidget::enterEvent(event);
}

void AntMessage::leaveEvent(QEvent* event)
{
    m_hovered = false;
    if (m_pauseOnHover)
    {
        startTimers();
    }
    QWidget::leaveEvent(event);
}

void AntMessage::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        sendMouseClick(widgetBelowMessageAt(m_anchor.data(), this, antEventGlobalPosition(event)),
                       antEventGlobalPosition(event),
                       event->modifiers());
        AntPopupMotion::close(this, messageMotionPlacement(m_placement), MessageMotionDistance);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

QList<AntMessage*>& AntMessage::activeMessages()
{
    static QList<AntMessage*> messages;
    return messages;
}

void AntMessage::relayoutMessages(QWidget* anchor)
{
    QRect targetRect;
    if (anchor)
    {
        targetRect = QRect(anchor->mapToGlobal(QPoint(0, 0)), anchor->size());
    }
    else if (QScreen* screen = QGuiApplication::primaryScreen())
    {
        targetRect = screen->availableGeometry();
    }
    else
    {
        targetRect = QRect(0, 0, 800, 600);
    }

    const auto isBottomPlacement = [](Ant::Placement placement) {
        return placement == Ant::Placement::Bottom ||
               placement == Ant::Placement::BottomLeft ||
               placement == Ant::Placement::BottomRight;
    };

    const auto layoutPlacement = [&](Ant::Placement placement) {
        int cursor = isBottomPlacement(placement) ? targetRect.bottom() - 12 : targetRect.top() + 12;
        for (AntMessage* message : activeMessages())
        {
            if (!message || message->m_placement != placement)
            {
                continue;
            }
            if (message->m_anchor.data() != anchor)
            {
                continue;
            }
            if (!message->anchorReady())
            {
                if (message->isVisible() && !AntPopupMotion::isClosing(message))
                {
                    AntPopupMotion::close(message, messageMotionPlacement(message->m_placement), MessageMotionDistance);
                }
                continue;
            }

            message->applyMessageSizeHint();
            int x = targetRect.center().x() - message->width() / 2;
            switch (placement)
            {
            case Ant::Placement::TopLeft:
            case Ant::Placement::BottomLeft:
                x = targetRect.left() + 24;
                break;
            case Ant::Placement::TopRight:
            case Ant::Placement::BottomRight:
                x = targetRect.right() - message->width() - 24;
                break;
            default:
                break;
            }

            if (isBottomPlacement(placement))
            {
                cursor -= message->height();
                const QPoint targetPos(x, cursor);
                if (message->pos() == targetPos)
                {
                    ++message->m_relayoutSkipCount;
                }
                else
                {
                    if (message->isVisible() && !AntPopupMotion::isClosing(message))
                    {
                        AntPopupMotion::stop(message);
                    }
                    message->move(targetPos);
                    ++message->m_relayoutMoveCount;
                }
                message->syncMessagePerfCounters();
                cursor -= 4;
            }
            else
            {
                const QPoint targetPos(x, cursor);
                if (message->pos() == targetPos)
                {
                    ++message->m_relayoutSkipCount;
                }
                else
                {
                    if (message->isVisible() && !AntPopupMotion::isClosing(message))
                    {
                        AntPopupMotion::stop(message);
                    }
                    message->move(targetPos);
                    ++message->m_relayoutMoveCount;
                }
                message->syncMessagePerfCounters();
                cursor += message->height() + 4;
            }
        }
    };

    for (Ant::Placement placement :
         {Ant::Placement::Top,
          Ant::Placement::TopLeft,
          Ant::Placement::TopRight,
          Ant::Placement::Bottom,
          Ant::Placement::BottomLeft,
          Ant::Placement::BottomRight})
    {
        layoutPlacement(placement);
    }
}

void AntMessage::installAnchorWatcher(QWidget* anchor)
{
    uninstallAnchorWatcher();
    m_anchor = anchor;
    if (!anchor)
    {
        return;
    }

    anchor->installEventFilter(this);
    connect(anchor, &QObject::destroyed, this, [this]() {
        if (m_anchorWindow && m_anchorWindow.data() != m_anchor.data())
        {
            m_anchorWindow->removeEventFilter(this);
        }
        m_anchor.clear();
        m_anchorWindow.clear();
        if (!AntPopupMotion::isClosing(this))
        {
            AntPopupMotion::close(this, messageMotionPlacement(m_placement), MessageMotionDistance);
        }
    });

    QWidget* window = anchor->window();
    if (window && window != anchor)
    {
        m_anchorWindow = window;
        window->installEventFilter(this);
    }
}

void AntMessage::uninstallAnchorWatcher()
{
    if (m_anchor)
    {
        m_anchor->removeEventFilter(this);
    }
    if (m_anchorWindow)
    {
        m_anchorWindow->removeEventFilter(this);
    }
    m_anchor.clear();
    m_anchorWindow.clear();
}

bool AntMessage::anchorReady() const
{
    return anchorCanHostMessage(m_anchor.data());
}

void AntMessage::handleAnchorChanged(QEvent::Type type)
{
    if (type == QEvent::Hide)
    {
        if (isVisible() && !AntPopupMotion::isClosing(this))
        {
            AntPopupMotion::close(this, messageMotionPlacement(m_placement), MessageMotionDistance);
        }
        return;
    }

    if (!anchorReady())
    {
        return;
    }

    relayoutMessages(m_anchor.data());
}

const AntMessage::MessageLayout& AntMessage::messageLayout() const
{
    const QString fontKey = font().toString();
    const bool cacheMatches = m_layoutCache.valid
        && m_layoutCache.widgetSize == size()
        && m_layoutCache.text == m_text
        && m_layoutCache.messageType == m_messageType
        && m_layoutCache.themeMode == antTheme->themeMode()
        && m_layoutCache.fontKey == fontKey;
    if (cacheMatches)
    {
        ++m_layoutCacheHitCount;
        syncMessagePerfCounters();
        return m_layoutCache;
    }

    ++m_layoutBuildCount;
    const auto& token = antTheme->tokens();

    MessageLayout layout;
    layout.valid = true;
    layout.widgetSize = size();
    layout.text = m_text;
    layout.messageType = m_messageType;
    layout.themeMode = antTheme->themeMode();
    layout.fontKey = fontKey;
    layout.radius = token.borderRadiusLG;
    layout.accentColor = accentColor();

    QFont textFont = font();
    textFont.setPixelSize(token.fontSize);
    QFontMetrics fm(textFont);
    const int textWidth = std::min(420, fm.horizontalAdvance(m_text));
    layout.sizeHint = QSize(textWidth + 64, token.controlHeightLG + 12);
    layout.minimumSizeHint = QSize(120, token.controlHeightLG);

    layout.bubbleRect = rect().adjusted(8, 4, -8, -8);
    layout.iconRect = QRectF(layout.bubbleRect.left() + token.paddingSM,
                             layout.bubbleRect.center().y() - 8,
                             16,
                             16);
    layout.textRect = layout.bubbleRect.adjusted(token.paddingSM + 24, 0, -token.paddingSM, 0);

    m_layoutCache = layout;
    syncMessagePerfCounters();
    return m_layoutCache;
}

void AntMessage::invalidateMessageLayout() const
{
    m_layoutCache.valid = false;
}

QPixmap AntMessage::cachedShadowPixmap() const
{
    const auto& layout = messageLayout();
    const qreal ratio = devicePixelRatioF();
    const QString key = QStringLiteral("%1x%2:%3:%4:%5:%6")
        .arg(width())
        .arg(height())
        .arg(qRound(ratio * 100.0))
        .arg(static_cast<int>(antTheme->themeMode()))
        .arg(antTheme->tokens().colorShadow.rgba())
        .arg(layout.radius);

    if (!m_shadowPixmapCache.isNull() && m_shadowPixmapCacheKey == key)
    {
        ++m_shadowCacheHitCount;
        syncMessagePerfCounters();
        return m_shadowPixmapCache;
    }

    ++m_shadowBuildCount;
    m_shadowPixmapCacheKey = key;
    const QSize pixmapSize(qMax(1, qRound(width() * ratio)), qMax(1, qRound(height() * ratio)));
    QPixmap pixmap(pixmapSize);
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    drawMessageShadow(painter, layout.bubbleRect, layout.radius);
    painter.end();

    m_shadowPixmapCache = pixmap;
    syncMessagePerfCounters();
    return m_shadowPixmapCache;
}

void AntMessage::clearShadowCache() const
{
    m_shadowPixmapCache = QPixmap();
    m_shadowPixmapCacheKey.clear();
}

void AntMessage::applyMessageSizeHint()
{
    const QSize target = sizeHint();
    if (size() == target)
    {
        ++m_sizeSkipCount;
        syncMessagePerfCounters();
        return;
    }
    resize(target);
    invalidateMessageLayout();
    clearShadowCache();
    ++m_sizeApplyCount;
    syncMessagePerfCounters();
}

void AntMessage::requestMessageUpdate(const QRect& region, const QString& mode, bool spinnerScoped)
{
    m_lastUpdateMode = mode;
    ++m_regionUpdateCount;
    if (spinnerScoped)
    {
        ++m_spinnerRegionUpdateCount;
    }
    syncMessagePerfCounters();
    if (region.isValid() && !region.isEmpty())
    {
        update(region);
        return;
    }
    update();
}

void AntMessage::syncMessagePerfCounters() const
{
    auto* that = const_cast<AntMessage*>(this);
    that->setProperty("antMessageLayoutBuildCount", m_layoutBuildCount);
    that->setProperty("antMessageLayoutCacheHitCount", m_layoutCacheHitCount);
    that->setProperty("antMessageShadowBuildCount", m_shadowBuildCount);
    that->setProperty("antMessageShadowCacheHitCount", m_shadowCacheHitCount);
    that->setProperty("antMessageSizeApplyCount", m_sizeApplyCount);
    that->setProperty("antMessageSizeSkipCount", m_sizeSkipCount);
    that->setProperty("antMessageRegionUpdateCount", m_regionUpdateCount);
    that->setProperty("antMessageSpinnerRegionUpdateCount", m_spinnerRegionUpdateCount);
    that->setProperty("antMessageRelayoutMoveCount", m_relayoutMoveCount);
    that->setProperty("antMessageRelayoutSkipCount", m_relayoutSkipCount);
    that->setProperty("antMessageLastUpdateMode", m_lastUpdateMode);
}

QColor AntMessage::accentColor() const
{
    const auto& token = antTheme->tokens();
    switch (m_messageType)
    {
    case Ant::MessageType::Success:
        return token.colorSuccess;
    case Ant::MessageType::Warning:
        return token.colorWarning;
    case Ant::MessageType::Error:
        return token.colorError;
    case Ant::MessageType::Loading:
    case Ant::MessageType::Info:
    default:
        return token.colorPrimary;
    }
}

void AntMessage::startTimers()
{
    if (m_duration > 0 && isVisible() && !m_hovered)
    {
        m_closeTimer->start(m_duration);
    }
    else
    {
        m_closeTimer->stop();
    }
}

void AntMessage::updateLoadingState()
{
    if (m_messageType == Ant::MessageType::Loading && isVisible())
    {
        m_loadingTimer->start(80);
    }
    else
    {
        m_loadingTimer->stop();
    }
}

void AntMessage::drawLoadingIcon(QPainter& painter, const QRectF& rect) const
{
    painter.save();
    painter.setPen(QPen(accentColor(), 1.8, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(rect.adjusted(1, 1, -1, -1), m_loadingAngle * 16, 270 * 16);
    painter.restore();
}
