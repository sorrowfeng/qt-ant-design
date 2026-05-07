#include "AntTree.h"

#include <algorithm>
#include <functional>

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include "../styles/AntTreeStyle.h"
#include "core/AntTheme.h"

namespace
{
constexpr int RowHeight = 28;
constexpr int IndentWidth = 24;
constexpr int ArrowSize = 8;
constexpr int CheckboxSize = 16;
constexpr int IconSize = 16;
constexpr int ArrowZoneWidth = 24;
constexpr int CheckboxZoneWidth = 24;
constexpr int ScrollStep = 40;
} // namespace

AntTree::AntTree(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntTreeStyle>(this);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumWidth(120);
}

bool AntTree::isSelectable() const { return m_selectable; }

void AntTree::setSelectable(bool selectable)
{
    if (m_selectable == selectable)
    {
        return;
    }
    m_selectable = selectable;
    update();
    Q_EMIT selectableChanged(m_selectable);
}

bool AntTree::isCheckable() const { return m_checkable; }

void AntTree::setCheckable(bool checkable)
{
    if (m_checkable == checkable)
    {
        return;
    }
    m_checkable = checkable;
    update();
    Q_EMIT checkableChanged(m_checkable);
}

bool AntTree::isShowLine() const { return m_showLine; }

void AntTree::setShowLine(bool showLine)
{
    if (m_showLine == showLine)
    {
        return;
    }
    m_showLine = showLine;
    update();
    Q_EMIT showLineChanged(m_showLine);
}

bool AntTree::isShowIcon() const { return m_showIcon; }

void AntTree::setShowIcon(bool showIcon)
{
    if (m_showIcon == showIcon)
    {
        return;
    }
    m_showIcon = showIcon;
    update();
    Q_EMIT showIconChanged(m_showIcon);
}

bool AntTree::isMultiple() const { return m_multiple; }

void AntTree::setMultiple(bool multiple)
{
    if (m_multiple == multiple)
    {
        return;
    }
    m_multiple = multiple;
    if (!m_multiple && m_selectedKeys.size() > 1)
    {
        m_selectedKeys = QStringList{m_selectedKeys.first()};
    }
    update();
    Q_EMIT multipleChanged(m_multiple);
}

void AntTree::setTreeData(const QVector<AntTreeNode>& data)
{
    m_data = data;
    m_scrollY = 0;
    m_hoveredRow = -1;
    m_selectedKeys.clear();
    clampScrollY();
    update();
}

QVector<AntTreeNode> AntTree::treeData() const
{
    return m_data;
}

void AntTree::addNode(const QString& parentKey, const AntTreeNode& node)
{
    if (parentKey.isEmpty())
    {
        m_data.append(node);
    }
    else
    {
        AntTreeNode* parent = findNode(parentKey);
        if (parent)
        {
            parent->children.append(node);
        }
    }
    clampScrollY();
    update();
}

void AntTree::removeNode(const QString& key)
{
    removeNodeRecursive(m_data, key);
    m_selectedKeys.removeAll(key);
    clampScrollY();
    update();
}

AntTreeNode* AntTree::findNode(const QString& key)
{
    return findNodeRecursive(m_data, key);
}

bool AntTree::containsNode(const QString& key) const
{
    return findNodeRecursive(m_data, key) != nullptr;
}

int AntTree::topLevelNodeCount() const
{
    return m_data.size();
}

int AntTree::nodeCount() const
{
    return countNodesRecursive(m_data);
}

void AntTree::clear()
{
    if (m_data.isEmpty() && m_selectedKeys.isEmpty() && m_scrollY == 0 && m_hoveredRow == -1)
    {
        return;
    }

    m_data.clear();
    m_selectedKeys.clear();
    m_scrollY = 0;
    m_hoveredRow = -1;
    updateGeometry();
    update();
}

QStringList AntTree::expandedKeys() const
{
    QStringList keys;
    collectExpandedKeys(m_data, keys);
    return keys;
}

QStringList AntTree::selectedKeys() const
{
    return m_selectedKeys;
}

QStringList AntTree::checkedKeys() const
{
    QStringList keys;
    collectCheckedKeys(m_data, keys);
    return keys;
}

void AntTree::setExpandedKeys(const QStringList& keys)
{
    // Collapse all nodes recursively
    std::function<void(QVector<AntTreeNode>&)> collapseAll = [&](QVector<AntTreeNode>& nodes) {
        for (auto& node : nodes)
        {
            node.expanded = false;
            collapseAll(node.children);
        }
    };
    collapseAll(m_data);

    // Expand matching
    for (const QString& key : keys)
    {
        AntTreeNode* n = findNode(key);
        if (n && !n->isLeaf)
        {
            n->expanded = true;
        }
    }
    clampScrollY();
    update();
}

void AntTree::setSelectedKeys(const QStringList& keys)
{
    m_selectedKeys = m_multiple ? keys : (keys.isEmpty() ? QStringList{} : QStringList{keys.first()});
    update();
    if (!m_selectedKeys.isEmpty())
    {
        Q_EMIT nodeSelected(m_selectedKeys.first());
    }
}

