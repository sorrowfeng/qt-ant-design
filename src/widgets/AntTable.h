#pragma once

#include "core/QtAntDesignExport.h"

#include <QMap>
#include <QStringList>
#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QMouseEvent;
class QWheelEvent;

struct AntTableColumn
{
    QString title;
    QString dataIndex;
    QString key;
    int width = 120;
    int minWidth = 50;
    Ant::TableColumnAlign align = Ant::TableColumnAlign::Left;
    bool fixed = false;
    bool sorter = false;
    QStringList filters;
    bool hidden = false;
};

struct AntTableRow
{
    QMap<QString, QVariant> data;
    bool selected = false;
    bool expanded = false;
    bool disabled = false;
};

class QT_ANT_DESIGN_EXPORT AntTable : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool bordered READ isBordered WRITE setBordered NOTIFY borderedChanged)
    Q_PROPERTY(Ant::Size tableSize READ tableSize WRITE setTableSize NOTIFY tableSizeChanged)
    Q_PROPERTY(bool loading READ isLoading WRITE setLoading NOTIFY loadingChanged)
    Q_PROPERTY(Ant::TableSelectionMode rowSelection READ rowSelection WRITE setRowSelection NOTIFY rowSelectionChanged)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY pageChanged)
    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)

public:
    explicit AntTable(QWidget* parent = nullptr);

    // Column management
    void addColumn(const AntTableColumn& column);
    void removeColumn(const QString& key);
    void setColumns(const QVector<AntTableColumn>& columns);
    QVector<AntTableColumn> columns() const;
    int columnCount() const;
    AntTableColumn columnAt(int index) const;
    QStringList headerLabels() const;

    // Row management
    void addRow(const AntTableRow& row);
    void removeRow(int index);
    void setRows(const QVector<AntTableRow>& rows);
    void clearRows();
    int rowCount() const;
    AntTableRow rowAt(int index) const;
    QVariant cellData(int row, const QString& dataIndex) const;
    void setData(int row, const QString& dataIndex, const QVariant& value);

    // Properties
    bool isBordered() const;
    void setBordered(bool bordered);

    Ant::Size tableSize() const;
    void setTableSize(Ant::Size size);

    bool isLoading() const;
    void setLoading(bool loading);

    Ant::TableSelectionMode rowSelection() const;
    void setRowSelection(Ant::TableSelectionMode mode);

    // Sorting
    QString currentSortColumn() const;
    Ant::TableSortOrder sortOrder() const;
    void setSortOrder(const QString& columnKey, Ant::TableSortOrder order);

    // Pagination
    int currentPage() const;
    void setCurrentPage(int page);

    int pageSize() const;
    void setPageSize(int size);

    int totalPages() const;

    // Selection helpers
    QStringList selectedRowKeys() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    // Layout accessors (for style drawing)
    int scrollY() const;
    int hoveredRow() const;

Q_SIGNALS:
    void rowClicked(int index);
    void selectionChanged(const QStringList& keys);
    void sortChanged(const QString& key, Ant::TableSortOrder order);
    void pageChanged(int page);
    void pageSizeChanged(int size);
    void borderedChanged(bool bordered);
    void tableSizeChanged(Ant::Size size);
    void loadingChanged(bool loading);
    void rowSelectionChanged(Ant::TableSelectionMode mode);
    void columnsChanged();
    void rowsChanged();
    void cellDataChanged(int row, const QString& dataIndex, const QVariant& value);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    struct TableMetrics
    {
        int headerHeight = 40;
        int rowHeight = 48;
        int selectionColWidth = 48;
        int cellHPadding = 12;
        int cellVPadding = 12;
        int fontSize = 14;
        int fontSizeSM = 12;
    };

    TableMetrics metrics() const;
    int visibleColumnCount() const;
    int totalColumnsWidth() const;
    int bodyHeight() const;
    int visibleRowCount() const;
    int rowAtPos(const QPoint& pos) const;
    QString columnKeyAtPos(const QPoint& pos) const;
    bool isInSelectionColumn(const QPoint& pos) const;
    bool isInHeader(const QPoint& pos) const;
    bool isInPagination(const QPoint& pos) const;
    int paginationButtonAtPos(const QPoint& pos) const;
    int pageStartIndex() const;
    int pageEndIndex() const;
    void updatePageButtons();
    bool isAllSelected() const;
    void toggleSelectAll();
    void toggleRowSelection(int index);
    void rebuildDisplayOrder();
    int sourceRowIndex(int displayIndex) const;
    QString sortDataIndex() const;

    QVector<AntTableColumn> m_columns;
    QVector<AntTableRow> m_rows;
    QVector<int> m_displayOrder;

    bool m_bordered = true;
    Ant::Size m_tableSize = Ant::Size::Middle;
    bool m_loading = false;
    Ant::TableSelectionMode m_rowSelection = Ant::TableSelectionMode::None;

    QString m_sortColumn;
    Ant::TableSortOrder m_sortOrder = Ant::TableSortOrder::None;

    int m_currentPage = 1;
    int m_pageSize = 10;
    int m_scrollY = 0;

    int m_hoveredRow = -1;
    QString m_pressedColumn;
    int m_pressedPageButton = -1;
};
