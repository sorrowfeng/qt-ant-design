#pragma once

#include <QVector>
#include <QWidget>

class QEvent;
class QMouseEvent;
class QPaintEvent;

struct AntBreadcrumbItem
{
    QString title;
    QString href;
    QString iconText;
    bool disabled = false;
    bool separatorOnly = false;
    QString separator;
};

class AntBreadcrumb : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString separator READ separator WRITE setSeparator NOTIFY separatorChanged)

public:
    explicit AntBreadcrumb(QWidget* parent = nullptr);

    QString separator() const;
    void setSeparator(const QString& separator);

    void addItem(const QString& title,
                 const QString& href = QString(),
                 const QString& iconText = QString(),
                 bool disabled = false);
    void addSeparator(const QString& separator);
    void clearItems();

    int count() const;
    AntBreadcrumbItem itemAt(int index) const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void separatorChanged(const QString& separator);
    void itemClicked(int index, const QString& title, const QString& href);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QVector<QRect> itemRects() const;
    int itemIndexAt(const QPoint& pos) const;
    bool isLastRouteItem(int index) const;
    int itemWidth(const AntBreadcrumbItem& item) const;
    QColor itemColor(const AntBreadcrumbItem& item, int index, bool hovered) const;
    void updateBreadcrumbGeometry();

    QVector<AntBreadcrumbItem> m_items;
    QString m_separator = QStringLiteral("/");
    int m_hoveredIndex = -1;
};
