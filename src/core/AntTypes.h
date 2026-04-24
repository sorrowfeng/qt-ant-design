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

enum class InputVariant
{
    Outlined,
    Borderless,
    Filled,
    Underlined,
};
Q_ENUM_NS(InputVariant)

enum class InputNumberVariant
{
    Outlined,
    Borderless,
    Filled,
    Underlined,
};
Q_ENUM_NS(InputNumberVariant)

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

enum class AlertType
{
    Success,
    Info,
    Warning,
    Error,
};
Q_ENUM_NS(AlertType)

enum class TooltipPlacement
{
    Top,
    TopLeft,
    TopRight,
    Bottom,
    BottomLeft,
    BottomRight,
    Left,
    Right,
};
Q_ENUM_NS(TooltipPlacement)

enum class DropdownPlacement
{
    Bottom,
    BottomLeft,
    BottomRight,
    Top,
    TopLeft,
    TopRight,
};
Q_ENUM_NS(DropdownPlacement)

enum class DropdownTrigger
{
    Hover,
    Click,
    ContextMenu,
};
Q_ENUM_NS(DropdownTrigger)

enum class PopoverTrigger
{
    Hover,
    Click,
};
Q_ENUM_NS(PopoverTrigger)

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

enum class PaginationSize
{
    Large,
    Middle,
    Small,
};
Q_ENUM_NS(PaginationSize)

enum class TagVariant
{
    Filled,
    Solid,
    Outlined,
};
Q_ENUM_NS(TagVariant)

enum class BadgeSize
{
    Middle,
    Small,
};
Q_ENUM_NS(BadgeSize)

enum class BadgeStatus
{
    None,
    Success,
    Processing,
    Default,
    Error,
    Warning,
};
Q_ENUM_NS(BadgeStatus)

enum class AvatarShape
{
    Circle,
    Square,
};
Q_ENUM_NS(AvatarShape)

enum class AvatarSize
{
    Large,
    Middle,
    Small,
};
Q_ENUM_NS(AvatarSize)

enum class DividerOrientation
{
    Horizontal,
    Vertical,
};
Q_ENUM_NS(DividerOrientation)

enum class DividerTitlePlacement
{
    Start,
    Center,
    End,
};
Q_ENUM_NS(DividerTitlePlacement)

enum class DividerVariant
{
    Solid,
    Dashed,
    Dotted,
};
Q_ENUM_NS(DividerVariant)

enum class DividerSize
{
    Large,
    Middle,
    Small,
};
Q_ENUM_NS(DividerSize)

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

enum class FormLayout
{
    Horizontal,
    Vertical,
    Inline,
};
Q_ENUM_NS(FormLayout)

enum class FormLabelAlign
{
    Left,
    Right,
};
Q_ENUM_NS(FormLabelAlign)

enum class StepStatus
{
    Wait,
    Process,
    Finish,
    Error,
};
Q_ENUM_NS(StepStatus)

enum class StepsDirection
{
    Horizontal,
    Vertical,
};
Q_ENUM_NS(StepsDirection)

enum class IconType
{
    None,
    Search,
    Close,
    Plus,
    Minus,
    Check,
    InfoCircle,
    ExclamationCircle,
    CloseCircle,
    CheckCircle,
    Loading,
    Down,
    Up,
    Left,
    Right,
    Calendar,
    ClockCircle,
    User,
    Home,
    Star,
};
Q_ENUM_NS(IconType)

enum class IconTheme
{
    Outlined,
    Filled,
    TwoTone,
};
Q_ENUM_NS(IconTheme)

enum class TimelineMode
{
    Start,
    Alternate,
    End,
};
Q_ENUM_NS(TimelineMode)

enum class TimelineOrientation
{
    Vertical,
    Horizontal,
};
Q_ENUM_NS(TimelineOrientation)

enum class TimelineDotVariant
{
    Outlined,
    Filled,
};
Q_ENUM_NS(TimelineDotVariant)

enum class SpaceOrientation
{
    Horizontal,
    Vertical,
};
Q_ENUM_NS(SpaceOrientation)

enum class SpaceSize
{
    Small,
    Middle,
    Large,
};
Q_ENUM_NS(SpaceSize)

enum class LayoutSiderTheme
{
    Dark,
    Light,
};
Q_ENUM_NS(LayoutSiderTheme)

enum class TypographyType
{
    Default,
    Secondary,
    Success,
    Warning,
    Danger,
    LightSolid,
};
Q_ENUM_NS(TypographyType)

enum class TypographyTitleLevel
{
    H1,
    H2,
    H3,
    H4,
    H5,
};
Q_ENUM_NS(TypographyTitleLevel)

enum class TableSize
{
    Large,
    Middle,
    Small,
};
Q_ENUM_NS(TableSize)

enum class TableSortOrder
{
    None,
    Ascending,
    Descending,
};
Q_ENUM_NS(TableSortOrder)

enum class TableSelectionMode
{
    None,
    Checkbox,
    Radio,
};
Q_ENUM_NS(TableSelectionMode)

enum class TableColumnAlign
{
    Left,
    Center,
    Right,
};
Q_ENUM_NS(TableColumnAlign)

enum class UploadListType
{
    Text,
    Picture,
    PictureCard,
};
Q_ENUM_NS(UploadListType)

enum class UploadFileStatus
{
    Uploading,
    Done,
    Error,
    Removed,
};
Q_ENUM_NS(UploadFileStatus)

enum class CascaderExpandTrigger
{
    Click,
    Hover,
};
Q_ENUM_NS(CascaderExpandTrigger)

enum class DrawerPlacement
{
    Left,
    Right,
    Top,
    Bottom,
};
Q_ENUM_NS(DrawerPlacement)

enum class RateSize
{
    Small,
    Middle,
    Large,
};
Q_ENUM_NS(RateSize)

enum class SegmentedSize
{
    Small,
    Middle,
    Large,
};
Q_ENUM_NS(SegmentedSize)

enum class SegmentedShape
{
    Default,
    Round,
};
Q_ENUM_NS(SegmentedShape)

enum class QRCodeErrorLevel
{
    L,
    M,
    Q,
    H,
};
Q_ENUM_NS(QRCodeErrorLevel)

enum class QRCodeStatus
{
    Active,
    Expired,
    Loading,
    Scanned,
};
Q_ENUM_NS(QRCodeStatus)

enum class FloatButtonType
{
    Default,
    Primary,
};
Q_ENUM_NS(FloatButtonType)

enum class FloatButtonShape
{
    Circle,
    Square,
};
Q_ENUM_NS(FloatButtonShape)

enum class FloatButtonTrigger
{
    Click,
    Hover,
};
Q_ENUM_NS(FloatButtonTrigger)

enum class FloatButtonPlacement
{
    BottomRight,
    BottomLeft,
    TopRight,
    TopLeft,
};
Q_ENUM_NS(FloatButtonPlacement)

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
