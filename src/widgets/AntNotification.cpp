#include "AntNotification.h"

#include <QApplication>
#include <QEnterEvent>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPointer>
#include <QScreen>
#include <QShowEvent>
#include <QTextOption>
#include <QTimer>

#include <algorithm>

#include "core/AntPopupMotion.h"
#include "core/AntTheme.h"
#include "styles/AntNotificationStyle.h"
#include "styles/AntPalette.h"

namespace
{
constexpr int NoticeWidth = 384;
constexpr int ShadowInset = 18;
constexpr int NotificationMotionDistance = 24;

bool isTopPlacement(Ant::Placement placement)
{
    return placement == Ant::Placement::Top || placement == Ant::Placement::TopLeft ||
           placement == Ant::Placement::TopRight;
}

bool isLeftPlacement(Ant::Placement placement)
{
    return placement == Ant::Placement::TopLeft || placement == Ant::Placement::BottomLeft;
}

bool isRightPlacement(Ant::Placement placement)
{
    return placement == Ant::Placement::TopRight || placement == Ant::Placement::BottomRight;
}

bool anchorCanHostNotification(QWidget* anchor)
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

AntPopupMotion::Placement notificationMotionPlacement(Ant::Placement placement)
{
    if (isRightPlacement(placement))
    {
        return AntPopupMotion::Placement::Left;
    }
    if (isLeftPlacement(placement))
    {
        return AntPopupMotion::Placement::Right;
    }
    return AntPopupMotion::fromPlacement(placement);
}

void drawShadowLayer(QPainter& painter, const QRectF& card, int blur, qreal yOffset, qreal alpha, qreal radius)
{
    QColor shadow = antTheme->tokens().colorShadow;
    for (int i = blur; i >= 1; --i)
    {
        const qreal progress = 1.0 - static_cast<qreal>(i) / static_cast<qreal>(blur);
        shadow.setAlphaF(alpha * progress * progress);
        const QRectF layer = card.adjusted(-i, -i + yOffset, i, i + yOffset);
        QPainterPath outer;
        outer.addRoundedRect(layer, radius + i, radius + i);
        QPainterPath inner;
        inner.addRoundedRect(card, radius, radius);
        painter.fillPath(outer.subtracted(inner), shadow);
    }
}

void drawNotificationShadow(QPainter& painter, const QRectF& card, qreal radius)
{
    const bool dark = antTheme->themeMode() == Ant::ThemeMode::Dark;
    drawShadowLayer(painter, card, 12, 4, dark ? 0.048 : 0.026, radius);
    drawShadowLayer(painter, card, 5, 1, dark ? 0.022 : 0.012, radius);
}
} // namespace

AntNotification::AntNotification(QWidget* parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
{
    installAntStyle<AntNotificationStyle>(this);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setMouseTracking(true);

    m_closeTimer = new QTimer(this);
    m_closeTimer->setSingleShot(true);
    connect(m_closeTimer, &QTimer::timeout, this, [this]() {
        AntPopupMotion::close(this, notificationMotionPlacement(m_placement), NotificationMotionDistance);
    });

    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, [this]() {
        requestNotificationUpdate(notificationLayout().progressTrackRect.toAlignedRect().adjusted(-1, -1, 1, 1),
                                  QStringLiteral("progress"),
                                  true);
    });

    m_spinnerTimer = new QTimer(this);
    connect(m_spinnerTimer, &QTimer::timeout, this, [this]() {
        m_spinnerAngle = (m_spinnerAngle + 30) % 360;
        requestNotificationUpdate(notificationLayout().iconRect.toAlignedRect().adjusted(-3, -3, 3, 3),
                                  QStringLiteral("loading"),
                                  false,
                                  true);
    });

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateNotificationLayout();
        clearShadowCache();
        applyNotificationSizeHint();
        requestNotificationUpdate(rect(), QStringLiteral("theme"));
    });
    syncNotificationPerfCounters();
}

AntNotification::~AntNotification()
{
    uninstallAnchorWatcher();
}

AntNotification* AntNotification::open(const QString& title,
                                       const QString& description,
                                       QWidget* anchor,
                                       int durationMs,
                                       Ant::Placement placement)
{
    return create(title, description, Ant::MessageType::Info, false, anchor, durationMs, placement);
}

