#include "AntInputNumberStyle.h"

#include <QApplication>
#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionSpinBox>

#include "styles/AntPalette.h"
#include "widgets/AntInputNumber.h"

namespace
{
struct InputNumberMetrics
{
    int height = 32;
    int fontSize = 14;
    int radius = 6;
    int paddingX = 11;
    int handlerWidth = 22;
};

InputNumberMetrics metricsFor(const AntInputNumber* input)
{
    const auto& token = antTheme->tokens();
    InputNumberMetrics metrics;
    metrics.height = token.controlHeight;
    metrics.fontSize = token.fontSize;
    metrics.radius = token.borderRadius;
    metrics.paddingX = token.paddingSM - token.lineWidth;
    metrics.handlerWidth = token.fontSize + token.paddingSM;

    if (!input)
    {
        return metrics;
    }

    switch (input->inputSize())
    {
    case Ant::Size::Large:
        metrics.height = token.controlHeightLG;
        metrics.fontSize = token.fontSizeLG;
        metrics.radius = token.borderRadiusLG;
        metrics.handlerWidth = token.fontSizeLG + token.padding;
        break;
    case Ant::Size::Small:
        metrics.height = token.controlHeightSM;
        metrics.fontSize = token.fontSizeSM;
        metrics.radius = token.borderRadiusSM;
        metrics.paddingX = token.paddingXS;
        metrics.handlerWidth = token.fontSize + token.paddingXS + token.paddingXXS;
        break;
    case Ant::Size::Middle:
        break;
    }

    return metrics;
}

QRectF controlRectFor(const AntInputNumber* input, const QRect& rect)
{
    const InputNumberMetrics metrics = metricsFor(input);
    return QRectF(1, (rect.height() - metrics.height) / 2.0, rect.width() - 2, metrics.height);
}

bool focusedFor(const AntInputNumber* input)
{
    if (!input)
    {
        return false;
    }
    if (input->hasFocus())
    {
        return true;
    }
    const QWidget* focusWidget = QApplication::focusWidget();
    return focusWidget && input->isAncestorOf(focusWidget);
}

QColor borderColorFor(const AntInputNumber* input)
{
    const auto& token = antTheme->tokens();
    if (!input || !input->isEnabled())
    {
        return token.colorBorderDisabled;
    }
    if (input->status() == Ant::Status::Error)
    {
        return (input->isHoveredState() || focusedFor(input)) ? token.colorErrorHover : token.colorError;
    }
    if (input->status() == Ant::Status::Warning)
    {
        return (input->isHoveredState() || focusedFor(input)) ? token.colorWarningHover : token.colorWarning;
    }
    if (focusedFor(input))
    {
        return token.colorPrimary;
    }
    if (input->isHoveredState())
    {
        return token.colorPrimaryHover;
    }
    return token.colorBorder;
}

QColor backgroundColorFor(const AntInputNumber* input)
{
    const auto& token = antTheme->tokens();
    if (!input || !input->isEnabled())
    {
        return token.colorBgContainerDisabled;
    }
    switch (input->variant())
    {
    case Ant::Variant::Filled:
        return input->isHoveredState() ? token.colorFillTertiary : token.colorFillQuaternary;
    case Ant::Variant::Borderless:
    case Ant::Variant::Underlined:
        return QColor(0, 0, 0, 0);
    case Ant::Variant::Outlined:
    default:
        return token.colorBgContainer;
    }
}

bool handlersVisibleFor(const AntInputNumber* input)
{
    return input && input->controlsVisible() && input->isEnabled() && (input->isHoveredState() || focusedFor(input));
}

int addonAfterWidthFor(const AntInputNumber* input, const InputNumberMetrics& metrics)
{
    if (!input || input->addonAfterText().isEmpty())
    {
        return 0;
    }
    QFont font = input->font();
    font.setPixelSize(metrics.fontSize);
    return QFontMetrics(font).horizontalAdvance(input->addonAfterText()) + metrics.paddingX * 2;
}
}

AntInputNumberStyle::AntInputNumberStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntInputNumber>();
}

void AntInputNumberStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntInputNumber*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

void AntInputNumberStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntInputNumber*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntInputNumberStyle::drawComplexControl(ComplexControl control,
                                             const QStyleOptionComplex* option,
                                             QPainter* painter,
                                             const QWidget* widget) const
{
    if (control == QStyle::CC_SpinBox && qobject_cast<const AntInputNumber*>(widget))
    {
        drawSpinBox(option, painter, widget);
        return;
    }

    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

QRect AntInputNumberStyle::subControlRect(ComplexControl control,
                                          const QStyleOptionComplex* option,
                                          SubControl subControl,
                                          const QWidget* widget) const
{
    if (control == QStyle::CC_SpinBox)
    {
        const auto* input = qobject_cast<const AntInputNumber*>(widget);
        const InputNumberMetrics metrics = metricsFor(input);
        const QRectF controlRect = controlRectFor(input, option ? option->rect : QRect());
        const int handlerWidth = handlersVisibleFor(input) ? metrics.handlerWidth : 0;
        const int addonAfterWidth = addonAfterWidthFor(input, metrics);

        if (subControl == SC_SpinBoxEditField)
        {
            return controlRect.adjusted(metrics.paddingX,
                                        0,
                                        -(metrics.paddingX + handlerWidth + addonAfterWidth),
                                        0).toRect();
        }
        if (subControl == SC_SpinBoxUp && handlerWidth > 0)
        {
            return QRect(controlRect.right() - handlerWidth + 1,
                         controlRect.top() + 1,
                         handlerWidth,
                         controlRect.height() / 2);
        }
        if (subControl == SC_SpinBoxDown && handlerWidth > 0)
        {
            return QRect(controlRect.right() - handlerWidth + 1,
                         controlRect.center().y(),
                         handlerWidth,
                         controlRect.height() / 2);
        }
        if (subControl == SC_SpinBoxFrame)
        {
            return controlRect.toRect();
        }
    }

    return QProxyStyle::subControlRect(control, option, subControl, widget);
}

QSize AntInputNumberStyle::sizeFromContents(ContentsType type,
                                            const QStyleOption* option,
                                            const QSize& contentsSize,
                                            const QWidget* widget) const
{
    if (type == CT_SpinBox && qobject_cast<const AntInputNumber*>(widget))
    {
        const auto* input = qobject_cast<const AntInputNumber*>(widget);
        const InputNumberMetrics metrics = metricsFor(input);
        return QSize(qMax(contentsSize.width(), 120), metrics.height);
    }

    return QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
}

bool AntInputNumberStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* input = qobject_cast<AntInputNumber*>(watched);
    if (input && event->type() == QEvent::Paint)
    {
        QStyleOptionSpinBox option;
        option.initFrom(input);
        option.rect = input->rect();
        option.subControls = SC_SpinBoxFrame | SC_SpinBoxEditField;
        if (input->controlsVisible())
        {
            option.subControls |= SC_SpinBoxUp | SC_SpinBoxDown;
        }
        if (input->isHoveredState())
        {
            option.state |= State_MouseOver;
        }
        if (focusedFor(input))
        {
            option.state |= State_HasFocus;
        }
        option.activeSubControls = input->activeSubControl();
        if (input->isStepPressed())
        {
            option.state |= State_Sunken;
        }
        option.stepEnabled = QAbstractSpinBox::StepNone;
        if (input->stepEnabledFlags().testFlag(QAbstractSpinBox::StepUpEnabled))
        {
            option.stepEnabled |= QAbstractSpinBox::StepUpEnabled;
        }
        if (input->stepEnabledFlags().testFlag(QAbstractSpinBox::StepDownEnabled))
        {
            option.stepEnabled |= QAbstractSpinBox::StepDownEnabled;
        }

        QPainter painter(input);
        drawComplexControl(CC_SpinBox, &option, &painter, input);
    }

    return QProxyStyle::eventFilter(watched, event);
}