void AntTree::setCheckedKeys(const QStringList& keys)
{
    // Uncheck all nodes recursively
    std::function<void(QVector<AntTreeNode>&)> uncheckAll = [&](QVector<AntTreeNode>& nodes) {
        for (auto& node : nodes)
        {
            node.checked = false;
            node.halfChecked = false;
            uncheckAll(node.children);
        }
    };
    uncheckAll(m_data);

    // Check matching
    for (const QString& key : keys)
    {
        AntTreeNode* n = findNode(key);
        if (n)
        {
            n->checked = true;
        }
    }
    refreshCheckStateUp();
    update();
    Q_EMIT nodeChecked(checkedKeys());
}

void AntTree::setNodeExpanded(const QString& key, bool expanded)
{
    AntTreeNode* node = findNode(key);
    if (!node || node->isLeaf || node->children.isEmpty() || node->expanded == expanded)
    {
        return;
    }

    node->expanded = expanded;
    clampScrollY();
    updateGeometry();
    update();
    Q_EMIT nodeExpanded(node->key, node->expanded);
}

void AntTree::setNodeChecked(const QString& key, bool checked)
{
    AntTreeNode* node = findNode(key);
    if (!node || !node->checkable || node->checked == checked)
    {
        return;
    }

    node->checked = checked;
    node->halfChecked = false;
    refreshCheckStateDown(*node, checked);
    refreshCheckStateUp();
    update();
    Q_EMIT nodeChecked(checkedKeys());
}

QSize AntTree::sizeHint() const
{
    return QSize(240, std::max(RowHeight * 5, totalVisibleHeight()));
}

QSize AntTree::minimumSizeHint() const
{
    return QSize(120, RowHeight);
}

void AntTree::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntTree::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        QWidget::mousePressEvent(event);
        return;
    }

    const int row = rowAtPoint(event->pos());
    const QVector<FlatNode> flat = flattenVisible();
    if (row < 0 || row >= flat.size())
    {
        QWidget::mousePressEvent(event);
        return;
    }

    const FlatNode& fn = flat.at(row);
    AntTreeNode* node = fn.node;
    if (!node || node->disabled)
    {
        QWidget::mousePressEvent(event);
        return;
    }

    const int indent = fn.depth * IndentWidth;
    const int x = event->pos().x();

    // Arrow zone: leftmost ArrowZoneWidth pixels after indent
    if (x >= indent && x < indent + ArrowZoneWidth && !node->isLeaf && !node->children.isEmpty())
    {
        node->expanded = !node->expanded;
        clampScrollY();
        update();
        Q_EMIT nodeExpanded(node->key, node->expanded);
        event->accept();
        return;
    }

    // Checkbox zone: after indent + arrow zone
    if (m_checkable && node->checkable)
    {
        const int checkboxLeft = indent + ArrowZoneWidth;
        if (x >= checkboxLeft && x < checkboxLeft + CheckboxZoneWidth)
        {
            const bool newChecked = !node->checked;
            node->checked = newChecked;
            node->halfChecked = false;
            refreshCheckStateDown(*node, newChecked);
            refreshCheckStateUp();
            update();
            Q_EMIT nodeChecked(checkedKeys());
            event->accept();
            return;
        }
    }

    // Selection zone: the rest of the row
    if (m_selectable && node->selectable)
    {
        if (m_multiple)
        {
            if (m_selectedKeys.contains(node->key))
            {
                m_selectedKeys.removeAll(node->key);
            }
            else
            {
                m_selectedKeys.append(node->key);
            }
        }
        else
        {
            m_selectedKeys = QStringList{node->key};
        }
        update();
        Q_EMIT nodeSelected(node->key);
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}

