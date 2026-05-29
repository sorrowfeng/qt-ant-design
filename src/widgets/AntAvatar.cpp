#include "AntAvatar.h"

#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QtMath>

#include <algorithm>

#include "core/AntTheme.h"
#include "core/AntThemeRefresh_p.h"
#include "styles/AntAvatarStyle.h"

AntAvatar::AntAvatar(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntAvatarStyle>(this);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateImagePixmapCache();
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
        requestAvatarUpdate(QStringLiteral("theme"));
    });
    syncAvatarPerfCounters();
}

AntAvatar::AntAvatar(const QString& text, QWidget* parent)
    : AntAvatar(parent)
{
    m_text = text;
}

QString AntAvatar::text() const { return m_text; }

void AntAvatar::setText(const QString& text)
{
    if (m_text == text)
    {
        return;
    }
    m_text = text;
    requestAvatarUpdate(QStringLiteral("text"));
    Q_EMIT textChanged(m_text);
}

QString AntAvatar::iconText() const { return m_iconText; }

void AntAvatar::setIconText(const QString& iconText)
{
    if (m_iconText == iconText)
    {
        return;
    }
    m_iconText = iconText;
    requestAvatarUpdate(QStringLiteral("icon"));
    Q_EMIT iconTextChanged(m_iconText);
}

QString AntAvatar::imagePath() const { return m_imagePath; }

void AntAvatar::setImagePath(const QString& imagePath)
{
    if (m_imagePath == imagePath)
    {
        return;
    }
    m_imagePath = imagePath;
    m_pixmap = QPixmap(m_imagePath);
    invalidateImagePixmapCache();
    requestAvatarUpdate(QStringLiteral("image"));
    Q_EMIT imagePathChanged(m_imagePath);
}

QColor AntAvatar::backgroundColor() const { return m_backgroundColor; }

void AntAvatar::setBackgroundColor(const QColor& color)
{
    if (m_backgroundColor == color)
    {
        return;
    }
    m_backgroundColor = color;
    requestAvatarUpdate(QStringLiteral("background"));
    Q_EMIT backgroundColorChanged(m_backgroundColor);
}

int AntAvatar::gap() const { return m_gap; }

void AntAvatar::setGap(int gap)
{
    gap = std::max(0, gap);
    if (m_gap == gap)
    {
        return;
    }
    m_gap = gap;
    requestAvatarUpdate(QStringLiteral("gap"));
    Q_EMIT gapChanged(m_gap);
}

int AntAvatar::customSize() const { return m_customSize; }

void AntAvatar::setCustomSize(int size)
{
    size = std::max(0, size);
    if (m_customSize == size)
    {
        return;
    }
    m_customSize = size;
    invalidateImagePixmapCache();
    updateGeometry();
    requestAvatarUpdate(QStringLiteral("size"));
    Q_EMIT customSizeChanged(m_customSize);
}

Ant::AvatarShape AntAvatar::shape() const { return m_shape; }

void AntAvatar::setShape(Ant::AvatarShape shape)
{
    if (m_shape == shape)
    {
        return;
    }
    m_shape = shape;
    invalidateImagePixmapCache();
    requestAvatarUpdate(QStringLiteral("shape"));
    Q_EMIT shapeChanged(m_shape);
}

Ant::Size AntAvatar::avatarSize() const { return m_avatarSize; }

void AntAvatar::setAvatarSize(Ant::Size size)
{
    if (m_avatarSize == size)
    {
        return;
    }
    m_avatarSize = size;
    invalidateImagePixmapCache();
    updateGeometry();
    requestAvatarUpdate(QStringLiteral("avatarSize"));
    Q_EMIT avatarSizeChanged(m_avatarSize);
}

QSize AntAvatar::sizeHint() const
{
    const int extent = avatarExtent();
    return QSize(extent, extent);
}

QSize AntAvatar::minimumSizeHint() const
{
    return QSize(antTheme->tokens().controlHeightSM, antTheme->tokens().controlHeightSM);
}

void AntAvatar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

int AntAvatar::avatarExtent() const
{
    if (m_customSize > 0)
    {
        return m_customSize;
    }
    const auto& token = antTheme->tokens();
    switch (m_avatarSize)
    {
    case Ant::Size::Large:
        return token.controlHeightLG;
    case Ant::Size::Small:
        return token.controlHeightSM;
    case Ant::Size::Middle:
        return token.controlHeight;
    }
    return token.controlHeight;
}

