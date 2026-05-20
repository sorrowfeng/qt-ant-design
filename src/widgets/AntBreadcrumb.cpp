#include "AntBreadcrumb.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>

#include <algorithm>

#include "../styles/AntBreadcrumbStyle.h"
#include "core/AntTheme.h"

AntBreadcrumb::AntBreadcrumb(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntBreadcrumbStyle>(this);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    syncBreadcrumbPerfCounters();
}

QString AntBreadcrumb::separator() const { return m_separator; }

void AntBreadcrumb::setSeparator(const QString& separator)
{
    const QString next = separator.isEmpty() ? QStringLiteral("/") : separator;
    if (m_separator == next)
    {
        return;
    }
    m_separator = next;
    updateBreadcrumbGeometry();
    update();
    Q_EMIT separatorChanged(m_separator);
}

void AntBreadcrumb::addItem(const QString& title, const QString& href, const QString& iconText, bool disabled)
{
    AntBreadcrumbItem item;
    item.title = title;
    item.href = href;
    item.iconText = iconText;
    item.disabled = disabled;
    m_items.append(item);
    updateBreadcrumbGeometry();
    update();
}

void AntBreadcrumb::addSeparator(const QString& separator)
{
    AntBreadcrumbItem item;
    item.separatorOnly = true;
    item.separator = separator.isEmpty() ? m_separator : separator;
    m_items.append(item);
    updateBreadcrumbGeometry();
    update();
}

void AntBreadcrumb::clearItems()
{
    m_items.clear();
    m_hoveredIndex = -1;
    updateBreadcrumbGeometry();
    update();
}

int AntBreadcrumb::count() const { return static_cast<int>(m_items.size()); }

AntBreadcrumbItem AntBreadcrumb::itemAt(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }
    return m_items.at(index);
}

QSize AntBreadcrumb::sizeHint() const
{
    const auto& token = antTheme->tokens();
    ensureBreadcrumbLayoutCache();
    return QSize(std::max(m_cachedTotalWidth, 160), token.controlHeight);
}

QSize AntBreadcrumb::minimumSizeHint() const
{
    return QSize(120, antTheme->tokens().controlHeightSM);
}

void AntBreadcrumb::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntBreadcrumb::mouseMoveEvent(QMouseEvent* event)
{
    const int index = itemIndexAt(event->pos());
    if (m_hoveredIndex != index)
    {
        const QRect dirtyRect = itemDirtyRect(m_hoveredIndex).united(itemDirtyRect(index));
        m_hoveredIndex = index;
        if (dirtyRect.isValid())
        {
            ++m_hoverScopedUpdateCount;
            syncBreadcrumbPerfCounters();
            update(dirtyRect);
        }
        else
        {
            update();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void AntBreadcrumb::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const int index = itemIndexAt(event->pos());
        if (index >= 0)
        {
            const AntBreadcrumbItem& item = m_items.at(index);
            if (!item.disabled && !isLastRouteItem(index))
            {
                Q_EMIT itemClicked(index, item.title, item.href);
                event->accept();
                return;
            }
        }
    }
    QWidget::mousePressEvent(event);
}

void AntBreadcrumb::leaveEvent(QEvent* event)
{
    if (m_hoveredIndex != -1)
    {
        const QRect dirtyRect = itemDirtyRect(m_hoveredIndex);
        m_hoveredIndex = -1;
        if (dirtyRect.isValid())
        {
            ++m_hoverScopedUpdateCount;
            syncBreadcrumbPerfCounters();
            update(dirtyRect);
        }
        else
        {
            update();
        }
    }
    QWidget::leaveEvent(event);
}

void AntBreadcrumb::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::FontChange ||
        event->type() == QEvent::ApplicationFontChange ||
        event->type() == QEvent::StyleChange)
    {
        updateBreadcrumbGeometry();
    }
    QWidget::changeEvent(event);
}

void AntBreadcrumb::resizeEvent(QResizeEvent* event)
{
    invalidateBreadcrumbLayoutCache();
    QWidget::resizeEvent(event);
}

QVector<QRect> AntBreadcrumb::itemRects() const
{
    ensureBreadcrumbLayoutCache();
    return m_cachedItemRects;
}

int AntBreadcrumb::itemIndexAt(const QPoint& pos) const
{
    ensureBreadcrumbLayoutCache();
    for (int i = 0; i < m_cachedItemRects.size(); ++i)
    {
        if (m_cachedItemRects.at(i).contains(pos))
        {
            const AntBreadcrumbItem& item = m_items.at(i);
            if (item.separatorOnly || item.disabled || isLastRouteItem(i))
            {
                return -1;
            }
            return i;
        }
    }
    return -1;
}

