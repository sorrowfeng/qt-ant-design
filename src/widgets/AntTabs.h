#pragma once

#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QStackedWidget;

struct AntTabItem
{
    QString key;
    QString label;
    QString iconText;
    QWidget* page = nullptr;
    bool disabled = false;
    bool closable = true;
};

class AntTabs : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString activeKey READ activeKey WRITE setActiveKey NOTIFY activeKeyChanged)
    Q_PROPERTY(Ant::TabsType tabsType READ tabsType WRITE setTabsType NOTIFY tabsTypeChanged)
    Q_PROPERTY(Ant::Size tabsSize READ tabsSize WRITE setTabsSize NOTIFY tabsSizeChanged)
    Q_PROPERTY(Ant::TabsPlacement tabPlacement READ tabPlacement WRITE setTabPlacement NOTIFY tabPlacementChanged)
    Q_PROPERTY(bool centered READ isCentered WRITE setCentered NOTIFY centeredChanged)
    Q_PROPERTY(bool animated READ isAnimated WRITE setAnimated NOTIFY animatedChanged)
    Q_PROPERTY(bool hideAdd READ isHideAdd WRITE setHideAdd NOTIFY hideAddChanged)
    Q_PROPERTY(int tabBarGutter READ tabBarGutter WRITE setTabBarGutter NOTIFY tabBarGutterChanged)

public:
    explicit AntTabs(QWidget* parent = nullptr);

    QString activeKey() const;
    void setActiveKey(const QString& key);

    Ant::TabsType tabsType() const;
    void setTabsType(Ant::TabsType type);

    Ant::Size tabsSize() const;
    void setTabsSize(Ant::Size size);

    Ant::TabsPlacement tabPlacement() const;
    void setTabPlacement(Ant::TabsPlacement placement);

    bool isCentered() const;
    void setCentered(bool centered);

    bool isAnimated() const;
    void setAnimated(bool animated);

    bool isHideAdd() const;
    void setHideAdd(bool hide);

    int tabBarGutter() const;
    void setTabBarGutter(int gutter);

    int addTab(QWidget* page,
               const QString& key,
               const QString& label,
               const QString& iconText = QString(),
               bool disabled = false,
               bool closable = true);
    void removeTab(const QString& key);
    void clearTabs();
    void setTabText(const QString& key, const QString& label);
    void setTabEnabled(const QString& key, bool enabled);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void activeKeyChanged(const QString& key);
    void tabsTypeChanged(Ant::TabsType type);
    void tabsSizeChanged(Ant::Size size);
    void tabPlacementChanged(Ant::TabsPlacement placement);
    void centeredChanged(bool centered);
    void animatedChanged(bool animated);
    void hideAddChanged(bool hide);
    void tabBarGutterChanged(int gutter);
    void currentChanged(int index);
    void tabClicked(const QString& key);
    void tabCloseRequested(const QString& key);
    void tabAddRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    int indexOfKey(const QString& key) const;
    int activeIndex() const;
    QRect tabBarRect() const;
    QRect pageRect() const;
    QVector<QRect> tabRects() const;
    QRect addButtonRect() const;
    QRect closeRect(const QRect& tabRect) const;
    int tabAt(const QPoint& pos) const;
    int closeAt(const QPoint& pos) const;
    bool isHorizontal() const;
    int tabBarExtent() const;
    int tabPaddingX() const;
    int tabFontSize() const;
    int tabLength(const AntTabItem& item) const;
    QColor tabTextColor(const AntTabItem& item, bool active, bool hovered) const;
    QColor tabBackgroundColor(bool active, bool hovered) const;
    void setActiveIndex(int index);
    void updateStackGeometry();
    void drawTab(QPainter& painter, const AntTabItem& item, const QRect& rect, bool active, bool hovered) const;
    void drawAddButton(QPainter& painter, const QRect& rect) const;

    QVector<AntTabItem> m_tabs;
    QStackedWidget* m_stack = nullptr;
    QString m_activeKey;
    Ant::TabsType m_tabsType = Ant::TabsType::Line;
    Ant::Size m_tabsSize = Ant::Size::Middle;
    Ant::TabsPlacement m_tabPlacement = Ant::TabsPlacement::Top;
    bool m_centered = false;
    bool m_animated = true;
    bool m_hideAdd = false;
    int m_tabBarGutter = 32;
    int m_hoveredIndex = -1;
    int m_hoveredCloseIndex = -1;
    bool m_addHovered = false;
};
