#include "AntResult.h"

#include <QFontMetrics>
#include <QPainter>
#include <QResizeEvent>

#include "core/AntTheme.h"
#include "styles/AntIconPainter.h"
#include "styles/AntPalette.h"
#include "styles/AntResultStyle.h"

AntResult::AntResult(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntResultStyle>(this);
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateResultLayout();
        invalidateResultIconPixmap();
        syncExtraGeometry();
        requestResultUpdate(rect(), QStringLiteral("theme"));
    });
    syncResultPerfCounters();
}

AntResult::AntResult(const QString& title, QWidget* parent)
    : AntResult(parent)
{
    m_title = title;
}

Ant::AlertType AntResult::status() const { return m_status; }

void AntResult::setStatus(Ant::AlertType status)
{
    if (m_status == status)
    {
        return;
    }
    m_status = status;
    invalidateResultLayout();
    invalidateResultIconPixmap();
    requestResultUpdate(iconRect(), QStringLiteral("status"));
    Q_EMIT statusChanged(m_status);
}

QString AntResult::title() const { return m_title; }

void AntResult::setTitle(const QString& title)
{
    if (m_title == title)
    {
        return;
    }
    m_title = title;
    invalidateResultLayout();
    syncExtraGeometry();
    updateGeometry();
    requestResultUpdate(rect(), QStringLiteral("title"));
    Q_EMIT titleChanged(m_title);
}

QString AntResult::subTitle() const { return m_subTitle; }

void AntResult::setSubTitle(const QString& subTitle)
{
    if (m_subTitle == subTitle)
    {
        return;
    }
    m_subTitle = subTitle;
    invalidateResultLayout();
    syncExtraGeometry();
    updateGeometry();
    requestResultUpdate(rect(), QStringLiteral("subTitle"));
    Q_EMIT subTitleChanged(m_subTitle);
}

bool AntResult::isIconVisible() const { return m_iconVisible; }

void AntResult::setIconVisible(bool visible)
{
    if (m_iconVisible == visible)
    {
        return;
    }
    m_iconVisible = visible;
    invalidateResultLayout();
    syncExtraGeometry();
    updateGeometry();
    requestResultUpdate(rect(), QStringLiteral("iconVisible"));
    Q_EMIT iconVisibleChanged(m_iconVisible);
}

QWidget* AntResult::extraWidget() const
{
    return m_extraWidget.data();
}

void AntResult::setExtraWidget(QWidget* widget)
{
    if (m_extraWidget == widget)
    {
        return;
    }
    if (m_extraWidget)
    {
        m_extraWidget->setParent(nullptr);
    }
    m_extraWidget = widget;
    if (m_extraWidget)
    {
        m_extraWidget->setParent(this);
        m_extraWidget->show();
    }
    invalidateResultLayout();
    syncExtraGeometry();
    updateGeometry();
    requestResultUpdate(rect(), QStringLiteral("extra"));
}

QSize AntResult::sizeHint() const
{
    return resultLayout().sizeHint;
}

QSize AntResult::minimumSizeHint() const
{
    return resultLayout().minimumSizeHint;
}

void AntResult::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntResult::resizeEvent(QResizeEvent* event)
{
    syncExtraGeometry();
    QWidget::resizeEvent(event);
}

AntResult::Metrics AntResult::metrics() const
{
    return {};
}

