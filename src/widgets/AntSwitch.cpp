#include "AntSwitch.h"

#include <QFontMetrics>
#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QSizePolicy>
#include <QTimer>

#include <algorithm>
#include <cmath>

#include "../styles/AntSwitchStyle.h"
#include "core/AntTheme.h"
#include "core/AntThemeRefresh_p.h"
#include "core/AntWave.h"

AntSwitch::AntSwitch(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_progressAnimation = new QPropertyAnimation(this, "handleProgress", this);
    m_progressAnimation->setDuration(180);
    m_progressAnimation->setEasingCurve(QEasingCurve::InOutSine);

    m_stretchAnimation = new QPropertyAnimation(this, "handleStretch", this);
    m_stretchAnimation->setDuration(120);
    m_stretchAnimation->setEasingCurve(QEasingCurve::InOutSine);

    m_loadingTimer = new QTimer(this);
    connect(m_loadingTimer, &QTimer::timeout, this, [this]() {
        m_loadingAngle = (m_loadingAngle + 30) % 360;
        ++m_loadingRegionUpdateCount;
        updateSwitchRegion(switchLoadingDirtyRect(), QStringLiteral("loading"));
    });

    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometryFromState(false);
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
        updateSwitchRegion(switchTrackDirtyRect(), QStringLiteral("theme"));
    });

    auto* switchStyle = new AntSwitchStyle(style());
    switchStyle->setParent(this);
    setStyle(switchStyle);

    updateGeometryFromState();
    syncSwitchPerfCounters();
}

bool AntSwitch::isChecked() const { return m_checked; }

void AntSwitch::setChecked(bool checked)
{
    if (m_checked == checked)
    {
        return;
    }

    m_checked = checked;
    updateSwitchRegion(switchTrackDirtyRect(), QStringLiteral("checked"));
    animateToChecked(m_checked);
    Q_EMIT checkedChanged(m_checked);
    Q_EMIT toggled(m_checked);
}

Ant::Size AntSwitch::switchSize() const { return m_switchSize; }

void AntSwitch::setSwitchSize(Ant::Size size)
{
    if (m_switchSize == size)
    {
        return;
    }

    m_switchSize = size;
    updateGeometryFromState();
    updateSwitchRegion(switchTrackDirtyRect(), QStringLiteral("size"));
    Q_EMIT switchSizeChanged(m_switchSize);
}

bool AntSwitch::isLoading() const { return m_loading; }

void AntSwitch::setLoading(bool loading)
{
    if (m_loading == loading)
    {
        return;
    }

    m_loading = loading;
    updateLoadingTimerState();
    setCursor(isEnabled() && !m_loading ? Qt::PointingHandCursor : Qt::ArrowCursor);
    updateSwitchRegion(switchTrackDirtyRect(), QStringLiteral("loading"));
    Q_EMIT loadingChanged(m_loading);
}

QString AntSwitch::checkedText() const { return m_checkedText; }

void AntSwitch::setCheckedText(const QString& text)
{
    if (m_checkedText == text)
    {
        return;
    }

    m_checkedText = text;
    updateGeometryFromState();
    updateSwitchRegion(switchTrackDirtyRect(), QStringLiteral("text"));
    Q_EMIT checkedTextChanged(m_checkedText);
}

QString AntSwitch::uncheckedText() const { return m_uncheckedText; }

void AntSwitch::setUncheckedText(const QString& text)
{
    if (m_uncheckedText == text)
    {
        return;
    }

    m_uncheckedText = text;
    updateGeometryFromState();
    updateSwitchRegion(switchTrackDirtyRect(), QStringLiteral("text"));
    Q_EMIT uncheckedTextChanged(m_uncheckedText);
}

qreal AntSwitch::handleProgress() const { return m_handleProgress; }

void AntSwitch::setHandleProgress(qreal progress)
{
    const qreal oldProgress = m_handleProgress;
    const qreal nextProgress = std::clamp(progress, 0.0, 1.0);
    if (std::abs(oldProgress - nextProgress) < 0.0001)
    {
        return;
    }

    m_handleProgress = nextProgress;
    ++m_handleRegionUpdateCount;
    updateSwitchRegion(switchHandleDirtyRect(oldProgress, m_handleStretch), QStringLiteral("handle"));
}

