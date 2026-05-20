#pragma once

#include "core/QtAntDesignExport.h"

#include <QPointer>
#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"

class QHBoxLayout;
class QVBoxLayout;
class QBoxLayout;

class QT_ANT_DESIGN_EXPORT AntSpace : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Ant::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(Ant::Size size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(bool wrap READ isWrap WRITE setWrap NOTIFY wrapChanged)

public:
    explicit AntSpace(QWidget* parent = nullptr);

    Ant::Orientation orientation() const;
    void setOrientation(Ant::Orientation orientation);

    Ant::Size size() const;
    void setSize(Ant::Size size);

    int customSpacing() const;
    void setCustomSpacing(int spacing);

    bool isWrap() const;
    void setWrap(bool wrap);

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);

    void addItem(QWidget* widget);
    void insertItem(int index, QWidget* widget);
    void removeItem(QWidget* widget);
    int itemCount() const;
    QWidget* itemAt(int index) const;
    void clearItems();

    void setSeparator(QWidget* widget);
    QWidget* separator() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void orientationChanged(Ant::Orientation orientation);
    void sizeChanged(Ant::Size size);
    void wrapChanged(bool wrap);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    int spacingValue() const;
    Qt::Alignment childAlignment() const;
    QBoxLayout* createLayoutForOrientation();
    void clearSeparatorCopies();
    QWidget* createSeparatorCopy();
    void appendWidgetToLayout(QWidget* widget, int itemIndex);
    void invalidateSizeCache();
    void rebuildLayout();
    void syncPerfCounters() const;

    Ant::Orientation m_orientation = Ant::Orientation::Horizontal;
    Ant::Size m_size = Ant::Size::Small;
    int m_customSpacing = -1;
    bool m_wrap = false;
    Qt::Alignment m_alignment = Qt::Alignment();
    QVector<QPointer<QWidget>> m_items;
    QPointer<QWidget> m_separator;
    QVector<QPointer<QWidget>> m_separatorCopies;
    QBoxLayout* m_layout = nullptr;
    mutable QSize m_cachedSizeHint;
    mutable QSize m_cachedMinimumSizeHint;
    mutable bool m_sizeHintDirty = true;
    mutable bool m_minimumSizeHintDirty = true;
    mutable int m_rebuildCount = 0;
    mutable int m_incrementalAddCount = 0;
    mutable int m_spacingUpdateCount = 0;
    mutable int m_sizeHintComputeCount = 0;
    mutable int m_minimumSizeHintComputeCount = 0;
};