const AntResult::ResultLayoutCache& AntResult::resultLayout() const
{
    const auto& token = antTheme->tokens();
    const bool hasExtraWidget = m_extraWidget != nullptr;
    const QSize extraSize = m_extraWidget ? m_extraWidget->sizeHint() : QSize();
    if (m_layoutCache.valid &&
        m_layoutCache.widgetSize == size() &&
        m_layoutCache.font == font() &&
        m_layoutCache.themeMode == antTheme->themeMode() &&
        m_layoutCache.tokenFontSize == token.fontSize &&
        m_layoutCache.title == m_title &&
        m_layoutCache.subTitle == m_subTitle &&
        m_layoutCache.status == m_status &&
        m_layoutCache.iconVisible == m_iconVisible &&
        m_layoutCache.hasExtraWidget == hasExtraWidget &&
        m_layoutCache.extraSize == extraSize)
    {
        ++m_layoutCacheHitCount;
        syncResultPerfCounters();
        return m_layoutCache;
    }

    ++m_layoutBuildCount;
    ResultLayoutCache cache;
    cache.valid = true;
    cache.widgetSize = size();
    cache.font = font();
    cache.themeMode = antTheme->themeMode();
    cache.tokenFontSize = token.fontSize;
    cache.title = m_title;
    cache.subTitle = m_subTitle;
    cache.status = m_status;
    cache.iconVisible = m_iconVisible;
    cache.hasExtraWidget = hasExtraWidget;
    cache.extraSize = extraSize;
    cache.metrics = metrics();
    cache.minimumSizeHint = QSize(180, 120);
    cache.iconColor = iconColor();
    cache.titleColor = token.colorText;
    cache.subTitleColor = token.colorTextSecondary;
    cache.iconSecondaryColor = token.colorTextLightSolid;
    cache.iconType = iconTypeForStatus();

    const Metrics& m = cache.metrics;
    const int contentWidth = qMax(240, QWidget::width() > 0 ? QWidget::width() : 400);
    const int textWidth = qMax(120, contentWidth - m.padding * 2);
    const int widgetTextWidth = qMax(120, width() - m.padding * 2);

    QFont titleFont = font();
    titleFont.setPixelSize(m.titleFontSize);
    titleFont.setWeight(QFont::DemiBold);
    const QFontMetrics titleFm(titleFont);

    QFont subFont = font();
    subFont.setPixelSize(m.subTitleFontSize);
    const QFontMetrics subFm(subFont);

    int hintHeight = m.padding;
    if (m_iconVisible)
    {
        hintHeight += m.iconSize + m.spacing;
    }

    const int hintTitleHeight =
        titleFm.boundingRect(QRect(0, 0, textWidth, 200), Qt::AlignCenter | Qt::TextWordWrap, m_title).height();
    const int titleHeight =
        titleFm.boundingRect(QRect(0, 0, widgetTextWidth, 200), Qt::AlignCenter | Qt::TextWordWrap, m_title).height();
    hintHeight += hintTitleHeight;

    int top = m.padding;
    if (m_iconVisible)
    {
        cache.iconRect = QRect((width() - m.iconSize) / 2, top, m.iconSize, m.iconSize);
        top += m.iconSize + m.spacing;
    }
    cache.titleRect = QRect(m.padding, top, widgetTextWidth, titleHeight);
    top += titleHeight;

    if (!m_subTitle.isEmpty())
    {
        hintHeight += m.spacing;
        const int hintSubTitleHeight =
            subFm.boundingRect(QRect(0, 0, textWidth, 200), Qt::AlignCenter | Qt::TextWordWrap, m_subTitle).height();
        hintHeight += hintSubTitleHeight;

        top += m.spacing;
        const int subTitleHeight =
            subFm.boundingRect(QRect(0, 0, widgetTextWidth, 200), Qt::AlignCenter | Qt::TextWordWrap, m_subTitle).height();
        cache.subTitleRect = QRect(m.padding, top, widgetTextWidth, subTitleHeight);
        top += subTitleHeight;
    }

    if (hasExtraWidget)
    {
        hintHeight += m.extraSpacing + extraSize.height();
        top += m.extraSpacing;
        cache.extraRect = QRect((width() - extraSize.width()) / 2, top, extraSize.width(), extraSize.height());
    }

    hintHeight += m.padding;
    cache.sizeHint = QSize(contentWidth, hintHeight);

    m_layoutCache = cache;
    syncResultPerfCounters();
    return m_layoutCache;
}

