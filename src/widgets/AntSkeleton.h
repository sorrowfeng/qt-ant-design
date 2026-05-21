#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QPainterPath>
#include <QPointer>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QString>
#include <QWidget>
#include <QVector>

#include "core/AntTypes.h"

class AntSkeletonStyle;
class QEvent;
class QHideEvent;
class QShowEvent;
class QTimer;

class QT_ANT_DESIGN_EXPORT AntSkeleton : public QWidget
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
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    friend class AntSkeletonStyle;

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

    struct PlaceholderItem
    {
        QRectF rect;
        QPainterPath path;
        qreal radius = 0.0;
        bool imageElement = false;
    };

    struct SkeletonLayoutCache
    {
        bool valid = false;
        QSize widgetSize;
        Ant::ThemeMode themeMode = Ant::ThemeMode::Default;
        int tokenControlHeightLG = 0;
        int tokenFontSizeLG = 0;
        int tokenFontSizeSM = 0;
        int tokenMarginSM = 0;
        int tokenMargin = 0;
        int tokenBorderRadiusSM = 0;
        bool loading = true;
        bool active = true;
        bool round = false;
        bool avatarVisible = false;
        Ant::AvatarShape avatarShape = Ant::AvatarShape::Circle;
        bool titleVisible = true;
        bool paragraphVisible = true;
        int paragraphRows = 3;
        Ant::SkeletonElement element = Ant::SkeletonElement::Default;
        qreal titleWidthRatio = 0.42;
        QList<qreal> paragraphWidthRatios;
        bool hasContentWidget = false;
        QSize contentSizeHint;
        Metrics metrics;
        QSize sizeHint;
        QSize minimumSizeHint;
        QColor baseColor;
        QColor highlightColor;
        QVector<PlaceholderItem> placeholders;
        QRect visualRect;
    };

    const SkeletonLayoutCache& skeletonLayout() const;
    void invalidateSkeletonLayout() const;
    Metrics resolveMetrics() const;
    qreal rowWidthRatio(int rowIndex) const;
    QSize skeletonSizeHint(const Metrics& metrics) const;
    void appendPlaceholder(SkeletonLayoutCache& cache, const QRectF& rect, bool imageElement = false) const;
    QRect skeletonVisualRect() const;
    QRect shimmerDirtyRect(int oldOffset, int newOffset) const;
    QRect shimmerBandRectForOffset(const QRectF& rect, int offset) const;
    void requestSkeletonUpdate(const QRect& region, const QString& mode);
    void syncSkeletonPerfCounters() const;
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
    mutable SkeletonLayoutCache m_layoutCache;
    mutable int m_layoutBuildCount = 0;
    mutable int m_layoutCacheHitCount = 0;
    int m_shimmerRegionUpdateCount = 0;
    int m_visualRegionUpdateCount = 0;
    int m_contentGeometryApplyCount = 0;
    int m_contentGeometrySkipCount = 0;
    QString m_lastUpdateMode;
};
