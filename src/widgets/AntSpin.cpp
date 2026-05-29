#include "AntSpin.h"

#include <QEvent>
#include <QHideEvent>
#include <QShowEvent>
#include <QTimer>

#include <algorithm>

#include "../styles/AntSpinStyle.h"
#include "core/AntTheme.h"
#include "core/AntThemeRefresh_p.h"

AntSpin::AntSpin(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    m_animationTimer = new QTimer(this);
    m_animationTimer->setTimerType(Qt::PreciseTimer);
    connect(m_animationTimer, &QTimer::timeout, this, [this]() {
        m_angle = (m_angle + 5) % 360;
        requestSpinUpdate(spinIndicatorDirtyRect(), QStringLiteral("animation"));
    });

    m_delayTimer = new QTimer(this);
    m_delayTimer->setSingleShot(true);
    connect(m_delayTimer, &QTimer::timeout, this, [this]() {
        m_effectiveSpinning = m_spinning;
        invalidateSpinLayout();
        updateAnimationState();
        requestSpinUpdate(spinVisualRect(), QStringLiteral("delay"));
    });

    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateSpinLayout();
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
        requestSpinUpdate(spinVisualRect(), QStringLiteral("theme"));
    });

    auto* spinStyle = new AntSpinStyle(style());
    spinStyle->setParent(this);
    setStyle(spinStyle);

    updateAnimationState();
    syncSpinPerfCounters();
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
    invalidateSpinLayout();
    updateAnimationState();
    requestSpinUpdate(spinVisualRect(), QStringLiteral("spinning"));
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
    invalidateSpinLayout();
    updateGeometry();
    requestSpinUpdate(rect(), QStringLiteral("size"));
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
    invalidateSpinLayout();
    updateGeometry();
    requestSpinUpdate(rect(), QStringLiteral("description"));
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
    invalidateSpinLayout();
    updateAnimationState();
    requestSpinUpdate(spinIndicatorDirtyRect(), QStringLiteral("percent"));
    Q_EMIT percentChanged(m_percent);
}

bool AntSpin::isEffectiveSpinning() const { return m_effectiveSpinning; }

int AntSpin::angle() const { return m_angle; }

QSize AntSpin::sizeHint() const
{
    return spinLayout().sizeHint;
}

QSize AntSpin::minimumSizeHint() const
{
    return spinLayout().minimumSizeHint;
}

void AntSpin::showEvent(QShowEvent* event)
{
    updateAnimationState();
    QWidget::showEvent(event);
}

void AntSpin::hideEvent(QHideEvent* event)
{
    updateAnimationState();
    QWidget::hideEvent(event);
}

