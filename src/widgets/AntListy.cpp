#include "AntListy.h"

#include "core/AntTheme.h"
#include "AntScrollBar.h"

#include <QAbstractListModel>
#include <QEvent>
#include <QListView>
#include <QPainter>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

#include <functional>

namespace
{
constexpr int kRowHeight = 40;
constexpr int kGroupHeaderHeight = 32;

enum ListyRole
{
    TitleRole = Qt::UserRole,
    DescriptionRole,
    KeyRole,
    GroupRole,
    RowKindRole, // 0 = item, 1 = group header, 2 = loading footer
};

struct ListyRow
{
    int kind = 0;
    QString key;
    QString title;
    QString description;
    QString group;
};
} // namespace

// 内部模型：持有扁平行（条目 + 分组头 + 加载行），对外不可见。
class AntListyModel : public QAbstractListModel
{
public:
    explicit AntListyModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : m_rows.size();
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
        {
            return {};
        }
        const ListyRow& row = m_rows.at(index.row());
        switch (role)
        {
        case TitleRole: return row.title;
        case DescriptionRole: return row.description;
        case KeyRole: return row.key;
        case GroupRole: return row.group;
        case RowKindRole: return row.kind;
        default: return {};
        }
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        if (!index.isValid())
        {
            return Qt::NoItemFlags;
        }
        const int kind = m_rows.at(index.row()).kind;
        if (kind == 2)
        {
            // 加载行不可交互
            return Qt::NoItemFlags;
        }
        if (kind == 1)
        {
            // 分组头不可选中、不可拖动，但可作为放置目标
            return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
        }
        Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        if (m_dragSorting)
        {
            f |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        }
        return f;
    }

    Qt::DropActions supportedDropActions() const override { return Qt::MoveAction; }

    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count,
                  const QModelIndex& destinationParent, int destinationChild) override
    {
        if (m_grouping)
        {
            // 分组模式下拖动会打乱分组头位置，直接拒绝
            return false;
        }
        const int lastSource = sourceRow + count - 1;
        if (sourceRow < 0 || lastSource >= m_items.size() || destinationChild < 0 ||
            destinationChild > m_items.size() || count <= 0)
        {
            return false;
        }
        if (!beginMoveRows(sourceParent, sourceRow, lastSource, destinationParent, destinationChild))
        {
            return false;
        }
        QList<AntListyItem> moved = m_items.mid(sourceRow, count);
        m_items.remove(sourceRow, count);
        const int adjusted = destinationChild > sourceRow ? destinationChild - count : destinationChild;
        for (int i = 0; i < moved.size(); ++i)
        {
            m_items.insert(adjusted + i, moved.at(i));
        }
        endMoveRows();
        rebuildRows();
        if (onOrderChanged)
        {
            onOrderChanged();
        }
        return true;
    }

    void setItems(const QList<AntListyItem>& items)
    {
        beginResetModel();
        m_items = items;
        rebuildRows();
        endResetModel();
    }

    void append(const QList<AntListyItem>& items)
    {
        if (items.isEmpty())
        {
            return;
        }
        beginResetModel();
        m_items.append(items);
        rebuildRows();
        endResetModel();
    }

    void clearItems()
    {
        beginResetModel();
        m_items.clear();
        rebuildRows();
        endResetModel();
    }

    void setGrouping(bool grouping)
    {
        if (m_grouping == grouping)
        {
            return;
        }
        beginResetModel();
        m_grouping = grouping;
        rebuildRows();
        endResetModel();
    }

    void setLoading(bool loading)
    {
        if (m_loading == loading)
        {
            return;
        }
        // 加载行会改变 rowCount，必须走 reset
        beginResetModel();
        m_loading = loading;
        rebuildRows();
        endResetModel();
    }

    void setDragSorting(bool sorting)
    {
        m_dragSorting = sorting;
    }

    const QList<AntListyItem>& items() const { return m_items; }

    // 内部模型不用 Q_OBJECT，通过回调通知拖拽排序完成。
    std::function<void()> onOrderChanged;

    // 数据行号 -> items 下标（跳过分组头和加载行）
    int itemIndexForRow(int row) const
    {
        if (row < 0 || row >= m_rows.size() || m_rows.at(row).kind != 0)
        {
            return -1;
        }
        int itemIndex = -1;
        for (int r = 0; r <= row; ++r)
        {
            if (m_rows.at(r).kind == 0)
            {
                ++itemIndex;
            }
        }
        return itemIndex;
    }

    int rowForKey(const QString& key) const
    {
        for (int r = 0; r < m_rows.size(); ++r)
        {
            if (m_rows.at(r).kind == 0 && m_rows.at(r).key == key)
            {
                return r;
            }
        }
        return -1;
    }

    QString groupAtRow(int row) const
    {
        for (int r = qMin(row, m_rows.size() - 1); r >= 0; --r)
        {
            if (m_rows.at(r).kind == 1)
            {
                return m_rows.at(r).group;
            }
            if (m_rows.at(r).kind == 0 && !m_rows.at(r).group.isEmpty())
            {
                return m_rows.at(r).group;
            }
        }
        return {};
    }

    int rowKind(int row) const
    {
        return (row >= 0 && row < m_rows.size()) ? m_rows.at(row).kind : -1;
    }

