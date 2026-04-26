#pragma once

#include <QWidget>

class QLabel;

class AntNavItem : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit AntNavItem(const QString& text, QWidget* parent = nullptr);

    bool isActive() const;
    void setActive(bool active);

    QString text() const;
    void setText(const QString& text);

Q_SIGNALS:
    void activeChanged(bool active);
    void textChanged(const QString& text);
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QLabel* m_label = nullptr;
    bool m_active = false;
    bool m_hovered = false;
};
