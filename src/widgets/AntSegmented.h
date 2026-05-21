#pragma once

#include "core/QtAntDesignExport.h"

#include <QFont>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QString>
#include <QVector>
#include <QWidget>
#include <QVariantAnimation>

#include "core/AntTypes.h"

class AntSegmentedStyle;
class QEvent;

struct AntSegmentedOption
{
    QString value;
    QString label;
    QString icon;
    bool disabled = false;
    QString tooltip;
};

class QT_ANT_DESIGN_EXPORT AntSegmented : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(bool block READ isBlock WRITE setBlock NOTIFY blockChanged)
    Q_PROPERTY(Ant::Size segmentedSize READ segmentedSize WRITE setSegmentedSize NOTIFY segmentedSizeChanged)
    Q_PROPERTY(bool vertical READ isVertical WRITE setVertical NOTIFY verticalChanged)
    Q_PROPERTY(Ant::SegmentedShape shape READ shape WRITE setShape NOTIFY shapeChanged)

public:
    explicit AntSegmented(QWidget* parent = nullptr);

    void setOptions(const QVector<AntSegmentedOption>& options);
    QVector<AntSegmentedOption> options() const;

    QString value() const;
    void setValue(const QString& value);
    int currentIndex() const;
    void setCurrentIndex(int index);
    bool isBlock() const;
    void setBlock(bool block);
    Ant::Size segmentedSize() const;
    void setSegmentedSize(Ant::Size size);
    bool isVertical() const;
    void setVertical(bool vertical);
    Ant::SegmentedShape shape() const;
    void setShape(Ant::SegmentedShape shape);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    // Accessors needed by AntSegmentedStyle
    int hoveredIndex() const;
    int pressedIndex() const;
    int selectedIndex() const;
    qreal thumbPosition() const;
    QVector<QRectF> segmentRects() const;

Q_SIGNALS:
    void valueChanged(const QString& value);
    void currentIndexChanged(int index);
    void blockChanged(bool block);
    void segmentedSizeChanged(Ant::Size size);
    void verticalChanged(bool vertical);
    void shapeChanged(Ant::SegmentedShape shape);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    friend class AntSegmentedStyle;

    struct SegmentLayout
    {
        QRectF rect;
        QRectF iconRect;
        QRectF textRect;
        int textWidth = 0;
        int segmentWidth = 0;
        bool hasIcon = false;
    };

    struct LayoutCache
    {
        QSize widgetSize;
        QFont font;
        int optionsRevision = 0;
        Ant::Size segmentedSize = Ant::Size::Middle;
        bool block = false;
        bool vertical = false;
        Ant::SegmentedShape shape = Ant::SegmentedShape::Default;
        int fontSize = 14;
        int height = 32;
        int paddingHorizontal = 11;
        int trackRadius = 6;
        int itemRadius = 4;
        QSize sizeHint;
        QVector<QRectF> segmentRects;
        QVector<SegmentLayout> segments;
        bool valid = false;
    };

    const QVector<AntSegmentedOption>& optionList() const;
    const LayoutCache& layoutCache() const;
    QRectF thumbRect(qreal position) const;
    QRectF thumbRect() const;
    QRect segmentDirtyRect(int index) const;
    QRect thumbDirtyRect(qreal position) const;
    void updateSegmentRegions(const QVector<int>& indices, const QString& mode);
    void updateSelectionRegion(int oldIndex, int newIndex, qreal oldThumbPosition);
    void updateThumbRegion(qreal oldPosition, qreal newPosition);
    void invalidateLayoutCache() const;
    void syncSegmentedPerfCounters() const;
    int segmentIndexAt(const QPoint& pos) const;
    void startThumbAnimation(int newIndex);

    QVector<AntSegmentedOption> m_options;
    QString m_value;
    bool m_block = false;
    Ant::Size m_size = Ant::Size::Middle;
    bool m_vertical = false;
    Ant::SegmentedShape m_shape = Ant::SegmentedShape::Default;
    int m_hoveredIndex = -1;
    int m_pressedIndex = -1;
    qreal m_thumbPos = 0;
    QVariantAnimation* m_thumbAnimation = nullptr;
    mutable LayoutCache m_layoutCache;
    int m_optionsRevision = 0;
    mutable int m_layoutBuildCount = 0;
    mutable int m_sizeHintResolveCount = 0;
    mutable int m_textMetricResolveCount = 0;
    int m_regionUpdateCount = 0;
    int m_thumbRegionUpdateCount = 0;
};