QPixmap AntResult::statusIconPixmap(qreal devicePixelRatio) const
{
    const auto& layout = resultLayout();
    const qreal dpr = qMax<qreal>(1.0, devicePixelRatio);
    if (m_iconPixmapCache.valid &&
        qFuzzyCompare(m_iconPixmapCache.devicePixelRatio, dpr) &&
        m_iconPixmapCache.logicalSize == layout.iconRect.size() &&
        m_iconPixmapCache.status == layout.status &&
        m_iconPixmapCache.themeMode == layout.themeMode &&
        m_iconPixmapCache.iconColor == layout.iconColor &&
        m_iconPixmapCache.secondaryColor == layout.iconSecondaryColor &&
        m_iconPixmapCache.iconType == layout.iconType)
    {
        ++m_iconPixmapCacheHitCount;
        syncResultPerfCounters();
        return m_iconPixmapCache.pixmap;
    }

    ++m_iconPixmapBuildCount;
    ResultIconPixmapCache cache;
    cache.valid = true;
    cache.devicePixelRatio = dpr;
    cache.logicalSize = layout.iconRect.size();
    cache.status = layout.status;
    cache.themeMode = layout.themeMode;
    cache.iconColor = layout.iconColor;
    cache.secondaryColor = layout.iconSecondaryColor;
    cache.iconType = layout.iconType;
    cache.pixmap = QPixmap(cache.logicalSize * dpr);
    cache.pixmap.setDevicePixelRatio(dpr);
    cache.pixmap.fill(Qt::transparent);

    QPainter painter(&cache.pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    AntIconPainter::drawIcon(painter,
                             cache.iconType,
                             QRectF(QPointF(0, 0), QSizeF(cache.logicalSize)),
                             cache.iconColor,
                             Ant::IconTheme::Filled,
                             cache.secondaryColor);

    m_iconPixmapCache = cache;
    syncResultPerfCounters();
    return m_iconPixmapCache.pixmap;
}

void AntResult::invalidateResultLayout() const
{
    m_layoutCache.valid = false;
}

void AntResult::invalidateResultIconPixmap() const
{
    m_iconPixmapCache.valid = false;
}

void AntResult::requestResultUpdate(const QRect& region, const QString& mode)
{
    m_lastUpdateMode = mode;
    ++m_regionUpdateCount;
    syncResultPerfCounters();
    if (region.isValid() && !region.isEmpty())
    {
        update(region);
        return;
    }
    update();
}

void AntResult::syncResultPerfCounters() const
{
    auto* self = const_cast<AntResult*>(this);
    self->setProperty("antResultLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antResultLayoutCacheHitCount", m_layoutCacheHitCount);
    self->setProperty("antResultIconPixmapBuildCount", m_iconPixmapBuildCount);
    self->setProperty("antResultIconPixmapCacheHitCount", m_iconPixmapCacheHitCount);
    self->setProperty("antResultExtraGeometryApplyCount", m_extraGeometryApplyCount);
    self->setProperty("antResultExtraGeometrySkipCount", m_extraGeometrySkipCount);
    self->setProperty("antResultRegionUpdateCount", m_regionUpdateCount);
    self->setProperty("antResultLastUpdateMode", m_lastUpdateMode);
}

QRect AntResult::iconRect() const
{
    return resultLayout().iconRect;
}

QRect AntResult::titleRect() const
{
    return resultLayout().titleRect;
}

QRect AntResult::subTitleRect() const
{
    return resultLayout().subTitleRect;
}

QRect AntResult::extraRect() const
{
    return resultLayout().extraRect;
}

void AntResult::syncExtraGeometry()
{
    if (m_extraWidget)
    {
        const QRect targetGeometry = extraRect();
        if (m_extraWidget->geometry() == targetGeometry && m_extraWidget->isVisible())
        {
            ++m_extraGeometrySkipCount;
            syncResultPerfCounters();
            return;
        }

        ++m_extraGeometryApplyCount;
        m_extraWidget->setGeometry(targetGeometry);
        m_extraWidget->show();
        syncResultPerfCounters();
    }
}

QColor AntResult::iconColor() const
{
    const auto& token = antTheme->tokens();
    switch (m_status)
    {
    case Ant::AlertType::Success:
        return token.colorSuccess;
    case Ant::AlertType::Warning:
        return token.colorWarning;
    case Ant::AlertType::Error:
        return token.colorError;
    case Ant::AlertType::Info:
    default:
        return token.colorPrimary;
    }
}

Ant::IconType AntResult::iconTypeForStatus() const
{
    switch (m_status)
    {
    case Ant::AlertType::Success:
        return Ant::IconType::CheckCircle;
    case Ant::AlertType::Error:
        return Ant::IconType::CloseCircle;
    case Ant::AlertType::Warning:
        return Ant::IconType::ExclamationCircle;
    case Ant::AlertType::Info:
    default:
        return Ant::IconType::InfoCircle;
    }
}
