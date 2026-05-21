#include "AntSkeleton.h"

#include <QEvent>
#include <QHideEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QTimer>
#include <utility>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "styles/AntSkeletonStyle.h"

AntSkeleton::AntSkeleton(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntSkeletonStyle>(this);
    setAttribute(Qt::WA_Hover, false);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        const int oldOffset = m_shimmerOffset;
        m_shimmerOffset = (m_shimmerOffset + 12) % qMax(180, width() + 180);
        requestSkeletonUpdate(shimmerDirtyRect(oldOffset, m_shimmerOffset), QStringLiteral("shimmer"));
    });

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateSkeletonLayout();
        updateGeometry();
        requestSkeletonUpdate(rect(), QStringLiteral("theme"));
        updateTimerState();
    });

    updateTimerState();
    syncSkeletonPerfCounters();
}

bool AntSkeleton::isActive() const { return m_active; }

void AntSkeleton::setActive(bool active)
{
    if (m_active == active)
    {
        return;
    }
    m_active = active;
    updateTimerState();
    requestSkeletonUpdate(skeletonVisualRect(), QStringLiteral("active"));
    Q_EMIT activeChanged(m_active);
}

bool AntSkeleton::isLoading() const { return m_loading; }

void AntSkeleton::setLoading(bool loading)
{
    if (m_loading == loading)
    {
        return;
    }
    m_loading = loading;
    invalidateSkeletonLayout();
    if (m_contentWidget)
    {
        m_contentWidget->setVisible(!m_loading);
    }
    updateTimerState();
    updateGeometry();
    requestSkeletonUpdate(rect(), QStringLiteral("loading"));
    Q_EMIT loadingChanged(m_loading);
}

bool AntSkeleton::isRound() const { return m_round; }

void AntSkeleton::setRound(bool round)
{
    if (m_round == round)
    {
        return;
    }
    m_round = round;
    invalidateSkeletonLayout();
    requestSkeletonUpdate(skeletonVisualRect(), QStringLiteral("round"));
    Q_EMIT roundChanged(m_round);
}

bool AntSkeleton::avatarVisible() const { return m_avatarVisible; }

void AntSkeleton::setAvatarVisible(bool visible)
{
    if (m_avatarVisible == visible)
    {
        return;
    }
    m_avatarVisible = visible;
    invalidateSkeletonLayout();
    updateGeometry();
    requestSkeletonUpdate(rect(), QStringLiteral("avatarVisible"));
    Q_EMIT avatarVisibleChanged(m_avatarVisible);
}

Ant::AvatarShape AntSkeleton::avatarShape() const { return m_avatarShape; }

void AntSkeleton::setAvatarShape(Ant::AvatarShape shape)
{
    if (m_avatarShape == shape)
    {
        return;
    }
    m_avatarShape = shape;
    invalidateSkeletonLayout();
    requestSkeletonUpdate(skeletonVisualRect(), QStringLiteral("avatarShape"));
    Q_EMIT avatarShapeChanged(m_avatarShape);
}

bool AntSkeleton::titleVisible() const { return m_titleVisible; }

void AntSkeleton::setTitleVisible(bool visible)
{
    if (m_titleVisible == visible)
    {
        return;
    }
    m_titleVisible = visible;
    invalidateSkeletonLayout();
    updateGeometry();
    requestSkeletonUpdate(rect(), QStringLiteral("titleVisible"));
    Q_EMIT titleVisibleChanged(m_titleVisible);
}

bool AntSkeleton::paragraphVisible() const { return m_paragraphVisible; }

void AntSkeleton::setParagraphVisible(bool visible)
{
    if (m_paragraphVisible == visible)
    {
        return;
    }
    m_paragraphVisible = visible;
    invalidateSkeletonLayout();
    updateGeometry();
    requestSkeletonUpdate(rect(), QStringLiteral("paragraphVisible"));
    Q_EMIT paragraphVisibleChanged(m_paragraphVisible);
}

int AntSkeleton::paragraphRows() const { return m_paragraphRows; }

