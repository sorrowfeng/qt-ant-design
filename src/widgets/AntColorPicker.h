#pragma once

#include <QColor>
#include <QRect>
#include <QWidget>

#include "core/AntTypes.h"

class QEnterEvent;
class QEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;

class AntColorPicker : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor currentColor READ currentColor WRITE setCurrentColor NOTIFY currentColorChanged)
    Q_PROPERTY(bool showText READ showText WRITE setShowText NOTIFY showTextChanged)

public:
    explicit AntColorPicker(QWidget* parent = nullptr);
    explicit AntColorPicker(const QColor& initial, QWidget* parent = nullptr);

    QColor currentColor() const;
    void setCurrentColor(const QColor& color);

    bool showText() const;
    void setShowText(bool showText);

    static QColor getColor(const QColor& initial = Qt::white, QWidget* parent = nullptr,
                           const QString& title = QString());

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void colorSelected(const QColor& color);
    void currentColorChanged(const QColor& color);
    void showTextChanged(bool showText);

private:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

    void openEditor();
    QRect colorBlockRect() const;

    bool m_showText = false;
    bool m_hovered = false;
    bool m_pressed = false;
    QColor m_currentColor = Qt::white;
};
