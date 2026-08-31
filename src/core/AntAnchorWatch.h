#pragma once

// Shared anchor-lifecycle tracking for popup/feedback widgets (AntMessage,
// AntNotification). Encapsulates the duplicated pattern of installing an event
// filter on an anchor widget plus its top-level window, clearing both on
// anchor destruction, and invoking a host-supplied callback so the popup can
// animate closed. Collapses the ~40-line installAnchorWatcher/uninstallAnchorWatcher
// pair that was previously copy-pasted between the two feedback widgets.

#include "QtAntDesignExport.h"

#include <QPointer>
#include <QWidget>

#include <functional>

class QT_ANT_DESIGN_EXPORT AntAnchorWatch
{
public:
    AntAnchorWatch() = default;

    ~AntAnchorWatch() { clear(); }

    // Install the event filter on `anchor` and (when distinct) its top-level
    // window. When `anchor` is destroyed, both filters are removed and
    // `onAnchorDestroyed` is invoked. Passing a null anchor clears any existing
    // watch. `filter` is the QObject that receives the filtered events (the
    // host popup widget); it must outlive this watch or be cleared first.
    void set(QWidget* anchor, QObject* filter, std::function<void()> onAnchorDestroyed)
    {
        clear();
        if (!anchor || !filter)
        {
            return;
        }

        m_anchor = anchor;
        m_filter = filter;

        anchor->installEventFilter(filter);

        QWidget* window = anchor->window();
        if (window && window != anchor)
        {
            m_anchorWindow = window;
            window->installEventFilter(filter);
        }

        // Anchor destruction: Qt emits destroyed() before clearing QPointer
        // guards, so m_anchor is still non-null here. The anchor's own filter is
        // removed automatically by QObject destruction; only the window filter
        // needs explicit removal. We do NOT call removeEventFilter on the
        // half-destructed anchor.
        QObject::connect(anchor, &QObject::destroyed, filter, [this, onAnchorDestroyed]() {
            if (m_anchorWindow)
            {
                m_anchorWindow->removeEventFilter(m_filter);
            }
            m_anchor.clear();
            m_anchorWindow.clear();
            m_filter = nullptr;
            if (onAnchorDestroyed)
            {
                onAnchorDestroyed();
            }
        });
    }

    // Remove all installed filters and forget the anchor. Safe to call when the
    // anchor is still alive (e.g. host destructor, explicit detach).
    void clear()
    {
        if (m_filter)
        {
            if (m_anchor)
            {
                m_anchor->removeEventFilter(m_filter);
            }
            if (m_anchorWindow)
            {
                m_anchorWindow->removeEventFilter(m_filter);
            }
        }
        m_anchor.clear();
        m_anchorWindow.clear();
        m_filter = nullptr;
    }

    QWidget* anchor() const { return m_anchor.data(); }
    QWidget* anchorWindow() const { return m_anchorWindow.data(); }
    bool isValid() const { return !m_anchor.isNull(); }

private:
    QPointer<QWidget> m_anchor;
    QPointer<QWidget> m_anchorWindow;
    QObject* m_filter = nullptr;
};
