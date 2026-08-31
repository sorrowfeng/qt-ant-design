#include "AntCardStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "core/AntSpinner.h"
#include "styles/AntPalette.h"
#include "widgets/AntCard.h"

AntCardStyle::AntCardStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntCardStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    installPaintFilter<AntCard>(widget);
}

void AntCardStyle::unpolish(QWidget* widget)
{
    removePaintFilter<AntCard>(widget);
    AntStyleBase::unpolish(widget);
}

void AntCardStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntCard*>(widget))
    {
        drawCard(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntCardStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntCardStyle::drawWidget(QWidget* widget, QPaintEvent* event)
{
    auto* card = qobject_cast<AntCard*>(widget);
    if (!card)
    {
        return false;
    }

    QStyleOption option;
    option.initFrom(card);
    option.rect = card->rect();
    QPainter painter(card);
    drawPrimitive(QStyle::PE_Widget, &option, &painter, card);

    return true;
}

void AntCardStyle::drawCard(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* card = qobject_cast<const AntCard*>(widget);
    if (!card || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const int radius = token.borderRadiusLG;
    const auto& cache = card->cardPaintCache(option->rect);
    const QRect cardRect = cache.cardRect;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Hover shadow（上游 hoverable:hover：边框透明 + cardShadow）
    const bool hovered = card->isHoverable() && (option->state & QStyle::State_MouseOver);
    if (hovered)
    {
        antTheme->drawEffectShadow(painter, cardRect, 12, radius, 1.35);
    }

    // Card border and background（hover 时边框透明，贴近上游 hoverable 语义）
    AntStyleBase::drawCrispRoundedRect(painter, cardRect,
        card->isBordered() && !hovered
            ? QPen(token.colorBorderSecondary, token.lineWidth)
            : Qt::NoPen,
        token.colorBgContainer, radius, radius);

    // Header separator line
    if (cache.headerVisible)
    {
        painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter->drawLine(cardRect.left() + 1, cache.headerSeparatorY,
                         cardRect.right() - 1, cache.headerSeparatorY);
    }

    if (cache.actionsVisible)
    {
        painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter->drawLine(cardRect.left() + 1, cache.actionsSeparatorY,
                          cardRect.right() - 1, cache.actionsSeparatorY);

        const QRect actionsRect = card->m_actions->geometry();
        for (const int x : cache.actionSeparatorXs)
        {
            painter->drawLine(x, actionsRect.top() + 12, x, actionsRect.bottom() - 12);
        }
    }

    // Card.Grid 内部 1px 分隔线（对应上游 contain-grid 的 box-shadow 网格线；
    // 最外侧由卡片边框承担，这里只画内部边界）。网格范围收在 cardRect 内，
    // 避免 hoverable 缩进时线条越过边框。
    if (cache.gridMode)
    {
        const QRect bodyRect = card->m_body->geometry();
        const QRect gridArea = bodyRect.intersected(cardRect);
        painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        for (const int x : cache.gridColumnXs)
        {
            if (x >= gridArea.left() && x <= gridArea.right())
                painter->drawLine(x, gridArea.top(), x, gridArea.bottom());
        }
        for (const int y : cache.gridRowYs)
        {
            if (y >= gridArea.top() && y <= gridArea.bottom())
                painter->drawLine(gridArea.left(), y, gridArea.right(), y);
        }
    }

    // Loading overlay
    if (card->isLoading())
    {
        QColor mask = token.colorBgContainer;
        mask.setAlphaF(0.72);
        AntStyleBase::drawCrispRoundedRect(painter, cardRect, Qt::NoPen, mask, radius, radius);

        AntSpinner::drawArc(painter, cache.spinnerRect, token.colorPrimary,
                            card->m_spinner.angle(), 280, 3.0);
    }

    painter->restore();
}
