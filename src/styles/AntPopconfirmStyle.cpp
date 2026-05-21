#include "AntPopconfirmStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntPopconfirm.h"

AntPopconfirmStyle::AntPopconfirmStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntPopconfirmStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
}

void AntPopconfirmStyle::unpolish(QWidget* widget)
{
    QProxyStyle::unpolish(widget);
}

void AntPopconfirmStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntPopconfirm*>(widget))
    {
        drawPopconfirm(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntPopconfirmStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntPopconfirmStyle::eventFilter(QObject* watched, QEvent* event)
{
    return QProxyStyle::eventFilter(watched, event);
}

void AntPopconfirmStyle::drawPopconfirm(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    Q_UNUSED(option)
    Q_UNUSED(painter)
    Q_UNUSED(widget)
    // AntPopconfirm has no own paintEvent - it delegates visual rendering to its child AntPopover.
    // Nothing to draw here.
}
