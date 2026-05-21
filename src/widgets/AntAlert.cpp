#include "AntAlert.h"

#include <QEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QSizePolicy>

#include "AntButton.h"
#include "AntIcon.h"
#include "core/AntTheme.h"
#include "styles/AntAlertStyle.h"
#include "styles/AntPalette.h"

AntAlert::AntAlert(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntAlertStyle>(this);
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateAlertLayout();
        clearIconPixmapCache();
        syncLayout();
        updateGeometry();
        requestAlertUpdate(rect(), QStringLiteral("theme"));
    });
    syncAlertPerfCounters();
}

AntAlert::AntAlert(const QString& title, QWidget* parent)
    : AntAlert(parent)
{
    m_title = title;
}

QString AntAlert::title() const { return m_title; }

void AntAlert::setTitle(const QString& title)
{
    if (m_title == title)
    {
        return;
    }
    m_title = title;
    invalidateAlertLayout();
    updateGeometry();
    syncLayout();
    requestAlertUpdate(rect(), QStringLiteral("title"));
    Q_EMIT titleChanged(m_title);
}

QString AntAlert::description() const { return m_description; }

void AntAlert::setDescription(const QString& description)
{
    if (m_description == description)
    {
        return;
    }
    m_description = description;
    invalidateAlertLayout();
    updateGeometry();
    syncLayout();
    requestAlertUpdate(rect(), QStringLiteral("description"));
    Q_EMIT descriptionChanged(m_description);
}

Ant::AlertType AntAlert::alertType() const { return m_alertType; }

void AntAlert::setAlertType(Ant::AlertType type)
{
    if (m_alertType == type)
    {
        return;
    }
    m_alertType = type;
    invalidateAlertLayout();
    requestAlertUpdate(rect(), QStringLiteral("alertType"));
    Q_EMIT alertTypeChanged(m_alertType);
}

bool AntAlert::showIcon() const { return m_showIcon; }

void AntAlert::setShowIcon(bool showIcon)
{
    if (m_showIcon == showIcon)
    {
        return;
    }
    m_showIcon = showIcon;
    invalidateAlertLayout();
    updateGeometry();
    syncLayout();
    requestAlertUpdate(rect(), QStringLiteral("showIcon"));
    Q_EMIT showIconChanged(m_showIcon);
}

bool AntAlert::isClosable() const { return m_closable; }

void AntAlert::setClosable(bool closable)
{
    if (m_closable == closable)
    {
        return;
    }
    m_closable = closable;
    if (!m_closable)
    {
        m_hoverClose = false;
    }
    invalidateAlertLayout();
    updateGeometry();
    syncLayout();
    requestAlertUpdate(rect(), QStringLiteral("closable"));
    Q_EMIT closableChanged(m_closable);
}

bool AntAlert::isBanner() const { return m_banner; }

void AntAlert::setBanner(bool banner)
{
    if (m_banner == banner)
    {
        return;
    }
    m_banner = banner;
    if (m_banner && !m_showIcon)
    {
        m_showIcon = true;
        Q_EMIT showIconChanged(m_showIcon);
    }
    if (m_banner && m_alertType == Ant::AlertType::Info)
    {
        m_alertType = Ant::AlertType::Warning;
        Q_EMIT alertTypeChanged(m_alertType);
    }
    invalidateAlertLayout();
    updateGeometry();
    syncLayout();
    requestAlertUpdate(rect(), QStringLiteral("banner"));
    Q_EMIT bannerChanged(m_banner);
}

QWidget* AntAlert::actionWidget() const
{
    return m_actionWidget.data();
}

void AntAlert::setActionWidget(QWidget* widget)
{
    if (m_actionWidget == widget)
    {
        return;
    }

    if (m_actionWidget && m_actionWidget->parent() == this)
    {
        m_actionWidget->deleteLater();
    }

    m_actionWidget = widget;
    if (m_actionWidget)
    {
        m_actionWidget->setParent(this);
        m_actionWidget->show();
    }
    invalidateAlertLayout();
    syncLayout();
    updateGeometry();
    requestAlertUpdate(rect(), QStringLiteral("actionWidget"));
}

