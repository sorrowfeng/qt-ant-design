#pragma once

#include <QToolButton>

#include "core/AntTypes.h"

class AntMenu;
class QPropertyAnimation;

class AntToolButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(Ant::ButtonType buttonType READ buttonType WRITE setButtonType NOTIFY buttonTypeChanged)
    Q_PROPERTY(Ant::Size buttonSize READ buttonSize WRITE setButtonSize NOTIFY buttonSizeChanged)
    Q_PROPERTY(bool danger READ isDanger WRITE setDanger NOTIFY dangerChanged)
    Q_PROPERTY(bool loading READ isLoading WRITE setLoading NOTIFY loadingChanged)
    Q_PROPERTY(qreal arrowRotation READ arrowRotation WRITE setArrowRotation NOTIFY arrowRotationChanged)

public:
    explicit AntToolButton(QWidget* parent = nullptr);
    explicit AntToolButton(const QString& text, QWidget* parent = nullptr);

    Ant::ButtonType buttonType() const;
    void setButtonType(Ant::ButtonType type);

    Ant::Size buttonSize() const;
    void setButtonSize(Ant::Size size);

    bool isDanger() const;
    void setDanger(bool danger);

    bool isLoading() const;
    void setLoading(bool loading);

    qreal arrowRotation() const;
    void setArrowRotation(qreal rotation);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    int spinnerAngle() const;

Q_SIGNALS:
    void buttonTypeChanged(Ant::ButtonType type);
    void buttonSizeChanged(Ant::Size size);
    void dangerChanged(bool danger);
    void loadingChanged(bool loading);
    void arrowRotationChanged(qreal rotation);

protected:
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
        int arrowSize = 10;
    };
    Metrics metrics() const;
    int cornerRadius(const Metrics& m) const;
    void updateGeometryFromState();

    Ant::ButtonType m_buttonType = Ant::ButtonType::Default;
    Ant::Size m_buttonSize = Ant::Size::Middle;
    bool m_danger = false;
    bool m_loading = false;
    bool m_hovered = false;
    bool m_pressed = false;
    qreal m_arrowRotation = 0.0;
    int m_spinnerAngle = 0;
    QTimer* m_spinnerTimer = nullptr;
    QPropertyAnimation* m_arrowAnimation = nullptr;
};
