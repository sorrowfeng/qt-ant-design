#include "PageRegistry.h"

#include "Pages.h"

namespace example::pages
{
namespace
{
const QString kShowcase = QStringLiteral("Showcase");
const QString kGeneral = QStringLiteral("General");
const QString kLayout = QStringLiteral("Layout");
const QString kNavigation = QStringLiteral("Navigation");
const QString kDataEntry = QStringLiteral("Data Entry");
const QString kDataDisplay = QStringLiteral("Data Display");
const QString kFeedback = QStringLiteral("Feedback");
const QString kOther = QStringLiteral("Other");
const QString kQtExtensions = QStringLiteral("Qt Extensions");
}

QVector<PageEntry> buildPageRegistry()
{
    QVector<PageEntry> registry;
    auto add = [&registry](const QString& category, const QString& name,
                           std::function<QWidget*(QWidget*)> factory) {
        registry.push_back({category, name, std::move(factory)});
    };

    add(kShowcase, QStringLiteral("Showcase"), &createShowcasePage);

    add(kGeneral, QStringLiteral("Button"), &createButtonPage);
    add(kGeneral, QStringLiteral("Icon"), &createIconPage);
    add(kGeneral, QStringLiteral("Typography"), &createTypographyPage);

    add(kLayout, QStringLiteral("Divider"), &createDividerPage);
    add(kLayout, QStringLiteral("Flex"), &createFlexPage);
    add(kLayout, QStringLiteral("Grid"), &createGridPage);
    add(kLayout, QStringLiteral("Layout"), &createLayoutPage);
    add(kLayout, QStringLiteral("Space"), &createSpacePage);
    add(kLayout, QStringLiteral("Splitter"), &createSplitterPage);

    add(kNavigation, QStringLiteral("Affix"), &createAffixPage);
    add(kNavigation, QStringLiteral("Anchor"), &createAnchorPage);
    add(kNavigation, QStringLiteral("Breadcrumb"), &createBreadcrumbPage);
    add(kNavigation, QStringLiteral("Dropdown"), &createDropdownPage);
    add(kNavigation, QStringLiteral("Menu"), &createMenuPage);
    add(kNavigation, QStringLiteral("Pagination"), &createPaginationPage);
    add(kNavigation, QStringLiteral("Steps"), &createStepsPage);

    add(kDataEntry, QStringLiteral("AutoComplete"), &createAutoCompletePage);
    add(kDataEntry, QStringLiteral("Cascader"), &createCascaderPage);
    add(kDataEntry, QStringLiteral("Checkbox"), &createCheckboxPage);
    add(kDataEntry, QStringLiteral("ColorPicker"), &createColorPickerPage);
    add(kDataEntry, QStringLiteral("DatePicker"), &createDatePickerPage);
    add(kDataEntry, QStringLiteral("Form"), &createFormPage);
    add(kDataEntry, QStringLiteral("Input"), &createInputPage);
    add(kDataEntry, QStringLiteral("InputNumber"), &createInputNumberPage);
    add(kDataEntry, QStringLiteral("Mentions"), &createMentionsPage);
    add(kDataEntry, QStringLiteral("Radio"), &createRadioPage);
    add(kDataEntry, QStringLiteral("Rate"), &createRatePage);
    add(kDataEntry, QStringLiteral("Select"), &createSelectPage);
    add(kDataEntry, QStringLiteral("Slider"), &createSliderPage);
    add(kDataEntry, QStringLiteral("Switch"), &createSwitchPage);
    add(kDataEntry, QStringLiteral("TimePicker"), &createTimePickerPage);
    add(kDataEntry, QStringLiteral("Transfer"), &createTransferPage);
    add(kDataEntry, QStringLiteral("TreeSelect"), &createTreeSelectPage);
    add(kDataEntry, QStringLiteral("Upload"), &createUploadPage);

    add(kDataDisplay, QStringLiteral("Avatar"), &createAvatarPage);
    add(kDataDisplay, QStringLiteral("Badge"), &createBadgePage);
    add(kDataDisplay, QStringLiteral("Calendar"), &createCalendarPage);
    add(kDataDisplay, QStringLiteral("Card"), &createCardPage);
    add(kDataDisplay, QStringLiteral("Carousel"), &createCarouselPage);
    add(kDataDisplay, QStringLiteral("Collapse"), &createCollapsePage);
    add(kDataDisplay, QStringLiteral("Descriptions"), &createDescriptionsPage);
    add(kDataDisplay, QStringLiteral("Empty"), &createEmptyPage);
    add(kDataDisplay, QStringLiteral("Image"), &createImagePage);
    add(kDataDisplay, QStringLiteral("List"), &createListPage);
    add(kDataDisplay, QStringLiteral("Popover"), &createPopoverPage);
    add(kDataDisplay, QStringLiteral("QRCode"), &createQRCodePage);
    add(kDataDisplay, QStringLiteral("Segmented"), &createSegmentedPage);
    add(kDataDisplay, QStringLiteral("Statistic"), &createStatisticPage);
    add(kDataDisplay, QStringLiteral("Table"), &createTablePage);
    add(kDataDisplay, QStringLiteral("Tabs"), &createTabsPage);
    add(kDataDisplay, QStringLiteral("Tag"), &createTagPage);
    add(kDataDisplay, QStringLiteral("Timeline"), &createTimelinePage);
    add(kDataDisplay, QStringLiteral("Tooltip"), &createTooltipPage);
    add(kDataDisplay, QStringLiteral("Tree"), &createTreePage);

    add(kFeedback, QStringLiteral("Alert"), &createAlertPage);
    add(kFeedback, QStringLiteral("Drawer"), &createDrawerPage);
    add(kFeedback, QStringLiteral("Message"), &createMessagePage);
    add(kFeedback, QStringLiteral("Modal"), &createModalPage);
    add(kFeedback, QStringLiteral("Notification"), &createNotificationPage);
    add(kFeedback, QStringLiteral("Popconfirm"), &createPopconfirmPage);
    add(kFeedback, QStringLiteral("Progress"), &createProgressPage);
    add(kFeedback, QStringLiteral("Result"), &createResultPage);
    add(kFeedback, QStringLiteral("Skeleton"), &createSkeletonPage);
    add(kFeedback, QStringLiteral("Spin"), &createSpinPage);
    add(kFeedback, QStringLiteral("Tour"), &createTourPage);
    add(kFeedback, QStringLiteral("Watermark"), &createWatermarkPage);

    add(kOther, QStringLiteral("App"), &createAppPage);
    add(kOther, QStringLiteral("ConfigProvider"), &createConfigProviderPage);
    add(kOther, QStringLiteral("FloatButton"), &createFloatButtonPage);

    add(kQtExtensions, QStringLiteral("Window"), &createWindowPage);
    add(kQtExtensions, QStringLiteral("ScrollArea"), &createScrollAreaPage);
    add(kQtExtensions, QStringLiteral("ScrollBar"), &createScrollBarPage);
    add(kQtExtensions, QStringLiteral("StatusBar"), &createStatusBarPage);
    add(kQtExtensions, QStringLiteral("MenuBar"), &createMenuBarPage);
    add(kQtExtensions, QStringLiteral("ToolBar"), &createToolBarPage);
    add(kQtExtensions, QStringLiteral("ToolButton"), &createToolButtonPage);
    add(kQtExtensions, QStringLiteral("DockWidget"), &createDockWidgetPage);
    add(kQtExtensions, QStringLiteral("PlainTextEdit"), &createPlainTextEditPage);
    add(kQtExtensions, QStringLiteral("Log"), &createLogPage);
    add(kQtExtensions, QStringLiteral("Masonry"), &createMasonryPage);

    return registry;
}
}
