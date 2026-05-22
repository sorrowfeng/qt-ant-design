#include "AntEmpty.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QResizeEvent>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "styles/AntEmptyStyle.h"

AntEmpty::AntEmpty(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntEmptyStyle>(this);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateEmptyCaches();
        updateGeometry();
        requestEmptyUpdate(rect(), QStringLiteral("theme"));
    });
    syncEmptyPerfCounters();
}

QString AntEmpty::description() const { return m_description; }

void AntEmpty::setDescription(const QString& description)
{
    if (m_description == description)
    {
        return;
    }
    m_description = description;
    invalidateEmptyCaches();
    updateGeometry();
    requestEmptyUpdate(descriptionRect(), QStringLiteral("description"));
    Q_EMIT descriptionChanged(m_description);
}

bool AntEmpty::imageVisible() const { return m_imageVisible; }

void AntEmpty::setImageVisible(bool visible)
{
    if (m_imageVisible == visible)
    {
        return;
    }
    m_imageVisible = visible;
    invalidateEmptyCaches();
    updateGeometry();
    syncExtraGeometry();
    requestEmptyUpdate(rect(), QStringLiteral("imageVisible"));
    Q_EMIT imageVisibleChanged(m_imageVisible);
}

bool AntEmpty::isSimple() const { return m_simple; }

void AntEmpty::setSimple(bool simple)
{
    if (m_simple == simple)
    {
        return;
    }
    m_simple = simple;
    invalidateEmptyCaches();
    updateGeometry();
    syncExtraGeometry();
    requestEmptyUpdate(rect(), QStringLiteral("simple"));
    Q_EMIT simpleChanged(m_simple);
}

QSize AntEmpty::imageSize() const { return m_imageSize; }

void AntEmpty::setImageSize(const QSize& size)
{
    const QSize clamped(qMax(48, size.width()), qMax(36, size.height()));
    if (m_imageSize == clamped)
    {
        return;
    }
    m_imageSize = clamped;
    invalidateEmptyCaches();
    updateGeometry();
    syncExtraGeometry();
    requestEmptyUpdate(rect(), QStringLiteral("imageSize"));
    Q_EMIT imageSizeChanged(m_imageSize);
}

QWidget* AntEmpty::extraWidget() const
{
    return m_extraWidget.data();
}

void AntEmpty::setExtraWidget(QWidget* widget)
{
    if (m_extraWidget == widget)
    {
        return;
    }
    if (m_extraWidget)
    {
        m_extraWidget->removeEventFilter(this);
        m_extraWidget->setParent(nullptr);
    }
    m_extraWidget = widget;
    if (m_extraWidget)
    {
        m_extraWidget->setParent(this);
        m_extraWidget->installEventFilter(this);
        m_extraWidget->show();
    }
    invalidateEmptyCaches();
    syncExtraGeometry();
    updateGeometry();
    requestEmptyUpdate(rect(), QStringLiteral("extra"));
}

QSize AntEmpty::sizeHint() const
{
    return cachedSizeHint();
}

QSize AntEmpty::minimumSizeHint() const
{
    return QSize(160, 120);
}

void AntEmpty::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

bool AntEmpty::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_extraWidget)
    {
        switch (event->type())
        {
        case QEvent::LayoutRequest:
        case QEvent::Resize:
        case QEvent::FontChange:
        case QEvent::StyleChange:
            invalidateEmptyCaches();
            syncExtraGeometry();
            updateGeometry();
            requestEmptyUpdate(rect(), QStringLiteral("extra"));
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntEmpty::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange)
    {
        invalidateEmptyCaches();
        updateGeometry();
        syncExtraGeometry();
        requestEmptyUpdate(rect(), QStringLiteral("style"));
    }
}

void AntEmpty::resizeEvent(QResizeEvent* event)
{
    syncExtraGeometry();
    QWidget::resizeEvent(event);
}

QRect AntEmpty::imageRect() const
{
    return emptyLayoutCache(size()).imageRect;
}

QRect AntEmpty::descriptionRect() const
{
    return emptyLayoutCache(size()).descriptionRect;
}

QRect AntEmpty::extraRect() const
{
    return emptyLayoutCache(size()).extraRect;
}

void AntEmpty::syncExtraGeometry()
{
    if (m_extraWidget)
    {
        const QRect target = extraRect();
        if (m_extraWidget->geometry() != target)
        {
            m_extraWidget->setGeometry(target);
            ++m_extraGeometryUpdateCount;
        }
        if (m_extraWidget->isHidden())
            m_extraWidget->show();
        syncEmptyPerfCounters();
    }
}

QSize AntEmpty::effectiveImageSize() const
{
    if (m_imageSize.isValid() && !m_imageSize.isEmpty())
    {
        return m_imageSize;
    }
    return m_simple ? QSize(96, 56) : QSize(121, 100);
}

