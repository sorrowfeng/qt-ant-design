#include "AntSpaceStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntSpace.h"

AntSpaceStyle::AntSpaceStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntSpace>();
}

void AntSpaceStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntSpace*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntSpaceStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntSpace*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntSpaceStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntSpace*>(widget))
    {
        drawSpace(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntSpaceStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntSpaceStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* space = qobject_cast<AntSpace*>(watched);
    if (space && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(space);
        option.rect = space->rect();
        QPainter painter(space);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, space);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntSpaceStyle::drawSpace(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    Q_UNUSED(widget)

    if (!painter || !option)
    {
        return;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    // Space container is transparent by default; no background painting needed.
    // Layout and spacing are handled by QBoxLayout in AntSpace.

    painter->restore();
}
