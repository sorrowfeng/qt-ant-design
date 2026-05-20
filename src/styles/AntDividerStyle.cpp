#include "AntDividerStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntDivider.h"

AntDividerStyle::AntDividerStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntDivider>();
}

void AntDividerStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntDivider*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntDividerStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntDivider*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntDividerStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntDivider*>(widget))
    {
        drawDivider(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntDividerStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntDividerStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* divider = qobject_cast<AntDivider*>(watched);
    if (divider && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(divider);
        option.rect = divider->rect();
        QPainter painter(divider);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, divider);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntDividerStyle::drawDivider(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* divider = qobject_cast<const AntDivider*>(widget);
    if (!divider || !painter || !option)
    {
        return;
    }

    const auto& cache = divider->paintCache(option->rect);
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    if (cache.horizontal)
    {
        painter->setPen(cache.linePen);
        if (!cache.hasTitle)
        {
            painter->drawLine(cache.firstLine);
        }
        else
        {
            painter->drawLine(cache.firstLine);
            painter->drawLine(cache.secondLine);

            painter->setPen(cache.textColor);
            painter->setFont(cache.titleFont);
            painter->drawText(cache.textRect, Qt::AlignCenter, cache.text);
        }
    }
    else
    {
        painter->setPen(cache.linePen);
        painter->drawLine(cache.firstLine);
    }

    painter->restore();
}
