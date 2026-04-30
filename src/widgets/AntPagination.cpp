#include "AntPagination.h"

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

#include <algorithm>
#include <cmath>

#include "../styles/AntPaginationStyle.h"
#include "core/AntTheme.h"
#include "styles/AntIconPainter.h"

AntPagination::AntPagination(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntPaginationStyle(style()));
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

int AntPagination::current() const { return m_current; }

void AntPagination::setCurrent(int current)
{
    const int next = std::clamp(current, 1, std::max(1, pageCount()));
    if (m_current == next)
    {
        return;
    }
    m_current = next;
    update();
    Q_EMIT currentChanged(m_current);
    Q_EMIT change(m_current, m_pageSize);
}

int AntPagination::pageSize() const { return m_pageSize; }

void AntPagination::setPageSize(int pageSize)
{
    pageSize = std::max(1, pageSize);
    if (m_pageSize == pageSize)
    {
        return;
    }
    m_pageSize = pageSize;
    normalizeCurrent();
    updatePaginationGeometry();
    update();
    Q_EMIT pageSizeChanged(m_pageSize);
    Q_EMIT showSizeChange(m_current, m_pageSize);
    Q_EMIT change(m_current, m_pageSize);
}

int AntPagination::total() const { return m_total; }

void AntPagination::setTotal(int total)
{
    total = std::max(0, total);
    if (m_total == total)
    {
        return;
    }
    m_total = total;
    normalizeCurrent();
    updatePaginationGeometry();
    update();
    Q_EMIT totalChanged(m_total);
}

bool AntPagination::isDisabled() const { return m_disabled; }

void AntPagination::setDisabled(bool disabled)
{
    if (m_disabled == disabled)
    {
        return;
    }
    m_disabled = disabled;
    update();
    Q_EMIT disabledChanged(m_disabled);
}

bool AntPagination::isSimple() const { return m_simple; }

void AntPagination::setSimple(bool simple)
{
    if (m_simple == simple)
    {
        return;
    }
    m_simple = simple;
    updatePaginationGeometry();
    update();
    Q_EMIT simpleChanged(m_simple);
}

bool AntPagination::isShowLessItems() const { return m_showLessItems; }

void AntPagination::setShowLessItems(bool show)
{
    if (m_showLessItems == show)
    {
        return;
    }
    m_showLessItems = show;
    updatePaginationGeometry();
    update();
    Q_EMIT showLessItemsChanged(m_showLessItems);
}

bool AntPagination::isShowTotal() const { return m_showTotal; }

void AntPagination::setShowTotal(bool show)
{
    if (m_showTotal == show)
    {
        return;
    }
    m_showTotal = show;
    updatePaginationGeometry();
    update();
    Q_EMIT showTotalChanged(m_showTotal);
}

bool AntPagination::isShowQuickJumper() const { return m_showQuickJumper; }

void AntPagination::setShowQuickJumper(bool show)
{
    if (m_showQuickJumper == show)
    {
        return;
    }
    m_showQuickJumper = show;
    updatePaginationGeometry();
    update();
    Q_EMIT showQuickJumperChanged(m_showQuickJumper);
}

bool AntPagination::isShowSizeChanger() const { return m_showSizeChanger; }

void AntPagination::setShowSizeChanger(bool show)
{
    if (m_showSizeChanger == show)
    {
        return;
    }
    m_showSizeChanger = show;
    updatePaginationGeometry();
    update();
    Q_EMIT showSizeChangerChanged(m_showSizeChanger);
}

Ant::Size AntPagination::paginationSize() const { return m_paginationSize; }

void AntPagination::setPaginationSize(Ant::Size size)
{
    if (m_paginationSize == size)
    {
        return;
    }
    m_paginationSize = size;
    updatePaginationGeometry();
    update();
    Q_EMIT paginationSizeChanged(m_paginationSize);
}

int AntPagination::pageCount() const
{
    return std::max(1, static_cast<int>(std::ceil(static_cast<double>(m_total) / static_cast<double>(m_pageSize))));
}

QSize AntPagination::sizeHint() const
{
    const auto items = pageItems();
    int width = antTheme->tokens().paddingXS;
    for (const auto& item : items)
    {
        width = std::max(width, item.rect.right() + antTheme->tokens().paddingXS);
    }
    return QSize(width, itemSize());
}

