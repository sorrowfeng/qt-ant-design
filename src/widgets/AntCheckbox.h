#pragma once

#include <QWidget>

class QEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;

class AntCheckbox : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(bool indeterminate READ isIndeterminate WRITE setIndeterminate NOTIFY indeterminateChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit AntCheckbox(QWidget* parent = nullptr);
    explicit AntCheckbox(const QString& text, QWidget* parent = nullptr);

    bool isChecked() const;
    void setChecked(bool checked);

    bool isIndeterminate() const;
    void setIndeterminate(bool indeterminate);

    QString text() const;
    void setText(const QString& text);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void checkedChanged(bool checked);
    void indeterminateChanged(bool indeterminate);
    void textChanged(const QString& text);
    void stateChanged(int state);
    void toggled(bool checked);
    void clicked(bool checked);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QRectF indicatorRect() const;
    void toggle();
    QColor indicatorBorderColor() const;
    QColor indicatorBackgroundColor() const;

    bool m_checked = false;
    bool m_indeterminate = false;
    bool m_hovered = false;
    bool m_pressed = false;
    QString m_text;
};
