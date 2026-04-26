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

namespace
{
bool isLastRouteItem(const AntBreadcrumb* breadcrumb, int index)
{
    for (int i = breadcrumb->count() - 1; i >= 0; --i)
    {
        if (!breadcrumb->itemAt(i).separatorOnly)
        {
            return i == index;
        }
    }
    return false;
}

int computeItemWidth(const AntBreadcrumb* breadcrumb, const AntBreadcrumbItem& item, const QFont& baseFont, const QString& separator)
{
    const auto& token = antTheme->tokens();
    QFont f = baseFont;
    f.setPixelSize(token.fontSize);
    if (item.separatorOnly)
    {
        const QString sep = item.separator.isEmpty() ? separator : item.separator;
        return QFontMetrics(f).horizontalAdvance(sep) + token.marginXS * 2;
    }
    return QFontMetrics(f).horizontalAdvance(item.title) + (item.iconText.isEmpty() ? 0 : 22);
}
} // namespace

void AntBreadcrumbStyle::drawBreadcrumb(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* breadcrumb = qobject_cast<const AntBreadcrumb*>(widget);
    if (!breadcrumb || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QFont textFont = painter->font();
    textFont.setPixelSize(token.fontSize);
    painter->setFont(textFont);

    const int count = breadcrumb->count();
    const QString separator = breadcrumb->separator();
    QFontMetrics fm(textFont);
    const int separatorWidth = fm.horizontalAdvance(separator) + token.marginXS * 2;
    int x = token.paddingXS;
    const int h = option->rect.height();

    for (int i = 0; i < count; ++i)
    {
        const AntBreadcrumbItem item = breadcrumb->itemAt(i);

        if (item.separatorOnly)
        {
            const QString sep = item.separator.isEmpty() ? separator : item.separator;
            const int w = fm.horizontalAdvance(sep) + token.marginXS * 2;
            painter->setPen(token.colorTextTertiary);
            painter->drawText(QRect(x, 0, w, h), Qt::AlignCenter, sep);
            x += w;
            continue;
        }

        const int itemW = computeItemWidth(breadcrumb, item, textFont, separator);
        const QRect rect(x, 0, itemW, h);
        const bool last = isLastRouteItem(breadcrumb, i);

        QColor color = token.colorTextSecondary;
        if (item.disabled)
        {
            color = token.colorTextDisabled;
        }
        else if (last)
        {
            color = token.colorText;
        }

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

        x += itemW;

        if (i < count - 1)
        {
            const AntBreadcrumbItem next = breadcrumb->itemAt(i + 1);
            if (!next.separatorOnly)
            {
                const QRect sepRect(x + token.marginXS, 0, 20, h);
                painter->setPen(token.colorTextTertiary);
                painter->drawText(sepRect, Qt::AlignCenter, separator);
                x += separatorWidth;
            }
        }
    }

    painter->restore();
}
