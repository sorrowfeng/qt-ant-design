#pragma once

#include <QList>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QPainter;
class QVariantAnimation;

struct AntMenuItem
{
    QString key;
    QString label;
    QString iconText;
    Ant::IconType iconType = Ant::IconType::None;
    QString extra;
    QString parentKey;
    int level = 0;
    bool disabled = false;
    bool danger = false;
    bool divider = false;
    bool subMenu = false;
};

class AntMenu : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Ant::MenuMode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(Ant::MenuTheme menuTheme READ menuTheme WRITE setMenuTheme NOTIFY menuThemeChanged)
    Q_PROPERTY(bool inlineCollapsed READ isInlineCollapsed WRITE setInlineCollapsed NOTIFY inlineCollapsedChanged)
    Q_PROPERTY(bool selectable READ isSelectable WRITE setSelectable NOTIFY selectableChanged)
    Q_PROPERTY(QString selectedKey READ selectedKey WRITE setSelectedKey NOTIFY selectedKeyChanged)
    Q_PROPERTY(QStringList openKeys READ openKeys WRITE setOpenKeys NOTIFY openKeysChanged)
    Q_PROPERTY(int inlineIndent READ inlineIndent WRITE setInlineIndent NOTIFY inlineIndentChanged)

public:
    explicit AntMenu(QWidget* parent = nullptr);

    Ant::MenuMode mode() const;
    void setMode(Ant::MenuMode mode);

    Ant::MenuTheme menuTheme() const;
    void setMenuTheme(Ant::MenuTheme theme);

    bool isInlineCollapsed() const;
    void setInlineCollapsed(bool collapsed);

    bool isSelectable() const;
    void setSelectable(bool selectable);

    bool isCompact() const;
    void setCompact(bool compact);

    QString selectedKey() const;
    void setSelectedKey(const QString& key);

    QStringList openKeys() const;
    void setOpenKeys(const QStringList& keys);

    int inlineIndent() const;
    void setInlineIndent(int indent);

    void addItem(const QString& key,
                 const QString& label,
                 const QString& iconText = QString(),
                 const QString& extra = QString(),
                 bool disabled = false,
                 bool danger = false);
    void addItem(const QString& key,
                 const QString& label,
                 Ant::IconType iconType,
                 const QString& extra = QString(),
                 bool disabled = false,
                 bool danger = false);
    void addSubMenu(const QString& key,
                    const QString& label,
                    const QString& iconText = QString(),
                    bool disabled = false);
    void addSubMenu(const QString& key,
                    const QString& label,
                    Ant::IconType iconType,
                    bool disabled = false);
    void addSubItem(const QString& parentKey,
                    const QString& key,
                    const QString& label,
                    const QString& iconText = QString(),
                    const QString& extra = QString(),
                    bool disabled = false,
                    bool danger = false);
    void addSubItem(const QString& parentKey,
                    const QString& key,
                    const QString& label,
                    Ant::IconType iconType,
                    const QString& extra = QString(),
                    bool disabled = false,
                    bool danger = false);
    void addDivider();
    void clearItems();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void modeChanged(Ant::MenuMode mode);
    void menuThemeChanged(Ant::MenuTheme theme);
    void inlineCollapsedChanged(bool collapsed);
    void selectableChanged(bool selectable);
    void selectedKeyChanged(const QString& key);
    void openKeysChanged(const QStringList& keys);
    void inlineIndentChanged(int indent);
    void itemClicked(const QString& key);
    void itemSelected(const QString& key);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    struct VisibleItem
    {
        int index = -1;
        QRect rect;
    };

    QList<VisibleItem> visibleItems() const;
    int itemAt(const QPoint& pos) const;
    int selectedVisibleIndex() const;
    int nextSelectableVisibleIndex(int from, int direction) const;
    bool isOpen(const QString& key) const;
    bool isSubMenuVisible(const QString& key) const;
    qreal subMenuProgress(const QString& key) const;
    void toggleOpen(const QString& key);
    void animateSubMenu(const QString& key, bool open);
    void stopSubMenuAnimation(const QString& key);
    void activateItem(int itemIndex);
    int itemHeight() const;
    int horizontalItemWidth(const AntMenuItem& item) const;
    QRect itemContentRect(const QRect& rect, const AntMenuItem& item) const;
    QColor menuBackgroundColor() const;
    QColor itemTextColor(const AntMenuItem& item, bool selected, bool hovered) const;
    QColor itemBackgroundColor(const AntMenuItem& item, bool selected, bool hovered) const;
    void drawItem(QPainter& painter, const AntMenuItem& item, const QRect& rect, bool selected, bool hovered) const;
    void updateMenuGeometry();

    QVector<AntMenuItem> m_items;
    Ant::MenuMode m_mode = Ant::MenuMode::Vertical;
    Ant::MenuTheme m_menuTheme = Ant::MenuTheme::Light;
    bool m_inlineCollapsed = false;
    bool m_selectable = true;
    bool m_compact = false;
    QString m_selectedKey;
    QStringList m_openKeys;
    QHash<QString, qreal> m_subMenuProgress;
    QHash<QString, QVariantAnimation*> m_subMenuAnimations;
    int m_inlineIndent = 24;
    int m_hoveredIndex = -1;
};