int AntAvatar::textFontSize() const
{
    const auto& token = antTheme->tokens();
    if (m_avatarSize == Ant::Size::Small)
    {
        return token.fontSizeSM;
    }
    return token.fontSize;
}

int AntAvatar::iconFontSize() const
{
    const auto& token = antTheme->tokens();
    if (m_avatarSize == Ant::Size::Large || m_customSize >= token.controlHeightLG)
    {
        return token.fontSizeXL;
    }
    if (m_avatarSize == Ant::Size::Small && m_customSize == 0)
    {
        return token.fontSize;
    }
    return (token.fontSizeLG + token.fontSizeXL) / 2;
}

QPainterPath AntAvatar::clipPath(const QRectF& rect) const
{
    QPainterPath path;
    if (m_shape == Ant::AvatarShape::Circle)
    {
        path.addEllipse(rect);
    }
    else
    {
        path.addRoundedRect(rect, antTheme->tokens().borderRadius, antTheme->tokens().borderRadius);
    }
    return path;
}

QRectF AntAvatar::imageSourceRect(const QPixmap& pixmap, const QSizeF& targetSize) const
{
    const QSizeF source = pixmap.size();
    const qreal sourceRatio = source.width() / source.height();
    const qreal targetRatio = targetSize.width() / targetSize.height();
    if (sourceRatio > targetRatio)
    {
        const qreal width = source.height() * targetRatio;
        return QRectF((source.width() - width) / 2.0, 0, width, source.height());
    }
    const qreal height = source.width() / targetRatio;
    return QRectF(0, (source.height() - height) / 2.0, source.width(), height);
}

