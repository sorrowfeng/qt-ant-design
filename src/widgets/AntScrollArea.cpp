#include "AntScrollArea.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QAbstractSlider>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPalette>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QScroller>
#include <QTabBar>
#include <QTextEdit>

#include "AntScrollBar.h"
#include "core/AntTheme.h"

namespace
{

void applySurfaceColors(QPalette& palette, const QColor& bg, const QColor& text)
{
    palette.setColor(QPalette::Window, bg);
    palette.setColor(QPalette::Base, bg);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::WindowText, text);
}

bool surfaceColorsMatch(const QPalette& palette, const QColor& bg, const QColor& text)
{
    return palette.color(QPalette::Window) == bg &&
           palette.color(QPalette::Base) == bg &&
           palette.color(QPalette::Text) == text &&
           palette.color(QPalette::WindowText) == text;
}

} // namespace

AntScrollArea::AntScrollArea(QWidget* parent)
    : QScrollArea(parent)
{
    setFrameShape(QFrame::NoFrame);
    setWidgetResizable(true);

    m_vScrollBar = new AntScrollBar(Qt::Vertical, this);
    m_vScrollBar->setAutoHide(m_autoHideScrollBar);
    setVerticalScrollBar(m_vScrollBar);

    m_hScrollBar = new AntScrollBar(Qt::Horizontal, this);
    m_hScrollBar->setAutoHide(m_autoHideScrollBar);
    setHorizontalScrollBar(m_hScrollBar);

    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    updateTheme();
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        if (updateTheme())
        {
            update();
            if (viewport())
            {
                viewport()->update();
                ++m_viewportUpdateCount;
                syncScrollAreaPerfCounters();
            }
        }
    });

    updateGestureGrab();
    updateMouseDragTargets();
}

void AntScrollArea::setWidget(QWidget* widget)
{
    clearMouseDragTargets();
    resetMouseDragScrollState();
    QScrollArea::setWidget(widget);
    updateGestureGrab();
    updateMouseDragTargets();
    updateTheme();
}

bool AntScrollArea::autoHideScrollBar() const { return m_autoHideScrollBar; }

void AntScrollArea::setAutoHideScrollBar(bool autoHide)
{
    if (m_autoHideScrollBar == autoHide) return;
    m_autoHideScrollBar = autoHide;
    if (m_vScrollBar) m_vScrollBar->setAutoHide(autoHide);
    if (m_hScrollBar) m_hScrollBar->setAutoHide(autoHide);
    Q_EMIT autoHideScrollBarChanged(m_autoHideScrollBar);
}

bool AntScrollArea::isGestureEnabled() const { return m_enableGesture; }

void AntScrollArea::setEnableGesture(bool enable)
{
    if (m_enableGesture == enable) return;
    m_enableGesture = enable;
    updateGestureGrab();
    Q_EMIT enableGestureChanged(m_enableGesture);
}

bool AntScrollArea::isMouseDragScrollEnabled() const { return m_mouseDragScrollEnabled; }

void AntScrollArea::setMouseDragScrollEnabled(bool enabled)
{
    if (m_mouseDragScrollEnabled == enabled) return;
    m_mouseDragScrollEnabled = enabled;
    resetMouseDragScrollState();
    updateMouseDragTargets();
    Q_EMIT mouseDragScrollEnabledChanged(m_mouseDragScrollEnabled);
}

bool AntScrollArea::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::ChildAdded || event->type() == QEvent::ChildRemoved)
    {
        if (watched == viewport() || watched == widget())
        {
            updateMouseDragTargets();
        }
    }

    if (handleMouseDragScrollEvent(watched, event))
    {
        return true;
    }

    return QScrollArea::eventFilter(watched, event);
}

