#pragma once

#include <QSize>
#include <QVariant>
#include <QWidget>

namespace AntThemeRefresh
{
constexpr auto kCachedSizeHint = "antThemeRefreshCachedSizeHint";
constexpr auto kCachedMinimumSizeHint = "antThemeRefreshCachedMinimumSizeHint";
constexpr auto kSizeHintBefore = "antThemeRefreshSizeHintBefore";
constexpr auto kMinimumSizeHintBefore = "antThemeRefreshMinimumSizeHintBefore";
constexpr auto kSizeHintChanged = "antThemeRefreshSizeHintChanged";
constexpr auto kUpdateGeometryCount = "antThemeRefreshUpdateGeometryCount";

inline void cacheGeometryHints(QWidget* widget)
{
    if (!widget)
    {
        return;
    }

    widget->setProperty(kCachedSizeHint, widget->sizeHint());
    widget->setProperty(kCachedMinimumSizeHint, widget->minimumSizeHint());
}

inline bool updateGeometryIfSizeHintChanged(QWidget* widget)
{
    if (!widget)
    {
        return false;
    }

    const QVariant beforeSizeHintValue = widget->property(kCachedSizeHint);
    const QVariant beforeMinimumSizeHintValue = widget->property(kCachedMinimumSizeHint);
    const QSize beforeSizeHint = beforeSizeHintValue.toSize();
    const QSize beforeMinimumSizeHint = beforeMinimumSizeHintValue.toSize();
    const QSize currentSizeHint = widget->sizeHint();
    const QSize currentMinimumSizeHint = widget->minimumSizeHint();
    const bool hasBeforeHints = beforeSizeHintValue.isValid() && beforeMinimumSizeHintValue.isValid();
    const bool sizeHintChanged = !hasBeforeHints ||
                                 beforeSizeHint != currentSizeHint ||
                                 beforeMinimumSizeHint != currentMinimumSizeHint;

    widget->setProperty(kSizeHintBefore, beforeSizeHintValue);
    widget->setProperty(kMinimumSizeHintBefore, beforeMinimumSizeHintValue);
    widget->setProperty(kSizeHintChanged, sizeHintChanged);
    if (sizeHintChanged)
    {
        widget->updateGeometry();
        widget->setProperty(kUpdateGeometryCount, widget->property(kUpdateGeometryCount).toInt() + 1);
    }
    return sizeHintChanged;
}
} // namespace AntThemeRefresh