void AntSpin::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        invalidateSpinLayout();
        updateAnimationState();
        requestSpinUpdate(spinVisualRect(), QStringLiteral("enabled"));
    }
    else if (event->type() == QEvent::FontChange)
    {
        invalidateSpinLayout();
        updateGeometry();
        requestSpinUpdate(spinVisualRect(), QStringLiteral("font"));
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

const AntSpin::SpinLayoutCache& AntSpin::spinLayout() const
{
    const auto& token = antTheme->tokens();
    if (m_layoutCache.valid &&
        m_layoutCache.widgetSize == size() &&
        m_layoutCache.themeMode == antTheme->themeMode() &&
        m_layoutCache.tokenFontSize == token.fontSize &&
        m_layoutCache.tokenFontSizeSM == token.fontSizeSM &&
        m_layoutCache.tokenFontSizeLG == token.fontSizeLG &&
        m_layoutCache.enabled == isEnabled() &&
        m_layoutCache.effectiveSpinning == m_effectiveSpinning &&
        m_layoutCache.spinSize == m_spinSize &&
        m_layoutCache.description == m_description &&
        m_layoutCache.percent == m_percent)
    {
        ++m_layoutCacheHitCount;
        syncSpinPerfCounters();
        return m_layoutCache;
    }

    ++m_layoutBuildCount;
    SpinLayoutCache cache;
    cache.valid = true;
    cache.widgetSize = size();
    cache.themeMode = antTheme->themeMode();
    cache.tokenFontSize = token.fontSize;
    cache.tokenFontSizeSM = token.fontSizeSM;
    cache.tokenFontSizeLG = token.fontSizeLG;
    cache.enabled = isEnabled();
    cache.effectiveSpinning = m_effectiveSpinning;
    cache.spinSize = m_spinSize;
    cache.description = m_description;
    cache.percent = m_percent;
    cache.metrics = metrics();
    const int descHeight = m_description.isEmpty() ? 0 : cache.metrics.fontSize + cache.metrics.spacing;
    cache.sizeHint = QSize(std::max(cache.metrics.indicatorSize + 12, 96),
                           cache.metrics.indicatorSize + descHeight + 12);
    cache.minimumSizeHint = QSize(cache.metrics.indicatorSize + 12, cache.metrics.indicatorSize + 12);

    const int totalHeight = cache.metrics.indicatorSize + descHeight;
    cache.indicatorRect = QRectF((width() - cache.metrics.indicatorSize) / 2.0,
                                 (height() - totalHeight) / 2.0,
                                 cache.metrics.indicatorSize,
                                 cache.metrics.indicatorSize);
    cache.textRect = m_description.isEmpty()
        ? QRectF()
        : QRectF(0,
                 cache.indicatorRect.bottom() + cache.metrics.spacing,
                 width(),
                 cache.metrics.fontSize + 4);
    cache.indicatorCenter = cache.indicatorRect.center();
    cache.dotTravelRadius = cache.indicatorRect.width() / 2.0 - cache.metrics.dotSize / 2.0;
    cache.arcRect = cache.indicatorRect.adjusted(2, 2, -2, -2);
    cache.percentLineWidth = std::max(2, cache.metrics.indicatorSize / 10);
    cache.percentFontSize = m_spinSize == Ant::Size::Large ? 10 : 8;
    cache.primaryColor = token.colorPrimary;
    cache.trackColor = token.colorFillTertiary;
    cache.textColor = token.colorTextSecondary;
    cache.percentText = m_percent >= 0 ? QStringLiteral("%1").arg(m_percent) : QString();

    m_layoutCache = cache;
    syncSpinPerfCounters();
    return m_layoutCache;
}

void AntSpin::invalidateSpinLayout() const
{
    m_layoutCache.valid = false;
}

QRect AntSpin::spinIndicatorDirtyRect() const
{
    const QRect dirty = spinLayout().indicatorRect.toAlignedRect().adjusted(-3, -3, 3, 3);
    return dirty.intersected(rect());
}

QRect AntSpin::spinVisualRect() const
{
    const auto& layout = spinLayout();
    QRect visual = layout.indicatorRect.toAlignedRect().adjusted(-3, -3, 3, 3);
    if (!layout.textRect.isEmpty())
    {
        visual = visual.united(layout.textRect.toAlignedRect().adjusted(-2, -2, 2, 2));
    }
    return visual.intersected(rect());
}

void AntSpin::requestSpinUpdate(const QRect& region, const QString& mode)
{
    m_lastUpdateMode = mode;
    if (mode == QStringLiteral("animation") || mode == QStringLiteral("percent"))
    {
        ++m_spinnerRegionUpdateCount;
    }
    else
    {
        ++m_visualRegionUpdateCount;
    }
    syncSpinPerfCounters();

    if (region.isValid() && !region.isEmpty())
    {
        update(region);
        return;
    }
    update();
}

void AntSpin::syncSpinPerfCounters() const
{
    auto* self = const_cast<AntSpin*>(this);
    self->setProperty("antSpinLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antSpinLayoutCacheHitCount", m_layoutCacheHitCount);
    self->setProperty("antSpinSpinnerRegionUpdateCount", m_spinnerRegionUpdateCount);
    self->setProperty("antSpinVisualRegionUpdateCount", m_visualRegionUpdateCount);
    self->setProperty("antSpinAnimationTimerActive", m_animationTimer && m_animationTimer->isActive());
    self->setProperty("antSpinLastUpdateMode", m_lastUpdateMode);
}

void AntSpin::updateAnimationState()
{
    const bool shouldAnimate = isVisible() && isEnabled() && m_effectiveSpinning && m_percent < 0;
    if (shouldAnimate && !m_animationTimer->isActive())
    {
        m_animationTimer->start(16);
    }
    else if (!shouldAnimate)
    {
        m_animationTimer->stop();
    }
    syncSpinPerfCounters();
}
