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
    Q_PROPERTY(Ant::ProgressType progressType READ progressType WRITE setProgressType NOTIFY progressTypeChanged)
    Q_PROPERTY(Ant::ProgressStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(bool showInfo READ showInfo WRITE setShowInfo NOTIFY showInfoChanged)
    Q_PROPERTY(int strokeWidth READ strokeWidth WRITE setStrokeWidth NOTIFY strokeWidthChanged)
    Q_PROPERTY(int circleSize READ circleSize WRITE setCircleSize NOTIFY circleSizeChanged)

public:
    explicit AntProgress(QWidget* parent = nullptr);

    int percent() const;
    void setPercent(int percent);

    Ant::ProgressType progressType() const;
    void setProgressType(Ant::ProgressType type);

    Ant::ProgressStatus status() const;
    void setStatus(Ant::ProgressStatus status);

    bool showInfo() const;
    void setShowInfo(bool showInfo);

    int strokeWidth() const;
    void setStrokeWidth(int width);

    int circleSize() const;
    void setCircleSize(int size);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void percentChanged(int percent);
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
    void drawLineProgress(QPainter& painter) const;
    void drawCircleProgress(QPainter& painter) const;

    int m_percent = 0;
    Ant::ProgressType m_progressType = Ant::ProgressType::Line;
    Ant::ProgressStatus m_status = Ant::ProgressStatus::Normal;
    bool m_showInfo = true;
    int m_strokeWidth = 8;
    int m_circleSize = 120;
    int m_activeOffset = 0;
    QTimer* m_activeTimer = nullptr;
};
