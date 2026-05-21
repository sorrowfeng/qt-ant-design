#pragma once

#include "core/QtAntDesignExport.h"

#include <QRect>
#include <QStringList>
#include <QVector>
#include <QWidget>

class QListWidget;
class AntButton;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QWheelEvent;

class QT_ANT_DESIGN_EXPORT AntTransfer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList sourceItems READ sourceItems NOTIFY itemsChanged)
    Q_PROPERTY(QStringList targetItems READ targetItems NOTIFY itemsChanged)

public:
    explicit AntTransfer(QWidget* parent = nullptr);

    QStringList sourceItems() const;
    QStringList targetItems() const;

    void setSourceItems(const QStringList& items);
    void setTargetItems(const QStringList& items);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void itemsChanged();

private:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    struct RowLayout
    {
        int itemIndex = -1;
        QRect rowRect;
        QRect checkRect;
    };

    struct PanelLayout
    {
        bool sourcePanel = true;
        QSize widgetSize;
        int itemCount = 0;
        int scrollOffset = 0;
        QRect panelRect;
        QRect headerRect;
        QRect headerCheckRect;
        QRect bodyRect;
        QRect scrollTrackRect;
        QRect scrollThumbRect;
        QVector<RowLayout> visibleRows;
        bool hasScrollBar = false;
        bool valid = false;
    };

    void doTransfer(bool toTarget);
    void updateButtons();
    int rowAt(const QPoint& pos, bool sourcePanel) const;
    QRect panelRect(bool sourcePanel) const;
    QRect headerRect(bool sourcePanel) const;
    QRect headerCheckRect(bool sourcePanel) const;
    QRect rowRectForIndex(bool sourcePanel, int itemIndex) const;
    QRect buttonRect(bool toTarget) const;
    QRect buttonColumnRect() const;
    int visibleRowCount() const;
    int maxScrollOffset(bool sourcePanel) const;
    int scrollOffset(bool sourcePanel) const;
    void setScrollOffset(bool sourcePanel, int offset);
    void togglePanelSelection(bool sourcePanel);
    const QStringList& panelItems(bool sourcePanel) const;
    void syncListWidget(bool sourcePanel);
    const PanelLayout& panelLayout(bool sourcePanel) const;
    void invalidatePanelLayout(bool sourcePanel) const;
    void invalidatePanelLayouts() const;
    void updateTransferRegion(const QRect& dirty, const QString& mode, bool rowScoped = false, bool panelScoped = false);
    void syncTransferPerfCounters() const;

    QListWidget* m_sourceList = nullptr;
    QListWidget* m_targetList = nullptr;
    AntButton* m_toTargetBtn = nullptr;
    AntButton* m_toSourceBtn = nullptr;
    QStringList m_sourceItemsData;
    QStringList m_targetItemsData;
    QStringList m_selectedSourceItems;
    QStringList m_selectedTargetItems;
    int m_sourceScrollOffset = 0;
    int m_targetScrollOffset = 0;
    mutable PanelLayout m_sourceLayout;
    mutable PanelLayout m_targetLayout;
    mutable int m_layoutBuildCount = 0;
    mutable int m_layoutCacheHitCount = 0;
    int m_regionUpdateCount = 0;
    int m_rowRegionUpdateCount = 0;
    int m_panelRegionUpdateCount = 0;
};