AntNotification* AntNotification::open(const QString& title,
                                       const QString& description,
                                       Ant::MessageType type,
                                       QWidget* anchor,
                                       int durationMs,
                                       Ant::Placement placement)
{
    return create(title, description, type, true, anchor, durationMs, placement);
}

AntNotification* AntNotification::create(const QString& title,
                                         const QString& description,
                                         Ant::MessageType type,
                                         bool iconVisible,
                                         QWidget* anchor,
                                         int durationMs,
                                         Ant::Placement placement)
{
    auto* notification = new AntNotification();
    notification->installAnchorWatcher(anchor);
    notification->setTitle(title);
    notification->setDescription(description);
    notification->setNotificationType(type);
    notification->setIconVisible(iconVisible);
    notification->setPlacement(placement);
    notification->setDuration(durationMs);
    notification->applyNotificationSizeHint();

    if (!notification->anchorReady())
    {
        notification->setProperty("antNotificationSuppressedForHiddenAnchor", true);
        QPointer<AntNotification> guard(notification);
        QTimer::singleShot(0, notification, [guard]() {
            if (guard)
            {
                guard->deleteLater();
            }
        });
        return notification;
    }

    activeNotifications().append(notification);

    QPointer<QWidget> anchorGuard(anchor);
    QObject::connect(notification, &QObject::destroyed, qApp, [notification, anchorGuard]() {
        activeNotifications().removeAll(notification);
        relayoutNotifications(anchorGuard.data());
    });

    relayoutNotifications(anchor);
    AntPopupMotion::show(notification, notificationMotionPlacement(placement), NotificationMotionDistance);
    return notification;
}

AntNotification* AntNotification::info(const QString& title,
                                       const QString& description,
                                       QWidget* anchor,
                                       int durationMs,
                                       Ant::Placement placement)
{
    return open(title, description, Ant::MessageType::Info, anchor, durationMs, placement);
}

AntNotification* AntNotification::success(const QString& title,
                                          const QString& description,
                                          QWidget* anchor,
                                          int durationMs,
                                          Ant::Placement placement)
{
    return open(title, description, Ant::MessageType::Success, anchor, durationMs, placement);
}

AntNotification* AntNotification::warning(const QString& title,
                                          const QString& description,
                                          QWidget* anchor,
                                          int durationMs,
                                          Ant::Placement placement)
{
    return open(title, description, Ant::MessageType::Warning, anchor, durationMs, placement);
}

AntNotification* AntNotification::error(const QString& title,
                                        const QString& description,
                                        QWidget* anchor,
                                        int durationMs,
                                        Ant::Placement placement)
{
    return open(title, description, Ant::MessageType::Error, anchor, durationMs, placement);
}

AntNotification* AntNotification::progress(const QString& title,
                                           const QString& description,
                                           int progress,
                                           QWidget* anchor,
                                           Ant::Placement placement)
{
    auto* notification = create(title,
                                description,
                                Ant::MessageType::Loading,
                                true,
                                anchor,
                                0,
                                placement);
    notification->setProgress(progress);
    return notification;
}

void AntNotification::closeAll()
{
    const auto notifications = activeNotifications();
    for (AntNotification* notification : notifications)
    {
        if (notification)
        {
            AntPopupMotion::close(notification,
                                  notificationMotionPlacement(notification->placement()),
                                  NotificationMotionDistance);
        }
    }
}

QString AntNotification::title() const { return m_title; }

void AntNotification::setTitle(const QString& title)
{
    if (m_title == title)
    {
        return;
    }
    m_title = title;
    invalidateNotificationLayout();
    applyNotificationSizeHint();
    requestNotificationUpdate(rect(), QStringLiteral("title"));
    Q_EMIT titleChanged(m_title);
}

QString AntNotification::description() const { return m_description; }

void AntNotification::setDescription(const QString& description)
{
    if (m_description == description)
    {
        return;
    }
    m_description = description;
    invalidateNotificationLayout();
    applyNotificationSizeHint();
    requestNotificationUpdate(rect(), QStringLiteral("description"));
    Q_EMIT descriptionChanged(m_description);
}

Ant::MessageType AntNotification::notificationType() const { return m_notificationType; }

