#include "AntInputStyle.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
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
        if (variant == Ant::Variant::Underlined)
        {
            painter->drawLine(QPointF(frame.left(), frame.bottom() + 1), QPointF(frame.right(), frame.bottom() + 1));
        }
        else
        {
            AntStyleBase::drawCrispRoundedRect(painter, frame.adjusted(-1, -1, 1, 1).toRect(),
                QPen(focus, token.controlOutlineWidth), Qt::NoBrush, m.radius + 1, m.radius + 1);
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
        AntStyleBase::drawCrispRoundedRect(painter, frame.toRect(),
            focused ? QPen(border, token.lineWidth) : Qt::NoPen, bgFill, m.radius, m.radius);
        break;
    }
    case Ant::Variant::Borderless:
    {
        // Borderless: transparent bg, no border at all
        AntStyleBase::drawCrispRoundedRect(painter, frame.toRect(), Qt::NoPen,
            enabled ? QBrush(Qt::transparent) : QBrush(QColor(token.colorBgContainerDisabled)), m.radius, m.radius);
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
        AntStyleBase::drawCrispRoundedRect(painter, frame.toRect(), QPen(border, token.lineWidth),
            enabled ? QBrush(token.colorBgContainer) : QBrush(token.colorBgContainerDisabled), m.radius, m.radius);
        break;
    }
    }

    // Addons and Input.Search suffix button only make sense on outlined; on other variants they'd look wrong
    if (variant == Ant::Variant::Outlined)
    {
        const QRect beforeRect = input->addonBeforeRect();
        const QRect afterRect = input->addonAfterRect();
        const QRect searchRect = input->searchButtonRect();
        const bool hasSegments = (beforeRect.isValid() && !beforeRect.isEmpty())
            || (afterRect.isValid() && !afterRect.isEmpty())
            || (searchRect.isValid() && !searchRect.isEmpty());

        if (hasSegments)
        {
            QPainterPath clip;
            clip.addRoundedRect(frame, m.radius, m.radius);
            painter->save();
            painter->setClipPath(clip);
            painter->setPen(Qt::NoPen);
            painter->setBrush(enabled ? token.colorFillQuaternary : token.colorBgContainerDisabled);
            if (beforeRect.isValid() && !beforeRect.isEmpty())
            {
                painter->drawRect(beforeRect.adjusted(0, 1, 0, -1));
            }
            if (afterRect.isValid() && !afterRect.isEmpty())
            {
                painter->drawRect(afterRect.adjusted(0, 1, 0, -1));
            }
            if (searchRect.isValid() && !searchRect.isEmpty())
            {
                painter->setBrush(enabled ? token.colorBgContainer : token.colorBgContainerDisabled);
                painter->drawRect(searchRect.adjusted(0, 1, 0, -1));
            }
            painter->restore();

            painter->setPen(QPen(border, token.lineWidth));
            auto drawDivider = [&](qreal x) {
                painter->drawLine(QPointF(x, frame.top()), QPointF(x, frame.bottom()));
            };
            if (beforeRect.isValid() && !beforeRect.isEmpty())
            {
                drawDivider(beforeRect.right() + 0.5);
            }
            if (afterRect.isValid() && !afterRect.isEmpty())
            {
                drawDivider(afterRect.left() - 0.5);
            }
            if (searchRect.isValid() && !searchRect.isEmpty())
            {
                drawDivider(searchRect.left() - 0.5);
            }

            AntStyleBase::drawCrispRoundedRect(painter, frame.toRect(), QPen(border, token.lineWidth),
                Qt::NoBrush, m.radius, m.radius);
        }
    }

    painter->restore();
}
