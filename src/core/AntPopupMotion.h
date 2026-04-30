#pragma once

#include <QWidget>

#include "core/AntTypes.h"

class AntPopupMotion
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
    static Placement fromDropdownPlacement(Ant::DropdownPlacement placement);
    static Placement fromTooltipPlacement(Ant::TooltipPlacement placement);

private:
    AntPopupMotion() = default;
};
