#pragma once

#include "core/QtAntDesignExport.h"

#include <QPainterPath>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"

class QResizeEvent;
class QVariantAnimation;

class QT_ANT_DESIGN_EXPORT AntRate : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)
    Q_PROPERTY(bool allowHalf READ allowHalf WRITE setAllowHalf NOTIFY allowHalfChanged)
    Q_PROPERTY(bool allowClear READ allowClear WRITE setAllowClear NOTIFY allowClearChanged)
    Q_PROPERTY(bool disabled READ isDisabled WRITE setDisabled NOTIFY disabledChanged)
    Q_PROPERTY(Ant::Size rateSize READ rateSize WRITE setRateSize NOTIFY rateSizeChanged)

public:
    explicit AntRate(QWidget* parent = nullptr);

    double value() const;
    void setValue(double value);

    int count() const;
    void setCount(int count);

    bool allowHalf() const;
    void setAllowHalf(bool allow);

    bool allowClear() const;
    void setAllowClear(bool allow);

    bool isDisabled() const;
    void setDisabled(bool disabled);

    Ant::Size rateSize() const;
    void setRateSize(Ant::Size size);

    double hoverValue() const;
    bool isHoveredState() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void valueChanged(double value);
    void countChanged(int count);
    void allowHalfChanged(bool allow);
    void allowClearChanged(bool allow);
    void disabledChanged(bool disabled);
    void rateSizeChanged(Ant::Size size);
    void hoverChanged(double value);

protected:
    void enterEvent(AntEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    friend class AntRateStyle;

    struct LayoutCache
    {
        QSize widgetSize;
        Ant::Size rateSize = Ant::Size::Middle;
        int count = 5;
        int starSize = 20;
        int margin = 8;
        int totalWidth = 0;
        QSize sizeHint;
        QVector<QRectF> starRects;
        QVector<QPainterPath> starPaths;
        bool valid = false;
    };

    const LayoutCache& layoutCache() const;
    QSize cachedSizeHint() const;
    QRect starDirtyRect(int index) const;
    QRect valueDirtyRect(double oldValue, double newValue) const;
    QRect focusDirtyRect() const;
    void updateValueRegion(double oldValue, double newValue);
    void updateStarRegion(int index);
    void updateFocusRegion();
    void invalidateLayoutCache() const;
    void syncRatePerfCounters() const;
    double starValueAt(const QPoint& pos) const;
    void updateHoverValue(const QPoint& pos);
    void startSelectionAnimation(double selectedValue);

    double m_value = 0.0;
    double m_hoverValue = -1.0;
    int m_count = 5;
    bool m_allowHalf = false;
    bool m_allowClear = true;
    bool m_disabled = false;
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_focused = false;
    Ant::Size m_rateSize = Ant::Size::Middle;
    int m_selectionAnimationIndex = -1;
    qreal m_selectionScale = 1.0;
    QVariantAnimation* m_selectionAnimation = nullptr;
    mutable LayoutCache m_layoutCache;
    mutable int m_layoutBuildCount = 0;
    mutable int m_sizeHintResolveCount = 0;
    mutable int m_starPathBuildCount = 0;
    int m_valueRegionUpdateCount = 0;
    int m_starRegionUpdateCount = 0;
    int m_focusRegionUpdateCount = 0;
};
