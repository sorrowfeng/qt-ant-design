#include "AntBorderBeam.h"

#include "../styles/AntBorderBeamStyle.h"
#include "core/AntTheme.h"

#include <QVariantAnimation>
#include <QVBoxLayout>

AntBorderBeam::AntBorderBeam(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntBorderBeamStyle>(this);
    setAttribute(Qt::WA_Hover);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    m_contentLayout = new QVBoxLayout(this);
    const int margin = contentMargin();
    m_contentLayout->setContentsMargins(margin, margin, margin, margin);
    m_contentLayout->setSpacing(0);

    m_animation = new QVariantAnimation(this);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setDuration(m_duration);
    m_animation->setLoopCount(-1);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        m_phase = value.toReal();
        Q_EMIT phaseChanged(m_phase);
        update();
    });
    updateAnimationState();
}

AntBorderBeam::AntBorderBeam(QWidget* content, QWidget* parent)
    : AntBorderBeam(parent)
{
    setContentWidget(content);
}

AntBorderBeam::~AntBorderBeam()
{
    if (m_animation)
    {
        m_animation->stop();
    }
}

QColor AntBorderBeam::color() const { return m_color; }

void AntBorderBeam::setColor(const QColor& color)
{
    if (m_color == color)
    {
        return;
    }
    m_color = color;
    update();
    Q_EMIT colorChanged(m_color);
}

int AntBorderBeam::count() const { return m_count; }

void AntBorderBeam::setCount(int count)
{
    const int clamped = qBound(1, count, 8);
    if (m_count == clamped)
    {
        return;
    }
    m_count = clamped;
    update();
    Q_EMIT countChanged(m_count);
}

int AntBorderBeam::duration() const { return m_duration; }

void AntBorderBeam::setDuration(int msecs)
{
    const int clamped = qMax(200, msecs);
    if (m_duration == clamped)
    {
        return;
    }
    m_duration = clamped;
    if (m_animation)
    {
        const bool wasRunning = m_animation->state() == QAbstractAnimation::Running;
        m_animation->setDuration(m_duration);
        if (wasRunning)
        {
            m_animation->start();
        }
    }
    Q_EMIT durationChanged(m_duration);
}

int AntBorderBeam::beamLength() const { return m_beamLength; }

void AntBorderBeam::setBeamLength(int pixels)
{
    const int clamped = qMax(8, pixels);
    if (m_beamLength == clamped)
    {
        return;
    }
    m_beamLength = clamped;
    update();
    Q_EMIT beamLengthChanged(m_beamLength);
}

int AntBorderBeam::lineWidth() const { return m_lineWidth; }

void AntBorderBeam::setLineWidth(int pixels)
{
    const int clamped = qBound(1, pixels, 16);
    if (m_lineWidth == clamped)
    {
        return;
    }
    m_lineWidth = clamped;
    if (m_contentLayout)
    {
        const int margin = contentMargin();
        m_contentLayout->setContentsMargins(margin, margin, margin, margin);
    }
    updateGeometry();
    update();
    Q_EMIT lineWidthChanged(m_lineWidth);
}

int AntBorderBeam::borderRadius() const { return m_borderRadius; }

void AntBorderBeam::setBorderRadius(int radius)
{
    const int clamped = qMax(-1, radius);
    if (m_borderRadius == clamped)
    {
        return;
    }
    m_borderRadius = clamped;
    update();
    Q_EMIT borderRadiusChanged(m_borderRadius);
}

bool AntBorderBeam::isRunning() const { return m_running; }

void AntBorderBeam::setRunning(bool running)
{
    if (m_running == running)
    {
        return;
    }
    m_running = running;
    updateAnimationState();
    Q_EMIT runningChanged(m_running);
}

bool AntBorderBeam::isActiveOnHover() const { return m_activeOnHover; }

void AntBorderBeam::setActiveOnHover(bool hoverOnly)
{
    if (m_activeOnHover == hoverOnly)
    {
        return;
    }
    m_activeOnHover = hoverOnly;
    updateAnimationState();
    Q_EMIT activeOnHoverChanged(m_activeOnHover);
}

qreal AntBorderBeam::phase() const { return m_phase; }

void AntBorderBeam::setContentWidget(QWidget* widget)
{
    if (m_content == widget)
    {
        return;
    }
    if (m_content)
    {
        m_contentLayout->removeWidget(m_content);
        m_content->setParent(nullptr);
    }
    m_content = widget;
    if (m_content)
    {
        m_contentLayout->addWidget(m_content);
    }
    updateGeometry();
    update();
}

QWidget* AntBorderBeam::contentWidget() const { return m_content; }

QSize AntBorderBeam::sizeHint() const
{
    const int margin = contentMargin();
    const QSize base = m_content ? m_content->sizeHint() : QSize(160, 96);
    return base + QSize(margin * 2, margin * 2);
}

QSize AntBorderBeam::minimumSizeHint() const
{
    const int margin = contentMargin();
    const QSize base = m_content ? m_content->minimumSizeHint() : QSize(64, 48);
    return base + QSize(margin * 2, margin * 2);
}

void AntBorderBeam::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        updateAnimationState();
        update();
    }
    else if (event->type() == QEvent::PaletteChange)
    {
        update();
    }
    QWidget::changeEvent(event);
}

void AntBorderBeam::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    updateAnimationState();
}

void AntBorderBeam::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    updateAnimationState();
}

void AntBorderBeam::enterEvent(AntEnterEvent* event)
{
    m_hovered = true;
    if (m_activeOnHover)
    {
        updateAnimationState();
    }
    QWidget::enterEvent(event);
}

void AntBorderBeam::leaveEvent(QEvent* event)
{
    m_hovered = false;
    if (m_activeOnHover)
    {
        updateAnimationState();
    }
    QWidget::leaveEvent(event);
}

void AntBorderBeam::updateAnimationState()
{
    if (!m_animation)
    {
        return;
    }
    const bool shouldRun = m_running && (!m_activeOnHover || m_hovered) && isVisible() && isEnabled();
    if (shouldRun && m_animation->state() != QAbstractAnimation::Running)
    {
        m_animation->start();
    }
    else if (!shouldRun && m_animation->state() == QAbstractAnimation::Running)
    {
        m_animation->stop();
        if (m_activeOnHover && !m_hovered)
        {
            m_phase = 0.0;
            update();
        }
    }
}

int AntBorderBeam::contentMargin() const
{
    return m_lineWidth + 4;
}

QColor AntBorderBeam::resolvedBeamColor() const
{
    if (m_color.isValid())
    {
        return m_color;
    }
    return AntTheme::instance()->tokens().colorPrimary;
}

int AntBorderBeam::resolvedBorderRadius() const
{
    if (m_borderRadius >= 0)
    {
        return m_borderRadius;
    }
    return AntTheme::instance()->tokens().borderRadiusLG;
}
