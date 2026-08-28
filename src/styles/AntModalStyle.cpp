#include "AntModalStyle.h"

#include <QEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntModal.h"

AntModalStyle::AntModalStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntModalStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    installPaintFilter<AntModal>(widget);
}

void AntModalStyle::unpolish(QWidget* widget)
{
    removePaintFilter<AntModal>(widget);
    AntStyleBase::unpolish(widget);
}

void AntModalStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntModal*>(widget))
    {
        drawModal(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntModalStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntModalStyle::drawWidget(QWidget* widget, QPaintEvent* event)
{
    auto* modal = qobject_cast<AntModal*>(widget);
    if (!modal)
    {
        return false;
    }

    QStyleOption option;
    option.initFrom(modal);
    option.rect = event->rect();
    QPainter painter(modal);
    drawPrimitive(QStyle::PE_Widget, &option, &painter, modal);

    return true;
}

void AntModalStyle::drawModal(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* modal = qobject_cast<const AntModal*>(widget);
    if (!modal || !painter || !option)
    {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    const qreal baseOpacity = antTheme->isDarkMode() ? 0.58 : 0.45;
    // Fade mask alongside the open/close animation
    const qreal opacity = baseOpacity * modal->animationProgress();
    painter->fillRect(option->rect, AntPalette::alpha(Qt::black, opacity));
    painter->restore();
}
