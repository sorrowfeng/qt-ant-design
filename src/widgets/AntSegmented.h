#pragma once

#include "core/QtAntDesignExport.h"

#include <QVector>
#include <QWidget>
#include <QVariantAnimation>

#include "core/AntTypes.h"

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

private:
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
};
