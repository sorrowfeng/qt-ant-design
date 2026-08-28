#include "AntCalendarStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntCalendar.h"

AntCalendarStyle::AntCalendarStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntCalendarStyle::onThemeUpdate(QWidget* w)
{
    w->update();
}

void AntCalendarStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    if (qobject_cast<AntCalendar*>(widget))
        installPaintFilter<AntCalendar>(widget);
}

void AntCalendarStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntCalendar*>(widget))
        removePaintFilter<AntCalendar>(widget);
    AntStyleBase::unpolish(widget);
}

void AntCalendarStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                                      QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntCalendar*>(widget))
    {
        drawFrame(option, painter);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

bool AntCalendarStyle::drawWidget(QWidget* widget, QPaintEvent* event)
{
    auto* cal = qobject_cast<AntCalendar*>(widget);
    if (!cal)
    {
        return false;
    }

    QStyleOption option;
    option.initFrom(cal);
    option.rect = cal->rect();
    QPainter painter(cal);
    drawPrimitive(QStyle::PE_Widget, &option, &painter, cal);

    return false;
}

void AntCalendarStyle::drawFrame(const QStyleOption* option, QPainter* painter) const
{
    if (!painter || !option) return;

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    AntStyleBase::drawCrispRoundedRect(painter, option->rect,
        QPen(token.colorBorderSecondary, token.lineWidth),
        token.colorBgElevated, 8, 8);

    painter->restore();
}
