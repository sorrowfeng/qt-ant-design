#pragma once

#include "core/QtAntDesignExport.h"

#include <QFont>
#include <QPointer>
#include <QPolygonF>
#include <QRect>
#include <QSize>
#include <QWidget>

#include "core/AntTypes.h"

class AntPopoverStyle;
class QTimer;

class QT_ANT_DESIGN_EXPORT AntPopover : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
    Q_PROPERTY(Ant::TooltipPlacement placement READ placement WRITE setPlacement NOTIFY placementChanged)
    Q_PROPERTY(Ant::PopoverTrigger trigger READ trigger WRITE setTrigger NOTIFY triggerChanged)
    Q_PROPERTY(bool arrowVisible READ arrowVisible WRITE setArrowVisible NOTIFY arrowVisibleChanged)
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)

public:
    explicit AntPopover(QWidget* parent = nullptr);
    ~AntPopover() override;

    QString title() const;
    void setTitle(const QString& title);
    Ant::IconType titleIconType() const;
    void setTitleIconType(Ant::IconType iconType);

    QString content() const;
    void setContent(const QString& content);

    Ant::TooltipPlacement placement() const;
    void setPlacement(Ant::TooltipPlacement placement);
    Ant::TooltipPlacement renderPlacement() const;

    Ant::PopoverTrigger trigger() const;
    void setTrigger(Ant::PopoverTrigger trigger);

    bool arrowVisible() const;
    void setArrowVisible(bool visible);

    bool isOpen() const;
    void setOpen(bool open);

    QWidget* target() const;
    void setTarget(QWidget* target);

    QWidget* actionWidget() const;
    void setActionWidget(QWidget* widget);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void titleChanged(const QString& title);
    void contentChanged(const QString& content);
    void placementChanged(Ant::TooltipPlacement placement);
    void triggerChanged(Ant::PopoverTrigger trigger);
    void arrowVisibleChanged(bool visible);
    void openChanged(bool open);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void enterEvent(AntEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    friend class AntPopoverStyle;

    struct Metrics
    {
        int shadowMargin = 8;
        int paddingX = 12;
        int paddingY = 12;
        int radius = 8;
        int arrowSize = 8;
        int gap = 0;
        int titleBodyGap = 8;
        int titleMinWidth = 177;
        int maxWidth = 320;
    };

    struct PopoverLayoutCache
    {
        bool valid = false;
        QSize widgetSize;
        QString title;
        QString content;
        Ant::IconType titleIconType = Ant::IconType::None;
        Ant::TooltipPlacement renderPlacement = Ant::TooltipPlacement::Top;
        bool arrowVisible = true;
        bool hasActionWidget = false;
        QSize actionSize;
        QFont font;
        Ant::ThemeMode themeMode = Ant::ThemeMode::Default;
        int tokenFontSize = 0;
        qreal tokenLineHeight = 0.0;
        Metrics metrics;
        QSize sizeHint;
        QSize minimumSizeHint;
        QRect bubbleRect;
        QRect headerRect;
        QRect bodyRect;
        QRect actionRect;
        QPolygonF arrowPolygon;
    };

    Metrics metrics() const;
    const PopoverLayoutCache& popoverLayout() const;
    void invalidatePopoverLayout() const;
    void invalidatePopoverPlacementCache() const;
    void applyPopoverSizeHint();
    void requestPopoverUpdate(const QRect& region, const QString& mode);
    void syncPopoverPerfCounters() const;
    QRect bubbleRect() const;
    QPolygonF arrowPolygon() const;
    QRect headerRect() const;
    QRect bodyRect() const;
    QRect actionRect() const;
    void updatePosition();
    void syncActionGeometry();
    void installTarget(QWidget* target);
    void uninstallTarget();
    bool isHoveringInteractiveArea() const;
    Ant::TooltipPlacement resolvedPlacement(const QRect& targetRect, const QRect& screenRect, const QSize& popupSize) const;
    QPoint popupTopLeft(const QRect& targetRect, const QSize& popupSize, Ant::TooltipPlacement placement) const;

    QString m_title;
    Ant::IconType m_titleIconType = Ant::IconType::None;
    QString m_content;
    Ant::TooltipPlacement m_placement = Ant::TooltipPlacement::Top;
    Ant::TooltipPlacement m_renderPlacement = Ant::TooltipPlacement::Top;
    Ant::PopoverTrigger m_trigger = Ant::PopoverTrigger::Hover;
    bool m_arrowVisible = true;
    bool m_open = false;
    QPointer<QWidget> m_target;
    QPointer<QWidget> m_actionWidget;
    QTimer* m_openTimer = nullptr;
    QTimer* m_closeTimer = nullptr;
    mutable PopoverLayoutCache m_layoutCache;
    mutable bool m_positionCacheValid = false;
    mutable QRect m_cachedTargetRect;
    mutable QRect m_cachedScreenRect;
    mutable QSize m_cachedPopupSize;
    mutable Ant::TooltipPlacement m_cachedPlacementRequest = Ant::TooltipPlacement::Top;
    mutable Ant::TooltipPlacement m_cachedResolvedPlacement = Ant::TooltipPlacement::Top;
    mutable int m_layoutBuildCount = 0;
    mutable int m_layoutCacheHitCount = 0;
    mutable int m_positionResolveCount = 0;
    mutable int m_positionResolveSkipCount = 0;
    int m_sizeApplyCount = 0;
    int m_sizeSkipCount = 0;
    int m_actionGeometryApplyCount = 0;
    int m_actionGeometrySkipCount = 0;
    int m_positionApplyCount = 0;
    int m_positionSkipCount = 0;
    int m_regionUpdateCount = 0;
    QString m_lastUpdateMode;
};
