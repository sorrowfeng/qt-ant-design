#pragma once

#include "QtAntDesignExport.h"

#include <QColor>
#include <QObject>
#include <QRectF>
#include <QTimer>

class QPainter;

// Unified loading-spinner unit: owns the rotation timer + angle state, and
// provides the canonical arc-spinner draw entry. Components that render a
// rotating loading indicator (AntButton, AntToolButton, AntSwitch, AntSelect,
// AntCard, AntMessage, AntNotification, AntIcon) host an AntSpinner member and
// react to ticked() with their own region-update strategy, instead of each
// owning a raw QTimer with a duplicated `(angle + step) % 360` callback.
class QT_ANT_DESIGN_EXPORT AntSpinner : public QObject
{
    Q_OBJECT
public:
    explicit AntSpinner(QObject* parent = nullptr);

    // Timer control. setRunning(true) is a no-op while already running, so
    // repeated show/state refreshes never reset the rotation phase.
    void start();
    void stop();
    void setRunning(bool running);
    bool isRunning() const;

    int angle() const;
    void setAngle(int angle);

    // Interval/step defaults (80ms / 30deg per tick) match the Ant Design
    // loading arc; button-family components switch to 16ms / 6deg for the
    // smoother 60fps variant. setInterval() re-applies a running timer.
    void setInterval(int intervalMs);
    void setStep(int stepDegrees);
    void setPrecise(bool precise);

    // Canonical arc spinner: 3 o'clock start, counter-clockwise sweep, pen
    // centered on rect — exactly the geometry shared by Switch/Select/Card/
    // Message/Notification loading icons (they differ only in span/pen/inset).
    static void drawArc(QPainter* painter, const QRectF& rect, const QColor& color,
                        int angle, int spanAngle = 270, qreal penWidth = 1.6);

signals:
    void ticked(int angle);

private:
    QTimer m_timer;
    int m_angle = 0;
    int m_intervalMs = 80;
    int m_step = 30;
};
