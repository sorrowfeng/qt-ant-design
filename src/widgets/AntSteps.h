#pragma once

#include "core/QtAntDesignExport.h"

#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QMouseEvent;
class QPaintEvent;

struct AntStepItem
{
    QString title;
    QString description;
    QString subTitle;
    Ant::StepStatus status = Ant::StepStatus::Wait;
    bool disabled = false;
};

class QT_ANT_DESIGN_EXPORT AntSteps : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(Ant::Orientation direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(bool clickable READ isClickable WRITE setClickable NOTIFY clickableChanged)

public:
    explicit AntSteps(QWidget* parent = nullptr);

    int currentIndex() const;
    void setCurrentIndex(int index);

    Ant::Orientation direction() const;
    void setDirection(Ant::Orientation direction);

    bool isClickable() const;
    void setClickable(bool clickable);

    int count() const;
    AntStepItem stepAt(int index) const;
    void addStep(const AntStepItem& step);
    void addStep(const QString& title,
                 const QString& description = QString(),
                 const QString& subTitle = QString(),
                 Ant::StepStatus status = Ant::StepStatus::Wait,
                 bool disabled = false);
    void setStepStatus(int index, Ant::StepStatus status);
    void clearSteps();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void currentIndexChanged(int index);
    void directionChanged(Ant::Orientation direction);
    void clickableChanged(bool clickable);
    void stepClicked(int index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    friend class AntStepsStyle;

    struct Metrics
    {
        int iconSize = 32;
        int titleGap = 12;
        int tailThickness = 2;
        int itemGap = 20;
        int titleFontSize = 14;
        int descFontSize = 12;
    };

    struct LayoutItem
    {
        QRect itemRect;
        QRect iconRect;
        QRect textRect;
    };

    Metrics metrics() const;
    QVector<LayoutItem> layoutItems() const;
    QRect iconRect(const QRect& itemRect) const;
    QRect textRect(const QRect& itemRect) const;
    int stepAtPos(const QPoint& pos) const;
    Ant::StepStatus effectiveStatus(int index) const;
    QColor statusColor(Ant::StepStatus status) const;
    QString iconText(Ant::StepStatus status, int index) const;
    void invalidateLayoutCache();
    QRect stepDirtyRect(int index) const;
    void updateStepStateRegion(int oldIndex, int newIndex);
    void updateStepRangeRegion(int oldIndex, int newIndex);
    void updateStepsGeometry();
    void syncStepsPerfCounters() const;

    QVector<AntStepItem> m_steps;
    int m_currentIndex = 0;
    Ant::Orientation m_direction = Ant::Orientation::Horizontal;
    bool m_clickable = true;
    int m_hoveredIndex = -1;
    quint64 m_layoutRevision = 1;
    mutable bool m_layoutCacheValid = false;
    mutable quint64 m_layoutCacheRevision = 0;
    mutable QSize m_layoutCacheSize;
    mutable QVector<LayoutItem> m_cachedLayoutItems;
    mutable int m_layoutCacheHitCount = 0;
    mutable int m_layoutBuildCount = 0;
    int m_scopedStepUpdateCount = 0;
};
