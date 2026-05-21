#pragma once

#include "core/QtAntDesignExport.h"

#include <QPointer>
#include <QPropertyAnimation>
#include <QRect>
#include <QRegion>
#include <QWidget>

#include "core/AntTypes.h"

class QAbstractButton;
class QEvent;
class QKeyEvent;
class QLabel;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;

class QT_ANT_DESIGN_EXPORT AntDrawer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(Ant::DrawerPlacement placement READ placement WRITE setPlacement NOTIFY placementChanged)
    Q_PROPERTY(int drawerWidth READ drawerWidth WRITE setDrawerWidth NOTIFY drawerWidthChanged)
    Q_PROPERTY(int drawerHeight READ drawerHeight WRITE setDrawerHeight NOTIFY drawerHeightChanged)
    Q_PROPERTY(bool closable READ isClosable WRITE setClosable NOTIFY closableChanged)
    Q_PROPERTY(bool maskClosable READ isMaskClosable WRITE setMaskClosable NOTIFY maskClosableChanged)
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)

public:
    explicit AntDrawer(QWidget* parent = nullptr);

    QString title() const;
    void setTitle(const QString& title);

    Ant::DrawerPlacement placement() const;
    void setPlacement(Ant::DrawerPlacement placement);

    int drawerWidth() const;
    void setDrawerWidth(int width);

    int drawerHeight() const;
    void setDrawerHeight(int height);

    bool isClosable() const;
    void setClosable(bool closable);

    bool isMaskClosable() const;
    void setMaskClosable(bool closable);

    bool isOpen() const;
    void setOpen(bool open);

    QWidget* bodyWidget() const;
    void setBodyWidget(QWidget* widget);

    void open();
    void close();
    void toggle();

    // 0 when panel is fully offscreen, 1 when fully onscreen. Read by style
    // to fade the mask alongside the slide animation.
    qreal maskProgress() const;
    QRect currentPanelGeometry() const;

Q_SIGNALS:
    void titleChanged(const QString& title);
    void placementChanged(Ant::DrawerPlacement placement);
    void drawerWidthChanged(int width);
    void drawerHeightChanged(int height);
    void closableChanged(bool closable);
    void maskClosableChanged(bool closable);
    void openChanged(bool open);
    void opened();
    void closed();
    void aboutToClose();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    class DrawerPanel;

    struct GeometryCache
    {
        bool valid = false;
        QSize overlaySize;
        Ant::DrawerPlacement placement = Ant::DrawerPlacement::Right;
        int drawerWidth = 0;
        int drawerHeight = 0;
        QRect endGeometry;
        QRect startGeometry;
    };

    void ensureHostWidget();
    void releaseHostWidget();
    void syncTheme();
    void updateOverlayGeometry();
    void updatePanelGeometry();
    QRect panelEndGeometry() const;
    QRect panelStartGeometry() const;
    void startAnimation(const QRect& start, const QRect& end);
    void updateAnimationFrame(const QRect& panelGeometry);
    void onAnimationFinished();
    const GeometryCache& drawerGeometryCache() const;
    void invalidateDrawerGeometry() const;
    qreal maskProgressForPanelGeometry(const QRect& panelGeometry) const;
    QRect panelShadowRegion(const QRect& panelGeometry) const;
    QRegion animationDirtyRegion(const QRect& panelGeometry);
    void requestDrawerUpdate(const QRegion& region, const QString& mode);
    void syncDrawerPerfCounters() const;

    QString m_title;
    Ant::DrawerPlacement m_placement = Ant::DrawerPlacement::Right;
    int m_drawerWidth = 378;
    int m_drawerHeight = 378;
    bool m_closable = true;
    bool m_maskClosable = true;
    bool m_open = false;
    bool m_animating = false;

    QPointer<QWidget> m_hostWidget;
    DrawerPanel* m_panel = nullptr;
    QWidget* m_headerWidget = nullptr;
    QWidget* m_bodyWidget = nullptr;
    QWidget* m_customBodyWidget = nullptr;
    QLabel* m_titleLabel = nullptr;
    QAbstractButton* m_closeButton = nullptr;
    QPropertyAnimation* m_animation = nullptr;
    mutable GeometryCache m_geometryCache;
    mutable int m_geometryBuildCount = 0;
    mutable int m_geometryCacheHitCount = 0;
    int m_overlayGeometryApplyCount = 0;
    int m_overlayGeometrySkipCount = 0;
    int m_panelGeometryApplyCount = 0;
    int m_panelGeometrySkipCount = 0;
    int m_animationRegionUpdateCount = 0;
    int m_maskRegionUpdateCount = 0;
    int m_panelRegionUpdateCount = 0;
    int m_themeApplyCount = 0;
    int m_themeSkipCount = 0;
    QRect m_lastPanelAnimationGeometry;
    qreal m_lastMaskProgress = -1.0;
    QString m_themeKey;
    QString m_lastUpdateMode;
};
