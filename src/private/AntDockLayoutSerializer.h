#pragma once

#include <QByteArray>
#include <QList>
#include <QRect>
#include <QSet>
#include <QStringList>
#include <Qt>

class AntDockWidget;
class QWidget;

namespace AntDockInternal
{
enum class DockLayoutNodeType : quint8
{
    Empty = 0,
    Area = 1,
    Splitter = 2,
};

struct DockLayoutNode
{
    DockLayoutNodeType type = DockLayoutNodeType::Empty;
    Qt::Orientation orientation = Qt::Horizontal;
    QList<int> sizes;
    QStringList dockIds;
    QList<DockLayoutNode> children;
    int currentIndex = 0;
};

struct FloatingDockSnapshot
{
    QString dockId;
    QRect geometry;
    bool visible = true;
};

QString dockPersistentId(AntDockWidget* dockWidget);
DockLayoutNode captureDockLayoutNode(QWidget* widget);
void collectDockIds(const DockLayoutNode& node, QSet<QString>* ids);
QByteArray serializeDockPerspective(const DockLayoutNode& rootNode,
                                    const QList<FloatingDockSnapshot>& floatingSnapshots);
bool deserializeDockPerspective(const QByteArray& state,
                                DockLayoutNode* rootNode,
                                QList<FloatingDockSnapshot>* floatingSnapshots);
bool isLegacyDockPerspective(const QByteArray& state);
} // namespace AntDockInternal
