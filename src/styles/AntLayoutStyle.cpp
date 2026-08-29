#include "AntLayoutStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntLayout.h"

AntLayoutStyle::AntLayoutStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntLayout>();
}

void AntLayoutStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    installPaintFilter<AntLayout>(widget);
}

void AntLayoutStyle::unpolish(QWidget* widget)
{
    removePaintFilter<AntLayout>(widget);
    AntStyleBase::unpolish(widget);
}

void AntLayoutStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntLayout*>(widget))
    {
        drawLayout(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntLayoutStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntLayoutStyle::drawWidget(QWidget* widget, QPaintEvent* event)
{
    auto* layout = qobject_cast<AntLayout*>(widget);
    if (!layout)
    {
        return false;
    }

    QStyleOption option;
    option.initFrom(layout);
    option.rect = layout->rect();
    QPainter painter(layout);
    drawPrimitive(QStyle::PE_Widget, &option, &painter, layout);

    return true;
}

void AntLayoutStyle::drawLayout(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (!painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    // Draw layout background
    painter->setPen(Qt::NoPen);
    painter->setBrush(token.colorBgLayout);
    const auto* layout = qobject_cast<const AntLayout*>(widget);
    const int radius = layout ? layout->borderRadius() : 0;
    if (radius > 0)
    {
        painter->drawRoundedRect(QRectF(option->rect), radius, radius);
    }
    else
    {
        painter->drawRect(option->rect);
    }

    painter->restore();
}
