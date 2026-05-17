#include "AntDockLayoutSerializer.h"

#include <QDataStream>
#include <QIODevice>
#include <QSplitter>
#include <QTabWidget>

#include "widgets/AntDockWidget.h"

namespace
{
constexpr const char* kPerspectiveMagic = "AntDockManagerPerspective";
constexpr quint16 kPerspectiveVersion = 1;

void writeLayoutNode(QDataStream& stream, const AntDockInternal::DockLayoutNode& node)
{
    stream << static_cast<quint8>(node.type);
    stream << static_cast<qint32>(node.orientation);
    stream << static_cast<qint32>(node.currentIndex);

    stream << static_cast<qint32>(node.sizes.size());
    for (int size : node.sizes)
    {
        stream << static_cast<qint32>(size);
    }

    stream << static_cast<qint32>(node.dockIds.size());
    for (const QString& id : node.dockIds)
    {
        stream << id;
    }

    stream << static_cast<qint32>(node.children.size());
    for (const AntDockInternal::DockLayoutNode& child : node.children)
    {
        writeLayoutNode(stream, child);
    }
}

bool readLayoutNode(QDataStream& stream, AntDockInternal::DockLayoutNode* node)
{
    if (!node) return false;

    quint8 rawType = 0;
    qint32 rawOrientation = 0;
    qint32 rawCurrentIndex = 0;
    stream >> rawType >> rawOrientation >> rawCurrentIndex;
    if (stream.status() != QDataStream::Ok) return false;

    if (rawType > static_cast<quint8>(AntDockInternal::DockLayoutNodeType::Splitter))
    {
        return false;
    }
    if (rawOrientation != static_cast<qint32>(Qt::Horizontal) &&
        rawOrientation != static_cast<qint32>(Qt::Vertical))
    {
        return false;
    }

    node->type = static_cast<AntDockInternal::DockLayoutNodeType>(rawType);
    node->orientation = static_cast<Qt::Orientation>(rawOrientation);
    node->currentIndex = qMax<qint32>(0, rawCurrentIndex);
    node->sizes.clear();
    node->dockIds.clear();
    node->children.clear();

    qint32 sizeCount = 0;
    stream >> sizeCount;
    if (stream.status() != QDataStream::Ok || sizeCount < 0 || sizeCount > 256) return false;
    for (qint32 i = 0; i < sizeCount; ++i)
    {
        qint32 size = 0;
        stream >> size;
        node->sizes.append(qMax<qint32>(0, size));
    }

    qint32 dockCount = 0;
    stream >> dockCount;
    if (stream.status() != QDataStream::Ok || dockCount < 0 || dockCount > 1024) return false;
    for (qint32 i = 0; i < dockCount; ++i)
    {
        QString id;
        stream >> id;
        if (!id.isEmpty())
        {
            node->dockIds.append(id);
        }
    }

    qint32 childCount = 0;
    stream >> childCount;
    if (stream.status() != QDataStream::Ok || childCount < 0 || childCount > 256) return false;
    for (qint32 i = 0; i < childCount; ++i)
    {
        AntDockInternal::DockLayoutNode child;
        if (!readLayoutNode(stream, &child))
        {
            return false;
        }
        if (child.type != AntDockInternal::DockLayoutNodeType::Empty)
        {
            node->children.append(child);
        }
    }

    if (node->type == AntDockInternal::DockLayoutNodeType::Area && node->dockIds.isEmpty())
    {
        node->type = AntDockInternal::DockLayoutNodeType::Empty;
    }
    if (node->type == AntDockInternal::DockLayoutNodeType::Splitter && node->children.isEmpty())
    {
        node->type = AntDockInternal::DockLayoutNodeType::Empty;
    }

    return stream.status() == QDataStream::Ok;
}

void writeFloatingSnapshots(QDataStream& stream, const QList<AntDockInternal::FloatingDockSnapshot>& snapshots)
{
    stream << static_cast<qint32>(snapshots.size());
    for (const AntDockInternal::FloatingDockSnapshot& snapshot : snapshots)
    {
        stream << snapshot.dockId << snapshot.geometry << snapshot.visible;
    }
}

bool readFloatingSnapshots(QDataStream& stream, QList<AntDockInternal::FloatingDockSnapshot>* snapshots)
{
    if (!snapshots) return false;

    snapshots->clear();
    qint32 count = 0;
    stream >> count;
    if (stream.status() != QDataStream::Ok || count < 0 || count > 1024) return false;

    for (qint32 i = 0; i < count; ++i)
    {
        AntDockInternal::FloatingDockSnapshot snapshot;
        stream >> snapshot.dockId >> snapshot.geometry >> snapshot.visible;
        if (stream.status() != QDataStream::Ok) return false;
        if (!snapshot.dockId.isEmpty())
        {
            snapshots->append(snapshot);
        }
    }
    return true;
}
} // namespace

