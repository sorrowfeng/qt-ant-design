#pragma once

#include "core/QtAntDesignExport.h"

#include <QIcon>
#include <QRectF>
#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"

class QAction;
class QLabel;
class QEvent;
class QFrame;
class QHBoxLayout;
class QMouseEvent;
class QPaintEvent;
class QPropertyAnimation;
class QResizeEvent;
class QScrollArea;
class QStackedWidget;
class QVBoxLayout;

class QT_ANT_DESIGN_EXPORT AntRibbonGroup : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

public:
    explicit AntRibbonGroup(QWidget* parent = nullptr);
    explicit AntRibbonGroup(const QString& title, QWidget* parent = nullptr);

    QString title() const;
    void setTitle(const QString& title);

    void addLargeAction(QAction* action);
    void addSmallAction(QAction* action);
    QAction* addAction(const QString& text,
                       const QIcon& icon = QIcon(),
                       Ant::RibbonItemSize size = Ant::RibbonItemSize::Large);
    QAction* addWidget(QWidget* widget, Ant::RibbonItemSize size = Ant::RibbonItemSize::Large);
    void clearItems();
    int itemCount() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void titleChanged(const QString& title);
    void actionTriggered(QAction* action);

protected:
    void paintEvent(QPaintEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void init();
    void addActionInternal(QAction* action, Ant::RibbonItemSize size);
    void addItemWidget(QWidget* widget, Ant::RibbonItemSize size);
    QVBoxLayout* ensureSmallColumn();
    void clearSmallColumns();
    void syncTheme();

    QString m_title;
    QWidget* m_content = nullptr;
    QLabel* m_titleLabel = nullptr;
    QVBoxLayout* m_rootLayout = nullptr;
    QHBoxLayout* m_contentLayout = nullptr;
    QVector<QWidget*> m_itemWidgets;
    QVector<QWidget*> m_smallColumns;
    int m_itemCount = 0;
};

class QT_ANT_DESIGN_EXPORT AntRibbonPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged)

public:
    explicit AntRibbonPage(QWidget* parent = nullptr);
    explicit AntRibbonPage(const QString& title, const QString& key = QString(), QWidget* parent = nullptr);

    QString title() const;
    void setTitle(const QString& title);

    QString key() const;
    void setKey(const QString& key);

    AntRibbonGroup* addGroup(const QString& title);
    AntRibbonGroup* insertGroup(int index, const QString& title);
    void removeGroup(int index);
    void clearGroups();
    int groupCount() const;
    AntRibbonGroup* groupAt(int index) const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void titleChanged(const QString& title);
    void keyChanged(const QString& key);
    void actionTriggered(QAction* action);

private:
    void init();
    void connectGroup(AntRibbonGroup* group);

    QString m_title;
    QString m_key;
    QHBoxLayout* m_layout = nullptr;
    QVector<AntRibbonGroup*> m_groups;
};

class QT_ANT_DESIGN_EXPORT AntRibbon : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool collapsed READ isCollapsed WRITE setCollapsed NOTIFY collapsedChanged)
    Q_PROPERTY(bool collapseButtonVisible READ isCollapseButtonVisible WRITE setCollapseButtonVisible NOTIFY collapseButtonVisibleChanged)
    Q_PROPERTY(int currentPageIndex READ currentPageIndex WRITE setCurrentPageIndex NOTIFY currentPageChanged)
    Q_PROPERTY(QString currentPageKey READ currentPageKey WRITE setCurrentPageKey NOTIFY currentPageKeyChanged)
    Q_PROPERTY(QRectF indicatorRect READ indicatorRect WRITE setIndicatorRect)
    Q_PROPERTY(qreal contentHeight READ contentHeight WRITE setContentHeight)

public:
    explicit AntRibbon(QWidget* parent = nullptr);

    AntRibbonPage* addPage(const QString& title, const QString& key = QString());
    AntRibbonPage* insertPage(int index, const QString& title, const QString& key = QString());
    void removePage(const QString& key);
    void clearPages();
    int pageCount() const;
    AntRibbonPage* pageAt(int index) const;
    AntRibbonPage* pageByKey(const QString& key) const;

    int currentPageIndex() const;
    void setCurrentPageIndex(int index);
    QString currentPageKey() const;
    void setCurrentPageKey(const QString& key);

    bool isCollapsed() const;
    void setCollapsed(bool collapsed);
    bool isCollapseButtonVisible() const;
    void setCollapseButtonVisible(bool visible);
    QRectF indicatorRect() const;
    void setIndicatorRect(const QRectF& rect);
    qreal contentHeight() const;
    void setContentHeight(qreal height);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void currentPageChanged(int index);
    void currentPageKeyChanged(const QString& key);
    void pageClicked(const QString& key);
    void collapsedChanged(bool collapsed);
    void collapseButtonVisibleChanged(bool visible);
    void actionTriggered(QAction* action);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    static constexpr int TabBarHeight = 42;
    static constexpr int ContentHeight = 148;
    static constexpr int CollapseButtonWidth = 36;

    QString normalizedKey(const QString& title, const QString& key) const;
    QVector<QRect> tabRects() const;
    QRect collapseButtonRect() const;
    int tabAt(const QPoint& pos) const;
    QRectF targetIndicatorRect(int index) const;
    void syncIndicatorRect();
    void animateIndicator(const QRectF& from, const QRectF& to);
    void updatePageVisibility();
    void updateStackSize();
    void updateScrollAreaGeometry();
    void showCurrentPagePopup();
    void hidePopup();
    void restoreScrollAreaParent();
    void connectPage(AntRibbonPage* page);
    void syncTheme();

    QVector<AntRibbonPage*> m_pages;
    QScrollArea* m_scrollArea = nullptr;
    QStackedWidget* m_stack = nullptr;
    QFrame* m_popup = nullptr;
    bool m_scrollAreaInPopup = false;
    bool m_collapsed = false;
    bool m_collapseButtonVisible = true;
    int m_currentPageIndex = -1;
    int m_hoveredTab = -1;
    bool m_collapseHovered = false;
    QRectF m_indicatorRect;
    bool m_indicatorReady = false;
    qreal m_contentHeight = ContentHeight;
    QPropertyAnimation* m_indicatorAnimation = nullptr;
    QPropertyAnimation* m_contentAnimation = nullptr;
};