private:
    void rebuildRows()
    {
        m_rows.clear();
        if (m_grouping)
        {
            QString lastGroup;
            bool first = true;
            for (const AntListyItem& item : m_items)
            {
                if (first || item.group != lastGroup)
                {
                    ListyRow header;
                    header.kind = 1;
                    header.group = item.group;
                    header.title = item.group;
                    m_rows.append(header);
                    lastGroup = item.group;
                    first = false;
                }
                ListyRow row;
                row.kind = 0;
                row.key = item.key;
                row.title = item.title;
                row.description = item.description;
                row.group = item.group;
                m_rows.append(row);
            }
        }
        else
        {
            for (const AntListyItem& item : m_items)
            {
                ListyRow row;
                row.kind = 0;
                row.key = item.key;
                row.title = item.title;
                row.description = item.description;
                row.group = item.group;
                m_rows.append(row);
            }
        }
        if (m_loading)
        {
            ListyRow footer;
            footer.kind = 2;
            footer.title = QStringLiteral("Loading...");
            m_rows.append(footer);
        }
    }

    QList<AntListyItem> m_items;
    QList<ListyRow> m_rows;
    bool m_grouping = false;
    bool m_loading = false;
    bool m_dragSorting = false;
};

// 吸顶分组指示 chip，悬浮在列表顶部。
class AntListyGroupChip : public QWidget
{
public:
    explicit AntListyGroupChip(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        hide();
    }

    void setText(const QString& text)
    {
        if (m_text == text)
        {
            return;
        }
        m_text = text;
        updateGeometry();
        update();
    }

    QSize sizeHint() const override
    {
        const QFontMetrics fm(font());
        return QSize(fm.horizontalAdvance(m_text) + 20, 24);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& tokens = AntTheme::instance()->tokens();
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);

        QColor bg = tokens.colorBgElevated;
        painter.setPen(QPen(tokens.colorBorderSecondary, 1));
        painter.setBrush(bg);
        painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 6, 6);

        painter.setPen(tokens.colorTextSecondary);
        painter.drawText(rect(), Qt::AlignCenter, m_text);
    }

private:
    QString m_text;
};

// 行绘制：Ant token 驱动的 item delegate。
class AntListyItemDelegate : public QStyledItemDelegate
{
public:
    explicit AntListyItemDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        Q_UNUSED(option)
        const int kind = index.data(RowKindRole).toInt();
        return QSize(100, kind == 1 ? kGroupHeaderHeight : kRowHeight);
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        const auto& tokens = AntTheme::instance()->tokens();
        const int kind = index.data(RowKindRole).toInt();
        const QRect r = option.rect;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setRenderHint(QPainter::TextAntialiasing, true);

