#include "AntDockLayoutSerializer.h"

#include <QDataStream>
#include <QIODevice>
#include <QSplitter>
#include <QTabWidget>

#include <utility>

#include "widgets/AntDockWidget.h"

namespace
{
constexpr const char* kPerspectiveMagic = "AntDockManagerPerspective";
constexpr quint16 kPerspectiveVersion = 1;

struct DockPerspectiveReadBudget
{
    int nodes = 0;
    int dockIds = 0;
    QSet<QString> seenDockIds;
};

bool reserveBudget(int current, int requested, int maximum)
{
    return requested >= 0 && current <= maximum && requested <= maximum - current;
}

bool readBoundedString(QDataStream& stream, QString* value, int maximumCharacters)
{
    if (!value || maximumCharacters < 0 || !stream.device()) return false;

    QIODevice* device = stream.device();
    const qint64 stringStart = device->pos();
    quint32 byteLength = 0;
    stream >> byteLength;
    if (stream.status() != QDataStream::Ok) return false;

    constexpr quint32 kNullStringLength = 0xffffffffu;
    constexpr quint32 kExtendedStringLength = 0xfffffffeu;
    if (byteLength == kNullStringLength)
    {
        *value = QString();
        return true;
    }
    if (byteLength == kExtendedStringLength || (byteLength % sizeof(quint16)) != 0)
    {
        return false;
    }

    const quint64 maximumBytes = static_cast<quint64>(maximumCharacters) * sizeof(quint16);
    if (static_cast<quint64>(byteLength) > maximumBytes ||
        static_cast<quint64>(byteLength) > static_cast<quint64>(device->bytesAvailable()))
    {
        return false;
    }
    if (byteLength == 0)
    {
        *value = QStringLiteral("");
        return true;
    }
    if (!device->seek(stringStart)) return false;

    stream >> *value;
    return stream.status() == QDataStream::Ok && value->size() <= maximumCharacters;
}

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

bool readLayoutNode(QDataStream& stream,
                    AntDockInternal::DockLayoutNode* node,
                    int depth,
                    DockPerspectiveReadBudget* budget)
{
    if (!node || !budget || depth > AntDockInternal::DockPerspectiveMaxDepth ||
        budget->nodes >= AntDockInternal::DockPerspectiveMaxNodes)
    {
        return false;
    }
    ++budget->nodes;

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
    node->currentIndex = rawCurrentIndex;
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
        if (stream.status() != QDataStream::Ok || size < 0) return false;
        node->sizes.append(size);
    }

    qint32 dockCount = 0;
    stream >> dockCount;
    if (stream.status() != QDataStream::Ok || dockCount < 0 || dockCount > 1024 ||
        !reserveBudget(budget->dockIds, dockCount, AntDockInternal::DockPerspectiveMaxDockIds))
    {
        return false;
    }
    budget->dockIds += dockCount;
    for (qint32 i = 0; i < dockCount; ++i)
    {
        QString id;
        if (!readBoundedString(stream, &id, AntDockInternal::DockPerspectiveMaxIdentifierCharacters))
        {
            return false;
        }
        if (id.isEmpty() || budget->seenDockIds.contains(id))
        {
            return false;
        }
        budget->seenDockIds.insert(id);
        node->dockIds.append(id);
    }

    qint32 childCount = 0;
    stream >> childCount;
    if (stream.status() != QDataStream::Ok || childCount < 0 || childCount > 256) return false;
    for (qint32 i = 0; i < childCount; ++i)
    {
        AntDockInternal::DockLayoutNode child;
        if (!readLayoutNode(stream, &child, depth + 1, budget))
        {
            return false;
        }
        node->children.append(child);
    }

    using AntDockInternal::DockLayoutNodeType;
    switch (node->type)
    {
    case DockLayoutNodeType::Empty:
        if (node->currentIndex != 0 || !node->sizes.isEmpty() ||
            !node->dockIds.isEmpty() || !node->children.isEmpty())
        {
            return false;
        }
        break;
    case DockLayoutNodeType::Area:
        if (node->dockIds.isEmpty() || !node->sizes.isEmpty() || !node->children.isEmpty() ||
            node->currentIndex < 0 || node->currentIndex >= node->dockIds.size())
        {
            return false;
        }
        break;
    case DockLayoutNodeType::Splitter:
        if (node->currentIndex != 0 || !node->dockIds.isEmpty() || node->children.isEmpty() ||
            (!node->sizes.isEmpty() && node->sizes.size() != node->children.size()))
        {
            return false;
        }
        for (const AntDockInternal::DockLayoutNode& child : std::as_const(node->children))
        {
            if (child.type == DockLayoutNodeType::Empty)
            {
                return false;
            }
        }
        break;
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

bool readFloatingSnapshots(QDataStream& stream,
                           QList<AntDockInternal::FloatingDockSnapshot>* snapshots,
                           DockPerspectiveReadBudget* budget)
{
    if (!snapshots || !budget) return false;

    snapshots->clear();
    qint32 count = 0;
    stream >> count;
    if (stream.status() != QDataStream::Ok || count < 0 ||
        count > AntDockInternal::DockPerspectiveMaxFloatingSnapshots ||
        !reserveBudget(budget->dockIds, count, AntDockInternal::DockPerspectiveMaxDockIds))
    {
        return false;
    }
    budget->dockIds += count;

    for (qint32 i = 0; i < count; ++i)
    {
        AntDockInternal::FloatingDockSnapshot snapshot;
        if (!readBoundedString(stream,
                               &snapshot.dockId,
                               AntDockInternal::DockPerspectiveMaxIdentifierCharacters))
        {
            return false;
        }
        stream >> snapshot.geometry >> snapshot.visible;
        if (stream.status() != QDataStream::Ok) return false;
        if (snapshot.dockId.isEmpty() || budget->seenDockIds.contains(snapshot.dockId))
        {
            return false;
        }
        budget->seenDockIds.insert(snapshot.dockId);
        snapshots->append(snapshot);
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
        const QList<int> splitterSizes = splitter->sizes();
        for (int i = 0; i < splitter->count(); ++i)
        {
            DockLayoutNode child = captureDockLayoutNode(splitter->widget(i));
            if (child.type != DockLayoutNodeType::Empty)
            {
                node.children.append(child);
                node.sizes.append(i < splitterSizes.size() ? qMax(0, splitterSizes.at(i)) : 0);
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
    if (!rootNode || !floatingSnapshots || state.isEmpty() ||
        state.size() > DockPerspectiveMaxStateBytes)
    {
        return false;
    }

    QDataStream stream(state);
    stream.setVersion(QDataStream::Qt_5_0);

    QString magic;
    quint16 version = 0;
    if (!readBoundedString(stream, &magic, 64)) return false;
    stream >> version;
    if (stream.status() != QDataStream::Ok ||
        magic != QString::fromLatin1(kPerspectiveMagic) ||
        version != kPerspectiveVersion)
    {
        return false;
    }

    DockPerspectiveReadBudget budget;
    return readLayoutNode(stream, rootNode, 0, &budget) &&
        readFloatingSnapshots(stream, floatingSnapshots, &budget) &&
        stream.status() == QDataStream::Ok && stream.atEnd();
}

bool isLegacyDockPerspective(const QByteArray& state)
{
    return state.startsWith("AntDockManagerLayout\n");
}
} // namespace AntDockInternal
