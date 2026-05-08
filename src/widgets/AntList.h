#pragma once

#include "core/QtAntDesignExport.h"

#include <QAbstractItemView>
#include <QHash>
#include <QIcon>
#include <QList>
#include <QPointer>
#include <QStringList>
#include <QVariant>
#include <QWidget>

class QKeyEvent;
class QPaintEvent;
class QMouseEvent;
class QResizeEvent;
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
    void dataChanged(int role, const QVariant& value);
    void checkStateChanged(Qt::CheckState state);
    void flagsChanged(Qt::ItemFlags flags);
    void selectedChanged(bool selected);
    void changed();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QRect metaRect() const;
    QRect extraRect() const;
    QRect actionsRect() const;
    int actionsWidth() const;
    void syncLayout();

    QPointer<AntListItemMeta> m_meta;
    QPointer<QWidget> m_extra;
    QPointer<QWidget> m_content;
    QList<QPointer<QWidget>> m_actions;
    QString m_text;
    QIcon m_icon;
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

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

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
    void syncLayout();
    void adoptItem(AntListItem* item);
    void detachItem(AntListItem* item);
    void handleItemChanged(AntListItem* item);
    void setCurrentItemInternal(AntListItem* item);
    bool setItemSelectedInternal(AntListItem* item, bool selected, bool clearOthers);
    bool clearSelectionExcept(AntListItem* keepItem);
    AntListItem* enabledSelectableItem(int startRow, int direction) const;

    bool m_bordered = false;
    bool m_split = true;
    int m_listSize = Default;
    QPointer<QWidget> m_header;
    QPointer<QWidget> m_footer;
    QList<QPointer<AntListItem>> m_items;
    QPointer<AntListItem> m_currentItem;
    QAbstractItemView::SelectionMode m_selectionMode = QAbstractItemView::SingleSelection;
};
