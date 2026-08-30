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

    // antd v6 mask.blur：遮罩下内容做毛玻璃模糊。Qt 自绘遮罩层无法直接
    // 采样底层像素，这里用一层随透明度衰减的白色雾化近似 blur 的视觉观感，
    // 叠加在黑色遮罩上以弱化边缘对比，模拟 backdrop-filter 的柔和效果。
    if (modal->mask().blur)
    {
        const qreal haze = 0.22 * modal->animationProgress();
        painter->fillRect(option->rect, AntPalette::alpha(Qt::white, haze));
    }
    painter->restore();
}
