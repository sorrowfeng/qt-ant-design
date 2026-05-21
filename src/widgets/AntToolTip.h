#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QFont>
#include <QMetaObject>
#include <QPolygonF>
#include <QPointer>
#include <QRect>
#include <QSize>
#include <QString>
#include <QWidget>

#include "core/AntTypes.h"

class AntToolTipStyle;
class QEnterEvent;
class QEvent;
class QFocusEvent;
class QMouseEvent;
class QPaintEvent;
class QTimer;

class QT_ANT_DESIGN_EXPORT AntToolTip : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(Ant::TooltipPlacement placement READ placement WRITE setPlacement NOTIFY placementChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool arrowVisible READ arrowVisible WRITE setArrowVisible NOTIFY arrowVisibleChanged)
    Q_PROPERTY(int openDelay READ openDelay WRITE setOpenDelay NOTIFY openDelayChanged)

public:
    explicit AntToolTip(QWidget* parent = nullptr);
    ~AntToolTip() override;

    QString title() const;
    void setTitle(const QString& title);

    Ant::TooltipPlacement placement() const;
    void setPlacement(Ant::TooltipPlacement placement);
    Ant::TooltipPlacement renderPlacement() const;

    QColor color() const;
    void setColor(const QColor& color);

    bool arrowVisible() const;
    void setArrowVisible(bool visible);

    int openDelay() const;
    void setOpenDelay(int delayMs);

    QWidget* target() const;
    void setTarget(QWidget* target);

    void showTooltip();
    void hideTooltip();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void titleChanged(const QString& title);
    void placementChanged(Ant::TooltipPlacement placement);
    void colorChanged(const QColor& color);
    void arrowVisibleChanged(bool visible);
    void openDelayChanged(int delayMs);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    friend class AntToolTipStyle;

    struct Metrics
    {
        int paddingX = 12;
        int paddingY = 8;
        int radius = 6;
        int arrowSize = 8;
        int gap = 10;
        int maxWidth = 280;
    };

    struct ToolTipLayoutCache
    {
        bool valid = false;
        QSize widgetSize;
        QFont font;
        Ant::ThemeMode themeMode = Ant::ThemeMode::Default;
        int tokenFontSizeSM = 0;
        int tokenBorderRadiusSM = 0;
        QString title;
        QColor customColor;
        bool arrowVisible = true;
        Ant::TooltipPlacement renderPlacement = Ant::TooltipPlacement::Top;
        Metrics metrics;
        QSize sizeHint;
        QSize minimumSizeHint;
        QRect bubbleRect;
        QRect textRect;
        QPolygonF arrowPolygon;
        QColor bubbleColor;
        QColor textColor;
    };

    Metrics metrics() const;
    const ToolTipLayoutCache& tooltipLayout() const;
    void invalidateToolTipLayout() const;
    void invalidateToolTipPosition();
    QRect bubbleRect() const;
    QPolygonF arrowPolygon() const;
    QColor bubbleColor() const;
    QColor textColor() const;
    Ant::TooltipPlacement resolvedPlacement(const QRect& targetRect, const QRect& screenRect, const QSize& tooltipSize) const;
    QPoint tooltipTopLeft(const QRect& targetRect, const QSize& tooltipSize, Ant::TooltipPlacement placement) const;
    void updatePosition();
    void requestToolTipUpdate(const QRect& region, const QString& mode);
    void maybeStartOpenTimer();
    void syncToolTipPerfCounters() const;
    void installTarget(QWidget* target);
    void uninstallTarget();

    QString m_title;
    Ant::TooltipPlacement m_placement = Ant::TooltipPlacement::Top;
    Ant::TooltipPlacement m_renderPlacement = Ant::TooltipPlacement::Top;
    QColor m_color;
    bool m_arrowVisible = true;
    int m_openDelay = 120;
    QPointer<QWidget> m_target;
    QTimer* m_openTimer = nullptr;
    QMetaObject::Connection m_targetDestroyedConnection;
    mutable ToolTipLayoutCache m_layoutCache;
    QRect m_lastTargetRect;
    QRect m_lastScreenRect;
    QSize m_lastTooltipSize;
    QPoint m_lastTooltipTopLeft;
    Ant::TooltipPlacement m_lastPositionPlacement = Ant::TooltipPlacement::Top;
    bool m_positionCacheValid = false;
    mutable int m_layoutBuildCount = 0;
    mutable int m_layoutCacheHitCount = 0;
    int m_positionApplyCount = 0;
    int m_positionSkipCount = 0;
    int m_openTimerStartCount = 0;
    int m_openTimerSkipCount = 0;
    int m_regionUpdateCount = 0;
    QString m_lastUpdateMode;
};