void AntScrollArea::updateGestureGrab()
{
#ifndef QT_NO_GESTURES
    QScroller::ungrabGesture(this);
    QWidget* target = viewport();
    if (!target)
    {
        return;
    }

    QScroller::ungrabGesture(target);
    if (m_vScrollBar)
    {
        QScroller::ungrabGesture(m_vScrollBar);
    }
    if (m_hScrollBar)
    {
        QScroller::ungrabGesture(m_hScrollBar);
    }
    if (!m_enableGesture)
    {
        return;
    }

    QScroller::grabGesture(target, QScroller::TouchGesture);
    QScrollerProperties sp = QScroller::scroller(target)->scrollerProperties();
    sp.setScrollMetric(QScrollerProperties::MousePressEventDelay, 0.5);
    sp.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.35);
    sp.setScrollMetric(QScrollerProperties::OvershootScrollTime, 0.5);
    sp.setScrollMetric(QScrollerProperties::FrameRate, QScrollerProperties::Fps60);
    QScroller::scroller(target)->setScrollerProperties(sp);
#endif
}

void AntScrollArea::updateMouseDragTargets()
{
    clearMouseDragTargets();
    if (!m_mouseDragScrollEnabled)
    {
        return;
    }

    installMouseDragTarget(viewport());

    QWidget* content = widget();
    if (!content)
    {
        return;
    }

    installMouseDragTarget(content);
    const auto descendants = content->findChildren<QWidget*>();
    for (QWidget* child : descendants)
    {
        installMouseDragTarget(child);
    }
}

void AntScrollArea::clearMouseDragTargets()
{
    for (const QPointer<QObject>& object : m_mouseDragFilteredObjects)
    {
        if (object)
        {
            object->removeEventFilter(this);
        }
    }
    m_mouseDragFilteredObjects.clear();
}

void AntScrollArea::installMouseDragTarget(QWidget* target)
{
    if (!isMouseDragTargetCandidate(target))
    {
        return;
    }

    for (const QPointer<QObject>& object : m_mouseDragFilteredObjects)
    {
        if (object == target)
        {
            return;
        }
    }

    target->installEventFilter(this);
    m_mouseDragFilteredObjects.append(target);
}

bool AntScrollArea::isMouseDragTargetCandidate(QWidget* target) const
{
    if (!target || target == m_vScrollBar || target == m_hScrollBar)
    {
        return false;
    }
    if (qobject_cast<QScrollBar*>(target))
    {
        return false;
    }
    if (target == viewport())
    {
        return true;
    }
    if (qobject_cast<QAbstractScrollArea*>(target) ||
        qobject_cast<QAbstractButton*>(target) ||
        qobject_cast<QAbstractItemView*>(target) ||
        qobject_cast<QAbstractSlider*>(target) ||
        qobject_cast<QAbstractSpinBox*>(target) ||
        qobject_cast<QComboBox*>(target) ||
        qobject_cast<QLineEdit*>(target) ||
        qobject_cast<QPlainTextEdit*>(target) ||
        qobject_cast<QTabBar*>(target) ||
        qobject_cast<QTextEdit*>(target))
    {
        return false;
    }
    if (target == widget())
    {
        return true;
    }
    if (target->focusPolicy() != Qt::NoFocus)
    {
        return false;
    }
    return true;
}

bool AntScrollArea::handleMouseDragScrollEvent(QObject* watched, QEvent* event)
{
    if (!m_mouseDragScrollEnabled)
    {
        return false;
    }

    auto* target = qobject_cast<QWidget*>(watched);
    if (!target)
    {
        return false;
    }
    if (!isMouseDragTargetCandidate(target))
    {
        return false;
    }

    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() != Qt::LeftButton || !hasMouseDragScrollableRange())
        {
            return false;
        }

        m_mouseDragPressed = true;
        m_mouseDragActive = false;
        m_mouseDragStartPos = mouseDragViewportPos(watched, mouseEvent);
        m_mouseDragStartHValue = horizontalScrollBar() ? horizontalScrollBar()->value() : 0;
        m_mouseDragStartVValue = verticalScrollBar() ? verticalScrollBar()->value() : 0;
        return false;
    }

    case QEvent::MouseMove:
    {
        if (!m_mouseDragPressed)
        {
            return false;
        }

        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (!mouseEvent->buttons().testFlag(Qt::LeftButton))
        {
            resetMouseDragScrollState();
            return false;
        }

        const QPoint pos = mouseDragViewportPos(watched, mouseEvent);
        const QPoint delta = pos - m_mouseDragStartPos;
        if (!m_mouseDragActive && delta.manhattanLength() < QApplication::startDragDistance())
        {
            return false;
        }

        m_mouseDragActive = true;
        if (horizontalScrollBar())
        {
            horizontalScrollBar()->setValue(m_mouseDragStartHValue - delta.x());
        }
        if (verticalScrollBar())
        {
            verticalScrollBar()->setValue(m_mouseDragStartVValue - delta.y());
        }
        mouseEvent->accept();
        return true;
    }

    case QEvent::MouseButtonRelease:
    {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() != Qt::LeftButton || !m_mouseDragPressed)
        {
            return false;
        }

        const bool wasActive = m_mouseDragActive;
        resetMouseDragScrollState();
        if (wasActive)
        {
            mouseEvent->accept();
        }
        return wasActive;
    }

    case QEvent::Leave:
    case QEvent::Hide:
        if (m_mouseDragPressed && !QApplication::mouseButtons().testFlag(Qt::LeftButton))
        {
            resetMouseDragScrollState();
        }
        return false;

    default:
        break;
    }

    return false;
}

