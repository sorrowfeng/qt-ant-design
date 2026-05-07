#include "AntMessageStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntIconPainter.h"
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

void drawStatusIcon(QPainter& painter, const QRectF& rect, const QColor& color, Ant::MessageType type)
{
    Ant::IconType iconType = Ant::IconType::InfoCircle;
    switch (type)
    {
    case Ant::MessageType::Success:
        iconType = Ant::IconType::CheckCircle;
        break;
    case Ant::MessageType::Error:
        iconType = Ant::IconType::CloseCircle;
        break;
    case Ant::MessageType::Warning:
        iconType = Ant::IconType::ExclamationCircle;
        break;
    case Ant::MessageType::Info:
    default:
        break;
    }
    AntIconPainter::drawIcon(painter,
                             iconType,
                             rect,
                             color,
                             Ant::IconTheme::Filled,
                             antTheme->tokens().colorTextLightSolid);
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
        QPen(token.colorBorderSecondary, token.lineWidth),
        token.colorBgElevated,
        token.borderRadiusLG,
        token.borderRadiusLG);

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