qreal AntSwitch::handleStretch() const { return m_handleStretch; }

void AntSwitch::setHandleStretch(qreal stretch)
{
    const qreal oldStretch = m_handleStretch;
    const qreal nextStretch = std::clamp(stretch, 0.0, 1.0);
    if (std::abs(oldStretch - nextStretch) < 0.0001)
    {
        return;
    }

    m_handleStretch = nextStretch;
    ++m_handleRegionUpdateCount;
    updateSwitchRegion(switchHandleDirtyRect(m_handleProgress, oldStretch), QStringLiteral("handle"));
}

bool AntSwitch::isHoveredState() const { return m_hovered; }

bool AntSwitch::isPressedState() const { return m_pressed; }

int AntSwitch::loadingAngle() const { return m_loadingAngle; }

QSize AntSwitch::sizeHint() const
{
    return layoutCache().sizeHint;
}

QSize AntSwitch::minimumSizeHint() const
{
    return layoutCache().minimumSizeHint;
}

void AntSwitch::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    updateSwitchRegion(switchTrackDirtyRect(), QStringLiteral("hover"));
    QWidget::enterEvent(event);
}

void AntSwitch::leaveEvent(QEvent* event)
{
    m_hovered = false;
    // Do NOT clear m_pressed here: while a mouse press is in flight Qt holds
    // an implicit grab and the matching release is still routed back to us,
    // even if the cursor briefly leaves the widget rect or an unrelated
    // top-level window (e.g. a freshly-floated AntDockWidget) changes the
    // active window. Clearing m_pressed in leaveEvent caused the release to
    // be silently discarded — the press animation played but the toggle
    // never committed. m_pressed is cleared in mouseReleaseEvent which is
    // guaranteed to be delivered while the implicit grab is active.
    if (!m_pressed)
    {
        animateStretch(0.0);
    }
    updateSwitchRegion(switchTrackDirtyRect(), QStringLiteral("hover"));
    QWidget::leaveEvent(event);
}

void AntSwitch::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isEnabled() && !m_loading)
    {
        m_pressed = true;
        animateStretch(1.0);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntSwitch::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed)
    {
        m_pressed = false;
        animateStretch(0.0);
        if (rect().contains(event->pos()))
        {
            setChecked(!m_checked);
            const auto& cache = layoutCache();
            AntWave::triggerRect(this,
                                 cache.trackRect.toAlignedRect(),
                                 antTheme->tokens().colorTextDisabled,
                                 cache.metrics.trackHeight / 2);
            Q_EMIT clicked(m_checked);
        }
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void AntSwitch::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        setCursor(isEnabled() && !m_loading ? Qt::PointingHandCursor : Qt::ArrowCursor);
        updateSwitchRegion(switchTrackDirtyRect(), QStringLiteral("enabled"));
    }
    else if (event->type() == QEvent::FontChange)
    {
        updateGeometryFromState();
        updateSwitchRegion(switchTrackDirtyRect(), QStringLiteral("font"));
    }
    QWidget::changeEvent(event);
}

