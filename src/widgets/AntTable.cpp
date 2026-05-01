#include "AntTable.h"

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include <algorithm>

#include "../styles/AntTableStyle.h"
#include "core/AntTheme.h"

namespace
{
int compareSortValues(const QVariant& left, const QVariant& right)
{
    const bool leftEmpty = !left.isValid() || left.isNull();
    const bool rightEmpty = !right.isValid() || right.isNull();
    if (leftEmpty || rightEmpty)
    {
        if (leftEmpty == rightEmpty)
            return 0;
        return leftEmpty ? 1 : -1;
    }

    bool leftOk = false;
    bool rightOk = false;
    const double leftNumber = left.toDouble(&leftOk);
    const double rightNumber = right.toDouble(&rightOk);
    if (leftOk && rightOk)
    {
        if (leftNumber < rightNumber)
            return -1;
        if (leftNumber > rightNumber)
            return 1;
        return 0;
    }

    return QString::localeAwareCompare(left.toString(), right.toString());
}
} // namespace

// ─── AntTable ───

AntTable::AntTable(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntTableStyle>(this);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

// ─── Column management ───

void AntTable::addColumn(const AntTableColumn& column)
{
    m_columns.append(column);
    rebuildDisplayOrder();
    updateGeometry();
    update();
}

void AntTable::removeColumn(const QString& key)
{
    for (int i = m_columns.size() - 1; i >= 0; --i)
    {
        if (m_columns.at(i).key == key)
        {
            m_columns.remove(i);
        }
    }
    if (m_sortColumn == key)
    {
        m_sortColumn.clear();
        m_sortOrder = Ant::TableSortOrder::None;
    }
    rebuildDisplayOrder();
    updateGeometry();
    update();
}

void AntTable::setColumns(const QVector<AntTableColumn>& columns)
{
    m_columns = columns;
    m_sortColumn.clear();
    m_sortOrder = Ant::TableSortOrder::None;
    rebuildDisplayOrder();
    updateGeometry();
    update();
}

QVector<AntTableColumn> AntTable::columns() const
{
    return m_columns;
}

// ─── Row management ───

void AntTable::addRow(const AntTableRow& row)
{
    m_rows.append(row);
    rebuildDisplayOrder();
    update();
}

void AntTable::removeRow(int index)
{
    const int sourceIndex = sourceRowIndex(index);
    if (sourceIndex < 0 || sourceIndex >= m_rows.size())
    {
        return;
    }
    m_rows.remove(sourceIndex);
    rebuildDisplayOrder();
    if (m_hoveredRow >= m_rows.size())
    {
        m_hoveredRow = -1;
    }
    update();
}

void AntTable::setRows(const QVector<AntTableRow>& rows)
{
    m_rows = rows;
    m_scrollY = 0;
    m_hoveredRow = -1;
    rebuildDisplayOrder();
    update();
}

void AntTable::clearRows()
{
    m_rows.clear();
    m_displayOrder.clear();
    m_scrollY = 0;
    m_hoveredRow = -1;
    update();
}

int AntTable::rowCount() const
{
    return m_rows.size();
}

AntTableRow AntTable::rowAt(int index) const
{
    const int sourceIndex = sourceRowIndex(index);
    if (sourceIndex < 0 || sourceIndex >= m_rows.size())
    {
        return {};
    }
    return m_rows.at(sourceIndex);
}

// ─── Properties ───

bool AntTable::isBordered() const { return m_bordered; }

void AntTable::setBordered(bool bordered)
{
    if (m_bordered == bordered)
    {
        return;
    }
    m_bordered = bordered;
    update();
    Q_EMIT borderedChanged(m_bordered);
}

Ant::Size AntTable::tableSize() const { return m_tableSize; }

void AntTable::setTableSize(Ant::Size size)
{
    if (m_tableSize == size)
    {
        return;
    }
    m_tableSize = size;
    m_scrollY = 0;
    updateGeometry();
    update();
    Q_EMIT tableSizeChanged(m_tableSize);
}

bool AntTable::isLoading() const { return m_loading; }

void AntTable::setLoading(bool loading)
{
    if (m_loading == loading)
    {
        return;
    }
    m_loading = loading;
    update();
    Q_EMIT loadingChanged(m_loading);
}

Ant::TableSelectionMode AntTable::rowSelection() const { return m_rowSelection; }

void AntTable::setRowSelection(Ant::TableSelectionMode mode)
{
    if (m_rowSelection == mode)
    {
        return;
    }
    m_rowSelection = mode;
    // Clear selections when mode changes
    for (auto& row : m_rows)
    {
        row.selected = false;
    }
    update();
    Q_EMIT rowSelectionChanged(m_rowSelection);
}

// ─── Sorting ───

QString AntTable::currentSortColumn() const { return m_sortColumn; }

Ant::TableSortOrder AntTable::sortOrder() const { return m_sortOrder; }

void AntTable::setSortOrder(const QString& columnKey, Ant::TableSortOrder order)
{
    const QString nextColumn = order == Ant::TableSortOrder::None ? QString() : columnKey;
    if (m_sortColumn == nextColumn && m_sortOrder == order)
    {
        return;
    }
    m_sortColumn = nextColumn;
    m_sortOrder = order;
    m_scrollY = 0;
    rebuildDisplayOrder();
    update();
    Q_EMIT sortChanged(m_sortColumn, m_sortOrder);
}

// ─── Pagination ───

int AntTable::currentPage() const { return m_currentPage; }

void AntTable::setCurrentPage(int page)
{
    const int total = totalPages();
    page = qBound(1, page, qMax(1, total));
    if (m_currentPage == page)
    {
        return;
    }
    m_currentPage = page;
    m_scrollY = 0;
    update();
    Q_EMIT pageChanged(m_currentPage);
}

int AntTable::pageSize() const { return m_pageSize; }

void AntTable::setPageSize(int size)
{
    size = qMax(1, size);
    if (m_pageSize == size)
    {
        return;
    }
    m_pageSize = size;
    m_currentPage = qBound(1, m_currentPage, qMax(1, totalPages()));
    m_scrollY = 0;
    update();
    Q_EMIT pageSizeChanged(m_pageSize);
}

int AntTable::totalPages() const
{
    if (m_pageSize <= 0)
    {
        return 1;
    }
    return qMax(1, (m_rows.size() + m_pageSize - 1) / m_pageSize);
}

QSize AntTable::sizeHint() const
{
    const TableMetrics m = metrics();
    const int selectionWidth = (m_rowSelection != Ant::TableSelectionMode::None) ? m.selectionColWidth : 0;
    const int tableWidth = qMax(520, selectionWidth + totalColumnsWidth());
    const int visibleRows = m_rows.isEmpty() ? 3 : qMin(m_rows.size(), m_pageSize);
    const int bodyH = m_rows.isEmpty() ? 180 : visibleRows * m.rowHeight;
    const int paginationH = (m_rows.size() > m_pageSize) ? 48 : 0;
    return QSize(tableWidth, m.headerHeight + bodyH + paginationH);
}

QSize AntTable::minimumSizeHint() const
{
    const TableMetrics m = metrics();
    return QSize(320, m.headerHeight + m.rowHeight);
}

// ─── Layout accessors ───

int AntTable::scrollY() const { return m_scrollY; }

int AntTable::hoveredRow() const { return m_hoveredRow; }

// ─── Selection helpers ───

QStringList AntTable::selectedRowKeys() const
{
    QStringList keys;
    for (int displayIndex = 0; displayIndex < m_rows.size(); ++displayIndex)
    {
        const int sourceIndex = sourceRowIndex(displayIndex);
        if (sourceIndex >= 0 && sourceIndex < m_rows.size() && m_rows.at(sourceIndex).selected)
        {
            const QString primaryKey = m_columns.isEmpty()
                                           ? QString::number(displayIndex)
                                           : m_rows.at(sourceIndex).data.value(m_columns.first().dataIndex).toString();
            keys.append(primaryKey);
        }
    }
    return keys;
}

// ─── Events ───

void AntTable::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntTable::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || m_loading)
    {
        QWidget::mousePressEvent(event);
        return;
    }

    const QPoint pos = event->pos();

    // Pagination area
    if (isInPagination(pos))
    {
        const int btnIndex = paginationButtonAtPos(pos);
        if (btnIndex >= 0)
        {
            m_pressedPageButton = btnIndex;
            setCurrentPage(btnIndex + 1);
            m_pressedPageButton = -1;
            update();
        }
        event->accept();
        return;
    }

    // Header area
    if (isInHeader(pos))
    {
        // Select-all checkbox
        if (m_rowSelection != Ant::TableSelectionMode::None && isInSelectionColumn(pos))
        {
            toggleSelectAll();
            event->accept();
            return;
        }

        // Column sort
        const QString colKey = columnKeyAtPos(pos);
        if (!colKey.isEmpty())
        {
            const AntTableColumn col = [this, &colKey]() {
                for (const auto& c : m_columns)
                {
                    if (c.key == colKey)
                    {
                        return c;
                    }
                }
                return AntTableColumn{};
            }();

            if (col.sorter)
            {
                Ant::TableSortOrder nextOrder = Ant::TableSortOrder::None;
                if (m_sortColumn != colKey)
                {
                    nextOrder = Ant::TableSortOrder::Ascending;
                }
                else if (m_sortOrder == Ant::TableSortOrder::Ascending)
                {
                    nextOrder = Ant::TableSortOrder::Descending;
                }
                else
                {
                    nextOrder = Ant::TableSortOrder::None;
                }
                setSortOrder(colKey, nextOrder);
            }
        }
        event->accept();
        return;
    }

    // Body area
    const int rowIdx = rowAtPos(pos);
    if (rowIdx >= 0 && rowIdx < m_rows.size())
    {
        const AntTableRow clickedRow = rowAt(rowIdx);
        if (clickedRow.disabled)
        {
            event->accept();
            return;
        }

        // Selection column click
        if (m_rowSelection != Ant::TableSelectionMode::None && isInSelectionColumn(pos))
        {
            toggleRowSelection(rowIdx);
            event->accept();
            return;
        }

        // Row click
        Q_EMIT rowClicked(rowIdx);
    }
    event->accept();
}

