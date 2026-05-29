#include "AntNotificationStyle.h"

#include <QEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>
#include <QTextOption>

#include "styles/AntIconPainter.h"
#include "widgets/AntNotification.h"

namespace
{
void drawTypeIcon(QPainter& painter, const QRectF& rect, Ant::MessageType type, const QColor& accentColor)
{
    Ant::IconType iconType = Ant::IconType::InfoCircle;
    switch (type)
    {
    case Ant::MessageType::Success:
        iconType = Ant::IconType::CheckCircle;
        break;
    case Ant::MessageType::Warning:
        iconType = Ant::IconType::ExclamationCircle;
        break;
    case Ant::MessageType::Error:
        iconType = Ant::IconType::CloseCircle;
        break;
    case Ant::MessageType::Info:
    default:
        break;
    }
    AntIconPainter::drawIcon(painter,
                             iconType,
                             rect,
                             accentColor,
                             Ant::IconTheme::Filled,
                             antTheme->tokens().colorTextLightSolid);
}
} // namespace

AntNotificationStyle::AntNotificationStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntNotificationStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    if (qobject_cast<AntNotification*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntNotificationStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntNotification*>(widget))
    {
        widget->removeEventFilter(this);
    }
    AntStyleBase::unpolish(widget);
}

void AntNotificationStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntNotification*>(widget))
    {
        drawNotification(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntNotificationStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntNotificationStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* notification = qobject_cast<AntNotification*>(watched);
    if (!notification)
    {
        return QProxyStyle::eventFilter(watched, event);
    }

    if (event->type() == QEvent::Paint)
    {
        auto* paintEvent = static_cast<QPaintEvent*>(event);
        QStyleOption option;
        option.initFrom(notification);
        option.rect = paintEvent->rect();
        QPainter painter(notification);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, notification);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntNotificationStyle::drawNotification(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* notification = qobject_cast<const AntNotification*>(widget);
    if (!notification || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    Q_UNUSED(option)
    const auto& layout = notification->notificationLayout();
    const Ant::MessageType notificationType = notification->notificationType();
    const bool closable = notification->isClosable();
    const bool iconVisible = notification->iconVisible();
    const bool showProgress = notification->showProgress();
    const bool closeHovered = notification->m_closeHovered;
    const QColor accent = layout.accentColor;
    const QRectF card = layout.cardRect;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // Shadow
    painter->drawPixmap(QPoint(0, 0), notification->cachedShadowPixmap());

    // Card background
    AntStyleBase::drawCrispRoundedRect(painter, card.toRect(),
        QPen(token.colorBorderSecondary, token.lineWidth),
        token.colorBgElevated,
        layout.radius,
        layout.radius);

    if (iconVisible)
    {
        if (notificationType == Ant::MessageType::Loading)
        {
            notification->drawLoadingIcon(*painter, layout.iconRect);
        }
        else
        {
            drawTypeIcon(*painter, layout.iconRect, notificationType, accent);
        }
    }

    // Close button
    if (closable)
    {
        const QRectF closeRect = layout.closeRect;
        AntStyleBase::drawCrispRoundedRect(painter, closeRect.toRect(),
            Qt::NoPen, closeHovered ? token.colorFillTertiary : Qt::transparent,
            token.borderRadiusSM, token.borderRadiusSM);
        painter->setPen(QPen(closeHovered ? token.colorTextSecondary : token.colorTextTertiary,
                             1.6, Qt::SolidLine, Qt::RoundCap));
        const QPointF c = closeRect.center();
        painter->drawLine(QPointF(c.x() - 4, c.y() - 4), QPointF(c.x() + 4, c.y() + 4));
        painter->drawLine(QPointF(c.x() + 4, c.y() - 4), QPointF(c.x() - 4, c.y() + 4));
    }

    // Title
    if (!notification->title().isEmpty())
    {
        QFont titleFont = painter->font();
        titleFont.setPixelSize(token.fontSizeLG);
        titleFont.setWeight(QFont::DemiBold);
        painter->setFont(titleFont);
        painter->setPen(token.colorText);
        painter->drawText(layout.titleRect, Qt::AlignLeft | Qt::AlignVCenter, notification->title());
    }

    // Description
    QFont descFont = painter->font();
    descFont.setPixelSize(token.fontSize);
    descFont.setWeight(QFont::Normal);
    painter->setFont(descFont);
    painter->setPen(token.colorTextSecondary);
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::WordWrap);
    textOption.setAlignment(Qt::AlignLeft | Qt::AlignTop);
    painter->drawText(layout.descriptionRect, notification->description(), textOption);

    // Progress bar
    if (showProgress && notification->duration() > 0)
    {
        const QRectF track = layout.progressTrackRect;
        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorFillQuaternary);
        painter->drawRect(track);
        painter->setBrush(accent);
        painter->drawRect(QRectF(track.left(), track.top(), track.width() * notification->progressRatio(), track.height()));
    }

    painter->restore();
}
