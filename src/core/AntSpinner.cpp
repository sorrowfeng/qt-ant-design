#include "AntSpinner.h"

#include <QPainter>

AntSpinner::AntSpinner(QObject* parent)
    : QObject(parent)
{
    m_timer.setTimerType(Qt::CoarseTimer);
    connect(&m_timer, &QTimer::timeout, this, [this]() {
        m_angle = (m_angle + m_step) % 360;
        Q_EMIT ticked(m_angle);
    });
}

void AntSpinner::start()
{
    if (!m_timer.isActive())
    {
        m_timer.start(m_intervalMs);
    }
}

void AntSpinner::stop()
{
    m_timer.stop();
}

void AntSpinner::setRunning(bool running)
{
    if (running)
    {
        start();
    }
    else
    {
        stop();
    }
}

bool AntSpinner::isRunning() const
{
    return m_timer.isActive();
}

int AntSpinner::angle() const
{
    return m_angle;
}

void AntSpinner::setAngle(int angle)
{
    m_angle = angle;
}

void AntSpinner::setInterval(int intervalMs)
{
    if (m_intervalMs == intervalMs)
    {
        return;
    }
    m_intervalMs = intervalMs;
    if (m_timer.isActive())
    {
        m_timer.start(intervalMs);
    }
}

void AntSpinner::setStep(int stepDegrees)
{
    m_step = stepDegrees;
}

void AntSpinner::setPrecise(bool precise)
{
    m_timer.setTimerType(precise ? Qt::PreciseTimer : Qt::CoarseTimer);
}

void AntSpinner::drawArc(QPainter* painter, const QRectF& rect, const QColor& color,
                         int angle, int spanAngle, qreal penWidth)
{
    if (!painter || rect.isEmpty())
    {
        return;
    }
    painter->save();
    painter->setPen(QPen(color, penWidth, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(Qt::NoBrush);
    painter->drawArc(rect, angle * 16, spanAngle * 16);
    painter->restore();
}