QSize AntPagination::minimumSizeHint() const
{
    return QSize(160, itemSize());
}

void AntPagination::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntPagination::mouseMoveEvent(QMouseEvent* event)
{
    const int index = itemAt(event->pos());
    if (m_hoveredIndex != index)
    {
        m_hoveredIndex = index;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void AntPagination::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const int index = itemAt(event->pos());
        const auto items = pageItems();
        if (index >= 0 && index < items.size())
        {
            activateItem(items.at(index));
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void AntPagination::leaveEvent(QEvent* event)
{
    if (m_hoveredIndex != -1)
    {
        m_hoveredIndex = -1;
        update();
    }
    QWidget::leaveEvent(event);
}

void AntPagination::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Left)
    {
        setCurrent(m_current - 1);
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Right)
    {
        setCurrent(m_current + 1);
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

QVector<AntPagination::PageItem> AntPagination::pageItems() const
{
    const auto& token = antTheme->tokens();
    QVector<PageItem> items;
    const int size = itemSize();
    const int spacing = itemSpacing();
    int x = token.paddingXS;
    auto append = [&](ItemKind kind, int page, const QString& text, bool enabled = true, bool active = false, int width = 0) {
        PageItem item;
        item.kind = kind;
        item.page = page;
        item.text = text;
        item.enabled = enabled && !m_disabled;
        item.active = active;
        item.rect = QRect(x, 0, width > 0 ? width : size, size);
        items.append(item);
        x += item.rect.width() + spacing;
    };

    if (m_showTotal)
    {
        const QString totalText = QStringLiteral("Total %1 items").arg(m_total);
        QFont f = font();
        f.setPixelSize(fontSize());
        append(ItemKind::Text, 0, totalText, false, false, QFontMetrics(f).horizontalAdvance(totalText) + token.paddingSM);
    }

    append(ItemKind::Prev, m_current - 1, QString(), m_current > 1);
    if (m_simple)
    {
        QFont f = font();
        f.setPixelSize(fontSize());
        const int separatorWidth = QFontMetrics(f).horizontalAdvance(QStringLiteral("/")) + token.paddingXS;
        const int totalWidth = QFontMetrics(f).horizontalAdvance(QString::number(pageCount())) + token.paddingXS;
        append(ItemKind::QuickJumper, m_current, QString::number(m_current), true, false, itemSize() * 2);
        append(ItemKind::Text, 0, QStringLiteral("/"), false, false, separatorWidth);
        append(ItemKind::Text, 0, QString::number(pageCount()), false, false, totalWidth);
    }
    else
    {
        const int count = pageCount();
        const int sibling = m_showLessItems ? 1 : 2;
        auto appendPage = [&](int page) {
            append(ItemKind::Page, page, QString::number(page), true, page == m_current);
        };

        appendPage(1);
        const int left = std::max(2, m_current - sibling);
        const int right = std::min(count - 1, m_current + sibling);
        if (left > 2)
        {
            append(ItemKind::JumpPrev, std::max(1, m_current - 5), QString());
        }
        for (int page = left; page <= right; ++page)
        {
            appendPage(page);
        }
        if (right < count - 1)
        {
            append(ItemKind::JumpNext, std::min(count, m_current + 5), QString());
        }
        if (count > 1)
        {
            appendPage(count);
        }
    }
    append(ItemKind::Next, m_current + 1, QString(), m_current < pageCount());

    if (m_showSizeChanger)
    {
        append(ItemKind::SizeChanger, 0, QStringLiteral("%1 / page").arg(m_pageSize), true, false, itemSize() * 3);
    }
    if (m_showQuickJumper)
    {
        QFont f = font();
        f.setPixelSize(fontSize());
        const int goToWidth = QFontMetrics(f).horizontalAdvance(QStringLiteral("Go to")) + token.paddingXS;
        const int pageWidth = QFontMetrics(f).horizontalAdvance(QStringLiteral("Page")) + token.paddingXS;
        append(ItemKind::Text, 0, QStringLiteral("Go to"), false, false, goToWidth);
        append(ItemKind::QuickJumper, pageCount(), QString(), true, false, itemSize() + token.paddingLG);
        append(ItemKind::Text, 0, QStringLiteral("Page"), false, false, pageWidth);
    }
    return items;
}

int AntPagination::itemAt(const QPoint& pos) const
{
    const auto items = pageItems();
    for (int i = 0; i < items.size(); ++i)
    {
        if (items.at(i).rect.contains(pos) && items.at(i).enabled)
        {
            return i;
        }
    }
    return -1;
}

int AntPagination::itemSize() const
{
    const auto& token = antTheme->tokens();
    switch (m_paginationSize)
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

int AntPagination::itemSpacing() const
{
    return m_paginationSize == Ant::Size::Small ? antTheme->tokens().paddingXXS : antTheme->tokens().marginXS;
}

int AntPagination::fontSize() const
{
    return m_paginationSize == Ant::Size::Small ? antTheme->tokens().fontSizeSM : antTheme->tokens().fontSize;
}

int AntPagination::rangeStart() const
{
    if (m_total <= 0)
    {
        return 0;
    }
    return (m_current - 1) * m_pageSize + 1;
}

int AntPagination::rangeEnd() const
{
    return std::min(m_total, m_current * m_pageSize);
}

QColor AntPagination::itemTextColor(const PageItem& item, bool hovered) const
{
    const auto& token = antTheme->tokens();
    if (!item.enabled && item.kind != ItemKind::Text)
    {
        return token.colorTextDisabled;
    }
    if (item.active || hovered)
    {
        return token.colorPrimary;
    }
    if (item.kind == ItemKind::Text)
    {
        return token.colorTextSecondary;
    }
    return token.colorText;
}

QColor AntPagination::itemBackgroundColor(const PageItem& item, bool hovered) const
{
    const auto& token = antTheme->tokens();
    if (item.kind == ItemKind::Text)
    {
        return Qt::transparent;
    }
    if (item.active)
    {
        return m_disabled ? token.colorBgContainerDisabled : token.colorBgContainer;
    }
    return hovered ? token.colorFillQuaternary : token.colorBgContainer;
}

void AntPagination::drawItem(QPainter& painter, const PageItem& item, bool hovered) const
{
    const auto& token = antTheme->tokens();
    if (item.kind != ItemKind::Text)
    {
        painter.setPen(QPen(item.active ? token.colorPrimary : token.colorBorder, token.lineWidth));
        painter.setBrush(itemBackgroundColor(item, hovered));
        painter.drawRoundedRect(item.rect.adjusted(0, 0, -1, -1), token.borderRadius, token.borderRadius);
    }
    const QColor color = itemTextColor(item, hovered);
    const qreal side = qMin(item.rect.width(), item.rect.height()) * 0.44;
    const QRectF iconRect(item.rect.center().x() - side / 2.0,
                          item.rect.center().y() - side / 2.0,
                          side,
                          side);
    bool drewIcon = false;
    switch (item.kind)
    {
    case ItemKind::Prev:
        drewIcon = AntIconPainter::drawIcon(painter, Ant::IconType::Left, iconRect, color);
        break;
    case ItemKind::Next:
        drewIcon = AntIconPainter::drawIcon(painter, Ant::IconType::Right, iconRect, color);
        break;
    case ItemKind::JumpPrev:
    case ItemKind::JumpNext:
        AntIconPainter::drawEllipsis(painter, iconRect, color);
        drewIcon = true;
        break;
    default:
        break;
    }
    if (!drewIcon)
    {
        painter.setPen(color);
        painter.drawText(item.rect, Qt::AlignCenter | Qt::TextSingleLine, item.text);
    }
}

void AntPagination::activateItem(const PageItem& item)
{
    if (!item.enabled || item.kind == ItemKind::Text)
    {
        return;
    }
    if (item.kind == ItemKind::SizeChanger)
    {
        const int next = m_pageSize == 10 ? 20 : (m_pageSize == 20 ? 50 : 10);
        setPageSize(next);
        return;
    }
    if (item.kind == ItemKind::QuickJumper && item.text.isEmpty())
    {
        return;
    }
    setCurrent(item.page);
}

void AntPagination::normalizeCurrent()
{
    m_current = std::clamp(m_current, 1, std::max(1, pageCount()));
}

void AntPagination::updatePaginationGeometry()
{
    updateGeometry();
}
