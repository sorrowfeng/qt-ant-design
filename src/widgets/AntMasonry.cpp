#include "AntMasonry.h"

#include <QPalette>
#include <QResizeEvent>

#include <algorithm>

#include "core/AntTheme.h"

AntMasonry::AntMasonry(QWidget* parent)
    : QWidget(parent)
{
    updateTheme();
    syncPerfCounters();
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateTheme();
        update();
    });
}

int AntMasonry::columns() const { return m_columns; }
void AntMasonry::setColumns(int cols)
{
    cols = std::max(1, cols);
    if (m_columns == cols) return;
    m_columns = cols;
    markLayoutDirty();
    relayout();
    Q_EMIT columnsChanged(m_columns);
}

int AntMasonry::spacing() const { return m_spacing; }
void AntMasonry::setSpacing(int px)
{
    px = std::max(0, px);
    if (m_spacing == px) return;
    m_spacing = px;
    markLayoutDirty();
    relayout();
    Q_EMIT spacingChanged(m_spacing);
}

void AntMasonry::addWidget(QWidget* widget)
{
    if (!widget)
    {
        return;
    }
    widget->setParent(this);
    m_items.append(widget);
    widget->show();
    if (!appendItemLayout(widget))
    {
        markLayoutDirty();
        relayout();
    }
}

void AntMasonry::clear()
{
    for (auto* w : m_items) w->deleteLater();
    m_items.clear();
    resetLayoutCache();
    setMinimumHeight(0);
    updateGeometry();
    syncPerfCounters();
}

void AntMasonry::resizeEvent(QResizeEvent* event)
{
    if (!event || event->size().width() != event->oldSize().width())
    {
        markLayoutDirty();
        relayout();
    }
    else
    {
        syncPerfCounters();
    }
    QWidget::resizeEvent(event);
}

void AntMasonry::relayout()
{
    const int layoutWidth = width();
    if (!m_layoutDirty && m_cachedWidth == layoutWidth && m_cachedColumns == m_columns && m_cachedSpacing == m_spacing &&
        m_cachedItemCount == m_items.size())
    {
        syncPerfCounters();
        return;
    }

    if (m_items.isEmpty())
    {
        resetLayoutCache();
        setMinimumHeight(0);
        updateGeometry();
        syncPerfCounters();
        return;
    }

    const int colW = columnWidthFor(layoutWidth);
    if (colW <= 0)
    {
        syncPerfCounters();
        return;
    }

    ++m_fullRelayoutCount;
    m_columnHeights = QVector<int>(m_columns, 0);

    for (auto* w : m_items)
    {
        if (!w)
        {
            continue;
        }

        const int shortestCol = shortestColumn();
        const int x = shortestCol * (colW + m_spacing);
        const int y = m_columnHeights.value(shortestCol);
        const int itemHeight = itemHeightFor(w, colW);

        applyItemGeometry(w, QRect(x, y, colW, itemHeight));
        m_columnHeights[shortestCol] += itemHeight + m_spacing;
    }

    m_cachedWidth = layoutWidth;
    m_cachedColumns = m_columns;
    m_cachedSpacing = m_spacing;
    m_cachedColumnWidth = colW;
    m_cachedItemCount = m_items.size();
    m_layoutDirty = false;

    updateMinimumHeightFromColumns();
    syncPerfCounters();
}

void AntMasonry::markLayoutDirty()
{
    m_layoutDirty = true;
}

void AntMasonry::resetLayoutCache()
{
    m_columnHeights.clear();
    m_cachedWidth = -1;
    m_cachedColumns = -1;
    m_cachedSpacing = -1;
    m_cachedColumnWidth = -1;
    m_cachedItemCount = 0;
    m_layoutDirty = true;
}

int AntMasonry::columnWidthFor(int layoutWidth) const
{
    const int totalSpacing = (m_columns - 1) * m_spacing;
    return qMax(0, (layoutWidth - totalSpacing) / m_columns);
}

int AntMasonry::itemHeightFor(QWidget* widget, int columnWidth)
{
    if (!widget)
    {
        return 0;
    }

    ++m_heightQueryCount;
    int itemHeight = widget->hasHeightForWidth() ? widget->heightForWidth(columnWidth) : widget->sizeHint().height();
    itemHeight = qMax(itemHeight, widget->minimumSizeHint().height());
    itemHeight = qMax(itemHeight, widget->minimumHeight());
    if (widget->maximumHeight() < QWIDGETSIZE_MAX)
    {
        itemHeight = qMin(itemHeight, widget->maximumHeight());
    }
    return qMax(0, itemHeight);
}

int AntMasonry::shortestColumn() const
{
    if (m_columnHeights.isEmpty())
    {
        return 0;
    }

    int shortestCol = 0;
    for (int i = 1; i < m_columnHeights.size(); ++i)
    {
        if (m_columnHeights.at(i) < m_columnHeights.at(shortestCol))
        {
            shortestCol = i;
        }
    }
    return shortestCol;
}

bool AntMasonry::canAppendIncrementally() const
{
    return !m_layoutDirty && m_cachedWidth == width() && m_cachedColumns == m_columns && m_cachedSpacing == m_spacing &&
           m_cachedItemCount == m_items.size() - 1 && m_cachedColumnWidth > 0 && m_columnHeights.size() == m_columns;
}

bool AntMasonry::appendItemLayout(QWidget* widget)
{
    if (!widget || !canAppendIncrementally())
    {
        return false;
    }

    const int shortestCol = shortestColumn();
    const int x = shortestCol * (m_cachedColumnWidth + m_spacing);
    const int y = m_columnHeights.value(shortestCol);
    const int itemHeight = itemHeightFor(widget, m_cachedColumnWidth);

    applyItemGeometry(widget, QRect(x, y, m_cachedColumnWidth, itemHeight));
    m_columnHeights[shortestCol] += itemHeight + m_spacing;
    m_cachedItemCount = m_items.size();
    ++m_incrementalLayoutCount;

    updateMinimumHeightFromColumns();
    syncPerfCounters();
    return true;
}

bool AntMasonry::applyItemGeometry(QWidget* widget, const QRect& geometry)
{
    if (!widget)
    {
        return false;
    }

    if (widget->geometry() == geometry)
    {
        return false;
    }
    widget->setGeometry(geometry);
    ++m_geometryApplyCount;
    return true;
}

void AntMasonry::updateMinimumHeightFromColumns()
{
    int maxH = 0;
    for (int h : m_columnHeights) maxH = std::max(maxH, h);
    if (maxH > 0)
    {
        maxH -= m_spacing;
    }
    if (minimumHeight() != maxH)
    {
        setMinimumHeight(maxH);
        updateGeometry();
    }
}

void AntMasonry::updateTheme()
{
    const auto& token = antTheme->tokens();
    QPalette pal = palette();
    pal.setColor(QPalette::Window, token.colorBgContainer);
    pal.setColor(QPalette::WindowText, token.colorText);
    setPalette(pal);
    setAutoFillBackground(true);
}

void AntMasonry::syncPerfCounters()
{
    setProperty("antMasonryFullRelayoutCount", m_fullRelayoutCount);
    setProperty("antMasonryIncrementalLayoutCount", m_incrementalLayoutCount);
    setProperty("antMasonryGeometryApplyCount", m_geometryApplyCount);
    setProperty("antMasonryHeightQueryCount", m_heightQueryCount);
    setProperty("antMasonryLayoutDirty", m_layoutDirty);
    setProperty("antMasonryCachedColumnWidth", m_cachedColumnWidth);
}
