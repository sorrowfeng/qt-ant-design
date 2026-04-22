#pragma once

#include <QPushButton>
#include <QTimer>

#include "core/AntTypes.h"

class AntButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(Ant::ButtonType buttonType READ buttonType WRITE setButtonType NOTIFY buttonTypeChanged)
    Q_PROPERTY(Ant::ButtonSize buttonSize READ buttonSize WRITE setButtonSize NOTIFY buttonSizeChanged)
    Q_PROPERTY(Ant::ButtonShape buttonShape READ buttonShape WRITE setButtonShape NOTIFY buttonShapeChanged)
    Q_PROPERTY(bool loading READ isLoading WRITE setLoading NOTIFY loadingChanged)
    Q_PROPERTY(bool danger READ isDanger WRITE setDanger NOTIFY dangerChanged)
    Q_PROPERTY(bool ghost READ isGhost WRITE setGhost NOTIFY ghostChanged)
    Q_PROPERTY(bool block READ isBlock WRITE setBlock NOTIFY blockChanged)

public:
    explicit AntButton(QWidget* parent = nullptr);
    explicit AntButton(const QString& text, QWidget* parent = nullptr);

    Ant::ButtonType buttonType() const;
    void setButtonType(Ant::ButtonType type);
    Ant::ButtonSize buttonSize() const;
    void setButtonSize(Ant::ButtonSize size);
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

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void buttonTypeChanged(Ant::ButtonType type);
    void buttonSizeChanged(Ant::ButtonSize size);
    void buttonShapeChanged(Ant::ButtonShape shape);
    void loadingChanged(bool loading);
    void dangerChanged(bool danger);
    void ghostChanged(bool ghost);
    void blockChanged(bool block);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;

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
    QColor semanticColor() const;
    void updateGeometryFromState();
    void drawSpinner(QPainter& painter, const QRectF& rect, const QColor& color) const;

    Ant::ButtonType m_buttonType = Ant::ButtonType::Default;
    Ant::ButtonSize m_buttonSize = Ant::ButtonSize::Middle;
    Ant::ButtonShape m_buttonShape = Ant::ButtonShape::Default;
    bool m_loading = false;
    bool m_danger = false;
    bool m_ghost = false;
    bool m_block = false;
    bool m_hovered = false;
    bool m_pressed = false;
    int m_spinnerAngle = 0;
    QTimer m_spinnerTimer;
};
