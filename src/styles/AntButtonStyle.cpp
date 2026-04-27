#include "AntButtonStyle.h"

#include <QPainter>
#include <QStyleOption>

#include "widgets/AntButton.h"
#include "widgets/AntIcon.h"
#include "styles/AntPalette.h"

namespace
{
struct ButtonMetrics
{
    int height = 32;
    int fontSize = 14;
    int paddingX = 15;
    int radius = 6;
    int iconSize = 14;
};

struct ButtonColors
{
    QColor background;
    QColor border;
    QColor text;
    Qt::PenStyle borderStyle = Qt::SolidLine;
};

ButtonMetrics metricsFor(const AntButton* button)
{
    const auto& token = antTheme->tokens();
    ButtonMetrics m;
    switch (button ? button->buttonSize() : Ant::Size::Middle)
    {
    case Ant::Size::Large:
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
        m.paddingX = token.padding;
        m.radius = token.borderRadiusLG;
        m.iconSize = 16;
        break;
    case Ant::Size::Small:
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSize;
        m.paddingX = token.paddingXS;
        m.radius = token.borderRadiusSM;
        m.iconSize = 12;
        break;
    case Ant::Size::Middle:
        m.height = token.controlHeight;
        m.fontSize = token.fontSize;
        m.paddingX = token.paddingSM + token.lineWidth * 3;
        m.radius = token.borderRadius;
        m.iconSize = 14;
        break;
    }
    return m;
}

int cornerRadiusFor(const AntButton* button, const ButtonMetrics& metrics)
{
    if (button && (button->buttonShape() == Ant::ButtonShape::Circle || button->buttonShape() == Ant::ButtonShape::Round))
    {
        return metrics.height / 2;
    }
    return metrics.radius;
}

QRectF contentRectFor(const AntButton* button, const ButtonMetrics& metrics, const QRect& rect)
{
    if (button && button->buttonShape() == Ant::ButtonShape::Circle)
    {
        return rect;
    }
    return rect.adjusted(metrics.paddingX, 0, -metrics.paddingX, 0);
}

QColor semanticColorFor(const AntButton* button)
{
    return button && button->isDanger() ? antTheme->tokens().colorError : antTheme->tokens().colorPrimary;
}

void drawSpinner(QPainter& painter, const QRectF& rect, const QColor& color, int angle)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(rect, angle * 16, 108 * 16);
    painter.restore();
}
}

AntButtonStyle::AntButtonStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntButton>();
}

void AntButtonStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::CE_PushButton && qobject_cast<const AntButton*>(widget))
    {
        drawButton(option, painter, widget);
        return;
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

int AntButtonStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    const auto* button = qobject_cast<const AntButton*>(widget);
    if (button)
    {
        const ButtonMetrics m = metricsFor(button);
        switch (metric)
        {
        case QStyle::PM_ButtonMargin:
            return m.paddingX;
        case QStyle::PM_ButtonDefaultIndicator:
            return 0;
        case QStyle::PM_DefaultFrameWidth:
            return antTheme->tokens().lineWidth;
        default:
            break;
        }
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

QSize AntButtonStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    const auto* button = qobject_cast<const AntButton*>(widget);
    if (type == QStyle::CT_PushButton && button)
    {
        const ButtonMetrics m = metricsFor(button);
        QFont f = button->font();
        f.setPixelSize(m.fontSize);
        int width = QFontMetrics(f).horizontalAdvance(button->text()) + m.paddingX * 2;
        if (button->isLoading() || button->buttonIconType() != Ant::IconType::None)
        {
            width += m.iconSize + 8;
        }
        if (button->buttonShape() == Ant::ButtonShape::Circle)
        {
            width = m.height;
        }
        return QSize(std::max(width, m.height), m.height);
    }
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

void AntButtonStyle::drawButton(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* button = qobject_cast<const AntButton*>(widget);
    if (!button || !painter)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const ButtonMetrics m = metricsFor(button);
    const QRectF outer = option->rect;
    const int radius = cornerRadiusFor(button, m);
    const bool hovered = option->state.testFlag(QStyle::State_MouseOver);
    const bool pressed = option->state.testFlag(QStyle::State_Sunken) || button->isDown();
    const bool enabled = option->state.testFlag(QStyle::State_Enabled);
    const bool focused = option->state.testFlag(QStyle::State_HasFocus);
    const bool plainType = button->buttonType() == Ant::ButtonType::Text || button->buttonType() == Ant::ButtonType::Link;
    const QColor accent = semanticColorFor(button);

    ButtonColors colors;
    colors.background = plainType || button->isGhost() ? QColor(Qt::transparent) : token.colorBgContainer;
    colors.border = plainType ? QColor(Qt::transparent) : token.colorBorder;
    colors.text = token.colorText;

    if (button->buttonType() == Ant::ButtonType::Primary)
    {
        colors.background = accent;
        colors.border = accent;
        colors.text = token.colorTextLightSolid;
        if (hovered)
        {
            colors.background = colors.border = antTheme->hoverColor(accent);
        }
        if (pressed)
        {
            colors.background = colors.border = antTheme->activeColor(accent);
        }
    }
    else if (button->buttonType() == Ant::ButtonType::Default || button->buttonType() == Ant::ButtonType::Dashed)
    {
        colors.borderStyle = button->buttonType() == Ant::ButtonType::Dashed ? Qt::DashLine : Qt::SolidLine;
        if (button->isDanger())
        {
            colors.text = colors.border = token.colorError;
        }
        if (hovered)
        {
            colors.text = colors.border = button->isDanger() ? token.colorErrorHover : token.colorPrimaryHover;
        }
        if (pressed)
        {
            colors.text = colors.border = button->isDanger() ? token.colorErrorActive : token.colorPrimaryActive;
        }
    }
    else if (button->buttonType() == Ant::ButtonType::Text)
    {
        colors.text = button->isDanger() ? token.colorError : token.colorText;
        if (hovered)
        {
            colors.background = token.colorFillTertiary;
        }
        if (pressed)
        {
            colors.background = token.colorFill;
        }
    }
    else if (button->buttonType() == Ant::ButtonType::Link)
    {
        colors.text = button->isDanger() ? token.colorError : token.colorLink;
        if (hovered)
        {
            colors.text = button->isDanger() ? token.colorErrorHover : token.colorLinkHover;
        }
        if (pressed)
        {
            colors.text = button->isDanger() ? token.colorErrorActive : token.colorLinkActive;
        }
    }

    if (button->isGhost() && !plainType)
    {
        colors.background = QColor(Qt::transparent);
        if (button->buttonType() == Ant::ButtonType::Primary)
        {
            colors.text = colors.border = accent;
        }
    }

    if (button->isLoading())
    {
        if (button->buttonType() == Ant::ButtonType::Primary)
        {
            colors.background = colors.border = token.colorPrimaryLoading;
            colors.text = token.colorTextLightSolid;
            if (hovered)
            {
                colors.background = colors.border = token.colorPrimaryLoadingHover;
            }
            if (pressed)
            {
                colors.background = colors.border = token.colorPrimaryLoadingActive;
            }
        }
        else
        {
            colors.background = plainType || button->isGhost() ? QColor(Qt::transparent) : token.colorBgContainerDisabled;
            colors.border = plainType ? QColor(Qt::transparent) : token.colorBorderDisabled;
            colors.text = token.colorTextDisabled;
        }
    }
    else if (!enabled)
    {
        colors.background = plainType || button->isGhost() ? QColor(Qt::transparent) : token.colorBgContainerDisabled;
        colors.border = plainType ? QColor(Qt::transparent) : token.colorBorderDisabled;
        colors.text = token.colorTextDisabled;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (!plainType && enabled && !button->isGhost() && !pressed)
    {
        antTheme->drawEffectShadow(painter, option->rect, 4, radius, button->buttonType() == Ant::ButtonType::Primary ? 0.45 : 0.25);
    }

    AntStyleBase::drawCrispRoundedRect(painter, outer.toRect(),
        colors.border.alpha() == 0 ? Qt::NoPen : QPen(colors.border, token.lineWidth, colors.borderStyle),
        colors.background, radius, radius);

    if (focused && enabled && !plainType)
    {
        const QColor focus = AntPalette::alpha(button->isDanger() ? token.colorError : token.colorPrimary, 0.18);
        AntStyleBase::drawCrispRoundedRect(painter, outer.toRect(),
            QPen(focus, token.controlOutlineWidth), Qt::NoBrush, radius, radius);
    }

    QRectF textRect = contentRectFor(button, m, option->rect);
    painter->setFont(button->font());
    painter->setPen(colors.text);
    if (button->isLoading())
    {
        QRectF spinnerRect(textRect.left(), textRect.center().y() - m.iconSize / 2.0, m.iconSize, m.iconSize);
        drawSpinner(*painter, spinnerRect, colors.text, button->spinnerAngle());
        textRect.adjust(m.iconSize + 8, 0, 0, 0);
    }
    else if (button->buttonIconType() != Ant::IconType::None)
    {
        QRectF iconRect;
        if (button->buttonShape() == Ant::ButtonShape::Circle && button->text().isEmpty())
        {
            iconRect = QRectF(outer.center().x() - m.iconSize / 2.0, outer.center().y() - m.iconSize / 2.0, m.iconSize, m.iconSize);
        }
        else
        {
            iconRect = QRectF(textRect.left(), textRect.center().y() - m.iconSize / 2.0, m.iconSize, m.iconSize);
            textRect.adjust(m.iconSize + 8, 0, 0, 0);
        }

        const AntIcon::IconPaths paths = AntIcon::builtinPaths(button->buttonIconType(), Ant::IconTheme::Outlined);
        const QPainterPath primaryPath = AntIcon::transformPath(paths.primary, iconRect);
        const QPainterPath secondaryPath = paths.secondary.isEmpty() ? QPainterPath() : AntIcon::transformPath(paths.secondary, iconRect);

        QColor iconColor = button->buttonIconColor().isValid() ? button->buttonIconColor() : colors.text;
        if (!enabled)
            iconColor = token.colorTextDisabled;

        painter->save();
        if (paths.useStroke)
        {
            QPen pen(iconColor, qMax<qreal>(1.6, iconRect.width() * 0.1), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(primaryPath);
            if (!secondaryPath.isEmpty())
                painter->drawPath(secondaryPath);
        }
        else
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(iconColor);
            if (!secondaryPath.isEmpty())
                painter->drawPath(secondaryPath);
            painter->drawPath(primaryPath);
        }
        painter->restore();
    }
    painter->drawText(textRect, Qt::AlignCenter, button->text());
    painter->restore();
}
