#include "AntTypes.h"

#include <QMetaType>

namespace Ant
{

void registerMetaTypes()
{
    static const bool registered = []() {
        qRegisterMetaType<Qt::Alignment>("Qt::Alignment");
        qRegisterMetaType<Qt::CaseSensitivity>("Qt::CaseSensitivity");
        qRegisterMetaType<Qt::Orientation>("Qt::Orientation");

        qRegisterMetaType<Size>("Ant::Size");
        qRegisterMetaType<Variant>("Ant::Variant");
        qRegisterMetaType<Status>("Ant::Status");
        qRegisterMetaType<Placement>("Ant::Placement");
        qRegisterMetaType<Orientation>("Ant::Orientation");
        qRegisterMetaType<RibbonItemSize>("Ant::RibbonItemSize");
        qRegisterMetaType<Trigger>("Ant::Trigger");
        qRegisterMetaType<ThemeMode>("Ant::ThemeMode");
        qRegisterMetaType<ButtonType>("Ant::ButtonType");
        qRegisterMetaType<ButtonShape>("Ant::ButtonShape");
        qRegisterMetaType<SelectMode>("Ant::SelectMode");
        qRegisterMetaType<ProgressType>("Ant::ProgressType");
        qRegisterMetaType<ProgressStatus>("Ant::ProgressStatus");
        qRegisterMetaType<MessageType>("Ant::MessageType");
        qRegisterMetaType<AlertType>("Ant::AlertType");
        qRegisterMetaType<TooltipPlacement>("Ant::TooltipPlacement");
        qRegisterMetaType<DropdownPlacement>("Ant::DropdownPlacement");
        qRegisterMetaType<DropdownTrigger>("Ant::DropdownTrigger");
        qRegisterMetaType<PopoverTrigger>("Ant::PopoverTrigger");
        qRegisterMetaType<MenuMode>("Ant::MenuMode");
        qRegisterMetaType<MenuTheme>("Ant::MenuTheme");
        qRegisterMetaType<TabsType>("Ant::TabsType");
        qRegisterMetaType<TabsPlacement>("Ant::TabsPlacement");
        qRegisterMetaType<TagVariant>("Ant::TagVariant");
        qRegisterMetaType<BadgeStatus>("Ant::BadgeStatus");
        qRegisterMetaType<BadgeMode>("Ant::BadgeMode");
        qRegisterMetaType<AvatarShape>("Ant::AvatarShape");
        qRegisterMetaType<DividerTitlePlacement>("Ant::DividerTitlePlacement");
        qRegisterMetaType<DividerVariant>("Ant::DividerVariant");
        qRegisterMetaType<CardSize>("Ant::CardSize");
        qRegisterMetaType<CardVariant>("Ant::CardVariant");
        qRegisterMetaType<SkeletonElement>("Ant::SkeletonElement");
        qRegisterMetaType<FormLayout>("Ant::FormLayout");
        qRegisterMetaType<FormLabelAlign>("Ant::FormLabelAlign");
        qRegisterMetaType<StepStatus>("Ant::StepStatus");
        qRegisterMetaType<IconType>("Ant::IconType");
        qRegisterMetaType<IconTheme>("Ant::IconTheme");
        qRegisterMetaType<TimelineMode>("Ant::TimelineMode");
        qRegisterMetaType<TimelineOrientation>("Ant::TimelineOrientation");
        qRegisterMetaType<TimelineDotVariant>("Ant::TimelineDotVariant");
        qRegisterMetaType<LayoutSiderTheme>("Ant::LayoutSiderTheme");
        qRegisterMetaType<TypographyType>("Ant::TypographyType");
        qRegisterMetaType<TypographyTitleLevel>("Ant::TypographyTitleLevel");
        qRegisterMetaType<TableSortOrder>("Ant::TableSortOrder");
        qRegisterMetaType<TableSelectionMode>("Ant::TableSelectionMode");
        qRegisterMetaType<TableColumnAlign>("Ant::TableColumnAlign");
        qRegisterMetaType<UploadListType>("Ant::UploadListType");
        qRegisterMetaType<UploadFileStatus>("Ant::UploadFileStatus");
        qRegisterMetaType<DrawerPlacement>("Ant::DrawerPlacement");
        qRegisterMetaType<SegmentedShape>("Ant::SegmentedShape");
        qRegisterMetaType<QRCodeErrorLevel>("Ant::QRCodeErrorLevel");
        qRegisterMetaType<QRCodeStatus>("Ant::QRCodeStatus");
        qRegisterMetaType<FloatButtonType>("Ant::FloatButtonType");
        qRegisterMetaType<FloatButtonShape>("Ant::FloatButtonShape");
        qRegisterMetaType<FloatButtonPlacement>("Ant::FloatButtonPlacement");
        qRegisterMetaType<ColorPickerMode>("Ant::ColorPickerMode");
        qRegisterMetaType<CalendarMode>("Ant::CalendarMode");
        return true;
    }();
    Q_UNUSED(registered);
}

} // namespace Ant
