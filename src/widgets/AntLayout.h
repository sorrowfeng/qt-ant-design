#pragma once

#include "core/QtAntDesignExport.h"

#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QMouseEvent;

class QT_ANT_DESIGN_EXPORT AntLayoutHeader : public QWidget
{
    Q_OBJECT
public:
    explicit AntLayoutHeader(QWidget* parent = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
};

class QT_ANT_DESIGN_EXPORT AntLayoutFooter : public QWidget
{
    Q_OBJECT
public:
    explicit AntLayoutFooter(QWidget* parent = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
};

class QT_ANT_DESIGN_EXPORT AntLayoutContent : public QWidget
{
    Q_OBJECT
public:
    explicit AntLayoutContent(QWidget* parent = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
};

class QT_ANT_DESIGN_EXPORT AntLayoutSider : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool collapsed READ isCollapsed WRITE setCollapsed NOTIFY collapsedChanged)
    Q_PROPERTY(bool collapsible READ isCollapsible WRITE setCollapsible NOTIFY collapsibleChanged)
    Q_PROPERTY(int width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(int collapsedWidth READ collapsedWidth WRITE setCollapsedWidth NOTIFY collapsedWidthChanged)
    Q_PROPERTY(Ant::LayoutSiderTheme siderTheme READ siderTheme WRITE setSiderTheme NOTIFY siderThemeChanged)
    Q_PROPERTY(bool reverseArrow READ isReverseArrow WRITE setReverseArrow NOTIFY reverseArrowChanged)

public:
    explicit AntLayoutSider(QWidget* parent = nullptr);

    bool isCollapsed() const;
    void setCollapsed(bool collapsed);

    bool isCollapsible() const;
    void setCollapsible(bool collapsible);

    int siderWidth() const;
    void setWidth(int width);

    int collapsedWidth() const;
    void setCollapsedWidth(int width);

    Ant::LayoutSiderTheme siderTheme() const;
    void setSiderTheme(Ant::LayoutSiderTheme theme);

    bool isReverseArrow() const;
    void setReverseArrow(bool reverse);

    void setTriggerWidget(QWidget* widget);
    QWidget* triggerWidget() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void collapsedChanged(bool collapsed);
    void collapsibleChanged(bool collapsible);
    void widthChanged(int width);
    void collapsedWidthChanged(int width);
    void siderThemeChanged(Ant::LayoutSiderTheme theme);
    void reverseArrowChanged(bool reverse);
    void collapseTriggered(bool collapsed);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QRect triggerRect() const;
    void syncTriggerGeometry();

    bool m_collapsed = false;
    bool m_collapsible = false;
    int m_width = 200;
    int m_collapsedWidth = 80;
    Ant::LayoutSiderTheme m_siderTheme = Ant::LayoutSiderTheme::Dark;
    bool m_reverseArrow = false;
    bool m_triggerHovered = false;
    QWidget* m_triggerWidget = nullptr;
};

class QT_ANT_DESIGN_EXPORT AntLayout : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool hasSider READ hasSider NOTIFY hasSiderChanged)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius NOTIFY borderRadiusChanged)

public:
    explicit AntLayout(QWidget* parent = nullptr);

    bool hasSider() const;

    int borderRadius() const;
    void setBorderRadius(int radius);

    void setHeader(AntLayoutHeader* header);
    AntLayoutHeader* header() const;

    void setFooter(AntLayoutFooter* footer);
    AntLayoutFooter* footer() const;

    void setContent(AntLayoutContent* content);
    AntLayoutContent* content() const;

    void addSider(AntLayoutSider* sider);
    void removeSider(AntLayoutSider* sider);
    int siderCount() const;
    AntLayoutSider* siderAt(int index) const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void hasSiderChanged(bool hasSider);
    void borderRadiusChanged(int radius);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void syncLayout();
    void syncMask();
    void updateHasSider();

    QWidget* m_header = nullptr;
    QWidget* m_footer = nullptr;
    QWidget* m_content = nullptr;
    QVector<QWidget*> m_siders;
    bool m_hasSider = false;
    int m_borderRadius = 0;
};
