#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QPoint>
#include <QPointer>
#include <QScrollArea>
#include <QVector>

class AntScrollBar;
class QMouseEvent;

class QT_ANT_DESIGN_EXPORT AntScrollArea : public QScrollArea
{
    Q_OBJECT
    Q_PROPERTY(bool autoHideScrollBar READ autoHideScrollBar WRITE setAutoHideScrollBar NOTIFY autoHideScrollBarChanged)
    Q_PROPERTY(bool enableGesture READ isGestureEnabled WRITE setEnableGesture NOTIFY enableGestureChanged)
    Q_PROPERTY(bool mouseDragScrollEnabled READ isMouseDragScrollEnabled WRITE setMouseDragScrollEnabled NOTIFY mouseDragScrollEnabledChanged)

public:
    explicit AntScrollArea(QWidget* parent = nullptr);

    void setWidget(QWidget* widget);

    bool autoHideScrollBar() const;
    void setAutoHideScrollBar(bool autoHide);

    bool isGestureEnabled() const;
    void setEnableGesture(bool enable);

    bool isMouseDragScrollEnabled() const;
    void setMouseDragScrollEnabled(bool enabled);

Q_SIGNALS:
    void autoHideScrollBarChanged(bool autoHide);
    void enableGestureChanged(bool enable);
    void mouseDragScrollEnabledChanged(bool enabled);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void updateGestureGrab();
    void updateMouseDragTargets();
    void clearMouseDragTargets();
    void installMouseDragTarget(QWidget* target);
    bool isMouseDragTargetCandidate(QWidget* target) const;
    bool handleMouseDragScrollEvent(QObject* watched, QEvent* event);
    bool hasMouseDragScrollableRange() const;
    QPoint mouseDragViewportPos(QObject* watched, const QMouseEvent* event) const;
    void resetMouseDragScrollState();
    bool updateTheme();
    void syncScrollAreaPerfCounters() const;

    AntScrollBar* m_vScrollBar = nullptr;
    AntScrollBar* m_hScrollBar = nullptr;
    bool m_autoHideScrollBar = true;
    bool m_enableGesture = true;
    bool m_mouseDragScrollEnabled = true;
    bool m_mouseDragPressed = false;
    bool m_mouseDragActive = false;
    QPoint m_mouseDragStartPos;
    int m_mouseDragStartHValue = 0;
    int m_mouseDragStartVValue = 0;
    QVector<QPointer<QObject>> m_mouseDragFilteredObjects;
    bool m_themeCacheValid = false;
    QColor m_cachedBgColor;
    QColor m_cachedTextColor;
    QPointer<QWidget> m_cachedContentWidget;
    int m_surfacePaletteApplyCount = 0;
    int m_viewportPaletteApplyCount = 0;
    int m_contentPaletteApplyCount = 0;
    int m_viewportUpdateCount = 0;
};