void AntSkeleton::setParagraphRows(int rows)
{
    rows = qMax(0, rows);
    if (m_paragraphRows == rows)
    {
        return;
    }
    m_paragraphRows = rows;
    invalidateSkeletonLayout();
    updateGeometry();
    requestSkeletonUpdate(rect(), QStringLiteral("paragraphRows"));
    Q_EMIT paragraphRowsChanged(m_paragraphRows);
}

Ant::SkeletonElement AntSkeleton::element() const { return m_element; }

void AntSkeleton::setElement(Ant::SkeletonElement element)
{
    if (m_element == element)
    {
        return;
    }
    m_element = element;
    invalidateSkeletonLayout();
    updateGeometry();
    requestSkeletonUpdate(rect(), QStringLiteral("element"));
    Q_EMIT elementChanged(m_element);
}

QWidget* AntSkeleton::contentWidget() const
{
    return m_contentWidget.data();
}

void AntSkeleton::setContentWidget(QWidget* widget)
{
    if (m_contentWidget == widget)
    {
        return;
    }
    if (m_contentWidget)
    {
        m_contentWidget->hide();
        m_contentWidget->setParent(nullptr);
    }
    m_contentWidget = widget;
    if (m_contentWidget)
    {
        m_contentWidget->setParent(this);
        m_contentWidget->setVisible(!m_loading);
    }
    invalidateSkeletonLayout();
    syncContentGeometry();
    updateGeometry();
    requestSkeletonUpdate(rect(), QStringLiteral("content"));
}

void AntSkeleton::setTitleWidthRatio(qreal ratio)
{
    ratio = qBound(0.15, ratio, 1.0);
    if (qFuzzyCompare(m_titleWidthRatio, ratio))
    {
        return;
    }
    m_titleWidthRatio = ratio;
    invalidateSkeletonLayout();
    requestSkeletonUpdate(skeletonVisualRect(), QStringLiteral("titleWidth"));
}

qreal AntSkeleton::titleWidthRatio() const
{
    return m_titleWidthRatio;
}

void AntSkeleton::setParagraphWidthRatios(const QList<qreal>& ratios)
{
    if (m_paragraphWidthRatios == ratios)
    {
        return;
    }
    m_paragraphWidthRatios = ratios;
    invalidateSkeletonLayout();
    requestSkeletonUpdate(skeletonVisualRect(), QStringLiteral("paragraphWidths"));
}

QList<qreal> AntSkeleton::paragraphWidthRatios() const
{
    return m_paragraphWidthRatios;
}

int AntSkeleton::shimmerOffset() const
{
    return m_shimmerOffset;
}

QSize AntSkeleton::sizeHint() const
{
    return skeletonLayout().sizeHint;
}

QSize AntSkeleton::minimumSizeHint() const
{
    return skeletonLayout().minimumSizeHint;
}

void AntSkeleton::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        updateTimerState();
        requestSkeletonUpdate(skeletonVisualRect(), QStringLiteral("enabled"));
    }
    QWidget::changeEvent(event);
}

void AntSkeleton::showEvent(QShowEvent* event)
{
    updateTimerState();
    QWidget::showEvent(event);
}

void AntSkeleton::hideEvent(QHideEvent* event)
{
    updateTimerState();
    QWidget::hideEvent(event);
}

void AntSkeleton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntSkeleton::resizeEvent(QResizeEvent* event)
{
    invalidateSkeletonLayout();
    syncContentGeometry();
    updateTimerState();
    QWidget::resizeEvent(event);
}