void AntInputNumberStyle::drawSpinBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const
{
    const auto* input = qobject_cast<const AntInputNumber*>(widget);
    const auto* spinOption = qstyleoption_cast<const QStyleOptionSpinBox*>(option);
    if (!input || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const InputNumberMetrics metrics = metricsFor(input);
    const QRectF control = controlRectFor(input, option->rect);
    const QRect frameRect = subControlRect(CC_SpinBox, option, SC_SpinBoxFrame, widget);
    const QRect upRect = subControlRect(CC_SpinBox, option, SC_SpinBoxUp, widget);
    const QRect downRect = subControlRect(CC_SpinBox, option, SC_SpinBoxDown, widget);
    const int addonAfterWidth = addonAfterWidthFor(input, metrics);
    const QColor borderColor = borderColorFor(input);
    const QColor backgroundColor = backgroundColorFor(input);
    const bool focused = focusedFor(input);
    const bool disabled = !option->state.testFlag(State_Enabled);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    if (focused
        && !disabled
        && input->variant() != Ant::Variant::Borderless
        && input->variant() != Ant::Variant::Underlined)
    {
        AntStyleBase::drawCrispRoundedRect(painter, control.adjusted(-1, -1, 1, 1).toRect(),
            QPen(AntPalette::alpha(borderColor, 0.16), token.controlOutlineWidth), Qt::NoBrush, metrics.radius + 1, metrics.radius + 1);
    }

    if (input->variant() != Ant::Variant::Borderless
        && input->variant() != Ant::Variant::Underlined)
    {
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(), QPen(borderColor, token.lineWidth),
            backgroundColor, metrics.radius, metrics.radius);
    }
    else
    {
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(), Qt::NoPen,
            backgroundColor, metrics.radius, metrics.radius);
        if (input->variant() == Ant::Variant::Underlined)
        {
            painter->setPen(QPen(borderColor, focused ? 2 : token.lineWidth));
            painter->drawLine(QPointF(control.left(), control.bottom() - 0.5),
                              QPointF(control.right(), control.bottom() - 0.5));
        }
    }

    if (handlersVisibleFor(input))
    {
        const QColor splitColor = disabled ? token.colorBorderDisabled : token.colorSplit;
        const QColor hoverColor = token.colorFillTertiary;
        const QColor pressedColor = token.colorFillQuaternary;
        const QColor iconColor = disabled ? token.colorTextDisabled : token.colorTextTertiary;

        painter->setPen(QPen(splitColor, token.lineWidth));
        painter->drawLine(QPointF(upRect.left() + 0.5, control.top() + 3),
                          QPointF(upRect.left() + 0.5, control.bottom() - 3));
        painter->drawLine(QPointF(upRect.left() + 2, control.center().y()),
                          QPointF(control.right() - 2, control.center().y()));

        const bool upHovered = spinOption && spinOption->activeSubControls == SC_SpinBoxUp;
        const bool downHovered = spinOption && spinOption->activeSubControls == SC_SpinBoxDown;
        const bool pressed = option->state.testFlag(State_Sunken);

        if (upHovered)
        {
            painter->fillRect(upRect.adjusted(1, 1, -1, 0), pressed ? pressedColor : hoverColor);
        }
        if (downHovered)
        {
            painter->fillRect(downRect.adjusted(1, 0, -1, -1), pressed ? pressedColor : hoverColor);
        }

        painter->setPen(QPen(iconColor, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        QPainterPath upArrow;
        upArrow.moveTo(upRect.center().x() - 3, upRect.center().y() + 1);
        upArrow.lineTo(upRect.center().x(), upRect.center().y() - 2);
        upArrow.lineTo(upRect.center().x() + 3, upRect.center().y() + 1);
        painter->drawPath(upArrow);

        QPainterPath downArrow;
        downArrow.moveTo(downRect.center().x() - 3, downRect.center().y() - 1);
        downArrow.lineTo(downRect.center().x(), downRect.center().y() + 2);
        downArrow.lineTo(downRect.center().x() + 3, downRect.center().y() - 1);
        painter->drawPath(downArrow);
    }

    if (addonAfterWidth > 0)
    {
        const QRectF addonRect(control.right() - addonAfterWidth, control.top(), addonAfterWidth, control.height());
        QPainterPath clip;
        clip.addRoundedRect(control, metrics.radius, metrics.radius);
        painter->save();
        painter->setClipPath(clip);
        painter->setPen(Qt::NoPen);
        painter->setBrush(disabled ? token.colorBgContainerDisabled : token.colorFillQuaternary);
        painter->drawRect(addonRect.adjusted(0, 1, 0, -1));
        painter->restore();

        painter->setPen(QPen(disabled ? token.colorBorderDisabled : token.colorSplit, token.lineWidth));
        painter->drawLine(QPointF(addonRect.left(), control.top()), QPointF(addonRect.left(), control.bottom()));

        QFont addonFont = painter->font();
        addonFont.setPixelSize(metrics.fontSize);
        painter->setFont(addonFont);
        painter->setPen(disabled ? token.colorTextDisabled : token.colorText);
        painter->drawText(addonRect, Qt::AlignCenter, input->addonAfterText());
    }

    Q_UNUSED(frameRect)
    painter->restore();
}
