#include "AntNotification.h"

#include <QApplication>
#include <QEnterEvent>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
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
constexpr int ShadowInset = 10;

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
} // namespace

AntNotification::AntNotification(QWidget* parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
{
    setStyle(new AntNotificationStyle(style()));
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setMouseTracking(true);

    m_closeTimer = new QTimer(this);
    m_closeTimer->setSingleShot(true);
    connect(m_closeTimer, &QTimer::timeout, this, [this]() {
        AntPopupMotion::close(this, AntPopupMotion::fromPlacement(m_placement));
    });

    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, [this]() { update(); });

    m_spinnerTimer = new QTimer(this);
    connect(m_spinnerTimer, &QTimer::timeout, this, [this]() {
        m_spinnerAngle = (m_spinnerAngle + 30) % 360;
        update();
    });
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
    notification->m_anchor = anchor;
    notification->setTitle(title);
    notification->setDescription(description);
    notification->setNotificationType(type);
    notification->setIconVisible(iconVisible);
    notification->setPlacement(placement);
    notification->setDuration(durationMs);
    notification->adjustSize();
    activeNotifications().append(notification);

    QObject::connect(notification, &QObject::destroyed, [notification, anchor]() {
        activeNotifications().removeAll(notification);
        relayoutNotifications(anchor);
    });

    relayoutNotifications(anchor);
    AntPopupMotion::show(notification, AntPopupMotion::fromPlacement(placement));
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

void AntNotification::closeAll()
{
    const auto notifications = activeNotifications();
    for (AntNotification* notification : notifications)
    {
        if (notification)
        {
            AntPopupMotion::close(notification, AntPopupMotion::fromPlacement(notification->placement()));
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
    adjustSize();
    update();
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
    adjustSize();
    update();
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
    updateSpinnerState();
    update();
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
    relayoutNotifications(m_anchor);
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
    if (m_showProgress && m_duration > 0 && isVisible())
    {
        m_progressTimer->start(50);
    }
    else
    {
        m_progressTimer->stop();
    }
    update();
    Q_EMIT showProgressChanged(m_showProgress);
}

bool AntNotification::isClosable() const { return m_closable; }

void AntNotification::setClosable(bool closable)
{
    if (m_closable == closable)
    {
        return;
    }
    m_closable = closable;
    update();
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
    updateSpinnerState();
    updateGeometry();
    update();
    Q_EMIT iconVisibleChanged(m_iconVisible);
}

int AntNotification::spinnerAngle() const { return m_spinnerAngle; }

QSize AntNotification::sizeHint() const
{
    const auto& token = antTheme->tokens();
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
    const int cardHeight = std::max(86, token.padding * 2 + titleHeight + gap + descHeight + (m_showProgress ? 2 : 0));
    return QSize(NoticeWidth + ShadowInset * 2, cardHeight + ShadowInset * 2);
}

QSize AntNotification::minimumSizeHint() const
{
    return QSize(NoticeWidth + ShadowInset * 2, 96);
}

void AntNotification::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntNotification::showEvent(QShowEvent* event)
{
    m_remainingMs = m_duration;
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

void AntNotification::enterEvent(QEnterEvent* event)
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
    m_closeHovered = false;
    if (m_pauseOnHover)
    {
        resumeCloseTimer();
    }
    update();
    QWidget::leaveEvent(event);
}

void AntNotification::mouseMoveEvent(QMouseEvent* event)
{
    const bool hoveringClose = m_closable && closeButtonRect().contains(event->position());
    if (m_closeHovered != hoveringClose)
    {
        m_closeHovered = hoveringClose;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void AntNotification::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (m_closable && closeButtonRect().contains(event->position()))
        {
            AntPopupMotion::close(this, AntPopupMotion::fromPlacement(m_placement));
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
            notification->adjustSize();
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
            notification->move(x, y);
        }
    }
}

QRectF AntNotification::noticeRect() const
{
    return rect().adjusted(ShadowInset, ShadowInset, -ShadowInset, -ShadowInset);
}

QRectF AntNotification::closeButtonRect() const
{
    const auto& token = antTheme->tokens();
    const QRectF card = noticeRect();
    const qreal size = token.controlHeightLG * 0.55;
    return QRectF(card.right() - token.paddingLG - size, card.top() + token.paddingMD - 4, size, size);
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

qreal AntNotification::progressRatio() const
{
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
        if (m_showProgress)
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
        update();
    }
}

void AntNotification::resumeCloseTimer()
{
    if (m_duration > 0 && isVisible() && m_remainingMs > 0)
    {
        m_countdown.restart();
        m_closeTimer->start(m_remainingMs);
        if (m_showProgress)
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
