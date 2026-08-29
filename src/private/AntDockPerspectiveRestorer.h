#pragma once

#include "AntDockLayoutSerializer.h"

#include <QHash>
#include <QList>
#include <QRect>
#include <QSet>
#include <QString>

#include <functional>

class AntDockWidget;
class QSplitter;
class QWidget;

namespace AntDockInternal
{
struct DockPerspectiveRestorePlan
{
    QString errorReason;
    DockLayoutNode rootNode;
    QList<FloatingDockSnapshot> floatingSnapshots;
    QHash<QString, AntDockWidget*> docksById;
    QSet<QString> embeddedDockIds;
    QSet<QString> stateDockIds;

    bool isValid() const { return errorReason.isEmpty(); }
};

struct DockLayoutRestoreCallbacks
{
    QWidget* splitterParent = nullptr;
    std::function<QWidget*()> createArea;
    std::function<void(QWidget*, AntDockWidget*)> addDockToArea;
    std::function<int(QWidget*)> areaDockCount;
    std::function<void(QWidget*, int)> setAreaCurrentIndex;
    std::function<void(QSplitter*)> configureSplitter;
};

struct DockLayoutRestoreResult
{
    QWidget* rootWidget = nullptr;
    QSet<QString> placedDockIds;
    int restoredAreaCount = 0;
};

struct FloatingDockRestoreInstruction
{
    AntDockWidget* dock = nullptr;
    QString dockId;
    QRect geometry;
    bool visible = true;
};

class DockPerspectiveRestorer
{
public:
    DockPerspectiveRestorer() = delete;

    static DockPerspectiveRestorePlan plan(
        const QByteArray& state,
        const QList<AntDockWidget*>& availableDocks);

    static DockLayoutRestoreResult restoreLayoutTree(
        const DockLayoutNode& rootNode,
        const QHash<QString, AntDockWidget*>& docksById,
        const DockLayoutRestoreCallbacks& callbacks);

    static QList<FloatingDockRestoreInstruction> planFloatingDocks(
        const DockPerspectiveRestorePlan& plan,
        const QSet<QString>& placedDockIds,
        const QList<AntDockWidget*>& availableDocks,
        const QHash<AntDockWidget*, QRect>& fallbackGeometry,
        const QHash<AntDockWidget*, bool>& fallbackVisibility);
};
} // namespace AntDockInternal
