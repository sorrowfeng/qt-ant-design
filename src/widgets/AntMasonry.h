#pragma once

#include "core/QtAntDesignExport.h"

#include <QRect>
#include <QVector>
#include <QWidget>

class QT_ANT_DESIGN_EXPORT AntMasonry : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int columns READ columns WRITE setColumns NOTIFY columnsChanged)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing NOTIFY spacingChanged)

public:
    explicit AntMasonry(QWidget* parent = nullptr);

    int columns() const;
    void setColumns(int cols);

    int spacing() const;
    void setSpacing(int px);

    void addWidget(QWidget* widget);
    void clear();

Q_SIGNALS:
    void columnsChanged(int cols);
    void spacingChanged(int px);

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    void relayout();
    void markLayoutDirty();
    void resetLayoutCache();
    int columnWidthFor(int layoutWidth) const;
    int itemHeightFor(QWidget* widget, int columnWidth);
    int shortestColumn() const;
    bool canAppendIncrementally() const;
    bool appendItemLayout(QWidget* widget);
    bool applyItemGeometry(QWidget* widget, const QRect& geometry);
    void updateMinimumHeightFromColumns();
    void updateTheme();
    void syncPerfCounters();

    int m_columns = 3;
    int m_spacing = 8;
    QList<QWidget*> m_items;
    QVector<int> m_columnHeights;
    int m_cachedWidth = -1;
    int m_cachedColumns = -1;
    int m_cachedSpacing = -1;
    int m_cachedColumnWidth = -1;
    int m_cachedItemCount = 0;
    int m_fullRelayoutCount = 0;
    int m_incrementalLayoutCount = 0;
    int m_geometryApplyCount = 0;
    int m_heightQueryCount = 0;
    bool m_layoutDirty = true;
};
