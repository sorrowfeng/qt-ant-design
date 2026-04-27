#pragma once

#include <QPushButton>
#include <QTimer>

#include "core/AntTypes.h"

class AntButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(Ant::ButtonType buttonType READ buttonType WRITE setButtonType NOTIFY buttonTypeChanged)
    Q_PROPERTY(Ant::Size buttonSize READ buttonSize WRITE setButtonSize NOTIFY buttonSizeChanged)
    Q_PROPERTY(Ant::ButtonShape buttonShape READ buttonShape WRITE setButtonShape NOTIFY buttonShapeChanged)
    Q_PROPERTY(bool loading READ isLoading WRITE setLoading NOTIFY loadingChanged)
    Q_PROPERTY(bool danger READ isDanger WRITE setDanger NOTIFY dangerChanged)
    Q_PROPERTY(bool ghost READ isGhost WRITE setGhost NOTIFY ghostChanged)
    Q_PROPERTY(bool block READ isBlock WRITE setBlock NOTIFY blockChanged)
    Q_PROPERTY(Ant::IconType buttonIconType READ buttonIconType WRITE setButtonIconType NOTIFY buttonIconTypeChanged)
    Q_PROPERTY(QColor buttonIconColor READ buttonIconColor WRITE setButtonIconColor NOTIFY buttonIconColorChanged)

public:
    explicit AntButton(QWidget* parent = nullptr);
    explicit AntButton(const QString& text, QWidget* parent = nullptr);

    Ant::ButtonType buttonType() const;
    void setButtonType(Ant::ButtonType type);
    Ant::Size buttonSize() const;
    void setButtonSize(Ant::Size size);
    Ant::ButtonShape buttonShape() const;
    void setButtonShape(Ant::ButtonShape shape);

    bool isLoading() const;
    void setLoading(bool loading);
    bool isDanger() const;
    void setDanger(bool danger);
    bool isGhost() const;
    void setGhost(bool ghost);
    bool isBlock() const;
    void setBlock(bool block);

    Ant::IconType buttonIconType() const;
    void setButtonIconType(Ant::IconType iconType);
    QColor buttonIconColor() const;
    void setButtonIconColor(const QColor& color);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    int spinnerAngle() const;

Q_SIGNALS:
    void buttonTypeChanged(Ant::ButtonType type);
    void buttonSizeChanged(Ant::Size size);
    void buttonShapeChanged(Ant::ButtonShape shape);
    void loadingChanged(bool loading);
    void dangerChanged(bool danger);
    void ghostChanged(bool ghost);
    void blockChanged(bool block);
    void buttonIconTypeChanged(Ant::IconType iconType);
    void buttonIconColorChanged(const QColor& color);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool hitButton(const QPoint& pos) const override;

private:
    struct Metrics
    {
        int height = 32;
        int fontSize = 14;
        int paddingX = 15;
        int radius = 6;
        int iconSize = 14;
    };

    Metrics metrics() const;
    int cornerRadius(const Metrics& metrics) const;
    QRectF contentRect(const Metrics& metrics) const;
    void updateGeometryFromState();

    Ant::ButtonType m_buttonType = Ant::ButtonType::Default;
    Ant::Size m_buttonSize = Ant::Size::Middle;
    Ant::ButtonShape m_buttonShape = Ant::ButtonShape::Default;
    bool m_loading = false;
    bool m_danger = false;
    bool m_ghost = false;
    bool m_block = false;
    bool m_hovered = false;
    bool m_pressed = false;
    int m_spinnerAngle = 0;
    QTimer m_spinnerTimer;
    Ant::IconType m_buttonIconType = Ant::IconType::None;
    QColor m_buttonIconColor;
};
