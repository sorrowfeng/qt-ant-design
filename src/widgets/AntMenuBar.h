#pragma once

#include "core/QtAntDesignExport.h"

#include <QHash>
#include <QMenuBar>
#include <QRect>

class QAction;
class QActionEvent;
class QEvent;
class QMenu;
class QMouseEvent;
class QResizeEvent;

class QT_ANT_DESIGN_EXPORT AntMenuBar : public QMenuBar
{
    Q_OBJECT

public:
    explicit AntMenuBar(QWidget* parent = nullptr);

    QMenu* addMenu(const QString& title);
    QMenu* addMenu(const QIcon& icon, const QString& title);

protected:
    void actionEvent(QActionEvent* event) override;
    void changeEvent(QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QRect cachedActionGeometry(QAction* action) const;
    void invalidateActionGeometryCache() const;
    void syncMenuBarPerfCounters() const;

    mutable QHash<const QAction*, QRect> m_actionGeometryCache;
    mutable int m_actionGeometryCacheBuildCount = 0;
    QAction* m_hoveredAction = nullptr;
    bool m_lastHoverUpdateWasScoped = false;
};