const AntSkeleton::SkeletonLayoutCache& AntSkeleton::skeletonLayout() const
{
    const auto& token = antTheme->tokens();
    const QSize contentHint = m_contentWidget ? m_contentWidget->sizeHint() : QSize();
    if (m_layoutCache.valid &&
        m_layoutCache.widgetSize == size() &&
        m_layoutCache.themeMode == antTheme->themeMode() &&
        m_layoutCache.tokenControlHeightLG == token.controlHeightLG &&
        m_layoutCache.tokenFontSizeLG == token.fontSizeLG &&
        m_layoutCache.tokenFontSizeSM == token.fontSizeSM &&
        m_layoutCache.tokenMarginSM == token.marginSM &&
        m_layoutCache.tokenMargin == token.margin &&
        m_layoutCache.tokenBorderRadiusSM == token.borderRadiusSM &&
        m_layoutCache.loading == m_loading &&
        m_layoutCache.active == m_active &&
        m_layoutCache.round == m_round &&
        m_layoutCache.avatarVisible == m_avatarVisible &&
        m_layoutCache.avatarShape == m_avatarShape &&
        m_layoutCache.titleVisible == m_titleVisible &&
        m_layoutCache.paragraphVisible == m_paragraphVisible &&
        m_layoutCache.paragraphRows == m_paragraphRows &&
        m_layoutCache.element == m_element &&
        qFuzzyCompare(m_layoutCache.titleWidthRatio, m_titleWidthRatio) &&
        m_layoutCache.paragraphWidthRatios == m_paragraphWidthRatios &&
        m_layoutCache.hasContentWidget == static_cast<bool>(m_contentWidget) &&
        m_layoutCache.contentSizeHint == contentHint)
    {
        ++m_layoutCacheHitCount;
        syncSkeletonPerfCounters();
        return m_layoutCache;
    }

    ++m_layoutBuildCount;
    SkeletonLayoutCache cache;
    cache.valid = true;
    cache.widgetSize = size();
    cache.themeMode = antTheme->themeMode();
    cache.tokenControlHeightLG = token.controlHeightLG;
    cache.tokenFontSizeLG = token.fontSizeLG;
    cache.tokenFontSizeSM = token.fontSizeSM;
    cache.tokenMarginSM = token.marginSM;
    cache.tokenMargin = token.margin;
    cache.tokenBorderRadiusSM = token.borderRadiusSM;
    cache.loading = m_loading;
    cache.active = m_active;
    cache.round = m_round;
    cache.avatarVisible = m_avatarVisible;
    cache.avatarShape = m_avatarShape;
    cache.titleVisible = m_titleVisible;
    cache.paragraphVisible = m_paragraphVisible;
    cache.paragraphRows = m_paragraphRows;
    cache.element = m_element;
    cache.titleWidthRatio = m_titleWidthRatio;
    cache.paragraphWidthRatios = m_paragraphWidthRatios;
    cache.hasContentWidget = static_cast<bool>(m_contentWidget);
    cache.contentSizeHint = contentHint;
    cache.metrics = resolveMetrics();
    cache.sizeHint = skeletonSizeHint(cache.metrics);
    cache.minimumSizeHint = QSize(180, 24);
    cache.baseColor = token.colorFillTertiary;
    cache.highlightColor = AntPalette::mix(token.colorBgContainer, token.colorFillQuaternary, 0.45);

    if (m_loading)
    {
        const Metrics& m = cache.metrics;
        if (m_element != Ant::SkeletonElement::Default)
        {
            const QSize hint = cache.sizeHint;
            const qreal w = qMin(static_cast<qreal>(width()), static_cast<qreal>(hint.width()));
            const qreal h = hint.height();
            switch (m_element)
            {
            case Ant::SkeletonElement::Button:
                appendPlaceholder(cache, QRectF(0, 0, w, h));
                break;
            case Ant::SkeletonElement::Avatar:
                appendPlaceholder(cache, QRectF(0, 0, m.avatarSize, m.avatarSize));
                break;
            case Ant::SkeletonElement::Input:
                appendPlaceholder(cache, QRectF(0, 0, w, h));
                break;
            case Ant::SkeletonElement::Image:
            {
                const qreal sz = qMin(w, h);
                appendPlaceholder(cache, QRectF((w - sz) / 2, 0, sz, sz), true);
                break;
            }
            case Ant::SkeletonElement::Node:
                appendPlaceholder(cache, QRectF(0, 0, w, h));
                break;
            default:
                break;
            }
        }
        else
        {
            int textLeft = 0;
            int textWidth = width();
            const int totalHeight = cache.sizeHint.height();
            int top = 0;

            if (m_avatarVisible)
            {
                const int avatarY = qMax(0, (totalHeight - m.avatarSize) / 2);
                appendPlaceholder(cache, QRectF(0, avatarY, m.avatarSize, m.avatarSize));
                textLeft = m.avatarSize + m.columnGap;
                textWidth = qMax(40, width() - textLeft);
            }

            if (m_titleVisible)
            {
                const int titleWidth = qMax(48, static_cast<int>(textWidth * m_titleWidthRatio));
                appendPlaceholder(cache, QRectF(textLeft, top + m.titleTop, titleWidth, m.titleHeight));
                top += m.titleHeight;
            }

            if (m_paragraphVisible && m_paragraphRows > 0)
            {
                if (m_titleVisible)
                {
                    top += token.marginSM;
                }
                for (int i = 0; i < m_paragraphRows; ++i)
                {
                    const int rowWidth = qMax(56, static_cast<int>(textWidth * rowWidthRatio(i)));
                    appendPlaceholder(cache, QRectF(textLeft, top, rowWidth, m.paragraphHeight));
                    top += m.paragraphHeight + m.rowSpacing;
                }
            }
        }

        for (const PlaceholderItem& item : std::as_const(cache.placeholders))
        {
            cache.visualRect = cache.visualRect.united(item.rect.toAlignedRect().adjusted(-2, -2, 2, 2));
        }
        cache.visualRect = cache.visualRect.intersected(rect());
    }

    m_layoutCache = cache;
    syncSkeletonPerfCounters();
    return m_layoutCache;
}

