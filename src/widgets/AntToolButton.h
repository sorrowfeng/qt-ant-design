#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QRectF>
#include <QToolButton>

#include "core/AntTypes.h"

class AntMenu;
class AntToolTip;
class QActionEvent;
class QEvent;
class QFocusEvent;
class QHideEvent;
class QKeyEvent;
class QPropertyAnimation;
class QShowEvent;
class QTimer;

class QT_ANT_DESIGN_EXPORT AntToolButton : public QToolButton
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
    bool isFocusVisibleState() const;

Q_SIGNALS:
    void buttonTypeChanged(Ant::ButtonType type);
    void buttonSizeChanged(Ant::Size size);
    void dangerChanged(bool danger);
    void loadingChanged(bool loading);
    void arrowRotationChanged(qreal rotation);

protected:
    bool event(QEvent* event) override;
    void actionEvent(QActionEvent* event) override;
    void enterEvent(AntEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    bool hitButton(const QPoint& pos) const override;

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
    QRectF contentRect(const Metrics& metrics) const;
    QColor waveColor() const;
    void updateCursorState();
    void updateGeometryFromState(bool notifyGeometry = true);
    QRect spinnerIndicatorRect() const;
    QRect arrowIndicatorRect() const;
    void updateSpinnerRegion();
    void updateArrowRegion();
    void updateIndicatorRegion(const QRect& rect, int& counter);
    void updateSpinnerTimer();
    QString effectiveToolTipText() const;
    void syncAntToolTip();
    void syncToolButtonPerfCounters() const;

    Ant::ButtonType m_buttonType = Ant::ButtonType::Default;
    Ant::Size m_buttonSize = Ant::Size::Middle;
    bool m_danger = false;
    bool m_loading = false;
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_focusVisible = false;
    qreal m_arrowRotation = 0.0;
    int m_spinnerAngle = 0;
    QTimer* m_spinnerTimer = nullptr;
    QPropertyAnimation* m_arrowAnimation = nullptr;
    AntToolTip* m_antToolTip = nullptr;
    int m_spinnerRegionUpdateCount = 0;
    int m_arrowRegionUpdateCount = 0;
};
