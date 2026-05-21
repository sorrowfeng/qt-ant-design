#include "AntProgress.h"

#include <QHideEvent>
#include <QSizePolicy>
#include <QShowEvent>
#include <QTimer>

#include <algorithm>

#include "core/AntTheme.h"
#include "styles/AntProgressStyle.h"
#include "styles/AntPalette.h"

namespace
{
bool progressHasStatusIcon(Ant::ProgressStatus status, int percent)
{
    return status == Ant::ProgressStatus::Success ||
           status == Ant::ProgressStatus::Exception ||
           percent >= 100;
}

QColor progressInfoColor(Ant::ProgressStatus status, int percent)
{
    const auto& token = antTheme->tokens();
    if (status == Ant::ProgressStatus::Success || percent >= 100)
    {
        return token.colorSuccess;
    }
    if (status == Ant::ProgressStatus::Exception)
    {
        return token.colorError;
    }
    return token.colorTextSecondary;
}
} // namespace

AntProgress::AntProgress(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntProgressStyle>(this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_activeTimer = new QTimer(this);
    connect(m_activeTimer, &QTimer::timeout, this, [this]() {
        m_activeOffset = (m_activeOffset + 6) % 120;
        requestProgressUpdate(activeDirtyRect(), QStringLiteral("active"));
    });

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateProgressLayout();
        requestProgressUpdate(progressVisualRect(), QStringLiteral("theme"));
    });

    updateAnimationState();
    syncProgressPerfCounters();
}

int AntProgress::percent() const { return m_percent; }

void AntProgress::setPercent(int percent)
{
    percent = std::clamp(percent, 0, 100);
    setValueAndPercent(valueForPercent(percent), percent);
}

int AntProgress::minimum() const { return m_minimum; }

void AntProgress::setMinimum(int minimum)
{
    setRange(minimum, m_maximum);
}

int AntProgress::maximum() const { return m_maximum; }

void AntProgress::setMaximum(int maximum)
{
    setRange(m_minimum, maximum);
}

void AntProgress::setRange(int minimum, int maximum)
{
    if (minimum > maximum)
    {
        std::swap(minimum, maximum);
    }

    const bool rangeWasChanged = m_minimum != minimum || m_maximum != maximum;
    const int oldValue = m_value;
    const int oldPercent = m_percent;

    m_minimum = minimum;
    m_maximum = maximum;
    m_value = std::clamp(m_value, m_minimum, m_maximum);
    syncPercentFromValue();

    if (rangeWasChanged || oldValue != m_value || oldPercent != m_percent)
    {
        invalidateProgressLayout();
        updateAnimationState();
        requestProgressUpdate(progressDirtyRectForPercent(oldPercent, m_percent), QStringLiteral("value"));
    }
    if (rangeWasChanged)
    {
        Q_EMIT rangeChanged(m_minimum, m_maximum);
    }
    if (oldValue != m_value)
    {
        Q_EMIT valueChanged(m_value);
    }
    if (oldPercent != m_percent)
    {
        Q_EMIT percentChanged(m_percent);
    }
}

int AntProgress::value() const { return m_value; }

void AntProgress::setValue(int value)
{
    value = std::clamp(value, m_minimum, m_maximum);
    setValueAndPercent(value, percentForValue(value));
}

void AntProgress::reset()
{
    setValue(m_minimum);
}

Ant::ProgressType AntProgress::progressType() const { return m_progressType; }

void AntProgress::setProgressType(Ant::ProgressType type)
{
    if (m_progressType == type)
    {
        return;
    }
    m_progressType = type;
    setSizePolicy(m_progressType == Ant::ProgressType::Line ? QSizePolicy::Expanding : QSizePolicy::Fixed,
                  QSizePolicy::Fixed);
    invalidateProgressLayout();
    updateAnimationState();
    updateGeometry();
    requestProgressUpdate(rect(), QStringLiteral("type"));
    Q_EMIT progressTypeChanged(m_progressType);
}

Ant::ProgressStatus AntProgress::status() const { return m_status; }

void AntProgress::setStatus(Ant::ProgressStatus status)
{
    if (m_status == status)
    {
        return;
    }
    m_status = status;
    invalidateProgressLayout();
    updateAnimationState();
    requestProgressUpdate(progressVisualRect(), QStringLiteral("status"));
    Q_EMIT statusChanged(m_status);
}