void AntTree::mouseMoveEvent(QMouseEvent* event)
{
    const int row = rowAtPoint(event->pos());
    if (m_hoveredRow != row)
    {
        m_hoveredRow = row;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void AntTree::leaveEvent(QEvent* event)
{
    if (m_hoveredRow != -1)
    {
        m_hoveredRow = -1;
        update();
    }
    QWidget::leaveEvent(event);
}

void AntTree::wheelEvent(QWheelEvent* event)
{
    const int delta = event->angleDelta().y();
    if (delta != 0)
    {
        m_scrollY -= (delta > 0) ? ScrollStep : -ScrollStep;
        clampScrollY();
        update();
        event->accept();
        return;
    }
    QWidget::wheelEvent(event);
}

QVector<FlatNode> AntTree::flattenVisible() const
{
    QVector<FlatNode> result;
    flattenRecursive(m_data, 0, result);
    return result;
}

int AntTree::rowAtPoint(const QPoint& pos) const
{
    if (pos.y() < 0)
    {
        return -1;
    }
    const int row = (pos.y() + m_scrollY) / RowHeight;
    const QVector<FlatNode> flat = flattenVisible();
    if (row < 0 || row >= flat.size())
    {
        return -1;
    }
    return row;
}

void AntTree::flattenRecursive(const QVector<AntTreeNode>& nodes, int depth, QVector<FlatNode>& result) const
{
    for (int i = 0; i < nodes.size(); ++i)
    {
        const AntTreeNode& node = nodes.at(i);
        FlatNode fn;
        fn.node = const_cast<AntTreeNode*>(&node);
        fn.depth = depth;
        fn.isLastChild = (i == nodes.size() - 1);
        result.append(fn);

        if (!node.isLeaf && node.expanded && !node.children.isEmpty())
        {
            flattenRecursive(node.children, depth + 1, result);
        }
    }
}

void AntTree::updateCheckState(AntTreeNode& node)
{
    if (node.children.isEmpty())
    {
        return;
    }

    int checkedCount = 0;
    int halfCheckedCount = 0;
    for (auto& child : node.children)
    {
        if (child.checked)
        {
            ++checkedCount;
        }
        else if (child.halfChecked)
        {
            ++halfCheckedCount;
        }
    }

    if (checkedCount == node.children.size())
    {
        node.checked = true;
        node.halfChecked = false;
    }
    else if (checkedCount > 0 || halfCheckedCount > 0)
    {
        node.checked = false;
        node.halfChecked = true;
    }
    else
    {
        node.checked = false;
        node.halfChecked = false;
    }
}

void AntTree::collectCheckedKeys(const QVector<AntTreeNode>& nodes, QStringList& keys) const
{
    for (const auto& node : nodes)
    {
        if (node.checked && !node.halfChecked)
        {
            keys.append(node.key);
        }
        collectCheckedKeys(node.children, keys);
    }
}

void AntTree::collectExpandedKeys(const QVector<AntTreeNode>& nodes, QStringList& keys) const
{
    for (const auto& node : nodes)
    {
        if (node.expanded)
        {
            keys.append(node.key);
        }
        collectExpandedKeys(node.children, keys);
    }
}

AntTreeNode* AntTree::findNodeRecursive(QVector<AntTreeNode>& nodes, const QString& key)
{
    for (auto& node : nodes)
    {
        if (node.key == key)
        {
            return &node;
        }
        AntTreeNode* found = findNodeRecursive(node.children, key);
        if (found)
        {
            return found;
        }
    }
    return nullptr;
}

const AntTreeNode* AntTree::findNodeRecursive(const QVector<AntTreeNode>& nodes, const QString& key) const
{
    for (const auto& node : nodes)
    {
        if (node.key == key)
        {
            return &node;
        }
        const AntTreeNode* found = findNodeRecursive(node.children, key);
        if (found)
        {
            return found;
        }
    }
    return nullptr;
}

int AntTree::countNodesRecursive(const QVector<AntTreeNode>& nodes) const
{
    int count = nodes.size();
    for (const auto& node : nodes)
    {
        count += countNodesRecursive(node.children);
    }
    return count;
}

bool AntTree::removeNodeRecursive(QVector<AntTreeNode>& nodes, const QString& key)
{
    for (int i = 0; i < nodes.size(); ++i)
    {
        if (nodes.at(i).key == key)
        {
            nodes.removeAt(i);
            return true;
        }
        if (removeNodeRecursive(nodes[i].children, key))
        {
            return true;
        }
    }
    return false;
}

void AntTree::setNodeExpanded(QVector<AntTreeNode>& nodes, const QString& key, bool expanded)
{
    for (auto& node : nodes)
    {
        if (node.key == key)
        {
            node.expanded = expanded;
            return;
        }
        setNodeExpanded(node.children, key, expanded);
    }
}

void AntTree::setNodeChecked(QVector<AntTreeNode>& nodes, const QString& key, bool checked)
{
    for (auto& node : nodes)
    {
        if (node.key == key)
        {
            node.checked = checked;
            node.halfChecked = false;
            refreshCheckStateDown(node, checked);
            return;
        }
        setNodeChecked(node.children, key, checked);
    }
}

void AntTree::refreshCheckStateDown(AntTreeNode& node, bool checked)
{
    for (auto& child : node.children)
    {
        if (child.checkable)
        {
            child.checked = checked;
            child.halfChecked = false;
            refreshCheckStateDown(child, checked);
        }
    }
}

void AntTree::refreshCheckStateUp()
{
    // Iteratively update parent check states bottom-up
    bool changed = true;
    while (changed)
    {
        changed = refreshCheckStateUpRecursive(m_data);
    }
}

bool AntTree::refreshCheckStateUpRecursive(QVector<AntTreeNode>& nodes)
{
    bool changed = false;
    for (auto& node : nodes)
    {
        if (!node.children.isEmpty())
        {
            // Recurse into children first (bottom-up)
            if (refreshCheckStateUpRecursive(node.children))
            {
                changed = true;
            }
            const bool oldChecked = node.checked;
            const bool oldHalf = node.halfChecked;
            updateCheckState(node);
            if (node.checked != oldChecked || node.halfChecked != oldHalf)
            {
                changed = true;
            }
        }
    }
    return changed;
}

int AntTree::totalVisibleHeight() const
{
    return flattenVisible().size() * RowHeight;
}

void AntTree::clampScrollY()
{
    const int maxScroll = std::max(0, totalVisibleHeight() - height());
    m_scrollY = std::clamp(m_scrollY, 0, maxScroll);
}
