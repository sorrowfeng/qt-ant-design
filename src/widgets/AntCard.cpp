#include "AntCard.h"

#include <QEvent>
#include <QGridLayout>
#include <QHideEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QShowEvent>
#include <QVBoxLayout>

#include "../styles/AntCardStyle.h"
#include "core/AntTheme.h"
#include "core/AntThemeRefresh_p.h"

AntCard::AntCard(QWidget* parent)
    : QFrame(parent)
{
    installAntStyle<AntCardStyle>(this);
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFrameShape(QFrame::NoFrame);

    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setSpacing(0);
    m_rootLayout->setContentsMargins(1, 1, 1, 1);

    m_header = new QWidget(this);
    m_header->setObjectName(QStringLiteral("ant-card-header"));
    auto* headerLayout = new QHBoxLayout(m_header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(12);
    m_titleLabel = new QLabel(m_header);
    m_extraLabel = new QLabel(m_header);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    headerLayout->addWidget(m_titleLabel, 1);
    headerLayout->addWidget(m_extraLabel);

    m_body = new QWidget(this);
    m_body->setObjectName(QStringLiteral("ant-card-body"));
    m_bodyLayout = new QVBoxLayout(m_body);
    m_bodyLayout->setSpacing(8);

    m_meta = new QWidget(this);
    m_meta->setObjectName(QStringLiteral("ant-card-meta"));
    m_metaLayout = new QHBoxLayout(m_meta);
    m_metaLayout->setContentsMargins(0, 0, 0, 0);
    m_metaLayout->setSpacing(12);
    m_metaAvatarContainer = new QWidget(m_meta);
    m_metaAvatarContainer->setFixedSize(0, 0);
    m_metaAvatarContainer->hide();
    m_metaTextLayout = new QVBoxLayout();
    m_metaTextLayout->setSpacing(8);
    m_metaTextLayout->setContentsMargins(0, 0, 0, 0);
    m_metaTitleLabel = new QLabel(m_meta);
    m_metaDescLabel = new QLabel(m_meta);
    m_metaTextLayout->addWidget(m_metaTitleLabel);
    m_metaTextLayout->addWidget(m_metaDescLabel);
    m_metaLayout->addWidget(m_metaAvatarContainer);
    m_metaLayout->addLayout(m_metaTextLayout, 1);
    m_meta->hide();

    m_actions = new QWidget(this);
    m_actions->setObjectName(QStringLiteral("ant-card-actions"));
    m_actionsLayout = new QHBoxLayout(m_actions);
    m_actionsLayout->setContentsMargins(0, 0, 0, 0);
    m_actionsLayout->setSpacing(0);

    m_rootLayout->addWidget(m_header);
    m_rootLayout->addWidget(m_meta);
    m_rootLayout->addWidget(m_body, 1);
    m_rootLayout->addWidget(m_actions);

    connect(&m_spinnerTimer, &QTimer::timeout, this, [this]() {
        m_spinnerAngle = (m_spinnerAngle + 30) % 360;
        requestCardUpdate(spinnerDirtyRect(), QStringLiteral("spinner"), true);
    });
    m_spinnerTimer.setTimerType(Qt::PreciseTimer);
    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateCardPaintCache();
        updateTheme();
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
        requestCardUpdate(rect(), QStringLiteral("theme"));
    });

    rebuildChrome();
    updateTheme();
    syncCardPerfCounters();
}

AntCard::AntCard(const QString& title, QWidget* parent)
    : AntCard(parent)
{
    setTitle(title);
}

QString AntCard::title() const { return m_title; }

void AntCard::setTitle(const QString& title)
{
    if (m_title == title)
        return;
    m_title = title;
    m_titleLabel->setText(title);
    rebuildChrome();
    requestCardUpdate(rect(), QStringLiteral("title"));
    Q_EMIT titleChanged(m_title);
}

QString AntCard::extra() const { return m_extra; }

