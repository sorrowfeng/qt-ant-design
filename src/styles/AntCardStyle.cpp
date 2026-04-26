#include "AntCardStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntCard.h"

AntCardStyle::AntCardStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntCard>();
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
    QRect cardRect = option->rect;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Hover shadow
    const bool hovered = option->state & QStyle::State_MouseOver;
    if (card->isHoverable() && hovered)
    {
        antTheme->drawEffectShadow(painter, option->rect, 12, radius, 1.35);
        cardRect.adjust(2, 2, -2, -2);
    }

    // Card border and background
    AntStyleBase::drawCrispRoundedRect(painter, cardRect,
        card->isBordered() ? QPen(token.colorBorderSecondary, token.lineWidth) : Qt::NoPen,
        token.colorBgContainer, radius, radius);

    // Header separator line
    QWidget* headerWidget = card->findChild<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    if (headerWidget && headerWidget->isVisible())
    {
        painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter->drawLine(cardRect.left() + 1, headerWidget->geometry().bottom(),
                         cardRect.right() - 1, headerWidget->geometry().bottom());
    }

    // Loading overlay
    if (card->isLoading())
    {
        QColor mask = token.colorBgContainer;
        mask.setAlphaF(0.72);
        AntStyleBase::drawCrispRoundedRect(painter, cardRect, Qt::NoPen, mask, radius, radius);

        // Spinner
        painter->setPen(QPen(token.colorPrimary, 3, Qt::SolidLine, Qt::RoundCap));
        painter->setBrush(Qt::NoBrush);
        const QRectF spinnerRect(option->rect.width() / 2.0 - 14, option->rect.height() / 2.0 - 14, 28, 28);
        painter->drawArc(spinnerRect, 0, 280 * 16);
    }

    painter->restore();
}
