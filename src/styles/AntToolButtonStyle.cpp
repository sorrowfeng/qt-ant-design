#include "AntToolButtonStyle.h"

#include <algorithm>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOptionComplex>
#include <QStyleOptionToolButton>

#include "styles/AntIconPainter.h"
#include "widgets/AntToolButton.h"

namespace
{
struct ToolButtonMetrics
{
    int height = 32;
    int fontSize = 14;
    int paddingX = 15;
    int radius = 6;
    int iconSize = 14;
    int arrowSize = 10;
};

struct ToolButtonColors
{
    QColor background;
    QColor border;
    QColor text;
    Qt::PenStyle borderStyle = Qt::SolidLine;
};

ToolButtonMetrics toolMetrics(const AntToolButton* btn)
{
    const auto& token = antTheme->tokens();
    ToolButtonMetrics m;
    switch (btn ? btn->buttonSize() : Ant::Size::Middle)
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

int focusPaddingForToolButton()
{
    const auto& token = antTheme->tokens();
    return token.lineWidthFocus + 1;
}

QRectF contentRectFor(const ToolButtonMetrics& metrics, const QRect& rect)
{
    return rect.adjusted(metrics.paddingX, 0, -metrics.paddingX, 0);
}

QColor semanticColorFor(const AntToolButton* btn)
{
    return btn && btn->isDanger() ? antTheme->tokens().colorError : antTheme->tokens().colorPrimary;
}

QColor loadingColor(const QColor& color)
{
    QColor result = color;
    result.setAlphaF(result.alphaF() * 0.65);
    return result;
}

QColor shadowColorFor(const AntToolButton* btn)
{
    const auto& token = antTheme->tokens();
    if (!btn)
    {
        return QColor(Qt::transparent);
    }
    if (btn->buttonType() == Ant::ButtonType::Primary)
    {
        return btn->isDanger() ? token.colorErrorBg : token.colorPrimaryBg;
    }
    if (btn->isDanger())
    {
        return token.colorErrorBg;
    }
    return token.colorFillQuaternary;
}

ToolButtonColors computeColors(const AntToolButton* btn, bool hovered, bool pressed, bool enabled)
{
    const auto& token = antTheme->tokens();
    const QColor accent = semanticColorFor(btn);
    const QColor accentHover = btn && btn->isDanger() ? token.colorErrorHover : token.colorPrimaryHover;
    const QColor accentActive = btn && btn->isDanger() ? token.colorErrorActive : token.colorPrimaryActive;

    ToolButtonColors c;
    const bool plainType = btn && (btn->buttonType() == Ant::ButtonType::Text || btn->buttonType() == Ant::ButtonType::Link);
    c.background = plainType ? QColor(Qt::transparent) : token.colorBgContainer;
    c.border = plainType ? QColor(Qt::transparent) : token.colorBorder;
    c.text = token.colorText;

    if (btn && btn->buttonType() == Ant::ButtonType::Primary)
    {
        c.background = accent;
        c.border = QColor(Qt::transparent);
        c.text = token.colorTextLightSolid;
        if (hovered)
        {
            c.background = accentHover;
        }
        if (pressed)
        {
            c.background = accentActive;
        }
    }
    else if (btn && (btn->buttonType() == Ant::ButtonType::Default || btn->buttonType() == Ant::ButtonType::Dashed))
    {
        c.borderStyle = btn->buttonType() == Ant::ButtonType::Dashed ? Qt::DashLine : Qt::SolidLine;
        if (btn->isDanger())
        {
            c.text = c.border = token.colorError;
        }
        if (hovered)
        {
            c.text = c.border = btn->isDanger() ? token.colorErrorHover : token.colorPrimaryHover;
        }
        if (pressed)
        {
            c.text = c.border = btn->isDanger() ? token.colorErrorActive : token.colorPrimaryActive;
        }
    }
    else if (btn && btn->buttonType() == Ant::ButtonType::Text)
    {
        c.text = btn->isDanger() ? token.colorError : token.colorText;
        if (hovered)
        {
            c.background = token.colorFillTertiary;
        }
        if (pressed)
        {
            c.background = token.colorFill;
        }
    }
    else if (btn && btn->buttonType() == Ant::ButtonType::Link)
    {
        c.text = btn->isDanger() ? token.colorError : token.colorLink;
        if (hovered)
        {
            c.text = btn->isDanger() ? token.colorErrorHover : token.colorLinkHover;
        }
        if (pressed)
        {
            c.text = btn->isDanger() ? token.colorErrorActive : token.colorLinkActive;
        }
    }

    if (btn && btn->isLoading())
    {
        c.background = loadingColor(c.background);
        c.border = loadingColor(c.border);
        c.text = loadingColor(c.text);
    }

    if (!enabled)
    {
        c.background = plainType ? QColor(Qt::transparent) : token.colorBgContainerDisabled;
        c.border = plainType ? QColor(Qt::transparent) : token.colorBorderDisabled;
        c.text = token.colorTextDisabled;
    }
    return c;
}

void drawArrow(QPainter* painter, const QRectF& rect, const QColor& color, qreal rotation)
{
    painter->save();
    painter->translate(rect.center());
    painter->rotate(rotation);
    painter->translate(-rect.center());
    AntIconPainter::drawIcon(*painter, Ant::IconType::Down, rect, color);
    painter->restore();
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
} // namespace

AntToolButtonStyle::AntToolButtonStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntToolButtonStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                                            QPainter* painter, const QWidget* widget) const
{
    if (control == QStyle::CC_ToolButton && qobject_cast<const AntToolButton*>(widget))
    {
        drawToolButton(option, painter, widget);
        return;
    }
    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

int AntToolButtonStyle::pixelMetric(PixelMetric metric, const QStyleOption* option,
                                    const QWidget* widget) const
{
    const auto* btn = qobject_cast<const AntToolButton*>(widget);
    if (btn)
    {
        const ToolButtonMetrics m = toolMetrics(btn);
        switch (metric)
        {
        case QStyle::PM_ButtonMargin:
            return m.paddingX;
        case QStyle::PM_DefaultFrameWidth:
            return antTheme->tokens().lineWidth;
        default:
            break;
        }
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

QSize AntToolButtonStyle::sizeFromContents(ContentsType type, const QStyleOption* option,
                                           const QSize& size, const QWidget* widget) const
{
    const auto* btn = qobject_cast<const AntToolButton*>(widget);
    if (type == QStyle::CT_ToolButton && btn)
    {
        const ToolButtonMetrics m = toolMetrics(btn);
        QFont f = btn->font();
        f.setPixelSize(m.fontSize);
        int width = QFontMetrics(f).horizontalAdvance(btn->text()) + m.paddingX * 2;
        if (btn->isLoading() || !btn->icon().isNull())
        {
            width += m.iconSize + 8;
        }
        if (btn->menu() || btn->popupMode() == QToolButton::MenuButtonPopup)
        {
            width += m.arrowSize + 8;
        }

        const int focusPadding = focusPaddingForToolButton();
        return QSize(std::max(width, m.height) + focusPadding * 2, m.height + focusPadding * 2);
    }
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

void AntToolButtonStyle::drawToolButton(const QStyleOptionComplex* option, QPainter* painter,
                                        const QWidget* widget) const
{
    const auto* btn = qobject_cast<const AntToolButton*>(widget);
    const auto* bopt = qstyleoption_cast<const QStyleOptionToolButton*>(option);
    if (!btn || !bopt || !painter)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const ToolButtonMetrics m = toolMetrics(btn);
    const int focusPadding = focusPaddingForToolButton();
    const QRectF outer = QRectF(bopt->rect).adjusted(focusPadding, focusPadding, -focusPadding, -focusPadding);
    const int radius = m.radius;
    const bool enabled = bopt->state.testFlag(QStyle::State_Enabled);
    const bool hovered = enabled && bopt->state.testFlag(QStyle::State_MouseOver);
    const bool pressed = enabled && (bopt->state.testFlag(QStyle::State_Sunken) || btn->isDown());
    const bool focused = btn->isFocusVisibleState();
    const bool plainType = btn->buttonType() == Ant::ButtonType::Text || btn->buttonType() == Ant::ButtonType::Link;
    const bool hasMenu = btn->menu() || btn->popupMode() == QToolButton::MenuButtonPopup;

    const ToolButtonColors colors = computeColors(btn, hovered, pressed, enabled);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (!plainType && enabled && !pressed && !btn->isLoading())
    {
        drawButtonBottomShadow(*painter, outer, radius, shadowColorFor(btn));
    }

    AntStyleBase::drawCrispRoundedRect(painter, outer.toRect(),
        colors.border.alpha() == 0 ? Qt::NoPen : QPen(colors.border, token.lineWidth, colors.borderStyle),
        colors.background, radius, radius);

    if (focused && enabled)
    {
        drawButtonFocusOutline(*painter, outer, radius);
    }

    QRectF textRect = contentRectFor(m, outer.toRect());
    QRectF arrowRect;
    if (hasMenu)
    {
        arrowRect = QRectF(textRect.right() - m.arrowSize,
                           textRect.center().y() - m.arrowSize / 2.0,
                           m.arrowSize,
                           m.arrowSize);
        textRect.adjust(0, 0, -(m.arrowSize + 8), 0);
    }

    painter->setFont(btn->font());
    painter->setPen(colors.text);
    if (btn->isLoading())
    {
        const bool spinnerOnly = btn->text().isEmpty();
        const QRectF spinnerRect = spinnerOnly
            ? QRectF(textRect.center().x() - m.iconSize / 2.0, textRect.center().y() - m.iconSize / 2.0, m.iconSize, m.iconSize)
            : QRectF(textRect.left(), textRect.center().y() - m.iconSize / 2.0, m.iconSize, m.iconSize);
        drawSpinner(*painter, spinnerRect, colors.text, btn->spinnerAngle());
        if (!spinnerOnly)
        {
            textRect.adjust(m.iconSize + 8, 0, 0, 0);
        }
    }
    else if (!btn->icon().isNull())
    {
        QRectF iconRect;
        if (btn->text().isEmpty())
        {
            iconRect = QRectF(textRect.center().x() - m.iconSize / 2.0,
                              textRect.center().y() - m.iconSize / 2.0,
                              m.iconSize,
                              m.iconSize);
        }
        else
        {
            iconRect = QRectF(textRect.left(), textRect.center().y() - m.iconSize / 2.0, m.iconSize, m.iconSize);
            textRect.adjust(m.iconSize + 8, 0, 0, 0);
        }
        btn->icon().paint(painter,
                          iconRect.toRect(),
                          Qt::AlignCenter,
                          enabled ? QIcon::Normal : QIcon::Disabled,
                          btn->isChecked() ? QIcon::On : QIcon::Off);
    }

    if (!btn->text().isEmpty())
    {
        painter->drawText(textRect, Qt::AlignCenter, btn->text());
    }

    if (hasMenu)
    {
        drawArrow(painter, arrowRect, colors.text, btn->arrowRotation());
    }

    painter->restore();
}
