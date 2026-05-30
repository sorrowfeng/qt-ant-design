#include "AntBadge.h"

#include <QEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QStyleOption>
#include <QTimer>

#include <algorithm>

#include "core/AntTheme.h"
#include "core/AntThemeRefresh_p.h"
#include "styles/AntPalette.h"
#include "styles/AntBadgeStyle.h"

namespace
{
// Small anti-clip guard around the content widget for the badge outline and
// any subtle child shadow without making avatar badges look loosely spaced.
constexpr int kShadowMargin = 2;

// Transparent overlay painted on top of the content widget to host the
// indicator. Needed because AntBadge's own paintEvent runs BEFORE child
// widgets (the content), so painting the indicator there gets covered by
// e.g. an AntButton child's opaque background.
class BadgeIndicatorOverlay : public QWidget
{
public:
    BadgeIndicatorOverlay(AntBadge* owner)
        : QWidget(owner)
        , m_owner(owner)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TranslucentBackground);
        setFocusPolicy(Qt::NoFocus);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        if (!m_owner)
        {
            return;
        }
        QStyleOption option;
        option.initFrom(m_owner);
        option.rect = m_owner->rect();
        QPainter p(this);
        m_owner->style()->drawPrimitive(QStyle::PE_Widget, &option, &p, m_owner);
    }

private:
    AntBadge* m_owner = nullptr;
};
} // namespace

AntBadge::AntBadge(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntBadgeStyle>(this);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, [this]() {
        const QRect oldDirty = processingDirtyRectForPulse(processingPulseProgress());
        m_pulse = (m_pulse + 6) % 100;
        requestBadgeUpdate(oldDirty.united(processingDirtyRectForPulse(processingPulseProgress())),
                           QStringLiteral("processing"),
                           true,
                           true);
    });

    m_indicatorOverlay = new BadgeIndicatorOverlay(this);
    m_indicatorOverlay->hide();
    m_indicatorOverlay->raise();

    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateBadgePaintCache();
        if (AntThemeRefresh::updateGeometryIfSizeHintChanged(this))
        {
            updateContentGeometry();
        }
        requestBadgeUpdate(rect(), QStringLiteral("theme"), true);
    });

    updateAnimationState();
    syncBadgePerfCounters();
}

AntBadge::AntBadge(int count, QWidget* parent)
    : AntBadge(parent)
{
    m_count = count;
}

int AntBadge::count() const { return m_count; }

void AntBadge::setCount(int count)
{
    if (m_count == count)
    {
        return;
    }
    const QRect oldDirty = badgeVisualDirtyRect();
    m_count = count;
    invalidateBadgePaintCache();
    updateGeometry();
    requestBadgeUpdate(oldDirty.united(badgeVisualDirtyRect()), QStringLiteral("count"), true);
    Q_EMIT countChanged(m_count);
}

QString AntBadge::text() const { return m_text; }

void AntBadge::setText(const QString& text)
{
    if (m_text == text)
    {
        return;
    }
    const QRect oldDirty = badgeVisualDirtyRect();
    m_text = text;
    invalidateBadgePaintCache();
    updateGeometry();
    requestBadgeUpdate(oldDirty.united(badgeVisualDirtyRect()), QStringLiteral("text"), true);
    Q_EMIT textChanged(m_text);
}

QString AntBadge::color() const { return m_color; }

void AntBadge::setColor(const QString& color)
{
    if (m_color == color)
    {
        return;
    }
    const QRect oldDirty = badgeVisualDirtyRect();
    m_color = color;
    invalidateBadgePaintCache();
    requestBadgeUpdate(oldDirty.united(badgeVisualDirtyRect()), QStringLiteral("color"), true);
    Q_EMIT colorChanged(m_color);
}

bool AntBadge::isDot() const { return m_dot; }

