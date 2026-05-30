#pragma once

#include "core/QtAntDesignExport.h"

#include <QSize>
#include <QString>
#include <QVariant>
#include <QVector>

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

    int currentIndex() const;
    QString currentText() const;
    QVariant currentData() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public Q_SLOTS:
    void setCurrentIndex(int index);
    void scrollToIndex(int index);

Q_SIGNALS:
    void currentIndexChanged(int index);
    void currentTextChanged(const QString& text);
    void currentDataChanged(const QVariant& data);
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
    void emitCurrentChanged(const QString& previousText, const QVariant& previousData, int previousIndex);
    void syncNavPerfCounters() const;

    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_scrollContent = nullptr;
    QVBoxLayout* m_navLayout = nullptr;
    QVector<NavEntry> m_entries;
    int m_currentIndex = -1;
    mutable int m_selectionApplyCount = 0;
};
