#pragma once

#include <QList>
#include <QPointer>
#include <QWidget>

class QPaintEvent;
class QResizeEvent;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;

class AntListItemMeta : public QWidget
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

class AntListItem : public QWidget
{
    Q_OBJECT

public:
    explicit AntListItem(QWidget* parent = nullptr);

    void setMeta(AntListItemMeta* meta);
    AntListItemMeta* meta() const;

    void setExtraWidget(QWidget* widget);
    QWidget* extraWidget() const;

    void addActionWidget(QWidget* widget);
    QList<QWidget*> actionWidgets() const;

    void setContentWidget(QWidget* widget);
    QWidget* contentWidget() const;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QRect metaRect() const;
    QRect extraRect() const;
    QRect actionsRect() const;
    void syncLayout();

    QPointer<AntListItemMeta> m_meta;
    QPointer<QWidget> m_extra;
    QPointer<QWidget> m_content;
    QList<QPointer<QWidget>> m_actions;
};

class AntList : public QWidget
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
    void insertItem(int index, AntListItem* item);
    void removeItem(AntListItem* item);
    int itemCount() const;
    AntListItem* itemAt(int index) const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void borderedChanged(bool bordered);
    void splitChanged(bool split);
    void listSizeChanged(int size);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

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

    bool m_bordered = false;
    bool m_split = true;
    int m_listSize = Default;
    QPointer<QWidget> m_header;
    QPointer<QWidget> m_footer;
    QList<QPointer<AntListItem>> m_items;
};
