#include "AntNotificationStyle.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QTextOption>

#include "styles/AntIconPainter.h"
#include "styles/AntPalette.h"
#include "widgets/AntNotification.h"

namespace
{
constexpr int NoticeWidth = 384;
constexpr int ShadowInset = 18;

QRectF computeNoticeRect(const QRect& rect)
{
    return rect.adjusted(ShadowInset, ShadowInset, -ShadowInset, -ShadowInset);
}

QColor computeAccentColor(Ant::MessageType type)
{
    const auto& token = antTheme->tokens();
    switch (type)
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

QRectF computeCloseButtonRect(const QRectF& card)
{
    const auto& token = antTheme->tokens();
    const qreal size = token.controlHeightLG * 0.55;
    return QRectF(card.right() - token.paddingLG - size, card.top() + token.paddingMD - 4, size, size);
}

void drawShadowLayer(QPainter& painter, const QRectF& card, int blur, qreal yOffset, qreal alpha, qreal radius)
{
    QColor shadow = antTheme->tokens().colorShadow;
    for (int i = blur; i >= 1; --i)
    {
        const qreal progress = 1.0 - static_cast<qreal>(i) / static_cast<qreal>(blur);
        shadow.setAlphaF(alpha * progress * progress);
        painter.setPen(QPen(shadow, 1));
        painter.setBrush(Qt::NoBrush);
        const QRectF layer = card.adjusted(-i, -i + yOffset, i, i + yOffset);
        painter.drawRoundedRect(layer, radius + i, radius + i);
    }
}

void drawNotificationShadow(QPainter& painter, const QRectF& card, qreal radius)
{
    const bool dark = antTheme->themeMode() == Ant::ThemeMode::Dark;
    drawShadowLayer(painter, card, 18, 6, dark ? 0.30 : 0.16, radius);
    drawShadowLayer(painter, card, 10, 2, dark ? 0.22 : 0.10, radius);
    drawShadowLayer(painter, card, 4, 0, dark ? 0.18 : 0.08, radius);
}

void drawTypeIcon(QPainter& painter, const QRectF& rect, Ant::MessageType type, const QColor& accentColor, int spinnerAngle)
{
    if (type == Ant::MessageType::Loading)
    {
        painter.save();
        painter.setPen(QPen(accentColor, 2.0, Qt::SolidLine, Qt::RoundCap));
        painter.setBrush(Qt::NoBrush);
        painter.drawArc(rect.adjusted(2, 2, -2, -2), spinnerAngle * 16, 270 * 16);
        painter.restore();
        return;
    }

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
    connectThemeUpdate<AntNotification>();
}

void AntNotificationStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
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
    QProxyStyle::unpolish(widget);
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
        QStyleOption option;
        option.initFrom(notification);
        option.rect = notification->rect();
        QPainter painter(notification);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, notification);
        return true;
    }

    // Track close button hover state
    if (event->type() == QEvent::MouseMove && notification->isClosable())
    {
        auto* me = static_cast<QMouseEvent*>(event);
        const QRectF card = computeNoticeRect(notification->rect());
        const QRectF closeRect = computeCloseButtonRect(card);
        const bool hover = closeRect.contains(me->position());
        const bool wasHover = notification->property("antStyle_closeHover").toBool();
        if (hover != wasHover)
        {
            notification->setProperty("antStyle_closeHover", hover);
            notification->update();
        }
    }

    if (event->type() == QEvent::Leave)
    {
        if (notification->property("antStyle_closeHover").toBool())
        {
            notification->setProperty("antStyle_closeHover", false);
            notification->update();
        }
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
    const Ant::MessageType notificationType = notification->notificationType();
    const bool closable = notification->isClosable();
    const bool iconVisible = notification->iconVisible();
    const bool showProgress = notification->showProgress();
    const bool closeHovered = notification->property("antStyle_closeHover").toBool();
    const QColor accent = computeAccentColor(notificationType);

    const QRectF card = computeNoticeRect(option->rect);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // Shadow
    drawNotificationShadow(*painter, card, token.borderRadiusLG);

    // Card background
    AntStyleBase::drawCrispRoundedRect(painter, card.toRect(),
        QPen(token.colorBorderSecondary, token.lineWidth), token.colorBgElevated,
        token.borderRadiusLG, token.borderRadiusLG);

    const QRectF iconRect(card.left() + token.paddingLG, card.top() + token.padding + 2, 22, 22);
    if (iconVisible)
    {
        drawTypeIcon(*painter, iconRect, notificationType, accent, notification->spinnerAngle());
    }

    // Close button
    if (closable)
    {
        const QRectF closeRect = computeCloseButtonRect(card);
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
    const qreal textLeft = iconVisible ? iconRect.right() + token.marginSM : card.left() + token.paddingLG;
    const qreal textRight = card.right() - token.paddingLG - (closable ? 28 : 0);
    QRectF titleRect(textLeft, card.top() + token.padding - 1, textRight - textLeft, token.controlHeightSM);

    if (!notification->title().isEmpty())
    {
        QFont titleFont = painter->font();
        titleFont.setPixelSize(token.fontSizeLG);
        titleFont.setWeight(QFont::DemiBold);
        painter->setFont(titleFont);
        painter->setPen(token.colorText);
        painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, notification->title());
        titleRect.moveTop(titleRect.bottom() + token.marginXS - 2);
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
    const QRectF descRect(textLeft, titleRect.top(), textRight - textLeft, card.bottom() - titleRect.top() - token.padding);
    painter->drawText(descRect, notification->description(), textOption);

    // Progress bar
    if (showProgress && notification->duration() > 0)
    {
        const QRectF track(card.left(), card.bottom() - 2, card.width(), 2);
        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorFillQuaternary);
        painter->drawRect(track);
        painter->setBrush(accent);
        painter->drawRect(QRectF(track.left(), track.top(), track.width() * notification->progressRatio(), track.height()));
    }

    painter->restore();
}
