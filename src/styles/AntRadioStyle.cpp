#include "AntRadioStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOptionButton>

#include "widgets/AntRadio.h"
#include "styles/AntPalette.h"

namespace
{
constexpr int RadioSize = 16;
constexpr int TextSpacing = 8;
constexpr qreal DotRatio = 0.5;
}

AntRadioStyle::AntRadioStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntRadioStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    if (qobject_cast<AntRadio*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

void AntRadioStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntRadio*>(widget))
    {
        widget->removeEventFilter(this);
    }
    AntStyleBase::unpolish(widget);
}

void AntRadioStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::CE_RadioButton)
    {
        const auto* radio = qobject_cast<const AntRadio*>(widget);
        const auto* bopt = qstyleoption_cast<const QStyleOptionButton*>(option);
        if (!radio || !bopt || !painter)
        {
            return;
        }

        const auto& token = antTheme->tokens();
        const bool enabled = bopt->state.testFlag(QStyle::State_Enabled);
        const bool hovered = bopt->state.testFlag(QStyle::State_MouseOver);
        const bool pressed = bopt->state.testFlag(QStyle::State_Sunken);
        if (radio->isButtonStyle())
        {
            const QRectF frame = radio->buttonFrameRect();
            const QPainterPath& path = radio->buttonSegmentPath();
            QColor border = token.colorBorder;
            QColor bg = token.colorBgContainer;
            QColor text = token.colorText;
            if (!enabled)
            {
                border = token.colorBorderDisabled;
                bg = token.colorBgContainerDisabled;
                text = token.colorTextDisabled;
            }
            else if (radio->isChecked())
            {
                border = token.colorPrimary;
                bg = pressed ? token.colorPrimaryActive : (hovered ? token.colorPrimaryHover : token.colorPrimary);
                text = token.colorTextLightSolid;
            }
            else if (hovered)
            {
                border = token.colorPrimary;
                text = token.colorPrimary;
            }

            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
            painter->setPen(QPen(border, token.lineWidth));
            painter->setBrush(bg);
            painter->drawPath(path);

            painter->setFont(radio->radioFont());
            painter->setPen(text);
            painter->drawText(frame, Qt::AlignCenter, radio->text());
            painter->restore();
            return;
        }

        const QRectF circle = radio->indicatorRect();

        QColor border;
        QColor background;
        if (!enabled)
        {
            border = token.colorBorder;
            background = token.colorBgContainerDisabled;
        }
        else if (radio->isChecked())
        {
            border = hovered ? token.colorPrimaryHover : token.colorPrimary;
            background = token.colorBgContainer;
            Q_UNUSED(pressed)
        }
        else
        {
            border = hovered ? token.colorPrimary : token.colorBorder;
            background = token.colorBgContainer;
        }

        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        painter->setPen(QPen(border, token.lineWidth));
        painter->setBrush(background);
        painter->drawEllipse(circle);

        if (radio->isChecked())
        {
            const qreal dotSize = RadioSize * DotRatio;
            const QRectF dot(circle.center().x() - dotSize / 2.0, circle.center().y() - dotSize / 2.0, dotSize, dotSize);
            painter->setPen(Qt::NoPen);
            painter->setBrush(enabled ? token.colorPrimary : token.colorTextDisabled);
            painter->drawEllipse(dot);
        }

        if (bopt->state.testFlag(QStyle::State_HasFocus) && enabled)
        {
            const QColor focus = AntPalette::alpha(token.colorPrimary, 0.10);
            painter->setPen(QPen(focus, token.controlOutlineWidth));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(circle.adjusted(-2, -2, 2, 2));
        }

        if (!radio->text().isEmpty())
        {
            painter->setFont(radio->radioFont());
            painter->setPen(enabled ? token.colorText : token.colorTextDisabled);
            painter->drawText(radio->textRect(), Qt::AlignLeft | Qt::AlignVCenter, radio->text());
        }
        painter->restore();
        return;
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}

int AntRadioStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    if (qobject_cast<const AntRadio*>(widget))
    {
        switch (metric)
        {
        case QStyle::PM_ExclusiveIndicatorWidth:
        case QStyle::PM_ExclusiveIndicatorHeight:
            return RadioSize;
        case QStyle::PM_RadioButtonLabelSpacing:
            return TextSpacing;
        default:
            break;
        }
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

bool AntRadioStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* radio = qobject_cast<AntRadio*>(watched);
    if (radio && event->type() == QEvent::Paint)
    {
        QStyleOptionButton option;
        option.initFrom(radio);
        option.rect = radio->rect();
        option.text = radio->text();
        option.state |= radio->isChecked() ? QStyle::State_On : QStyle::State_Off;
        if (radio->isHoveredState())
        {
            option.state |= QStyle::State_MouseOver;
        }
        if (radio->isPressedState())
        {
            option.state |= QStyle::State_Sunken;
        }

        QPainter painter(radio);
        drawControl(QStyle::CE_RadioButton, &option, &painter, radio);
        return false;
    }
    return QProxyStyle::eventFilter(watched, event);
}
