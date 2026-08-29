#include "AntDockPerspectiveRestorer.h"

#include <QSplitter>
#include <QWidget>

namespace AntDockInternal
{
DockPerspectiveRestorePlan DockPerspectiveRestorer::plan(
    const QByteArray& state,
    const QList<AntDockWidget*>& availableDocks)
{
    DockPerspectiveRestorePlan plan;
    if (state.isEmpty())
    {
        plan.errorReason = QStringLiteral("perspective-not-found");
        return plan;
    }
    if (isLegacyDockPerspective(state))
    {
        plan.errorReason = QStringLiteral("unsupported-legacy-format");
        return plan;
    }
    if (!deserializeDockPerspective(state, &plan.rootNode, &plan.floatingSnapshots))
    {
        plan.errorReason = QStringLiteral("invalid-current-format");
        return plan;
    }

    for (AntDockWidget* dock : availableDocks)
    {
        const QString id = dockPersistentId(dock);
        if (id.isEmpty())
        {
            continue;
        }
        if (plan.docksById.contains(id))
        {
            plan.errorReason = QStringLiteral("duplicate-runtime-dock-id");
            return plan;
        }
        if (dock)
        {
            plan.docksById.insert(id, dock);
        }
    }

    collectDockIds(plan.rootNode, &plan.embeddedDockIds);
    plan.stateDockIds = plan.embeddedDockIds;
    for (const FloatingDockSnapshot& snapshot : plan.floatingSnapshots)
    {
        if (!snapshot.dockId.isEmpty())
        {
            plan.stateDockIds.insert(snapshot.dockId);
        }
    }

    bool hasMatchedDock = plan.stateDockIds.isEmpty();
    for (const QString& id : plan.stateDockIds)
    {
        if (plan.docksById.contains(id))
        {
            hasMatchedDock = true;
            break;
        }
    }
    if (!hasMatchedDock)
    {
        plan.errorReason = QStringLiteral("no-matching-dock");
    }
    return plan;
}

DockLayoutRestoreResult DockPerspectiveRestorer::restoreLayoutTree(
    const DockLayoutNode& rootNode,
    const QHash<QString, AntDockWidget*>& docksById,
    const DockLayoutRestoreCallbacks& callbacks)
{
    DockLayoutRestoreResult result;
    if (!callbacks.createArea || !callbacks.addDockToArea || !callbacks.areaDockCount ||
        !callbacks.setAreaCurrentIndex)
    {
        return result;
    }

    const std::function<QWidget*(const DockLayoutNode&)> buildNode =
        [&](const DockLayoutNode& node) -> QWidget* {
        switch (node.type)
        {
        case DockLayoutNodeType::Area:
        {
            QWidget* area = callbacks.createArea();
            if (!area)
            {
                return nullptr;
            }

            ++result.restoredAreaCount;
            for (const QString& id : node.dockIds)
            {
                AntDockWidget* dock = docksById.value(id, nullptr);
                if (!dock || result.placedDockIds.contains(id))
                {
                    continue;
                }

                callbacks.addDockToArea(area, dock);
                result.placedDockIds.insert(id);
            }

            const int dockCount = callbacks.areaDockCount(area);
            if (dockCount == 0)
            {
                area->deleteLater();
                return nullptr;
            }

            callbacks.setAreaCurrentIndex(area, qBound(0, node.currentIndex, dockCount - 1));
            return area;
        }
        case DockLayoutNodeType::Splitter:
        {
            auto* splitter = new QSplitter(node.orientation, callbacks.splitterParent);
            if (callbacks.configureSplitter)
            {
                callbacks.configureSplitter(splitter);
            }

            for (const DockLayoutNode& child : node.children)
            {
                if (QWidget* childWidget = buildNode(child))
                {
                    splitter->addWidget(childWidget);
                }
            }

            if (splitter->count() == 0)
            {
                splitter->deleteLater();
                return nullptr;
            }
            if (splitter->count() == 1)
            {
                QWidget* onlyChild = splitter->widget(0);
                onlyChild->setParent(nullptr);
                splitter->deleteLater();
                return onlyChild;
            }

            if (node.sizes.size() == splitter->count())
            {
                splitter->setSizes(node.sizes);
            }
            return splitter;
        }
        case DockLayoutNodeType::Empty:
        default:
            return nullptr;
        }
    };

    result.rootWidget = buildNode(rootNode);
    return result;
}

QList<FloatingDockRestoreInstruction> DockPerspectiveRestorer::planFloatingDocks(
    const DockPerspectiveRestorePlan& plan,
    const QSet<QString>& placedDockIds,
    const QList<AntDockWidget*>& availableDocks,
    const QHash<AntDockWidget*, QRect>& fallbackGeometry,
    const QHash<AntDockWidget*, bool>& fallbackVisibility)
{
    QList<FloatingDockRestoreInstruction> instructions;
    QSet<QString> assignedDockIds = placedDockIds;

    for (const FloatingDockSnapshot& snapshot : plan.floatingSnapshots)
    {
        AntDockWidget* dock = plan.docksById.value(snapshot.dockId, nullptr);
        if (!dock || assignedDockIds.contains(snapshot.dockId))
        {
            continue;
        }

        FloatingDockRestoreInstruction instruction;
        instruction.dock = dock;
        instruction.dockId = snapshot.dockId;
        instruction.geometry = snapshot.geometry.isEmpty()
            ? fallbackGeometry.value(dock)
            : snapshot.geometry;
        instruction.visible = snapshot.visible;
        instructions.append(instruction);
        assignedDockIds.insert(snapshot.dockId);
    }

    for (AntDockWidget* dock : availableDocks)
    {
        if (!dock)
        {
            continue;
        }

        const QString id = dockPersistentId(dock);
        if (id.isEmpty() || assignedDockIds.contains(id))
        {
            continue;
        }

        FloatingDockRestoreInstruction instruction;
        instruction.dock = dock;
        instruction.dockId = id;
        instruction.geometry = fallbackGeometry.value(dock);
        instruction.visible = fallbackVisibility.value(dock, true);
        instructions.append(instruction);
    }
    return instructions;
}
} // namespace AntDockInternal