void AntCard::setExtra(const QString& extra)
{
    if (m_extra == extra)
        return;
    m_extra = extra;
    m_extraLabel->setText(extra);
    rebuildChrome();
    requestCardUpdate(rect(), QStringLiteral("extra"));
    Q_EMIT extraChanged(m_extra);
}

bool AntCard::isBordered() const { return m_bordered; }

void AntCard::setBordered(bool bordered)
{
    if (m_bordered == bordered)
        return;
    m_bordered = bordered;
    invalidateCardPaintCache();
    requestCardUpdate(rect(), QStringLiteral("bordered"));
    Q_EMIT borderedChanged(m_bordered);
}

bool AntCard::isHoverable() const { return m_hoverable; }

void AntCard::setHoverable(bool hoverable)
{
    if (m_hoverable == hoverable)
        return;
    m_hoverable = hoverable;
    setCursor(hoverable ? Qt::PointingHandCursor : Qt::ArrowCursor);
    invalidateCardPaintCache();
    requestCardUpdate(rect(), QStringLiteral("hoverable"));
    Q_EMIT hoverableChanged(m_hoverable);
}

bool AntCard::isLoading() const { return m_loading; }

void AntCard::setLoading(bool loading)
{
    if (m_loading == loading)
        return;
    m_loading = loading;
    updateLoadingTimer();
    invalidateCardPaintCache();
    requestCardUpdate(rect(), QStringLiteral("loading"));
    Q_EMIT loadingChanged(m_loading);
}

Ant::CardSize AntCard::cardSize() const { return m_cardSize; }

void AntCard::setCardSize(Ant::CardSize size)
{
    if (m_cardSize == size)
        return;
    m_cardSize = size;
    rebuildChrome();
    updateTheme();
    requestCardUpdate(rect(), QStringLiteral("size"));
    Q_EMIT cardSizeChanged(m_cardSize);
}

QWidget* AntCard::bodyWidget() const { return m_body; }
QVBoxLayout* AntCard::bodyLayout() const { return m_bodyLayout; }