void AntSkeleton::invalidateSkeletonLayout() const
{
    m_layoutCache.valid = false;
}

AntSkeleton::Metrics AntSkeleton::resolveMetrics() const
{
    Metrics m;
    const auto& token = antTheme->tokens();
    m.avatarSize = token.controlHeightLG;
    m.titleHeight = token.fontSizeLG;
    m.paragraphHeight = token.fontSizeSM + 2;
    m.rowSpacing = token.marginSM;
    m.columnGap = token.margin;
    m.radius = token.borderRadiusSM;
    return m;
}

QSize AntSkeleton::skeletonSizeHint(const Metrics& m) const
{
    if (!m_loading && m_contentWidget)
    {
        return m_contentWidget->sizeHint();
    }

    if (m_element != Ant::SkeletonElement::Default)
    {
        switch (m_element)
        {
        case Ant::SkeletonElement::Button:
            return QSize(120, 32);
        case Ant::SkeletonElement::Avatar:
            return QSize(m.avatarSize, m.avatarSize);
        case Ant::SkeletonElement::Input:
            return QSize(200, 32);
        case Ant::SkeletonElement::Image:
            return QSize(200, 200);
        case Ant::SkeletonElement::Node:
            if (m_contentWidget)
            {
                return m_contentWidget->sizeHint();
            }
            return QSize(200, 100);
        default:
            break;
        }
    }

    int height = 0;

    if (m_avatarVisible)
    {
        height = qMax(height, m.avatarSize);
    }

    int textHeight = 0;
    if (m_titleVisible)
    {
        textHeight += m.titleHeight;
    }
    if (m_paragraphVisible && m_paragraphRows > 0)
    {
        if (textHeight > 0)
        {
            textHeight += m.rowSpacing;
        }
        textHeight += m_paragraphRows * m.paragraphHeight;
        textHeight += qMax(0, m_paragraphRows - 1) * m.rowSpacing;
    }

    height = m_avatarVisible ? qMax(height, textHeight) : textHeight;
    return QSize(320, qMax(24, height));
}

void AntSkeleton::appendPlaceholder(SkeletonLayoutCache& cache, const QRectF& rect, bool imageElement) const
{
    PlaceholderItem item;
    item.rect = rect;
    item.imageElement = imageElement;

    const Metrics& m = cache.metrics;
    item.radius = m_round ? rect.height() / 2.0 : m.radius;
    if (m_element == Ant::SkeletonElement::Avatar ||
        (m_element == Ant::SkeletonElement::Default &&
         m_avatarVisible &&
         qFuzzyCompare(rect.height(), static_cast<qreal>(m.avatarSize)) &&
         qFuzzyCompare(rect.width(), static_cast<qreal>(m.avatarSize)) &&
         m_avatarShape == Ant::AvatarShape::Circle))
    {
        item.radius = rect.width() / 2.0;
    }
    if (m_element == Ant::SkeletonElement::Button || m_element == Ant::SkeletonElement::Input)
    {
        item.radius = rect.height() / 2.0;
    }
    item.path.addRoundedRect(rect, item.radius, item.radius);
    cache.placeholders.append(item);
}

