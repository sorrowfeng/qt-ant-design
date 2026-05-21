#include "AntCardStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntCard.h"

AntCardStyle::AntCardStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntCardStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntCard*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntCardStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntCard*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
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

bool AntCardStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* card = qobject_cast<AntCard*>(watched);
    if (card && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(card);
        option.rect = card->rect();
        QPainter painter(card);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, card);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
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

    // Hover shadow
    if (card->isHoverable() && (option->state & QStyle::State_MouseOver))
    {
        antTheme->drawEffectShadow(painter, cardRect, 12, radius, 1.35);
    }

    // Card border and background
    AntStyleBase::drawCrispRoundedRect(painter, cardRect,
        card->isBordered() ? QPen(token.colorBorderSecondary, token.lineWidth) : Qt::NoPen,
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

    // Loading overlay
    if (card->isLoading())
    {
        QColor mask = token.colorBgContainer;
        mask.setAlphaF(0.72);
        AntStyleBase::drawCrispRoundedRect(painter, cardRect, Qt::NoPen, mask, radius, radius);

        painter->setPen(QPen(token.colorPrimary, 3, Qt::SolidLine, Qt::RoundCap));
        painter->setBrush(Qt::NoBrush);
        painter->drawArc(cache.spinnerRect, card->m_spinnerAngle * 16, 280 * 16);
    }

    painter->restore();
}
