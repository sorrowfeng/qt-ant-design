#pragma once

#include "QtAntDesignExport.h"

#include <QObject>

namespace Ant
{
Q_NAMESPACE

// ============================================================================
// Shared enums (consolidated from per-component duplicates)
// ============================================================================

enum class Size
{
    Large,
    Middle,
    Small,
};
Q_ENUM_NS(Size)

enum class Variant
{
    Outlined,
    Borderless,
    Filled,
    Underlined,
};
Q_ENUM_NS(Variant)

enum class Status
{
    Normal,
    Error,
    Warning,
};
Q_ENUM_NS(Status)

enum class StatusBarStatus
{
    Default,
    Info,
    Success,
    Warning,
    Error,
    Inherit,
};
Q_ENUM_NS(StatusBarStatus)

// Unified placement superset shared by Tooltip/Dropdown/Drawer/FloatButton.
// Kept in the same value order as the former TooltipPlacement so ordinal
// behavior is unchanged; the other placement enums are aliases of this one.
enum class Placement
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
Q_ENUM_NS(Placement)

enum class Orientation
{
    Horizontal,
    Vertical,
};
Q_ENUM_NS(Orientation)

enum class RibbonItemSize
{
    Large,
    Small,
};
Q_ENUM_NS(RibbonItemSize)

enum class Trigger
{
    Click,
    Hover,
};
Q_ENUM_NS(Trigger)

// ============================================================================
// Component-specific enums (unique values, not duplicated)
// ============================================================================

enum class ThemeMode
{
    Default,
    Dark,
};
Q_ENUM_NS(ThemeMode)

// 主题密度（对应上游 theme algorithm 的 defaultAlgorithm / compactAlgorithm）。
enum class ThemeDensity
{
    Default,
    Compact,
};
Q_ENUM_NS(ThemeDensity)

// 内建文案语言（对应上游 LocaleProvider 的 locale 包）。
enum class LocaleLanguage
{
    English,
    ChineseSimplified,
};
Q_ENUM_NS(LocaleLanguage)

enum class ButtonType
{
    Default,
    Primary,
    Dashed,
    Text,
    Link,
};
Q_ENUM_NS(ButtonType)

// 图标相对文本的位置（对应上游 iconPlacement="start|end"）。
enum class IconPlacement
{
    Start,
    End,
};
Q_ENUM_NS(IconPlacement)

enum class ButtonShape
{
    Default,
    Circle,
    Round,
};
Q_ENUM_NS(ButtonShape)

enum class SelectMode
{
    Single,
    Multiple,
    Tags,
};
Q_ENUM_NS(SelectMode)

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

// Result status shares the same four semantic states as AlertType but is a
// distinct concept (what a Result page communicates), so it gets its own
// well-named type instead of reusing the alert-oriented one.
enum class ResultStatus
{
    Success,
    Info,
    Warning,
    Error,
};
Q_ENUM_NS(ResultStatus)

// Legacy alias - prefer Ant::Placement.
using TooltipPlacement = Placement;

// Legacy alias - prefer Ant::Placement.
using DropdownPlacement = Placement;

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

enum class TabsPlacement
{
    Top,
    Bottom,
    Left,
    Right,
};
Q_ENUM_NS(TabsPlacement)

enum class TagVariant
{
    Filled,
    Solid,
    Outlined,
};
Q_ENUM_NS(TagVariant)

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

enum class BadgeMode
{
    Default,
    Dot,
    Ribbon,
};
Q_ENUM_NS(BadgeMode)

enum class AvatarShape
{
    Circle,
    Square,
};
Q_ENUM_NS(AvatarShape)

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

enum class SkeletonElement
{
    Default,
    Button,
    Avatar,
    Input,
    Image,
    Node,
};
Q_ENUM_NS(SkeletonElement)

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
    Setting,
    Heart,
    Bell,
    Mail,
    Edit,
    Delete,
    CloudUpload,
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
    Link,
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

// Legacy alias - prefer Ant::Placement.
using DrawerPlacement = Placement;

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

// Legacy alias - prefer Ant::Placement.
using FloatButtonPlacement = Placement;

enum class ColorPickerMode
{
    RGB,
    HSV,
    HEX,
};
Q_ENUM_NS(ColorPickerMode)

enum class CalendarMode
{
    Day,
    Month,
    Year,
};
Q_ENUM_NS(CalendarMode)

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
constexpr int MaximumThemeFontSize = 512;
constexpr int MaximumThemeBorderRadius = 512;

QT_ANT_DESIGN_EXPORT void registerMetaTypes();

} // namespace Ant
