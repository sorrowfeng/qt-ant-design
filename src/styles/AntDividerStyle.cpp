#include "AntDividerStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntDivider.h"

AntDividerStyle::AntDividerStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntDivider>();
}

void AntDividerStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntDivider*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntDividerStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntDivider*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntDividerStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntDivider*>(widget))
    {
        drawDivider(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntDividerStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntDividerStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* divider = qobject_cast<AntDivider*>(watched);
    if (divider && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(divider);
        option.rect = divider->rect();
        QPainter painter(divider);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, divider);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntDividerStyle::drawDivider(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* divider = qobject_cast<const AntDivider*>(widget);
    if (!divider || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    const bool horizontal = divider->orientation() == Ant::Orientation::Horizontal;
    const bool hasTitle = !divider->text().isEmpty();
    const QColor lineColor = token.colorSplit;
    const QColor textColor = token.colorText;

    Qt::PenStyle penStyle = Qt::SolidLine;
    if (divider->variant() == Ant::DividerVariant::Dashed)
    {
        penStyle = Qt::DashLine;
    }
    else if (divider->variant() == Ant::DividerVariant::Dotted)
    {
        penStyle = Qt::DotLine;
    }

    const int lineWidth = token.lineWidth;

    if (horizontal)
    {
        const int y = option->rect.height() / 2;

        if (!hasTitle)
        {
            painter->setPen(QPen(lineColor, lineWidth, penStyle));
            painter->drawLine(option->rect.left(), y, option->rect.right(), y);
        }
        else
        {
            QFont titleFont = divider->font();
            titleFont.setPixelSize(divider->isPlain() ? token.fontSize : token.fontSizeLG);
            titleFont.setWeight(divider->isPlain() ? QFont::Normal : QFont::Medium);
            QFontMetrics fm(titleFont);
            const int textWidth = fm.horizontalAdvance(divider->text());
            const int textPadding = titleFont.pixelSize();
            const int totalTextWidth = textWidth + textPadding * 2;

            int blockX;
            switch (divider->titlePlacement())
            {
            case Ant::DividerTitlePlacement::Start:
                blockX = option->rect.left() + qRound(option->rect.width() * 0.05);
                break;
            case Ant::DividerTitlePlacement::End:
                blockX = option->rect.right() - qRound(option->rect.width() * 0.05) - totalTextWidth;
                break;
            default:
                blockX = option->rect.left() + (option->rect.width() - totalTextWidth) / 2;
                break;
            }
            blockX = qBound(option->rect.left(), blockX, option->rect.right() - totalTextWidth);

            painter->setPen(QPen(lineColor, lineWidth, penStyle));
            painter->drawLine(option->rect.left(), y, blockX, y);
            painter->drawLine(blockX + totalTextWidth, y, option->rect.right(), y);

            painter->setPen(textColor);
            painter->setFont(titleFont);
            painter->drawText(QRect(blockX + textPadding, y - fm.height() / 2 - 1, textWidth, fm.height()),
                              Qt::AlignCenter, divider->text());
        }
    }
    else
    {
        const int x = option->rect.width() / 2;
        const int lineHeight = qRound(token.fontSize * 0.9);
        const int y1 = option->rect.top() + (option->rect.height() - lineHeight) / 2;
        const int y2 = y1 + lineHeight;
        painter->setPen(QPen(lineColor, lineWidth, penStyle));
        painter->drawLine(x, y1, x, y2);
    }

    painter->restore();
}
