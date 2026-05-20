#include "AntWidget.h"

#include <QApplication>
#include <QPalette>
#include <QStyle>

#include "core/AntTheme.h"

AntWidget::AntWidget(QWidget* parent)
    : QWidget(parent)
{
    refreshThemeCache();
    connect(antTheme, &AntTheme::themeModeChanged, this, &AntWidget::handleThemeChanged);
}

const AntThemeTokens& AntWidget::tokens() const
{
    return antTheme->tokens();
}

Ant::ThemeMode AntWidget::currentTheme() const
{
    return antTheme->themeMode();
}

void AntWidget::onThemeChanged(Ant::ThemeMode mode)
{
    Q_UNUSED(mode)
}

void AntWidget::handleThemeChanged(Ant::ThemeMode mode)
{
    ++m_themeChangeCount;

    const bool styleOrPaletteChanged = style() != m_cachedStyle || palette().cacheKey() != m_cachedPaletteKey;
    if (styleOrPaletteChanged)
    {
        style()->unpolish(this);
        style()->polish(this);
        ++m_repolishCount;
    }

    if (refreshCachedHints())
    {
        updateGeometry();
        ++m_updateGeometryCount;
    }

    update();
    ++m_surfaceUpdateCount;
    refreshThemeCache();
    syncPerfCounters();
    onThemeChanged(mode);
}

bool AntWidget::refreshCachedHints()
{
    const QSize currentSizeHint = sizeHint();
    const QSize currentMinimumSizeHint = minimumSizeHint();
    const bool changed = currentSizeHint != m_cachedSizeHint || currentMinimumSizeHint != m_cachedMinimumSizeHint;
    m_cachedSizeHint = currentSizeHint;
    m_cachedMinimumSizeHint = currentMinimumSizeHint;
    return changed;
}

void AntWidget::refreshThemeCache()
{
    m_cachedStyle = style();
    m_cachedPaletteKey = palette().cacheKey();
    refreshCachedHints();
    syncPerfCounters();
}

void AntWidget::syncPerfCounters() const
{
    auto* self = const_cast<AntWidget*>(this);
    self->setProperty("antWidgetThemeChangeCount", m_themeChangeCount);
    self->setProperty("antWidgetThemeRepolishCount", m_repolishCount);
    self->setProperty("antWidgetUpdateGeometryCount", m_updateGeometryCount);
    self->setProperty("antWidgetSurfaceUpdateCount", m_surfaceUpdateCount);
    self->setProperty("antWidgetCachedSizeHint", m_cachedSizeHint);
    self->setProperty("antWidgetCachedMinimumSizeHint", m_cachedMinimumSizeHint);
}
