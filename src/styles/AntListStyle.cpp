#include "AntListStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntList.h"

AntListStyle::AntListStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntList>();
}

void AntListStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntList*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntListStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntList*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntListStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntList*>(widget))
    {
        drawList(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntListStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntListStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* list = qobject_cast<AntList*>(watched);
    if (list && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(list);
        option.rect = list->rect();
        QPainter painter(list);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, list);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

namespace
{
struct ListMetrics
{
    int padding = 16;
    int itemPaddingH = 16;
    int itemPaddingV = 12;
    int fontSize = 14;
    int headerHeight = 48;
    int footerHeight = 48;
    int radius = 8;
};

ListMetrics listMetrics(const AntList* list)
{
    ListMetrics m;
    switch (list->listSize())
    {
    case AntList::Small:
        m.itemPaddingV = 8;
        m.fontSize = 14;
        break;
    case AntList::Large:
        m.itemPaddingV = 16;
        m.fontSize = 16;
        break;
    default:
        m.itemPaddingV = 12;
        m.fontSize = 14;
        break;
    }
    return m;
}

QRect listHeaderRect(const AntList* list, const ListMetrics& m)
{
    QWidget* header = list->headerWidget();
    if (!header)
    {
        return {};
    }
    return QRect(1, 1, list->width() - 2, m.headerHeight);
}

QRect listFooterRect(const AntList* list, const ListMetrics& m)
{
    QWidget* footer = list->footerWidget();
    if (!footer)
    {
        return {};
    }
    return QRect(1, list->height() - m.footerHeight - 1, list->width() - 2, m.footerHeight);
}

QRect listContentRect(const AntList* list, const ListMetrics& m)
{
    int top = m.padding;
    int bottom = list->height() - m.padding;
    QWidget* header = list->headerWidget();
    QWidget* footer = list->footerWidget();
    if (header)
    {
        top = listHeaderRect(list, m).bottom() + 1;
    }
    if (footer)
    {
        bottom = listFooterRect(list, m).top();
    }
    return QRect(0, top, list->width(), bottom - top);
}
} // namespace

void AntListStyle::drawList(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* list = qobject_cast<const AntList*>(widget);
    if (!list || !painter || !option)
    {
        return;
    }

    const ListMetrics m = listMetrics(list);
    const auto& token = antTheme->tokens();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    // Border
    if (list->isBordered())
    {
        painter->setPen(QPen(token.colorBorder, token.lineWidth));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(option->rect.adjusted(0, 0, -1, -1), m.radius, m.radius);
    }

    // Header background
    if (list->headerWidget())
    {
        const QRect hr = listHeaderRect(list, m);
        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorFillQuaternary);
        if (list->isBordered())
        {
            painter->drawRect(hr);
        }
        else
        {
            painter->drawRoundedRect(hr, m.radius, m.radius);
        }
        painter->setPen(QPen(token.colorBorder, token.lineWidth));
        painter->drawLine(hr.bottomLeft(), hr.bottomRight());
    }

    // Split lines between items
    if (list->isSplit() && list->itemCount() > 1)
    {
        painter->setPen(QPen(token.colorSplit, token.lineWidth));
        const QRect cr = listContentRect(list, m);
        int y = cr.top();
        for (int i = 0; i < list->itemCount() - 1; ++i)
        {
            AntListItem* item = list->itemAt(i);
            if (item)
            {
                y += item->sizeHint().height();
                painter->drawLine(cr.left() + m.itemPaddingH, y, cr.right() - m.itemPaddingH, y);
                y += 1;
            }
        }
    }

    // Footer separator
    if (list->footerWidget())
    {
        const QRect fr = listFooterRect(list, m);
        painter->setPen(QPen(token.colorBorder, token.lineWidth));
        painter->drawLine(fr.topLeft(), fr.topRight());
    }

    painter->restore();
}
