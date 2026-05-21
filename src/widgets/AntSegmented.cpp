#include "AntSegmented.h"

#include <QEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QVariantAnimation>

#include <algorithm>
#include <cmath>

#include "core/AntTheme.h"
#include "styles/AntSegmentedStyle.h"

namespace
{
constexpr int kTrackPadding = 2;
constexpr int kIconSize = 14;
constexpr int kIconGap = 6;

int segmentedFontSize(Ant::Size size)
{
    const auto& token = antTheme->tokens();
    switch (size)
    {
    case Ant::Size::Small: return token.fontSizeSM;
    case Ant::Size::Large: return token.fontSizeLG;
    default: return token.fontSize;
    }
}

int segmentedHeight(Ant::Size size)
{
    const auto& token = antTheme->tokens();
    switch (size)
    {
    case Ant::Size::Small: return token.controlHeightSM;
    case Ant::Size::Large: return token.controlHeightLG;
    default: return token.controlHeight;
    }
}

int segmentedPaddingHorizontal(Ant::Size size)
{
    const auto& token = antTheme->tokens();
    return (size == Ant::Size::Small ? token.paddingXS : token.paddingSM) - token.lineWidth;
}

int segmentedRadius(Ant::Size size, Ant::SegmentedShape shape)
{
    const auto& token = antTheme->tokens();
    const int h = segmentedHeight(size);
    if (shape == Ant::SegmentedShape::Round)
    {
        return h / 2;
    }
    switch (size)
    {
    case Ant::Size::Small: return token.borderRadiusSM;
    case Ant::Size::Large: return token.borderRadiusLG;
    default: return token.borderRadius;
    }
}

int segmentedItemRadius(Ant::Size size, Ant::SegmentedShape shape)
{
    const auto& token = antTheme->tokens();
    const int h = segmentedHeight(size);
    if (shape == Ant::SegmentedShape::Round)
    {
        return h / 2;
    }
    switch (size)
    {
    case Ant::Size::Small: return token.borderRadiusXS;
    case Ant::Size::Large: return token.borderRadius;
    default: return token.borderRadiusSM;
    }
}

int segmentWidth(const AntSegmentedOption& option, int textWidth, Ant::Size size)
{
    const int iconWidth = option.icon.isEmpty() ? 0 : kIconSize + (option.label.isEmpty() ? 0 : kIconGap);
    return textWidth + iconWidth + segmentedPaddingHorizontal(size) * 2;
}
}

AntSegmented::AntSegmented(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntSegmentedStyle>(this);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        const QSize oldHint = sizeHint();
        invalidateLayoutCache();
        if (oldHint != sizeHint())
        {
            updateGeometry();
        }
        update();
    });
    syncSegmentedPerfCounters();
}

void AntSegmented::setOptions(const QVector<AntSegmentedOption>& options)
{
    m_options = options;
    ++m_optionsRevision;
    if (!m_value.isEmpty())
    {
        int idx = -1;
        for (int i = 0; i < m_options.size(); ++i)
        {
            if (m_options[i].value == m_value)
            {
                idx = i;
                break;
            }
        }
        if (idx < 0 && !m_options.isEmpty())
        {
            m_value = m_options[0].value;
            idx = 0;
        }
        m_thumbPos = static_cast<qreal>(idx);
    }
    else if (!m_options.isEmpty())
    {
        m_value = m_options[0].value;
        m_thumbPos = 0;
    }
    m_hoveredIndex = -1;
    m_pressedIndex = -1;
    invalidateLayoutCache();
    updateGeometry();
    update();
}

QVector<AntSegmentedOption> AntSegmented::options() const { return m_options; }

QString AntSegmented::value() const { return m_value; }

