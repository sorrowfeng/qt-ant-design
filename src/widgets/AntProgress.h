#pragma once

#include <QColor>
#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QPaintEvent;
class QPainter;
class QShowEvent;
class QTimer;

class AntProgress : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int percent READ percent WRITE setPercent NOTIFY percentChanged)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum NOTIFY rangeChanged)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum NOTIFY rangeChanged)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(Ant::ProgressType progressType READ progressType WRITE setProgressType NOTIFY progressTypeChanged)
    Q_PROPERTY(Ant::ProgressStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(bool showInfo READ showInfo WRITE setShowInfo NOTIFY showInfoChanged)
    Q_PROPERTY(bool textVisible READ textVisible WRITE setTextVisible NOTIFY showInfoChanged)
    Q_PROPERTY(int strokeWidth READ strokeWidth WRITE setStrokeWidth NOTIFY strokeWidthChanged)
    Q_PROPERTY(int circleSize READ circleSize WRITE setCircleSize NOTIFY circleSizeChanged)

public:
    explicit AntProgress(QWidget* parent = nullptr);

    int percent() const;
    void setPercent(int percent);
    int minimum() const;
    void setMinimum(int minimum);
    int maximum() const;
    void setMaximum(int maximum);
    void setRange(int minimum, int maximum);
    int value() const;
    void setValue(int value);
    void reset();

    Ant::ProgressType progressType() const;
    void setProgressType(Ant::ProgressType type);

    Ant::ProgressStatus status() const;
    void setStatus(Ant::ProgressStatus status);

    bool showInfo() const;
    void setShowInfo(bool showInfo);
    bool textVisible() const;
    void setTextVisible(bool visible);

    int strokeWidth() const;
    void setStrokeWidth(int width);

    int circleSize() const;
    void setCircleSize(int size);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void percentChanged(int percent);
    void valueChanged(int value);
    void rangeChanged(int minimum, int maximum);
    void progressTypeChanged(Ant::ProgressType type);
    void statusChanged(Ant::ProgressStatus status);
    void showInfoChanged(bool showInfo);
    void strokeWidthChanged(int width);
    void circleSizeChanged(int size);

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    QColor progressColor() const;
    QColor railColor() const;
    QString infoText() const;
    void updateAnimationState();
    int percentForValue(int value) const;
    int valueForPercent(int percent) const;
    void syncPercentFromValue();
    void setValueAndPercent(int value, int percent);
    void drawLineProgress(QPainter& painter) const;
    void drawCircleProgress(QPainter& painter) const;

    int m_percent = 0;
    int m_minimum = 0;
    int m_maximum = 100;
    int m_value = 0;
    Ant::ProgressType m_progressType = Ant::ProgressType::Line;
    Ant::ProgressStatus m_status = Ant::ProgressStatus::Normal;
    bool m_showInfo = true;
    int m_strokeWidth = 8;
    int m_circleSize = 120;
    int m_activeOffset = 0;
    QTimer* m_activeTimer = nullptr;
};
