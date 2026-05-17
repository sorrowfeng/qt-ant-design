#pragma once

#include "core/QtAntDesignExport.h"

#include <QByteArray>
#include <QList>
#include <QHash>
#include <QMainWindow>
#include <QPoint>
#include <QRect>
#include <QSet>
#include <QStringList>

class AntDockWidget;
class QEvent;
class QGraphicsOpacityEffect;
class QResizeEvent;
class QWidget;

class QT_ANT_DESIGN_EXPORT AntDockManager : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(bool placeholderVisible READ isPlaceholderVisible WRITE setPlaceholderVisible NOTIFY placeholderVisibleChanged)
    Q_PROPERTY(bool dropGuideVisible READ isDropGuideVisible WRITE setDropGuideVisible NOTIFY dropGuideVisibleChanged)

public:
    enum class DockPlacement
    {
        None,
        Left,
        Right,
        Top,
        Bottom,
        Center,
    };
    Q_ENUM(DockPlacement)

    explicit AntDockManager(QWidget* parent = nullptr);
    ~AntDockManager() override;

    using QMainWindow::addDockWidget;
    using QMainWindow::removeDockWidget;
    using QMainWindow::splitDockWidget;
    using QMainWindow::tabifyDockWidget;

    void addDockWidget(Qt::DockWidgetArea area, AntDockWidget* dockWidget);
    void addDockWidget(Qt::DockWidgetArea area, AntDockWidget* dockWidget, Qt::Orientation orientation);
    void splitDockWidget(AntDockWidget* after, AntDockWidget* dockWidget, Qt::Orientation orientation);
    void tabifyDockWidget(AntDockWidget* first, AntDockWidget* second);
    Qt::DockWidgetArea dockWidgetArea(AntDockWidget* dockWidget) const;
    QList<AntDockWidget*> tabifiedDockWidgets(AntDockWidget* dockWidget) const;
    void addDockWidget(AntDockWidget* dockWidget, AntDockWidget* relativeTo, DockPlacement placement);
    void removeDockWidget(AntDockWidget* dockWidget);

    QList<AntDockWidget*> dockWidgets() const;

    QWidget* centralContent() const;
    void setCentralContent(QWidget* widget);

    bool isPlaceholderVisible() const;
    void setPlaceholderVisible(bool visible);

    bool isDropGuideVisible() const;
    void setDropGuideVisible(bool visible);
    DockPlacement activeDropGuide() const;
    bool isDropPreviewVisible() const;
    QRect dropPreviewRect() const;

    bool savePerspective(const QString& name);
    bool restorePerspective(const QString& name);
    bool removePerspective(const QString& name);
    void clearPerspectives();
    QStringList perspectiveNames() const;
    QByteArray perspectiveState(const QString& name) const;
    bool setPerspectiveState(const QString& name, const QByteArray& state);

Q_SIGNALS:
    void dockWidgetAdded(AntDockWidget* dockWidget);
    void dockWidgetRemoved(AntDockWidget* dockWidget);
    void placeholderVisibleChanged(bool visible);
    void dropGuideVisibleChanged(bool visible);
    void dropPreviewVisibleChanged(bool visible);
    void activeDropGuideChanged(AntDockManager::DockPlacement placement);
    void perspectiveSaved(const QString& name);
    void perspectiveRestored(const QString& name);
    void perspectiveRemoved(const QString& name);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    class Workspace;
    class DockArea;
    class DockGuideOverlay;
    class DockDragPreviewWindow;
    class DockDropPreviewWindow;

    struct DropTarget;

    bool prepareDockWidget(AntDockWidget* dockWidget);
    DockArea* createDockArea();
    DockArea* areaForDock(AntDockWidget* dockWidget) const;
    DockArea* firstDockArea() const;
    void setRootDockWidget(QWidget* widget);
    void insertDockWidget(AntDockWidget* dockWidget, DockArea* targetArea, DockPlacement placement, bool containerDrop = false);
    void splitAreaWithWidget(QWidget* targetWidget, QWidget* newWidget, DockPlacement placement);
    void removeDockFromArea(AntDockWidget* dockWidget, bool detach);
    void pruneEmptyArea(DockArea* area);
    void collapseSplitter(QWidget* splitterWidget);
    AntDockWidget* dockForWatchedObject(QObject* watched) const;
    void installDockEventFilters(AntDockWidget* dockWidget);
    void removeDockEventFilters(AntDockWidget* dockWidget);
    void handleDockTitleMouseEvent(AntDockWidget* dockWidget, QEvent* event);
    bool handleGlobalDockDragEvent(QObject* watched, QEvent* event);
    void startDockDragTracking(AntDockWidget* dockWidget, const QPoint& globalPos);
    void finishDockDragTracking(const QPoint& globalPos);
    void stopDockDragTracking();
    void applyDropTarget(AntDockWidget* dockWidget, AntDockWidget* targetDock, DockPlacement placement, bool containerDrop);
    void setDraggedDockTranslucent(bool translucent);
    void showDropGuideAt(const QPoint& globalPos);
    void hideDropGuide();
    DropTarget dropTargetAt(const QPoint& globalPos) const;
    DropTarget rememberedDropTarget() const;
    void rememberDropTarget(const DropTarget& target);
    void clearRememberedDropTarget();
    AntDockWidget* dockWidgetAt(const QPoint& globalPos) const;
    DockPlacement placementForTarget(const QPoint& globalPos, const QRect& targetGlobalRect) const;
    QRect previewRectForTarget(const QRect& targetGlobalRect, DockPlacement placement) const;
    QString dropTargetLabel(AntDockWidget* dockWidget, DockPlacement placement) const;
    void updateTheme();
    void updatePlaceholderState();
    int visibleDockWidgetCount() const;

    Workspace* m_workspace = nullptr;
    QWidget* m_rootDockWidget = nullptr;
    DockGuideOverlay* m_dropGuideOverlay = nullptr;
    DockDragPreviewWindow* m_dragPreviewWindow = nullptr;
    DockDropPreviewWindow* m_dropPreviewWindow = nullptr;
    QSet<AntDockWidget*> m_docks;
    QHash<AntDockWidget*, DockArea*> m_dockAreas;
    QHash<QString, QByteArray> m_perspectives;
    bool m_placeholderVisible = true;
    bool m_dropGuideVisible = true;
    bool m_draggingDockTitle = false;
    bool m_appEventFilterInstalled = false;
    AntDockWidget* m_draggedDock = nullptr;
    QPoint m_dragStartGlobal;
    QPoint m_dragPreviewOffset;
    qreal m_draggedDockPreviousOpacity = 1.0;
    QGraphicsOpacityEffect* m_draggedDockOpacityEffect = nullptr;
    bool m_draggedDockOpacityChanged = false;
    bool m_hasLastDropTarget = false;
    bool m_lastDropTargetIsContainer = false;
    AntDockWidget* m_lastDropTargetDock = nullptr;
    DockPlacement m_lastDropPlacement = DockPlacement::None;
    QRect m_lastDropTargetRect;
    QRect m_lastDropPreviewRect;
    QString m_lastDropLabel;
    int m_autoObjectNameCounter = 0;
};
