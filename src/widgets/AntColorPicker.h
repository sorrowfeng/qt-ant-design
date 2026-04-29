#pragma once

#include <QColor>
#include <QRect>
#include <QWidget>

#include "core/AntTypes.h"

class QEnterEvent;
class QEvent;
class QFrame;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;

class AntColorPicker : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor currentColor READ currentColor WRITE setCurrentColor NOTIFY currentColorChanged)
    Q_PROPERTY(bool showText READ showText WRITE setShowText NOTIFY showTextChanged)
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)

public:
    explicit AntColorPicker(QWidget* parent = nullptr);
    explicit AntColorPicker(const QColor& initial, QWidget* parent = nullptr);

    QColor currentColor() const;
    void setCurrentColor(const QColor& color);

    bool showText() const;
    void setShowText(bool showText);

    bool isOpen() const;
    void setOpen(bool open);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void colorSelected(const QColor& color);
    void currentColorChanged(const QColor& color);
    void showTextChanged(bool showText);
    void openChanged(bool open);

private:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

    void openEditor();
    void updatePopupGeometry();
    QRect colorBlockRect() const;

    bool m_showText = false;
    bool m_open = false;
    bool m_hovered = false;
    bool m_pressed = false;
    QColor m_currentColor = Qt::white;
    QFrame* m_popup = nullptr;
};