        if (kind == 1)
        {
            // 分组头：小字标题 + 底部分隔
            QFont f = option.font;
            f.setBold(true);
            painter->setFont(f);
            painter->setPen(tokens.colorTextSecondary);
            painter->drawText(r.adjusted(12, 0, -8, 0), Qt::AlignVCenter | Qt::AlignLeft, index.data(TitleRole).toString());
            painter->setPen(QPen(tokens.colorSplit, 1));
            painter->drawLine(r.left() + 12, r.bottom(), r.right(), r.bottom());
        }
        else if (kind == 2)
        {
            painter->setPen(tokens.colorTextTertiary);
            painter->drawText(r, Qt::AlignCenter, index.data(TitleRole).toString());
        }
        else
        {
            const bool selected = option.state & QStyle::State_Selected;
            const bool hovered = option.state & QStyle::State_MouseOver;
            if (selected)
            {
                painter->setPen(Qt::NoPen);
                painter->setBrush(tokens.colorPrimaryBg);
                painter->drawRoundedRect(r.adjusted(4, 2, -4, -2), 6, 6);
            }
            else if (hovered)
            {
                painter->setPen(Qt::NoPen);
                painter->setBrush(tokens.colorFillTertiary);
                painter->drawRoundedRect(r.adjusted(4, 2, -4, -2), 6, 6);
            }

            const QString title = index.data(TitleRole).toString();
            const QString description = index.data(DescriptionRole).toString();
            QRect textRect = r.adjusted(16, 0, -12, 0);
            if (description.isEmpty())
            {
                painter->setPen(selected ? tokens.colorPrimary : tokens.colorText);
                painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, title);
            }
            else
            {
                painter->setPen(selected ? tokens.colorPrimary : tokens.colorText);
                painter->drawText(textRect.adjusted(0, 5, 0, 0), Qt::AlignTop | Qt::AlignLeft, title);
                painter->setPen(tokens.colorTextSecondary);
                QFont f = option.font;
                f.setPointSizeF(f.pointSizeF() - 1.0);
                painter->setFont(f);
                painter->drawText(textRect.adjusted(0, 21, 0, 0), Qt::AlignTop | Qt::AlignLeft, description);
            }
        }
        painter->restore();
    }
};

AntListy::AntListy(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_model = new AntListyModel(this);
    m_view = new QListView(this);
    m_view->setModel(m_model);
    m_view->setItemDelegate(new AntListyItemDelegate(m_view));
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_view->setVerticalScrollBar(new AntScrollBar(Qt::Vertical, m_view));
    m_view->setUniformItemSizes(false);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(m_view);

    m_stickyChip = new AntListyGroupChip(m_view->viewport());

    connect(m_view, &QListView::clicked, this, &AntListy::onViewClicked);
    m_model->onOrderChanged = [this]() { Q_EMIT orderChanged(); };
    connect(m_view->verticalScrollBar(), &QScrollBar::valueChanged, this, &AntListy::onScrolled);
    connect(m_view->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, [this](const QModelIndex& current, const QModelIndex&) {
        const int itemIndex = current.isValid() ? m_model->itemIndexForRow(current.row()) : -1;
        Q_EMIT currentRowChanged(itemIndex);
    });

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(160, 120);
}

AntListy::~AntListy() = default;

void AntListy::setItems(const QList<AntListyItem>& items)
{
    m_model->setItems(items);
    m_loadMoreArmed = true;
    syncStickyChip();
}

void AntListy::addItem(const AntListyItem& item)
{
    m_model->append({item});
    syncStickyChip();
}

void AntListy::addItem(const QString& title, const QString& group)
{
    AntListyItem item;
    item.title = title;
    item.group = group;
    m_model->append({item});
    syncStickyChip();
}

void AntListy::addItems(const QList<AntListyItem>& items)
{
    m_model->append(items);
    syncStickyChip();
}

void AntListy::clear()
{
    m_model->clearItems();
    syncStickyChip();
}

int AntListy::count() const
{
    return m_model->items().size();
}

AntListyItem AntListy::itemAt(int row) const
{
    const auto& items = m_model->items();
    if (row < 0 || row >= items.size())
    {
        return {};
    }
    return items.at(row);
}

QString AntListy::itemKey(int row) const
{
    return itemAt(row).key;
}

bool AntListy::isGroupingEnabled() const { return m_groupingEnabled; }

void AntListy::setGroupingEnabled(bool enabled)
{
    if (m_groupingEnabled == enabled)
    {
        return;
    }
    m_groupingEnabled = enabled;
    m_model->setGrouping(enabled);
    if (enabled)
    {
        setDragSortingEnabled(false);
    }
    syncStickyChip();
    Q_EMIT groupingEnabledChanged(m_groupingEnabled);
}

bool AntListy::hasStickyGroupHeader() const { return m_stickyGroupHeader; }