void AntSwitch::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && isEnabled() && !m_loading)
    {
        setChecked(!m_checked);
        Q_EMIT clicked(m_checked);
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void AntSwitch::showEvent(QShowEvent* event)
{
    updateLoadingTimerState();
    QWidget::showEvent(event);
}

void AntSwitch::hideEvent(QHideEvent* event)
{
    updateLoadingTimerState();
    QWidget::hideEvent(event);
}

void AntSwitch::resizeEvent(QResizeEvent* event)
{
    invalidateLayoutCache();
    QWidget::resizeEvent(event);
}

AntSwitch::Metrics AntSwitch::metrics() const
{
    if (m_layoutCache.valid)
    {
        return m_layoutCache.metrics;
    }

    const auto& token = antTheme->tokens();
    Metrics m;
    const int fontSize = token.fontSize > 0 ? token.fontSize : Ant::FontSize;
    const int fontSizeSM = token.fontSizeSM > 0 ? token.fontSizeSM : Ant::FontSizeSmall;
    const int controlHeight = token.controlHeight > 0 ? token.controlHeight : Ant::ControlHeight;
    const qreal lineHeight = token.lineHeight > 0.0 ? token.lineHeight : 1.5715;
    const qreal height = fontSize * lineHeight;
    if (m_switchSize == Ant::Size::Small)
    {
        m.trackHeight = controlHeight / 2;
        m.trackPadding = 2;
        m.handleSize = m.trackHeight - m.trackPadding * 2;
        m.trackMinWidth = m.handleSize * 2 + m.trackPadding * 2;
        m.fontSize = fontSizeSM;
        m.innerMinMargin = m.handleSize / 2;
        m.innerMaxMargin = m.handleSize + m.trackPadding * 3;
    }
    else
    {
        m.trackHeight = static_cast<int>(std::round(height));
        m.trackPadding = 2;
        m.handleSize = m.trackHeight - m.trackPadding * 2;
        m.trackMinWidth = m.handleSize * 2 + m.trackPadding * 4;
        m.fontSize = fontSizeSM;
        m.innerMinMargin = m.handleSize / 2;
        m.innerMaxMargin = m.handleSize + m.trackPadding * 3;
    }

    QFont f = font();
    f.setPixelSize(m.fontSize);
    QFontMetrics fm(f);
    const int labelWidth = std::max(fm.horizontalAdvance(m_checkedText), fm.horizontalAdvance(m_uncheckedText));
    if (labelWidth > 0)
    {
        m.trackMinWidth = std::max(m.trackMinWidth, labelWidth + m.handleSize + m.trackPadding * 8);
    }

    ++m_metricsResolveCount;
    syncSwitchPerfCounters();
    return m;
}

const AntSwitch::LayoutCache& AntSwitch::layoutCache() const
{
    const Metrics currentMetrics = metrics();
    const auto sameMetrics = [](const Metrics& lhs, const Metrics& rhs) {
        return lhs.trackHeight == rhs.trackHeight
            && lhs.trackMinWidth == rhs.trackMinWidth
            && lhs.trackPadding == rhs.trackPadding
            && lhs.handleSize == rhs.handleSize
            && lhs.fontSize == rhs.fontSize
            && lhs.innerMinMargin == rhs.innerMinMargin
            && lhs.innerMaxMargin == rhs.innerMaxMargin;
    };

    if (m_layoutCache.valid
        && m_layoutCache.widgetSize == size()
        && sameMetrics(m_layoutCache.metrics, currentMetrics)
        && m_layoutCache.switchSize == m_switchSize
        && m_layoutCache.checkedText == m_checkedText
        && m_layoutCache.uncheckedText == m_uncheckedText
        && std::abs(m_layoutCache.handleProgress - m_handleProgress) < 0.0001
        && std::abs(m_layoutCache.handleStretch - m_handleStretch) < 0.0001)
    {
        return m_layoutCache;
    }

    m_layoutCache.widgetSize = size();
    m_layoutCache.metrics = currentMetrics;
    m_layoutCache.switchSize = m_switchSize;
    m_layoutCache.checkedText = m_checkedText;
    m_layoutCache.uncheckedText = m_uncheckedText;
    m_layoutCache.handleProgress = m_handleProgress;
    m_layoutCache.handleStretch = m_handleStretch;
    m_layoutCache.sizeHint = QSize(currentMetrics.trackMinWidth, currentMetrics.trackHeight);
    m_layoutCache.minimumSizeHint = m_layoutCache.sizeHint;
    m_layoutCache.trackRect = QRectF((width() - currentMetrics.trackMinWidth) / 2.0,
                                     (height() - currentMetrics.trackHeight) / 2.0,
                                     currentMetrics.trackMinWidth,
                                     currentMetrics.trackHeight);
    m_layoutCache.handleRect = handleRectForState(m_layoutCache, m_handleProgress, m_handleStretch);
    m_layoutCache.valid = true;
    ++m_layoutBuildCount;
    ++m_sizeHintResolveCount;
    syncSwitchPerfCounters();
    return m_layoutCache;
}

QRectF AntSwitch::handleRectForState(const LayoutCache& cache, qreal progress, qreal stretch) const
{
    const Metrics& m = cache.metrics;
    const QRectF& track = cache.trackRect;
    const qreal left = track.left() + m.trackPadding;
    const qreal right = track.right() - m.trackPadding - m.handleSize;
    qreal x = left + (right - left) * std::clamp(progress, 0.0, 1.0);
    const qreal width = m.handleSize + m.trackPadding * 2 * std::clamp(stretch, 0.0, 1.0);

    if (m_checked)
    {
        x -= m.trackPadding * 2 * std::clamp(stretch, 0.0, 1.0);
    }

    return QRectF(x, track.top() + m.trackPadding, width, m.handleSize);
}

QRect AntSwitch::switchTrackDirtyRect() const
{
    const auto& cache = layoutCache();
    return cache.trackRect
        .adjusted(-4, -4, 4, 4)
        .toAlignedRect()
        .intersected(rect());
}

QRect AntSwitch::switchHandleDirtyRect(qreal oldProgress, qreal oldStretch) const
{
    const auto& cache = layoutCache();
    QRectF dirty = handleRectForState(cache, oldProgress, oldStretch).united(cache.handleRect);
    if (!m_checkedText.isEmpty() || !m_uncheckedText.isEmpty())
    {
        dirty = dirty.united(cache.trackRect);
    }
    return dirty
        .adjusted(-6, -6, 6, 6)
        .toAlignedRect()
        .intersected(rect());
}

QRect AntSwitch::switchLoadingDirtyRect() const
{
    const auto& cache = layoutCache();
    return cache.handleRect
        .adjusted(-4, -4, 4, 4)
        .toAlignedRect()
        .intersected(rect());
}

void AntSwitch::updateSwitchRegion(const QRect& dirty, const QString& mode)
{
    QRect updateRect = dirty;
    if (!updateRect.isValid() || updateRect.isEmpty())
    {
        updateRect = rect();
    }
    ++m_regionUpdateCount;
    setProperty("antSwitchLastUpdateMode", mode);
    syncSwitchPerfCounters();
    update(updateRect);
}

void AntSwitch::invalidateLayoutCache() const
{
    m_layoutCache.valid = false;
    syncSwitchPerfCounters();
}

void AntSwitch::updateLoadingTimerState()
{
    const bool shouldRun = m_loading && isVisible();
    if (shouldRun && !m_loadingTimer->isActive())
    {
        m_loadingTimer->start(80);
    }
    else if (!shouldRun && m_loadingTimer->isActive())
    {
        m_loadingTimer->stop();
    }
    setProperty("antSwitchLoadingTimerActive", m_loadingTimer->isActive());
}

void AntSwitch::syncSwitchPerfCounters() const
{
    auto* self = const_cast<AntSwitch*>(this);
    self->setProperty("antSwitchLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antSwitchMetricsResolveCount", m_metricsResolveCount);
    self->setProperty("antSwitchSizeHintResolveCount", m_sizeHintResolveCount);
    self->setProperty("antSwitchRegionUpdateCount", m_regionUpdateCount);
    self->setProperty("antSwitchHandleRegionUpdateCount", m_handleRegionUpdateCount);
    self->setProperty("antSwitchLoadingRegionUpdateCount", m_loadingRegionUpdateCount);
    self->setProperty("antSwitchLoadingTimerActive", m_loadingTimer ? m_loadingTimer->isActive() : false);
}

void AntSwitch::animateToChecked(bool checked)
{
    m_progressAnimation->stop();
    m_progressAnimation->setStartValue(m_handleProgress);
    m_progressAnimation->setEndValue(checked ? 1.0 : 0.0);
    m_progressAnimation->start();
}

void AntSwitch::animateStretch(qreal endValue)
{
    m_stretchAnimation->stop();
    m_stretchAnimation->setStartValue(m_handleStretch);
    m_stretchAnimation->setEndValue(endValue);
    m_stretchAnimation->start();
}

void AntSwitch::updateGeometryFromState(bool notifyGeometry)
{
    const QSize oldMinimum = minimumSize();
    invalidateLayoutCache();
    const QSize newHint = sizeHint();
    if (oldMinimum != newHint)
    {
        setMinimumSize(newHint);
        if (notifyGeometry)
        {
            updateGeometry();
        }
    }
}
