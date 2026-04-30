#pragma once

#include <QPointer>
#include <QWidget>

#include "core/AntTypes.h"

class QTimer;

class AntSkeleton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool loading READ isLoading WRITE setLoading NOTIFY loadingChanged)
    Q_PROPERTY(bool round READ isRound WRITE setRound NOTIFY roundChanged)
    Q_PROPERTY(bool avatarVisible READ avatarVisible WRITE setAvatarVisible NOTIFY avatarVisibleChanged)
    Q_PROPERTY(Ant::AvatarShape avatarShape READ avatarShape WRITE setAvatarShape NOTIFY avatarShapeChanged)
    Q_PROPERTY(bool titleVisible READ titleVisible WRITE setTitleVisible NOTIFY titleVisibleChanged)
    Q_PROPERTY(bool paragraphVisible READ paragraphVisible WRITE setParagraphVisible NOTIFY paragraphVisibleChanged)
    Q_PROPERTY(int paragraphRows READ paragraphRows WRITE setParagraphRows NOTIFY paragraphRowsChanged)
    Q_PROPERTY(Ant::SkeletonElement element READ element WRITE setElement NOTIFY elementChanged)

public:
    explicit AntSkeleton(QWidget* parent = nullptr);

    bool isActive() const;
    void setActive(bool active);

    bool isLoading() const;
    void setLoading(bool loading);

    bool isRound() const;
    void setRound(bool round);

    bool avatarVisible() const;
    void setAvatarVisible(bool visible);

    Ant::AvatarShape avatarShape() const;
    void setAvatarShape(Ant::AvatarShape shape);

    bool titleVisible() const;
    void setTitleVisible(bool visible);

    bool paragraphVisible() const;
    void setParagraphVisible(bool visible);

    int paragraphRows() const;
    void setParagraphRows(int rows);

    Ant::SkeletonElement element() const;
    void setElement(Ant::SkeletonElement element);

    QWidget* contentWidget() const;
    void setContentWidget(QWidget* widget);

    void setTitleWidthRatio(qreal ratio);
    qreal titleWidthRatio() const;

    void setParagraphWidthRatios(const QList<qreal>& ratios);
    QList<qreal> paragraphWidthRatios() const;
    int shimmerOffset() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void activeChanged(bool active);
    void loadingChanged(bool loading);
    void roundChanged(bool round);
    void avatarVisibleChanged(bool visible);
    void avatarShapeChanged(Ant::AvatarShape shape);
    void titleVisibleChanged(bool visible);
    void paragraphVisibleChanged(bool visible);
    void paragraphRowsChanged(int rows);
    void elementChanged(Ant::SkeletonElement element);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct Metrics
    {
        int avatarSize = 40;
        int titleHeight = 16;
        int paragraphHeight = 12;
        int rowSpacing = 12;
        int columnGap = 16;
        int radius = 6;
        int titleTop = 4;
    };

    Metrics metrics() const;
    QList<QRectF> placeholderRects() const;
    qreal rowWidthRatio(int rowIndex) const;
    void syncContentGeometry();
    void updateTimerState();

    bool m_active = true;
    bool m_loading = true;
    bool m_round = false;
    bool m_avatarVisible = false;
    Ant::AvatarShape m_avatarShape = Ant::AvatarShape::Circle;
    bool m_titleVisible = true;
    bool m_paragraphVisible = true;
    int m_paragraphRows = 3;
    Ant::SkeletonElement m_element = Ant::SkeletonElement::Default;
    qreal m_titleWidthRatio = 0.42;
    QList<qreal> m_paragraphWidthRatios;
    QPointer<QWidget> m_contentWidget;
    QTimer* m_timer = nullptr;
    int m_shimmerOffset = 0;
};