bool AntBreadcrumb::isLastRouteItem(int index) const
{
    for (int i = m_items.size() - 1; i >= 0; --i)
    {
        if (!m_items.at(i).separatorOnly)
        {
            return i == index;
        }
    }
    return false;
}

int AntBreadcrumb::itemWidth(const AntBreadcrumbItem& item) const
{
    const auto& token = antTheme->tokens();
    QFont f = font();
    f.setPixelSize(token.fontSize);
    if (item.separatorOnly)
    {
        return QFontMetrics(f).horizontalAdvance(item.separator.isEmpty() ? m_separator : item.separator) + token.marginXS * 2;
    }
    return QFontMetrics(f).horizontalAdvance(item.title) + (item.iconText.isEmpty() ? 0 : 22);
}

QColor AntBreadcrumb::itemColor(const AntBreadcrumbItem& item, int index, bool hovered) const
{
    const auto& token = antTheme->tokens();
    if (item.disabled)
    {
        return token.colorTextDisabled;
    }
    if (isLastRouteItem(index))
    {
        return token.colorText;
    }
    return hovered ? token.colorText : token.colorTextSecondary;
}

void AntBreadcrumb::updateBreadcrumbGeometry()
{
    invalidateBreadcrumbLayoutCache();
    ++m_geometryUpdateCount;
    syncBreadcrumbPerfCounters();
    updateGeometry();
}

void AntBreadcrumb::invalidateBreadcrumbLayoutCache() const
{
    m_layoutCacheValid = false;
}

void AntBreadcrumb::ensureBreadcrumbLayoutCache() const
{
    const auto& token = antTheme->tokens();
    QFont f = font();
    f.setPixelSize(token.fontSize);
    const int h = height() > 0 ? height() : token.controlHeight;

    if (m_layoutCacheValid &&
        m_cachedHeight == h &&
        m_cachedFont == f &&
        m_cachedSeparator == m_separator &&
        m_cachedTokenFontSize == token.fontSize &&
        m_cachedPaddingXS == token.paddingXS &&
        m_cachedMarginXS == token.marginXS &&
        m_cachedItemRects.size() == m_items.size() &&
        m_cachedItemWidths.size() == m_items.size())
    {
        ++m_layoutCacheHitCount;
        syncBreadcrumbPerfCounters();
        return;
    }

    QFontMetrics fm(f);
    const int separatorWidth = fm.horizontalAdvance(m_separator) + token.marginXS * 2;
    int x = token.paddingXS;

    m_cachedItemRects.clear();
    m_cachedItemWidths.clear();
    m_cachedItemRects.reserve(m_items.size());
    m_cachedItemWidths.reserve(m_items.size());

    for (int i = 0; i < m_items.size(); ++i)
    {
        const AntBreadcrumbItem& item = m_items.at(i);
        int w = 0;
        if (item.separatorOnly)
        {
            const QString sep = item.separator.isEmpty() ? m_separator : item.separator;
            w = fm.horizontalAdvance(sep) + token.marginXS * 2;
        }
        else
        {
            w = fm.horizontalAdvance(item.title) + (item.iconText.isEmpty() ? 0 : 22);
        }

        m_cachedItemWidths.append(w);
        m_cachedItemRects.append(QRect(x, 0, w, h));
        x += w;
        if (i < m_items.size() - 1 && !item.separatorOnly)
        {
            x += separatorWidth;
        }
    }

    m_cachedTotalWidth = x;
    m_cachedHeight = h;
    m_cachedFont = f;
    m_cachedSeparator = m_separator;
    m_cachedTokenFontSize = token.fontSize;
    m_cachedPaddingXS = token.paddingXS;
    m_cachedMarginXS = token.marginXS;
    m_layoutCacheValid = true;
    ++m_layoutCacheMissCount;
    syncBreadcrumbPerfCounters();
}

QRect AntBreadcrumb::itemDirtyRect(int index) const
{
    if (index < 0)
    {
        return {};
    }
    ensureBreadcrumbLayoutCache();
    if (index >= m_cachedItemRects.size())
    {
        return {};
    }
    const auto& token = antTheme->tokens();
    return m_cachedItemRects.at(index).adjusted(-token.marginXS, 0, token.marginXS, 0).intersected(rect());
}

void AntBreadcrumb::syncBreadcrumbPerfCounters() const
{
    auto* self = const_cast<AntBreadcrumb*>(this);
    self->setProperty("antBreadcrumbLayoutCacheHitCount", m_layoutCacheHitCount);
    self->setProperty("antBreadcrumbLayoutCacheMissCount", m_layoutCacheMissCount);
    self->setProperty("antBreadcrumbHoverScopedUpdateCount", m_hoverScopedUpdateCount);
    self->setProperty("antBreadcrumbGeometryUpdateCount", m_geometryUpdateCount);
}
