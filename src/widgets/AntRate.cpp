#include "AntRate.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QEasingCurve>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QStyle>
#include <QToolTip>
#include <QVariantAnimation>
#include <QtMath>

#include <algorithm>
#include <cmath>

#include "../styles/AntRateStyle.h"
#include "core/AntTheme.h"

namespace
{
constexpr int kSelectionPadding = 4;

int starSizeFor(Ant::Size size)
{
    const auto& token = antTheme->tokens();
    switch (size)
    {
    case Ant::Size::Small:
        return static_cast<int>(std::round(token.controlHeightSM * 0.625));
    case Ant::Size::Large:
        return static_cast<int>(std::round(token.controlHeightLG * 0.625));
    case Ant::Size::Middle:
    default:
        return static_cast<int>(std::round(token.controlHeight * 0.625));
    }
}

QPainterPath starPathForRect(const QRectF& rect)
{
    QPainterPath path;
    const qreal cx = rect.center().x();
    const qreal cy = rect.center().y();
    const qreal outerR = std::min(rect.width(), rect.height()) / 2.0;
    const qreal innerR = outerR * 0.4;

    for (int i = 0; i < 10; ++i)
    {
        const qreal r = (i % 2 == 0) ? outerR : innerR;
        const qreal angle = M_PI / 2.0 - i * 2.0 * M_PI / 10.0;
        const qreal x = cx + r * std::cos(angle);
        const qreal y = cy - r * std::sin(angle);
        if (i == 0)
        {
            path.moveTo(x, y);
        }
        else
        {
            path.lineTo(x, y);
        }
    }
    path.closeSubpath();
    return path;
}
}

AntRate::AntRate(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        const QSize oldHint = sizeHint();
        invalidateLayoutCache();
        if (oldHint != sizeHint())
        {
            updateGeometry();
        }
        update();
    });

    m_selectionAnimation = new QVariantAnimation(this);
    m_selectionAnimation->setObjectName(QStringLiteral("antRateSelectionScaleAnimation"));
    m_selectionAnimation->setDuration(220);
    m_selectionAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_selectionAnimation->setKeyValueAt(0.0, 1.0);
    m_selectionAnimation->setKeyValueAt(0.45, 1.18);
    m_selectionAnimation->setKeyValueAt(1.0, 1.0);
    connect(m_selectionAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        const int animatedIndex = m_selectionAnimationIndex;
        m_selectionScale = value.toReal();
        updateStarRegion(animatedIndex);
    });
    connect(m_selectionAnimation, &QVariantAnimation::finished, this, [this]() {
        const int animatedIndex = m_selectionAnimationIndex;
        m_selectionAnimationIndex = -1;
        m_selectionScale = 1.0;
        updateStarRegion(animatedIndex);
    });

    auto* rateStyle = new AntRateStyle(style());
    rateStyle->setParent(this);
    setStyle(rateStyle);
    syncRatePerfCounters();
}

double AntRate::value() const
{
    return m_value;
}

void AntRate::setValue(double value)
{
    double newValue = std::clamp(value, 0.0, static_cast<double>(m_count));
    if (m_allowHalf)
    {
        newValue = std::round(newValue * 2.0) / 2.0;
    }
    else
    {
        newValue = std::round(newValue);
    }

    if (qFuzzyCompare(m_value, newValue))
    {
        return;
    }

    const double oldValue = m_value;
    m_value = newValue;
    updateValueRegion(oldValue, m_value);
    Q_EMIT valueChanged(m_value);
}

int AntRate::count() const
{
    return m_count;
}

void AntRate::setCount(int count)
{
    if (count < 1 || m_count == count)
    {
        return;
    }

    m_count = count;
    setValue(m_value);
    invalidateLayoutCache();
    updateGeometry();
    update();
    Q_EMIT countChanged(m_count);
}

bool AntRate::allowHalf() const
{
    return m_allowHalf;
}

