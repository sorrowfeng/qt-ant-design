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

    // Draw the focus outline glow shared by input-family widgets (Input,
    // InputNumber, Select, Cascader, DatePicker, TimePicker, TreeSelect,
    // AutoComplete). The glow is a soft rounded outline drawn 1px outside the
    // frame, following the Ant Design "focus outline" convention:
    //   frameRect.adjusted(-1, -1, 1, 1) + pen(glowColor, outlineWidth) + radius + 1
    // glowColor must already carry its desired alpha (e.g. alpha(border, 0.16)).
    static void drawInputFocusGlow(QPainter* painter, const QRectF& frameRect,
        qreal radius, const QColor& glowColor, qreal outlineWidth);

    // ---- Button-family shared drawing helpers ----
    // Used by AntButtonStyle / AntToolButtonStyle / AntToolBarStyle to avoid
    // three copies of the same shadow / focus outline / spinner code.

    // Padding reserved around a button for its focus outline
    // (lineWidthFocus + 1). Used for size hints and hit-testing.
    static int focusPaddingFor();

    // Draw the soft bottom shadow under a button body (Ant Design elevation).
    // No-op when color is transparent.
    static void drawButtonBottomShadow(QPainter* painter, const QRectF& outer,
        int radius, const QColor& color);

    // Draw the focus outline around a button body: a rounded ring 1px outside
    // the body using token.colorPrimaryBorder at lineWidthFocus width.
    static void drawButtonFocusOutline(QPainter* painter, const QRectF& bodyRect,
        int radius);

    // Draw a loading spinner arc. The arc starts at the top when angle == 0
    // and its start point rotates clockwise as angle increases.
    //   spanAngle - arc length in degrees, drawn clockwise (Ant Design loader
    //               shows ~30%, i.e. 96 degrees); default 96
    //   penWidth  - stroke width; when <= 0 it derives from rect width
    //               (max(1.5, rect.width() * 0.12))
    static void drawSpinner(QPainter* painter, const QRectF& rect,
        const QColor& color, int angle, int spanAngle = 96, qreal penWidth = -1);

    // Draw the Ant Design empty-state illustration, shared by AntEmptyStyle
    // and AntTableStyle (empty table state). The illustration is drawn in its
    // intrinsic 184x152 (default) or 128x80 (simple) coordinate system: the
    // painter is translated to targetRect.topLeft() and scaled so the artwork
    // fills targetRect exactly. Colors derive from the current theme tokens
    // (dark mode adjusts opacities automatically).
    // extraLine additionally draws the third text line used by the Table
    // empty-state variant.
    static void drawEmptyIllustration(QPainter* painter, const QRectF& targetRect,
        bool simple, bool extraLine = false);

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
        connect(antTheme, &AntTheme::themeAboutToChange, this, [this]() {
            bool usedGlobalWidgetScan = false;
            const QList<QWidget*> targets = collectThemeTargets<WidgetType>(&usedGlobalWidgetScan);
            for (QWidget* widget : targets)
            {
                cacheThemeGeometryHints(widget);
            }
        });

        connect(antTheme, &AntTheme::themeChanged, this, [this]() {
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