const AntEmpty::EmptyLayoutCache& AntEmpty::emptyLayoutCache(const QSize& widgetSize) const
{
    const QSize extraSize = m_extraWidget ? m_extraWidget->sizeHint() : QSize();
    if (m_layoutCache.valid &&
        m_layoutCache.widgetSize == widgetSize &&
        m_layoutCache.imageVisible == m_imageVisible &&
        m_layoutCache.simple == m_simple &&
        m_layoutCache.imageSize == m_imageSize &&
        m_layoutCache.description == m_description &&
        m_layoutCache.extraSize == extraSize)
    {
        ++m_layoutCacheHitCount;
        syncEmptyPerfCounters();
        return m_layoutCache;
    }

    const auto& token = antTheme->tokens();
    EmptyLayoutCache cache;
    cache.valid = true;
    cache.widgetSize = widgetSize;
    cache.imageVisible = m_imageVisible;
    cache.simple = m_simple;
    cache.imageSize = m_imageSize;
    cache.description = m_description;
    cache.extraSize = extraSize;
    cache.effectiveImageSize = effectiveImageSize();
    cache.imageRect = QRect((widgetSize.width() - cache.effectiveImageSize.width()) / 2,
                            token.padding,
                            cache.effectiveImageSize.width(),
                            cache.effectiveImageSize.height());
    if (m_extraWidget)
    {
        cache.extraRect = QRect((widgetSize.width() - extraSize.width()) / 2,
                                widgetSize.height() - token.padding - extraSize.height(),
                                extraSize.width(),
                                extraSize.height());
    }
    const int top = m_imageVisible ? cache.imageRect.bottom() + token.marginXS : token.padding;
    const int bottom = m_extraWidget ? cache.extraRect.top() - token.margin : widgetSize.height() - token.padding;
    cache.descriptionRect = QRect(token.paddingSM,
                                  top,
                                  qMax(40, widgetSize.width() - token.paddingSM * 2),
                                  qMax(token.fontSize + 4, bottom - top));

    m_layoutCache = cache;
    ++m_layoutCacheBuildCount;
    syncEmptyPerfCounters();
    return m_layoutCache;
}

QSize AntEmpty::cachedSizeHint() const
{
    if (!m_sizeHintDirty)
    {
        ++m_sizeHintHitCount;
        syncEmptyPerfCounters();
        return m_cachedSizeHint;
    }

    const auto& token = antTheme->tokens();
    const QSize image = effectiveImageSize();
    int height = token.padding;

    if (m_imageVisible)
    {
        height += image.height();
        height += token.marginXS;
    }

    QFont descFont = font();
    descFont.setPixelSize(token.fontSize);
    const int descHeight = QFontMetrics(descFont).boundingRect(QRect(0, 0, 360, 120),
                                                               Qt::AlignCenter | Qt::TextWordWrap,
                                                               m_description)
                               .height();
    height += qMax(token.fontSize, descHeight);

    if (m_extraWidget)
    {
        height += token.margin;
        height += m_extraWidget->sizeHint().height();
    }
    height += token.padding;

    m_cachedSizeHint = QSize(qMax(220, image.width() + token.paddingXL), height);
    m_sizeHintDirty = false;
    ++m_sizeHintBuildCount;
    syncEmptyPerfCounters();
    return m_cachedSizeHint;
}

void AntEmpty::invalidateEmptyCaches() const
{
    m_layoutCache.valid = false;
    m_sizeHintDirty = true;
}

void AntEmpty::requestEmptyUpdate(const QRect& region, const QString& mode)
{
    QRect dirty = region.isValid() && !region.isEmpty() ? region : rect();
    dirty = dirty.intersected(rect());
    if (dirty.isEmpty())
        dirty = rect();

    ++m_regionUpdateCount;
    m_lastUpdateMode = mode;
    syncEmptyPerfCounters();
    update(dirty);
}

void AntEmpty::syncEmptyPerfCounters() const
{
    auto* self = const_cast<AntEmpty*>(this);
    self->setProperty("antEmptyLayoutCacheBuildCount", m_layoutCacheBuildCount);
    self->setProperty("antEmptyLayoutCacheHitCount", m_layoutCacheHitCount);
    self->setProperty("antEmptySizeHintBuildCount", m_sizeHintBuildCount);
    self->setProperty("antEmptySizeHintHitCount", m_sizeHintHitCount);
    self->setProperty("antEmptyIllustrationPixmapBuildCount", m_illustrationPixmapBuildCount);
    self->setProperty("antEmptyIllustrationPixmapHitCount", m_illustrationPixmapHitCount);
    self->setProperty("antEmptyRegionUpdateCount", m_regionUpdateCount);
    self->setProperty("antEmptyExtraGeometryUpdateCount", m_extraGeometryUpdateCount);
    self->setProperty("antEmptyLastUpdateMode", m_lastUpdateMode);
}
