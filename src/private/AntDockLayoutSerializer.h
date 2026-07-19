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
inline constexpr int DockPerspectiveMaxStateBytes = 1024 * 1024;
inline constexpr int DockPerspectiveMaxDepth = 64;
inline constexpr int DockPerspectiveMaxNodes = 4096;
inline constexpr int DockPerspectiveMaxDockIds = 4096;
inline constexpr int DockPerspectiveMaxIdentifierCharacters = 1024;
inline constexpr int DockPerspectiveMaxFloatingSnapshots = 1024;

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
