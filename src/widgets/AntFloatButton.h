#pragma once

#include <QWidget>
#include <QTimer>
#include <QVector>

#include "core/AntTypes.h"

class AntFloatButton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
    Q_PROPERTY(Ant::FloatButtonType floatButtonType READ floatButtonType WRITE setFloatButtonType NOTIFY floatButtonTypeChanged)
    Q_PROPERTY(Ant::FloatButtonShape floatButtonShape READ floatButtonShape WRITE setFloatButtonShape NOTIFY floatButtonShapeChanged)
    Q_PROPERTY(Ant::FloatButtonPlacement placement READ placement WRITE setPlacement NOTIFY placementChanged)
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)

public:
    explicit AntFloatButton(QWidget* parent = nullptr);

    // Main button
    QString icon() const;
    void setIcon(const QString& icon);
    QString content() const;
    void setContent(const QString& content);
    Ant::FloatButtonType floatButtonType() const;
    void setFloatButtonType(Ant::FloatButtonType type);
    Ant::FloatButtonShape floatButtonShape() const;
    void setFloatButtonShape(Ant::FloatButtonShape shape);

    // Badge
    bool badgeDot() const;
    void setBadgeDot(bool dot);
    int badgeCount() const;
    void setBadgeCount(int count);

    // Group mode
    void addChild(AntFloatButton* child);
    void removeChild(AntFloatButton* child);
    QVector<AntFloatButton*> childButtons() const;
    Ant::Trigger groupTrigger() const;
    void setGroupTrigger(Ant::Trigger trigger);
    bool isOpen() const;
    void setOpen(bool open);
    QString closeIcon() const;
    void setCloseIcon(const QString& icon);

    // BackTop mode
    bool isBackTop() const;
    void setBackTop(bool backTop);
    int visibilityHeight() const;
    void setVisibilityHeight(int height);
    int scrollDuration() const;
    void setScrollDuration(int duration);
    QWidget* scrollTarget() const;
    void setScrollTarget(QWidget* target);

    // Placement
    Ant::FloatButtonPlacement placement() const;
    void setPlacement(Ant::FloatButtonPlacement placement);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void iconChanged(const QString& icon);
    void contentChanged(const QString& content);
    void floatButtonTypeChanged(Ant::FloatButtonType type);
    void floatButtonShapeChanged(Ant::FloatButtonShape shape);
    void placementChanged(Ant::FloatButtonPlacement placement);
    void openChanged(bool open);
    void clicked();
    void backTopClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void updatePosition();
    void layoutChildren();
    void checkBackTopVisibility();
    void animateScrollToTop();

    // Main props
    QString m_icon;
    QString m_content;
    Ant::FloatButtonType m_type = Ant::FloatButtonType::Default;
    Ant::FloatButtonShape m_shape = Ant::FloatButtonShape::Circle;

    // Badge
    bool m_badgeDot = false;
    int m_badgeCount = 0;

    // Group
    QVector<AntFloatButton*> m_children;
    Ant::Trigger m_groupTrigger = Ant::Trigger::Click;
    bool m_open = false;
    QString m_closeIcon;

    // BackTop
    bool m_backTop = false;
    int m_visibilityHeight = 400;
    int m_scrollDuration = 450;
    QWidget* m_scrollTarget = nullptr;

    // State
    Ant::FloatButtonPlacement m_placement = Ant::FloatButtonPlacement::BottomRight;
    bool m_hovered = false;
    bool m_pressed = false;
    QTimer* m_positionTimer = nullptr;
};