void AntRate::setAllowHalf(bool allow)
{
    if (m_allowHalf == allow)
    {
        return;
    }

    m_allowHalf = allow;
    setValue(m_value);
    updateValueRegion(0.0, static_cast<double>(m_count));
    Q_EMIT allowHalfChanged(m_allowHalf);
}

bool AntRate::allowClear() const
{
    return m_allowClear;
}

void AntRate::setAllowClear(bool allow)
{
    if (m_allowClear == allow)
    {
        return;
    }

    m_allowClear = allow;
    Q_EMIT allowClearChanged(m_allowClear);
}

bool AntRate::isDisabled() const
{
    return m_disabled;
}

void AntRate::setDisabled(bool disabled)
{
    if (m_disabled == disabled)
    {
        return;
    }

    m_disabled = disabled;
    setCursor(m_disabled ? Qt::ArrowCursor : Qt::PointingHandCursor);
    updateValueRegion(0.0, static_cast<double>(m_count));
    Q_EMIT disabledChanged(m_disabled);
}

Ant::Size AntRate::rateSize() const
{
    return m_rateSize;
}

void AntRate::setRateSize(Ant::Size size)
{
    if (m_rateSize == size)
    {
        return;
    }

    m_rateSize = size;
    invalidateLayoutCache();
    updateGeometry();
    update();
    Q_EMIT rateSizeChanged(m_rateSize);
}

double AntRate::hoverValue() const
{
    return m_hoverValue;
}

bool AntRate::isHoveredState() const
{
    return m_hovered;
}

QSize AntRate::sizeHint() const
{
    return cachedSizeHint();
}

QSize AntRate::minimumSizeHint() const
{
    return sizeHint();
}

void AntRate::enterEvent(AntEnterEvent* event)
{
    if (m_hovered)
    {
        QWidget::enterEvent(event);
        return;
    }
    m_hovered = true;
    updateValueRegion(hoverValue(), hoverValue());
    QWidget::enterEvent(event);
}

void AntRate::leaveEvent(QEvent* event)
{
    if (!m_hovered && m_hoverValue < 0.0)
    {
        QWidget::leaveEvent(event);
        return;
    }
    const double oldHover = m_hoverValue;
    m_hovered = false;
    m_hoverValue = -1.0;
    Q_EMIT hoverChanged(m_hoverValue);
    updateValueRegion(oldHover, m_value);
    QWidget::leaveEvent(event);
}

void AntRate::mouseMoveEvent(QMouseEvent* event)
{
    if (m_disabled || !isEnabled())
    {
        QWidget::mouseMoveEvent(event);
        return;
    }

    updateHoverValue(event->pos());
    QWidget::mouseMoveEvent(event);
}

void AntRate::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && !m_disabled && isEnabled())
    {
        m_pressed = true;
        updateHoverValue(event->pos());
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntRate::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed)
    {
        m_pressed = false;
        if (rect().contains(event->pos()))
        {
            updateHoverValue(event->pos());
            const double clickedValue = m_hoverValue >= 0.0 ? m_hoverValue : m_value;
            if (m_allowClear && qFuzzyCompare(clickedValue, m_value) && m_value > 0.0)
            {
                setValue(0.0);
            }
            else
            {
                setValue(clickedValue);
                startSelectionAnimation(clickedValue);
            }
        }
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void AntRate::keyPressEvent(QKeyEvent* event)
{
    if (m_disabled || !isEnabled())
    {
        QWidget::keyPressEvent(event);
        return;
    }

    const double step = m_allowHalf ? 0.5 : 1.0;
    switch (event->key())
    {
        case Qt::Key_Left:
            setValue(m_value - step);
            event->accept();
            return;
        case Qt::Key_Right:
            setValue(m_value + step);
            event->accept();
            return;
        case Qt::Key_Home:
            setValue(0.0);
            event->accept();
            return;
        case Qt::Key_End:
            setValue(static_cast<double>(m_count));
            event->accept();
            return;
        default:
            break;
    }
    QWidget::keyPressEvent(event);
}

void AntRate::focusInEvent(QFocusEvent* event)
{
    m_focused = true;
    updateFocusRegion();
    QWidget::focusInEvent(event);
}

void AntRate::focusOutEvent(QFocusEvent* event)
{
    m_focused = false;
    updateFocusRegion();
    QWidget::focusOutEvent(event);
}

void AntRate::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        setCursor(isEnabled() && !m_disabled ? Qt::PointingHandCursor : Qt::ArrowCursor);
        updateValueRegion(0.0, static_cast<double>(m_count));
    }
    QWidget::changeEvent(event);
}

