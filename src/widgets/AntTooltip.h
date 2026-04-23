#pragma once

#include <QPointer>
#include <QWidget>

#include "core/AntTypes.h"

class QEnterEvent;
class QEvent;
class QFocusEvent;
class QMouseEvent;
class QPaintEvent;
class QTimer;

class AntTooltip : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(Ant::TooltipPlacement placement READ placement WRITE setPlacement NOTIFY placementChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool arrowVisible READ arrowVisible WRITE setArrowVisible NOTIFY arrowVisibleChanged)
    Q_PROPERTY(int openDelay READ openDelay WRITE setOpenDelay NOTIFY openDelayChanged)

public:
    explicit AntTooltip(QWidget* parent = nullptr);
    ~AntTooltip() override;

    QString title() const;
    void setTitle(const QString& title);

    Ant::TooltipPlacement placement() const;
    void setPlacement(Ant::TooltipPlacement placement);

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
    struct Metrics
    {
        int paddingX = 12;
        int paddingY = 8;
        int radius = 6;
        int arrowSize = 8;
        int gap = 10;
        int maxWidth = 280;
    };

    Metrics metrics() const;
    QRect bubbleRect() const;
    QPolygonF arrowPolygon() const;
    QColor bubbleColor() const;
    QColor textColor() const;
    Ant::TooltipPlacement resolvedPlacement(const QRect& targetRect, const QRect& screenRect) const;
    QPoint tooltipTopLeft(const QRect& targetRect, const QSize& tooltipSize, Ant::TooltipPlacement placement) const;
    void updatePosition();
    void installTarget(QWidget* target);
    void uninstallTarget();

    QString m_title;
    Ant::TooltipPlacement m_placement = Ant::TooltipPlacement::Top;
    QColor m_color;
    bool m_arrowVisible = true;
    int m_openDelay = 120;
    QPointer<QWidget> m_target;
    QTimer* m_openTimer = nullptr;
};