void AntNotification::setNotificationType(Ant::MessageType type)
{
    if (m_notificationType == type)
    {
        return;
    }
    m_notificationType = type;
    invalidateNotificationLayout();
    updateSpinnerState();
    QRect dirty = notificationLayout().iconRect.toAlignedRect().adjusted(-3, -3, 3, 3);
    if (m_showProgress)
    {
        dirty = dirty.united(notificationLayout().progressTrackRect.toAlignedRect().adjusted(-1, -1, 1, 1));
    }
    requestNotificationUpdate(dirty, QStringLiteral("type"));
    Q_EMIT notificationTypeChanged(m_notificationType);
}

Ant::Placement AntNotification::placement() const { return m_placement; }

void AntNotification::setPlacement(Ant::Placement placement)
{
    if (m_placement == placement)
    {
        return;
    }
    m_placement = placement;
    relayoutNotifications(m_anchor.data());
    Q_EMIT placementChanged(m_placement);
}

int AntNotification::duration() const { return m_duration; }

void AntNotification::setDuration(int durationMs)
{
    durationMs = std::max(0, durationMs);
    if (m_duration == durationMs)
    {
        return;
    }
    m_duration = durationMs;
    m_remainingMs = durationMs;
    if (m_showProgress)
    {
        invalidateNotificationLayout();
        applyNotificationSizeHint();
        requestNotificationUpdate(notificationLayout().progressTrackRect.toAlignedRect().adjusted(-1, -1, 1, 1),
                                  QStringLiteral("duration"),
                                  true);
    }
    startCloseTimer();
    Q_EMIT durationChanged(m_duration);
}

bool AntNotification::pauseOnHover() const { return m_pauseOnHover; }

void AntNotification::setPauseOnHover(bool pause)
{
    if (m_pauseOnHover == pause)
    {
        return;
    }
    m_pauseOnHover = pause;
    Q_EMIT pauseOnHoverChanged(m_pauseOnHover);
}

bool AntNotification::showProgress() const { return m_showProgress; }

void AntNotification::setShowProgress(bool show)
{
    if (m_showProgress == show)
    {
        return;
    }
    m_showProgress = show;
    invalidateNotificationLayout();
    applyNotificationSizeHint();
    if (m_showProgress && m_progressMode == ProgressMode::Countdown && m_duration > 0 && isVisible())
    {
        m_progressTimer->start(50);
    }
    else
    {
        m_progressTimer->stop();
    }
    requestNotificationUpdate(rect(), QStringLiteral("progressVisible"));
    Q_EMIT showProgressChanged(m_showProgress);
}

AntNotification::ProgressMode AntNotification::progressMode() const { return m_progressMode; }

void AntNotification::setProgressMode(ProgressMode mode)
{
    if (m_progressMode == mode)
    {
        return;
    }

    m_progressMode = mode;
    invalidateNotificationLayout();
    applyNotificationSizeHint();
    if (m_showProgress && m_progressMode == ProgressMode::Countdown && m_duration > 0 && isVisible())
    {
        m_progressTimer->start(50);
    }
    else
    {
        m_progressTimer->stop();
    }
    requestNotificationUpdate(rect(), QStringLiteral("progressMode"));
    Q_EMIT progressModeChanged(m_progressMode);
}

int AntNotification::progress() const { return m_progress; }

void AntNotification::setProgress(int progress)
{
    progress = std::clamp(progress, 0, 100);
    const bool valueChanged = m_progress != progress;
    if (m_progressMode != ProgressMode::Manual)
    {
        setProgressMode(ProgressMode::Manual);
    }
    if (!m_showProgress)
    {
        setShowProgress(true);
    }
    if (!valueChanged)
    {
        return;
    }

    m_progress = progress;
    const QRect progressDirty = notificationLayout().progressTrackRect.toAlignedRect().adjusted(-1, -1, 1, 1);
    requestNotificationUpdate(progressDirty, QStringLiteral("manualProgress"), true);
    Q_EMIT progressChanged(m_progress);
}

bool AntNotification::isClosable() const { return m_closable; }

void AntNotification::setClosable(bool closable)
{
    if (m_closable == closable)
    {
        return;
    }
    m_closable = closable;
    invalidateNotificationLayout();
    applyNotificationSizeHint();
    requestNotificationUpdate(rect(), QStringLiteral("closable"));
    Q_EMIT closableChanged(m_closable);
}

