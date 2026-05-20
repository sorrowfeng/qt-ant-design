#pragma once

#include "core/QtAntDesignExport.h"

#include <QPointer>
#include <QRectF>
#include <QVector>
#include <QWidget>

class QLabel;
class QResizeEvent;
class QScrollArea;
class QTimer;
class QVariantAnimation;
class QVBoxLayout;

class QT_ANT_DESIGN_EXPORT AntAnchor : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int activeIndex READ activeIndex NOTIFY activeIndexChanged)

public:
    explicit AntAnchor(QWidget* parent = nullptr);

    void setScrollArea(QScrollArea* area);
    int activeIndex() const;

    void addLink(const QString& title, int targetY);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent* event) override;

Q_SIGNALS:
    void activeIndexChanged(int index);
    void linkClicked(int index, int targetY);

private:
    void setActiveIndexInternal(int index, bool animate);
    QRectF indicatorRectForIndex(int index) const;
    void scheduleScrollResolve(int value);
    void resolvePendingScrollPosition();
    void applyLabelVisualState();
    void invalidateLinkRectCache() const;
    void ensureLinkRectCache() const;
    void refreshActiveIndicatorGeometry();
    QRect indicatorDirtyRect(const QRectF& previous, const QRectF& current) const;
    void syncAnchorPerfCounters() const;

    QScrollArea* m_scrollArea = nullptr;
    QVBoxLayout* m_layout = nullptr;
    QVariantAnimation* m_indicatorAnimation = nullptr;
    QTimer* m_scrollResolveTimer = nullptr;
    QRectF m_indicatorRect;
    int m_activeIndex = -1;
    struct Link { QString title; int targetY = 0; };
    QList<Link> m_links;
    QVector<QPointer<QLabel>> m_linkLabels;
    int m_pendingScrollValue = 0;

    mutable bool m_linkRectCacheValid = false;
    mutable QVector<QRectF> m_linkIndicatorRects;
    mutable int m_linkRectCacheHitCount = 0;
    mutable int m_linkRectCacheMissCount = 0;
    mutable int m_labelVisualApplyCount = 0;
    mutable int m_scrollResolveCount = 0;
};
