#include "AntStyleBase.h"

#include <QEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>
#include <QStyleFactory>
#include <QStringList>

namespace
{
QStyle* createDetachedBaseStyle(QStyle* sourceStyle)
{
    auto createByName = [](const QString& name) -> QStyle* {
        return name.isEmpty() ? nullptr : QStyleFactory::create(name);
    };

    if (sourceStyle)
    {
        if (auto* style = createByName(sourceStyle->objectName()))
            return style;

        if (auto* sourceProxy = qobject_cast<QProxyStyle*>(sourceStyle))
        {
            if (QStyle* baseStyle = sourceProxy->baseStyle())
            {
                if (auto* style = createByName(baseStyle->objectName()))
                    return style;
            }
        }
    }

    if (QApplication::instance())
    {
        if (QStyle* appStyle = QApplication::style())
        {
            if (auto* style = createByName(appStyle->objectName()))
                return style;
        }
    }

    if (auto* style = QStyleFactory::create(QStringLiteral("Fusion")))
        return style;

    const QStringList keys = QStyleFactory::keys();
    return keys.isEmpty() ? nullptr : QStyleFactory::create(keys.constFirst());
}
}

AntStyleBase::AntStyleBase(QStyle* style)
    // QProxyStyle owns its base style, so never pass a shared QApplication/widget style directly.
    : QProxyStyle(createDetachedBaseStyle(style))
{
}

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
