#include "AntInputStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntInput.h"

namespace
{
struct InputMetrics
{
    int height = 32;
    int fontSize = 14;
    int paddingX = 11;
    int radius = 6;
    int iconSize = 16;
};

InputMetrics metricsFor(const AntInput* input)
{
    const auto& token = antTheme->tokens();
    InputMetrics m;
    switch (input ? input->inputSize() : Ant::Size::Middle)
    {
    case Ant::Size::Large:
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
        m.paddingX = token.paddingSM;
        m.radius = token.borderRadiusLG;
        m.iconSize = 18;
        break;
    case Ant::Size::Small:
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSize;
        m.paddingX = token.paddingXS;
        m.radius = token.borderRadiusSM;
        m.iconSize = 14;
        break;
    case Ant::Size::Middle:
        m.height = token.controlHeight;
        m.fontSize = token.fontSize;
        m.paddingX = token.paddingSM - token.lineWidth;
        m.radius = token.borderRadius;
        m.iconSize = 16;
        break;
    }
    return m;
}

QColor borderColorFor(const AntInput* input)
{
    const auto& token = antTheme->tokens();
    if (!input || !input->isEnabled())
    {
        return token.colorBorderDisabled;
    }
    if (input->status() == Ant::Status::Error)
    {
        return input->isInputHovered() || input->isInputFocused() ? token.colorErrorHover : token.colorError;
    }
    if (input->status() == Ant::Status::Warning)
    {
        return input->isInputHovered() || input->isInputFocused() ? token.colorWarningHover : token.colorWarning;
    }
    if (input->isInputFocused())
    {
        return token.colorPrimary;
    }
    if (input->isInputHovered())
    {
        return token.colorPrimaryHover;
    }
    return token.colorBorder;
}
}

AntInputStyle::AntInputStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntInput>();
}

void AntInputStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntInput*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

void AntInputStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntInput*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntInputStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if ((element == QStyle::PE_Widget || element == QStyle::PE_PanelLineEdit) && qobject_cast<const AntInput*>(widget))
    {
        drawInputFrame(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntInputStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    const auto* input = qobject_cast<const AntInput*>(widget);
    if (type == QStyle::CT_LineEdit && input)
    {
        const InputMetrics m = metricsFor(input);
        return QSize(std::max(size.width(), 220), m.height);
    }
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntInputStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* input = qobject_cast<AntInput*>(watched);
    if (input && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(input);
        QPainter painter(input);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, input);
        return false;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntInputStyle::drawInputFrame(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* input = qobject_cast<const AntInput*>(widget);
    if (!input || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const InputMetrics m = metricsFor(input);
    const QRectF frame = option->rect.adjusted(0.5, 0.5, -0.5, -0.5);
    const auto variant = input->variant();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Focus outline glow (outlined / filled / underlined)
    if (input->isInputFocused() && input->isEnabled() && variant != Ant::Variant::Borderless)
    {
        QColor focus = input->status() == Ant::Status::Error ? token.colorErrorBg : token.colorPrimaryBg;
        focus.setAlphaF(0.65);
        painter->setPen(QPen(focus, token.controlOutlineWidth));
        painter->setBrush(Qt::NoBrush);
        if (variant == Ant::Variant::Underlined)
        {
            painter->drawLine(QPointF(frame.left(), frame.bottom() + 1), QPointF(frame.right(), frame.bottom() + 1));
        }
        else
        {
            painter->drawRoundedRect(frame.adjusted(-1, -1, 1, 1), m.radius + 1, m.radius + 1);
        }
    }

    // Background + border per variant
    const bool enabled = input->isEnabled();
    const QColor border = borderColorFor(input);
    QColor bgFill;
    switch (variant)
    {
    case Ant::Variant::Filled:
    {
        // Filled: filled bg, hover lightens, focus reveals border + white bg
        const bool focused = input->isInputFocused() && enabled;
        if (!enabled)
        {
            bgFill = token.colorBgContainerDisabled;
        }
        else if (focused)
        {
            bgFill = token.colorBgContainer;
        }
        else if (input->isInputHovered())
        {
            bgFill = token.colorFillTertiary;
        }
        else
        {
            bgFill = token.colorFillQuaternary;
        }
        painter->setPen(focused ? QPen(border, token.lineWidth) : Qt::NoPen);
        painter->setBrush(bgFill);
        painter->drawRoundedRect(frame, m.radius, m.radius);
        break;
    }
    case Ant::Variant::Borderless:
    {
        // Borderless: transparent bg, no border at all
        painter->setPen(Qt::NoPen);
        painter->setBrush(enabled ? Qt::transparent : QColor(token.colorBgContainerDisabled));
        painter->drawRoundedRect(frame, m.radius, m.radius);
        break;
    }
    case Ant::Variant::Underlined:
    {
        // Underlined: transparent bg, only bottom border
        painter->setPen(Qt::NoPen);
        painter->setBrush(enabled ? Qt::transparent : QColor(token.colorBgContainerDisabled));
        painter->drawRect(frame);
        painter->setPen(QPen(border, token.lineWidth));
        painter->drawLine(QPointF(frame.left(), frame.bottom()), QPointF(frame.right(), frame.bottom()));
        break;
    }
    case Ant::Variant::Outlined:
    default:
    {
        painter->setPen(QPen(border, token.lineWidth));
        painter->setBrush(enabled ? token.colorBgContainer : token.colorBgContainerDisabled);
        painter->drawRoundedRect(frame, m.radius, m.radius);
        break;
    }
    }

    // Addons only make sense on outlined; on other variants they'd look wrong
    if (variant == Ant::Variant::Outlined)
    {
        auto drawAddon = [&](const QRect& rect) {
            if (!rect.isValid() || rect.isEmpty())
            {
                return;
            }
            painter->setPen(QPen(border, token.lineWidth));
            painter->setBrush(token.colorFillQuaternary);
            painter->drawRect(rect.adjusted(0, 0, -1, -1));
        };
        drawAddon(input->addonBeforeRect());
        drawAddon(input->addonAfterRect());
    }

    painter->restore();
}
