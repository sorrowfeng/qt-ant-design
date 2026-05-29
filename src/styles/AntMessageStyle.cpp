#include "AntMessageStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntIconPainter.h"
#include "widgets/AntMessage.h"

namespace
{
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
}

void AntMessageStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
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
    AntStyleBase::unpolish(widget);
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
    Q_UNUSED(option)
    const auto& layout = message->messageLayout();
    const QRectF bubble = layout.bubbleRect;
    const Ant::MessageType messageType = message->messageType();
    const QColor accent = layout.accentColor;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // Shadow
    painter->drawPixmap(QPoint(0, 0), message->cachedShadowPixmap());

    // Bubble
    AntStyleBase::drawCrispRoundedRect(painter, bubble.toRect(),
        QPen(token.colorBorderSecondary, token.lineWidth),
        token.colorBgElevated,
        layout.radius,
        layout.radius);

    // Icon
    if (messageType == Ant::MessageType::Loading)
    {
        message->drawLoadingIcon(*painter, layout.iconRect);
    }
    else
    {
        drawStatusIcon(*painter, layout.iconRect, accent, messageType);
    }

    // Text
    QFont textFont = painter->font();
    textFont.setPixelSize(token.fontSize);
    textFont.setWeight(QFont::Normal);
    painter->setFont(textFont);
    painter->setPen(token.colorText);
    painter->drawText(layout.textRect, Qt::AlignVCenter | Qt::AlignLeft, message->text());

    painter->restore();
}
