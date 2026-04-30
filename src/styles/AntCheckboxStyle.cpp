#include "AntCheckboxStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOptionButton>

#include "styles/AntIconPainter.h"
#include "styles/AntPalette.h"
#include "widgets/AntCheckbox.h"

namespace
{
constexpr int IndicatorSize = 16;
constexpr int TextSpacing = 8;
}

AntCheckboxStyle::AntCheckboxStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntCheckbox>();
}

void AntCheckboxStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntCheckbox*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

void AntCheckboxStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntCheckbox*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntCheckboxStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::CE_CheckBox)
    {
        const auto* checkbox = qobject_cast<const AntCheckbox*>(widget);
        const auto* bopt = qstyleoption_cast<const QStyleOptionButton*>(option);
        if (!checkbox || !bopt || !painter)
        {
            return;
        }

        const auto& token = antTheme->tokens();
        const bool enabled = bopt->state.testFlag(QStyle::State_Enabled);
        const bool hovered = bopt->state.testFlag(QStyle::State_MouseOver);
        const bool pressed = bopt->state.testFlag(QStyle::State_Sunken);
        const QRectF box(0.5, (bopt->rect.height() - IndicatorSize) / 2.0 + 0.5, IndicatorSize - 1, IndicatorSize - 1);

        QColor border;
        QColor background;
        if (!enabled)
        {
            border = token.colorBorder;
            background = token.colorBgContainerDisabled;
        }
        else if (checkbox->isChecked() && !checkbox->isIndeterminate())
        {
            border = hovered ? token.colorPrimaryHover : token.colorPrimary;
            background = (hovered || pressed) ? token.colorPrimaryHover : token.colorPrimary;
        }
        else
        {
            border = hovered ? token.colorPrimary : token.colorBorder;
            background = token.colorBgContainer;
        }

        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        AntStyleBase::drawCrispRoundedRect(painter, box.toRect(),
            QPen(border, token.lineWidth), background, token.borderRadiusSM, token.borderRadiusSM);

        if (checkbox->isChecked() && !checkbox->isIndeterminate())
        {
            AntIconPainter::drawIcon(*painter,
                                     Ant::IconType::Check,
                                     box.adjusted(3, 3, -3, -3),
                                     enabled ? token.colorTextLightSolid : token.colorTextDisabled);
        }
        else if (checkbox->isIndeterminate())
        {
            const QRectF mark(box.left() + 4, box.center().y() - 1.5, box.width() - 8, 3);
            AntStyleBase::drawCrispRoundedRect(painter, mark.toRect(),
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
            const QRectF textRect(box.right() + TextSpacing, 0, bopt->rect.width() - box.right() - TextSpacing, bopt->rect.height());
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, checkbox->text());
        }
        painter->restore();
        return;
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}

int AntCheckboxStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    if (qobject_cast<const AntCheckbox*>(widget))
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

bool AntCheckboxStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* checkbox = qobject_cast<AntCheckbox*>(watched);
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
