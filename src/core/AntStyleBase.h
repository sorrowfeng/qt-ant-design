#pragma once

#include <QApplication>
#include <QProxyStyle>
#include <QWidget>

#include "AntTheme.h"

// Base class for all Ant Design style classes.
// Provides automatic theme-change handling.
class AntStyleBase : public QProxyStyle
{
public:
    explicit AntStyleBase(QStyle* style = nullptr)
        : QProxyStyle(style)
    {
    }

protected:
    // Called when theme changes. Override to customize.
    // Default: calls updateGeometry() + update().
    virtual void onThemeUpdate(QWidget* w)
    {
        w->updateGeometry();
        w->update();
    }

    // Helper to connect theme change signal for a specific widget type.
    // Call this in derived constructor: connectThemeUpdate<AntButton>();
    template <typename WidgetType>
    void connectThemeUpdate()
    {
        connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
            const auto widgets = QApplication::allWidgets();
            for (QWidget* w : widgets)
            {
                if (qobject_cast<WidgetType*>(w) && w->style() == this)
                {
                    unpolish(w);
                    polish(w);
                    onThemeUpdate(w);
                }
            }
        });
    }

    // ---- Paint-event helpers (Pattern A) ----
    // Use these to reduce boilerplate in style classes that paint via eventFilter.

    // Install event filter + WA_Hover on a widget of the given type.
    template <typename WidgetType>
    void installPaintFilter(QWidget* widget)
    {
        if (qobject_cast<WidgetType*>(widget))
        {
            widget->installEventFilter(this);
            widget->setAttribute(Qt::WA_Hover);
        }
    }

    // Remove event filter on a widget of the given type.
    template <typename WidgetType>
    void removePaintFilter(QWidget* widget)
    {
        if (qobject_cast<WidgetType*>(widget))
        {
            widget->removeEventFilter(this);
        }
    }

    // Default event filter: intercepts Paint events and calls drawWidget().
    // Override drawWidget() in derived classes for custom painting.
    // If the derived class needs a different event filter, override this.
    bool eventFilter(QObject* watched, QEvent* event) override;

    // Override this in derived classes to paint the widget.
    // Called by the default eventFilter when a Paint event is received.
    // The base implementation draws nothing (returns false).
    virtual bool drawWidget(QWidget* widget, QPaintEvent* event);
};
