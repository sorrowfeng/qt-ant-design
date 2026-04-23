#pragma once

#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QMouseEvent;
class QPaintEvent;

class AntTag : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString iconText READ iconText WRITE setIconText NOTIFY iconTextChanged)
    Q_PROPERTY(bool closable READ isClosable WRITE setClosable NOTIFY closableChanged)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable NOTIFY checkableChanged)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(Ant::TagVariant variant READ variant WRITE setVariant NOTIFY variantChanged)

public:
    explicit AntTag(QWidget* parent = nullptr);
    explicit AntTag(const QString& text, QWidget* parent = nullptr);

    QString text() const;
    void setText(const QString& text);

    QString color() const;
    void setColor(const QString& color);

    QString iconText() const;
    void setIconText(const QString& iconText);

    bool isClosable() const;
    void setClosable(bool closable);

    bool isCheckable() const;
    void setCheckable(bool checkable);

    bool isChecked() const;
    void setChecked(bool checked);

    Ant::TagVariant variant() const;
    void setVariant(Ant::TagVariant variant);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void textChanged(const QString& text);
    void colorChanged(const QString& color);
    void iconTextChanged(const QString& iconText);
    void closableChanged(bool closable);
    void checkableChanged(bool checkable);
    void checkedChanged(bool checked);
    void variantChanged(Ant::TagVariant variant);
    void closeRequested();
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QRect closeRect() const;
    QColor baseColor() const;
    QColor backgroundColor() const;
    QColor borderColor() const;
    QColor textColor() const;
    bool hasCustomColor() const;

    QString m_text;
    QString m_color;
    QString m_iconText;
    bool m_closable = false;
    bool m_checkable = false;
    bool m_checked = false;
    bool m_hovered = false;
    bool m_closeHovered = false;
    Ant::TagVariant m_variant = Ant::TagVariant::Filled;
};