namespace AntDockInternal
{
QString dockPersistentId(AntDockWidget* dockWidget)
{
    return dockWidget ? dockWidget->objectName() : QString();
}

DockLayoutNode captureDockLayoutNode(QWidget* widget)
{
    DockLayoutNode node;
    if (!widget) return node;

    if (auto* splitter = qobject_cast<QSplitter*>(widget))
    {
        node.type = DockLayoutNodeType::Splitter;
        node.orientation = splitter->orientation();
        node.sizes = splitter->sizes();
        for (int i = 0; i < splitter->count(); ++i)
        {
            DockLayoutNode child = captureDockLayoutNode(splitter->widget(i));
            if (child.type != DockLayoutNodeType::Empty)
            {
                node.children.append(child);
            }
        }
        if (node.children.isEmpty())
        {
            node.type = DockLayoutNodeType::Empty;
            node.sizes.clear();
        }
        return node;
    }

    if (auto* tabs = qobject_cast<QTabWidget*>(widget))
    {
        if (tabs->objectName() != QStringLiteral("AntDockArea"))
        {
            return node;
        }

        node.type = DockLayoutNodeType::Area;
        node.currentIndex = qMax(0, tabs->currentIndex());
        for (int i = 0; i < tabs->count(); ++i)
        {
            if (auto* dock = qobject_cast<AntDockWidget*>(tabs->widget(i)))
            {
                const QString id = dockPersistentId(dock);
                if (!id.isEmpty())
                {
                    node.dockIds.append(id);
                }
            }
        }
        if (node.dockIds.isEmpty())
        {
            node.type = DockLayoutNodeType::Empty;
            node.currentIndex = 0;
        }
    }

    return node;
}

void collectDockIds(const DockLayoutNode& node, QSet<QString>* ids)
{
    if (!ids) return;

    for (const QString& id : node.dockIds)
    {
        if (!id.isEmpty())
        {
            ids->insert(id);
        }
    }
    for (const DockLayoutNode& child : node.children)
    {
        collectDockIds(child, ids);
    }
}

QByteArray serializeDockPerspective(const DockLayoutNode& rootNode,
                                    const QList<FloatingDockSnapshot>& floatingSnapshots)
{
    QByteArray state;
    QDataStream stream(&state, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream << QString::fromLatin1(kPerspectiveMagic) << kPerspectiveVersion;
    writeLayoutNode(stream, rootNode);
    writeFloatingSnapshots(stream, floatingSnapshots);
    return state;
}

bool deserializeDockPerspective(const QByteArray& state,
                                DockLayoutNode* rootNode,
                                QList<FloatingDockSnapshot>* floatingSnapshots)
{
    if (!rootNode || !floatingSnapshots || state.isEmpty()) return false;

    QDataStream stream(state);
    stream.setVersion(QDataStream::Qt_5_0);

    QString magic;
    quint16 version = 0;
    stream >> magic >> version;
    if (stream.status() != QDataStream::Ok ||
        magic != QString::fromLatin1(kPerspectiveMagic) ||
        version != kPerspectiveVersion)
    {
        return false;
    }

    return readLayoutNode(stream, rootNode) && readFloatingSnapshots(stream, floatingSnapshots);
}

bool isLegacyDockPerspective(const QByteArray& state)
{
    return state.startsWith("AntDockManagerLayout\n");
}
} // namespace AntDockInternal
