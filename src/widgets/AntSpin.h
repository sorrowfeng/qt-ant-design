#pragma once

#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QHideEvent;
class QPaintEvent;
class QPainter;
class QShowEvent;
class QTimer;

class AntSpin : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool spinning READ isSpinning WRITE setSpinning NOTIFY spinningChanged)
    Q_PROPERTY(Ant::SpinSize spinSize READ spinSize WRITE setSpinSize NOTIFY spinSizeChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(int delay READ delay WRITE setDelay NOTIFY delayChanged)
    Q_PROPERTY(int percent READ percent WRITE setPercent NOTIFY percentChanged)

public:
    explicit AntSpin(QWidget* parent = nullptr);

    bool isSpinning() const;
    void setSpinning(bool spinning);

    Ant::SpinSize spinSize() const;
    void setSpinSize(Ant::SpinSize size);

    QString description() const;
    void setDescription(const QString& description);

    int delay() const;
    void setDelay(int delayMs);

    int percent() const;
    void setPercent(int percent);
    bool isEffectiveSpinning() const;
    int angle() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void spinningChanged(bool spinning);
    void spinSizeChanged(Ant::SpinSize size);
    void descriptionChanged(const QString& description);
    void delayChanged(int delayMs);
    void percentChanged(int percent);

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    struct Metrics
    {
        int indicatorSize = 20;
        int dotSize = 6;
        int fontSize = 14;
        int spacing = 8;
    };

    Metrics metrics() const;
    void updateAnimationState();

    bool m_spinning = true;
    bool m_effectiveSpinning = true;
    Ant::SpinSize m_spinSize = Ant::SpinSize::Middle;
    QString m_description;
    int m_delay = 0;
    int m_percent = -1;
    int m_angle = 0;
    QTimer* m_animationTimer = nullptr;
    QTimer* m_delayTimer = nullptr;
};
