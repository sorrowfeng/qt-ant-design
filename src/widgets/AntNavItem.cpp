#include "AntNavItem.h"

#include <QEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QResizeEvent>

#include "core/AntTheme.h"
#include "core/AntTypes.h"
#include "widgets/AntIcon.h"

AntNavItem::AntNavItem(const QString& text, QWidget* parent)
    : QWidget(parent)
{
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover);
    setFixedHeight(36);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(20, 0, 20, 0);
    m_layout->setSpacing(8);

    m_iconWidget = new AntIcon(this);
    m_iconWidget->setObjectName(QStringLiteral("AntNavItemAntIcon"));
    m_iconWidget->hide();
    m_layout->addWidget(m_iconWidget);

    m_imageLabel = new QLabel(this);
    m_imageLabel->setObjectName(QStringLiteral("AntNavItemImageLabel"));
    m_imageLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->hide();
    m_layout->addWidget(m_imageLabel);

    m_label = new QLabel(text, this);
    m_layout->addWidget(m_label);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidatePaintCache();
        syncVisualState();
        syncIconWidgets();
        update();
    });
    syncVisualState();
    syncIconWidgets();
    syncNavPerfCounters();
}

bool AntNavItem::isActive() const { return m_active; }

void AntNavItem::setActive(bool active)
{
    if (m_active == active) return;
    m_active = active;
    invalidatePaintCache();
    syncVisualState();
    syncIconWidgets();
    update();
    Q_EMIT activeChanged(m_active);
}

QString AntNavItem::text() const { return m_label->text(); }

void AntNavItem::setText(const QString& text)
{
    if (m_label->text() == text) return;
    m_label->setText(text);
    syncVisualState();
    updateGeometry();
    Q_EMIT textChanged(text);
}

void AntNavItem::setIcon(const QIcon& icon)
{
    m_icon = icon;
    m_iconSource = m_icon.isNull() ? IconSource::None : IconSource::QtIcon;
    m_iconType = Ant::IconType::None;
    m_iconName.clear();
    m_iconPixmap = QPixmap();
    syncIconWidgets();
    updateGeometry();
    Q_EMIT iconChanged();
}

QIcon AntNavItem::icon() const { return m_icon; }

void AntNavItem::setIcon(Ant::IconType iconType, Ant::IconTheme theme)
{
    if (m_iconSource == IconSource::AntIconType && m_iconType == iconType && m_iconTheme == theme)
    {
        return;
    }
    m_iconSource = iconType == Ant::IconType::None ? IconSource::None : IconSource::AntIconType;
    m_icon = QIcon();
    m_iconType = iconType;
    m_iconName.clear();
    m_iconPixmap = QPixmap();
    m_iconTheme = theme;
    syncIconWidgets();
    updateGeometry();
    Q_EMIT iconChanged();
}

Ant::IconType AntNavItem::iconType() const { return m_iconType; }

void AntNavItem::setIconName(const QString& iconName, Ant::IconTheme theme)
{
    if (m_iconSource == IconSource::AntIconName && m_iconName == iconName && m_iconTheme == theme)
    {
        return;
    }
    m_iconSource = iconName.isEmpty() ? IconSource::None : IconSource::AntIconName;
    m_icon = QIcon();
    m_iconName = iconName;
    m_iconType = Ant::IconType::None;
    m_iconPixmap = QPixmap();
    m_iconTheme = theme;
    syncIconWidgets();
    updateGeometry();
    Q_EMIT iconChanged();
}

QString AntNavItem::iconName() const { return m_iconName; }

void AntNavItem::setIconTheme(Ant::IconTheme theme)
{
    if (m_iconTheme == theme)
    {
        return;
    }
    m_iconTheme = theme;
    syncIconWidgets();
    Q_EMIT iconChanged();
}

Ant::IconTheme AntNavItem::iconTheme() const { return m_iconTheme; }

void AntNavItem::setIconColor(const QColor& color)
{
    if (m_iconColor == color)
    {
        return;
    }
    m_iconColor = color;
    syncIconWidgets();
    Q_EMIT iconChanged();
}

QColor AntNavItem::iconColor() const { return m_iconColor; }

void AntNavItem::setIconTwoToneColor(const QColor& color)
{
    if (m_iconTwoToneColor == color)
    {
        return;
    }
    m_iconTwoToneColor = color;
    syncIconWidgets();
    Q_EMIT iconChanged();
}