bool AntScrollArea::hasMouseDragScrollableRange() const
{
    const QScrollBar* hBar = horizontalScrollBar();
    const QScrollBar* vBar = verticalScrollBar();
    return (hBar && hBar->maximum() > hBar->minimum()) ||
           (vBar && vBar->maximum() > vBar->minimum());
}

QPoint AntScrollArea::mouseDragViewportPos(QObject* watched, const QMouseEvent* event) const
{
    auto* target = qobject_cast<QWidget*>(watched);
    if (!target || !event || target == viewport())
    {
        return event ? event->pos() : QPoint();
    }
    return target->mapTo(viewport(), event->pos());
}

void AntScrollArea::resetMouseDragScrollState()
{
    m_mouseDragPressed = false;
    m_mouseDragActive = false;
}

bool AntScrollArea::updateTheme()
{
    const auto& token = antTheme->tokens();
    const QColor bg = token.colorBgContainer;
    const QColor text = token.colorText;
    const bool themeChanged = !m_themeCacheValid || m_cachedBgColor != bg || m_cachedTextColor != text;
    bool surfaceChanged = false;

    if (themeChanged || !surfaceColorsMatch(palette(), bg, text))
    {
        QPalette pal = palette();
        applySurfaceColors(pal, bg, text);
        setPalette(pal);
        ++m_surfacePaletteApplyCount;
        surfaceChanged = true;
    }

    if (autoFillBackground())
    {
        setAutoFillBackground(false);
    }

    if (viewport())
    {
        if (!viewport()->autoFillBackground())
        {
            viewport()->setAutoFillBackground(true);
        }
        if (themeChanged || !surfaceColorsMatch(viewport()->palette(), bg, text))
        {
            QPalette viewportPalette = viewport()->palette();
            applySurfaceColors(viewportPalette, bg, text);
            viewport()->setPalette(viewportPalette);
            ++m_viewportPaletteApplyCount;
            surfaceChanged = true;
        }
    }

    QWidget* content = widget();
    if (content)
    {
        const bool contentChanged = m_cachedContentWidget != content;
        if (themeChanged || contentChanged || !surfaceColorsMatch(content->palette(), bg, text))
        {
            QPalette contentPalette = content->palette();
            applySurfaceColors(contentPalette, bg, text);
            content->setPalette(contentPalette);
            content->update();
            ++m_contentPaletteApplyCount;
        }
    }

    m_cachedBgColor = bg;
    m_cachedTextColor = text;
    m_cachedContentWidget = content;
    m_themeCacheValid = true;
    syncScrollAreaPerfCounters();
    return surfaceChanged;
}

void AntScrollArea::syncScrollAreaPerfCounters() const
{
    auto* self = const_cast<AntScrollArea*>(this);
    self->setProperty("antScrollAreaSurfacePaletteApplyCount", m_surfacePaletteApplyCount);
    self->setProperty("antScrollAreaViewportPaletteApplyCount", m_viewportPaletteApplyCount);
    self->setProperty("antScrollAreaContentPaletteApplyCount", m_contentPaletteApplyCount);
    self->setProperty("antScrollAreaViewportUpdateCount", m_viewportUpdateCount);
}