qreal AntSkeleton::rowWidthRatio(int rowIndex) const
{
    if (rowIndex >= 0 && rowIndex < m_paragraphWidthRatios.size())
    {
        return qBound(0.15, m_paragraphWidthRatios.at(rowIndex), 1.0);
    }
    if (rowIndex == m_paragraphRows - 1)
    {
        return 0.62;
    }
    return 1.0;
}

QRect AntSkeleton::skeletonVisualRect() const
{
    const QRect visual = skeletonLayout().visualRect;
    return visual.isEmpty() ? rect() : visual;
}

QRect AntSkeleton::shimmerDirtyRect(int oldOffset, int newOffset) const
{
    const auto& layout = skeletonLayout();
    if (layout.placeholders.isEmpty())
    {
        return rect();
    }

    QRect dirty;
    for (const PlaceholderItem& item : layout.placeholders)
    {
        dirty = dirty.united(shimmerBandRectForOffset(item.rect, oldOffset));
        dirty = dirty.united(shimmerBandRectForOffset(item.rect, newOffset));
    }
    if (dirty.isEmpty())
    {
        dirty = layout.visualRect;
    }
    return dirty.adjusted(-2, -2, 2, 2).intersected(rect());
}

QRect AntSkeleton::shimmerBandRectForOffset(const QRectF& rect, int offset) const
{
    const qreal bandWidth = qMax<qreal>(80.0, rect.width() * 0.72);
    const qreal travel = rect.width() + bandWidth * 2.0;
    const qreal phase = static_cast<qreal>(offset % qMax(1, static_cast<int>(travel))) / travel;
    const qreal bandLeft = rect.left() - bandWidth + travel * phase;
    return QRectF(bandLeft, rect.top(), bandWidth, rect.height()).intersected(rect).toAlignedRect();
}

void AntSkeleton::requestSkeletonUpdate(const QRect& region, const QString& mode)
{
    m_lastUpdateMode = mode;
    if (mode == QStringLiteral("shimmer"))
    {
        ++m_shimmerRegionUpdateCount;
    }
    else
    {
        ++m_visualRegionUpdateCount;
    }
    syncSkeletonPerfCounters();

    if (region.isValid() && !region.isEmpty())
    {
        update(region);
        return;
    }
    update();
}

void AntSkeleton::syncSkeletonPerfCounters() const
{
    auto* self = const_cast<AntSkeleton*>(this);
    self->setProperty("antSkeletonLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antSkeletonLayoutCacheHitCount", m_layoutCacheHitCount);
    self->setProperty("antSkeletonShimmerRegionUpdateCount", m_shimmerRegionUpdateCount);
    self->setProperty("antSkeletonVisualRegionUpdateCount", m_visualRegionUpdateCount);
    self->setProperty("antSkeletonContentGeometryApplyCount", m_contentGeometryApplyCount);
    self->setProperty("antSkeletonContentGeometrySkipCount", m_contentGeometrySkipCount);
    self->setProperty("antSkeletonTimerRunning", m_timer && m_timer->isActive());
    self->setProperty("antSkeletonLastUpdateMode", m_lastUpdateMode);
}

void AntSkeleton::syncContentGeometry()
{
    if (m_contentWidget)
    {
        if (m_contentWidget->geometry() == rect())
        {
            ++m_contentGeometrySkipCount;
            syncSkeletonPerfCounters();
            return;
        }
        ++m_contentGeometryApplyCount;
        m_contentWidget->setGeometry(rect());
        syncSkeletonPerfCounters();
    }
}

void AntSkeleton::updateTimerState()
{
    const bool shouldAnimate = isVisible() && isEnabled() && m_active && m_loading && !skeletonLayout().placeholders.isEmpty();
    if (shouldAnimate && !m_timer->isActive())
    {
        m_timer->start(40);
    }
    else if (!shouldAnimate)
    {
        m_timer->stop();
    }
    syncSkeletonPerfCounters();
}
