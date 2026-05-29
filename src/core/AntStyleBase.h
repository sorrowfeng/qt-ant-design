#pragma once

#include "QtAntDesignExport.h"

#include <QApplication>
#include <QBrush>
#include <QList>
#include <QPen>
#include <QPointer>
#include <QProxyStyle>
#include <QSize>
#include <QVariant>
#include <QWidget>

#include "AntTheme.h"

class QPainter;

// Base class for all Ant Design style classes.
// Provides automatic theme-change handling.
class QT_ANT_DESIGN_EXPORT AntStyleBase : public QProxyStyle
{
public:
    explicit AntStyleBase(QStyle* style = nullptr);

    // Draw a rounded rect with sub-pixel precision for crisp borders.
    // When pen is valid, rect is inset by 0.5px to center the border on pixel boundaries.
    static void drawCrispRoundedRect(QPainter* painter, const QRect& rect,
        const QPen& pen, const QBrush& brush, qreal rx, qreal ry);

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;

protected:
    // Called when theme changes. Override to customize.
    // Default: calls updateGeometry() only when themed metrics changed, then update().
    virtual void onThemeUpdate(QWidget* w)
    {
        const QVariant beforeSizeHintValue = w->property("antStyleThemeSizeHintBefore");
        const QVariant beforeMinimumSizeHintValue = w->property("antStyleThemeMinimumSizeHintBefore");
        const QSize beforeSizeHint = beforeSizeHintValue.toSize();
        const QSize beforeMinimumSizeHint = beforeMinimumSizeHintValue.toSize();
        const QSize currentSizeHint = w->sizeHint();
        const QSize currentMinimumSizeHint = w->minimumSizeHint();
        const bool hasBeforeHints = beforeSizeHintValue.isValid() && beforeMinimumSizeHintValue.isValid();
        const bool sizeHintChanged = !hasBeforeHints ||
                                     beforeSizeHint != currentSizeHint ||
                                     beforeMinimumSizeHint != currentMinimumSizeHint;

        w->setProperty("antStyleThemeSizeHintChanged", sizeHintChanged);
        if (sizeHintChanged)
        {
            w->updateGeometry();
            w->setProperty("antStyleThemeUpdateGeometryCount",
                           w->property("antStyleThemeUpdateGeometryCount").toInt() + 1);
        }
        w->update();
    }

    // Helper to connect theme change signal for a specific widget type.
    // Call this in derived constructor: connectThemeUpdate<AntButton>();
    template <typename WidgetType>
    void connectThemeUpdate()
    {
        connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
            bool usedGlobalWidgetScan = false;
            const QList<QWidget*> targets = collectThemeTargets<WidgetType>(&usedGlobalWidgetScan);
            for (QWidget* widget : targets)
            {
                cacheThemeGeometryHints(widget);
            }
        });

        connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
            const int updateCount = property("antStyleThemeUpdateCount").toInt() + 1;
            bool usedGlobalWidgetScan = false;
            const QList<QWidget*> targets = collectThemeTargets<WidgetType>(&usedGlobalWidgetScan);

            for (QWidget* widget : targets)
            {
                updateThemeTarget(widget);
            }

            setProperty("antStyleThemeUpdateCount", updateCount);
            setProperty("antStyleThemeUsesGlobalWidgetScan", usedGlobalWidgetScan);
            setProperty("antStyleThemeCandidateCount", targets.size());
            setProperty("antStyleThemeUpdatedWidgetCount", targets.size());
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

private:
    void cacheThemeGeometryHints(QWidget* widget);
    void pruneThemeWidgets();
    void registerThemeWidget(QWidget* widget);
    void unregisterThemeWidget(QWidget* widget);
    void updateThemeTarget(QWidget* widget);

    template <typename WidgetType>
    QList<QWidget*> collectThemeTargets(bool* usedGlobalWidgetScan)
    {
        if (usedGlobalWidgetScan)
        {
            *usedGlobalWidgetScan = false;
        }

        pruneThemeWidgets();
        QList<QWidget*> targets;

        for (const QPointer<QWidget>& watched : m_themeWidgets)
        {
            QWidget* widget = watched.data();
            if (widget && qobject_cast<WidgetType*>(widget) && widget->style() == this)
            {
                targets.append(widget);
            }
        }

        if (targets.isEmpty())
        {
            if (auto* owner = qobject_cast<QWidget*>(parent()))
            {
                if (qobject_cast<WidgetType*>(owner) && owner->style() == this)
                {
                    targets.append(owner);
                }

                const auto children = owner->findChildren<WidgetType*>(QString(), Qt::FindChildrenRecursively);
                for (WidgetType* child : children)
                {
                    if (child && child->style() == this)
                    {
                        targets.append(child);
                    }
                }
            }
        }

        if (targets.isEmpty())
        {
            if (usedGlobalWidgetScan)
            {
                *usedGlobalWidgetScan = true;
            }
            const auto widgets = QApplication::allWidgets();
            for (QWidget* w : widgets)
            {
                if (qobject_cast<WidgetType*>(w) && w->style() == this)
                {
                    targets.append(w);
                }
            }
        }

        return targets;
    }

    QList<QPointer<QWidget>> m_themeWidgets;
};

template <typename StyleType, typename WidgetType>
StyleType* installAntStyle(WidgetType* widget)
{
    auto* antStyle = new StyleType(widget->style());
    antStyle->setParent(widget);
    widget->setStyle(antStyle);
    return antStyle;
}