void AntSegmented::setValue(const QString& value)
{
    for (int i = 0; i < m_options.size(); ++i)
    {
        if (m_options[i].value == value && !m_options[i].disabled)
        {
            if (m_value == value) return;
            const int oldIndex = selectedIndex();
            const qreal oldThumbPosition = m_thumbPos;
            m_value = value;
            updateSelectionRegion(oldIndex, i, oldThumbPosition);
            startThumbAnimation(i);
            Q_EMIT currentIndexChanged(i);
            Q_EMIT valueChanged(m_value);
            return;
        }
    }
}

int AntSegmented::currentIndex() const
{
    return selectedIndex();
}

void AntSegmented::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_options.size() || m_options[index].disabled)
    {
        return;
    }
    setValue(m_options[index].value);
}

bool AntSegmented::isBlock() const { return m_block; }

void AntSegmented::setBlock(bool block)
{
    if (m_block == block) return;
    m_block = block;
    setSizePolicy(m_block ? QSizePolicy::Expanding : QSizePolicy::Fixed, QSizePolicy::Fixed);
    invalidateLayoutCache();
    updateGeometry();
    update();
    Q_EMIT blockChanged(m_block);
}

Ant::Size AntSegmented::segmentedSize() const { return m_size; }

void AntSegmented::setSegmentedSize(Ant::Size size)
{
    if (m_size == size) return;
    m_size = size;
    invalidateLayoutCache();
    updateGeometry();
    update();
    Q_EMIT segmentedSizeChanged(m_size);
}

bool AntSegmented::isVertical() const { return m_vertical; }

void AntSegmented::setVertical(bool vertical)
{
    if (m_vertical == vertical) return;
    m_vertical = vertical;
    invalidateLayoutCache();
    updateGeometry();
    update();
    Q_EMIT verticalChanged(m_vertical);
}

Ant::SegmentedShape AntSegmented::shape() const { return m_shape; }

void AntSegmented::setShape(Ant::SegmentedShape shape)
{
    if (m_shape == shape) return;
    m_shape = shape;
    invalidateLayoutCache();
    update();
    Q_EMIT shapeChanged(m_shape);
}

QSize AntSegmented::sizeHint() const
{
    return layoutCache().sizeHint;
}

QSize AntSegmented::minimumSizeHint() const
{
    return sizeHint();
}

void AntSegmented::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntSegmented::mouseMoveEvent(QMouseEvent* event)
{
    const int oldHovered = m_hoveredIndex;
    const int oldPressed = m_pressedIndex;
    const int idx = segmentIndexAt(event->pos());
    const int pressedIdx = (event->buttons() & Qt::LeftButton) ? idx : -1;
    if (m_hoveredIndex != idx || m_pressedIndex != pressedIdx)
    {
        m_hoveredIndex = idx;
        m_pressedIndex = pressedIdx;
        updateSegmentRegions({oldHovered, m_hoveredIndex, oldPressed, m_pressedIndex}, QStringLiteral("segments"));
    }
    QWidget::mouseMoveEvent(event);
}

void AntSegmented::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const int idx = segmentIndexAt(event->pos());
        if (idx >= 0 && idx < m_options.size() && !m_options[idx].disabled)
        {
            const int oldPressed = m_pressedIndex;
            m_pressedIndex = idx;
            updateSegmentRegions({oldPressed, m_pressedIndex}, QStringLiteral("segments"));
        }
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntSegmented::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const int idx = segmentIndexAt(event->pos());
        const int pressedIdx = m_pressedIndex;
        m_pressedIndex = -1;
        if (idx == pressedIdx && idx >= 0 && idx < m_options.size() && !m_options[idx].disabled)
        {
            setValue(m_options[idx].value);
        }
        updateSegmentRegions({pressedIdx}, QStringLiteral("segments"));
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void AntSegmented::leaveEvent(QEvent* event)
{
    const int oldHovered = m_hoveredIndex;
    m_hoveredIndex = -1;
    // Do NOT clear m_pressedIndex here: Qt's implicit mouse grab keeps the
    // matching release routed to us even if the cursor briefly leaves the
    // widget rect (or an unrelated top-level HWND, e.g. a freshly-floated
    // AntDockWidget, changes the active window). Clearing m_pressedIndex on
    // leave caused mouseReleaseEvent to see "no pressed index" and skip the
    // value commit — the press indicator flashed but the segment never
    // actually switched. m_pressedIndex is cleared in mouseReleaseEvent.
    updateSegmentRegions({oldHovered}, QStringLiteral("segments"));
    QWidget::leaveEvent(event);
}

