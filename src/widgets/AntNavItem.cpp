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

AntNavItem::AntNavItem(const QString& text, QWidget* parent)
    : QWidget(parent)
{
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover);
    setFixedHeight(36);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 0, 20, 0);
    layout->setSpacing(0);

    m_label = new QLabel(text, this);
    layout->addWidget(m_label);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidatePaintCache();
        syncVisualState();
        update();
    });
    syncVisualState();
    syncNavPerfCounters();
}

bool AntNavItem::isActive() const { return m_active; }

void AntNavItem::setActive(bool active)
{
    if (m_active == active) return;
    m_active = active;
    invalidatePaintCache();
    syncVisualState();
    update();
    Q_EMIT activeChanged(m_active);
}

QString AntNavItem::text() const { return m_label->text(); }

void AntNavItem::setText(const QString& text)
{
    if (m_label->text() == text) return;
    m_label->setText(text);
    syncVisualState();
    Q_EMIT textChanged(text);
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

void AntNavItem::enterEvent(QEnterEvent* event)
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
