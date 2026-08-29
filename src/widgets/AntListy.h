#pragma once

#include "core/QtAntDesignExport.h"

#include <QList>
#include <QSize>
#include <QString>
#include <QWidget>

class QListView;
class QModelIndex;
class AntListyModel;
class AntListyGroupChip;

// 列表条目（对应上游 Listy 的 items 元素）。
struct AntListyItem
{
    QString key;
    QString title;
    QString description;
    QString group;

    bool operator==(const AntListyItem& other) const
    {
        return key == other.key && title == other.title &&
               description == other.description && group == other.group;
    }
    bool operator!=(const AntListyItem& other) const { return !(*this == other); }
};

// Ant Design 6.6+ Listy: 面向大数据量的高性能列表。
// 基于 Qt Model/View（QListView 原生虚拟化，仅绘制可视行），
// 支持分组头、吸顶分组指示、拖拽排序、无限加载和按键滚动定位。
class QT_ANT_DESIGN_EXPORT AntListy : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(bool groupingEnabled READ isGroupingEnabled WRITE setGroupingEnabled NOTIFY groupingEnabledChanged)
    Q_PROPERTY(bool stickyGroupHeader READ hasStickyGroupHeader WRITE setStickyGroupHeader NOTIFY stickyGroupHeaderChanged)
    Q_PROPERTY(bool dragSortingEnabled READ isDragSortingEnabled WRITE setDragSortingEnabled NOTIFY dragSortingEnabledChanged)
    Q_PROPERTY(bool loading READ isLoading WRITE setLoading NOTIFY loadingChanged)
    Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)

public:
    explicit AntListy(QWidget* parent = nullptr);
    ~AntListy() override;

    // ---- 数据 ----
    void setItems(const QList<AntListyItem>& items);
    void addItem(const AntListyItem& item);
    void addItem(const QString& title, const QString& group = QString());
    void addItems(const QList<AntListyItem>& items);
    void clear();
    int count() const;
    AntListyItem itemAt(int row) const;
    QString itemKey(int row) const;

    // ---- 分组 ----
    // 分组开启后，相邻相同 group 的条目前会插入不可选中的分组头行。
    bool isGroupingEnabled() const;
    void setGroupingEnabled(bool enabled);
    // 吸顶：滚动时在列表顶部悬浮显示当前可视区域的分组名。
    bool hasStickyGroupHeader() const;
    void setStickyGroupHeader(bool sticky);

    // ---- 拖拽排序 ----
    bool isDragSortingEnabled() const;
    void setDragSortingEnabled(bool enabled);

    // ---- 无限加载 ----
    // loading 为 true 时列表底部显示加载行；滚动到底部阈值时发 loadMoreRequested()。
    bool isLoading() const;
    void setLoading(bool loading);

    // ---- 滚动定位 ----
    void scrollToKey(const QString& key);
    void scrollToRow(int row);
    int currentRow() const;
    void setCurrentRow(int row);

    QSize sizeHint() const override;

Q_SIGNALS:
    void itemClicked(int row, const QString& key);
    void orderChanged();
    void loadMoreRequested();
    void currentRowChanged(int row);
    void groupingEnabledChanged(bool enabled);
    void stickyGroupHeaderChanged(bool sticky);
    void dragSortingEnabledChanged(bool enabled);
    void loadingChanged(bool loading);

private:
    friend class AntListyModel;

    void onViewClicked(const QModelIndex& index);
    void onScrolled(int value);
    void syncStickyChip();

    QListView* m_view = nullptr;
    AntListyModel* m_model = nullptr;
    AntListyGroupChip* m_stickyChip = nullptr;
    bool m_groupingEnabled = false;
    bool m_stickyGroupHeader = false;
    bool m_loading = false;
    bool m_loadMoreArmed = true;
};
