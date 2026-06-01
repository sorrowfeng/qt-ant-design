#include "AntTable.h"

#include <QHelpEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QSizePolicy>
#include <QWheelEvent>

#include <algorithm>

#include "../styles/AntTableStyle.h"
#include "core/AntTheme.h"
#include "widgets/AntToolTip.h"

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
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_rowToolTipTarget = new QWidget(this);
    m_rowToolTipTarget->setObjectName(QStringLiteral("AntTableRowToolTipTarget"));
    m_rowToolTipTarget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_rowToolTipTarget->setAttribute(Qt::WA_NoSystemBackground, true);
    m_rowToolTipTarget->setAutoFillBackground(false);
    m_rowToolTipTarget->hide();

    m_rowToolTip = new AntToolTip(this);
    m_rowToolTip->setObjectName(QStringLiteral("AntTableRowToolTip"));
    m_rowToolTip->setPlacement(Ant::TooltipPlacement::Top);
    m_rowToolTip->setTarget(m_rowToolTipTarget);
}

// ─── Column management ───

void AntTable::addColumn(const AntTableColumn& column)
{
    m_columns.append(column);
    rebuildDisplayOrder();
    updateGeometry();
    update();
    Q_EMIT columnsChanged();
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
    Q_EMIT columnsChanged();
}

void AntTable::setColumns(const QVector<AntTableColumn>& columns)
{
    m_columns = columns;
    m_sortColumn.clear();
    m_sortOrder = Ant::TableSortOrder::None;
    rebuildDisplayOrder();
    updateGeometry();
    update();
    Q_EMIT columnsChanged();
}

QVector<AntTableColumn> AntTable::columns() const
{
    return m_columns;
}

int AntTable::columnCount() const
{
    return m_columns.size();
}

AntTableColumn AntTable::columnAt(int index) const
{
    if (index < 0 || index >= m_columns.size())
    {
        return {};
    }
    return m_columns.at(index);
}

QStringList AntTable::headerLabels() const
{
    QStringList labels;
    labels.reserve(m_columns.size());
    for (const auto& column : m_columns)
    {
        labels.append(column.title);
    }
    return labels;
}

// ─── Row management ───

void AntTable::addRow(const AntTableRow& row)
{
    m_rows.append(row);
    rebuildDisplayOrder();
    update();
    Q_EMIT rowsChanged();
}

void AntTable::removeRow(int index)
{
    const int sourceIndex = sourceRowIndex(index);
    if (sourceIndex < 0 || sourceIndex >= m_rows.size())
    {
        return;
    }
    if (sourceIndex == m_currentSourceRowIndex)
    {
        m_currentSourceRowIndex = -1;
    }
    else if (sourceIndex < m_currentSourceRowIndex)
    {
        --m_currentSourceRowIndex;
    }
    m_rows.remove(sourceIndex);
    rebuildDisplayOrder();
    if (m_hoveredRow >= m_rows.size())
    {
        m_hoveredRow = -1;
    }
    update();
    Q_EMIT rowsChanged();
}

void AntTable::setRows(const QVector<AntTableRow>& rows)
{
    m_rows = rows;
    m_scrollY = 0;
    m_hoveredRow = -1;
    m_currentSourceRowIndex = -1;
    rebuildDisplayOrder();
    update();
    Q_EMIT rowsChanged();
}

