#pragma once

#include <QRectF>
#include <QWidget>

class QScrollArea;
class QVariantAnimation;
class QVBoxLayout;

class AntAnchor : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int activeIndex READ activeIndex NOTIFY activeIndexChanged)

public:
    explicit AntAnchor(QWidget* parent = nullptr);

    void setScrollArea(QScrollArea* area);
    int activeIndex() const;

    void addLink(const QString& title, int targetY);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent*) override;

Q_SIGNALS:
    void activeIndexChanged(int index);
    void linkClicked(int index, int targetY);

private:
    void setActiveIndexInternal(int index, bool animate);
    QRectF indicatorRectForIndex(int index) const;

    QScrollArea* m_scrollArea = nullptr;
    QVBoxLayout* m_layout = nullptr;
    QVariantAnimation* m_indicatorAnimation = nullptr;
    QRectF m_indicatorRect;
    int m_activeIndex = -1;
    struct Link { QString title; int targetY = 0; };
    QList<Link> m_links;
};
