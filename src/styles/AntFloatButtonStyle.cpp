#include "AntFloatButtonStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntFloatButton.h"

namespace
{

int floatButtonSize(const AntFloatButton* fb)
{
    return fb->floatButtonShape() == Ant::FloatButtonShape::Square ? fb->width() : 40;
}

int floatButtonRadius(const AntFloatButton* fb)
{
    const auto& token = antTheme->tokens();
    if (fb->floatButtonShape() == Ant::FloatButtonShape::Circle) return 20;
    return token.borderRadius;
}

QColor floatButtonBgColor(const AntFloatButton* fb, bool hovered, bool pressed)
{
    const auto& token = antTheme->tokens();
    if (fb->floatButtonType() == Ant::FloatButtonType::Primary)
    {
        if (pressed) return token.colorPrimaryActive;
        if (hovered) return token.colorPrimaryHover;
        return token.colorPrimary;
    }
    if (pressed) return token.colorFillTertiary;
    if (hovered) return token.colorBgElevated;
    return token.colorBgContainer;
}

QColor floatButtonIconColor(const AntFloatButton* fb)
{
    const auto& token = antTheme->tokens();
    if (fb->floatButtonType() == Ant::FloatButtonType::Primary)
        return token.colorTextLightSolid;
    return token.colorText;
}

void drawBadgeIndicator(QPainter* painter, const AntFloatButton* fb, const QRectF& buttonRect)
{
    const auto& token = antTheme->tokens();
    if (fb->badgeDot())
    {
        const qreal dotSize = 8;
        const QPointF topRight = buttonRect.topRight() + QPointF(-dotSize / 2, dotSize / 2);
        painter->setPen(QPen(token.colorBgContainer, 2));
        painter->setBrush(token.colorError);
        painter->drawEllipse(QRectF(topRight.x() - dotSize / 2, topRight.y() - dotSize / 2, dotSize, dotSize));
    }
    else if (fb->badgeCount() > 0)
    {
        const QString text = fb->badgeCount() > 99 ? QStringLiteral("99+") : QString::number(fb->badgeCount());
        QFont f = painter->font();
        f.setPixelSize(10);
        f.setWeight(QFont::Bold);
        QFontMetrics fm(f);
        const int textW = fm.horizontalAdvance(text);
        const int badgeW = qMax(16, textW + 8);
        const int badgeH = 16;
        const QPointF topRight = buttonRect.topRight();
        const QRectF badgeRect(topRight.x() - badgeW + 4, topRight.y() - 4, badgeW, badgeH);

        AntStyleBase::drawCrispRoundedRect(painter, badgeRect.toRect(),
            QPen(token.colorBgContainer, 2), token.colorError, badgeH / 2, badgeH / 2);

        painter->setFont(f);
        painter->setPen(token.colorTextLightSolid);
        painter->drawText(badgeRect, Qt::AlignCenter, text);
    }
}

} // namespace

AntFloatButtonStyle::AntFloatButtonStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntFloatButton>();
}

void AntFloatButtonStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntFloatButton*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntFloatButtonStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntFloatButton*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntFloatButtonStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntFloatButton*>(widget))
    {
        drawFloatButton(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntFloatButtonStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntFloatButtonStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* fb = qobject_cast<AntFloatButton*>(watched);
    if (fb && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(fb);
        option.rect = fb->rect();
        QPainter painter(fb);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, fb);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntFloatButtonStyle::drawFloatButton(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* fb = qobject_cast<const AntFloatButton*>(widget);
    if (!fb || !painter || !option) return;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    drawMainButton(option, painter, fb);

    painter->restore();
}

void AntFloatButtonStyle::drawMainButton(const QStyleOption* option, QPainter* painter, const AntFloatButton* fb) const
{
    const auto& token = antTheme->tokens();
    const QRectF r = option->rect;
    const int radius = floatButtonRadius(fb);
    const bool hovered = option->state & QStyle::State_MouseOver;

    // Shadow
    antTheme->drawEffectShadow(painter, r.toRect(), 12, radius, hovered ? 0.22 : 0.12);

    // Background
    QColor bg = floatButtonBgColor(fb, hovered, false);
    if (fb->floatButtonShape() == Ant::FloatButtonShape::Circle)
    {
        painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter->setBrush(bg);
        painter->drawEllipse(r.adjusted(0.5, 0.5, -0.5, -0.5));
    }
    else
    {
        AntStyleBase::drawCrispRoundedRect(painter, r.toRect(),
            QPen(token.colorBorderSecondary, token.lineWidth), bg, radius, radius);
    }

    // Badge
    if (fb->badgeDot() || fb->badgeCount() > 0)
    {
        drawBadgeIndicator(painter, fb, r);
    }

    // Icon
    QColor iconColor = floatButtonIconColor(fb);
    painter->setPen(iconColor);

    QString icon = fb->icon();
    if (icon.isEmpty() && fb->isBackTop())
    {
        icon = QStringLiteral("▲");
    }
    if (fb->isOpen() && !fb->closeIcon().isEmpty())
    {
        icon = fb->closeIcon();
    }

    if (!icon.isEmpty())
    {
        QFont f = painter->font();
        f.setPixelSize(18);
        painter->setFont(f);
        painter->drawText(r, Qt::AlignCenter, icon.left(2));
    }

    // Content text (square shape)
    if (!fb->content().isEmpty())
    {
        QFont f = painter->font();
        f.setPixelSize(token.fontSizeSM);
        painter->setFont(f);
        const int iconW = icon.isEmpty() ? 0 : 22;
        QRectF textRect = r.adjusted(iconW, 0, -token.paddingXS, 0);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, fb->content());
    }
}

void AntFloatButtonStyle::drawChildButton(const QStyleOption* option, QPainter* painter, const AntFloatButton* fb, int index) const
{
    Q_UNUSED(option)
    Q_UNUSED(painter)
    Q_UNUSED(fb)
    Q_UNUSED(index)
    // Child buttons are separate AntFloatButton widgets that render themselves
}