QSize AntAlert::sizeHint() const
{
    return alertLayout().sizeHint;
}

QSize AntAlert::minimumSizeHint() const
{
    return alertLayout().minimumSizeHint;
}

void AntAlert::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntAlert::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange)
    {
        invalidateAlertLayout();
        clearIconPixmapCache();
        syncLayout();
        updateGeometry();
        requestAlertUpdate(rect(), QStringLiteral("styleChange"));
    }
    QWidget::changeEvent(event);
}

void AntAlert::resizeEvent(QResizeEvent* event)
{
    invalidateAlertLayout();
    syncLayout();
    QWidget::resizeEvent(event);
}

void AntAlert::mouseMoveEvent(QMouseEvent* event)
{
    const QRect close = closeRect();
    const bool hover = m_closable && close.contains(event->pos());
    if (m_hoverClose != hover)
    {
        const QRect oldClose = close;
        m_hoverClose = hover;
        requestAlertUpdate(oldClose.united(closeRect()), QStringLiteral("closeHover"), true);
    }
    QWidget::mouseMoveEvent(event);
}

void AntAlert::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_closable && closeRect().contains(event->pos()))
    {
        Q_EMIT closeRequested();
        hide();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntAlert::leaveEvent(QEvent* event)
{
    if (m_hoverClose)
    {
        const QRect close = closeRect();
        m_hoverClose = false;
        requestAlertUpdate(close, QStringLiteral("closeHover"), true);
    }
    QWidget::leaveEvent(event);
}

AntAlert::Metrics AntAlert::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    m.minHeight = m_description.isEmpty() ? token.controlHeightLG : 72;
    m.radius = m_banner ? 0 : token.borderRadiusLG;
    m.paddingX = m_banner ? token.paddingLG : token.padding;
    m.paddingY = m_description.isEmpty() ? token.paddingXS : token.paddingSM;
    m.iconSize = m_description.isEmpty() ? 16 : 18;
    m.titleFontSize = token.fontSize;
    m.descFontSize = token.fontSize;
    m.closeSize = 20;
    return m;
}

QColor AntAlert::backgroundColor() const
{
    const auto& token = antTheme->tokens();
    switch (m_alertType)
    {
    case Ant::AlertType::Success:
        return token.colorSuccessBg;
    case Ant::AlertType::Warning:
        return token.colorWarningBg;
    case Ant::AlertType::Error:
        return token.colorErrorBg;
    case Ant::AlertType::Info:
    default:
        return token.colorPrimaryBg;
    }
}

QColor AntAlert::borderColor() const
{
    switch (m_alertType)
    {
    case Ant::AlertType::Success:
        return AntPalette::tint(antTheme->tokens().colorSuccess, 0.62);
    case Ant::AlertType::Warning:
        return AntPalette::tint(antTheme->tokens().colorWarning, 0.62);
    case Ant::AlertType::Error:
        return AntPalette::tint(antTheme->tokens().colorError, 0.62);
    case Ant::AlertType::Info:
    default:
        return AntPalette::tint(antTheme->tokens().colorPrimary, 0.62);
    }
}

QColor AntAlert::iconColor() const
{
    switch (m_alertType)
    {
    case Ant::AlertType::Success:
        return antTheme->tokens().colorSuccess;
    case Ant::AlertType::Warning:
        return antTheme->tokens().colorWarning;
    case Ant::AlertType::Error:
        return antTheme->tokens().colorError;
    case Ant::AlertType::Info:
    default:
        return antTheme->tokens().colorPrimary;
    }
}

QColor AntAlert::titleColor() const
{
    return antTheme->tokens().colorText;
}

QColor AntAlert::descriptionColor() const
{
    return antTheme->tokens().colorTextSecondary;
}

QRect AntAlert::closeRect() const
{
    return alertLayout().closeRect;
}

QRect AntAlert::actionRect() const
{
    return alertLayout().actionRect;
}

QRect AntAlert::contentRect() const
{
    return alertLayout().contentRect;
}