void AntSegmented::resizeEvent(QResizeEvent* event)
{
    invalidateLayoutCache();
    QWidget::resizeEvent(event);
}

void AntSegmented::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::FontChange)
    {
        invalidateLayoutCache();
        updateGeometry();
        update();
    }
    else if (event->type() == QEvent::EnabledChange)
    {
        update();
    }
    QWidget::changeEvent(event);
}

QVector<QRectF> AntSegmented::segmentRects() const
{
    return layoutCache().segmentRects;
}

const QVector<AntSegmentedOption>& AntSegmented::optionList() const
{
    return m_options;
}

const AntSegmented::LayoutCache& AntSegmented::layoutCache() const
{
    QFont layoutFont = font();
    const int fontSize = segmentedFontSize(m_size);
    layoutFont.setPixelSize(fontSize);
    const int height = segmentedHeight(m_size);
    const int paddingHorizontal = segmentedPaddingHorizontal(m_size);
    const int trackRadius = segmentedRadius(m_size, m_shape);
    const int itemRadius = segmentedItemRadius(m_size, m_shape);

    if (m_layoutCache.valid
        && m_layoutCache.widgetSize == size()
        && m_layoutCache.font == layoutFont
        && m_layoutCache.optionsRevision == m_optionsRevision
        && m_layoutCache.segmentedSize == m_size
        && m_layoutCache.block == m_block
        && m_layoutCache.vertical == m_vertical
        && m_layoutCache.shape == m_shape
        && m_layoutCache.fontSize == fontSize
        && m_layoutCache.height == height
        && m_layoutCache.paddingHorizontal == paddingHorizontal
        && m_layoutCache.trackRadius == trackRadius
        && m_layoutCache.itemRadius == itemRadius)
    {
        return m_layoutCache;
    }

    m_layoutCache.widgetSize = size();
    m_layoutCache.font = layoutFont;
    m_layoutCache.optionsRevision = m_optionsRevision;
    m_layoutCache.segmentedSize = m_size;
    m_layoutCache.block = m_block;
    m_layoutCache.vertical = m_vertical;
    m_layoutCache.shape = m_shape;
    m_layoutCache.fontSize = fontSize;
    m_layoutCache.height = height;
    m_layoutCache.paddingHorizontal = paddingHorizontal;
    m_layoutCache.trackRadius = trackRadius;
    m_layoutCache.itemRadius = itemRadius;
    m_layoutCache.segmentRects.clear();
    m_layoutCache.segments.clear();

    const int n = m_options.size();
    QVector<int> textWidths;
    QVector<int> segmentWidths;
    textWidths.reserve(n);
    segmentWidths.reserve(n);

    QFontMetrics fm(layoutFont);
    int maxWidth = height;
    int totalWidth = kTrackPadding * 2;
    for (const auto& option : m_options)
    {
        const int textWidth = fm.horizontalAdvance(option.label);
        const int width = segmentWidth(option, textWidth, m_size);
        textWidths.append(textWidth);
        segmentWidths.append(width);
        maxWidth = std::max(maxWidth, width);
        totalWidth += width;
    }

    if (m_vertical)
    {
        m_layoutCache.sizeHint = QSize(maxWidth + kTrackPadding * 2, n * height);
    }
    else
    {
        m_layoutCache.sizeHint = QSize(totalWidth, height);
    }

    if (n > 0)
    {
        const QRectF track = QRectF(rect()).adjusted(kTrackPadding, kTrackPadding, -kTrackPadding, -kTrackPadding);
        m_layoutCache.segmentRects.reserve(n);
        m_layoutCache.segments.reserve(n);

        if (m_vertical)
        {
            const qreal segmentHeight = track.height() / n;
            for (int i = 0; i < n; ++i)
            {
                m_layoutCache.segmentRects.append(QRectF(track.x(), track.y() + i * segmentHeight, track.width(), segmentHeight));
            }
        }
        else if (m_block)
        {
            const qreal segmentWidth = track.width() / n;
            for (int i = 0; i < n; ++i)
            {
                m_layoutCache.segmentRects.append(QRectF(track.x() + i * segmentWidth, track.y(), segmentWidth, track.height()));
            }
        }
        else
        {
            qreal x = track.x();
            for (int i = 0; i < n; ++i)
            {
                const qreal width = segmentWidths.at(i);
                m_layoutCache.segmentRects.append(QRectF(x, track.y(), width, track.height()));
                x += width;
            }
        }

        for (int i = 0; i < n; ++i)
        {
            SegmentLayout layout;
            layout.rect = m_layoutCache.segmentRects.at(i);
            layout.textWidth = textWidths.at(i);
            layout.segmentWidth = segmentWidths.at(i);
            layout.hasIcon = !m_options.at(i).icon.isEmpty();
            if (layout.hasIcon)
            {
                const int iconGap = m_options.at(i).label.isEmpty() ? 0 : kIconGap;
                const int totalContentWidth = kIconSize + iconGap + layout.textWidth;
                const qreal left = layout.rect.center().x() - totalContentWidth / 2.0;
                layout.iconRect = QRectF(left, layout.rect.center().y() - kIconSize / 2.0, kIconSize, kIconSize);
                layout.textRect = QRectF(left + kIconSize + iconGap, layout.rect.top(), layout.textWidth + 2, layout.rect.height());
            }
            else
            {
                layout.textRect = layout.rect;
            }
            m_layoutCache.segments.append(layout);
        }
    }

    m_layoutCache.valid = true;
    ++m_layoutBuildCount;
    ++m_sizeHintResolveCount;
    m_textMetricResolveCount += n;
    syncSegmentedPerfCounters();
    return m_layoutCache;
}

