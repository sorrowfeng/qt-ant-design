#include "AntToolButtonStyle.h"

#include <QPainter>
#include <QStyleOptionComplex>
#include <QStyleOptionToolButton>

#include "styles/AntIconPainter.h"
#include "styles/AntPalette.h"
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

ToolButtonMetrics toolMetrics(const AntToolButton* btn)
{
    const auto& token = antTheme->tokens();
    ToolButtonMetrics m;
    switch (btn ? btn->buttonSize() : Ant::Size::Middle)
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

QColor semanticColor(const AntToolButton* btn)
{
    return btn && btn->isDanger() ? antTheme->tokens().colorError : antTheme->tokens().colorPrimary;
}

struct Colors
{
    QColor bg;
    QColor border;
    QColor text;
};

Colors computeColors(const AntToolButton* btn, bool hovered, bool pressed, bool enabled)
{
    const auto& token = antTheme->tokens();
    const QColor accent = semanticColor(btn);
    Colors c;
    c.bg = token.colorBgContainer;
    c.border = token.colorBorder;
    c.text = token.colorText;

    if (btn->buttonType() == Ant::ButtonType::Primary)
    {
        c.bg = accent;
        c.border = accent;
        c.text = token.colorTextLightSolid;
        if (hovered) c.bg = c.border = antTheme->hoverColor(accent);
        if (pressed) c.bg = c.border = antTheme->activeColor(accent);
    }
    else if (btn->buttonType() == Ant::ButtonType::Default || btn->buttonType() == Ant::ButtonType::Dashed)
    {
        if (btn->isDanger()) c.text = c.border = token.colorError;
        if (hovered) c.text = c.border = btn->isDanger() ? token.colorErrorHover : token.colorPrimaryHover;
        if (pressed) c.text = c.border = btn->isDanger() ? token.colorErrorActive : token.colorPrimaryActive;
    }
    else if (btn->buttonType() == Ant::ButtonType::Text)
    {
        c.border = Qt::transparent;
        if (hovered) c.bg = token.colorFillTertiary;
        if (pressed) c.bg = token.colorFillQuaternary;
    }
    else if (btn->buttonType() == Ant::ButtonType::Link)
    {
        c.border = Qt::transparent;
        c.bg = Qt::transparent;
        c.text = btn->isDanger() ? token.colorError : token.colorLink;
        if (hovered) c.text = btn->isDanger() ? token.colorErrorHover : token.colorLinkHover;
        if (pressed) c.text = btn->isDanger() ? token.colorErrorActive : token.colorLinkActive;
    }

    if (!enabled)
    {
        c.bg = (btn->buttonType() == Ant::ButtonType::Text || btn->buttonType() == Ant::ButtonType::Link)
                   ? QColor(Qt::transparent) : token.colorBgContainerDisabled;
        c.border = (btn->buttonType() == Ant::ButtonType::Text || btn->buttonType() == Ant::ButtonType::Link)
                       ? QColor(Qt::transparent) : token.colorBorderDisabled;
        c.text = token.colorTextDisabled;
    }
    return c;
}

void drawArrow(QPainter* p, const QRectF& r, const QColor& color, qreal rotation)
{
    p->save();
    p->translate(r.center());
    p->rotate(rotation);
    p->translate(-r.center());
    AntIconPainter::drawIcon(*p, Ant::IconType::Down, r, color);
    p->restore();
}

void drawSpinner(QPainter& painter, const QRectF& rect, const QColor& color, int angle)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(rect, angle * 16, 270 * 16);
    painter.restore();
}

} // namespace

AntToolButtonStyle::AntToolButtonStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntToolButton>();
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
        if (btn->isLoading()) width += m.iconSize + 8;
        // Space for dropdown arrow
        if (btn->popupMode() == QToolButton::MenuButtonPopup || btn->menu())
            width += m.arrowSize + m.paddingX / 2;
        return QSize(std::max(width, m.height), m.height);
    }
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

void AntToolButtonStyle::drawToolButton(const QStyleOptionComplex* option, QPainter* painter,
                                        const QWidget* widget) const
{
    const auto* btn = qobject_cast<const AntToolButton*>(widget);
    if (!btn || !painter) return;

    const auto& token = antTheme->tokens();
    const ToolButtonMetrics m = toolMetrics(btn);
    const auto* bopt = qstyleoption_cast<const QStyleOptionToolButton*>(option);
    if (!bopt) return;

    const QRectF outer = bopt->rect;
    const bool hovered = bopt->state.testFlag(QStyle::State_MouseOver);
    const bool pressed = bopt->state.testFlag(QStyle::State_Sunken);
    const bool enabled = bopt->state.testFlag(QStyle::State_Enabled);
    const bool hasMenu = btn->menu() != nullptr;
    const bool plainType = btn->buttonType() == Ant::ButtonType::Text || btn->buttonType() == Ant::ButtonType::Link;

    Colors colors = computeColors(btn, hovered, pressed, enabled);
    const qreal arrowRot = btn->arrowRotation();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Shadow
    if (!plainType && enabled && !pressed)
    {
        antTheme->drawEffectShadow(painter, outer.toRect(), 4, m.radius,
                                   btn->buttonType() == Ant::ButtonType::Primary ? 0.45 : 0.25);
    }

    // Background + Border
    AntStyleBase::drawCrispRoundedRect(painter, outer.toRect(),
        colors.border.alpha() == 0 ? Qt::NoPen : QPen(colors.border, token.lineWidth),
        colors.bg, m.radius, m.radius);

    // Focus ring
    if (bopt->state.testFlag(QStyle::State_HasFocus) && enabled)
    {
        const QColor focus = AntPalette::alpha(btn->isDanger() ? token.colorError : token.colorPrimary, 0.18);
        AntStyleBase::drawCrispRoundedRect(painter, outer.toRect(),
            QPen(focus, token.controlOutlineWidth), Qt::NoBrush, m.radius, m.radius);
    }

    // Icon
    QRectF contentRect = outer.adjusted(m.paddingX, 0, -m.paddingX, 0);
    if (!bopt->icon.isNull())
    {
        const QRectF iconRect(contentRect.left(), contentRect.center().y() - m.iconSize / 2.0,
                              m.iconSize, m.iconSize);
        bopt->icon.paint(painter, iconRect.toRect(), Qt::AlignCenter, QIcon::Normal, enabled ? QIcon::On : QIcon::Off);
        contentRect.adjust(m.iconSize + 6, 0, 0, 0);
    }

    // Arrow indicator area
    QRectF arrowRect;
    if (hasMenu || btn->popupMode() == QToolButton::MenuButtonPopup)
    {
        const qreal arrowW = m.arrowSize + 8;
        arrowRect = QRectF(contentRect.right() - arrowW, contentRect.center().y() - m.arrowSize / 2.0,
                           m.arrowSize, m.arrowSize);
        contentRect.adjust(0, 0, -(arrowW + 2), 0);
    }

    // Text
    painter->setFont(btn->font());
    painter->setPen(colors.text);
    if (btn->isLoading())
    {
        QRectF spinnerRect(contentRect.left(), contentRect.center().y() - m.iconSize / 2.0, m.iconSize, m.iconSize);
        drawSpinner(*painter, spinnerRect, colors.text, btn->spinnerAngle());
        contentRect.adjust(m.iconSize + 8, 0, 0, 0);
    }
    painter->drawText(contentRect, Qt::AlignCenter, btn->text());

    // Arrow
    if (hasMenu || btn->popupMode() == QToolButton::MenuButtonPopup)
    {
        drawArrow(painter, arrowRect, colors.text, arrowRot);
    }

    painter->restore();
}
