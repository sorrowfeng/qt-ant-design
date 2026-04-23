#pragma once

#include <QVariant>
#include <QWidget>

class QEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;

class AntRadio : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool autoExclusive READ autoExclusive WRITE setAutoExclusive NOTIFY autoExclusiveChanged)

public:
    explicit AntRadio(QWidget* parent = nullptr);
    explicit AntRadio(const QString& text, QWidget* parent = nullptr);

    bool isChecked() const;
    void setChecked(bool checked);

    QString text() const;
    void setText(const QString& text);

    QVariant value() const;
    void setValue(const QVariant& value);

    bool autoExclusive() const;
    void setAutoExclusive(bool autoExclusive);
    bool isHoveredState() const;
    bool isPressedState() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void checkedChanged(bool checked);
    void textChanged(const QString& text);
    void valueChanged(const QVariant& value);
    void autoExclusiveChanged(bool autoExclusive);
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
    void toggleFromUser();
    void uncheckSiblings();
    QColor indicatorBorderColor() const;
    QColor indicatorBackgroundColor() const;

    bool m_checked = false;
    bool m_autoExclusive = true;
    bool m_hovered = false;
    bool m_pressed = false;
    QString m_text;
    QVariant m_value;
};