bool AntProgress::showInfo() const { return m_showInfo; }

void AntProgress::setShowInfo(bool showInfo)
{
    if (m_showInfo == showInfo)
    {
        return;
    }
    m_showInfo = showInfo;
    invalidateProgressLayout();
    updateGeometry();
    requestProgressUpdate(rect(), QStringLiteral("showInfo"));
    Q_EMIT showInfoChanged(m_showInfo);
}

bool AntProgress::textVisible() const { return m_showInfo; }

void AntProgress::setTextVisible(bool visible)
{
    setShowInfo(visible);
}

int AntProgress::strokeWidth() const { return m_strokeWidth; }

void AntProgress::setStrokeWidth(int width)
{
    width = std::clamp(width, 2, 24);
    if (m_strokeWidth == width)
    {
        return;
    }
    m_strokeWidth = width;
    invalidateProgressLayout();
    updateGeometry();
    requestProgressUpdate(rect(), QStringLiteral("strokeWidth"));
    Q_EMIT strokeWidthChanged(m_strokeWidth);
}

int AntProgress::circleSize() const { return m_circleSize; }

void AntProgress::setCircleSize(int size)
{
    size = std::clamp(size, 48, 240);
    if (m_circleSize == size)
    {
        return;
    }
    m_circleSize = size;
    invalidateProgressLayout();
    updateGeometry();
    requestProgressUpdate(rect(), QStringLiteral("circleSize"));
    Q_EMIT circleSizeChanged(m_circleSize);
}

QSize AntProgress::sizeHint() const
{
    return progressLayout().sizeHint;
}

QSize AntProgress::minimumSizeHint() const
{
    return progressLayout().minimumSizeHint;
}

void AntProgress::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntProgress::showEvent(QShowEvent* event)
{
    updateAnimationState();
    QWidget::showEvent(event);
}

void AntProgress::hideEvent(QHideEvent* event)
{
    m_activeTimer->stop();
    QWidget::hideEvent(event);
}

void AntProgress::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        invalidateProgressLayout();
        updateAnimationState();
        requestProgressUpdate(progressVisualRect(), QStringLiteral("enabled"));
    }
    QWidget::changeEvent(event);
}

QColor AntProgress::progressColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorTextDisabled;
    }
    if (m_status == Ant::ProgressStatus::Success || m_percent >= 100)
    {
        return token.colorSuccess;
    }
    if (m_status == Ant::ProgressStatus::Exception)
    {
        return token.colorError;
    }
    return token.colorPrimary;
}

QColor AntProgress::railColor() const
{
    const auto& token = antTheme->tokens();
    return isEnabled() ? token.colorFillQuaternary : token.colorBgContainerDisabled;
}

QString AntProgress::infoText() const
{
    if (m_status == Ant::ProgressStatus::Success || m_percent >= 100)
    {
        return QString();
    }
    if (m_status == Ant::ProgressStatus::Exception)
    {
        return QString();
    }
    return QStringLiteral("%1%").arg(m_percent);
}

