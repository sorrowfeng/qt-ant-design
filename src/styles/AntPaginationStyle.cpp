#include "AntPaginationStyle.h"

#include <QEvent>
#include <QLineEdit>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntIconPainter.h"
#include "widgets/AntPagination.h"

AntPaginationStyle::AntPaginationStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntPaginationStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntPagination*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntPaginationStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntPagination*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntPaginationStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntPagination*>(widget))
    {
        drawPagination(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntPaginationStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntPaginationStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* pagination = qobject_cast<AntPagination*>(watched);
    if (pagination && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(pagination);
        option.rect = pagination->rect();
        QPainter painter(pagination);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, pagination);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntPaginationStyle::drawPagination(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* pagination = qobject_cast<const AntPagination*>(widget);
    if (!pagination || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QFont f = painter->font();
    f.setPixelSize(pagination->fontSize());
    painter->setFont(f);

    const auto items = pagination->pageItems();
    const auto* quickJumper = pagination->findChild<QLineEdit*>(QStringLiteral("AntPaginationQuickJumper"));
    for (int i = 0; i < items.size(); ++i)
    {
        const AntPagination::PageItem& item = items.at(i);
        const bool hovered = pagination->m_hoveredIndex == i;
        const bool quickJumperOwnsItem = item.kind == AntPagination::ItemKind::QuickJumper && quickJumper && quickJumper->isVisible()
            && quickJumper->geometry() == item.rect;
        if (item.kind != AntPagination::ItemKind::Text)
        {
            AntStyleBase::drawCrispRoundedRect(painter, item.rect,
                QPen((item.active || (quickJumperOwnsItem && quickJumper->hasFocus())) ? token.colorPrimary : token.colorBorder,
                    token.lineWidth),
                pagination->itemBackgroundColor(item, hovered),
                token.borderRadius, token.borderRadius);
        }
        if (quickJumperOwnsItem)
        {
            continue;
        }
        const QColor itemColor = pagination->itemTextColor(item, hovered);
        const qreal side = qMin(item.rect.width(), item.rect.height()) * 0.44;
        const QRectF iconRect(item.rect.center().x() - side / 2.0,
                              item.rect.center().y() - side / 2.0,
                              side,
                              side);
        bool drewIcon = false;
        switch (item.kind)
        {
        case AntPagination::ItemKind::Prev:
            drewIcon = AntIconPainter::drawIcon(*painter, Ant::IconType::Left, iconRect, itemColor);
            break;
        case AntPagination::ItemKind::Next:
            drewIcon = AntIconPainter::drawIcon(*painter, Ant::IconType::Right, iconRect, itemColor);
            break;
        case AntPagination::ItemKind::JumpPrev:
        case AntPagination::ItemKind::JumpNext:
            AntIconPainter::drawEllipsis(*painter, iconRect, itemColor);
            drewIcon = true;
            break;
        default:
            break;
        }
        if (!drewIcon)
        {
            painter->setPen(itemColor);
            painter->drawText(item.rect, Qt::AlignCenter | Qt::TextSingleLine, item.text);
        }
    }

    painter->restore();
}
