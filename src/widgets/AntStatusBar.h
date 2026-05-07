#pragma once

#include <QVector>
#include <QWidget>

class QEvent;
class QMouseEvent;
class QPaintEvent;
class QTimer;

struct AntStatusBarItem
{
    QString text;
    QString icon;
    QString tooltip;
    int stretch = 0;
};

class AntStatusBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged)
    Q_PROPERTY(bool sizeGrip READ hasSizeGrip WRITE setSizeGrip NOTIFY sizeGripChanged)

public:
    explicit AntStatusBar(QWidget* parent = nullptr);

    QString message() const;
    void setMessage(const QString& message);
    QString currentMessage() const;
    void showMessage(const QString& message, int timeout = 0);
    void clearMessage();

    bool hasSizeGrip() const;
    void setSizeGrip(bool enabled);

    int addItem(const QString& text,
                const QString& icon = QString(),
                const QString& tooltip = QString(),
                int stretch = 0);
    int addPermanentItem(const QString& text,
                         const QString& icon = QString(),
                         const QString& tooltip = QString(),
                         int stretch = 0);
    void removeItem(int index);

    int itemCount() const;
    int permanentItemCount() const;
    AntStatusBarItem itemAt(int index) const;
    AntStatusBarItem permanentItemAt(int index) const;

    int hoveredRegularIndex() const;
    int hoveredPermanentIndex() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void messageChanged(const QString& message);
    void sizeGripChanged(bool enabled);
    void itemClicked(int index);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    int regularItemIndexAt(const QPoint& pos) const;
    int permanentItemIndexAt(const QPoint& pos) const;

    QVector<AntStatusBarItem> m_items;
    QVector<AntStatusBarItem> m_permanentItems;
    QString m_message;
    bool m_sizeGrip = true;
    int m_hoveredRegularIndex = -1;
    int m_hoveredPermanentIndex = -1;
    QTimer* m_messageTimer = nullptr;
};
