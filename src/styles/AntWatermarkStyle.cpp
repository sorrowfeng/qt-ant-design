#include "AntWatermarkStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntWatermark.h"

AntWatermarkStyle::AntWatermarkStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntWatermarkStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    installPaintFilter<AntWatermark>(widget);
}

void AntWatermarkStyle::unpolish(QWidget* widget)
{
    removePaintFilter<AntWatermark>(widget);
    AntStyleBase::unpolish(widget);
}

void AntWatermarkStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntWatermark*>(widget))
    {
        drawWatermark(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntWatermarkStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntWatermarkStyle::drawWidget(QWidget* widget, QPaintEvent* event)
{
    auto* wm = qobject_cast<AntWatermark*>(widget);
    if (!wm)
    {
        return false;
    }

    QStyleOption option;
    option.initFrom(wm);
    option.rect = wm->rect();
    QPainter painter(wm);
    drawPrimitive(QStyle::PE_Widget, &option, &painter, wm);

    return true;
}

void AntWatermarkStyle::drawWatermark(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* wm = qobject_cast<const AntWatermark*>(widget);
    if (!wm || !painter || !option) return;

    if (wm->content().isEmpty())
    {
        return;
    }

    const QPixmap pixmap = wm->watermarkPixmap(wm->devicePixelRatioF());
    if (pixmap.isNull())
    {
        return;
    }

    painter->drawPixmap(option->rect.topLeft(), pixmap);
}
