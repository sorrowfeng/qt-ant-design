#include "AntNotificationStyle.h"

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QTextOption>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "widgets/AntNotification.h"

namespace
{
constexpr int NoticeWidth = 384;
constexpr int ShadowInset = 10;

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

void drawTypeIcon(QPainter& painter, const QRectF& rect, Ant::MessageType type, const QColor& accentColor)
{
    if (type == Ant::MessageType::Loading)
    {
        // Spinner at angle 0
        painter.save();
        painter.setPen(QPen(accentColor, 2.0, Qt::SolidLine, Qt::RoundCap));
        painter.setBrush(Qt::NoBrush);
        painter.drawArc(rect.adjusted(2, 2, -2, -2), 0, 270 * 16);
        painter.restore();
        return;
    }

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(accentColor);

    if (type == Ant::MessageType::Warning)
    {
        QPainterPath triangle;
        triangle.moveTo(rect.center().x(), rect.top() + 1);
        triangle.lineTo(rect.right() - 1, rect.bottom() - 1);
        triangle.lineTo(rect.left() + 1, rect.bottom() - 1);
        triangle.closeSubpath();
        painter.drawPath(triangle);
        painter.setPen(QPen(antTheme->tokens().colorTextLightSolid, 1.6, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(rect.center().x(), rect.top() + 7), QPointF(rect.center().x(), rect.bottom() - 7));
        painter.drawPoint(QPointF(rect.center().x(), rect.bottom() - 4));
    }
    else
    {
        painter.drawEllipse(rect);
        painter.setPen(QPen(antTheme->tokens().colorTextLightSolid, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        const QPointF c = rect.center();
        if (type == Ant::MessageType::Success)
        {
            painter.drawLine(QPointF(c.x() - 5, c.y()), QPointF(c.x() - 1, c.y() + 4));
            painter.drawLine(QPointF(c.x() - 1, c.y() + 4), QPointF(c.x() + 6, c.y() - 5));
        }
        else if (type == Ant::MessageType::Error)
        {
            painter.drawLine(QPointF(c.x() - 5, c.y() - 5), QPointF(c.x() + 5, c.y() + 5));
            painter.drawLine(QPointF(c.x() + 5, c.y() - 5), QPointF(c.x() - 5, c.y() + 5));
        }
        else
        {
            QFont infoFont = painter.font();
            infoFont.setPixelSize(14);
            infoFont.setWeight(QFont::DemiBold);
            painter.setFont(infoFont);
            painter.drawText(rect, Qt::AlignCenter, QStringLiteral("i"));
        }
    }
    painter.restore();
}
} // namespace

AntNotificationStyle::AntNotificationStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntNotification*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->updateGeometry();
                w->update();
            }
        }
    });
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
    const bool showProgress = notification->showProgress();
    const bool closeHovered = notification->property("antStyle_closeHover").toBool();
    const QColor accent = computeAccentColor(notificationType);

    const QRectF card = computeNoticeRect(option->rect);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // Shadow
    antTheme->drawEffectShadow(painter, card.toRect(), 14, token.borderRadiusLG, 0.7);

    // Card background
    painter->setPen(Qt::NoPen);
    painter->setBrush(token.colorBgElevated);
    painter->drawRoundedRect(card, token.borderRadiusLG, token.borderRadiusLG);

    // Type icon
    const QRectF iconRect(card.left() + token.paddingLG, card.top() + token.padding + 2, 22, 22);
    drawTypeIcon(*painter, iconRect, notificationType, accent);

    // Close button
    if (closable)
    {
        const QRectF closeRect = computeCloseButtonRect(card);
        painter->setPen(Qt::NoPen);
        painter->setBrush(closeHovered ? token.colorFillTertiary : Qt::transparent);
        painter->drawRoundedRect(closeRect, token.borderRadiusSM, token.borderRadiusSM);
        painter->setPen(QPen(closeHovered ? token.colorTextSecondary : token.colorTextTertiary,
                             1.6, Qt::SolidLine, Qt::RoundCap));
        const QPointF c = closeRect.center();
        painter->drawLine(QPointF(c.x() - 4, c.y() - 4), QPointF(c.x() + 4, c.y() + 4));
        painter->drawLine(QPointF(c.x() + 4, c.y() - 4), QPointF(c.x() - 4, c.y() + 4));
    }

    // Title
    const qreal textLeft = iconRect.right() + token.marginSM;
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
        painter->drawRect(QRectF(track.left(), track.top(), track.width(), track.height()));
    }

    painter->restore();
}