const AntAlert::AlertLayout& AntAlert::alertLayout() const
{
    const QSize actionSize = m_actionWidget ? m_actionWidget->sizeHint() : QSize();
    const bool cacheMatches = m_layoutCache.valid
        && m_layoutCache.widgetSize == size()
        && m_layoutCache.title == m_title
        && m_layoutCache.description == m_description
        && m_layoutCache.alertType == m_alertType
        && m_layoutCache.showIcon == m_showIcon
        && m_layoutCache.closable == m_closable
        && m_layoutCache.banner == m_banner
        && m_layoutCache.actionSize == actionSize;

    if (cacheMatches)
    {
        ++m_layoutCacheHitCount;
        syncAlertPerfCounters();
        return m_layoutCache;
    }

    ++m_layoutBuildCount;

    AlertLayout layout;
    layout.valid = true;
    layout.widgetSize = size();
    layout.title = m_title;
    layout.description = m_description;
    layout.alertType = m_alertType;
    layout.showIcon = m_showIcon;
    layout.closable = m_closable;
    layout.banner = m_banner;
    layout.actionSize = actionSize;
    layout.metrics = metrics();
    layout.bodyRect = rect();
    layout.contentRect = rect().adjusted(layout.metrics.paddingX,
                                         layout.metrics.paddingY,
                                         -layout.metrics.paddingX,
                                         -layout.metrics.paddingY);
    layout.hasDescription = !m_description.isEmpty();
    layout.showIconEffective = m_showIcon || m_banner;
    layout.minimumSizeHint = QSize(180, layout.metrics.minHeight);

    const int widthForHint = qMax(220, width() > 0 ? width() : 420);
    int hintLeft = layout.metrics.paddingX;
    if (layout.showIconEffective)
    {
        hintLeft += layout.metrics.iconSize + 12;
    }
    int hintRight = layout.metrics.paddingX;
    if (m_closable)
    {
        hintRight += layout.metrics.closeSize + 8;
    }
    if (m_actionWidget)
    {
        hintRight += actionSize.width() + layout.metrics.actionSpacing;
    }

    QFont titleFont = font();
    titleFont.setPixelSize(layout.metrics.titleFontSize);
    titleFont.setWeight(layout.hasDescription ? QFont::DemiBold : QFont::Normal);
    QFontMetrics titleFm(titleFont);

    QFont descFont = font();
    descFont.setPixelSize(layout.metrics.descFontSize);
    QFontMetrics descFm(descFont);

    const int textWidthForHint = qMax(120, widthForHint - hintLeft - hintRight);
    int contentHeight = qMax(layout.metrics.minHeight - layout.metrics.paddingY * 2, titleFm.height());
    if (layout.hasDescription)
    {
        const QRect descBounds = descFm.boundingRect(QRect(0, 0, textWidthForHint, 1000),
                                                     Qt::TextWordWrap,
                                                     m_description);
        contentHeight = titleFm.height() + 6 + descBounds.height();
    }
    layout.sizeHint = QSize(widthForHint, qMax(layout.metrics.minHeight, contentHeight + layout.metrics.paddingY * 2));

    layout.textLeft = layout.contentRect.left();
    layout.textRight = layout.contentRect.right();
    if (layout.showIconEffective)
    {
        layout.iconRect = QRect(layout.textLeft,
                                layout.contentRect.top() + (layout.hasDescription
                                    ? 2
                                    : (layout.contentRect.height() - layout.metrics.iconSize) / 2),
                                layout.metrics.iconSize,
                                layout.metrics.iconSize);
        layout.textLeft = layout.iconRect.right() + 12;
    }

    if (m_closable)
    {
        layout.closeRect = QRect(width() - layout.metrics.paddingX - layout.metrics.closeSize,
                                 layout.contentRect.top(),
                                 layout.metrics.closeSize,
                                 layout.metrics.closeSize);
        layout.textRight = qMin(layout.textRight, layout.closeRect.left() - 8);
    }

    if (m_actionWidget)
    {
        const int x = width() - layout.metrics.paddingX - actionSize.width()
            - (m_closable ? (layout.metrics.closeSize + 8) : 0);
        const int y = layout.contentRect.top() + (layout.contentRect.height() - actionSize.height()) / 2;
        layout.actionRect = QRect(x, y, actionSize.width(), actionSize.height());
        layout.textRight = qMin(layout.textRight, layout.actionRect.left() - layout.metrics.actionSpacing);
    }

    const int textWidth = qMax(40, layout.textRight - layout.textLeft);
    layout.titleRect = QRect(layout.textLeft,
                             layout.contentRect.top(),
                             textWidth,
                             layout.hasDescription ? titleFont.pixelSize() + 6 : layout.contentRect.height());
    if (layout.hasDescription)
    {
        layout.descriptionRect = QRect(layout.textLeft,
                                       layout.titleRect.bottom() + 6,
                                       textWidth,
                                       layout.contentRect.bottom() - layout.titleRect.bottom() - 6);
    }

    m_layoutCache = layout;
    syncAlertPerfCounters();
    return m_layoutCache;
}

