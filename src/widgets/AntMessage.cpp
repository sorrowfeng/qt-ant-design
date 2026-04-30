#include "AntMessage.h"

#include <QApplication>
#include <QEnterEvent>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
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

AntPopupMotion::Placement messageMotionPlacement(Ant::Placement placement)
{
    return isBottomMessagePlacement(placement)
               ? AntPopupMotion::Placement::Top
               : AntPopupMotion::Placement::Bottom;
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
        update();
    });
}

AntMessage* AntMessage::open(const QString& text, Ant::MessageType type, QWidget* anchor, int durationMs, Ant::Placement placement)
{
    auto* message = new AntMessage();
    message->m_anchor = anchor;
    message->m_placement = placement;
    message->setText(text);
    message->setMessageType(type);
    message->setDuration(durationMs);
    message->adjustSize();
    activeMessages().append(message);

    QObject::connect(message, &QObject::destroyed, message, [message, anchor]() {
        activeMessages().removeAll(message);
        relayoutMessages(anchor);
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
    adjustSize();
    update();
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
    updateLoadingState();
    update();
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
    const auto& token = antTheme->tokens();
    QFont f = font();
    f.setPixelSize(token.fontSize);
    QFontMetrics fm(f);
    const int textWidth = std::min(420, fm.horizontalAdvance(m_text));
    return QSize(textWidth + 64, token.controlHeightLG + 12);
}

QSize AntMessage::minimumSizeHint() const
{
    return QSize(120, antTheme->tokens().controlHeightLG);
}

void AntMessage::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
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

void AntMessage::enterEvent(QEnterEvent* event)
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

            message->adjustSize();
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
                message->move(x, cursor);
                cursor -= 4;
            }
            else
            {
                message->move(x, cursor);
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