void AntBadge::setDot(bool dot)
{
    if (m_dot == dot)
    {
        return;
    }
    const QRect oldDirty = badgeVisualDirtyRect();
    m_dot = dot;
    invalidateBadgePaintCache();
    updateGeometry();
    updateContentGeometry();
    requestBadgeUpdate(oldDirty.united(badgeVisualDirtyRect()), QStringLiteral("dot"), true);
    Q_EMIT dotChanged(m_dot);
}

bool AntBadge::showZero() const { return m_showZero; }

void AntBadge::setShowZero(bool showZero)
{
    if (m_showZero == showZero)
    {
        return;
    }
    const QRect oldDirty = badgeVisualDirtyRect();
    m_showZero = showZero;
    invalidateBadgePaintCache();
    updateGeometry();
    updateContentGeometry();
    requestBadgeUpdate(oldDirty.united(badgeVisualDirtyRect()), QStringLiteral("showZero"), true);
    Q_EMIT showZeroChanged(m_showZero);
}

int AntBadge::overflowCount() const { return m_overflowCount; }

void AntBadge::setOverflowCount(int overflowCount)
{
    overflowCount = std::max(0, overflowCount);
    if (m_overflowCount == overflowCount)
    {
        return;
    }
    const QRect oldDirty = badgeVisualDirtyRect();
    m_overflowCount = overflowCount;
    invalidateBadgePaintCache();
    updateGeometry();
    updateContentGeometry();
    requestBadgeUpdate(oldDirty.united(badgeVisualDirtyRect()), QStringLiteral("overflow"), true);
    Q_EMIT overflowCountChanged(m_overflowCount);
}

QPoint AntBadge::offset() const { return m_offset; }

void AntBadge::setOffset(const QPoint& offset)
{
    if (m_offset == offset)
    {
        return;
    }
    const QRect oldDirty = badgeVisualDirtyRect();
    m_offset = offset;
    invalidateBadgePaintCache();
    updateGeometry();
    updateContentGeometry();
    requestBadgeUpdate(oldDirty.united(badgeVisualDirtyRect()), QStringLiteral("offset"), true);
    Q_EMIT offsetChanged(m_offset);
}

Ant::Size AntBadge::badgeSize() const { return m_badgeSize; }

void AntBadge::setBadgeSize(Ant::Size size)
{
    if (m_badgeSize == size)
    {
        return;
    }
    const QRect oldDirty = badgeVisualDirtyRect();
    m_badgeSize = size;
    invalidateBadgePaintCache();
    updateGeometry();
    updateContentGeometry();
    requestBadgeUpdate(oldDirty.united(badgeVisualDirtyRect()), QStringLiteral("size"), true);
    Q_EMIT badgeSizeChanged(m_badgeSize);
}

Ant::BadgeStatus AntBadge::status() const { return m_status; }

void AntBadge::setStatus(Ant::BadgeStatus status)
{
    if (m_status == status)
    {
        return;
    }
    const QRect oldDirty = badgeVisualDirtyRect();
    m_status = status;
    invalidateBadgePaintCache();
    updateAnimationState();
    updateGeometry();
    updateContentGeometry();
    requestBadgeUpdate(oldDirty.united(badgeVisualDirtyRect()), QStringLiteral("status"), true);
    Q_EMIT statusChanged(m_status);
}

Ant::BadgeMode AntBadge::badgeMode() const { return m_badgeMode; }

void AntBadge::setBadgeMode(Ant::BadgeMode mode)
{
    if (m_badgeMode == mode)
        return;
    const QRect oldDirty = badgeVisualDirtyRect();
    m_badgeMode = mode;
    invalidateBadgePaintCache();
    updateGeometry();
    requestBadgeUpdate(oldDirty.united(badgeVisualDirtyRect()), QStringLiteral("mode"), true);
    Q_EMIT badgeModeChanged(m_badgeMode);
}

QString AntBadge::ribbonText() const { return m_ribbonText; }

