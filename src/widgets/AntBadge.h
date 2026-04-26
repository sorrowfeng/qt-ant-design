#pragma once

#include <QPoint>
#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QHideEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;
class QTimer;

class AntBadge : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool dot READ isDot WRITE setDot NOTIFY dotChanged)
    Q_PROPERTY(bool showZero READ showZero WRITE setShowZero NOTIFY showZeroChanged)
    Q_PROPERTY(int overflowCount READ overflowCount WRITE setOverflowCount NOTIFY overflowCountChanged)
    Q_PROPERTY(QPoint offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(Ant::BadgeSize badgeSize READ badgeSize WRITE setBadgeSize NOTIFY badgeSizeChanged)
    Q_PROPERTY(Ant::BadgeStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(Ant::BadgeMode badgeMode READ badgeMode WRITE setBadgeMode NOTIFY badgeModeChanged)
    Q_PROPERTY(QString ribbonText READ ribbonText WRITE setRibbonText NOTIFY ribbonTextChanged)
    Q_PROPERTY(QString ribbonColor READ ribbonColor WRITE setRibbonColor NOTIFY ribbonColorChanged)

public:
    explicit AntBadge(QWidget* parent = nullptr);
    explicit AntBadge(int count, QWidget* parent = nullptr);

    int count() const;
    void setCount(int count);

    QString text() const;
    void setText(const QString& text);

    QString color() const;
    void setColor(const QString& color);

    bool isDot() const;
    void setDot(bool dot);

    bool showZero() const;
    void setShowZero(bool showZero);

    int overflowCount() const;
    void setOverflowCount(int overflowCount);

    QPoint offset() const;
    void setOffset(const QPoint& offset);

    Ant::BadgeSize badgeSize() const;
    void setBadgeSize(Ant::BadgeSize size);

    Ant::BadgeStatus status() const;
    void setStatus(Ant::BadgeStatus status);

    Ant::BadgeMode badgeMode() const;
    void setBadgeMode(Ant::BadgeMode mode);

    QString ribbonText() const;
    void setRibbonText(const QString& text);

    QString ribbonColor() const;
    void setRibbonColor(const QString& color);

    QWidget* contentWidget() const;
    void setContentWidget(QWidget* widget);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void countChanged(int count);
    void textChanged(const QString& text);
    void colorChanged(const QString& color);
    void dotChanged(bool dot);
    void showZeroChanged(bool showZero);
    void overflowCountChanged(int overflowCount);
    void offsetChanged(const QPoint& offset);
    void badgeSizeChanged(Ant::BadgeSize size);
    void statusChanged(Ant::BadgeStatus status);
    void badgeModeChanged(Ant::BadgeMode mode);
    void ribbonTextChanged(const QString& text);
    void ribbonColorChanged(const QString& color);
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    int indicatorHeight() const;
    int dotSize() const;
    int statusDotSize() const;
    int indicatorWidth() const;
    int contentTopReserve() const;
    int contentRightReserve() const;
    QString displayText() const;
    QRect contentRect() const;
    QRectF indicatorRect() const;
    QRectF standaloneStatusDotRect() const;
    QColor badgeColor() const;
    QColor statusColor() const;
    bool shouldShowIndicator() const;
    bool isStatusMode() const;
    void updateContentGeometry();
    void updateAnimationState();
    void drawIndicator(QPainter& painter);
    void drawStatus(QPainter& painter);

    int m_count = 0;
    QString m_text;
    QString m_color;
    bool m_dot = false;
    bool m_showZero = false;
    int m_overflowCount = 99;
    QPoint m_offset;
    Ant::BadgeSize m_badgeSize = Ant::BadgeSize::Middle;
    Ant::BadgeStatus m_status = Ant::BadgeStatus::None;
    Ant::BadgeMode m_badgeMode = Ant::BadgeMode::Default;
    QString m_ribbonText;
    QString m_ribbonColor;
    QWidget* m_contentWidget = nullptr;
    QWidget* m_indicatorOverlay = nullptr;
    QTimer* m_animationTimer = nullptr;
    int m_pulse = 0;
    bool m_hovered = false;
};
