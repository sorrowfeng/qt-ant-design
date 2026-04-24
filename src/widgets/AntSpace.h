#pragma once

#include <QPointer>
#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"

class QHBoxLayout;
class QVBoxLayout;
class QBoxLayout;

class AntSpace : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Ant::SpaceOrientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(Ant::SpaceSize size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(bool wrap READ isWrap WRITE setWrap NOTIFY wrapChanged)

public:
    explicit AntSpace(QWidget* parent = nullptr);

    Ant::SpaceOrientation orientation() const;
    void setOrientation(Ant::SpaceOrientation orientation);

    Ant::SpaceSize size() const;
    void setSize(Ant::SpaceSize size);

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
    void orientationChanged(Ant::SpaceOrientation orientation);
    void sizeChanged(Ant::SpaceSize size);
    void wrapChanged(bool wrap);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    int spacingValue() const;
    void rebuildLayout();

    Ant::SpaceOrientation m_orientation = Ant::SpaceOrientation::Horizontal;
    Ant::SpaceSize m_size = Ant::SpaceSize::Small;
    int m_customSpacing = -1;
    bool m_wrap = false;
    Qt::Alignment m_alignment = Qt::Alignment();
    QVector<QPointer<QWidget>> m_items;
    QPointer<QWidget> m_separator;
    QBoxLayout* m_layout = nullptr;
};
