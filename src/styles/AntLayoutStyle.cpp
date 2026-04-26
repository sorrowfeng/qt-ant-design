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
    QProxyStyle::polish(widget);
    if (qobject_cast<AntLayout*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntLayoutStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntLayout*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
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

bool AntLayoutStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* layout = qobject_cast<AntLayout*>(watched);
    if (layout && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(layout);
        option.rect = layout->rect();
        QPainter painter(layout);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, layout);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntLayoutStyle::drawLayout(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    Q_UNUSED(widget)
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
    painter->drawRect(option->rect);

    painter->restore();
}