void AntRate::resizeEvent(QResizeEvent* event)
{
    invalidateLayoutCache();
    QWidget::resizeEvent(event);
}

const AntRate::LayoutCache& AntRate::layoutCache() const
{
    if (m_layoutCache.valid
        && m_layoutCache.widgetSize == size()
        && m_layoutCache.rateSize == m_rateSize
        && m_layoutCache.count == m_count
        && m_layoutCache.starSize == starSizeFor(m_rateSize)
        && m_layoutCache.margin == antTheme->tokens().marginXS)
    {
        return m_layoutCache;
    }

    m_layoutCache.widgetSize = size();
    m_layoutCache.rateSize = m_rateSize;
    m_layoutCache.count = m_count;
    m_layoutCache.starSize = starSizeFor(m_rateSize);
    m_layoutCache.margin = antTheme->tokens().marginXS;
    m_layoutCache.totalWidth = m_count * m_layoutCache.starSize + (m_count - 1) * m_layoutCache.margin;
    m_layoutCache.sizeHint = QSize(m_layoutCache.totalWidth + kSelectionPadding * 2,
                                   m_layoutCache.starSize + kSelectionPadding * 2);
    m_layoutCache.starRects.clear();
    m_layoutCache.starPaths.clear();
    m_layoutCache.starRects.reserve(m_count);
    m_layoutCache.starPaths.reserve(m_count);

    const qreal startX = kSelectionPadding;
    const qreal startY = (height() - m_layoutCache.starSize) / 2.0;
    for (int i = 0; i < m_count; ++i)
    {
        const qreal x = startX + i * (m_layoutCache.starSize + m_layoutCache.margin);
        const QRectF starRect(x, startY, m_layoutCache.starSize, m_layoutCache.starSize);
        m_layoutCache.starRects.append(starRect);
        m_layoutCache.starPaths.append(starPathForRect(starRect));
    }

    m_layoutCache.valid = true;
    ++m_layoutBuildCount;
    ++m_sizeHintResolveCount;
    m_starPathBuildCount += m_layoutCache.starPaths.size();
    syncRatePerfCounters();
    return m_layoutCache;
}

QSize AntRate::cachedSizeHint() const
{
    return layoutCache().sizeHint;
}

QRect AntRate::starDirtyRect(int index) const
{
    const LayoutCache& cache = layoutCache();
    if (index < 0 || index >= cache.starRects.size())
    {
        return QRect();
    }
    return cache.starRects.at(index).toAlignedRect().adjusted(-kSelectionPadding,
                                                             -kSelectionPadding,
                                                             kSelectionPadding,
                                                             kSelectionPadding)
            .intersected(rect());
}

QRect AntRate::valueDirtyRect(double oldValue, double newValue) const
{
    if (m_count <= 0)
    {
        return rect();
    }

    const auto toIndex = [this](double value) {
        if (value < 0.0)
        {
            value = m_value;
        }
        if (value <= 0.0)
        {
            return 0;
        }
        return std::clamp(static_cast<int>(std::ceil(value)) - 1, 0, m_count - 1);
    };

    int first = std::min(toIndex(oldValue), toIndex(newValue));
    int last = std::max(toIndex(oldValue), toIndex(newValue));
    QRect dirty;
    for (int i = first; i <= last; ++i)
    {
        dirty = dirty.united(starDirtyRect(i));
    }
    if (!dirty.isValid() || dirty.isEmpty())
    {
        dirty = rect();
    }
    return dirty.intersected(rect());
}

