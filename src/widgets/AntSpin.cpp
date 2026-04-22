#include "AntSpin.h"

#include <QHideEvent>
#include <QPainter>
#include <QShowEvent>
#include <QTimer>

#include <algorithm>
#include <cmath>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"

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

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
        update();
    });

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

Ant::SpinSize AntSpin::spinSize() const { return m_spinSize; }

void AntSpin::setSpinSize(Ant::SpinSize size)
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

void AntSpin::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    if (!m_effectiveSpinning)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const Metrics m = metrics();
    const int totalHeight = m.indicatorSize + (m_description.isEmpty() ? 0 : m.spacing + m.fontSize);
    QRectF indicator((width() - m.indicatorSize) / 2.0,
                     (height() - totalHeight) / 2.0,
                     m.indicatorSize,
                     m.indicatorSize);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (m_percent >= 0)
    {
        drawPercent(painter, indicator);
    }
    else
    {
        drawIndeterminate(painter, indicator);
    }

    if (!m_description.isEmpty())
    {
        QFont f = painter.font();
        f.setPixelSize(m.fontSize);
        painter.setFont(f);
        painter.setPen(token.colorTextSecondary);
        QRectF textRect(0, indicator.bottom() + m.spacing, width(), m.fontSize + 4);
        painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignTop, m_description);
    }
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
    if (m_spinSize == Ant::SpinSize::Small)
    {
        m.indicatorSize = 14;
        m.dotSize = 4;
        m.fontSize = token.fontSizeSM;
        m.spacing = 6;
    }
    else if (m_spinSize == Ant::SpinSize::Large)
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

void AntSpin::drawIndeterminate(QPainter& painter, const QRectF& rect) const
{
    const auto& token = antTheme->tokens();
    const Metrics m = metrics();
    const QPointF center = rect.center();
    const qreal radius = rect.width() / 2.0 - m.dotSize / 2.0;
    constexpr qreal pi = 3.14159265358979323846;

    for (int i = 0; i < 4; ++i)
    {
        const qreal deg = (m_angle + i * 90) * pi / 180.0;
        QColor color = token.colorPrimary;
        color.setAlphaF(0.35 + 0.16 * i);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        const QPointF dot(center.x() + std::cos(deg) * radius, center.y() + std::sin(deg) * radius);
        painter.drawEllipse(dot, m.dotSize / 2.0, m.dotSize / 2.0);
    }
}

void AntSpin::drawPercent(QPainter& painter, const QRectF& rect) const
{
    const auto& token = antTheme->tokens();
    const Metrics m = metrics();
    const QRectF arcRect = rect.adjusted(2, 2, -2, -2);
    const int lineWidth = std::max(2, m.indicatorSize / 10);

    painter.setPen(QPen(token.colorFillTertiary, lineWidth, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(arcRect, 0, 360 * 16);

    painter.setPen(QPen(token.colorPrimary, lineWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(arcRect, 90 * 16, -m_percent * 360 * 16 / 100);

    if (m_spinSize != Ant::SpinSize::Small)
    {
        QFont f = painter.font();
        f.setPixelSize(m_spinSize == Ant::SpinSize::Large ? 10 : 8);
        f.setWeight(QFont::DemiBold);
        painter.setFont(f);
        painter.setPen(token.colorTextSecondary);
        painter.drawText(rect, Qt::AlignCenter, QStringLiteral("%1").arg(m_percent));
    }
}