void AntBadge::setRibbonText(const QString& text)
{
    if (m_ribbonText == text)
        return;
    const QRect oldDirty = badgeVisualDirtyRect();
    m_ribbonText = text;
    invalidateBadgePaintCache();
    updateGeometry();
    requestBadgeUpdate(oldDirty.united(badgeVisualDirtyRect()), QStringLiteral("ribbonText"), true);
    Q_EMIT ribbonTextChanged(m_ribbonText);
}

QString AntBadge::ribbonColor() const { return m_ribbonColor; }

void AntBadge::setRibbonColor(const QString& color)
{
    if (m_ribbonColor == color)
        return;
    const QRect oldDirty = badgeVisualDirtyRect();
    m_ribbonColor = color;
    invalidateBadgePaintCache();
    requestBadgeUpdate(oldDirty.united(badgeVisualDirtyRect()), QStringLiteral("ribbonColor"), true);
    Q_EMIT ribbonColorChanged(m_ribbonColor);
}

QWidget* AntBadge::contentWidget() const { return m_contentWidget; }

qreal AntBadge::processingPulseProgress() const
{
    return std::clamp(m_pulse / 100.0, 0.0, 1.0);
}

void AntBadge::setContentWidget(QWidget* widget)
{
    if (m_contentWidget == widget)
    {
        return;
    }
    const QRect oldDirty = badgeVisualDirtyRect();
    if (m_contentWidget)
    {
        m_contentWidget->removeEventFilter(this);
        m_contentWidget->setParent(nullptr);
    }
    m_contentWidget = widget;
    if (m_contentWidget)
    {
        m_contentWidget->setParent(this);
        m_contentWidget->show();
    }
    invalidateBadgePaintCache();
    updateAnimationState();
    updateGeometry();
    adjustSize();
    updateContentGeometry();
    // Keep the indicator overlay on top of the (now reparented) content.
    if (m_indicatorOverlay)
    {
        m_indicatorOverlay->setGeometry(rect());
        m_indicatorOverlay->setVisible(m_contentWidget != nullptr);
        m_indicatorOverlay->raise();
    }
    requestBadgeUpdate(oldDirty.united(rect()), QStringLiteral("content"), true);
}

QSize AntBadge::sizeHint() const
{
    if (isStatusMode() && !m_contentWidget)
    {
        const auto& token = antTheme->tokens();
        QFont f = font();
        f.setPixelSize(token.fontSize);
        const int textWidth = m_text.isEmpty() ? 0 : QFontMetrics(f).horizontalAdvance(m_text);
        return QSize(statusDotSize() + (m_text.isEmpty() ? 0 : token.marginXS + textWidth), token.controlHeightSM);
    }

    const int w = indicatorWidth();
    if (m_contentWidget)
    {
        // Total = left shadow margin + content + (right shadow / indicator half)
        //       and same for vertical.
        const QSize content = m_contentWidget->sizeHint().expandedTo(m_contentWidget->minimumSizeHint());
        return QSize(kShadowMargin + content.width() + contentRightReserve(),
                     contentTopReserve() + content.height() + kShadowMargin);
    }
    return QSize(w + 2, indicatorHeight() + 2);
}

QSize AntBadge::minimumSizeHint() const
{
    if (m_contentWidget)
    {
        const QSize cmin = m_contentWidget->minimumSizeHint();
        return QSize(kShadowMargin + cmin.width() + contentRightReserve(),
                     contentTopReserve() + cmin.height() + kShadowMargin);
    }
    return QSize(dotSize() + 2, dotSize() + 2);
}

void AntBadge::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    if (!m_contentWidget)
    {
        QStyleOption option;
        option.initFrom(this);
        option.rect = rect();
        QPainter painter(this);
        style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
        return;
    }

    // The indicator is painted by m_indicatorOverlay (sibling layer above
    // the content widget). Piggy-back on our own paint to refresh it so
    // any call-site `update()` also repaints the indicator without every
    // setter having to know about the overlay.
    if (m_indicatorOverlay)
    {
        m_indicatorOverlay->update();
    }
}

