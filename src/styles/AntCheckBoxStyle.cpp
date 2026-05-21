#include "AntCheckBoxStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOptionButton>

#include "styles/AntPalette.h"
#include "widgets/AntCheckBox.h"

namespace
{
constexpr int IndicatorSize = 16;
constexpr int TextSpacing = 8;
}

AntCheckBoxStyle::AntCheckBoxStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntCheckBoxStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntCheckBox*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

void AntCheckBoxStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntCheckBox*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntCheckBoxStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::CE_CheckBox)
    {
        const auto* checkbox = qobject_cast<const AntCheckBox*>(widget);
        const auto* bopt = qstyleoption_cast<const QStyleOptionButton*>(option);
        if (!checkbox || !bopt || !painter)
        {
            return;
        }

        const auto& token = antTheme->tokens();
        const bool enabled = bopt->state.testFlag(QStyle::State_Enabled);
        const auto& layout = checkbox->layoutData();
        const QRectF box = layout.indicatorRect;

        const QColor border = checkbox->indicatorBorderColor();
        const QColor background = checkbox->indicatorBackgroundColor();

        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        AntStyleBase::drawCrispRoundedRect(painter, box.toRect(),
            QPen(border, token.lineWidth), background, token.borderRadiusSM, token.borderRadiusSM);

        if (checkbox->isChecked() && !checkbox->isIndeterminate())
        {
            painter->setPen(QPen(enabled ? token.colorTextLightSolid : token.colorTextDisabled,
                                  2.0,
                                  Qt::SolidLine,
                                  Qt::RoundCap,
                                  Qt::RoundJoin));
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(layout.checkPath);
        }
        else if (checkbox->isIndeterminate())
        {
            AntStyleBase::drawCrispRoundedRect(painter, layout.indeterminateRect.toRect(),
                Qt::NoPen, enabled ? token.colorPrimary : token.colorTextDisabled, 1.5, 1.5);
        }

        if (bopt->state.testFlag(QStyle::State_HasFocus) && enabled)
        {
            const QColor focus = AntPalette::alpha(token.colorPrimary, 0.10);
            AntStyleBase::drawCrispRoundedRect(painter, box.toRect(),
                QPen(focus, token.controlOutlineWidth), Qt::NoBrush,
                token.borderRadiusSM + 2, token.borderRadiusSM + 2);
        }

        if (!checkbox->text().isEmpty())
        {
            QFont f = painter->font();
            f.setPixelSize(token.fontSize);
            painter->setFont(f);
            painter->setPen(enabled ? token.colorText : token.colorTextDisabled);
            painter->drawText(layout.textRect, Qt::AlignLeft | Qt::AlignVCenter, checkbox->text());
        }
        painter->restore();
        return;
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}

int AntCheckBoxStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    if (qobject_cast<const AntCheckBox*>(widget))
    {
        switch (metric)
        {
        case QStyle::PM_IndicatorWidth:
        case QStyle::PM_IndicatorHeight:
            return IndicatorSize;
        case QStyle::PM_CheckBoxLabelSpacing:
            return TextSpacing;
        default:
            break;
        }
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

bool AntCheckBoxStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* checkbox = qobject_cast<AntCheckBox*>(watched);
    if (checkbox && event->type() == QEvent::Paint)
    {
        QStyleOptionButton option;
        option.initFrom(checkbox);
        option.rect = checkbox->rect();
        option.text = checkbox->text();
        if (checkbox->isChecked())
        {
            option.state |= QStyle::State_On;
        }
        else
        {
            option.state |= QStyle::State_Off;
        }
        if (checkbox->isIndeterminate())
        {
            option.state |= QStyle::State_NoChange;
        }
        if (checkbox->isHoveredState())
        {
            option.state |= QStyle::State_MouseOver;
        }
        if (checkbox->isPressedState())
        {
            option.state |= QStyle::State_Sunken;
        }

        QPainter painter(checkbox);
        drawControl(QStyle::CE_CheckBox, &option, &painter, checkbox);
        return false;
    }
    return QProxyStyle::eventFilter(watched, event);
}