QRectF AntSegmented::thumbRect(qreal position) const
{
    const auto& rects = layoutCache().segmentRects;
    if (rects.isEmpty())
    {
        return QRectF();
    }

    const qreal pos = std::clamp(position, 0.0, static_cast<qreal>(rects.size() - 1));
    const int leftIdx = static_cast<int>(std::floor(pos));
    const int rightIdx = std::min(leftIdx + 1, static_cast<int>(rects.size()) - 1);
    const qreal t = pos - leftIdx;

    const QRectF a = rects.at(leftIdx);
    const QRectF b = rects.at(rightIdx);
    return QRectF(a.left() + (b.left() - a.left()) * t,
                  a.top() + (b.top() - a.top()) * t,
                  a.width() + (b.width() - a.width()) * t,
                  a.height() + (b.height() - a.height()) * t);
}

QRectF AntSegmented::thumbRect() const
{
    return thumbRect(m_thumbPos);
}

QRect AntSegmented::segmentDirtyRect(int index) const
{
    const auto& rects = layoutCache().segmentRects;
    if (index < 0 || index >= rects.size())
    {
        return QRect();
    }
    return rects.at(index).toAlignedRect().adjusted(-2, -2, 2, 2).intersected(rect());
}

QRect AntSegmented::thumbDirtyRect(qreal position) const
{
    const QRectF thumb = thumbRect(position);
    if (thumb.isEmpty())
    {
        return QRect();
    }
    return thumb.toAlignedRect().adjusted(-4, -3, 4, 5).intersected(rect());
}