QPixmap AntAvatar::cachedImagePixmap(qreal devicePixelRatio, const QSizeF& targetSize) const
{
    if (m_pixmap.isNull() || targetSize.isEmpty())
    {
        return {};
    }

    const qreal dpr = qMax<qreal>(1.0, devicePixelRatio);
    const QSize logicalSize(qMax(1, qCeil(targetSize.width())), qMax(1, qCeil(targetSize.height())));
    const int borderRadius = antTheme->tokens().borderRadius;
    const qint64 sourceCacheKey = m_pixmap.cacheKey();
    const QSize sourceSize = m_pixmap.size();

    if (m_imagePixmapCache.valid &&
        qFuzzyCompare(m_imagePixmapCache.devicePixelRatio, dpr) &&
        m_imagePixmapCache.logicalSize == logicalSize &&
        m_imagePixmapCache.shape == m_shape &&
        m_imagePixmapCache.borderRadius == borderRadius &&
        m_imagePixmapCache.sourceCacheKey == sourceCacheKey &&
        m_imagePixmapCache.sourceSize == sourceSize)
    {
        ++m_imagePixmapCacheHitCount;
        syncAvatarPerfCounters();
        return m_imagePixmapCache.pixmap;
    }

    ++m_imagePixmapBuildCount;
    AvatarImagePixmapCache cache;
    cache.valid = true;
    cache.devicePixelRatio = dpr;
    cache.logicalSize = logicalSize;
    cache.shape = m_shape;
    cache.borderRadius = borderRadius;
    cache.sourceCacheKey = sourceCacheKey;
    cache.sourceSize = sourceSize;
    cache.pixmap = QPixmap(QSize(qCeil(logicalSize.width() * dpr), qCeil(logicalSize.height() * dpr)));
    cache.pixmap.setDevicePixelRatio(dpr);
    cache.pixmap.fill(Qt::transparent);

    QPainter painter(&cache.pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    const QRectF targetRect(QPointF(0, 0), QSizeF(logicalSize));
    painter.setClipPath(clipPath(targetRect));
    painter.drawPixmap(targetRect, m_pixmap, imageSourceRect(m_pixmap, targetRect.size()));

    m_imagePixmapCache = cache;
    syncAvatarPerfCounters();
    return m_imagePixmapCache.pixmap;
}

void AntAvatar::invalidateImagePixmapCache() const
{
    m_imagePixmapCache.valid = false;
}

void AntAvatar::requestAvatarUpdate(const QString& mode)
{
    m_lastUpdateMode = mode;
    ++m_regionUpdateCount;
    syncAvatarPerfCounters();
    update();
}

void AntAvatar::syncAvatarPerfCounters() const
{
    auto* self = const_cast<AntAvatar*>(this);
    self->setProperty("antAvatarImagePixmapBuildCount", m_imagePixmapBuildCount);
    self->setProperty("antAvatarImagePixmapCacheHitCount", m_imagePixmapCacheHitCount);
    self->setProperty("antAvatarRegionUpdateCount", m_regionUpdateCount);
    self->setProperty("antAvatarLastUpdateMode", m_lastUpdateMode);
}

// ── AntAvatarGroup ──

AntAvatarGroup::AntAvatarGroup(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

int AntAvatarGroup::maxCount() const { return m_maxCount; }

void AntAvatarGroup::setMaxCount(int maxCount)
{
    maxCount = qMax(0, maxCount);
    if (m_maxCount == maxCount)
        return;
    m_maxCount = maxCount;
    relayout();
    Q_EMIT maxCountChanged(m_maxCount);
}

Ant::Size AntAvatarGroup::avatarSize() const { return m_avatarSize; }

void AntAvatarGroup::setAvatarSize(Ant::Size size)
{
    if (m_avatarSize == size)
        return;
    m_avatarSize = size;
    for (auto* av : m_avatars)
        av->setAvatarSize(size);
    relayout();
    Q_EMIT avatarSizeChanged(m_avatarSize);
}

void AntAvatarGroup::addAvatar(AntAvatar* avatar)
{
    if (!avatar || m_avatars.contains(avatar))
        return;
    avatar->setParent(this);
    avatar->setAvatarSize(m_avatarSize);
    m_avatars.append(avatar);
    relayout();
}

void AntAvatarGroup::removeAvatar(AntAvatar* avatar)
{
    if (m_avatars.removeOne(avatar))
    {
        avatar->setParent(nullptr);
        relayout();
    }
}

QList<AntAvatar*> AntAvatarGroup::avatars() const { return m_avatars; }

QSize AntAvatarGroup::sizeHint() const
{
    if (m_avatars.isEmpty())
        return QSize(0, 0);
    const int sz = m_avatars.first()->sizeHint().width();
    const int visible = (m_maxCount > 0 && m_avatars.size() > m_maxCount)
                            ? m_maxCount + 1 // +1 for overflow
                            : m_avatars.size();
    const int overlap = sz * 2 / 5;
    const int totalW = sz + (visible - 1) * (sz - overlap);
    return QSize(totalW, sz);
}

QSize AntAvatarGroup::minimumSizeHint() const
{
    return QSize(24, 24);
}

void AntAvatarGroup::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntAvatarGroup::resizeEvent(QResizeEvent* event)
{
    relayout();
    QWidget::resizeEvent(event);
}

void AntAvatarGroup::relayout()
{
    const int total = m_avatars.size();
    if (total == 0)
    {
        if (m_overflowAvatar)
        {
            m_overflowAvatar->hide();
        }
        return;
    }

    const int sz = m_avatars.first()->sizeHint().width();
    const int overlap = sz * 2 / 5;
    const int step = sz - overlap;

    int visibleCount = total;
    bool showOverflow = false;
    if (m_maxCount > 0 && total > m_maxCount)
    {
        visibleCount = m_maxCount;
        showOverflow = true;
    }

    // Hide overflow avatars
    for (int i = visibleCount; i < total; ++i)
        m_avatars[i]->hide();

    // Position visible avatars (left to right, later on top)
    for (int i = 0; i < visibleCount; ++i)
    {
        m_avatars[i]->setFixedSize(sz, sz);
        m_avatars[i]->move(i * step, 0);
        m_avatars[i]->show();
        m_avatars[i]->raise();
    }

    if (showOverflow)
    {
        if (!m_overflowAvatar)
        {
            m_overflowAvatar = new AntAvatar(this);
        }
        m_overflowAvatar->setAvatarSize(m_avatarSize);
        m_overflowAvatar->setText(QStringLiteral("+%1").arg(total - visibleCount));
        m_overflowAvatar->setFixedSize(sz, sz);
        m_overflowAvatar->move(visibleCount * step, 0);
        m_overflowAvatar->show();
        m_overflowAvatar->raise();
    }
    else if (m_overflowAvatar)
    {
        m_overflowAvatar->hide();
    }

    updateGeometry();
    update();
}
