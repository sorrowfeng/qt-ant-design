#include "AntPaginationStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include <algorithm>
#include <cmath>

#include "styles/AntPalette.h"
#include "widgets/AntPagination.h"

AntPaginationStyle::AntPaginationStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntPagination>();
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

namespace
{
enum class PageItemKind
{
    Prev,
    Next,
    Page,
    JumpPrev,
    JumpNext,
    Text,
    SizeChanger,
    QuickJumper,
};

struct PageItem
{
    PageItemKind kind = PageItemKind::Page;
    int page = 0;
    QString text;
    QRect rect;
    bool enabled = true;
    bool active = false;
};

int paginationItemSize(const AntPagination* p)
{
    const auto& token = antTheme->tokens();
    switch (p->paginationSize())
    {
    case Ant::Size::Large:
        return token.controlHeightLG;
    case Ant::Size::Small:
        return token.controlHeightSM;
    case Ant::Size::Middle:
    default:
        return token.controlHeight;
    }
}

int paginationItemSpacing(const AntPagination* p)
{
    return p->paginationSize() == Ant::Size::Small ? antTheme->tokens().paddingXXS : antTheme->tokens().marginXS;
}

int paginationFontSize(const AntPagination* p)
{
    return p->paginationSize() == Ant::Size::Small ? antTheme->tokens().fontSizeSM : antTheme->tokens().fontSize;
}

int paginationRangeStart(const AntPagination* p)
{
    if (p->total() <= 0)
    {
        return 0;
    }
    return (p->current() - 1) * p->pageSize() + 1;
}

int paginationRangeEnd(const AntPagination* p)
{
    return std::min(p->total(), p->current() * p->pageSize());
}

QVector<PageItem> buildPageItems(const AntPagination* p)
{
    const auto& token = antTheme->tokens();
    QVector<PageItem> items;
    const int size = paginationItemSize(p);
    const int spacing = paginationItemSpacing(p);
    const int current = p->current();
    const int total = p->total();
    const int pageSize = p->pageSize();
    const int pageCount = p->pageCount();
    const bool disabled = p->isDisabled();
    const bool simple = p->isSimple();
    const bool showLessItems = p->isShowLessItems();
    const bool showTotal = p->isShowTotal();
    const bool showQuickJumper = p->isShowQuickJumper();
    const bool showSizeChanger = p->isShowSizeChanger();

    int x = token.paddingXS;
    auto append = [&](PageItemKind kind, int page, const QString& text, bool enabled = true, bool active = false, int width = 0) {
        PageItem item;
        item.kind = kind;
        item.page = page;
        item.text = text;
        item.enabled = enabled && !disabled;
        item.active = active;
        item.rect = QRect(x, 0, width > 0 ? width : size, size);
        items.append(item);
        x += item.rect.width() + spacing;
    };

    if (showTotal)
    {
        const QString totalText = QStringLiteral("Total %1 items").arg(total);
        QFont f = p->font();
        f.setPixelSize(paginationFontSize(p));
        append(PageItemKind::Text, 0, totalText, false, false, QFontMetrics(f).horizontalAdvance(totalText) + token.paddingSM);
    }

    append(PageItemKind::Prev, current - 1, QStringLiteral("<"), current > 1);
    if (simple)
    {
        QFont f = p->font();
        f.setPixelSize(paginationFontSize(p));
        const int separatorWidth = QFontMetrics(f).horizontalAdvance(QStringLiteral("/")) + token.paddingXS;
        const int totalWidth = QFontMetrics(f).horizontalAdvance(QString::number(pageCount)) + token.paddingXS;
        append(PageItemKind::QuickJumper, current, QString::number(current), true, false, size * 2);
        append(PageItemKind::Text, 0, QStringLiteral("/"), false, false, separatorWidth);
        append(PageItemKind::Text, 0, QString::number(pageCount), false, false, totalWidth);
    }
    else
    {
        const int sibling = showLessItems ? 1 : 2;
        auto appendPage = [&](int page) {
            append(PageItemKind::Page, page, QString::number(page), true, page == current);
        };

        appendPage(1);
        const int left = std::max(2, current - sibling);
        const int right = std::min(pageCount - 1, current + sibling);
        if (left > 2)
        {
            append(PageItemKind::JumpPrev, std::max(1, current - 5), QStringLiteral("..."));
        }
        for (int page = left; page <= right; ++page)
        {
            appendPage(page);
        }
        if (right < pageCount - 1)
        {
            append(PageItemKind::JumpNext, std::min(pageCount, current + 5), QStringLiteral("..."));
        }
        if (pageCount > 1)
        {
            appendPage(pageCount);
        }
    }
    append(PageItemKind::Next, current + 1, QStringLiteral(">"), current < pageCount);

    if (showSizeChanger)
    {
        append(PageItemKind::SizeChanger, 0, QStringLiteral("%1 / page").arg(pageSize), true, false, size * 3);
    }
    if (showQuickJumper)
    {
        QFont f = p->font();
        f.setPixelSize(paginationFontSize(p));
        const int goToWidth = QFontMetrics(f).horizontalAdvance(QStringLiteral("Go to")) + token.paddingXS;
        const int pageWidth = QFontMetrics(f).horizontalAdvance(QStringLiteral("Page")) + token.paddingXS;
        append(PageItemKind::Text, 0, QStringLiteral("Go to"), false, false, goToWidth);
        append(PageItemKind::QuickJumper, pageCount, QString(), true, false, size + token.paddingLG);
        append(PageItemKind::Text, 0, QStringLiteral("Page"), false, false, pageWidth);
    }
    return items;
}

QColor paginationItemTextColor(const PageItem& item, bool hovered)
{
    const auto& token = antTheme->tokens();
    if (!item.enabled && item.kind != PageItemKind::Text)
    {
        return token.colorTextDisabled;
    }
    if (item.active || hovered)
    {
        return token.colorPrimary;
    }
    if (item.kind == PageItemKind::Text)
    {
        return token.colorTextSecondary;
    }
    return token.colorText;
}

QColor paginationItemBackgroundColor(const PageItem& item, bool hovered, bool disabled)
{
    const auto& token = antTheme->tokens();
    if (item.kind == PageItemKind::Text)
    {
        return Qt::transparent;
    }
    if (item.active)
    {
        return disabled ? token.colorBgContainerDisabled : token.colorBgContainer;
    }
    return hovered ? token.colorFillQuaternary : token.colorBgContainer;
}
} // namespace

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
    f.setPixelSize(paginationFontSize(pagination));
    painter->setFont(f);

    const auto items = buildPageItems(pagination);
    for (int i = 0; i < items.size(); ++i)
    {
        const PageItem& item = items.at(i);
        if (item.kind != PageItemKind::Text)
        {
            AntStyleBase::drawCrispRoundedRect(painter, item.rect,
                QPen(item.active ? token.colorPrimary : token.colorBorder, token.lineWidth),
                paginationItemBackgroundColor(item, false, pagination->isDisabled()),
                token.borderRadius, token.borderRadius);
        }
        painter->setPen(paginationItemTextColor(item, false));
        painter->drawText(item.rect, Qt::AlignCenter | Qt::TextSingleLine, item.text);
    }

    painter->restore();
}