void AntSegmented::updateSegmentRegions(const QVector<int>& indices, const QString& mode)
{
    QRect dirty;
    for (const int index : indices)
    {
        dirty = dirty.united(segmentDirtyRect(index));
    }
    if (!dirty.isValid() || dirty.isEmpty())
    {
        return;
    }

    ++m_regionUpdateCount;
    setProperty("antSegmentedLastUpdateMode", mode);
    syncSegmentedPerfCounters();
    update(dirty);
}

void AntSegmented::updateSelectionRegion(int oldIndex, int newIndex, qreal oldThumbPosition)
{
    QRect dirty = segmentDirtyRect(oldIndex).united(segmentDirtyRect(newIndex)).united(thumbDirtyRect(oldThumbPosition));
    if (!dirty.isValid() || dirty.isEmpty())
    {
        dirty = rect();
    }

    ++m_regionUpdateCount;
    setProperty("antSegmentedLastUpdateMode", QStringLiteral("selection"));
    syncSegmentedPerfCounters();
    update(dirty.intersected(rect()));
}

void AntSegmented::updateThumbRegion(qreal oldPosition, qreal newPosition)
{
    QRect dirty = thumbDirtyRect(oldPosition).united(thumbDirtyRect(newPosition));
    if (!dirty.isValid() || dirty.isEmpty())
    {
        return;
    }

    ++m_thumbRegionUpdateCount;
    setProperty("antSegmentedLastUpdateMode", QStringLiteral("thumb"));
    syncSegmentedPerfCounters();
    update(dirty);
}

void AntSegmented::invalidateLayoutCache() const
{
    m_layoutCache.valid = false;
    syncSegmentedPerfCounters();
}

void AntSegmented::syncSegmentedPerfCounters() const
{
    auto* self = const_cast<AntSegmented*>(this);
    self->setProperty("antSegmentedLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antSegmentedSizeHintResolveCount", m_sizeHintResolveCount);
    self->setProperty("antSegmentedTextMetricResolveCount", m_textMetricResolveCount);
    self->setProperty("antSegmentedRegionUpdateCount", m_regionUpdateCount);
    self->setProperty("antSegmentedThumbRegionUpdateCount", m_thumbRegionUpdateCount);
}

int AntSegmented::segmentIndexAt(const QPoint& pos) const
{
    const auto& rects = layoutCache().segmentRects;
    for (int i = 0; i < rects.size(); ++i)
    {
        if (rects[i].contains(pos)) return i;
    }

    if (!rect().contains(pos))
    {
        return -1;
    }

    for (int i = 0; i < rects.size(); ++i)
    {
        if (rects[i].adjusted(-kTrackPadding, -kTrackPadding, kTrackPadding, kTrackPadding).contains(pos))
        {
            return i;
        }
    }
    return -1;
}

int AntSegmented::selectedIndex() const
{
    for (int i = 0; i < m_options.size(); ++i)
    {
        if (m_options[i].value == m_value) return i;
    }
    return -1;
}

int AntSegmented::hoveredIndex() const
{
    return m_hoveredIndex;
}

int AntSegmented::pressedIndex() const
{
    return m_pressedIndex;
}

qreal AntSegmented::thumbPosition() const
{
    return m_thumbPos;
}

void AntSegmented::startThumbAnimation(int newIndex)
{
    if (!m_thumbAnimation)
    {
        m_thumbAnimation = new QVariantAnimation(this);
        m_thumbAnimation->setDuration(300);
        m_thumbAnimation->setEasingCurve(QEasingCurve::InOutCubic);
        connect(m_thumbAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
            const qreal oldPosition = m_thumbPos;
            m_thumbPos = v.toReal();
            updateThumbRegion(oldPosition, m_thumbPos);
        });
    }
    m_thumbAnimation->stop();
    const qreal oldPosition = m_thumbPos;
    m_thumbAnimation->setStartValue(m_thumbPos);
    m_thumbAnimation->setEndValue(static_cast<qreal>(newIndex));
    m_thumbAnimation->start();
    updateThumbRegion(oldPosition, static_cast<qreal>(newIndex));
}
