#include "AntStyleBase.h"

#include <QEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>

bool AntStyleBase::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Paint)
    {
        auto* widget = qobject_cast<QWidget*>(watched);
        if (widget && widget->style() == this)
        {
            auto* paintEvent = static_cast<QPaintEvent*>(event);
            if (drawWidget(widget, paintEvent))
            {
                return true;
            }
        }
    }
    return QProxyStyle::eventFilter(watched, event);
}

bool AntStyleBase::drawWidget(QWidget* /*widget*/, QPaintEvent* /*event*/)
{
    return false;
}
