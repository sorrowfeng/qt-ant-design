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

enum class SelectSize
{
    Large,
    Middle,
    Small,
};
Q_ENUM_NS(SelectSize)

enum class SelectStatus
{
    Normal,
    Error,
    Warning,
};
Q_ENUM_NS(SelectStatus)

enum class SelectVariant
{
    Outlined,
    Borderless,
    Filled,
    Underlined,
};
Q_ENUM_NS(SelectVariant)

enum class DatePickerSize
{
    Large,
    Middle,
    Small,
};
Q_ENUM_NS(DatePickerSize)

enum class DatePickerStatus
{
    Normal,
    Error,
    Warning,
};
Q_ENUM_NS(DatePickerStatus)

enum class DatePickerVariant
{
    Outlined,
    Borderless,
    Filled,
    Underlined,
};
Q_ENUM_NS(DatePickerVariant)

enum class TimePickerSize
{
    Large,
    Middle,
    Small,
};
Q_ENUM_NS(TimePickerSize)

enum class TimePickerStatus
{
    Normal,
    Error,
    Warning,
};
Q_ENUM_NS(TimePickerStatus)

enum class TimePickerVariant
{
    Outlined,
    Borderless,
    Filled,
    Underlined,
};
Q_ENUM_NS(TimePickerVariant)

enum class SpinSize
{
    Small,
    Middle,
    Large,
};
Q_ENUM_NS(SpinSize)

enum class ProgressType
{
    Line,
    Circle,
    Dashboard,
};
Q_ENUM_NS(ProgressType)

enum class ProgressStatus
{
    Normal,
    Success,
    Exception,
    Active,
};
Q_ENUM_NS(ProgressStatus)

enum class MessageType
{
    Info,
    Success,
    Warning,
    Error,
    Loading,
};
Q_ENUM_NS(MessageType)

enum class NotificationPlacement
{
    Top,
    TopLeft,
    TopRight,
    Bottom,
    BottomLeft,
    BottomRight,
};
Q_ENUM_NS(NotificationPlacement)

enum class MenuMode
{
    Vertical,
    Horizontal,
    Inline,
};
Q_ENUM_NS(MenuMode)

enum class MenuTheme
{
    Light,
    Dark,
};
Q_ENUM_NS(MenuTheme)

enum class TabsType
{
    Line,
    Card,
    EditableCard,
};
Q_ENUM_NS(TabsType)

enum class TabsSize
{
    Large,
    Middle,
    Small,
};
Q_ENUM_NS(TabsSize)

enum class TabsPlacement
{
    Top,
    Bottom,
    Left,
    Right,
};
Q_ENUM_NS(TabsPlacement)

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
