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
    Q_PROPERTY(Qt::CheckState checkState READ checkState WRITE setCheckState NOTIFY checkStateChanged)
    Q_PROPERTY(bool indeterminate READ isIndeterminate WRITE setIndeterminate NOTIFY indeterminateChanged)
    Q_PROPERTY(bool tristate READ isTristate WRITE setTristate NOTIFY tristateChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit AntCheckbox(QWidget* parent = nullptr);
    explicit AntCheckbox(const QString& text, QWidget* parent = nullptr);

    bool isChecked() const;
    void setChecked(bool checked);
    Qt::CheckState checkState() const;
    void setCheckState(Qt::CheckState state);

    bool isIndeterminate() const;
    void setIndeterminate(bool indeterminate);
    bool isTristate() const;
    void setTristate(bool tristate);
    void toggle();
    void click();

    QString text() const;
    void setText(const QString& text);
    bool isHoveredState() const;
    bool isPressedState() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void checkedChanged(bool checked);
    void checkStateChanged(Qt::CheckState state);
    void indeterminateChanged(bool indeterminate);
    void tristateChanged(bool tristate);
    void textChanged(const QString& text);
    void stateChanged(int state);
    void toggled(bool checked);
    void clicked(bool checked);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QRectF indicatorRect() const;
    QColor indicatorBorderColor() const;
    QColor indicatorBackgroundColor() const;

    bool m_checked = false;
    bool m_indeterminate = false;
    bool m_tristate = false;
    bool m_hovered = false;
    bool m_pressed = false;
    QString m_text;
};
