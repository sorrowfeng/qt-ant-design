#pragma once

#include <QIcon>
#include <QStringList>
#include <QVector>
#include <QWidget>

class QEvent;
class QMouseEvent;
class QPaintEvent;
class QWheelEvent;

struct AntTreeNode
{
    QString key;
    QString title;
    QIcon icon;
    QVector<AntTreeNode> children;
    bool isLeaf = false;
    bool disabled = false;
    bool checkable = true;
    bool selectable = true;
    bool expanded = false;
    bool checked = false;
    bool halfChecked = false;
};

struct FlatNode
{
    AntTreeNode* node = nullptr;
    int depth = 0;
    bool isLastChild = false;
};

class AntTree : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool selectable READ isSelectable WRITE setSelectable NOTIFY selectableChanged)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable NOTIFY checkableChanged)
    Q_PROPERTY(bool showLine READ isShowLine WRITE setShowLine NOTIFY showLineChanged)
    Q_PROPERTY(bool showIcon READ isShowIcon WRITE setShowIcon NOTIFY showIconChanged)
    Q_PROPERTY(bool multiple READ isMultiple WRITE setMultiple NOTIFY multipleChanged)

public:
    explicit AntTree(QWidget* parent = nullptr);

    bool isSelectable() const;
    void setSelectable(bool selectable);

    bool isCheckable() const;
    void setCheckable(bool checkable);

    bool isShowLine() const;
    void setShowLine(bool showLine);

    bool isShowIcon() const;
    void setShowIcon(bool showIcon);

    bool isMultiple() const;
    void setMultiple(bool multiple);

    void setTreeData(const QVector<AntTreeNode>& data);
    void addNode(const QString& parentKey, const AntTreeNode& node);
    void removeNode(const QString& key);
    AntTreeNode* findNode(const QString& key);

    QStringList expandedKeys() const;
    QStringList selectedKeys() const;
    QStringList checkedKeys() const;

    void setExpandedKeys(const QStringList& keys);
    void setSelectedKeys(const QStringList& keys);
    void setCheckedKeys(const QStringList& keys);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void selectableChanged(bool selectable);
    void checkableChanged(bool checkable);
    void showLineChanged(bool showLine);
    void showIconChanged(bool showIcon);
    void multipleChanged(bool multiple);
    void nodeExpanded(const QString& key, bool expanded);
    void nodeSelected(const QString& key);
    void nodeChecked(const QStringList& checkedKeys);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QVector<FlatNode> flattenVisible() const;
    int rowAtPoint(const QPoint& pos) const;
    void flattenRecursive(const QVector<AntTreeNode>& nodes, int depth, QVector<FlatNode>& result) const;
    void updateCheckState(AntTreeNode& node);
    void collectCheckedKeys(const QVector<AntTreeNode>& nodes, QStringList& keys) const;
    void collectExpandedKeys(const QVector<AntTreeNode>& nodes, QStringList& keys) const;
    AntTreeNode* findNodeRecursive(QVector<AntTreeNode>& nodes, const QString& key);
    bool removeNodeRecursive(QVector<AntTreeNode>& nodes, const QString& key);
    void setNodeExpanded(QVector<AntTreeNode>& nodes, const QString& key, bool expanded);
    void setNodeChecked(QVector<AntTreeNode>& nodes, const QString& key, bool checked);
    void refreshCheckStateDown(AntTreeNode& node, bool checked);
    void refreshCheckStateUp();
    bool refreshCheckStateUpRecursive(QVector<AntTreeNode>& nodes);
    int totalVisibleHeight() const;
    void clampScrollY();

    QVector<AntTreeNode> m_data;
    bool m_selectable = true;
    bool m_checkable = true;
    bool m_showLine = false;
    bool m_showIcon = true;
    bool m_multiple = false;
    int m_scrollY = 0;
    int m_hoveredRow = -1;
    QStringList m_selectedKeys;

    friend class AntTreeStyle;
};
