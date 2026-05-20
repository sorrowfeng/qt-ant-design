#include "AntBreadcrumbStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntBreadcrumb.h"

AntBreadcrumbStyle::AntBreadcrumbStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntBreadcrumb>();
}

void AntBreadcrumbStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntBreadcrumb*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntBreadcrumbStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntBreadcrumb*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntBreadcrumbStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntBreadcrumb*>(widget))
    {
        drawBreadcrumb(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntBreadcrumbStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntBreadcrumbStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* breadcrumb = qobject_cast<AntBreadcrumb*>(watched);
    if (breadcrumb && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(breadcrumb);
        option.rect = breadcrumb->rect();
        QPainter painter(breadcrumb);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, breadcrumb);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntBreadcrumbStyle::drawBreadcrumb(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* breadcrumb = qobject_cast<const AntBreadcrumb*>(widget);
    if (!breadcrumb || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    breadcrumb->ensureBreadcrumbLayoutCache();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QFont textFont = painter->font();
    textFont.setPixelSize(token.fontSize);
    painter->setFont(textFont);

    const int count = breadcrumb->count();
    const QString separator = breadcrumb->separator();
    QFontMetrics fm(textFont);
    const int separatorWidth = fm.horizontalAdvance(separator) + token.marginXS * 2;
    const int h = option->rect.height();

    for (int i = 0; i < count; ++i)
    {
        const AntBreadcrumbItem item = breadcrumb->itemAt(i);
        const QRect rect = i < breadcrumb->m_cachedItemRects.size()
            ? breadcrumb->m_cachedItemRects.at(i)
            : QRect();

        if (item.separatorOnly)
        {
            const QString sep = item.separator.isEmpty() ? separator : item.separator;
            painter->setPen(token.colorTextTertiary);
            painter->drawText(rect, Qt::AlignCenter, sep);
            continue;
        }

        const QColor color = breadcrumb->itemColor(item, i, breadcrumb->m_hoveredIndex == i);

        painter->setPen(color);
        int tx = rect.left();
        if (!item.iconText.isEmpty())
        {
            painter->drawText(QRect(tx, rect.top(), 18, rect.height()), Qt::AlignCenter, item.iconText.left(2));
            tx += 18 + token.marginXS / 2;
        }
        painter->drawText(QRect(tx, rect.top(), rect.right() - tx, rect.height()),
                          Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
                          item.title);

        if (i < count - 1)
        {
            const AntBreadcrumbItem next = breadcrumb->itemAt(i + 1);
            if (!next.separatorOnly)
            {
                const int x = rect.right() + 1;
                const QRect sepRect(x, 0, separatorWidth, h);
                painter->setPen(token.colorTextTertiary);
                painter->drawText(sepRect, Qt::AlignCenter, separator);
            }
        }
    }

    painter->restore();
}
