#pragma once

#include "core/QtAntDesignExport.h"

#include <QFont>
#include <QMargins>
#include <QPointer>
#include <QStringList>
#include <QWidget>

#include "core/AntTypes.h"

class AntMenu;
class QFrame;
class QTimer;

class QT_ANT_DESIGN_EXPORT AntDropdown : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList itemLabels READ itemLabels WRITE setItemLabels NOTIFY itemLabelsChanged)
    Q_PROPERTY(Ant::DropdownPlacement placement READ placement WRITE setPlacement NOTIFY placementChanged)
    Q_PROPERTY(Ant::DropdownTrigger trigger READ trigger WRITE setTrigger NOTIFY triggerChanged)
    Q_PROPERTY(bool arrowVisible READ arrowVisible WRITE setArrowVisible NOTIFY arrowVisibleChanged)
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)

public:
    explicit AntDropdown(QWidget* parent = nullptr);
    ~AntDropdown() override;

    QStringList itemLabels() const;
    void setItemLabels(const QStringList& labels);

    Ant::DropdownPlacement placement() const;
    void setPlacement(Ant::DropdownPlacement placement);
    Ant::DropdownPlacement renderPlacement() const;

    Ant::DropdownTrigger trigger() const;
    void setTrigger(Ant::DropdownTrigger trigger);

    bool arrowVisible() const;
    void setArrowVisible(bool visible);

    bool isOpen() const;
    void setOpen(bool open);

    QWidget* target() const;
    void setTarget(QWidget* target);

    AntMenu* menu() const;
    void clearItems();
    void addItem(const QString& key, const QString& label, const QString& iconText = QString(), bool disabled = false);
    void addDivider();

Q_SIGNALS:
    void itemLabelsChanged(const QStringList& labels);
    void placementChanged(Ant::DropdownPlacement placement);
    void triggerChanged(Ant::DropdownTrigger trigger);
    void arrowVisibleChanged(bool visible);
    void openChanged(bool open);
    void itemTriggered(const QString& key);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    class PopupFrame;

    QRect popupGeometry(const QRect& targetRect, const QSize& popupSize, Ant::DropdownPlacement placement) const;
    Ant::DropdownPlacement resolvedPlacement(const QRect& targetRect, const QSize& popupSize, const QRect& screenRect) const;
    int popupContentWidth() const;
    QMargins popupContentMargins() const;
    void invalidatePopupCaches();
    void syncDropdownPerfCounters() const;
    void updatePopupGeometry(const QPoint& contextPos = QPoint());
    void installTarget(QWidget* target);
    void uninstallTarget();
    void handleTargetEnter();
    void handleTargetLeave();
    void handlePopupEnter();
    void handlePopupLeave();
    void setOpenInternal(bool open, bool hoverDriven);

    QStringList m_itemLabels;
    Ant::DropdownPlacement m_placement = Ant::DropdownPlacement::BottomLeft;
    Ant::DropdownPlacement m_renderPlacement = Ant::DropdownPlacement::BottomLeft;
    Ant::DropdownTrigger m_trigger = Ant::DropdownTrigger::Hover;
    bool m_arrowVisible = false;
    bool m_open = false;
    QPointer<QWidget> m_target;
    PopupFrame* m_popup = nullptr;
    AntMenu* m_menu = nullptr;
    QTimer* m_openTimer = nullptr;
    QTimer* m_closeTimer = nullptr;
    QTimer* m_hoverTicker = nullptr;
    int m_offTicks = 0;
    bool m_hoverCloseTracking = false;
    QPoint m_lastContextPos;
    bool m_popupSizeDirty = true;
    bool m_popupGeometryCacheValid = false;
    QRect m_lastPopupGeometry;
    Ant::DropdownPlacement m_lastGeometryPlacement = Ant::DropdownPlacement::BottomLeft;

    mutable bool m_contentWidthCacheValid = false;
    mutable QStringList m_contentWidthCacheLabels;
    mutable QFont m_contentWidthCacheFont;
    mutable int m_contentWidthCacheTokenFontSize = 0;
    mutable int m_contentWidthCachePaddingSM = 0;
    mutable int m_contentWidthCachePaddingXS = 0;
    mutable int m_cachedContentWidth = 0;
    mutable int m_contentWidthCacheHitCount = 0;
    mutable int m_contentWidthCacheMissCount = 0;
    int m_geometryApplyCount = 0;
    int m_geometrySkipCount = 0;
    int m_marginApplyCount = 0;
    int m_menuWidthApplyCount = 0;
    int m_popupAdjustSizeCount = 0;
};
