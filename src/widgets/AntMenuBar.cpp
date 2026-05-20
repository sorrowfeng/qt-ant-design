#include "AntMenuBar.h"

#include <QActionEvent>
#include <QEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QResizeEvent>

#include "../styles/AntMenuBarStyle.h"
#include "core/AntTheme.h"

AntMenuBar::AntMenuBar(QWidget* parent)
    : QMenuBar(parent)
{
    auto* s = new AntMenuBarStyle(style());
    s->setParent(this);
    setStyle(s);
    setMouseTracking(true);
    syncMenuBarPerfCounters();

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateActionGeometryCache();
        update();
    });
}

QMenu* AntMenuBar::addMenu(const QString& title)
{
    auto* menu = new QMenu(title, this);
    QMenuBar::addMenu(menu);
    return menu;
}

void AntMenuBar::actionEvent(QActionEvent* event)
{
    invalidateActionGeometryCache();
    QMenuBar::actionEvent(event);
}

void AntMenuBar::changeEvent(QEvent* event)
{
    QMenuBar::changeEvent(event);
    if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange ||
        event->type() == QEvent::LayoutDirectionChange)
    {
        invalidateActionGeometryCache();
    }
}

void AntMenuBar::mouseMoveEvent(QMouseEvent* event)
{
    QAction* previousAction = m_hoveredAction;
    QAction* nextAction = actionAt(event->pos());
    QMenuBar::mouseMoveEvent(event);

    if (previousAction == nextAction)
    {
        m_lastHoverUpdateWasScoped = false;
        syncMenuBarPerfCounters();
        return;
    }

    m_hoveredAction = nextAction;
    QRect dirty;
    if (previousAction)
    {
        dirty = dirty.united(cachedActionGeometry(previousAction));
    }
    if (nextAction)
    {
        dirty = dirty.united(cachedActionGeometry(nextAction));
    }
    if (!dirty.isEmpty())
    {
        m_lastHoverUpdateWasScoped = true;
        update(dirty.adjusted(-2, -2, 2, 2));
    }
    else
    {
        m_lastHoverUpdateWasScoped = false;
    }
    syncMenuBarPerfCounters();
}

void AntMenuBar::leaveEvent(QEvent* event)
{
    QAction* previousAction = m_hoveredAction;
    m_hoveredAction = nullptr;
    m_lastHoverUpdateWasScoped = false;
    if (previousAction)
    {
        const QRect dirty = cachedActionGeometry(previousAction);
        if (!dirty.isEmpty())
        {
            m_lastHoverUpdateWasScoped = true;
            update(dirty.adjusted(-2, -2, 2, 2));
        }
    }
    syncMenuBarPerfCounters();
    QMenuBar::leaveEvent(event);
}

void AntMenuBar::resizeEvent(QResizeEvent* event)
{
    invalidateActionGeometryCache();
    QMenuBar::resizeEvent(event);
}

QRect AntMenuBar::cachedActionGeometry(QAction* action) const
{
    if (!action)
    {
        return {};
    }

    const auto it = m_actionGeometryCache.constFind(action);
    if (it != m_actionGeometryCache.constEnd())
    {
        return it.value();
    }

    const QRect geometry = actionGeometry(action);
    m_actionGeometryCache.insert(action, geometry);
    ++m_actionGeometryCacheBuildCount;
    syncMenuBarPerfCounters();
    return geometry;
}

void AntMenuBar::invalidateActionGeometryCache() const
{
    if (!m_actionGeometryCache.isEmpty())
    {
        m_actionGeometryCache.clear();
    }
    syncMenuBarPerfCounters();
}

void AntMenuBar::syncMenuBarPerfCounters() const
{
    auto* self = const_cast<AntMenuBar*>(this);
    self->setProperty("antMenuBarActionGeometryCacheBuildCount", m_actionGeometryCacheBuildCount);
    self->setProperty("antMenuBarActionGeometryCacheSize", m_actionGeometryCache.size());
    self->setProperty("antMenuBarScopedHoverUpdate", m_lastHoverUpdateWasScoped);
}

QMenu* AntMenuBar::addMenu(const QIcon& icon, const QString& title)
{
    auto* menu = new QMenu(title, this);
    menu->setIcon(icon);
    QMenuBar::addMenu(menu);
    return menu;
}
