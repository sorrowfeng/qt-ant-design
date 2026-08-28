#include "AntToolTipStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntToolTip.h"

AntToolTipStyle::AntToolTipStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntToolTipStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    installPaintFilter<AntToolTip>(widget);
}

void AntToolTipStyle::unpolish(QWidget* widget)
{
    removePaintFilter<AntToolTip>(widget);
    AntStyleBase::unpolish(widget);
}

void AntToolTipStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntToolTip*>(widget))
    {
        drawTooltip(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntToolTipStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntToolTipStyle::drawWidget(QWidget* widget, QPaintEvent* event)
{
    auto* tooltip = qobject_cast<AntToolTip*>(widget);
    if (!tooltip)
    {
        return false;
    }

    QStyleOption option;
    option.initFrom(tooltip);
    option.rect = tooltip->rect();
    QPainter painter(tooltip);
    drawPrimitive(QStyle::PE_Widget, &option, &painter, tooltip);

    return true;
}

void AntToolTipStyle::drawTooltip(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* tooltip = qobject_cast<const AntToolTip*>(widget);
    if (!tooltip || !painter || !option)
    {
        return;
    }

    Q_UNUSED(option)
    const auto& layout = tooltip->tooltipLayout();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    painter->setPen(Qt::NoPen);
    painter->setBrush(layout.bubbleColor);
    painter->drawRoundedRect(layout.bubbleRect, layout.tokenBorderRadiusSM, layout.tokenBorderRadiusSM);

    if (layout.arrowVisible)
    {
        painter->drawPolygon(layout.arrowPolygon);
    }

    painter->setPen(layout.textColor);
    const QFont textFont = AntStyleBase::withPixelSize(painter->font(), layout.tokenFontSizeSM);
    painter->setFont(textFont);
    painter->drawText(layout.textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, layout.title);

    painter->restore();
}