void AntCard::setBodyWidget(QWidget* widget)
{
    while (QLayoutItem* item = m_bodyLayout->takeAt(0))
    {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    if (widget)
    {
        widget->setParent(m_body);
        m_bodyLayout->addWidget(widget);
    }
    invalidateCardPaintCache();
    requestCardUpdate(rect(), QStringLiteral("body"));
}

void AntCard::setCoverWidget(QWidget* widget)
{
    if (m_cover)
    {
        m_rootLayout->removeWidget(m_cover);
        m_cover->deleteLater();
        m_cover = nullptr;
    }
    m_cover = widget;
    if (m_cover)
    {
        m_cover->setParent(this);
        m_rootLayout->insertWidget(m_header->isVisible() ? 1 : 0, m_cover);
    }
    invalidateCardPaintCache();
    requestCardUpdate(rect(), QStringLiteral("cover"));
}

void AntCard::addActionWidget(QWidget* widget)
{
    if (!widget)
        return;
    widget->setParent(m_actions);
    m_actionsLayout->addWidget(widget, 1, Qt::AlignCenter);
    rebuildChrome();
    requestCardUpdate(rect(), QStringLiteral("action"));
}

void AntCard::clearActions()
{
    while (QLayoutItem* item = m_actionsLayout->takeAt(0))
    {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    rebuildChrome();
    requestCardUpdate(rect(), QStringLiteral("action"));
}

void AntCard::setMetaAvatar(QWidget* avatar)
{
    // Remove old avatar
    if (auto* old = m_metaAvatarContainer->findChild<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
    {
        m_metaAvatarContainer->layout()->removeWidget(old);
        old->deleteLater();
    }
    // Clear existing layout children
    while (QLayoutItem* item = m_metaAvatarContainer->layout() ? m_metaAvatarContainer->layout()->takeAt(0) : nullptr)
    {
        delete item;
    }

    if (avatar)
    {
        if (!m_metaAvatarContainer->layout())
        {
            auto* lay = new QVBoxLayout(m_metaAvatarContainer);
            lay->setContentsMargins(0, 0, 0, 0);
        }
        avatar->setParent(m_metaAvatarContainer);
        m_metaAvatarContainer->layout()->addWidget(avatar);
        const int sz = 48;
        m_metaAvatarContainer->setFixedSize(sz, sz);
        m_metaAvatarContainer->show();
    }
    else
    {
        m_metaAvatarContainer->setFixedSize(0, 0);
        m_metaAvatarContainer->hide();
    }
    m_meta->setVisible(!m_metaTitleLabel->text().isEmpty() || !m_metaDescLabel->text().isEmpty() || avatar != nullptr);
    rebuildChrome();
    requestCardUpdate(rect(), QStringLiteral("meta"));
}

void AntCard::setMetaTitle(const QString& title)
{
    m_metaTitleLabel->setText(title);
    m_metaTitleLabel->setVisible(!title.isEmpty());
    updateTheme();
    m_meta->setVisible(!title.isEmpty() || !m_metaDescLabel->text().isEmpty());
    rebuildChrome();
    requestCardUpdate(rect(), QStringLiteral("meta"));
}

void AntCard::setMetaDescription(const QString& description)
{
    m_metaDescLabel->setText(description);
    m_metaDescLabel->setVisible(!description.isEmpty());
    m_meta->setVisible(!m_metaTitleLabel->text().isEmpty() || !description.isEmpty());
    rebuildChrome();
    requestCardUpdate(rect(), QStringLiteral("meta"));
}

void AntCard::addGridItem(QWidget* item)
{
    if (!m_gridLayout)
    {
        // Convert body layout from VBoxLayout to GridLayout
        delete m_bodyLayout;
        m_gridLayout = new QGridLayout(m_body);
        m_gridLayout->setSpacing(8);
        m_bodyLayout = nullptr;
    }
    const int count = m_gridLayout->count();
    const int row = count / 3;
    const int col = count % 3;
    m_gridLayout->addWidget(item, row, col);
    invalidateCardPaintCache();
    requestCardUpdate(rect(), QStringLiteral("grid"));
}

void AntCard::clearGridItems()
{
    if (m_gridLayout)
    {
        while (QLayoutItem* item = m_gridLayout->takeAt(0))
        {
            if (item->widget())
                item->widget()->deleteLater();
            delete item;
        }
    }
    invalidateCardPaintCache();
    requestCardUpdate(rect(), QStringLiteral("grid"));
}

void AntCard::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntCard::changeEvent(QEvent* event)
{
    QFrame::changeEvent(event);
    if (!m_rootLayout || !m_header || !m_body)
    {
        return;
    }
    if (event->type() == QEvent::EnabledChange)
    {
        updateLoadingTimer();
        requestCardUpdate(rect(), QStringLiteral("enabled"));
    }
    else if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange)
    {
        invalidateCardPaintCache();
        rebuildChrome();
        updateTheme();
        requestCardUpdate(rect(), QStringLiteral("style"));
    }
}

void AntCard::enterEvent(AntEnterEvent* event)
{
    if (!m_hovered)
    {
        m_hovered = true;
        invalidateCardPaintCache();
        requestCardUpdate(rect(), QStringLiteral("hover"));
    }
    QFrame::enterEvent(event);
}

void AntCard::leaveEvent(QEvent* event)
{
    if (m_hovered)
    {
        m_hovered = false;
        invalidateCardPaintCache();
        requestCardUpdate(rect(), QStringLiteral("hover"));
    }
    QFrame::leaveEvent(event);
}

void AntCard::showEvent(QShowEvent* event)
{
    updateLoadingTimer();
    QFrame::showEvent(event);
}

void AntCard::hideEvent(QHideEvent* event)
{
    m_spinnerTimer.stop();
    syncCardPerfCounters();
    QFrame::hideEvent(event);
}

void AntCard::rebuildChrome()
{
    const auto& token = antTheme->tokens();
    const bool small = m_cardSize == Ant::CardSize::Small;
    const int headerHeight = small ? 38 : 56;
    const int headerPadding = small ? token.paddingSM : token.paddingLG;
    const int bodyPadding = small ? token.paddingSM : token.paddingLG;

    const bool headerVisible = !m_title.isEmpty() || !m_extra.isEmpty();
    const bool extraVisible = !m_extra.isEmpty();
    const bool actionsVisible = m_actionsLayout->count() > 0;
    m_header->setVisible(headerVisible);
    m_extraLabel->setVisible(extraVisible);
    m_actions->setVisible(actionsVisible);
    if (m_header->minimumHeight() != headerHeight)
        m_header->setMinimumHeight(headerHeight);
    if (m_header->maximumHeight() != headerHeight)
        m_header->setMaximumHeight(headerHeight);

    if (auto* layout = qobject_cast<QHBoxLayout*>(m_header->layout()))
    {
        const QMargins target(headerPadding, 0, headerPadding, 0);
        if (layout->contentsMargins() != target)
            layout->setContentsMargins(target);
    }
    const QMargins bodyMargins(bodyPadding, bodyPadding, bodyPadding, bodyPadding);
    if (m_metaLayout->contentsMargins() != bodyMargins)
        m_metaLayout->setContentsMargins(bodyMargins);
    if (m_bodyLayout)
    {
        if (m_bodyLayout->contentsMargins() != bodyMargins)
            m_bodyLayout->setContentsMargins(bodyMargins);
    }
    else if (m_gridLayout)
    {
        if (m_gridLayout->contentsMargins() != bodyMargins)
            m_gridLayout->setContentsMargins(bodyMargins);
    }
    if (m_actions->minimumHeight() != 48)
        m_actions->setMinimumHeight(48);
    if (m_actions->maximumHeight() != 48)
        m_actions->setMaximumHeight(48);
    invalidateCardPaintCache();
    updateGeometry();
}

void AntCard::updateTheme()
{
    const auto& token = antTheme->tokens();
    const bool small = m_cardSize == Ant::CardSize::Small;
    QFont titleFont = m_titleLabel->font();
    titleFont.setPixelSize(small ? token.fontSize : token.fontSizeLG);
    titleFont.setWeight(QFont::DemiBold);
    m_titleLabel->setFont(titleFont);
    QPalette titlePal = m_titleLabel->palette();
    titlePal.setColor(QPalette::WindowText, token.colorText);
    m_titleLabel->setPalette(titlePal);

    QFont extraFont = m_extraLabel->font();
    extraFont.setPixelSize(token.fontSize);
    m_extraLabel->setFont(extraFont);
    QPalette extraPal = m_extraLabel->palette();
    extraPal.setColor(QPalette::WindowText, token.colorPrimary);
    m_extraLabel->setPalette(extraPal);

    // Meta labels
    QFont metaTitleFont = m_metaTitleLabel->font();
    metaTitleFont.setPixelSize(token.fontSizeLG);
    metaTitleFont.setWeight(QFont::DemiBold);
    m_metaTitleLabel->setFont(metaTitleFont);
    QPalette metaTitlePal = m_metaTitleLabel->palette();
    metaTitlePal.setColor(QPalette::WindowText, token.colorText);
    m_metaTitleLabel->setPalette(metaTitlePal);

    QFont metaDescFont = m_metaDescLabel->font();
    metaDescFont.setPixelSize(token.fontSize);
    m_metaDescLabel->setFont(metaDescFont);
    QPalette metaDescPal = m_metaDescLabel->palette();
    metaDescPal.setColor(QPalette::WindowText, token.colorTextSecondary);
    m_metaDescLabel->setPalette(metaDescPal);

    // Ensure container widgets don't paint their own backgrounds
    for (auto* w : {m_header, m_meta, m_body, m_actions})
    {
        if (w)
            w->setAutoFillBackground(false);
    }
}

void AntCard::updateLoadingTimer()
{
    const bool shouldRun = m_loading && isVisible() && isEnabled();
    if (shouldRun && !m_spinnerTimer.isActive())
    {
        m_spinnerTimer.start(80);
    }
    else if (!shouldRun && m_spinnerTimer.isActive())
    {
        m_spinnerTimer.stop();
    }
    syncCardPerfCounters();
}

const AntCard::CardPaintCache& AntCard::cardPaintCache(const QRect& widgetRect) const
{
    const int actionCount = m_actionsLayout ? m_actionsLayout->count() : 0;
    const bool headerVisible = m_header && m_header->isVisible();
    const bool actionsVisible = m_actions && m_actions->isVisible();

    if (m_paintCache.valid &&
        m_paintCache.widgetRect == widgetRect &&
        m_paintCache.headerVisible == headerVisible &&
        m_paintCache.actionsVisible == actionsVisible &&
        m_paintCache.actionCount == actionCount &&
        m_paintCache.headerSeparatorY == (headerVisible ? m_header->geometry().bottom() : -1) &&
        m_paintCache.actionsSeparatorY == (actionsVisible ? m_actions->geometry().top() : -1))
    {
        ++m_paintCacheHitCount;
        syncCardPerfCounters();
        return m_paintCache;
    }

    ++m_paintCacheBuildCount;
    CardPaintCache cache;
    cache.valid = true;
    cache.widgetRect = widgetRect;
    cache.cardRect = widgetRect;
    if (m_hoverable && m_hovered)
    {
        cache.cardRect.adjust(2, 2, -2, -2);
    }
    cache.headerVisible = headerVisible;
    cache.actionsVisible = actionsVisible;
    cache.actionCount = actionCount;
    cache.headerSeparatorY = headerVisible ? m_header->geometry().bottom() : -1;
    cache.actionsSeparatorY = actionsVisible ? m_actions->geometry().top() : -1;
    if (actionsVisible && actionCount > 1)
    {
        const QRect actionsRect = m_actions->geometry();
        cache.actionSeparatorXs.reserve(actionCount - 1);
        for (int i = 1; i < actionCount; ++i)
        {
            cache.actionSeparatorXs.append(actionsRect.left() + actionsRect.width() * i / actionCount);
        }
    }
    cache.spinnerRect = QRectF(widgetRect.width() / 2.0 - 14,
                               widgetRect.height() / 2.0 - 14,
                               28,
                               28);

    m_paintCache = cache;
    syncCardPerfCounters();
    return m_paintCache;
}

void AntCard::invalidateCardPaintCache() const
{
    m_paintCache.valid = false;
}

QRect AntCard::spinnerDirtyRect() const
{
    return cardPaintCache(rect()).spinnerRect.toAlignedRect().adjusted(-5, -5, 5, 5).intersected(rect());
}

void AntCard::requestCardUpdate(const QRect& region, const QString& mode, bool spinnerScoped)
{
    QRect dirty = region.isValid() && !region.isEmpty() ? region : rect();
    dirty = dirty.intersected(rect());
    if (dirty.isEmpty())
    {
        dirty = rect();
    }

    ++m_regionUpdateCount;
    if (spinnerScoped)
    {
        ++m_spinnerRegionUpdateCount;
    }
    m_lastUpdateMode = mode;
    syncCardPerfCounters();
    update(dirty);
}

void AntCard::syncCardPerfCounters() const
{
    auto* self = const_cast<AntCard*>(this);
    self->setProperty("antCardPaintCacheBuildCount", m_paintCacheBuildCount);
    self->setProperty("antCardPaintCacheHitCount", m_paintCacheHitCount);
    self->setProperty("antCardRegionUpdateCount", m_regionUpdateCount);
    self->setProperty("antCardSpinnerRegionUpdateCount", m_spinnerRegionUpdateCount);
    self->setProperty("antCardLastUpdateMode", m_lastUpdateMode);
    self->setProperty("antCardSpinnerTimerActive", m_spinnerTimer.isActive());
}

void AntCard::drawSpinner(QPainter& painter, const QRectF& rect) const
{
    painter.save();
    painter.setPen(QPen(antTheme->tokens().colorPrimary, 3, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(rect, m_spinnerAngle * 16, 280 * 16);
    painter.restore();
}
