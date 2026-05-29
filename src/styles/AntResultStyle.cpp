#include "AntResultStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntResult.h"

AntResultStyle::AntResultStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntResultStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    if (qobject_cast<AntResult*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntResultStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntResult*>(widget))
    {
        widget->removeEventFilter(this);
    }
    AntStyleBase::unpolish(widget);
}

void AntResultStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntResult*>(widget))
    {
        drawResult(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntResultStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntResultStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* result = qobject_cast<AntResult*>(watched);
    if (result && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(result);
        option.rect = result->rect();
        QPainter painter(result);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, result);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntResultStyle::drawResult(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* result = qobject_cast<const AntResult*>(widget);
    if (!result || !painter || !option)
    {
        return;
    }

    const auto& layout = result->resultLayout();
    const QFont widgetFont = result->font();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (layout.iconVisible)
    {
        painter->drawPixmap(layout.iconRect, result->statusIconPixmap(result->devicePixelRatioF()));
    }

    QFont titleFont = widgetFont;
    titleFont.setPixelSize(layout.metrics.titleFontSize);
    titleFont.setWeight(QFont::DemiBold);
    painter->setFont(titleFont);
    painter->setPen(layout.titleColor);
    painter->drawText(layout.titleRect, Qt::AlignCenter | Qt::TextWordWrap, layout.title);

    if (!layout.subTitle.isEmpty())
    {
        QFont subFont = widgetFont;
        subFont.setPixelSize(layout.metrics.subTitleFontSize);
        subFont.setWeight(QFont::Normal);
        painter->setFont(subFont);
        painter->setPen(layout.subTitleColor);
        painter->drawText(layout.subTitleRect, Qt::AlignCenter | Qt::TextWordWrap, layout.subTitle);
    }

    painter->restore();
}
