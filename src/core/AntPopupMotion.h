#pragma once

#include "QtAntDesignExport.h"

#include <QWidget>

#include "core/AntTypes.h"

class QT_ANT_DESIGN_EXPORT AntPopupMotion
{
public:
    enum class Placement
    {
        Top,
        Bottom,
        Left,
        Right,
        Center
    };

    static void show(QWidget* popup, Placement placement = Placement::Bottom, int distance = 4);
    static void hide(QWidget* popup, Placement placement = Placement::Bottom, int distance = 4);
    static void close(QWidget* popup, Placement placement = Placement::Bottom, int distance = 4);
    static void stop(QWidget* popup);
    static bool isClosing(const QWidget* popup);

    static Placement fromPlacement(Ant::Placement placement);
    static Placement fromDropdownPlacement(Ant::Placement placement);
    static Placement fromTooltipPlacement(Ant::Placement placement);

private:
    AntPopupMotion() = default;
};
