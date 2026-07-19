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
    if (event->type() == QEvent::ActionRemoved && m_hoveredAction == event->action())
    {
        m_hoveredAction.clear();
    }
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
    // Match QMenuBar's touch compatibility path: a synthesized move without a
    // pressed left button is ignored so that the following synthesized press
    // does not immediately close a menu that the move just opened.
    if (!(event->buttons() & Qt::LeftButton) && event->source() != Qt::MouseEventNotSynthesized)
    {
        QMenuBar::mouseMoveEvent(event);
        return;
    }

    QPointer<AntMenuBar> self(this);
    QPointer<QAction> previousAction(m_hoveredAction);
    QPointer<QAction> nextAction(actionAt(event->pos()));
    QPointer<QAction> baseActionBefore(activeAction());
    m_hoveredAction = nextAction;

    // QMenuBar activates QAction::Hover from inside its mouse-move
    // implementation and continues to use the QAction afterwards.  Direct
    // connections to either QAction::hovered or QMenuBar::hovered are allowed
    // to delete that action, so defer the action activation until the base
    // class has finished processing the borrowed pointer.
    const bool actionSignalsWereBlocked = nextAction ? nextAction->signalsBlocked() : true;
    if (nextAction)
    {
        nextAction->blockSignals(true);
    }
    QMenuBar::mouseMoveEvent(event);
    if (nextAction)
    {
        nextAction->blockSignals(actionSignalsWereBlocked);
    }
    if (!self)
    {
        return;
    }
    if (nextAction && !actions().contains(nextAction.data()))
    {
        nextAction.clear();
    }
    const QPointer<QAction> baseActionAfter(activeAction());
    const bool shouldReplayHover = !actionSignalsWereBlocked
        && nextAction
        && nextAction->isEnabled()
        && baseActionAfter == nextAction
        && baseActionBefore != baseActionAfter;
    m_hoveredAction = nextAction;

    if (previousAction == nextAction)
    {
        m_lastHoverUpdateWasScoped = false;
        syncMenuBarPerfCounters();
        if (shouldReplayHover && nextAction && nextAction->isEnabled())
        {
            nextAction->hover();
        }
        return;
    }

    QRect dirty;
    if (previousAction)
    {
        dirty = dirty.united(cachedActionGeometry(previousAction.data()));
    }
    if (nextAction)
    {
        dirty = dirty.united(cachedActionGeometry(nextAction.data()));
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

    if (shouldReplayHover && nextAction && nextAction->isEnabled())
    {
        nextAction->hover();
    }
}

void AntMenuBar::leaveEvent(QEvent* event)
{
    QAction* previousAction = m_hoveredAction.data();
    m_hoveredAction.clear();
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
