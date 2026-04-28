#include "AntMessageStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntMessage.h"

namespace
{
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

void drawLoadingIcon(QPainter& painter, const QRectF& rect, const QColor& color, int angle)
{
    painter.save();
    painter.setPen(QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(rect.adjusted(1, 1, -1, -1), angle * 16, 270 * 16);
    painter.restore();
}

void drawMessageShadow(QPainter& painter, const QRectF& bubble, qreal radius)
{
    painter.save();
    painter.setPen(Qt::NoPen);

    for (int i = 12; i >= 1; --i)
    {
        const qreal progress = static_cast<qreal>(i) / 12.0;
        const qreal alpha = 0.003 + 0.008 * (1.0 - progress);
        const QRectF layer = bubble.adjusted(-i * 0.42, i * 0.12, i * 0.42, i * 0.62 + 2.0);
        painter.setBrush(AntPalette::alpha(Qt::black, alpha));
        painter.drawRoundedRect(layer, radius + i * 0.25, radius + i * 0.25);
    }

    painter.restore();
}

void drawStatusIcon(QPainter& painter, const QRectF& rect, const QColor& color, Ant::MessageType type)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(rect.adjusted(1, 1, -1, -1));

    const QPointF center = rect.center();
    painter.setPen(QPen(Qt::white, 1.7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    switch (type)
    {
    case Ant::MessageType::Success:
        painter.drawLine(QPointF(center.x() - 4.2, center.y() + 0.1), QPointF(center.x() - 1.2, center.y() + 3.1));
        painter.drawLine(QPointF(center.x() - 1.2, center.y() + 3.1), QPointF(center.x() + 4.6, center.y() - 4.0));
        break;
    case Ant::MessageType::Error:
        painter.drawLine(QPointF(center.x() - 3.4, center.y() - 3.4), QPointF(center.x() + 3.4, center.y() + 3.4));
        painter.drawLine(QPointF(center.x() + 3.4, center.y() - 3.4), QPointF(center.x() - 3.4, center.y() + 3.4));
        break;
    case Ant::MessageType::Warning:
    case Ant::MessageType::Info:
    default:
    {
        QFont iconFont = painter.font();
        iconFont.setPixelSize(type == Ant::MessageType::Info ? 11 : 12);
        iconFont.setWeight(QFont::DemiBold);
        painter.setFont(iconFont);
        painter.setPen(Qt::white);
        painter.drawText(rect, Qt::AlignCenter, type == Ant::MessageType::Info ? QStringLiteral("i") : QStringLiteral("!"));
        break;
    }
    }

    painter.restore();
}
} // namespace

AntMessageStyle::AntMessageStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntMessage>();
}

void AntMessageStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntMessage*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntMessageStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntMessage*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntMessageStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntMessage*>(widget))
    {
        drawMessage(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntMessageStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntMessageStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* message = qobject_cast<AntMessage*>(watched);
    if (message && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(message);
        option.rect = message->rect();
        QPainter painter(message);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, message);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntMessageStyle::drawMessage(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* message = qobject_cast<const AntMessage*>(widget);
    if (!message || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const QRectF bubble = option->rect.adjusted(8, 4, -8, -8);
    const Ant::MessageType messageType = message->messageType();
    const QColor accent = computeAccentColor(messageType);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // Shadow
    drawMessageShadow(*painter, bubble, token.borderRadiusLG);

    // Bubble
    AntStyleBase::drawCrispRoundedRect(painter, bubble.toRect(),
        Qt::NoPen,
        token.colorBgElevated, token.borderRadiusLG, token.borderRadiusLG);

    // Icon
    const QRectF iconRect(bubble.left() + token.paddingSM, bubble.center().y() - 8, 16, 16);
    if (messageType == Ant::MessageType::Loading)
    {
        drawLoadingIcon(*painter, iconRect, accent, message->loadingAngle());
    }
    else
    {
        drawStatusIcon(*painter, iconRect, accent, messageType);
    }

    // Text
    QFont textFont = painter->font();
    textFont.setPixelSize(token.fontSize);
    textFont.setWeight(QFont::Normal);
    painter->setFont(textFont);
    painter->setPen(token.colorText);
    painter->drawText(bubble.adjusted(token.paddingSM + 24, 0, -token.paddingSM, 0),
                      Qt::AlignVCenter | Qt::AlignLeft,
                      message->text());

    painter->restore();
}