void AntBadge::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::EnabledChange)
    {
        updateAnimationState();
        invalidateBadgePaintCache();
        requestBadgeUpdate(rect(), QStringLiteral("enabled"), true);
    }
    else if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange)
    {
        invalidateBadgePaintCache();
        updateGeometry();
        updateContentGeometry();
        requestBadgeUpdate(rect(), QStringLiteral("style"), true);
    }
}

void AntBadge::resizeEvent(QResizeEvent* event)
{
    invalidateBadgePaintCache();
    updateContentGeometry();
    if (m_indicatorOverlay)
    {
        m_indicatorOverlay->setGeometry(rect());
        m_indicatorOverlay->setVisible(m_contentWidget != nullptr);
        m_indicatorOverlay->raise();
    }
    QWidget::resizeEvent(event);
}

void AntBadge::mouseMoveEvent(QMouseEvent* event)
{
    const bool hovered = indicatorRect().contains(event->pos());
    if (m_hovered != hovered)
    {
        const QRect dirty = badgeVisualDirtyRect();
        m_hovered = hovered;
        requestBadgeUpdate(dirty, QStringLiteral("hover"), true);
    }
    QWidget::mouseMoveEvent(event);
}

void AntBadge::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && shouldShowIndicator() && indicatorRect().contains(event->pos()))
    {
        Q_EMIT clicked();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntBadge::leaveEvent(QEvent* event)
{
    if (m_hovered)
    {
        const QRect dirty = badgeVisualDirtyRect();
        m_hovered = false;
        requestBadgeUpdate(dirty, QStringLiteral("hover"), true);
    }
    QWidget::leaveEvent(event);
}

void AntBadge::showEvent(QShowEvent* event)
{
    updateAnimationState();
    QWidget::showEvent(event);
}

void AntBadge::hideEvent(QHideEvent* event)
{
    m_animationTimer->stop();
    syncBadgePerfCounters();
    QWidget::hideEvent(event);
}

int AntBadge::indicatorHeight() const
{
    return m_badgeSize == Ant::Size::Small ? antTheme->tokens().fontSize : 20;
}

int AntBadge::dotSize() const
{
    return std::max(6, antTheme->tokens().fontSizeSM / 2);
}

int AntBadge::statusDotSize() const
{
    return dotSize();
}

int AntBadge::indicatorWidth() const
{
    if (m_dot)
    {
        return dotSize();
    }
    const int h = indicatorHeight();
    QFont f = font();
    f.setPixelSize(m_badgeSize == Ant::Size::Small ? antTheme->tokens().fontSizeSM : antTheme->tokens().fontSizeSM);
    const int textWidth = QFontMetrics(f).horizontalAdvance(displayText());
    return std::max(h, textWidth + antTheme->tokens().paddingXS * 2);
}

int AntBadge::contentTopReserve() const
{
    // Need enough room above the content for:
    //   - the top half of the indicator (h/2)
    //   - the content's top drop shadow (kShadowMargin)
    //   - an extra few px as anti-clip guard for the 1px white outline
    if (!shouldShowIndicator())
    {
        return kShadowMargin;
    }
    return std::max(indicatorHeight() / 2 + 4 + std::max(0, -m_offset.y()), kShadowMargin);
}

int AntBadge::contentRightReserve() const
{
    if (!shouldShowIndicator())
    {
        return kShadowMargin;
    }
    return std::max(indicatorWidth() / 2 + 4 + std::max(0, m_offset.x()), kShadowMargin);
}

QString AntBadge::displayText() const
{
    if (m_count > m_overflowCount)
    {
        return QStringLiteral("%1+").arg(m_overflowCount);
    }
    return QString::number(m_count);
}

QRect AntBadge::contentRect() const
{
    if (!m_contentWidget)
    {
        return rect();
    }
    const QSize hint = m_contentWidget->sizeHint().expandedTo(m_contentWidget->minimumSizeHint());
    // Content is inset by kShadowMargin on left/bottom and by contentTopReserve
    // on top (which already includes shadow + indicator headroom). Right side
    // gets contentRightReserve (indicator width + shadow). Size = hint.
    return QRect(kShadowMargin, contentTopReserve(), hint.width(), hint.height());
}

QRectF AntBadge::indicatorRect() const
{
    const int w = indicatorWidth();
    const int h = m_dot ? dotSize() : indicatorHeight();
    if (!m_contentWidget)
    {
        return QRectF(1, (height() - h) / 2.0, w, h);
    }
    const QRect content = contentRect();
    return QRectF(content.right() + 1 - w / 2.0 + m_offset.x(),
                  content.top() - h / 2.0 + m_offset.y(),
                  w,
                  h);
}

QRectF AntBadge::standaloneStatusDotRect() const
{
    const int d = statusDotSize();
    return QRectF(1, (height() - d) / 2.0, d, d);
}

QColor AntBadge::badgeColor() const
{
    const QColor preset = AntPalette::presetColor(m_color);
    if (preset.isValid())
        return preset;
    if (m_color.compare(QStringLiteral("success"), Qt::CaseInsensitive) == 0)
    {
        return antTheme->tokens().colorSuccess;
    }
    if (m_color.compare(QStringLiteral("warning"), Qt::CaseInsensitive) == 0)
    {
        return antTheme->tokens().colorWarning;
    }
    if (m_color.compare(QStringLiteral("processing"), Qt::CaseInsensitive) == 0 || m_color.compare(QStringLiteral("blue"), Qt::CaseInsensitive) == 0)
    {
        return antTheme->tokens().colorPrimary;
    }
    const QColor custom(m_color);
    if (custom.isValid())
    {
        return custom;
    }
    return antTheme->tokens().colorError;
}

QColor AntBadge::statusColor() const
{
    const QColor preset = AntPalette::presetColor(m_color);
    if (preset.isValid())
        return preset;
    const auto& token = antTheme->tokens();
    const QColor custom(m_color);
    if (custom.isValid())
    {
        return custom;
    }
    switch (m_status)
    {
    case Ant::BadgeStatus::Success:
        return token.colorSuccess;
    case Ant::BadgeStatus::Processing:
        return token.colorPrimary;
    case Ant::BadgeStatus::Default:
        return token.colorTextPlaceholder;
    case Ant::BadgeStatus::Error:
        return token.colorError;
    case Ant::BadgeStatus::Warning:
        return token.colorWarning;
    case Ant::BadgeStatus::None:
        return token.colorError;
    }
    return token.colorError;
}

QColor AntBadge::ribbonColorValue() const
{
    const auto& token = antTheme->tokens();
    const QColor preset = AntPalette::presetColor(m_ribbonColor);
    if (preset.isValid())
    {
        return preset;
    }
    if (m_ribbonColor.compare(QStringLiteral("success"), Qt::CaseInsensitive) == 0)
    {
        return token.colorSuccess;
    }
    if (m_ribbonColor.compare(QStringLiteral("warning"), Qt::CaseInsensitive) == 0)
    {
        return token.colorWarning;
    }
    if (m_ribbonColor.compare(QStringLiteral("processing"), Qt::CaseInsensitive) == 0)
    {
        return token.colorPrimary;
    }
    const QColor custom(m_ribbonColor);
    if (custom.isValid())
    {
        return custom;
    }
    return token.colorError;
}

bool AntBadge::shouldShowIndicator() const
{
    if (isStatusMode())
    {
        return true;
    }
    if (m_dot)
    {
        return true;
    }
    return m_count != 0 || m_showZero;
}

bool AntBadge::isStatusMode() const
{
    return m_status != Ant::BadgeStatus::None;
}

const AntBadge::BadgePaintCache& AntBadge::badgePaintCache(const QRect& widgetRect) const
{
    const QSize contentSize = m_contentWidget
                                  ? m_contentWidget->sizeHint().expandedTo(m_contentWidget->minimumSizeHint())
                                  : QSize();
    if (m_paintCache.valid &&
        m_paintCache.widgetRect == widgetRect &&
        m_paintCache.contentSize == contentSize &&
        m_paintCache.font == font() &&
        m_paintCache.badgeSize == m_badgeSize &&
        m_paintCache.status == m_status &&
        m_paintCache.badgeMode == m_badgeMode &&
        m_paintCache.dot == m_dot &&
        m_paintCache.showZero == m_showZero &&
        m_paintCache.enabled == isEnabled() &&
        m_paintCache.count == m_count &&
        m_paintCache.overflowCount == m_overflowCount &&
        m_paintCache.offset == m_offset &&
        m_paintCache.text == m_text &&
        m_paintCache.color == m_color &&
        m_paintCache.ribbonText == m_ribbonText &&
        m_paintCache.ribbonColor == m_ribbonColor)
    {
        ++m_layoutCacheHitCount;
        syncBadgePerfCounters();
        return m_paintCache;
    }

    ++m_layoutBuildCount;
    BadgePaintCache cache;
    cache.valid = true;
    cache.widgetRect = widgetRect;
    cache.contentSize = contentSize;
    cache.font = font();
    cache.badgeSize = m_badgeSize;
    cache.status = m_status;
    cache.badgeMode = m_badgeMode;
    cache.dot = m_dot;
    cache.showZero = m_showZero;
    cache.enabled = isEnabled();
    cache.count = m_count;
    cache.overflowCount = m_overflowCount;
    cache.offset = m_offset;
    cache.text = m_text;
    cache.color = m_color;
    cache.ribbonText = m_ribbonText;
    cache.ribbonColor = m_ribbonColor;
    cache.displayText = displayText();
    cache.indicatorHeight = indicatorHeight();
    cache.dotSize = dotSize();
    cache.statusDotSize = statusDotSize();
    cache.indicatorWidth = indicatorWidth();
    cache.contentTopReserve = contentTopReserve();
    cache.contentRightReserve = contentRightReserve();
    cache.contentRect = m_contentWidget
                            ? QRect(kShadowMargin, cache.contentTopReserve, contentSize.width(), contentSize.height())
                            : widgetRect;
    cache.indicatorRect = QRectF(1,
                                 (widgetRect.height() - (m_dot ? cache.dotSize : cache.indicatorHeight)) / 2.0,
                                 cache.indicatorWidth,
                                 m_dot ? cache.dotSize : cache.indicatorHeight);
    if (m_contentWidget)
    {
        cache.indicatorRect = QRectF(cache.contentRect.right() + 1 - cache.indicatorWidth / 2.0 + m_offset.x(),
                                     cache.contentRect.top() - (m_dot ? cache.dotSize : cache.indicatorHeight) / 2.0 + m_offset.y(),
                                     cache.indicatorWidth,
                                     m_dot ? cache.dotSize : cache.indicatorHeight);
    }
    cache.statusDotRect = QRectF(1,
                                 (widgetRect.height() - cache.statusDotSize) / 2.0,
                                 cache.statusDotSize,
                                 cache.statusDotSize);
    cache.statusTextRect = QRectF(cache.statusDotRect.right() + antTheme->tokens().marginXS,
                                  0,
                                  widgetRect.width() - cache.statusDotRect.right() - antTheme->tokens().marginXS,
                                  widgetRect.height());
    cache.badgeColor = badgeColor();
    cache.statusColor = statusColor();

    if (m_badgeMode == Ant::BadgeMode::Ribbon && !m_ribbonText.isEmpty())
    {
        const auto& token = antTheme->tokens();
        QFont ribbonFont = font();
        ribbonFont.setPixelSize(token.fontSizeSM);
        ribbonFont.setWeight(QFont::DemiBold);
        const QFontMetrics fm(ribbonFont);
        const int textW = fm.horizontalAdvance(m_ribbonText);
        const int ribbonW = textW + token.paddingXS * 2;
        const int ribbonH = fm.height() + token.paddingXXS * 2;
        const int foldSize = ribbonH / 2;
        const int rx = widgetRect.width() - ribbonW;
        constexpr int ry = 0;

        cache.ribbonPath.moveTo(rx, ry);
        cache.ribbonPath.lineTo(rx + ribbonW, ry);
        cache.ribbonPath.lineTo(rx + ribbonW, ry + ribbonH);
        cache.ribbonPath.lineTo(rx + ribbonW - foldSize, ry + ribbonH - foldSize);
        cache.ribbonPath.lineTo(rx, ry + ribbonH - foldSize);
        cache.ribbonPath.closeSubpath();

        cache.ribbonFoldPath.moveTo(rx + ribbonW - foldSize, ry + ribbonH - foldSize);
        cache.ribbonFoldPath.lineTo(rx + ribbonW, ry + ribbonH);
        cache.ribbonFoldPath.lineTo(rx + ribbonW - foldSize, ry + ribbonH);
        cache.ribbonFoldPath.closeSubpath();
        cache.ribbonTextRect = QRect(rx, ry, ribbonW - foldSize, ribbonH - foldSize);
        cache.ribbonFillColor = ribbonColorValue();
        cache.ribbonFoldColor = cache.ribbonFillColor.darker(130);
    }

    m_paintCache = cache;
    syncBadgePerfCounters();
    return m_paintCache;
}

void AntBadge::invalidateBadgePaintCache() const
{
    m_paintCache.valid = false;
}

QRect AntBadge::badgeVisualDirtyRect() const
{
    if (rect().isEmpty())
    {
        return {};
    }

    const auto& cache = badgePaintCache(rect());
    QRect dirty;
    if (isStatusMode() && !m_contentWidget)
    {
        dirty = cache.statusDotRect.united(cache.statusTextRect).toAlignedRect();
        if (m_status == Ant::BadgeStatus::Processing)
        {
            dirty = dirty.united(processingDirtyRectForPulse(processingPulseProgress()));
        }
    }
    else if (m_badgeMode == Ant::BadgeMode::Ribbon)
    {
        dirty = cache.ribbonPath.boundingRect().united(cache.ribbonFoldPath.boundingRect()).toAlignedRect();
        dirty = dirty.united(cache.ribbonTextRect);
    }
    else if (shouldShowIndicator())
    {
        dirty = cache.indicatorRect.toAlignedRect();
    }
    else
    {
        dirty = rect();
    }
    return dirty.adjusted(-3, -3, 3, 3).intersected(rect());
}

QRect AntBadge::processingDirtyRectForPulse(qreal pulse) const
{
    if (rect().isEmpty())
    {
        return {};
    }
    const auto& cache = badgePaintCache(rect());
    const QRectF dot = cache.statusDotRect;
    const qreal scale = 1.0 + 1.4 * std::clamp(pulse, 0.0, 1.0);
    const QRectF ringRect(dot.center().x() - dot.width() * scale / 2.0,
                          dot.center().y() - dot.height() * scale / 2.0,
                          dot.width() * scale,
                          dot.height() * scale);
    return ringRect.toAlignedRect().adjusted(-3, -3, 3, 3).intersected(rect());
}

void AntBadge::requestBadgeUpdate(const QRect& region,
                                  const QString& mode,
                                  bool indicatorScoped,
                                  bool animationScoped)
{
    QRect dirty = region.isValid() && !region.isEmpty() ? region : rect();
    dirty = dirty.intersected(rect());
    if (dirty.isEmpty())
    {
        dirty = rect();
    }

    ++m_regionUpdateCount;
    if (indicatorScoped)
    {
        ++m_indicatorRegionUpdateCount;
    }
    if (animationScoped)
    {
        ++m_animationRegionUpdateCount;
    }
    m_lastUpdateMode = mode;
    syncBadgePerfCounters();

    if (m_indicatorOverlay)
    {
        m_indicatorOverlay->update(dirty);
    }
    else
    {
        update(dirty);
    }
}

void AntBadge::syncBadgePerfCounters() const
{
    auto* self = const_cast<AntBadge*>(this);
    self->setProperty("antBadgeLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antBadgeLayoutCacheHitCount", m_layoutCacheHitCount);
    self->setProperty("antBadgeRegionUpdateCount", m_regionUpdateCount);
    self->setProperty("antBadgeIndicatorRegionUpdateCount", m_indicatorRegionUpdateCount);
    self->setProperty("antBadgeAnimationRegionUpdateCount", m_animationRegionUpdateCount);
    self->setProperty("antBadgeLastUpdateMode", m_lastUpdateMode);
    self->setProperty("antBadgeAnimationTimerActive", m_animationTimer && m_animationTimer->isActive());
}

void AntBadge::updateContentGeometry()
{
    if (!m_contentWidget)
    {
        return;
    }
    const QRect target = badgePaintCache(rect()).contentRect;
    if (m_contentWidget->geometry() != target)
    {
        m_contentWidget->setGeometry(target);
    }
}

void AntBadge::updateAnimationState()
{
    const bool animate = isVisible() && isEnabled() && m_status == Ant::BadgeStatus::Processing && !m_contentWidget;
    if (animate && !m_animationTimer->isActive())
    {
        m_animationTimer->start(40);
    }
    else if (!animate)
    {
        m_animationTimer->stop();
    }
    syncBadgePerfCounters();
}

void AntBadge::drawIndicator(QPainter& painter)
{
    const auto& token = antTheme->tokens();
    const QRectF r = indicatorRect();
    QColor fill = isEnabled() ? badgeColor() : token.colorTextDisabled;

    painter.setPen(QPen(token.colorBgContainer, token.lineWidth));
    painter.setBrush(fill);
    if (m_dot)
    {
        painter.drawEllipse(r);
        return;
    }

    painter.drawRoundedRect(r, r.height() / 2.0, r.height() / 2.0);

    QFont f = painter.font();
    f.setPixelSize(token.fontSizeSM);
    f.setWeight(QFont::Normal);
    painter.setFont(f);
    painter.setPen(token.colorTextLightSolid);
    painter.drawText(r.adjusted(token.paddingXXS, 0, -token.paddingXXS, 0), Qt::AlignCenter, displayText());
}

void AntBadge::drawStatus(QPainter& painter)
{
    const auto& token = antTheme->tokens();
    const QRectF dot = standaloneStatusDotRect();
    const QColor color = isEnabled() ? statusColor() : token.colorTextDisabled;

    if (m_status == Ant::BadgeStatus::Processing)
    {
        QColor ring = color;
        ring.setAlphaF(std::max(0.0, 0.45 * (1.0 - m_pulse / 100.0)));
        const qreal scale = 1.0 + 1.4 * (m_pulse / 100.0);
        QRectF ringRect(dot.center().x() - dot.width() * scale / 2.0,
                        dot.center().y() - dot.height() * scale / 2.0,
                        dot.width() * scale,
                        dot.height() * scale);
        painter.setPen(QPen(ring, token.lineWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(ringRect);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(dot);

    if (!m_text.isEmpty())
    {
        QFont f = painter.font();
        f.setPixelSize(token.fontSize);
        painter.setFont(f);
        painter.setPen(isEnabled() ? token.colorText : token.colorTextDisabled);
        const QRectF textRect(dot.right() + token.marginXS, 0, width() - dot.right() - token.marginXS, height());
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_text);
    }
}
