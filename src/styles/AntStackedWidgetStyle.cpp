#include "AntStackedWidgetStyle.h"

#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntStackedWidget.h"

AntStackedWidgetStyle::AntStackedWidgetStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntStackedWidget>();
}

void AntStackedWidgetStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    installPaintFilter<AntStackedWidget>(widget);
}

void AntStackedWidgetStyle::unpolish(QWidget* widget)
{
    removePaintFilter<AntStackedWidget>(widget);
    AntStyleBase::unpolish(widget);
}

void AntStackedWidgetStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                                          QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntStackedWidget*>(widget))
    {
        drawStackedWidgetFrame(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

bool AntStackedWidgetStyle::drawWidget(QWidget* widget, QPaintEvent* event)
{
    Q_UNUSED(event)
    auto* stacked = qobject_cast<AntStackedWidget*>(widget);
    if (!stacked)
    {
        return false;
    }
    if (stacked->variant() == Ant::Variant::Borderless)
    {
        return false;
    }

    QStyleOption option;
    option.initFrom(stacked);
    option.rect = stacked->rect();

    QPainter painter(stacked);
    drawPrimitive(QStyle::PE_Widget, &option, &painter, stacked);
    return true;
}

void AntStackedWidgetStyle::drawStackedWidgetFrame(const QStyleOption* option, QPainter* painter,
                                                   const QWidget* widget) const
{
    const auto* stacked = qobject_cast<const AntStackedWidget*>(widget);
    if (!stacked || !option || !painter)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const bool enabled = option->state.testFlag(QStyle::State_Enabled);
    const bool hovered = option->state.testFlag(QStyle::State_MouseOver);
    QColor background = enabled ? token.colorBgContainer : token.colorBgContainerDisabled;
    QColor border = enabled ? token.colorBorderSecondary : token.colorBorderDisabled;

    switch (stacked->variant())
    {
    case Ant::Variant::Borderless:
        border = Qt::transparent;
        background = Qt::transparent;
        break;
    case Ant::Variant::Filled:
        border = Qt::transparent;
        background = enabled ? token.colorFillQuaternary : token.colorBgContainerDisabled;
        break;
    case Ant::Variant::Underlined:
        border = enabled ? token.colorSplit : token.colorBorderDisabled;
        background = Qt::transparent;
        break;
    case Ant::Variant::Outlined:
        if (hovered && enabled)
        {
            border = token.colorPrimaryHover;
        }
        break;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    const QRect rect = option->rect;
    if (stacked->variant() == Ant::Variant::Underlined)
    {
        if (background.alpha() > 0)
        {
            painter->fillRect(rect, background);
        }
        painter->setPen(QPen(border, token.lineWidth));
        painter->drawLine(rect.bottomLeft(), rect.bottomRight());
    }
    else if (stacked->variant() == Ant::Variant::Borderless)
    {
        if (background.alpha() > 0)
        {
            painter->fillRect(rect, background);
        }
    }
    else
    {
        AntStyleBase::drawCrispRoundedRect(painter, rect,
                                           border.alpha() > 0 ? QPen(border, token.lineWidth) : Qt::NoPen,
                                           background,
                                           token.borderRadiusLG,
                                           token.borderRadiusLG);
    }

    if (stacked->hasFocus() && stacked->variant() == Ant::Variant::Outlined && enabled)
    {
        AntStyleBase::drawCrispRoundedRect(painter, rect.adjusted(1, 1, -1, -1),
                                           QPen(AntPalette::alpha(token.colorPrimary, 0.18),
                                                token.controlOutlineWidth),
                                           Qt::NoBrush,
                                           token.borderRadiusLG,
                                           token.borderRadiusLG);
    }

    painter->restore();
}
