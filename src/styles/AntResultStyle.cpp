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
    installPaintFilter<AntResult>(widget);
}

void AntResultStyle::unpolish(QWidget* widget)
{
    removePaintFilter<AntResult>(widget);
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

bool AntResultStyle::drawWidget(QWidget* widget, QPaintEvent* event)
{
    auto* result = qobject_cast<AntResult*>(widget);
    if (!result)
    {
        return false;
    }

    QStyleOption option;
    option.initFrom(result);
    option.rect = result->rect();
    QPainter painter(result);
    drawPrimitive(QStyle::PE_Widget, &option, &painter, result);

    return true;
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

    const QFont titleFont = AntStyleBase::withPixelSize(widgetFont, layout.metrics.titleFontSize, QFont::DemiBold);
    painter->setFont(titleFont);
    painter->setPen(layout.titleColor);
    painter->drawText(layout.titleRect, Qt::AlignCenter | Qt::TextWordWrap, layout.title);

    if (!layout.subTitle.isEmpty())
    {
        const QFont subFont = AntStyleBase::withPixelSize(widgetFont, layout.metrics.subTitleFontSize, QFont::Normal);
        painter->setFont(subFont);
        painter->setPen(layout.subTitleColor);
        painter->drawText(layout.subTitleRect, Qt::AlignCenter | Qt::TextWordWrap, layout.subTitle);
    }

    painter->restore();
}
