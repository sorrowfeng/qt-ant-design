#include "AntMessageStyle.h"

#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "core/AntTheme.h"
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

QString computeIconText(Ant::MessageType type)
{
    switch (type)
    {
    case Ant::MessageType::Success:
        return QStringLiteral("✓");
    case Ant::MessageType::Warning:
        return QStringLiteral("!");
    case Ant::MessageType::Error:
        return QStringLiteral("×");
    case Ant::MessageType::Info:
    default:
        return QStringLiteral("i");
    }
}

void drawLoadingIcon(QPainter& painter, const QRectF& rect, const QColor& color)
{
    painter.save();
    painter.setPen(QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(rect.adjusted(1, 1, -1, -1), 0, 270 * 16);
    painter.restore();
}
} // namespace

AntMessageStyle::AntMessageStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntMessage*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->updateGeometry();
                w->update();
            }
        }
    });
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
    antTheme->drawEffectShadow(painter, bubble.toRect(), 10, token.borderRadiusLG, 0.55);

    // Bubble
    painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
    painter->setBrush(token.colorBgElevated);
    painter->drawRoundedRect(bubble, token.borderRadiusLG, token.borderRadiusLG);

    // Icon
    const QRectF iconRect(bubble.left() + token.paddingSM, bubble.center().y() - 8, 16, 16);
    if (messageType == Ant::MessageType::Loading)
    {
        drawLoadingIcon(*painter, iconRect, accent);
    }
    else
    {
        QFont iconFont = painter->font();
        iconFont.setPixelSize(14);
        iconFont.setWeight(QFont::DemiBold);
        painter->setFont(iconFont);
        painter->setPen(accent);
        painter->drawText(iconRect, Qt::AlignCenter, computeIconText(messageType));
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
