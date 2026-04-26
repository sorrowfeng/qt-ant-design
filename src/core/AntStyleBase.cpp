#include "AntStyleBase.h"

#include <QEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>

void AntStyleBase::drawCrispRoundedRect(QPainter* painter, const QRect& rect,
    const QPen& pen, const QBrush& brush, qreal rx, qreal ry)
{
    painter->setPen(pen);
    painter->setBrush(brush);
    if (pen.style() != Qt::NoPen && pen.widthF() > 0)
        painter->drawRoundedRect(QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5), rx, ry);
    else
        painter->drawRoundedRect(QRectF(rect), rx, ry);
}

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