QColor AntNavItem::iconTwoToneColor() const { return m_iconTwoToneColor; }

void AntNavItem::setIconPixmap(const QPixmap& pixmap)
{
    if (m_iconSource == IconSource::Pixmap && m_iconPixmap.cacheKey() == pixmap.cacheKey())
    {
        return;
    }
    m_iconPixmap = pixmap;
    m_iconSource = m_iconPixmap.isNull() ? IconSource::None : IconSource::Pixmap;
    m_icon = QIcon();
    m_iconType = Ant::IconType::None;
    m_iconName.clear();
    syncIconWidgets();
    updateGeometry();
    Q_EMIT iconChanged();
}

QPixmap AntNavItem::iconPixmap() const { return m_iconPixmap; }

void AntNavItem::setIconImage(const QImage& image)
{
    setIconPixmap(image.isNull() ? QPixmap() : QPixmap::fromImage(image));
}

QImage AntNavItem::iconImage() const
{
    return m_iconPixmap.isNull() ? QImage() : m_iconPixmap.toImage();
}

void AntNavItem::setIconSize(const QSize& size)
{
    const QSize normalized(qMax(0, size.width()), qMax(0, size.height()));
    if (m_iconSize == normalized)
    {
        return;
    }
    m_iconSize = normalized;
    syncIconWidgets();
    updateGeometry();
    Q_EMIT iconSizeChanged(m_iconSize);
    Q_EMIT iconChanged();
}

QSize AntNavItem::iconSize() const { return m_iconSize; }

bool AntNavItem::hasIcon() const
{
    switch (m_iconSource)
    {
    case IconSource::QtIcon:
        return !m_icon.isNull();
    case IconSource::AntIconType:
        return m_iconType != Ant::IconType::None;
    case IconSource::AntIconName:
        return !m_iconName.isEmpty();
    case IconSource::Pixmap:
        return !m_iconPixmap.isNull();
    case IconSource::None:
    default:
        return false;
    }
}

void AntNavItem::clearIcon()
{
    if (!hasIcon() && m_iconSource == IconSource::None)
    {
        return;
    }
    m_iconSource = IconSource::None;
    m_icon = QIcon();
    m_iconType = Ant::IconType::None;
    m_iconName.clear();
    m_iconPixmap = QPixmap();
    syncIconWidgets();
    updateGeometry();
    Q_EMIT iconChanged();
}

void AntNavItem::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    ensurePaintCache();

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.fillRect(rect(), m_cachedBackground);

    if (!m_cachedIndicatorRect.isEmpty())
    {
        p.fillRect(m_cachedIndicatorRect, m_cachedIndicatorColor);
    }
}

void AntNavItem::enterEvent(AntEnterEvent* event)
{
    if (!m_hovered)
    {
        m_hovered = true;
        invalidatePaintCache();
        update();
    }
    QWidget::enterEvent(event);
}

void AntNavItem::leaveEvent(QEvent* event)
{
    if (m_hovered)
    {
        m_hovered = false;
        invalidatePaintCache();
        update();
    }
    QWidget::leaveEvent(event);
}

void AntNavItem::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        Q_EMIT clicked();
    }
    QWidget::mousePressEvent(event);
}

void AntNavItem::resizeEvent(QResizeEvent* event)
{
    invalidatePaintCache();
    syncIconWidgets();
    QWidget::resizeEvent(event);
}

void AntNavItem::syncVisualState()
{
    if (!m_label)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const QColor textColor = m_active ? token.colorPrimary : token.colorText;
    const int fontWeight = m_active ? QFont::DemiBold : QFont::Normal;
    if (m_cachedLabelColor == textColor && m_cachedLabelWeight == fontWeight)
    {
        syncNavPerfCounters();
        return;
    }

    QPalette pal = m_label->palette();
    pal.setColor(QPalette::WindowText, textColor);
    m_label->setPalette(pal);

    QFont f = m_label->font();
    f.setWeight(static_cast<QFont::Weight>(fontWeight));
    m_label->setFont(f);

    m_cachedLabelColor = textColor;
    m_cachedLabelWeight = fontWeight;
    ++m_visualStateApplyCount;
    syncNavPerfCounters();
}

