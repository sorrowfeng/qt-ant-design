#pragma once

class QWidget;

namespace example::pages
{
// Showcase
QWidget* createShowcasePage(QWidget* owner);

// 通用 (General)
QWidget* createButtonPage(QWidget* owner);
QWidget* createIconPage(QWidget* owner);
QWidget* createTypographyPage(QWidget* owner);

// 布局 (Layout)
QWidget* createDividerPage(QWidget* owner);
QWidget* createFlexPage(QWidget* owner);
QWidget* createGridPage(QWidget* owner);
QWidget* createLayoutPage(QWidget* owner);
QWidget* createSpacePage(QWidget* owner);
QWidget* createSplitterPage(QWidget* owner);

// 导航 (Navigation)
QWidget* createAffixPage(QWidget* owner);
QWidget* createAnchorPage(QWidget* owner);
QWidget* createBreadcrumbPage(QWidget* owner);
QWidget* createDropdownPage(QWidget* owner);
QWidget* createMenuPage(QWidget* owner);
QWidget* createPaginationPage(QWidget* owner);
QWidget* createStepsPage(QWidget* owner);

// 数据录入 (Data Entry)
QWidget* createAutoCompletePage(QWidget* owner);
QWidget* createCascaderPage(QWidget* owner);
QWidget* createCheckboxPage(QWidget* owner);
QWidget* createColorPickerPage(QWidget* owner);
QWidget* createDatePickerPage(QWidget* owner);
QWidget* createFormPage(QWidget* owner);
QWidget* createInputPage(QWidget* owner);
QWidget* createInputNumberPage(QWidget* owner);
QWidget* createMentionsPage(QWidget* owner);
QWidget* createRadioPage(QWidget* owner);
QWidget* createRatePage(QWidget* owner);
QWidget* createSelectPage(QWidget* owner);
QWidget* createSliderPage(QWidget* owner);
QWidget* createSwitchPage(QWidget* owner);
QWidget* createTimePickerPage(QWidget* owner);
QWidget* createTransferPage(QWidget* owner);
QWidget* createTreeSelectPage(QWidget* owner);
QWidget* createUploadPage(QWidget* owner);

// 数据展示 (Data Display)
QWidget* createAvatarPage(QWidget* owner);
QWidget* createBadgePage(QWidget* owner);
QWidget* createCalendarPage(QWidget* owner);
QWidget* createCardPage(QWidget* owner);
QWidget* createCarouselPage(QWidget* owner);
QWidget* createCollapsePage(QWidget* owner);
QWidget* createDescriptionsPage(QWidget* owner);
QWidget* createEmptyPage(QWidget* owner);
QWidget* createImagePage(QWidget* owner);
QWidget* createListPage(QWidget* owner);
QWidget* createPopoverPage(QWidget* owner);
QWidget* createQRCodePage(QWidget* owner);
QWidget* createSegmentedPage(QWidget* owner);
QWidget* createStatisticPage(QWidget* owner);
QWidget* createTablePage(QWidget* owner);
QWidget* createTabsPage(QWidget* owner);
QWidget* createTagPage(QWidget* owner);
QWidget* createTimelinePage(QWidget* owner);
QWidget* createTooltipPage(QWidget* owner);
QWidget* createTreePage(QWidget* owner);

// 反馈 (Feedback)
QWidget* createAlertPage(QWidget* owner);
QWidget* createDrawerPage(QWidget* owner);
QWidget* createMessagePage(QWidget* owner);
QWidget* createModalPage(QWidget* owner);
QWidget* createNotificationPage(QWidget* owner);
QWidget* createPopconfirmPage(QWidget* owner);
QWidget* createProgressPage(QWidget* owner);
QWidget* createResultPage(QWidget* owner);
QWidget* createSkeletonPage(QWidget* owner);
QWidget* createSpinPage(QWidget* owner);
QWidget* createTourPage(QWidget* owner);
QWidget* createWatermarkPage(QWidget* owner);

// 其他 (Other)
QWidget* createAppPage(QWidget* owner);
QWidget* createConfigProviderPage(QWidget* owner);
QWidget* createFloatButtonPage(QWidget* owner);

// Qt 扩展 (Qt Extensions)
QWidget* createDockWidgetPage(QWidget* owner);
QWidget* createLogPage(QWidget* owner);
QWidget* createMasonryPage(QWidget* owner);
QWidget* createMenuBarPage(QWidget* owner);
QWidget* createNavItemPage(QWidget* owner);
QWidget* createPlainTextEditPage(QWidget* owner);
QWidget* createScrollAreaPage(QWidget* owner);
QWidget* createScrollBarPage(QWidget* owner);
QWidget* createStatusBarPage(QWidget* owner);
QWidget* createToolBarPage(QWidget* owner);
QWidget* createToolButtonPage(QWidget* owner);
QWidget* createWidgetPage(QWidget* owner);
QWidget* createWindowPage(QWidget* owner);
}