QRect AntRate::focusDirtyRect() const
{
    const LayoutCache& cache = layoutCache();
    if (cache.starRects.isEmpty())
    {
        return rect();
    }
    QRect dirty = cache.starRects.first().toAlignedRect().united(cache.starRects.last().toAlignedRect());
    return dirty.adjusted(-4, -4, 4, 4).intersected(rect());
}

void AntRate::updateValueRegion(double oldValue, double newValue)
{
    ++m_valueRegionUpdateCount;
    syncRatePerfCounters();
    update(valueDirtyRect(oldValue, newValue));
}

void AntRate::updateStarRegion(int index)
{
    if (index < 0)
    {
        return;
    }
    ++m_starRegionUpdateCount;
    syncRatePerfCounters();
    update(starDirtyRect(index));
}

void AntRate::updateFocusRegion()
{
    ++m_focusRegionUpdateCount;
    syncRatePerfCounters();
    update(focusDirtyRect());
}

void AntRate::invalidateLayoutCache() const
{
    m_layoutCache.valid = false;
    syncRatePerfCounters();
}

void AntRate::syncRatePerfCounters() const
{
    auto* self = const_cast<AntRate*>(this);
    self->setProperty("antRateLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antRateSizeHintResolveCount", m_sizeHintResolveCount);
    self->setProperty("antRateStarPathBuildCount", m_starPathBuildCount);
    self->setProperty("antRateValueRegionUpdateCount", m_valueRegionUpdateCount);
    self->setProperty("antRateStarRegionUpdateCount", m_starRegionUpdateCount);
    self->setProperty("antRateFocusRegionUpdateCount", m_focusRegionUpdateCount);
}

double AntRate::starValueAt(const QPoint& pos) const
{
    const LayoutCache& cache = layoutCache();
    const int starSize = cache.starSize;
    const int startY = (height() - starSize) / 2;

    if (pos.y() < startY - 4 || pos.y() > startY + starSize + 4)
    {
        return -1.0;
    }

    const int startX = kSelectionPadding;
    const int relativeX = pos.x() - startX;
    if (relativeX < 0)
    {
        return 0.0;
    }

    const int starWithMargin = starSize + cache.margin;
    int starIndex = relativeX / starWithMargin;
    if (starIndex >= m_count)
    {
        starIndex = m_count - 1;
    }

    const int withinStar = relativeX - starIndex * starWithMargin;
    if (withinStar > starSize)
    {
        return static_cast<double>(starIndex + 1);
    }

    double value = static_cast<double>(starIndex);
    if (m_allowHalf)
    {
        value += (withinStar >= starSize / 2) ? 1.0 : 0.5;
    }
    else
    {
        value += 1.0;
    }

    return std::clamp(value, 0.0, static_cast<double>(m_count));
}

void AntRate::updateHoverValue(const QPoint& pos)
{
    const double newHover = starValueAt(pos);
    if (!qFuzzyCompare(m_hoverValue, newHover))
    {
        const double oldHover = m_hoverValue;
        m_hoverValue = newHover;
        Q_EMIT hoverChanged(m_hoverValue);
        updateValueRegion(oldHover, m_hoverValue);
    }
}

void AntRate::startSelectionAnimation(double selectedValue)
{
    if (selectedValue <= 0.0 || m_disabled || !isEnabled())
    {
        return;
    }

    const int selectedIndex = std::clamp(static_cast<int>(std::ceil(selectedValue)) - 1, 0, m_count - 1);
    m_selectionAnimationIndex = selectedIndex;
    m_selectionScale = 1.0;
    m_selectionAnimation->stop();
    m_selectionAnimation->start();
    updateStarRegion(m_selectionAnimationIndex);
}
