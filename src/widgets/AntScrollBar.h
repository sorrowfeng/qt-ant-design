#pragma once

#include "core/QtAntDesignExport.h"

#include <QScrollBar>

class AntScrollBarStyle;

class QT_ANT_DESIGN_EXPORT AntScrollBar : public QScrollBar
{
    Q_OBJECT
    Q_PROPERTY(bool autoHide READ autoHide WRITE setAutoHide NOTIFY autoHideChanged)

public:
    explicit AntScrollBar(QWidget* parent = nullptr);
    explicit AntScrollBar(Qt::Orientation orientation, QWidget* parent = nullptr);

    bool autoHide() const;
    void setAutoHide(bool autoHide);

    bool isHoveredState() const;
    bool isPressedState() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void autoHideChanged(bool autoHide);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void initStyle();

    bool m_autoHide = true;
    bool m_hovered = false;
    bool m_pressed = false;
};
