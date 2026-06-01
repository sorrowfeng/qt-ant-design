#pragma once

#include "core/QtAntDesignExport.h"

#include <QAbstractItemView>
#include <QColor>
#include <QHash>
#include <QIcon>
#include <QImage>
#include <QList>
#include <QPixmap>
#include <QPointer>
#include <QSize>
#include <QStringList>
#include <QVariant>
#include <QWidget>

#include "AntMenu.h"
#include "core/AntTypes.h"

class AntIcon;
class QContextMenuEvent;
class QEvent;
class QKeyEvent;
class QPaintEvent;
class QPainter;
class QMouseEvent;
class QResizeEvent;
class QWheelEvent;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;

class QT_ANT_DESIGN_EXPORT AntListItemMeta : public QWidget
{
    Q_OBJECT

public:
    explicit AntListItemMeta(QWidget* parent = nullptr);

    void setAvatar(QWidget* widget);
    QWidget* avatar() const;

    void setTitle(const QString& title);
    QString title() const;

    void setDescription(const QString& description);
    QString description() const;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QRect avatarRect() const;
    QRect textRect() const;
    void syncAvatarGeometry();

    QPointer<QWidget> m_avatar;
    QString m_title;
    QString m_description;
};

class QT_ANT_DESIGN_EXPORT AntListItem : public QWidget
{
    Q_OBJECT

public:
    explicit AntListItem(QWidget* parent = nullptr);

    void setText(const QString& text);
    QString text() const;

    void setIcon(const QIcon& icon);
    QIcon icon() const;
    void setIcon(Ant::IconType iconType, Ant::IconTheme theme = Ant::IconTheme::Outlined);
    Ant::IconType iconType() const;
    void setIconName(const QString& iconName, Ant::IconTheme theme = Ant::IconTheme::Outlined);
    QString iconName() const;
    void setIconTheme(Ant::IconTheme theme);
    Ant::IconTheme iconTheme() const;
    void setIconColor(const QColor& color);
    QColor iconColor() const;
    void setIconTwoToneColor(const QColor& color);
    QColor iconTwoToneColor() const;
    void setIconPixmap(const QPixmap& pixmap);
    QPixmap iconPixmap() const;
    void setIconImage(const QImage& image);
    QImage iconImage() const;
    void setIconSize(const QSize& size);
    QSize iconSize() const;
    bool hasIcon() const;
    void clearIcon();

    void setData(int role, const QVariant& value);
    QVariant data(int role) const;

    void setCheckState(Qt::CheckState state);
    Qt::CheckState checkState() const;

    void setFlags(Qt::ItemFlags flags);
    Qt::ItemFlags flags() const;

    void setSelected(bool selected);
    bool isSelected() const;

    void setMeta(AntListItemMeta* meta);
    AntListItemMeta* meta() const;

    void setExtraWidget(QWidget* widget);
    QWidget* extraWidget() const;

    void addActionWidget(QWidget* widget);
    QList<QWidget*> actionWidgets() const;

    void setContentWidget(QWidget* widget);
    QWidget* contentWidget() const;

    QSize sizeHint() const override;

Q_SIGNALS:
    void textChanged(const QString& text);
    void iconChanged();
    void iconSizeChanged(const QSize& size);
    void dataChanged(int role, const QVariant& value);
    void checkStateChanged(Qt::CheckState state);
    void flagsChanged(Qt::ItemFlags flags);
    void selectedChanged(bool selected);
    void changed();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    enum class IconSource
    {
        None,
        QtIcon,
        AntIconType,
        AntIconName,
        Pixmap,
    };

    QRect metaRect() const;
    QRect extraRect() const;
    QRect actionsRect() const;
    QRect leadingIconRect() const;
    int actionsWidth() const;
    void syncLayout();
    void syncAntIconWidget();
    bool usesAntIconWidget() const;
    bool usesPaintedIcon() const;
    QSize effectiveIconSize() const;
    void drawPaintedIcon(QPainter* painter, const QRect& iconRect) const;

    QPointer<AntListItemMeta> m_meta;
    QPointer<QWidget> m_extra;
    QPointer<QWidget> m_content;
    QList<QPointer<QWidget>> m_actions;
    QString m_text;
    QIcon m_icon;
    IconSource m_iconSource = IconSource::None;
    Ant::IconType m_iconType = Ant::IconType::None;
    QString m_iconName;
    Ant::IconTheme m_iconTheme = Ant::IconTheme::Outlined;
    QColor m_iconColor;
    QColor m_iconTwoToneColor;
    QPixmap m_iconPixmap;
    QSize m_iconSize = QSize(16, 16);
    AntIcon* m_antIconWidget = nullptr;
    QHash<int, QVariant> m_roleData;
    Qt::CheckState m_checkState = Qt::Unchecked;
    Qt::ItemFlags m_flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    bool m_selected = false;
};

