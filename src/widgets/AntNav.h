#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QIcon>
#include <QImage>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QVariant>
#include <QVector>

#include "core/AntTypes.h"
#include "widgets/AntWidget.h"

class AntNavItem;
class QScrollArea;
class QVBoxLayout;
class QWidget;

class QT_ANT_DESIGN_EXPORT AntNav : public AntWidget
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString currentText READ currentText NOTIFY currentTextChanged)
    Q_PROPERTY(QVariant currentData READ currentData NOTIFY currentDataChanged)
    Q_PROPERTY(bool multiple READ isMultiple WRITE setMultiple NOTIFY multipleChanged)

public:
    explicit AntNav(QWidget* parent = nullptr);

    int addCategory(const QString& title);
    int addItem(const QString& text, const QVariant& data = QVariant());
    int insertItem(int index, const QString& text, const QVariant& data = QVariant());
    void removeItem(int index);
    void clear();

    int count() const;
    AntNavItem* item(int index) const;

    QString itemText(int index) const;
    void setItemText(int index, const QString& text);

    QVariant itemData(int index) const;
    void setItemData(int index, const QVariant& data);

    QIcon itemIcon(int index) const;
    void setItemIcon(int index, const QIcon& icon);
    Ant::IconType itemIconType(int index) const;
    void setItemIcon(int index, Ant::IconType iconType, Ant::IconTheme theme = Ant::IconTheme::Outlined);
    QString itemIconName(int index) const;
    void setItemIconName(int index, const QString& iconName, Ant::IconTheme theme = Ant::IconTheme::Outlined);
    Ant::IconTheme itemIconTheme(int index) const;
    void setItemIconTheme(int index, Ant::IconTheme theme);
    QColor itemIconColor(int index) const;
    void setItemIconColor(int index, const QColor& color);
    QColor itemIconTwoToneColor(int index) const;
    void setItemIconTwoToneColor(int index, const QColor& color);
    QPixmap itemIconPixmap(int index) const;
    void setItemIconPixmap(int index, const QPixmap& pixmap);
    QImage itemIconImage(int index) const;
    void setItemIconImage(int index, const QImage& image);
    QSize itemIconSize(int index) const;
    void setItemIconSize(int index, const QSize& size);
    bool itemHasIcon(int index) const;
    void clearItemIcon(int index);

    int currentIndex() const;
    QString currentText() const;
    QVariant currentData() const;
    bool isMultiple() const;
    QVector<int> selectedIndices() const;
    bool isItemSelected(int index) const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public Q_SLOTS:
    void setCurrentIndex(int index);
    void setMultiple(bool multiple);
    void setSelectedIndices(const QVector<int>& indices);
    void setItemSelected(int index, bool selected);
    void clearSelection();
    void scrollToIndex(int index);

Q_SIGNALS:
    void currentIndexChanged(int index);
    void currentTextChanged(const QString& text);
    void currentDataChanged(const QVariant& data);
    void multipleChanged(bool multiple);
    void selectionChanged(const QVector<int>& selectedIndices);
    void itemClicked(int index);
    void countChanged(int count);

private:
    struct NavEntry
    {
        AntNavItem* item = nullptr;
        QVariant data;
    };

    int normalizedInsertIndex(int index) const;
    int layoutIndexForItemInsert(int index) const;
    int indexOfItem(AntNavItem* item) const;
    void updateTheme();
    void syncActiveItemStates();
    QVector<int> normalizedSelectedIndices(const QVector<int>& indices) const;
    void applySelection(const QVector<int>& indices, int currentIndex, bool scrollCurrent = true);
    void emitCurrentChanged(const QString& previousText, const QVariant& previousData, int previousIndex);
    void syncNavPerfCounters() const;

    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_scrollContent = nullptr;
    QVBoxLayout* m_navLayout = nullptr;
    QVector<NavEntry> m_entries;
    QVector<int> m_selectedIndices;
    int m_currentIndex = -1;
    bool m_multiple = false;
    bool m_syncingItemStates = false;
    mutable int m_selectionApplyCount = 0;
};