void AntListy::setStickyGroupHeader(bool sticky)
{
    if (m_stickyGroupHeader == sticky)
    {
        return;
    }
    m_stickyGroupHeader = sticky;
    syncStickyChip();
    Q_EMIT stickyGroupHeaderChanged(m_stickyGroupHeader);
}

bool AntListy::isDragSortingEnabled() const
{
    return m_view->dragDropMode() == QAbstractItemView::InternalMove;
}

void AntListy::setDragSortingEnabled(bool enabled)
{
    const bool current = isDragSortingEnabled();
    if (current == enabled)
    {
        return;
    }
    if (enabled && m_groupingEnabled)
    {
        setGroupingEnabled(false);
    }
    m_model->setDragSorting(enabled);
    m_view->setDragDropMode(enabled ? QAbstractItemView::InternalMove : QAbstractItemView::NoDragDrop);
    m_view->setDefaultDropAction(Qt::MoveAction);
    Q_EMIT dragSortingEnabledChanged(enabled);
}

bool AntListy::isLoading() const { return m_loading; }

void AntListy::setLoading(bool loading)
{
    if (m_loading == loading)
    {
        return;
    }
    m_loading = loading;
    m_model->setLoading(loading);
    Q_EMIT loadingChanged(m_loading);
}

void AntListy::scrollToKey(const QString& key)
{
    const int row = m_model->rowForKey(key);
    if (row >= 0)
    {
        m_view->scrollTo(m_model->index(row), QAbstractItemView::PositionAtCenter);
    }
}

void AntListy::scrollToRow(int row)
{
    if (row < 0 || row >= m_model->rowCount())
    {
        return;
    }
    m_view->scrollTo(m_model->index(row), QAbstractItemView::PositionAtCenter);
}

int AntListy::currentRow() const
{
    const QModelIndex current = m_view->currentIndex();
    return current.isValid() ? m_model->itemIndexForRow(current.row()) : -1;
}

void AntListy::setCurrentRow(int row)
{
    // row 是数据条目下标；需要换算为模型行号
    for (int r = 0; r < m_model->rowCount(); ++r)
    {
        if (m_model->itemIndexForRow(r) == row)
        {
            m_view->setCurrentIndex(m_model->index(r));
            return;
        }
    }
}

QSize AntListy::sizeHint() const
{
    return QSize(280, 240);
}

void AntListy::onViewClicked(const QModelIndex& index)
{
    const int itemIndex = m_model->itemIndexForRow(index.row());
    if (itemIndex >= 0)
    {
        Q_EMIT itemClicked(itemIndex, m_model->items().at(itemIndex).key);
    }
}

void AntListy::onScrolled(int value)
{
    Q_UNUSED(value)
    syncStickyChip();

    // 无限加载：接近底部且不在加载中时触发一次，setItems/addItems 后重新武装
    if (m_loading || !m_loadMoreArmed)
    {
        return;
    }
    auto* bar = m_view->verticalScrollBar();
    if (bar->maximum() > 0 && bar->value() >= bar->maximum() - 40)
    {
        m_loadMoreArmed = false;
        Q_EMIT loadMoreRequested();
    }
}

void AntListy::syncStickyChip()
{
    if (!m_stickyChip)
    {
        return;
    }
    const bool show = m_stickyGroupHeader && m_groupingEnabled && count() > 0;
    if (!show)
    {
        m_stickyChip->hide();
        return;
    }
    const QModelIndex topIndex = m_view->indexAt(QPoint(0, 0));
    const QString group = m_model->groupAtRow(topIndex.isValid() ? topIndex.row() : 0);
    if (group.isEmpty())
    {
        m_stickyChip->hide();
        return;
    }
    // 当前分组的分组头仍在视口内时不显示吸顶 chip，避免遮盖同一分组头
    const int topRow = topIndex.isValid() ? topIndex.row() : 0;
    for (int r = topRow; r >= 0; --r)
    {
        if (m_model->rowKind(r) == 1)
        {
            const QRect headerRect = m_view->visualRect(m_model->index(r));
            if (headerRect.isValid() && headerRect.bottom() >= 0)
            {
                m_stickyChip->hide();
                return;
            }
            break;
        }
    }
    m_stickyChip->setText(group);
    m_stickyChip->adjustSize();
    m_stickyChip->move(8, 6);
    m_stickyChip->show();
    m_stickyChip->raise();
}