void AntNavItem::syncIconWidgets()
{
    if (!m_iconWidget || !m_imageLabel)
    {
        return;
    }

    const QSize iconSize = effectiveIconSize();
    m_iconWidget->setFixedSize(iconSize);
    m_imageLabel->setFixedSize(iconSize);

    if (usesAntIconWidget())
    {
        m_imageLabel->hide();
        if (m_iconSource == IconSource::AntIconType)
        {
            m_iconWidget->setIconType(m_iconType);
        }
        else
        {
            m_iconWidget->setIconName(m_iconName);
        }
        m_iconWidget->setIconTheme(m_iconTheme);
        m_iconWidget->setColor(resolvedIconColor());
        m_iconWidget->setTwoToneColor(m_iconTwoToneColor);
        m_iconWidget->setIconSize(qMax(1, qMin(iconSize.width(), iconSize.height())));
        m_iconWidget->show();
        return;
    }

    m_iconWidget->hide();
    if (usesImageLabel())
    {
        m_imageLabel->setPixmap(scaledPaintedIconPixmap());
        m_imageLabel->show();
    }
    else
    {
        m_imageLabel->clear();
        m_imageLabel->hide();
    }
}

bool AntNavItem::usesAntIconWidget() const
{
    return m_iconSource == IconSource::AntIconType || m_iconSource == IconSource::AntIconName;
}

bool AntNavItem::usesImageLabel() const
{
    return m_iconSource == IconSource::QtIcon || m_iconSource == IconSource::Pixmap;
}

QSize AntNavItem::effectiveIconSize() const
{
    if (m_iconSize.isValid() && !m_iconSize.isEmpty())
    {
        return m_iconSize;
    }
    return QSize(16, 16);
}

QPixmap AntNavItem::scaledPaintedIconPixmap() const
{
    const QSize iconSize = effectiveIconSize();
    if (m_iconSource == IconSource::QtIcon)
    {
        return m_icon.pixmap(iconSize);
    }

    if (m_iconPixmap.isNull())
    {
        return {};
    }

    const qreal dpr = devicePixelRatioF();
    const QSize pixelSize(qMax(1, qRound(iconSize.width() * dpr)),
                          qMax(1, qRound(iconSize.height() * dpr)));
    QPixmap scaled = m_iconPixmap.scaled(pixelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(dpr);
    return scaled;
}

QColor AntNavItem::resolvedIconColor() const
{
    if (m_iconColor.isValid())
    {
        return m_iconColor;
    }
    const auto& token = antTheme->tokens();
    return m_active ? token.colorPrimary : token.colorText;
}

void AntNavItem::invalidatePaintCache() const
{
    m_paintCacheValid = false;
    syncNavPerfCounters();
}

void AntNavItem::ensurePaintCache() const
{
    if (m_paintCacheValid &&
        m_cachedPaintSize == size() &&
        m_cachedPaintActive == m_active &&
        m_cachedPaintHovered == m_hovered)
    {
        syncNavPerfCounters();
        return;
    }

    const auto& token = antTheme->tokens();
    const bool isDark = antTheme->themeMode() == Ant::ThemeMode::Dark;
    QColor bg(Qt::transparent);
    if (m_active)
    {
        bg = isDark ? QColor("#111d35") : QColor("#e6f4ff");
    }
    else if (m_hovered)
    {
        bg = isDark ? QColor("#1f1f1f") : QColor("#f5f5f5");
    }

    m_cachedPaintSize = size();
    m_cachedPaintActive = m_active;
    m_cachedPaintHovered = m_hovered;
    m_cachedBackground = bg;
    m_cachedIndicatorColor = token.colorPrimary;
    m_cachedIndicatorRect = m_active ? QRect(0, 0, 3, height()) : QRect();
    m_paintCacheValid = true;
    ++m_paintCacheBuildCount;
    syncNavPerfCounters();
}

void AntNavItem::syncNavPerfCounters() const
{
    auto* self = const_cast<AntNavItem*>(this);
    self->setProperty("antNavItemPaintCacheBuildCount", m_paintCacheBuildCount);
    self->setProperty("antNavItemPaintCacheValid", m_paintCacheValid);
    self->setProperty("antNavItemVisualStateApplyCount", m_visualStateApplyCount);
}