bool AntNotification::iconVisible() const { return m_iconVisible; }

void AntNotification::setIconVisible(bool visible)
{
    if (m_iconVisible == visible)
    {
        return;
    }
    m_iconVisible = visible;
    invalidateNotificationLayout();
    updateSpinnerState();
    applyNotificationSizeHint();
    requestNotificationUpdate(rect(), QStringLiteral("iconVisible"));
    Q_EMIT iconVisibleChanged(m_iconVisible);
}

int AntNotification::spinnerAngle() const { return m_spinnerAngle; }

QSize AntNotification::sizeHint() const
{
    return notificationLayout().sizeHint;
}

QSize AntNotification::minimumSizeHint() const
{
    return notificationLayout().minimumSizeHint;
}

void AntNotification::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

bool AntNotification::eventFilter(QObject* watched, QEvent* event)
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

void AntNotification::showEvent(QShowEvent* event)
{
    m_remainingMs = m_duration;
    applyNotificationSizeHint();
    startCloseTimer();
    updateSpinnerState();
    QWidget::showEvent(event);
}

void AntNotification::hideEvent(QHideEvent* event)
{
    m_closeTimer->stop();
    m_progressTimer->stop();
    m_spinnerTimer->stop();
    Q_EMIT closed();
    QWidget::hideEvent(event);
}

void AntNotification::enterEvent(AntEnterEvent* event)
{
    m_hovered = true;
    if (m_pauseOnHover)
    {
        pauseCloseTimer();
    }
    QWidget::enterEvent(event);
}

void AntNotification::leaveEvent(QEvent* event)
{
    m_hovered = false;
    const QRect closeDirty = notificationLayout().closeRect.toAlignedRect().adjusted(-2, -2, 2, 2);
    const bool closeChanged = m_closeHovered;
    m_closeHovered = false;
    if (m_pauseOnHover)
    {
        resumeCloseTimer();
    }
    if (closeChanged)
    {
        requestNotificationUpdate(closeDirty, QStringLiteral("closeHover"), false, false, true);
    }
    QWidget::leaveEvent(event);
}

void AntNotification::mouseMoveEvent(QMouseEvent* event)
{
    const bool hoveringClose = m_closable && closeButtonRect().contains(antEventPosition(event));
    if (m_closeHovered != hoveringClose)
    {
        const QRect closeDirty = notificationLayout().closeRect.toAlignedRect().adjusted(-2, -2, 2, 2);
        m_closeHovered = hoveringClose;
        requestNotificationUpdate(closeDirty, QStringLiteral("closeHover"), false, false, true);
    }
    QWidget::mouseMoveEvent(event);
}

void AntNotification::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (m_closable && closeButtonRect().contains(antEventPosition(event)))
        {
            AntPopupMotion::close(this, notificationMotionPlacement(m_placement), NotificationMotionDistance);
            event->accept();
            return;
        }
        Q_EMIT clicked();
    }
    QWidget::mousePressEvent(event);
}

QList<AntNotification*>& AntNotification::activeNotifications()
{
    static QList<AntNotification*> notifications;
    return notifications;
}

