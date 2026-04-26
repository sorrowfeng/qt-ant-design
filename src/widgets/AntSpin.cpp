#include "AntSpin.h"

#include <QHideEvent>
#include <QShowEvent>
#include <QTimer>

#include <algorithm>

#include "../styles/AntSpinStyle.h"
#include "core/AntTheme.h"

AntSpin::AntSpin(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, [this]() {
        m_angle = (m_angle + 18) % 360;
        update();
    });

    m_delayTimer = new QTimer(this);
    m_delayTimer->setSingleShot(true);
    connect(m_delayTimer, &QTimer::timeout, this, [this]() {
        m_effectiveSpinning = m_spinning;
        updateAnimationState();
        update();
    });

    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        updateGeometry();
        update();
    });

    auto* spinStyle = new AntSpinStyle(style());
    spinStyle->setParent(this);
    setStyle(spinStyle);

    updateAnimationState();
}

bool AntSpin::isSpinning() const { return m_spinning; }

void AntSpin::setSpinning(bool spinning)
{
    if (m_spinning == spinning)
    {
        return;
    }

    m_spinning = spinning;
    if (m_spinning && m_delay > 0)
    {
        m_effectiveSpinning = false;
        m_delayTimer->start(m_delay);
    }
    else
    {
        m_delayTimer->stop();
        m_effectiveSpinning = m_spinning;
    }
    updateAnimationState();
    update();
    Q_EMIT spinningChanged(m_spinning);
}

Ant::Size AntSpin::spinSize() const { return m_spinSize; }

void AntSpin::setSpinSize(Ant::Size size)
{
    if (m_spinSize == size)
    {
        return;
    }
    m_spinSize = size;
    updateGeometry();
    update();
    Q_EMIT spinSizeChanged(m_spinSize);
}

QString AntSpin::description() const { return m_description; }

void AntSpin::setDescription(const QString& description)
{
    if (m_description == description)
    {
        return;
    }
    m_description = description;
    updateGeometry();
    update();
    Q_EMIT descriptionChanged(m_description);
}

int AntSpin::delay() const { return m_delay; }

void AntSpin::setDelay(int delayMs)
{
    delayMs = std::max(0, delayMs);
    if (m_delay == delayMs)
    {
        return;
    }
    m_delay = delayMs;
    if (m_spinning)
    {
        setSpinning(false);
        setSpinning(true);
    }
    Q_EMIT delayChanged(m_delay);
}

int AntSpin::percent() const { return m_percent; }

void AntSpin::setPercent(int percent)
{
    percent = percent < 0 ? -1 : std::clamp(percent, 0, 100);
    if (m_percent == percent)
    {
        return;
    }
    m_percent = percent;
    update();
    Q_EMIT percentChanged(m_percent);
}

bool AntSpin::isEffectiveSpinning() const { return m_effectiveSpinning; }

int AntSpin::angle() const { return m_angle; }

QSize AntSpin::sizeHint() const
{
    const Metrics m = metrics();
    const int descHeight = m_description.isEmpty() ? 0 : m.fontSize + m.spacing;
    return QSize(std::max(m.indicatorSize + 12, 96), m.indicatorSize + descHeight + 12);
}

QSize AntSpin::minimumSizeHint() const
{
    const Metrics m = metrics();
    return QSize(m.indicatorSize + 12, m.indicatorSize + 12);
}

void AntSpin::showEvent(QShowEvent* event)
{
    updateAnimationState();
    QWidget::showEvent(event);
}

void AntSpin::hideEvent(QHideEvent* event)
{
    m_animationTimer->stop();
    QWidget::hideEvent(event);
}

void AntSpin::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        update();
    }
    QWidget::changeEvent(event);
}

AntSpin::Metrics AntSpin::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    m.fontSize = token.fontSize;
    if (m_spinSize == Ant::Size::Small)
    {
        m.indicatorSize = 14;
        m.dotSize = 4;
        m.fontSize = token.fontSizeSM;
        m.spacing = 6;
    }
    else if (m_spinSize == Ant::Size::Large)
    {
        m.indicatorSize = 32;
        m.dotSize = 8;
        m.fontSize = token.fontSizeLG;
        m.spacing = 10;
    }
    return m;
}

void AntSpin::updateAnimationState()
{
    const bool shouldAnimate = isVisible() && isEnabled() && m_effectiveSpinning && m_percent < 0;
    if (shouldAnimate && !m_animationTimer->isActive())
    {
        m_animationTimer->start(40);
    }
    else if (!shouldAnimate)
    {
        m_animationTimer->stop();
    }
}