void AntTable::clearRows()
{
    m_rows.clear();
    m_displayOrder.clear();
    m_scrollY = 0;
    m_hoveredRow = -1;
    m_currentSourceRowIndex = -1;
    update();
    Q_EMIT rowsChanged();
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

QVector<AntTableRow> AntTable::rows() const
{
    QVector<AntTableRow> orderedRows;
    orderedRows.reserve(m_rows.size());
    for (int displayIndex = 0; displayIndex < m_rows.size(); ++displayIndex)
    {
        const int sourceIndex = sourceRowIndex(displayIndex);
        if (sourceIndex >= 0 && sourceIndex < m_rows.size())
        {
            orderedRows.append(m_rows.at(sourceIndex));
        }
    }
    return orderedRows;
}

QVariant AntTable::cellData(int row, const QString& dataIndex) const
{
    const int sourceIndex = sourceRowIndex(row);
    if (sourceIndex < 0 || sourceIndex >= m_rows.size())
    {
        return {};
    }
    return m_rows.at(sourceIndex).data.value(dataIndex);
}

void AntTable::setData(int row, const QString& dataIndex, const QVariant& value)
{
    const int sourceIndex = sourceRowIndex(row);
    if (sourceIndex < 0 || sourceIndex >= m_rows.size())
    {
        return;
    }
    if (m_rows.at(sourceIndex).data.value(dataIndex) == value)
    {
        return;
    }

    m_rows[sourceIndex].data[dataIndex] = value;
    if (sortDataIndex() == dataIndex)
    {
        rebuildDisplayOrder();
    }
    update();
    Q_EMIT cellDataChanged(row, dataIndex, value);
    Q_EMIT rowsChanged();
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
    m_currentSourceRowIndex = -1;
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

void AntTable::selectRow(int index)
{
    const int sourceIndex = sourceRowIndex(index);
    if (sourceIndex < 0 || sourceIndex >= m_rows.size() || m_rows.at(sourceIndex).disabled)
    {
        return;
    }

    bool changed = false;
    m_currentSourceRowIndex = sourceIndex;
    if (m_rowSelection != Ant::TableSelectionMode::Checkbox)
    {
        for (int i = 0; i < m_rows.size(); ++i)
        {
            const bool selected = (i == sourceIndex);
            if (m_rows.at(i).selected != selected)
            {
                m_rows[i].selected = selected;
                changed = true;
            }
        }
    }
    else if (!m_rows.at(sourceIndex).selected)
    {
        m_rows[sourceIndex].selected = true;
        changed = true;
    }

    update();
    if (changed)
    {
        Q_EMIT selectionChanged(selectedRowKeys());
    }
}

int AntTable::currentRowIndex() const
{
    return displayRowIndex(m_currentSourceRowIndex);
}

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

bool AntTable::event(QEvent* event)
{
    if (event->type() == QEvent::ToolTip)
    {
        auto* helpEvent = static_cast<QHelpEvent*>(event);
        const int rowIndex = rowAtPos(helpEvent->pos());
        if (rowIndex >= 0)
        {
            const AntTableRow row = rowAt(rowIndex);
            if (!row.tooltip.isEmpty())
            {
                showRowToolTip(rowIndex, row.tooltip);
                event->accept();
                return true;
            }
        }

        hideRowToolTip();
        event->ignore();
        return true;
    }
    if (event->type() == QEvent::Hide || event->type() == QEvent::Resize)
    {
        hideRowToolTip();
    }
    return QWidget::event(event);
}

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
        if (m_rowSelection == Ant::TableSelectionMode::Checkbox)
        {
            m_currentSourceRowIndex = sourceRowIndex(rowIdx);
            update();
        }
        else
        {
            selectRow(rowIdx);
        }
        Q_EMIT rowClicked(rowIdx);
    }
    event->accept();
}

