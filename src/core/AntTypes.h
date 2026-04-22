#pragma once

#include <QObject>

namespace Ant
{
Q_NAMESPACE

enum class ThemeMode
{
    Default,
    Dark,
};
Q_ENUM_NS(ThemeMode)

enum class ButtonType
{
    Default,
    Primary,
    Dashed,
    Text,
    Link,
};
Q_ENUM_NS(ButtonType)

enum class ButtonSize
{
    Large,
    Middle,
    Small,
};
Q_ENUM_NS(ButtonSize)

enum class ButtonShape
{
    Default,
    Circle,
    Round,
};
Q_ENUM_NS(ButtonShape)

enum class InputSize
{
    Large,
    Middle,
    Small,
};
Q_ENUM_NS(InputSize)

enum class InputStatus
{
    Normal,
    Error,
    Warning,
};
Q_ENUM_NS(InputStatus)

enum class SwitchSize
{
    Middle,
    Small,
};
Q_ENUM_NS(SwitchSize)

enum class CardSize
{
    Default,
    Small,
};
Q_ENUM_NS(CardSize)

enum class CardVariant
{
    Outlined,
    Borderless,
};
Q_ENUM_NS(CardVariant)

constexpr int LineWidth = 1;
constexpr int SizeUnit = 4;
constexpr int SizeStep = 4;
constexpr int ControlHeight = 32;
constexpr int ControlHeightSmall = 24;
constexpr int ControlHeightLarge = 40;
constexpr int FontSize = 14;
constexpr int FontSizeSmall = 12;
constexpr int FontSizeLarge = 16;
constexpr int BorderRadius = 6;

} // namespace Ant
