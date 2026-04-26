#include "AntCalendarStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntCalendar.h"

AntCalendarStyle::AntCalendarStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntCalendar>();
}

void AntCalendarStyle::onThemeUpdate(QWidget* w)
{
    w->update();
}

void AntCalendarStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntCalendar*>(widget))
        widget->installEventFilter(this);
}

void AntCalendarStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntCalendar*>(widget))
        widget->removeEventFilter(this);
    QProxyStyle::unpolish(widget);
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

bool AntCalendarStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* cal = qobject_cast<AntCalendar*>(watched);
    if (cal && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(cal);
        option.rect = cal->rect();
        QPainter painter(cal);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, cal);
        return false;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntCalendarStyle::drawFrame(const QStyleOption* option, QPainter* painter) const
{
    if (!painter || !option) return;

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    antTheme->drawEffectShadow(painter, option->rect, 6, 8, 0.12);

    painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
    painter->setBrush(token.colorBgElevated);
    painter->drawRoundedRect(option->rect, 8, 8);

    painter->restore();
}