void AntNotification::relayoutNotifications(QWidget* anchor)
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
        targetRect = QRect(0, 0, 1024, 768);
    }

    const auto& token = antTheme->tokens();
    for (Ant::Placement placement :
         {Ant::Placement::Top,
          Ant::Placement::TopLeft,
          Ant::Placement::TopRight,
          Ant::Placement::Bottom,
          Ant::Placement::BottomLeft,
          Ant::Placement::BottomRight})
    {
        int edge = token.marginLG;
        int cursor = isTopPlacement(placement) ? targetRect.top() + edge : targetRect.bottom() - edge;
        for (AntNotification* notification : activeNotifications())
        {
            if (!notification || notification->m_placement != placement)
            {
                continue;
            }
            if (notification->m_anchor.data() != anchor)
            {
                continue;
            }
            if (!notification->anchorReady())
            {
                if (notification->isVisible() && !AntPopupMotion::isClosing(notification))
                {
                    AntPopupMotion::close(notification,
                                          notificationMotionPlacement(notification->m_placement),
                                          NotificationMotionDistance);
                }
                continue;
            }
            notification->applyNotificationSizeHint();
            int x = targetRect.center().x() - notification->width() / 2;
            if (isLeftPlacement(placement))
            {
                x = targetRect.left() + edge;
            }
            else if (isRightPlacement(placement))
            {
                x = targetRect.right() - edge - notification->width();
            }

            int y = cursor;
            if (isTopPlacement(placement))
            {
                cursor += notification->height() + token.margin;
            }
            else
            {
                y -= notification->height();
                cursor = y - token.margin;
            }
            const QPoint targetPos(x, y);
            if (notification->pos() == targetPos)
            {
                ++notification->m_relayoutSkipCount;
            }
            else
            {
                if (notification->isVisible() && !AntPopupMotion::isClosing(notification))
                {
                    AntPopupMotion::stop(notification);
                }
                notification->move(targetPos);
                ++notification->m_relayoutMoveCount;
            }
            notification->syncNotificationPerfCounters();
        }
    }
}

void AntNotification::installAnchorWatcher(QWidget* anchor)
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
            AntPopupMotion::close(this, notificationMotionPlacement(m_placement), NotificationMotionDistance);
        }
    });

    QWidget* window = anchor->window();
    if (window && window != anchor)
    {
        m_anchorWindow = window;
        window->installEventFilter(this);
    }
}

void AntNotification::uninstallAnchorWatcher()
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

bool AntNotification::anchorReady() const
{
    return anchorCanHostNotification(m_anchor.data());
}

void AntNotification::handleAnchorChanged(QEvent::Type type)
{
    if (type == QEvent::Hide)
    {
        if (isVisible() && !AntPopupMotion::isClosing(this))
        {
            AntPopupMotion::close(this, notificationMotionPlacement(m_placement), NotificationMotionDistance);
        }
        return;
    }

    if (!anchorReady())
    {
        return;
    }

    relayoutNotifications(m_anchor.data());
}

QRectF AntNotification::noticeRect() const
{
    return notificationLayout().cardRect;
}

QRectF AntNotification::closeButtonRect() const
{
    return notificationLayout().closeRect;
}