void AntAlert::invalidateAlertLayout() const
{
    m_layoutCache.valid = false;
}

QPixmap AntAlert::cachedIconPixmap(Ant::IconType iconType, const QColor& color, int iconSize) const
{
    const qreal ratio = devicePixelRatioF();
    const QString key = QStringLiteral("%1:%2:%3:%4")
        .arg(static_cast<int>(iconType))
        .arg(color.rgba())
        .arg(iconSize)
        .arg(qRound(ratio * 100.0));

    auto it = m_iconPixmapCache.constFind(key);
    if (it != m_iconPixmapCache.constEnd())
    {
        ++m_iconPixmapCacheHitCount;
        syncAlertPerfCounters();
        return it.value();
    }

    ++m_iconPixmapBuildCount;
    AntIcon icon(iconType);
    icon.setIconTheme(Ant::IconTheme::Filled);
    icon.setColor(color);
    icon.setIconSize(iconSize);
    icon.resize(iconSize, iconSize);

    const QSize pixmapSize(qMax(1, qRound(iconSize * ratio)), qMax(1, qRound(iconSize * ratio)));
    QPixmap pixmap(pixmapSize);
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);
    icon.render(&pixmap);
    m_iconPixmapCache.insert(key, pixmap);
    syncAlertPerfCounters();
    return pixmap;
}

void AntAlert::clearIconPixmapCache() const
{
    m_iconPixmapCache.clear();
}

void AntAlert::requestAlertUpdate(const QRect& region, const QString& mode, bool closeScoped)
{
    m_lastUpdateMode = mode;
    ++m_regionUpdateCount;
    if (closeScoped)
    {
        ++m_closeRegionUpdateCount;
    }
    syncAlertPerfCounters();
    if (region.isValid() && !region.isEmpty())
    {
        update(region);
        return;
    }
    update();
}

void AntAlert::syncAlertPerfCounters() const
{
    auto* that = const_cast<AntAlert*>(this);
    that->setProperty("antAlertLayoutBuildCount", m_layoutBuildCount);
    that->setProperty("antAlertLayoutCacheHitCount", m_layoutCacheHitCount);
    that->setProperty("antAlertIconPixmapBuildCount", m_iconPixmapBuildCount);
    that->setProperty("antAlertIconPixmapCacheHitCount", m_iconPixmapCacheHitCount);
    that->setProperty("antAlertRegionUpdateCount", m_regionUpdateCount);
    that->setProperty("antAlertCloseRegionUpdateCount", m_closeRegionUpdateCount);
    that->setProperty("antAlertLastUpdateMode", m_lastUpdateMode);
}

void AntAlert::syncLayout()
{
    if (m_actionWidget)
    {
        m_actionWidget->setGeometry(actionRect());
        m_actionWidget->show();
    }
}

Ant::IconType AntAlert::iconTypeForAlert() const
{
    switch (m_alertType)
    {
    case Ant::AlertType::Success:
        return Ant::IconType::CheckCircle;
    case Ant::AlertType::Warning:
        return Ant::IconType::ExclamationCircle;
    case Ant::AlertType::Error:
        return Ant::IconType::CloseCircle;
    case Ant::AlertType::Info:
    default:
        return Ant::IconType::InfoCircle;
    }
}