const AntProgress::ProgressLayoutCache& AntProgress::progressLayout() const
{
    const auto& token = antTheme->tokens();
    if (m_layoutCache.valid &&
        m_layoutCache.widgetSize == size() &&
        m_layoutCache.font == font() &&
        m_layoutCache.themeMode == antTheme->themeMode() &&
        m_layoutCache.tokenFontSize == token.fontSize &&
        m_layoutCache.tokenFontSizeSM == token.fontSizeSM &&
        m_layoutCache.enabled == isEnabled() &&
        m_layoutCache.percent == m_percent &&
        m_layoutCache.strokeWidth == m_strokeWidth &&
        m_layoutCache.circleSize == m_circleSize &&
        m_layoutCache.progressType == m_progressType &&
        m_layoutCache.status == m_status &&
        m_layoutCache.showInfo == m_showInfo)
    {
        ++m_layoutCacheHitCount;
        syncProgressPerfCounters();
        return m_layoutCache;
    }

    ++m_layoutBuildCount;
    ProgressLayoutCache cache;
    cache.valid = true;
    cache.widgetSize = size();
    cache.font = font();
    cache.themeMode = antTheme->themeMode();
    cache.tokenFontSize = token.fontSize;
    cache.tokenFontSizeSM = token.fontSizeSM;
    cache.enabled = isEnabled();
    cache.percent = m_percent;
    cache.strokeWidth = m_strokeWidth;
    cache.circleSize = m_circleSize;
    cache.progressType = m_progressType;
    cache.status = m_status;
    cache.showInfo = m_showInfo;
    cache.progressColor = progressColor();
    cache.railColor = railColor();
    cache.infoColor = progressInfoColor(m_status, m_percent);
    cache.infoText = infoText();
    cache.hasStatusIcon = progressHasStatusIcon(m_status, m_percent);

    if (m_progressType == Ant::ProgressType::Line)
    {
        cache.sizeHint = QSize(260, std::max(24, m_strokeWidth + 12));
        cache.minimumSizeHint = QSize(96, std::max(20, m_strokeWidth + 8));

        const int infoWidth = m_showInfo ? 48 : 0;
        QRectF bar(0, (height() - m_strokeWidth) / 2.0, width() - infoWidth - 8, m_strokeWidth);
        if (!m_showInfo)
        {
            bar.setWidth(width());
        }
        bar = bar.adjusted(1, 0, -1, 0);
        if (bar.width() < 0)
        {
            bar.setWidth(0);
        }
        cache.lineBarRect = bar;
        cache.lineFilledRect = bar;
        cache.lineFilledRect.setWidth(bar.width() * m_percent / 100.0);

        if (m_showInfo)
        {
            cache.lineInfoRect = QRectF(width() - infoWidth, 0, infoWidth, height());
            const qreal mark = 12.0;
            cache.lineStatusIconRect = QRectF(cache.lineInfoRect.center().x() - mark / 2.0,
                                              cache.lineInfoRect.center().y() - mark / 2.0,
                                              mark,
                                              mark);
        }
    }
    else
    {
        cache.sizeHint = QSize(m_circleSize, m_circleSize);
        cache.minimumSizeHint = QSize(48, 48);

        cache.circleVisualSize = std::min(width(), height());
        cache.circleLineWidth = std::max<qreal>(2.0, m_strokeWidth);
        cache.circleArcRect = QRectF((width() - cache.circleVisualSize) / 2.0 + cache.circleLineWidth,
                                     (height() - cache.circleVisualSize) / 2.0 + cache.circleLineWidth,
                                     cache.circleVisualSize - cache.circleLineWidth * 2,
                                     cache.circleVisualSize - cache.circleLineWidth * 2);
        cache.circleStartAngle = m_progressType == Ant::ProgressType::Dashboard ? 225 * 16 : 90 * 16;
        cache.circleFullSpan = m_progressType == Ant::ProgressType::Dashboard ? -270 * 16 : -360 * 16;
        cache.circleProgressSpan = cache.circleFullSpan * m_percent / 100;
        cache.circleInfoFontSize = std::max(12, static_cast<int>(cache.circleVisualSize / 7));
        const qreal mark = std::max<qreal>(18.0, cache.circleVisualSize / 3.6);
        const QPointF center = rect().center();
        cache.circleStatusIconRect = QRectF(center.x() - mark / 2.0,
                                           center.y() - mark / 2.0,
                                           mark,
                                           mark);
    }

    m_layoutCache = cache;
    syncProgressPerfCounters();
    return m_layoutCache;
}

void AntProgress::invalidateProgressLayout() const
{
    m_layoutCache.valid = false;
}

void AntProgress::requestProgressUpdate(const QRect& region, const QString& mode)
{
    m_lastUpdateMode = mode;
    if (mode == QStringLiteral("active"))
    {
        ++m_activeRegionUpdateCount;
    }
    else if (mode == QStringLiteral("value"))
    {
        ++m_valueRegionUpdateCount;
    }
    else
    {
        ++m_visualRegionUpdateCount;
    }
    syncProgressPerfCounters();

    if (region.isValid() && !region.isEmpty())
    {
        update(region);
        return;
    }
    update();
}

QRect AntProgress::lineFilledRectForPercent(int percent) const
{
    const auto& layout = progressLayout();
    QRectF filled = layout.lineBarRect;
    filled.setWidth(layout.lineBarRect.width() * std::clamp(percent, 0, 100) / 100.0);
    return filled.toAlignedRect().adjusted(-2, -2, 2, 2).intersected(rect());
}