QColor AntNotification::accentColor() const
{
    const auto& token = antTheme->tokens();
    switch (m_notificationType)
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

const AntNotification::NotificationLayout& AntNotification::notificationLayout() const
{
    const auto& token = antTheme->tokens();
    const QString fontKey = font().toString();
    const QColor accent = accentColor();
    const bool cacheMatches = m_layoutCache.valid
        && m_layoutCache.widgetSize == size()
        && m_layoutCache.title == m_title
        && m_layoutCache.description == m_description
        && m_layoutCache.notificationType == m_notificationType
        && m_layoutCache.themeMode == antTheme->themeMode()
        && m_layoutCache.fontKey == fontKey
        && m_layoutCache.closable == m_closable
        && m_layoutCache.iconVisible == m_iconVisible
        && m_layoutCache.showProgress == m_showProgress
        && m_layoutCache.progressMode == m_progressMode
        && m_layoutCache.duration == m_duration
        && m_layoutCache.accentColor == accent
        && m_layoutCache.radius == token.borderRadiusLG;
    if (cacheMatches)
    {
        ++m_layoutCacheHitCount;
        syncNotificationPerfCounters();
        return m_layoutCache;
    }

    ++m_layoutBuildCount;

    NotificationLayout layout;
    layout.valid = true;
    layout.widgetSize = size();
    layout.title = m_title;
    layout.description = m_description;
    layout.notificationType = m_notificationType;
    layout.themeMode = antTheme->themeMode();
    layout.fontKey = fontKey;
    layout.closable = m_closable;
    layout.iconVisible = m_iconVisible;
    layout.showProgress = m_showProgress;
    layout.progressMode = m_progressMode;
    layout.duration = m_duration;
    layout.radius = token.borderRadiusLG;
    layout.accentColor = accent;

    const int iconWidth = m_iconVisible ? 22 + token.marginSM : 0;
    const int contentWidth = NoticeWidth - token.paddingLG * 2 - iconWidth - (m_closable ? 28 : 0);

    QFont titleFont = font();
    titleFont.setPixelSize(token.fontSizeLG);
    titleFont.setWeight(QFont::DemiBold);
    QFont descFont = font();
    descFont.setPixelSize(token.fontSize);

    const int titleHeight = m_title.isEmpty() ? 0 : QFontMetrics(titleFont).height();
    const QRect descBounds = QFontMetrics(descFont).boundingRect(QRect(0, 0, contentWidth, 400),
                                                                 Qt::TextWordWrap,
                                                                 m_description);
    const int descHeight = m_description.isEmpty() ? 0 : descBounds.height();
    const int gap = (!m_title.isEmpty() && !m_description.isEmpty()) ? token.marginXS : 0;
    const bool progressTrackVisible = m_showProgress && (m_progressMode == ProgressMode::Manual || m_duration > 0);
    const int cardHeight =
        std::max(86, token.padding * 2 + titleHeight + gap + descHeight + (progressTrackVisible ? 2 : 0));
    layout.sizeHint = QSize(NoticeWidth + ShadowInset * 2, cardHeight + ShadowInset * 2);
    layout.minimumSizeHint = QSize(NoticeWidth + ShadowInset * 2, 96);

    QRect baseRect = rect();
    if (baseRect.width() <= 0 || baseRect.height() <= 0)
    {
        baseRect = QRect(QPoint(0, 0), layout.sizeHint);
    }
    layout.cardRect = baseRect.adjusted(ShadowInset, ShadowInset, -ShadowInset, -ShadowInset);

    layout.iconRect = QRectF(layout.cardRect.left() + token.paddingLG,
                             layout.cardRect.top() + token.padding + 2,
                             22,
                             22);
    const qreal closeSize = token.controlHeightLG * 0.55;
    layout.closeRect = QRectF(layout.cardRect.right() - token.paddingLG - closeSize,
                              layout.cardRect.top() + token.paddingMD - 4,
                              closeSize,
                              closeSize);

    const qreal textLeft = m_iconVisible ? layout.iconRect.right() + token.marginSM
                                         : layout.cardRect.left() + token.paddingLG;
    const qreal textRight = layout.cardRect.right() - token.paddingLG - (m_closable ? 28 : 0);
    qreal descTop = layout.cardRect.top() + token.padding - 1;
    if (!m_title.isEmpty())
    {
        layout.titleRect = QRectF(textLeft, descTop, textRight - textLeft, token.controlHeightSM);
        descTop = layout.titleRect.bottom() + token.marginXS - 2;
    }
    layout.descriptionRect =
        QRectF(textLeft, descTop, textRight - textLeft, layout.cardRect.bottom() - descTop - token.padding);

    if (progressTrackVisible)
    {
        layout.progressTrackRect =
            QRectF(layout.cardRect.left(), layout.cardRect.bottom() - 2, layout.cardRect.width(), 2);
    }

    m_layoutCache = layout;
    syncNotificationPerfCounters();
    return m_layoutCache;
}

void AntNotification::invalidateNotificationLayout() const
{
    m_layoutCache.valid = false;
}

QPixmap AntNotification::cachedShadowPixmap() const
{
    const auto& layout = notificationLayout();
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
        syncNotificationPerfCounters();
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
    drawNotificationShadow(painter, layout.cardRect, layout.radius);
    painter.end();

    m_shadowPixmapCache = pixmap;
    syncNotificationPerfCounters();
    return m_shadowPixmapCache;
}

void AntNotification::clearShadowCache() const
{
    m_shadowPixmapCache = QPixmap();
    m_shadowPixmapCacheKey.clear();
}

void AntNotification::applyNotificationSizeHint()
{
    const QSize target = sizeHint();
    if (size() == target)
    {
        ++m_sizeSkipCount;
        syncNotificationPerfCounters();
        return;
    }

    resize(target);
    invalidateNotificationLayout();
    clearShadowCache();
    ++m_sizeApplyCount;
    syncNotificationPerfCounters();
}

void AntNotification::requestNotificationUpdate(const QRect& region,
                                                const QString& mode,
                                                bool progressScoped,
                                                bool spinnerScoped,
                                                bool closeScoped)
{
    m_lastUpdateMode = mode;
    ++m_regionUpdateCount;
    if (progressScoped)
    {
        ++m_progressRegionUpdateCount;
    }
    if (spinnerScoped)
    {
        ++m_spinnerRegionUpdateCount;
    }
    if (closeScoped)
    {
        ++m_closeRegionUpdateCount;
    }
    syncNotificationPerfCounters();

    if (region.isValid() && !region.isEmpty())
    {
        update(region);
        return;
    }
    update();
}

void AntNotification::syncNotificationPerfCounters() const
{
    auto* that = const_cast<AntNotification*>(this);
    that->setProperty("antNotificationLayoutBuildCount", m_layoutBuildCount);
    that->setProperty("antNotificationLayoutCacheHitCount", m_layoutCacheHitCount);
    that->setProperty("antNotificationShadowBuildCount", m_shadowBuildCount);
    that->setProperty("antNotificationShadowCacheHitCount", m_shadowCacheHitCount);
    that->setProperty("antNotificationSizeApplyCount", m_sizeApplyCount);
    that->setProperty("antNotificationSizeSkipCount", m_sizeSkipCount);
    that->setProperty("antNotificationRegionUpdateCount", m_regionUpdateCount);
    that->setProperty("antNotificationProgressRegionUpdateCount", m_progressRegionUpdateCount);
    that->setProperty("antNotificationSpinnerRegionUpdateCount", m_spinnerRegionUpdateCount);
    that->setProperty("antNotificationCloseRegionUpdateCount", m_closeRegionUpdateCount);
    that->setProperty("antNotificationRelayoutMoveCount", m_relayoutMoveCount);
    that->setProperty("antNotificationRelayoutSkipCount", m_relayoutSkipCount);
    that->setProperty("antNotificationLastUpdateMode", m_lastUpdateMode);
}

void AntNotification::drawLoadingIcon(QPainter& painter, const QRectF& rect) const
{
    painter.save();
    painter.setPen(QPen(accentColor(), 2.0, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(rect.adjusted(2, 2, -2, -2), m_spinnerAngle * 16, 270 * 16);
    painter.restore();
}

qreal AntNotification::progressRatio() const
{
    if (m_progressMode == ProgressMode::Manual)
    {
        return static_cast<qreal>(m_progress) / 100.0;
    }

    if (m_duration <= 0)
    {
        return 0.0;
    }
    int remaining = m_remainingMs;
    if (m_closeTimer->isActive() && m_countdown.isValid())
    {
        remaining = std::max(0, m_remainingMs - static_cast<int>(m_countdown.elapsed()));
    }
    return std::clamp(static_cast<qreal>(remaining) / static_cast<qreal>(m_duration), 0.0, 1.0);
}

void AntNotification::startCloseTimer()
{
    if (m_duration > 0 && isVisible() && !m_hovered)
    {
        m_remainingMs = m_duration;
        m_countdown.restart();
        m_closeTimer->start(m_remainingMs);
        if (m_showProgress && m_progressMode == ProgressMode::Countdown)
        {
            m_progressTimer->start(50);
        }
    }
    else
    {
        m_closeTimer->stop();
        m_progressTimer->stop();
    }
}

void AntNotification::pauseCloseTimer()
{
    if (m_closeTimer->isActive() && m_countdown.isValid())
    {
        m_remainingMs = std::max(0, m_remainingMs - static_cast<int>(m_countdown.elapsed()));
        m_closeTimer->stop();
        m_progressTimer->stop();
        if (m_showProgress && m_progressMode == ProgressMode::Countdown)
        {
            requestNotificationUpdate(notificationLayout().progressTrackRect.toAlignedRect().adjusted(-1, -1, 1, 1),
                                      QStringLiteral("progress"),
                                      true);
        }
    }
}

void AntNotification::resumeCloseTimer()
{
    if (m_duration > 0 && isVisible() && m_remainingMs > 0)
    {
        m_countdown.restart();
        m_closeTimer->start(m_remainingMs);
        if (m_showProgress && m_progressMode == ProgressMode::Countdown)
        {
            m_progressTimer->start(50);
        }
    }
}

void AntNotification::updateSpinnerState()
{
    if (m_iconVisible && m_notificationType == Ant::MessageType::Loading && isVisible())
    {
        m_spinnerTimer->start(80);
    }
    else
    {
        m_spinnerTimer->stop();
    }
}