class QT_ANT_DESIGN_EXPORT AntList : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool bordered READ isBordered WRITE setBordered NOTIFY borderedChanged)
    Q_PROPERTY(bool split READ isSplit WRITE setSplit NOTIFY splitChanged)
    Q_PROPERTY(int listSize READ listSize WRITE setListSize NOTIFY listSizeChanged)

public:
    enum ListSize
    {
        Small,
        Default,
        Large,
    };
    Q_ENUM(ListSize)

    explicit AntList(QWidget* parent = nullptr);

    bool isBordered() const;
    void setBordered(bool bordered);

    bool isSplit() const;
    void setSplit(bool split);

    int listSize() const;
    void setListSize(int size);

    void setHeaderWidget(QWidget* widget);
    QWidget* headerWidget() const;

    void setFooterWidget(QWidget* widget);
    QWidget* footerWidget() const;

    void addItem(AntListItem* item);
    void addItem(const QString& text);
    void addItems(const QStringList& labels);
    void insertItem(int index, AntListItem* item);
    void insertItem(int index, const QString& text);
    void insertItems(int index, const QStringList& labels);
    void removeItem(AntListItem* item);
    int itemCount() const;
    int count() const;
    bool isEmpty() const;
    AntListItem* itemAt(int index) const;
    AntListItem* item(int index) const;
    AntListItem* itemAt(const QPoint& pos) const;
    QRect visualItemRect(AntListItem* item) const;
    int row(const AntListItem* item) const;
    QList<AntListItem*> findItems(const QString& text, Qt::MatchFlags flags) const;
    void sortItems(Qt::SortOrder order = Qt::AscendingOrder);
    AntListItem* takeItem(int index);
    void clearItems();
    void clear();

    AntListItem* currentItem() const;
    int currentRow() const;
    void setCurrentItem(AntListItem* item);
    void setCurrentRow(int row);

    QAbstractItemView::SelectionMode selectionMode() const;
    void setSelectionMode(QAbstractItemView::SelectionMode mode);
    QList<AntListItem*> selectedItems() const;
    void setItemSelected(AntListItem* item, bool selected);
    int verticalScrollOffset() const;
    int maximumScrollOffset() const;
    void setVerticalScrollOffset(int offset);
    void scrollToItem(AntListItem* item);

    void setContextMenu(AntMenu* menu);
    AntMenu* contextMenu() const;
    void clearContextMenu();
    void setItemContextMenu(AntListItem* item, AntMenu* menu);
    AntMenu* itemContextMenu(AntListItem* item) const;
    void clearItemContextMenu(AntListItem* item);
    AntListItem* contextMenuItem() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void borderedChanged(bool bordered);
    void splitChanged(bool split);
    void listSizeChanged(int size);
    void itemClicked(AntListItem* item);
    void itemDoubleClicked(AntListItem* item);
    void itemActivated(AntListItem* item);
    void itemChanged(AntListItem* item);
    void currentItemChanged(AntListItem* current, AntListItem* previous);
    void currentRowChanged(int currentRow);
    void itemSelectionChanged();
    void contextMenuRequested(AntListItem* item, const QPoint& globalPos);
    void contextMenuAboutToShow(AntMenu* menu, AntListItem* item);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    struct Metrics
    {
        int padding = 16;
        int itemPaddingH = 16;
        int itemPaddingV = 12;
        int fontSize = 14;
        int headerHeight = 48;
        int footerHeight = 48;
        int radius = 8;
    };

    Metrics metrics() const;
    QRect headerRect() const;
    QRect footerRect() const;
    QRect contentRect() const;
    int itemsHeight() const;
    void syncLayout();
    void adoptItem(AntListItem* item);
    void detachItem(AntListItem* item);
    void handleItemChanged(AntListItem* item);
    void setCurrentItemInternal(AntListItem* item);
    bool setItemSelectedInternal(AntListItem* item, bool selected, bool clearOthers);
    bool clearSelectionExcept(AntListItem* keepItem);
    AntListItem* enabledSelectableItem(int startRow, int direction) const;
    AntMenu* menuForContextItem(AntListItem* item) const;
    bool showContextMenuForPosition(const QPoint& pos, const QPoint& globalPos);
    AntListItem* itemForContextObject(QObject* object) const;
    void installContextMenuFilters(QObject* object);
    void uninstallContextMenuFilters(QObject* object);

    bool m_bordered = false;
    bool m_split = true;
    int m_listSize = Default;
    QPointer<QWidget> m_header;
    QPointer<QWidget> m_footer;
    QList<QPointer<AntListItem>> m_items;
    QPointer<AntListItem> m_currentItem;
    QAbstractItemView::SelectionMode m_selectionMode = QAbstractItemView::SingleSelection;
    int m_verticalScrollOffset = 0;
    QPointer<AntMenu> m_contextMenu;
    QHash<AntListItem*, QPointer<AntMenu>> m_itemContextMenus;
    QPointer<AntListItem> m_contextMenuItem;
    QPointer<QWidget> m_contextMenuPopup;
};