void AntTable::mouseMoveEvent(QMouseEvent* event)
{
    const int row = rowAtPos(event->pos());
    if (m_hoveredRow != row)
    {
        m_hoveredRow = row;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void AntTable::leaveEvent(QEvent* event)
{
    if (m_hoveredRow != -1)
    {
        m_hoveredRow = -1;
        update();
    }
    QWidget::leaveEvent(event);
}

void AntTable::wheelEvent(QWheelEvent* event)
{
    const TableMetrics m = metrics();
    const int bodyH = bodyHeight();
    const int contentH = visibleRowCount() * m.rowHeight;
    const int maxScroll = qMax(0, contentH - bodyH);

    if (maxScroll <= 0)
    {
        event->accept();
        return;
    }

    const int delta = event->angleDelta().y();
    const int scrollStep = m.rowHeight * 3;
    m_scrollY = qBound(0, m_scrollY - (delta > 0 ? scrollStep : -scrollStep), maxScroll);
    update();
    event->accept();
}

// ─── Private helpers ───

AntTable::TableMetrics AntTable::metrics() const
{
    TableMetrics m;
    const auto& token = antTheme->tokens();
    switch (m_tableSize)
    {
    case Ant::Size::Large:
        m.headerHeight = 64;
        m.rowHeight = 64;
        m.cellVPadding = 16;
        m.fontSize = token.fontSizeLG;
        m.fontSizeSM = token.fontSize;
        break;
    case Ant::Size::Small:
        m.headerHeight = 32;
        m.rowHeight = 40;
        m.cellVPadding = 8;
        m.fontSize = token.fontSize;
        m.fontSizeSM = token.fontSizeSM;
        break;
    default: // Middle
        m.headerHeight = 56;
        m.rowHeight = 56;
        m.cellVPadding = 16;
        m.fontSize = token.fontSize;
        m.fontSizeSM = token.fontSizeSM;
        break;
    }
    return m;
}

int AntTable::visibleColumnCount() const
{
    int count = 0;
    for (const auto& col : m_columns)
    {
        if (!col.hidden)
        {
            ++count;
        }
    }
    return count;
}

int AntTable::totalColumnsWidth() const
{
    int w = 0;
    for (const auto& col : m_columns)
    {
        if (!col.hidden)
        {
            w += qMax(col.minWidth, col.width);
        }
    }
    return w;
}

int AntTable::bodyHeight() const
{
    const bool hasPagination = m_rows.size() > m_pageSize;
    const int paginationH = hasPagination ? 48 : 0;
    return height() - metrics().headerHeight - paginationH;
}

int AntTable::visibleRowCount() const
{
    const int start = pageStartIndex();
    const int end = pageEndIndex();
    return qMax(0, end - start);
}

int AntTable::rowAtPos(const QPoint& pos) const
{
    const TableMetrics m = metrics();
    const int bodyTop = m.headerHeight;
    const int bodyBottom = bodyTop + bodyHeight();

    if (pos.y() < bodyTop || pos.y() >= bodyBottom)
    {
        return -1;
    }

    const int relY = pos.y() - bodyTop + m_scrollY;
    const int localIdx = relY / m.rowHeight;
    const int start = pageStartIndex();
    const int rowIdx = start + localIdx;

    if (rowIdx < start || rowIdx >= pageEndIndex())
    {
        return -1;
    }
    return rowIdx;
}

QString AntTable::columnKeyAtPos(const QPoint& pos) const
{
    const int selW = (m_rowSelection != Ant::TableSelectionMode::None) ? metrics().selectionColWidth : 0;
    int x = selW;

    for (const auto& col : m_columns)
    {
        if (col.hidden)
        {
            continue;
        }
        const int colW = qMax(col.minWidth, col.width);
        if (pos.x() >= x && pos.x() < x + colW)
        {
            return col.key;
        }
        x += colW;
    }
    return {};
}

bool AntTable::isInSelectionColumn(const QPoint& pos) const
{
    if (m_rowSelection == Ant::TableSelectionMode::None)
    {
        return false;
    }
    return pos.x() >= 0 && pos.x() < metrics().selectionColWidth;
}

bool AntTable::isInHeader(const QPoint& pos) const
{
    return pos.y() >= 0 && pos.y() < metrics().headerHeight;
}

bool AntTable::isInPagination(const QPoint& pos) const
{
    const bool hasPagination = m_rows.size() > m_pageSize;
    if (!hasPagination)
    {
        return false;
    }
    const int paginationTop = metrics().headerHeight + bodyHeight();
    return pos.y() >= paginationTop && pos.y() < height();
}

int AntTable::paginationButtonAtPos(const QPoint& pos) const
{
    if (!isInPagination(pos))
    {
        return -1;
    }

    const int btnSize = 32;
    const int gap = 4;
    const int pages = totalPages();
    if (pages <= 1)
    {
        return -1;
    }

    const int buttonsWidth = pages * (btnSize + gap) - gap;
    const int startX = width() - 16 - buttonsWidth;
    const int paginationTop = metrics().headerHeight + bodyHeight();
    const int buttonY = paginationTop + (48 - btnSize) / 2;

    for (int index = 0; index < pages; ++index)
    {
        const QRect buttonRect(startX + index * (btnSize + gap), buttonY, btnSize, btnSize);
        if (buttonRect.contains(pos))
        {
            return index;
        }
    }

    return -1;
}

int AntTable::pageStartIndex() const
{
    return (m_currentPage - 1) * m_pageSize;
}

int AntTable::pageEndIndex() const
{
    return qMin(m_currentPage * m_pageSize, m_rows.size());
}

void AntTable::rebuildDisplayOrder()
{
    m_displayOrder.clear();
    m_displayOrder.reserve(m_rows.size());
    for (int i = 0; i < m_rows.size(); ++i)
    {
        m_displayOrder.append(i);
    }

    if (m_sortColumn.isEmpty() || m_sortOrder == Ant::TableSortOrder::None)
    {
        m_currentPage = qBound(1, m_currentPage, qMax(1, totalPages()));
        return;
    }

    const QString dataIndex = sortDataIndex();
    if (dataIndex.isEmpty())
    {
        m_currentPage = qBound(1, m_currentPage, qMax(1, totalPages()));
        return;
    }

    const bool descending = m_sortOrder == Ant::TableSortOrder::Descending;
    std::stable_sort(m_displayOrder.begin(), m_displayOrder.end(),
        [this, &dataIndex, descending](int leftIndex, int rightIndex) {
            const QVariant leftValue = m_rows.at(leftIndex).data.value(dataIndex);
            const QVariant rightValue = m_rows.at(rightIndex).data.value(dataIndex);
            const bool leftEmpty = !leftValue.isValid() || leftValue.isNull();
            const bool rightEmpty = !rightValue.isValid() || rightValue.isNull();

            if (leftEmpty || rightEmpty)
            {
                return !leftEmpty && rightEmpty;
            }

            const int compare = compareSortValues(leftValue, rightValue);
            return descending ? compare > 0 : compare < 0;
        });

    m_currentPage = qBound(1, m_currentPage, qMax(1, totalPages()));
}

int AntTable::sourceRowIndex(int displayIndex) const
{
    if (displayIndex < 0 || displayIndex >= m_displayOrder.size())
    {
        return -1;
    }
    return m_displayOrder.at(displayIndex);
}

QString AntTable::sortDataIndex() const
{
    for (const auto& col : m_columns)
    {
        if (col.key == m_sortColumn)
        {
            return col.dataIndex;
        }
    }
    return {};
}

bool AntTable::isAllSelected() const
{
    const int start = pageStartIndex();
    const int end = pageEndIndex();
    for (int i = start; i < end; ++i)
    {
        const int sourceIndex = sourceRowIndex(i);
        if (sourceIndex >= 0 && sourceIndex < m_rows.size()
            && !m_rows.at(sourceIndex).disabled && !m_rows.at(sourceIndex).selected)
        {
            return false;
        }
    }
    return start < end;
}

void AntTable::toggleSelectAll()
{
    const bool selectAll = !isAllSelected();
    const int start = pageStartIndex();
    const int end = pageEndIndex();
    for (int i = start; i < end; ++i)
    {
        const int sourceIndex = sourceRowIndex(i);
        if (sourceIndex >= 0 && sourceIndex < m_rows.size() && !m_rows.at(sourceIndex).disabled)
        {
            m_rows[sourceIndex].selected = selectAll;
        }
    }
    update();
    Q_EMIT selectionChanged(selectedRowKeys());
}

void AntTable::toggleRowSelection(int index)
{
    const int sourceIndex = sourceRowIndex(index);
    if (sourceIndex < 0 || sourceIndex >= m_rows.size() || m_rows.at(sourceIndex).disabled)
    {
        return;
    }

    if (m_rowSelection == Ant::TableSelectionMode::Radio)
    {
        // Deselect all others
        for (int i = 0; i < m_rows.size(); ++i)
        {
            m_rows[i].selected = (i == sourceIndex);
        }
    }
    else
    {
        m_rows[sourceIndex].selected = !m_rows.at(sourceIndex).selected;
    }
    update();
    Q_EMIT selectionChanged(selectedRowKeys());
}
