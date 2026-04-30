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
        m.paddingX = token.padding - token.lineWidth;
        m.radius = token.borderRadiusLG;
        m.iconSize = 16;
        break;
    case Ant::Size::Small:
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSize;
        m.paddingX = token.paddingXS - token.lineWidth;
        m.radius = token.borderRadiusSM;
        m.iconSize = 14;
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

int focusPaddingFor()
{
    const auto& token = antTheme->tokens();
    return token.lineWidthFocus + 1;
}

QColor semanticColorFor(const AntButton* button)
{
    return button && button->isDanger() ? antTheme->tokens().colorError : antTheme->tokens().colorPrimary;
}

void drawSpinner(QPainter& painter, const QRectF& rect, const QColor& color, int angle)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    const qreal penWidth = qMax<qreal>(1.5, rect.width() * 0.12);
    const QRectF arcRect = rect.adjusted(penWidth / 2.0, penWidth / 2.0, -penWidth / 2.0, -penWidth / 2.0);
    painter.translate(arcRect.center());
    painter.rotate(angle);
    painter.translate(-arcRect.center());
    painter.setPen(QPen(color, penWidth, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(arcRect, 90 * 16, -96 * 16);
    painter.restore();
}

QColor loadingColor(const QColor& color)
{
    QColor result = color;
    result.setAlphaF(result.alphaF() * 0.65);
    return result;
}

QColor shadowColorFor(const AntButton* button)
{
    const auto& token = antTheme->tokens();
    if (!button)
    {
        return QColor(Qt::transparent);
    }
    if (button->buttonType() == Ant::ButtonType::Primary)
    {
        return button->isDanger() ? token.colorErrorBg : token.colorPrimaryBg;
    }
    if (button->isDanger())
    {
        return token.colorErrorBg;
    }
    return token.colorFillQuaternary;
}

void drawButtonBottomShadow(QPainter& painter, const QRectF& outer, int radius, const QColor& color)
{
    if (color.alpha() == 0)
    {
        return;
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(outer.adjusted(0, 2, 0, 2), radius, radius);
    painter.restore();
}

void drawButtonFocusOutline(QPainter& painter, const QRectF& bodyRect, int radius)
{
    const auto& token = antTheme->tokens();
    const qreal offset = 1.0;
    const qreal width = token.lineWidthFocus;
    const qreal expand = offset + width / 2.0;
    const QRectF focusRect = bodyRect.adjusted(-expand, -expand, expand, expand);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(token.colorPrimaryBorder, width, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(focusRect, radius + expand, radius + expand);
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
        const int focusPadding = focusPaddingFor();
        return QSize(std::max(width, m.height) + focusPadding * 2, m.height + focusPadding * 2);
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
    const int focusPadding = focusPaddingFor();
    const QRectF outer = QRectF(option->rect).adjusted(focusPadding, focusPadding, -focusPadding, -focusPadding);
    const int radius = cornerRadiusFor(button, m);
    const bool hovered = option->state.testFlag(QStyle::State_MouseOver);
    const bool pressed = option->state.testFlag(QStyle::State_Sunken) || button->isDown();
    const bool enabled = option->state.testFlag(QStyle::State_Enabled);
    const bool focused = button->isFocusVisibleState();
    const bool plainType = button->buttonType() == Ant::ButtonType::Text || button->buttonType() == Ant::ButtonType::Link;
    const QColor accent = semanticColorFor(button);
    const QColor accentHover = button->isDanger() ? token.colorErrorHover : token.colorPrimaryHover;
    const QColor accentActive = button->isDanger() ? token.colorErrorActive : token.colorPrimaryActive;

    ButtonColors colors;
    colors.background = plainType || button->isGhost() ? QColor(Qt::transparent) : token.colorBgContainer;
    colors.border = plainType ? QColor(Qt::transparent) : token.colorBorder;
    colors.text = token.colorText;

    if (button->buttonType() == Ant::ButtonType::Primary)
    {
        colors.background = accent;
        colors.border = QColor(Qt::transparent);
        colors.text = token.colorTextLightSolid;
        if (hovered)
        {
            colors.background = accentHover;
        }
        if (pressed)
        {
            colors.background = accentActive;
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
            if (hovered)
            {
                colors.text = colors.border = accentHover;
            }
            if (pressed)
            {
                colors.text = colors.border = accentActive;
            }
        }
    }

    if (button->isLoading())
    {
        colors.background = loadingColor(colors.background);
        colors.border = loadingColor(colors.border);
        colors.text = loadingColor(colors.text);
    }

    if (!enabled)
    {
        colors.background = plainType || button->isGhost() ? QColor(Qt::transparent) : token.colorBgContainerDisabled;
        colors.border = plainType ? QColor(Qt::transparent) : token.colorBorderDisabled;
        colors.text = token.colorTextDisabled;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (!plainType && enabled && !button->isGhost() && !pressed && !button->isLoading())
    {
        QColor shadowColor = shadowColorFor(button);
        drawButtonBottomShadow(*painter, outer, radius, shadowColor);
    }

    AntStyleBase::drawCrispRoundedRect(painter, outer.toRect(),
        colors.border.alpha() == 0 ? Qt::NoPen : QPen(colors.border, token.lineWidth, colors.borderStyle),
        colors.background, radius, radius);

    if (focused && enabled)
    {
        drawButtonFocusOutline(*painter, outer, radius);
    }

    QRectF textRect = contentRectFor(button, m, outer.toRect());
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
        const bool trailingIcon = button->buttonIconType() == Ant::IconType::Down && !button->text().isEmpty();
        if (button->buttonShape() == Ant::ButtonShape::Circle && button->text().isEmpty())
        {
            iconRect = QRectF(outer.center().x() - m.iconSize / 2.0, outer.center().y() - m.iconSize / 2.0, m.iconSize, m.iconSize);
        }
        else if (trailingIcon)
        {
            iconRect = QRectF(textRect.right() - m.iconSize, textRect.center().y() - m.iconSize / 2.0, m.iconSize, m.iconSize);
            textRect.adjust(0, 0, -(m.iconSize + 8), 0);
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
