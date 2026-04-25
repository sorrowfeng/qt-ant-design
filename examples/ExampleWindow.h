#pragma once

#include "widgets/AntWindow.h"

class AntButton;
class AntTypography;
class QHBoxLayout;
class QLabel;
class QStackedWidget;
class QVBoxLayout;

class ExampleWindow : public AntWindow
{
    Q_OBJECT

public:
    explicit ExampleWindow(QWidget* parent = nullptr);

protected:
    QSize sizeHint() const override;

private:
    QWidget* createShowcasePage();
    QWidget* createAlertPage();
    QWidget* createButtonPage();
    QWidget* createAvatarPage();
    QWidget* createBadgePage();
    QWidget* createBreadcrumbPage();
    QWidget* createCheckboxPage();
    QWidget* createDatePickerPage();
    QWidget* createDescriptionsPage();
    QWidget* createDropdownPage();
    QWidget* createDividerPage();
    QWidget* createEmptyPage();
    QWidget* createFormPage();
    QWidget* createIconPage();
    QWidget* createInputPage();
    QWidget* createListPage();
    QWidget* createInputNumberPage();
    QWidget* createMessagePage();
    QWidget* createMenuPage();
    QWidget* createModalPage();
    QWidget* createNotificationPage();
    QWidget* createPopoverPage();
    QWidget* createPopconfirmPage();
    QWidget* createPaginationPage();
    QWidget* createProgressPage();
    QWidget* createRadioPage();
    QWidget* createRatePage();
    QWidget* createResultPage();
    QWidget* createSelectPage();
    QWidget* createSkeletonPage();
    QWidget* createSliderPage();
    QWidget* createSpinPage();
    QWidget* createStatisticPage();
    QWidget* createStepsPage();
    QWidget* createSwitchPage();
    QWidget* createTabsPage();
    QWidget* createTagPage();
    QWidget* createTooltipPage();
    QWidget* createTimePickerPage();
    QWidget* createCardPage();
    QWidget* createTimelinePage();
    QWidget* createSpacePage();
    QWidget* createLayoutPage();
    QWidget* createTypographyPage();
    QWidget* createTablePage();
    QWidget* createTreePage();
    QWidget* createUploadPage();
    QWidget* createCascaderPage();
    QWidget* createTreeSelectPage();
    QWidget* createWindowPage();
    QWidget* createDrawerPage();
    QWidget* createStatusBarPage();
    QWidget* createScrollBarPage();
    QWidget* createSegmentedPage();
    QWidget* createFloatButtonPage();
    QWidget* createWatermarkPage();
    QWidget* createQRCodePage();
    QWidget* createAffixPage();
    QWidget* createToolButtonPage();
    QWidget* createScrollAreaPage();
    QWidget* createPlainTextEditPage();
    QWidget* createMenuBarPage();
    QWidget* createToolBarPage();
    QWidget* createAutoCompletePage();
    QWidget* createCalendarPage();
    QWidget* createColorPickerPage();
    QWidget* createDockWidgetPage();
    QWidget* createAnchorPage();
    QWidget* createAppPage();
    QWidget* createCarouselPage();
    QWidget* createConfigProviderPage();
    QWidget* createFlexPage();
    QWidget* createGridPage();
    QWidget* createMasonryPage();
    QWidget* createMentionsPage();
    QWidget* createTourPage();
    QWidget* createTransferPage();
    QWidget* createImagePage();
    QWidget* createCollapsePage();
    QWidget* createSplitterPage();
    QWidget* createLogPage();
    QWidget* wrapPage(QWidget* page);
    AntTypography* createSectionTitle(const QString& title);
    void addNavButton(const QString& text, int pageIndex);
    void applyTheme();

    QWidget* m_central = nullptr;
    QWidget* m_sidebar = nullptr;
    QWidget* m_content = nullptr;
    QVBoxLayout* m_navLayout = nullptr;
    QStackedWidget* m_stack = nullptr;
    AntButton* m_themeButton = nullptr;
};
