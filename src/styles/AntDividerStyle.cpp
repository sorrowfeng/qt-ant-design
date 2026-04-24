#include "AntDividerStyle.h"

#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "widgets/AntDivider.h"

AntDividerStyle::AntDividerStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntDivider*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->updateGeometry();
                w->update();
            }
        }
    });
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

    const bool horizontal = divider->orientation() == Ant::DividerOrientation::Horizontal;
    const bool hasTitle = !divider->text().isEmpty();
    const QColor lineColor = token.colorBorder;
    const QColor textColor = token.colorTextSecondary;

    Qt::PenStyle penStyle = Qt::SolidLine;
    if (divider->variant() == Ant::DividerVariant::Dashed)
    {
        penStyle = Qt::DashLine;
    }
    else if (divider->variant() == Ant::DividerVariant::Dotted)
    {
        penStyle = Qt::DotLine;
    }

    int lineWidth = 1;
    switch (divider->dividerSize())
    {
    case Ant::DividerSize::Small:
        lineWidth = 1;
        break;
    case Ant::DividerSize::Large:
        lineWidth = 3;
        break;
    default:
        lineWidth = 2;
        break;
    }

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
            titleFont.setPixelSize(token.fontSize);
            QFontMetrics fm(titleFont);
            const int textWidth = fm.horizontalAdvance(divider->text());
            const int gap = 12;
            const int totalTextWidth = textWidth + gap * 2;

            int textX;
            switch (divider->titlePlacement())
            {
            case Ant::DividerTitlePlacement::Start:
                textX = option->rect.left() + 16;
                break;
            case Ant::DividerTitlePlacement::End:
                textX = option->rect.right() - totalTextWidth - 16;
                break;
            default:
                textX = (option->rect.width() - totalTextWidth) / 2;
                break;
            }

            painter->setPen(QPen(lineColor, lineWidth, penStyle));
            painter->drawLine(option->rect.left(), y, textX - gap, y);
            painter->drawLine(textX + textWidth + gap, y, option->rect.right(), y);

            painter->setPen(textColor);
            painter->setFont(titleFont);
            painter->drawText(QRect(textX, y - fm.height() / 2 - 1, textWidth, fm.height()),
                              Qt::AlignCenter, divider->text());
        }
    }
    else
    {
        const int x = option->rect.width() / 2;
        painter->setPen(QPen(lineColor, lineWidth, penStyle));
        painter->drawLine(x, option->rect.top(), x, option->rect.bottom());
    }

    painter->restore();
}