void AntTable::mouseMoveEvent(QMouseEvent* event)
{
    const int row = rowAtPos(event->pos());
    if (m_hoveredRow != row)
    {
        const int previousHoveredRow = m_hoveredRow;
        m_hoveredRow = row;
        updateRows(previousHoveredRow, m_hoveredRow);
        if (m_toolTipDisplayRow >= 0 && m_toolTipDisplayRow != row)
        {
            hideRowToolTip();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void AntTable::leaveEvent(QEvent* event)
{
    if (m_hoveredRow != -1)
    {
        const int previousHoveredRow = m_hoveredRow;
        m_hoveredRow = -1;
        updateRow(previousHoveredRow);
    }
    hideRowToolTip();
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
    hideRowToolTip();
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

QRect AntTable::tableBodyRect() const
{
    const TableMetrics m = metrics();
    return QRect(0, m.headerHeight, width(), qMax(0, bodyHeight()));
}

QRect AntTable::rowUpdateRect(int displayIndex) const
{
    const int start = pageStartIndex();
    const int end = pageEndIndex();
    if (displayIndex < start || displayIndex >= end)
    {
        return QRect();
    }

    const TableMetrics m = metrics();
    const int localIndex = displayIndex - start;
    const int rowTop = m.headerHeight + localIndex * m.rowHeight - m_scrollY;
    const QRect rowRect(0, rowTop, width(), m.rowHeight);
    return rowRect.intersected(tableBodyRect());
}

void AntTable::showRowToolTip(int displayIndex, const QString& text)
{
    if (!m_rowToolTip || !m_rowToolTipTarget)
    {
        return;
    }

    QRect targetRect = rowUpdateRect(displayIndex);
    if (!targetRect.isValid() || targetRect.isEmpty())
    {
        hideRowToolTip();
        return;
    }

    targetRect = targetRect.adjusted(8, 0, -8, 0);
    m_rowToolTipTarget->setGeometry(targetRect);
    m_rowToolTipTarget->show();
    m_rowToolTipTarget->raise();
    m_rowToolTip->setTitle(text);
    m_rowToolTip->showTooltip();
    m_toolTipDisplayRow = displayIndex;
}

void AntTable::hideRowToolTip()
{
    if (m_rowToolTip)
    {
        m_rowToolTip->hideTooltip();
    }
    if (m_rowToolTipTarget)
    {
        m_rowToolTipTarget->hide();
    }
    m_toolTipDisplayRow = -1;
}

void AntTable::updateRow(int displayIndex)
{
    const QRect dirtyRect = rowUpdateRect(displayIndex);
    const int dirtyCount = dirtyRect.isValid() ? 1 : 0;
    setProperty("antTableLastUpdateMode", dirtyCount > 0 ? QStringLiteral("row") : QStringLiteral("none"));
    setProperty("antTableLastRowUpdateCount", dirtyCount);
    if (dirtyRect.isValid())
    {
        update(dirtyRect);
    }
}

void AntTable::updateRows(int firstDisplayIndex, int secondDisplayIndex)
{
    int dirtyCount = 0;
    const QRect firstRect = rowUpdateRect(firstDisplayIndex);
    if (firstRect.isValid())
    {
        update(firstRect);
        ++dirtyCount;
    }

    if (secondDisplayIndex != firstDisplayIndex)
    {
        const QRect secondRect = rowUpdateRect(secondDisplayIndex);
        if (secondRect.isValid())
        {
            update(secondRect);
            ++dirtyCount;
        }
    }

    setProperty("antTableLastUpdateMode", dirtyCount > 0 ? QStringLiteral("row") : QStringLiteral("none"));
    setProperty("antTableLastRowUpdateCount", dirtyCount);
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

int AntTable::displayRowIndex(int sourceIndex) const
{
    if (sourceIndex < 0)
    {
        return -1;
    }
    for (int displayIndex = 0; displayIndex < m_displayOrder.size(); ++displayIndex)
    {
        if (m_displayOrder.at(displayIndex) == sourceIndex)
        {
            return displayIndex;
        }
    }
    return -1;
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
    int firstSelectedSource = -1;
    for (int i = start; i < end; ++i)
    {
        const int sourceIndex = sourceRowIndex(i);
        if (sourceIndex >= 0 && sourceIndex < m_rows.size() && !m_rows.at(sourceIndex).disabled)
        {
            m_rows[sourceIndex].selected = selectAll;
            if (selectAll && firstSelectedSource < 0)
            {
                firstSelectedSource = sourceIndex;
            }
        }
    }
    if (selectAll)
    {
        m_currentSourceRowIndex = firstSelectedSource;
    }
    else if (displayRowIndex(m_currentSourceRowIndex) >= start && displayRowIndex(m_currentSourceRowIndex) < end)
    {
        m_currentSourceRowIndex = -1;
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
    m_currentSourceRowIndex = sourceIndex;
    update();
    Q_EMIT selectionChanged(selectedRowKeys());
}
