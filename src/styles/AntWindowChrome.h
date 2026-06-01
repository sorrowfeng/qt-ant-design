#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QIcon>
#include <QRect>
#include <QRectF>
#include <QString>
#include <QVector>

class QPainter;
class QWidget;

namespace AntWindowChrome
{
constexpr int TitleBarHeight = 40;
constexpr int TitleBarButtonWidth = 46;

enum class TitleBarButton
{
    None,
    Pin,
    Theme,
    Minimize,
    Maximize,
    Close,
};

struct TitleBarButtonState
{
    TitleBarButton button = TitleBarButton::None;
    QRect rect;
    bool hovered = false;
    bool pressed = false;
    bool active = false;
    bool destructive = false;
};

struct PaintOptions
{
    QRect surfaceRect;
    QRect titleBarRect;
    const QWidget* widget = nullptr;
    QString title;
    QIcon icon;
    QVector<TitleBarButtonState> buttons;
    int cornerRadius = 0;
    bool maximized = false;
    bool drawSurfaceBorder = false;
    QColor surfaceBorderColor;
    bool drawLegacyOutline = false;
};

QT_ANT_DESIGN_EXPORT QRectF centeredIconRect(const QRect& buttonRect, qreal iconSize = 14.0);
QT_ANT_DESIGN_EXPORT QColor legacyOutlineColor();
QT_ANT_DESIGN_EXPORT void drawChrome(QPainter* painter, const PaintOptions& options);
} // namespace AntWindowChrome