QRect AntProgress::progressDirtyRectForPercent(int oldPercent, int newPercent) const
{
    const auto& layout = progressLayout();
    if (layout.progressType != Ant::ProgressType::Line)
    {
        return rect();
    }

    QRect dirty = lineFilledRectForPercent(oldPercent).united(lineFilledRectForPercent(newPercent));
    if (layout.showInfo)
    {
        dirty = dirty.united(layout.lineInfoRect.toAlignedRect().adjusted(-2, -2, 2, 2));
    }
    if (dirty.isEmpty())
    {
        dirty = layout.lineBarRect.toAlignedRect().adjusted(-2, -2, 2, 2);
    }
    return dirty.intersected(rect());
}

QRect AntProgress::progressVisualRect() const
{
    const auto& layout = progressLayout();
    if (layout.progressType != Ant::ProgressType::Line)
    {
        return rect();
    }

    QRect visual = layout.lineBarRect.toAlignedRect().adjusted(-2, -2, 2, 2);
    if (layout.showInfo)
    {
        visual = visual.united(layout.lineInfoRect.toAlignedRect().adjusted(-2, -2, 2, 2));
    }
    return visual.intersected(rect());
}

QRect AntProgress::activeDirtyRect() const
{
    const auto& layout = progressLayout();
    if (layout.progressType != Ant::ProgressType::Line)
    {
        return rect();
    }
    return layout.lineFilledRect.toAlignedRect().adjusted(-2, -2, 2, 2).intersected(rect());
}

void AntProgress::syncProgressPerfCounters() const
{
    auto* self = const_cast<AntProgress*>(this);
    self->setProperty("antProgressLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antProgressLayoutCacheHitCount", m_layoutCacheHitCount);
    self->setProperty("antProgressValueRegionUpdateCount", m_valueRegionUpdateCount);
    self->setProperty("antProgressActiveRegionUpdateCount", m_activeRegionUpdateCount);
    self->setProperty("antProgressVisualRegionUpdateCount", m_visualRegionUpdateCount);
    self->setProperty("antProgressLastUpdateMode", m_lastUpdateMode);
}

void AntProgress::updateAnimationState()
{
    const bool shouldAnimate = isVisible() &&
        isEnabled() &&
        m_progressType == Ant::ProgressType::Line &&
        m_status == Ant::ProgressStatus::Active &&
        m_percent > 0 &&
        m_percent < 100;
    if (shouldAnimate && !m_activeTimer->isActive())
    {
        m_activeTimer->start(40);
    }
    else if (!shouldAnimate)
    {
        m_activeTimer->stop();
    }
}

int AntProgress::percentForValue(int value) const
{
    if (m_maximum <= m_minimum)
    {
        return 0;
    }
    value = std::clamp(value, m_minimum, m_maximum);
    return std::clamp(qRound((value - m_minimum) * 100.0 / (m_maximum - m_minimum)), 0, 100);
}

int AntProgress::valueForPercent(int percent) const
{
    percent = std::clamp(percent, 0, 100);
    if (m_maximum <= m_minimum)
    {
        return m_minimum;
    }
    return std::clamp(m_minimum + qRound((m_maximum - m_minimum) * percent / 100.0), m_minimum, m_maximum);
}

void AntProgress::syncPercentFromValue()
{
    m_percent = percentForValue(m_value);
}

void AntProgress::setValueAndPercent(int value, int percent)
{
    value = std::clamp(value, m_minimum, m_maximum);
    percent = std::clamp(percent, 0, 100);
    const bool valueWasChanged = m_value != value;
    const bool percentWasChanged = m_percent != percent;
    if (!valueWasChanged && !percentWasChanged)
    {
        return;
    }
    const int oldPercent = m_percent;
    m_value = value;
    m_percent = percent;
    invalidateProgressLayout();
    updateAnimationState();
    if (percentWasChanged)
    {
        requestProgressUpdate(progressDirtyRectForPercent(oldPercent, m_percent), QStringLiteral("value"));
    }
    if (valueWasChanged)
    {
        Q_EMIT valueChanged(m_value);
    }
    if (percentWasChanged)
    {
        Q_EMIT percentChanged(m_percent);
    }
}
