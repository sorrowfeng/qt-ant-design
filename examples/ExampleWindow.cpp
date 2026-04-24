#include "ExampleWindow.h"

#include <QFrame>
#include <QDate>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTime>
#include <QVBoxLayout>

#include "core/AntTheme.h"
#include "widgets/AntWidget.h"
#include "widgets/AntAlert.h"
#include "widgets/AntAvatar.h"
#include "widgets/AntBadge.h"
#include "widgets/AntBreadcrumb.h"
#include "widgets/AntButton.h"
#include "widgets/AntCard.h"
#include "widgets/AntCheckbox.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntDescriptions.h"
#include "widgets/AntDropdown.h"
#include "widgets/AntDivider.h"
#include "widgets/AntEmpty.h"
#include "widgets/AntForm.h"
#include "widgets/AntIcon.h"
#include "widgets/AntInput.h"
#include "widgets/AntInputNumber.h"
#include "widgets/AntList.h"
#include "widgets/AntMessage.h"
#include "widgets/AntMenu.h"
#include "widgets/AntModal.h"
#include "widgets/AntNotification.h"
#include "widgets/AntPopover.h"
#include "widgets/AntPopconfirm.h"
#include "widgets/AntPagination.h"
#include "widgets/AntProgress.h"
#include "widgets/AntRadio.h"
#include "widgets/AntRate.h"
#include "widgets/AntResult.h"
#include "widgets/AntSelect.h"
#include "widgets/AntSkeleton.h"
#include "widgets/AntSlider.h"
#include "widgets/AntSpin.h"
#include "widgets/AntStatistic.h"
#include "widgets/AntSteps.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntTabs.h"
#include "widgets/AntTag.h"
#include "widgets/AntTooltip.h"
#include "widgets/AntTimePicker.h"
#include "widgets/AntTimeline.h"
#include "widgets/AntSpace.h"
#include "widgets/AntLayout.h"
#include "widgets/AntTypography.h"
#include "widgets/AntTable.h"
#include "widgets/AntTree.h"
#include "widgets/AntUpload.h"
#include "widgets/AntCascader.h"
#include "widgets/AntTreeSelect.h"
#include "widgets/AntWindow.h"
#include "widgets/AntDrawer.h"
#include "widgets/AntStatusBar.h"
#include "widgets/AntScrollBar.h"

ExampleWindow::ExampleWindow(QWidget* parent)
    : AntWindow(parent)
{
    setWindowTitle(QStringLiteral("Ant Design Qt Widgets"));

    m_central = new AntWidget(this);
    auto* root = new QHBoxLayout(m_central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_sidebar = new AntWidget(m_central);
    m_sidebar->setFixedWidth(220);
    auto* sideLayout = new QVBoxLayout(m_sidebar);
    sideLayout->setContentsMargins(20, 12, 20, 12);
    sideLayout->setSpacing(8);

    auto* brand = new AntTypography(QStringLiteral("qt-ant-design"), m_sidebar);
    brand->setTitle(true);
    brand->setTitleLevel(Ant::TypographyTitleLevel::H4);
    sideLayout->addWidget(brand);

    m_themeButton = new AntButton(QStringLiteral("Dark"), m_sidebar);
    m_themeButton->setButtonType(Ant::ButtonType::Default);
    m_themeButton->setButtonShape(Ant::ButtonShape::Round);
    m_themeButton->setButtonSize(Ant::ButtonSize::Small);
    connect(m_themeButton, &AntButton::clicked, antTheme, &AntTheme::toggleThemeMode);
    sideLayout->addWidget(m_themeButton);

    auto* navScroll = new QScrollArea(m_sidebar);
    navScroll->setWidgetResizable(true);
    navScroll->setFrameShape(QFrame::NoFrame);
    navScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    navScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    navScroll->setVerticalScrollBar(new AntScrollBar(Qt::Vertical));

    auto* navContainer = new AntWidget();
    m_navLayout = new QVBoxLayout(navContainer);
    m_navLayout->setContentsMargins(0, 0, 4, 0);
    m_navLayout->setSpacing(4);
    m_navLayout->addStretch();

    navScroll->setWidget(navContainer);
    sideLayout->addWidget(navScroll, 1);

    m_content = new AntWidget(m_central);
    auto* contentLayout = new QVBoxLayout(m_content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_stack = new QStackedWidget(m_content);
    contentLayout->addWidget(m_stack, 1);

    m_stack->addWidget(wrapPage(createButtonPage()));
    m_stack->addWidget(wrapPage(createBreadcrumbPage()));
    m_stack->addWidget(wrapPage(createCheckboxPage()));
    m_stack->addWidget(wrapPage(createDatePickerPage()));
    m_stack->addWidget(wrapPage(createDescriptionsPage()));
    m_stack->addWidget(wrapPage(createDropdownPage()));
    m_stack->addWidget(wrapPage(createInputPage()));
    m_stack->addWidget(wrapPage(createMessagePage()));
    m_stack->addWidget(wrapPage(createMenuPage()));
    m_stack->addWidget(wrapPage(createTabsPage()));
    m_stack->addWidget(wrapPage(createBadgePage()));
    m_stack->addWidget(wrapPage(createAvatarPage()));
    m_stack->addWidget(wrapPage(createTagPage()));
    m_stack->addWidget(wrapPage(createNotificationPage()));
    m_stack->addWidget(wrapPage(createPopoverPage()));
    m_stack->addWidget(wrapPage(createPopconfirmPage()));
    m_stack->addWidget(wrapPage(createModalPage()));
    m_stack->addWidget(wrapPage(createPaginationPage()));
    m_stack->addWidget(wrapPage(createProgressPage()));
    m_stack->addWidget(wrapPage(createRadioPage()));
    m_stack->addWidget(wrapPage(createSelectPage()));
    m_stack->addWidget(wrapPage(createSliderPage()));
    m_stack->addWidget(wrapPage(createSpinPage()));
    m_stack->addWidget(wrapPage(createStepsPage()));
    m_stack->addWidget(wrapPage(createSwitchPage()));
    m_stack->addWidget(wrapPage(createTimePickerPage()));
    m_stack->addWidget(wrapPage(createCardPage()));
    m_stack->addWidget(wrapPage(createSkeletonPage()));
    m_stack->addWidget(wrapPage(createDividerPage()));
    m_stack->addWidget(wrapPage(createIconPage()));
    m_stack->addWidget(wrapPage(createInputNumberPage()));
    m_stack->addWidget(wrapPage(createAlertPage()));
    m_stack->addWidget(wrapPage(createTooltipPage()));
    m_stack->addWidget(wrapPage(createFormPage()));
    m_stack->addWidget(wrapPage(createEmptyPage()));
    m_stack->addWidget(wrapPage(createResultPage()));
    m_stack->addWidget(wrapPage(createListPage()));
    m_stack->addWidget(wrapPage(createStatisticPage()));
    m_stack->addWidget(wrapPage(createTimelinePage()));
    m_stack->addWidget(wrapPage(createSpacePage()));
    m_stack->addWidget(wrapPage(createLayoutPage()));
    m_stack->addWidget(wrapPage(createTypographyPage()));
    m_stack->addWidget(wrapPage(createTablePage()));
    m_stack->addWidget(wrapPage(createTreePage()));
    m_stack->addWidget(wrapPage(createUploadPage()));
    m_stack->addWidget(wrapPage(createCascaderPage()));
    m_stack->addWidget(wrapPage(createTreeSelectPage()));
    m_stack->addWidget(wrapPage(createWindowPage()));
    m_stack->addWidget(wrapPage(createDrawerPage()));
    m_stack->addWidget(wrapPage(createStatusBarPage()));
    m_stack->addWidget(wrapPage(createScrollBarPage()));
    m_stack->addWidget(wrapPage(createRatePage()));
    addNavButton(QStringLiteral("Button"), 0);
    addNavButton(QStringLiteral("Breadcrumb"), 1);
    addNavButton(QStringLiteral("Checkbox"), 2);
    addNavButton(QStringLiteral("DatePicker"), 3);
    addNavButton(QStringLiteral("Descriptions"), 4);
    addNavButton(QStringLiteral("Dropdown"), 5);
    addNavButton(QStringLiteral("Input"), 6);
    addNavButton(QStringLiteral("Message"), 7);
    addNavButton(QStringLiteral("Menu"), 8);
    addNavButton(QStringLiteral("Tabs"), 9);
    addNavButton(QStringLiteral("Badge"), 10);
    addNavButton(QStringLiteral("Avatar"), 11);
    addNavButton(QStringLiteral("Tag"), 12);
    addNavButton(QStringLiteral("Notification"), 13);
    addNavButton(QStringLiteral("Popover"), 14);
    addNavButton(QStringLiteral("Popconfirm"), 15);
    addNavButton(QStringLiteral("Modal"), 16);
    addNavButton(QStringLiteral("Pagination"), 17);
    addNavButton(QStringLiteral("Progress"), 18);
    addNavButton(QStringLiteral("Radio"), 19);
    addNavButton(QStringLiteral("Select"), 20);
    addNavButton(QStringLiteral("Slider"), 21);
    addNavButton(QStringLiteral("Spin"), 22);
    addNavButton(QStringLiteral("Steps"), 23);
    addNavButton(QStringLiteral("Switch"), 24);
    addNavButton(QStringLiteral("TimePicker"), 25);
    addNavButton(QStringLiteral("Card"), 26);
    addNavButton(QStringLiteral("Skeleton"), 27);
    addNavButton(QStringLiteral("Divider"), 28);
    addNavButton(QStringLiteral("Icon"), 29);
    addNavButton(QStringLiteral("InputNumber"), 30);
    addNavButton(QStringLiteral("Alert"), 31);
    addNavButton(QStringLiteral("Tooltip"), 32);
    addNavButton(QStringLiteral("Form"), 33);
    addNavButton(QStringLiteral("Empty"), 34);
    addNavButton(QStringLiteral("Result"), 35);
    addNavButton(QStringLiteral("List"), 36);
    addNavButton(QStringLiteral("Statistic"), 37);
    addNavButton(QStringLiteral("Timeline"), 38);
    addNavButton(QStringLiteral("Space"), 39);
    addNavButton(QStringLiteral("Layout"), 40);
    addNavButton(QStringLiteral("Typography"), 41);
    addNavButton(QStringLiteral("Table"), 42);
    addNavButton(QStringLiteral("Tree"), 43);
    addNavButton(QStringLiteral("Upload"), 44);
    addNavButton(QStringLiteral("Cascader"), 45);
    addNavButton(QStringLiteral("TreeSelect"), 46);
    addNavButton(QStringLiteral("Window"), 47);
    addNavButton(QStringLiteral("Drawer"), 48);
    addNavButton(QStringLiteral("StatusBar"), 49);
    addNavButton(QStringLiteral("ScrollBar"), 50);
    addNavButton(QStringLiteral("Rate"), 51);

    root->addWidget(m_sidebar);
    root->addWidget(m_content, 1);
    setCentralWidget(m_central);

    connect(antTheme, &AntTheme::themeChanged, this, &ExampleWindow::applyTheme);
    applyTheme();
}

QWidget* ExampleWindow::createButtonPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Types")));
    auto* typeRow = new QHBoxLayout();
    typeRow->setSpacing(12);
    const QList<QPair<QString, Ant::ButtonType>> types = {
        {QStringLiteral("Primary"), Ant::ButtonType::Primary},
        {QStringLiteral("Default"), Ant::ButtonType::Default},
        {QStringLiteral("Dashed"), Ant::ButtonType::Dashed},
        {QStringLiteral("Text"), Ant::ButtonType::Text},
        {QStringLiteral("Link"), Ant::ButtonType::Link},
    };
    for (const auto& item : types)
    {
        auto* button = new AntButton(item.first);
        button->setButtonType(item.second);
        typeRow->addWidget(button);
    }
    typeRow->addStretch();
    layout->addLayout(typeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Size and Shape")));
    auto* shapeRow = new QHBoxLayout();
    shapeRow->setSpacing(12);
    auto* large = new AntButton(QStringLiteral("Large"));
    large->setButtonType(Ant::ButtonType::Primary);
    large->setButtonSize(Ant::ButtonSize::Large);
    auto* middle = new AntButton(QStringLiteral("Middle"));
    middle->setButtonType(Ant::ButtonType::Primary);
    auto* small = new AntButton(QStringLiteral("Small"));
    small->setButtonType(Ant::ButtonType::Primary);
    small->setButtonSize(Ant::ButtonSize::Small);
    auto* round = new AntButton(QStringLiteral("Round"));
    round->setButtonShape(Ant::ButtonShape::Round);
    auto* circle = new AntButton(QStringLiteral("A"));
    circle->setButtonShape(Ant::ButtonShape::Circle);
    shapeRow->addWidget(large);
    shapeRow->addWidget(middle);
    shapeRow->addWidget(small);
    shapeRow->addWidget(round);
    shapeRow->addWidget(circle);
    shapeRow->addStretch();
    layout->addLayout(shapeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("States")));
    auto* stateRow = new QHBoxLayout();
    stateRow->setSpacing(12);
    auto* danger = new AntButton(QStringLiteral("Danger"));
    danger->setDanger(true);
    auto* ghost = new AntButton(QStringLiteral("Ghost"));
    ghost->setButtonType(Ant::ButtonType::Primary);
    ghost->setGhost(true);
    auto* loading = new AntButton(QStringLiteral("Loading"));
    loading->setButtonType(Ant::ButtonType::Primary);
    loading->setLoading(true);
    auto* disabled = new AntButton(QStringLiteral("Disabled"));
    disabled->setEnabled(false);
    stateRow->addWidget(danger);
    stateRow->addWidget(ghost);
    stateRow->addWidget(loading);
    stateRow->addWidget(disabled);
    stateRow->addStretch();
    layout->addLayout(stateRow);

    auto* block = new AntButton(QStringLiteral("Block Button"));
    block->setButtonType(Ant::ButtonType::Primary);
    block->setBlock(true);
    layout->addWidget(block);
    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createBreadcrumbPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basic = new AntBreadcrumb();
    basic->addItem(QStringLiteral("Home"), QStringLiteral("/"));
    basic->addItem(QStringLiteral("Application Center"), QStringLiteral("/apps"));
    basic->addItem(QStringLiteral("Application List"), QStringLiteral("/apps/list"));
    basic->addItem(QStringLiteral("An Application"));
    connect(basic, &AntBreadcrumb::itemClicked, this, [this](int, const QString& title, const QString&) {
        AntMessage::info(QStringLiteral("Clicked %1").arg(title), this, 1600);
    });
    layout->addWidget(basic);

    layout->addWidget(createSectionTitle(QStringLiteral("Icon and Custom Separator")));
    auto* custom = new AntBreadcrumb();
    custom->setSeparator(QStringLiteral(">"));
    custom->addItem(QStringLiteral("Home"), QStringLiteral("/"), QStringLiteral("H"));
    custom->addItem(QStringLiteral("Library"), QStringLiteral("/library"), QStringLiteral("L"));
    custom->addItem(QStringLiteral("Data"));
    layout->addWidget(custom);

    layout->addWidget(createSectionTitle(QStringLiteral("Disabled")));
    auto* disabled = new AntBreadcrumb();
    disabled->addItem(QStringLiteral("Workspace"), QStringLiteral("/workspace"));
    disabled->addItem(QStringLiteral("Disabled link"), QStringLiteral("/disabled"), QString(), true);
    disabled->addItem(QStringLiteral("Current"));
    layout->addWidget(disabled);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createCheckboxPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(24);
    auto* unchecked = new AntCheckbox(QStringLiteral("Checkbox"));
    auto* checked = new AntCheckbox(QStringLiteral("Checked"));
    checked->setChecked(true);
    auto* indeterminate = new AntCheckbox(QStringLiteral("Indeterminate"));
    indeterminate->setIndeterminate(true);
    basicRow->addWidget(unchecked);
    basicRow->addWidget(checked);
    basicRow->addWidget(indeterminate);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Disabled")));
    auto* disabledRow = new QHBoxLayout();
    disabledRow->setSpacing(24);
    auto* disabled = new AntCheckbox(QStringLiteral("Disabled"));
    disabled->setEnabled(false);
    auto* disabledChecked = new AntCheckbox(QStringLiteral("Disabled checked"));
    disabledChecked->setChecked(true);
    disabledChecked->setEnabled(false);
    auto* disabledIndeterminate = new AntCheckbox(QStringLiteral("Disabled indeterminate"));
    disabledIndeterminate->setIndeterminate(true);
    disabledIndeterminate->setEnabled(false);
    disabledRow->addWidget(disabled);
    disabledRow->addWidget(disabledChecked);
    disabledRow->addWidget(disabledIndeterminate);
    disabledRow->addStretch();
    layout->addLayout(disabledRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Controlled")));
    auto* controlledRow = new QHBoxLayout();
    controlledRow->setSpacing(18);
    auto* controller = new AntCheckbox(QStringLiteral("Check all"));
    auto* optionA = new AntCheckbox(QStringLiteral("Apple"));
    auto* optionB = new AntCheckbox(QStringLiteral("Pear"));
    auto updateController = [controller, optionA, optionB]() {
        const int checkedCount = (optionA->isChecked() ? 1 : 0) + (optionB->isChecked() ? 1 : 0);
        controller->setIndeterminate(checkedCount == 1);
        controller->setChecked(checkedCount == 2);
    };
    connect(controller, &AntCheckbox::clicked, this, [controller, optionA, optionB]() {
        optionA->setChecked(controller->isChecked());
        optionB->setChecked(controller->isChecked());
    });
    connect(optionA, &AntCheckbox::checkedChanged, this, updateController);
    connect(optionB, &AntCheckbox::checkedChanged, this, updateController);
    controlledRow->addWidget(controller);
    controlledRow->addSpacing(12);
    controlledRow->addWidget(optionA);
    controlledRow->addWidget(optionB);
    controlledRow->addStretch();
    layout->addLayout(controlledRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createDatePickerPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(16);
    auto* basic = new AntDatePicker();
    basic->setPlaceholderText(QStringLiteral("Select date"));
    auto* selected = new AntDatePicker();
    selected->setSelectedDate(QDate::currentDate());
    selected->setAllowClear(true);
    auto* customFormat = new AntDatePicker();
    customFormat->setSelectedDate(QDate::currentDate().addDays(7));
    customFormat->setDisplayFormat(QStringLiteral("MMM d, yyyy"));
    basicRow->addWidget(basic);
    basicRow->addWidget(selected);
    basicRow->addWidget(customFormat);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Size")));
    auto* sizeRow = new QHBoxLayout();
    sizeRow->setSpacing(16);
    auto* large = new AntDatePicker();
    large->setPickerSize(Ant::DatePickerSize::Large);
    large->setSelectedDate(QDate::currentDate());
    auto* middle = new AntDatePicker();
    middle->setSelectedDate(QDate::currentDate());
    auto* small = new AntDatePicker();
    small->setPickerSize(Ant::DatePickerSize::Small);
    small->setSelectedDate(QDate::currentDate());
    sizeRow->addWidget(large);
    sizeRow->addWidget(middle);
    sizeRow->addWidget(small);
    sizeRow->addStretch();
    layout->addLayout(sizeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Status and Variant")));
    auto* variantRow = new QHBoxLayout();
    variantRow->setSpacing(16);
    auto* error = new AntDatePicker();
    error->setStatus(Ant::DatePickerStatus::Error);
    error->setPlaceholderText(QStringLiteral("Error"));
    auto* warning = new AntDatePicker();
    warning->setStatus(Ant::DatePickerStatus::Warning);
    warning->setPlaceholderText(QStringLiteral("Warning"));
    auto* filled = new AntDatePicker();
    filled->setVariant(Ant::DatePickerVariant::Filled);
    filled->setSelectedDate(QDate::currentDate());
    auto* underlined = new AntDatePicker();
    underlined->setVariant(Ant::DatePickerVariant::Underlined);
    underlined->setSelectedDate(QDate::currentDate());
    variantRow->addWidget(error);
    variantRow->addWidget(warning);
    variantRow->addWidget(filled);
    variantRow->addWidget(underlined);
    variantRow->addStretch();
    layout->addLayout(variantRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Disabled")));
    auto* stateRow = new QHBoxLayout();
    stateRow->setSpacing(16);
    auto* disabledEmpty = new AntDatePicker();
    disabledEmpty->setEnabled(false);
    auto* disabledValue = new AntDatePicker();
    disabledValue->setSelectedDate(QDate::currentDate());
    disabledValue->setEnabled(false);
    stateRow->addWidget(disabledEmpty);
    stateRow->addWidget(disabledValue);
    stateRow->addStretch();
    layout->addLayout(stateRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createDescriptionsPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(28);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basic = new AntDescriptions(page);
    basic->setTitle(QStringLiteral("User Info"));
    basic->setColumnCount(3);
    basic->addItem(QStringLiteral("Name"), QStringLiteral("Liam Parker"));
    basic->addItem(QStringLiteral("Status"), QStringLiteral("Active"));
    basic->addItem(QStringLiteral("Region"), QStringLiteral("Singapore"));
    basic->addItem(QStringLiteral("Address"), QStringLiteral("88 Market Street, Floor 12"), 2);
    basic->addItem(QStringLiteral("Remarks"), QStringLiteral("Owns release workflow and deployment sign-off."));
    layout->addWidget(basic);

    layout->addWidget(createSectionTitle(QStringLiteral("Bordered")));
    auto* bordered = new AntDescriptions(page);
    bordered->setTitle(QStringLiteral("Order Summary"));
    bordered->setExtra(QStringLiteral("Processing"));
    bordered->setBordered(true);
    bordered->setColumnCount(2);
    bordered->addItem(QStringLiteral("Order No."), QStringLiteral("#SO-2048"));
    bordered->addItem(QStringLiteral("Placed At"), QStringLiteral("2026-04-24 14:30"));
    bordered->addItem(QStringLiteral("Amount"), QStringLiteral("$ 2,499.00"));
    auto* tagItem = new AntDescriptionsItem(QStringLiteral("Priority"), QString(), bordered);
    auto* priority = new AntTag(QStringLiteral("High"), bordered);
    priority->setColor(QStringLiteral("#fa541c"));
    priority->setVariant(Ant::TagVariant::Solid);
    tagItem->setContentWidget(priority);
    bordered->addItem(tagItem);
    bordered->addItem(QStringLiteral("Notes"), QStringLiteral("Customer requested expedited shipping and proactive status updates."), 2);
    layout->addWidget(bordered);

    layout->addWidget(createSectionTitle(QStringLiteral("Vertical")));
    auto* vertical = new AntDescriptions(page);
    vertical->setTitle(QStringLiteral("Deployment Details"));
    vertical->setVertical(true);
    vertical->setBordered(true);
    vertical->setColumnCount(3);
    vertical->addItem(QStringLiteral("Environment"), QStringLiteral("Production"));
    vertical->addItem(QStringLiteral("Version"), QStringLiteral("v2.8.1"));
    vertical->addItem(QStringLiteral("Triggered By"), QStringLiteral("CI Pipeline"));
    vertical->addItem(QStringLiteral("Rollback Plan"), QStringLiteral("Automatic rollback after 5 minutes of elevated error rate."), 2);
    vertical->addItem(QStringLiteral("Approval"), QStringLiteral("Ready"));
    layout->addWidget(vertical);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createDropdownPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(16);

    auto* hoverButton = new AntButton(QStringLiteral("Hover Dropdown"));
    auto* hoverDropdown = new AntDropdown(page);
    hoverDropdown->setTarget(hoverButton);
    hoverDropdown->addItem(QStringLiteral("new"), QStringLiteral("New File"));
    hoverDropdown->addItem(QStringLiteral("open"), QStringLiteral("Open Recent"));
    hoverDropdown->addDivider();
    hoverDropdown->addItem(QStringLiteral("exit"), QStringLiteral("Exit"));
    connect(hoverDropdown, &AntDropdown::itemTriggered, this, [this](const QString& key) {
        AntMessage::info(QStringLiteral("Triggered %1").arg(key), this, 1400);
    });

    auto* clickButton = new AntButton(QStringLiteral("Click Dropdown"));
    clickButton->setButtonType(Ant::ButtonType::Primary);
    auto* clickDropdown = new AntDropdown(page);
    clickDropdown->setTrigger(Ant::DropdownTrigger::Click);
    clickDropdown->setArrowVisible(true);
    clickDropdown->setTarget(clickButton);
    clickDropdown->addItem(QStringLiteral("edit"), QStringLiteral("Edit"));
    clickDropdown->addItem(QStringLiteral("duplicate"), QStringLiteral("Duplicate"));
    clickDropdown->addItem(QStringLiteral("archive"), QStringLiteral("Archive"));

    basicRow->addWidget(hoverButton);
    basicRow->addWidget(clickButton);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Placement")));
    auto* placementRow = new QHBoxLayout();
    placementRow->setSpacing(16);

    auto* bottomLeftButton = new AntButton(QStringLiteral("BottomLeft"));
    auto* bottomLeftDropdown = new AntDropdown(page);
    bottomLeftDropdown->setTarget(bottomLeftButton);
    bottomLeftDropdown->setPlacement(Ant::DropdownPlacement::BottomLeft);
    bottomLeftDropdown->addItem(QStringLiteral("profile"), QStringLiteral("Profile"));
    bottomLeftDropdown->addItem(QStringLiteral("settings"), QStringLiteral("Settings"));

    auto* bottomRightButton = new AntButton(QStringLiteral("BottomRight"));
    auto* bottomRightDropdown = new AntDropdown(page);
    bottomRightDropdown->setTarget(bottomRightButton);
    bottomRightDropdown->setPlacement(Ant::DropdownPlacement::BottomRight);
    bottomRightDropdown->setArrowVisible(true);
    bottomRightDropdown->addItem(QStringLiteral("share"), QStringLiteral("Share"));
    bottomRightDropdown->addItem(QStringLiteral("export"), QStringLiteral("Export"));

    auto* topButton = new AntButton(QStringLiteral("Top"));
    auto* topDropdown = new AntDropdown(page);
    topDropdown->setTrigger(Ant::DropdownTrigger::Click);
    topDropdown->setTarget(topButton);
    topDropdown->setPlacement(Ant::DropdownPlacement::Top);
    topDropdown->addItem(QStringLiteral("refresh"), QStringLiteral("Refresh"));
    topDropdown->addItem(QStringLiteral("reload"), QStringLiteral("Reload All"));

    placementRow->addWidget(bottomLeftButton);
    placementRow->addWidget(bottomRightButton);
    placementRow->addWidget(topButton);
    placementRow->addStretch();
    layout->addLayout(placementRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Context Menu")));
    auto* contextHint = new AntTypography(QStringLiteral("Right click the area below to open a context dropdown."), page);
    layout->addWidget(contextHint);

    auto* contextArea = new QLabel(QStringLiteral("Context trigger area"), page);
    contextArea->setMinimumSize(260, 120);
    contextArea->setAlignment(Qt::AlignCenter);
    contextArea->setStyleSheet(QStringLiteral("QLabel { border: 1px dashed rgba(0,0,0,0.18); border-radius: 8px; }"));
    auto* contextDropdown = new AntDropdown(page);
    contextDropdown->setTrigger(Ant::DropdownTrigger::ContextMenu);
    contextDropdown->setTarget(contextArea);
    contextDropdown->addItem(QStringLiteral("copy"), QStringLiteral("Copy"));
    contextDropdown->addItem(QStringLiteral("paste"), QStringLiteral("Paste"));
    contextDropdown->addItem(QStringLiteral("rename"), QStringLiteral("Rename"));
    connect(contextDropdown, &AntDropdown::itemTriggered, this, [this](const QString& key) {
        AntMessage::success(QStringLiteral("Context action: %1").arg(key), this, 1400);
    });
    layout->addWidget(contextArea, 0, Qt::AlignLeft);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createInputPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(18);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* large = new AntInput();
    large->setPlaceholderText(QStringLiteral("Large input"));
    large->setInputSize(Ant::InputSize::Large);
    auto* middle = new AntInput();
    middle->setPlaceholderText(QStringLiteral("Middle input with clear"));
    middle->setAllowClear(true);
    auto* small = new AntInput();
    small->setPlaceholderText(QStringLiteral("Small input"));
    small->setInputSize(Ant::InputSize::Small);
    layout->addWidget(large);
    layout->addWidget(middle);
    layout->addWidget(small);

    layout->addWidget(createSectionTitle(QStringLiteral("Addon and Status")));
    auto* addon = new AntInput();
    addon->setAddonBefore(QStringLiteral("https://"));
    addon->setAddonAfter(QStringLiteral(".com"));
    addon->setText(QStringLiteral("ant.design"));
    auto* password = new AntInput();
    password->setPlaceholderText(QStringLiteral("Password"));
    password->setPasswordMode(true);
    auto* error = new AntInput();
    error->setText(QStringLiteral("Invalid value"));
    error->setStatus(Ant::InputStatus::Error);
    auto* disabled = new AntInput();
    disabled->setText(QStringLiteral("Disabled"));
    disabled->setEnabled(false);
    layout->addWidget(addon);
    layout->addWidget(password);
    layout->addWidget(error);
    layout->addWidget(disabled);
    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createIconPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    auto createIconBlock = [this](const QString& title, AntIcon* icon) {
        auto* block = new QWidget();
        auto* blockLayout = new QVBoxLayout(block);
        blockLayout->setContentsMargins(0, 0, 0, 0);
        blockLayout->setSpacing(8);
        blockLayout->addWidget(icon, 0, Qt::AlignHCenter);

        auto* label = new QLabel(title, block);
        label->setAlignment(Qt::AlignCenter);
        blockLayout->addWidget(label);
        return block;
    };

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(24);
    const QList<QPair<QString, Ant::IconType>> basics = {
        {QStringLiteral("Search"), Ant::IconType::Search},
        {QStringLiteral("Home"), Ant::IconType::Home},
        {QStringLiteral("User"), Ant::IconType::User},
        {QStringLiteral("Calendar"), Ant::IconType::Calendar},
        {QStringLiteral("Clock"), Ant::IconType::ClockCircle},
        {QStringLiteral("Star"), Ant::IconType::Star},
    };
    for (const auto& item : basics)
    {
        auto* icon = new AntIcon(item.second);
        icon->setIconSize(24);
        basicRow->addWidget(createIconBlock(item.first, icon));
    }
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Themes and Colors")));
    auto* themeRow = new QHBoxLayout();
    themeRow->setSpacing(24);
    auto* outlined = new AntIcon(Ant::IconType::Star);
    outlined->setIconSize(28);
    outlined->setIconTheme(Ant::IconTheme::Outlined);
    themeRow->addWidget(createIconBlock(QStringLiteral("Outlined"), outlined));

    auto* filled = new AntIcon(Ant::IconType::Star);
    filled->setIconSize(28);
    filled->setIconTheme(Ant::IconTheme::Filled);
    filled->setColor(antTheme->tokens().colorWarning);
    themeRow->addWidget(createIconBlock(QStringLiteral("Filled"), filled));

    auto* twoTone = new AntIcon(Ant::IconType::InfoCircle);
    twoTone->setIconSize(28);
    twoTone->setIconTheme(Ant::IconTheme::TwoTone);
    twoTone->setTwoToneColor(antTheme->tokens().colorPrimary);
    themeRow->addWidget(createIconBlock(QStringLiteral("TwoTone"), twoTone));

    auto* error = new AntIcon(Ant::IconType::CloseCircle);
    error->setIconSize(28);
    error->setIconTheme(Ant::IconTheme::Filled);
    error->setColor(antTheme->tokens().colorError);
    themeRow->addWidget(createIconBlock(QStringLiteral("Status"), error));
    themeRow->addStretch();
    layout->addLayout(themeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Rotate and Spin")));
    auto* motionRow = new QHBoxLayout();
    motionRow->setSpacing(24);
    auto* loading = new AntIcon(Ant::IconType::Loading);
    loading->setIconSize(28);
    loading->setColor(antTheme->tokens().colorPrimary);
    loading->setSpin(true);
    motionRow->addWidget(createIconBlock(QStringLiteral("Loading"), loading));

    auto* rotate90 = new AntIcon(Ant::IconType::Right);
    rotate90->setIconSize(28);
    rotate90->setRotate(90);
    motionRow->addWidget(createIconBlock(QStringLiteral("Rotate 90"), rotate90));

    auto* rotate180 = new AntIcon(Ant::IconType::Down);
    rotate180->setIconSize(28);
    rotate180->setRotate(180);
    motionRow->addWidget(createIconBlock(QStringLiteral("Rotate 180"), rotate180));
    motionRow->addStretch();
    layout->addLayout(motionRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Custom Path")));
    auto* customRow = new QHBoxLayout();
    customRow->setSpacing(24);
    auto* customIcon = new AntIcon();
    customIcon->setIconSize(30);
    customIcon->setIconTheme(Ant::IconTheme::TwoTone);
    customIcon->setTwoToneColor(QColor(QStringLiteral("#eb2f96")));
    QPainterPath heart;
    heart.moveTo(16, 28);
    heart.cubicTo(7, 22, 3, 16, 3, 10.5);
    heart.cubicTo(3, 6.5, 6.2, 4, 10, 4);
    heart.cubicTo(12.8, 4, 15, 5.4, 16, 7.6);
    heart.cubicTo(17, 5.4, 19.2, 4, 22, 4);
    heart.cubicTo(25.8, 4, 29, 6.5, 29, 10.5);
    heart.cubicTo(29, 16, 25, 22, 16, 28);
    QPainterPath shine;
    shine.addEllipse(QRectF(10, 8, 4, 4));
    customIcon->setCustomPath(heart, shine);
    customRow->addWidget(createIconBlock(QStringLiteral("Custom"), customIcon));

    auto* inlineText = new AntTypography(QStringLiteral("Use icons inside rows for status, action and navigation."), page);
    auto* inlineWrap = new QWidget(page);
    auto* inlineLayout = new QHBoxLayout(inlineWrap);
    inlineLayout->setContentsMargins(0, 0, 0, 0);
    inlineLayout->setSpacing(8);
    auto* checkIcon = new AntIcon(Ant::IconType::CheckCircle);
    checkIcon->setIconTheme(Ant::IconTheme::Filled);
    checkIcon->setColor(antTheme->tokens().colorSuccess);
    checkIcon->setIconSize(18);
    inlineLayout->addWidget(checkIcon);
    inlineLayout->addWidget(inlineText);
    inlineLayout->addStretch();
    customRow->addWidget(inlineWrap, 1);
    layout->addLayout(customRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createMessagePage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Types")));
    auto* typeRow = new QHBoxLayout();
    typeRow->setSpacing(12);
    auto* info = new AntButton(QStringLiteral("Info"));
    auto* success = new AntButton(QStringLiteral("Success"));
    auto* warning = new AntButton(QStringLiteral("Warning"));
    auto* error = new AntButton(QStringLiteral("Error"));
    auto* loading = new AntButton(QStringLiteral("Loading"));
    connect(info, &AntButton::clicked, this, [this]() { AntMessage::info(QStringLiteral("This is an info message"), this); });
    connect(success, &AntButton::clicked, this, [this]() { AntMessage::success(QStringLiteral("Saved successfully"), this); });
    connect(warning, &AntButton::clicked, this, [this]() { AntMessage::warning(QStringLiteral("Please check the warning"), this); });
    connect(error, &AntButton::clicked, this, [this]() { AntMessage::error(QStringLiteral("Something went wrong"), this); });
    connect(loading, &AntButton::clicked, this, [this]() { AntMessage::loading(QStringLiteral("Loading data..."), this, 2500); });
    typeRow->addWidget(info);
    typeRow->addWidget(success);
    typeRow->addWidget(warning);
    typeRow->addWidget(error);
    typeRow->addWidget(loading);
    typeRow->addStretch();
    layout->addLayout(typeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Duration")));
    auto* durationRow = new QHBoxLayout();
    durationRow->setSpacing(12);
    auto* shortMsg = new AntButton(QStringLiteral("1 second"));
    auto* stickyMsg = new AntButton(QStringLiteral("Manual close"));
    connect(shortMsg, &AntButton::clicked, this, [this]() { AntMessage::success(QStringLiteral("This closes quickly"), this, 1000); });
    connect(stickyMsg, &AntButton::clicked, this, [this]() { AntMessage::info(QStringLiteral("Click this message to close it"), this, 0); });
    durationRow->addWidget(shortMsg);
    durationRow->addWidget(stickyMsg);
    durationRow->addStretch();
    layout->addLayout(durationRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createMenuPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Inline")));
    auto* inlineRow = new QHBoxLayout();
    inlineRow->setSpacing(18);
    auto* inlineMenu = new AntMenu();
    inlineMenu->setMode(Ant::MenuMode::Inline);
    inlineMenu->setSelectedKey(QStringLiteral("dashboard"));
    inlineMenu->addItem(QStringLiteral("dashboard"), QStringLiteral("Dashboard"), QStringLiteral("D"));
    inlineMenu->addSubMenu(QStringLiteral("workspace"), QStringLiteral("Workspace"), QStringLiteral("W"));
    inlineMenu->addSubItem(QStringLiteral("workspace"), QStringLiteral("projects"), QStringLiteral("Projects"), QStringLiteral("P"));
    inlineMenu->addSubItem(QStringLiteral("workspace"), QStringLiteral("tasks"), QStringLiteral("Tasks"), QStringLiteral("T"), QStringLiteral("12"));
    inlineMenu->addDivider();
    inlineMenu->addItem(QStringLiteral("settings"), QStringLiteral("Settings"), QStringLiteral("S"));
    inlineMenu->setMinimumHeight(250);
    inlineRow->addWidget(inlineMenu);

    auto* collapsedMenu = new AntMenu();
    collapsedMenu->setMode(Ant::MenuMode::Inline);
    collapsedMenu->setInlineCollapsed(true);
    collapsedMenu->setSelectedKey(QStringLiteral("projects"));
    collapsedMenu->addItem(QStringLiteral("dashboard"), QStringLiteral("Dashboard"), QStringLiteral("D"));
    collapsedMenu->addItem(QStringLiteral("projects"), QStringLiteral("Projects"), QStringLiteral("P"));
    collapsedMenu->addItem(QStringLiteral("settings"), QStringLiteral("Settings"), QStringLiteral("S"));
    collapsedMenu->setFixedWidth(80);
    collapsedMenu->setMinimumHeight(250);
    inlineRow->addWidget(collapsedMenu);
    inlineRow->addStretch();
    layout->addLayout(inlineRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Horizontal")));
    auto* horizontal = new AntMenu();
    horizontal->setMode(Ant::MenuMode::Horizontal);
    horizontal->setSelectedKey(QStringLiteral("mail"));
    horizontal->addItem(QStringLiteral("mail"), QStringLiteral("Mail"), QStringLiteral("M"));
    horizontal->addItem(QStringLiteral("calendar"), QStringLiteral("Calendar"), QStringLiteral("C"));
    horizontal->addSubMenu(QStringLiteral("reports"), QStringLiteral("Reports"), QStringLiteral("R"));
    horizontal->addItem(QStringLiteral("disabled"), QStringLiteral("Disabled"), QStringLiteral("X"), QString(), true);
    horizontal->setMinimumHeight(48);
    layout->addWidget(horizontal);

    layout->addWidget(createSectionTitle(QStringLiteral("Dark and Danger")));
    auto* darkRow = new QHBoxLayout();
    darkRow->setSpacing(18);
    auto* darkMenu = new AntMenu();
    darkMenu->setMenuTheme(Ant::MenuTheme::Dark);
    darkMenu->setMode(Ant::MenuMode::Vertical);
    darkMenu->setSelectedKey(QStringLiteral("profile"));
    darkMenu->addItem(QStringLiteral("profile"), QStringLiteral("Profile"), QStringLiteral("P"));
    darkMenu->addItem(QStringLiteral("billing"), QStringLiteral("Billing"), QStringLiteral("B"));
    darkMenu->addItem(QStringLiteral("danger"), QStringLiteral("Danger zone"), QStringLiteral("!"), QString(), false, true);
    darkMenu->addItem(QStringLiteral("disabled-dark"), QStringLiteral("Disabled"), QStringLiteral("X"), QString(), true);
    darkMenu->setMinimumHeight(200);
    darkRow->addWidget(darkMenu);
    darkRow->addStretch();
    layout->addLayout(darkRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createNotificationPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Types")));
    auto* typeRow = new QHBoxLayout();
    typeRow->setSpacing(12);
    auto* info = new AntButton(QStringLiteral("Info"));
    auto* success = new AntButton(QStringLiteral("Success"));
    auto* warning = new AntButton(QStringLiteral("Warning"));
    auto* error = new AntButton(QStringLiteral("Error"));
    connect(info, &AntButton::clicked, this, [this]() {
        AntNotification::info(QStringLiteral("Notification"),
                              QStringLiteral("This is an information notification with a longer description."),
                              this);
    });
    connect(success, &AntButton::clicked, this, [this]() {
        AntNotification::success(QStringLiteral("Success"),
                                 QStringLiteral("The operation completed and the result is ready."),
                                 this);
    });
    connect(warning, &AntButton::clicked, this, [this]() {
        AntNotification::warning(QStringLiteral("Warning"),
                                 QStringLiteral("Please review the pending configuration before continuing."),
                                 this);
    });
    connect(error, &AntButton::clicked, this, [this]() {
        AntNotification::error(QStringLiteral("Error"),
                               QStringLiteral("The request failed. Check the connection and try again."),
                               this);
    });
    typeRow->addWidget(info);
    typeRow->addWidget(success);
    typeRow->addWidget(warning);
    typeRow->addWidget(error);
    typeRow->addStretch();
    layout->addLayout(typeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Placement")));
    auto* placementRow = new QHBoxLayout();
    placementRow->setSpacing(12);
    const QList<QPair<QString, Ant::NotificationPlacement>> placements = {
        {QStringLiteral("Top left"), Ant::NotificationPlacement::TopLeft},
        {QStringLiteral("Top"), Ant::NotificationPlacement::Top},
        {QStringLiteral("Top right"), Ant::NotificationPlacement::TopRight},
        {QStringLiteral("Bottom left"), Ant::NotificationPlacement::BottomLeft},
        {QStringLiteral("Bottom"), Ant::NotificationPlacement::Bottom},
        {QStringLiteral("Bottom right"), Ant::NotificationPlacement::BottomRight},
    };
    for (const auto& item : placements)
    {
        auto* button = new AntButton(item.first);
        connect(button, &AntButton::clicked, this, [this, item]() {
            AntNotification::info(item.first,
                                  QStringLiteral("Placement follows the Ant Design notification API."),
                                  this,
                                  3000,
                                  item.second);
        });
        placementRow->addWidget(button);
    }
    placementRow->addStretch();
    layout->addLayout(placementRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Duration and Progress")));
    auto* durationRow = new QHBoxLayout();
    durationRow->setSpacing(12);
    auto* progress = new AntButton(QStringLiteral("Progress"));
    auto* sticky = new AntButton(QStringLiteral("Manual close"));
    auto* closeAll = new AntButton(QStringLiteral("Close all"));
    connect(progress, &AntButton::clicked, this, [this]() {
        auto* notification = AntNotification::success(QStringLiteral("Uploading"),
                                                      QStringLiteral("Progress bar shows the remaining display time."),
                                                      this,
                                                      5000);
        notification->setShowProgress(true);
    });
    connect(sticky, &AntButton::clicked, this, [this]() {
        AntNotification::warning(QStringLiteral("Manual close"),
                                 QStringLiteral("This notification will stay until the close button is clicked."),
                                 this,
                                 0);
    });
    connect(closeAll, &AntButton::clicked, this, []() { AntNotification::closeAll(); });
    durationRow->addWidget(progress);
    durationRow->addWidget(sticky);
    durationRow->addWidget(closeAll);
    durationRow->addStretch();
    layout->addLayout(durationRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createTabsPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    auto makePane = [](const QString& text) {
        auto* pane = new QWidget();
        auto* paneLayout = new QVBoxLayout(pane);
        paneLayout->setContentsMargins(16, 16, 16, 16);
        auto* label = new AntTypography(text);
        label->setParagraph(true);
        paneLayout->addWidget(label);
        paneLayout->addStretch();
        return pane;
    };

    layout->addWidget(createSectionTitle(QStringLiteral("Line")));
    auto* lineTabs = new AntTabs();
    lineTabs->setMinimumHeight(180);
    lineTabs->addTab(makePane(QStringLiteral("Content of Tab 1")), QStringLiteral("tab1"), QStringLiteral("Tab 1"));
    lineTabs->addTab(makePane(QStringLiteral("Content of Tab 2")), QStringLiteral("tab2"), QStringLiteral("Tab 2"));
    lineTabs->addTab(makePane(QStringLiteral("Disabled tab content")), QStringLiteral("disabled"), QStringLiteral("Disabled"), QString(), true);
    lineTabs->setActiveKey(QStringLiteral("tab1"));
    layout->addWidget(lineTabs);

    layout->addWidget(createSectionTitle(QStringLiteral("Centered and Placement")));
    auto* placementRow = new QHBoxLayout();
    placementRow->setSpacing(18);
    auto* centered = new AntTabs();
    centered->setCentered(true);
    centered->setTabsSize(Ant::TabsSize::Large);
    centered->setMinimumSize(360, 160);
    centered->addTab(makePane(QStringLiteral("Centered large tabs")), QStringLiteral("overview"), QStringLiteral("Overview"), QStringLiteral("O"));
    centered->addTab(makePane(QStringLiteral("Reports content")), QStringLiteral("reports"), QStringLiteral("Reports"), QStringLiteral("R"));
    centered->setActiveKey(QStringLiteral("overview"));
    placementRow->addWidget(centered, 1);

    auto* leftTabs = new AntTabs();
    leftTabs->setTabPlacement(Ant::TabsPlacement::Left);
    leftTabs->setMinimumSize(360, 180);
    leftTabs->addTab(makePane(QStringLiteral("Left placement keeps tabs on the start side.")), QStringLiteral("left-a"), QStringLiteral("Alpha"));
    leftTabs->addTab(makePane(QStringLiteral("Second left tab.")), QStringLiteral("left-b"), QStringLiteral("Beta"));
    leftTabs->setActiveKey(QStringLiteral("left-a"));
    placementRow->addWidget(leftTabs, 1);
    layout->addLayout(placementRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Card and Editable")));
    auto* cardRow = new QHBoxLayout();
    cardRow->setSpacing(18);
    auto* cardTabs = new AntTabs();
    cardTabs->setTabsType(Ant::TabsType::Card);
    cardTabs->setTabsSize(Ant::TabsSize::Small);
    cardTabs->setMinimumSize(360, 160);
    cardTabs->addTab(makePane(QStringLiteral("Small card tab")), QStringLiteral("card-a"), QStringLiteral("Card A"));
    cardTabs->addTab(makePane(QStringLiteral("Another card tab")), QStringLiteral("card-b"), QStringLiteral("Card B"));
    cardTabs->setActiveKey(QStringLiteral("card-a"));
    cardRow->addWidget(cardTabs, 1);

    auto* editableTabs = new AntTabs();
    editableTabs->setTabsType(Ant::TabsType::EditableCard);
    editableTabs->setMinimumSize(360, 160);
    editableTabs->addTab(makePane(QStringLiteral("Closable editable tab")), QStringLiteral("edit-a"), QStringLiteral("Editable A"));
    editableTabs->addTab(makePane(QStringLiteral("This tab cannot be closed")), QStringLiteral("edit-b"), QStringLiteral("Fixed"), QString(), false, false);
    editableTabs->setActiveKey(QStringLiteral("edit-a"));
    connect(editableTabs, &AntTabs::tabAddRequested, this, [editableTabs, makePane]() {
        const QString key = QStringLiteral("new-%1").arg(QDateTime::currentMSecsSinceEpoch());
        editableTabs->addTab(makePane(QStringLiteral("New editable tab")), key, QStringLiteral("New Tab"));
        editableTabs->setActiveKey(key);
    });
    cardRow->addWidget(editableTabs, 1);
    layout->addLayout(cardRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createPopoverPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(16);

    auto* hoverButton = new AntButton(QStringLiteral("Hover Popover"));
    auto* hoverPopover = new AntPopover(page);
    hoverPopover->setTarget(hoverButton);
    hoverPopover->setTitle(QStringLiteral("Profile Card"));
    hoverPopover->setContent(QStringLiteral("View quick details, recent activity and common shortcuts from this floating panel."));

    auto* clickButton = new AntButton(QStringLiteral("Click Popover"));
    clickButton->setButtonType(Ant::ButtonType::Primary);
    auto* clickPopover = new AntPopover(page);
    clickPopover->setTrigger(Ant::PopoverTrigger::Click);
    clickPopover->setTarget(clickButton);
    clickPopover->setTitle(QStringLiteral("Publish Changes"));
    clickPopover->setContent(QStringLiteral("Review your draft, run checks and publish when everything looks correct."));
    auto* publishAction = new AntButton(QStringLiteral("Run Checks"));
    publishAction->setButtonType(Ant::ButtonType::Link);
    connect(publishAction, &AntButton::clicked, this, [this]() {
        AntMessage::info(QStringLiteral("Running checks..."), this, 1400);
    });
    clickPopover->setActionWidget(publishAction);

    basicRow->addWidget(hoverButton);
    basicRow->addWidget(clickButton);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Placement")));
    auto* placementRow = new QHBoxLayout();
    placementRow->setSpacing(16);

    auto* topButton = new AntButton(QStringLiteral("Top"));
    auto* topPopover = new AntPopover(page);
    topPopover->setTarget(topButton);
    topPopover->setPlacement(Ant::TooltipPlacement::Top);
    topPopover->setTitle(QStringLiteral("Top Placement"));
    topPopover->setContent(QStringLiteral("Popover appears above the trigger target."));

    auto* rightButton = new AntButton(QStringLiteral("Right"));
    auto* rightPopover = new AntPopover(page);
    rightPopover->setTarget(rightButton);
    rightPopover->setPlacement(Ant::TooltipPlacement::Right);
    rightPopover->setTitle(QStringLiteral("Right Placement"));
    rightPopover->setContent(QStringLiteral("Useful when horizontal space is available."));

    auto* bottomLeftButton = new AntButton(QStringLiteral("BottomLeft"));
    auto* bottomLeftPopover = new AntPopover(page);
    bottomLeftPopover->setTarget(bottomLeftButton);
    bottomLeftPopover->setPlacement(Ant::TooltipPlacement::BottomLeft);
    bottomLeftPopover->setTitle(QStringLiteral("Bottom Left"));
    bottomLeftPopover->setContent(QStringLiteral("Aligned to the left edge of the trigger."));

    placementRow->addWidget(topButton);
    placementRow->addWidget(rightButton);
    placementRow->addWidget(bottomLeftButton);
    placementRow->addStretch();
    layout->addLayout(placementRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Arrow and Content")));
    auto* customRow = new QHBoxLayout();
    customRow->setSpacing(16);

    auto* noArrowButton = new AntButton(QStringLiteral("No Arrow"));
    auto* noArrowPopover = new AntPopover(page);
    noArrowPopover->setTarget(noArrowButton);
    noArrowPopover->setArrowVisible(false);
    noArrowPopover->setTitle(QStringLiteral("Minimal Card"));
    noArrowPopover->setContent(QStringLiteral("Use this when you want a cleaner floating panel without a directional arrow."));

    auto* longTextButton = new AntButton(QStringLiteral("Long Content"));
    auto* longTextPopover = new AntPopover(page);
    longTextPopover->setTrigger(Ant::PopoverTrigger::Click);
    longTextPopover->setTarget(longTextButton);
    longTextPopover->setTitle(QStringLiteral("Release Notes"));
    longTextPopover->setContent(QStringLiteral("This build includes a refreshed icon set, improved input number styling, and new overlay components including alert, tooltip, dropdown and popover."));

    customRow->addWidget(noArrowButton);
    customRow->addWidget(longTextButton);
    customRow->addStretch();
    layout->addLayout(customRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createPopconfirmPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(16);

    auto* deleteButton = new AntButton(QStringLiteral("Delete"));
    deleteButton->setDanger(true);
    auto* deleteConfirm = new AntPopconfirm(page);
    deleteConfirm->setTarget(deleteButton);
    deleteConfirm->setTitle(QStringLiteral("Delete this record?"));
    deleteConfirm->setDescription(QStringLiteral("This operation cannot be undone."));
    connect(deleteConfirm, &AntPopconfirm::confirmRequested, this, [this]() {
        AntMessage::success(QStringLiteral("Deleted"), this, 1400);
    });
    connect(deleteConfirm, &AntPopconfirm::cancelRequested, this, [this]() {
        AntMessage::info(QStringLiteral("Canceled"), this, 1400);
    });

    auto* archiveButton = new AntButton(QStringLiteral("Archive"));
    auto* archiveConfirm = new AntPopconfirm(page);
    archiveConfirm->setTarget(archiveButton);
    archiveConfirm->setTitle(QStringLiteral("Archive item?"));
    archiveConfirm->setDescription(QStringLiteral("You can restore it later from the archive list."));
    archiveConfirm->setOkText(QStringLiteral("Archive"));
    archiveConfirm->setCancelText(QStringLiteral("Keep"));

    basicRow->addWidget(deleteButton);
    basicRow->addWidget(archiveButton);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Placement")));
    auto* placementRow = new QHBoxLayout();
    placementRow->setSpacing(16);

    auto* topButton = new AntButton(QStringLiteral("Top"));
    auto* topConfirm = new AntPopconfirm(page);
    topConfirm->setTarget(topButton);
    topConfirm->setPlacement(Ant::TooltipPlacement::Top);
    topConfirm->setTitle(QStringLiteral("Publish changes now?"));

    auto* rightButton = new AntButton(QStringLiteral("Right"));
    auto* rightConfirm = new AntPopconfirm(page);
    rightConfirm->setTarget(rightButton);
    rightConfirm->setPlacement(Ant::TooltipPlacement::Right);
    rightConfirm->setTitle(QStringLiteral("Move to next step?"));

    auto* bottomLeftButton = new AntButton(QStringLiteral("BottomLeft"));
    auto* bottomLeftConfirm = new AntPopconfirm(page);
    bottomLeftConfirm->setTarget(bottomLeftButton);
    bottomLeftConfirm->setPlacement(Ant::TooltipPlacement::BottomLeft);
    bottomLeftConfirm->setTitle(QStringLiteral("Sign out this device?"));

    placementRow->addWidget(topButton);
    placementRow->addWidget(rightButton);
    placementRow->addWidget(bottomLeftButton);
    placementRow->addStretch();
    layout->addLayout(placementRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Variants")));
    auto* variantRow = new QHBoxLayout();
    variantRow->setSpacing(16);

    auto* minimalButton = new AntButton(QStringLiteral("No Cancel"));
    auto* minimalConfirm = new AntPopconfirm(page);
    minimalConfirm->setTarget(minimalButton);
    minimalConfirm->setTitle(QStringLiteral("Proceed with sync?"));
    minimalConfirm->setShowCancel(false);
    minimalConfirm->setOkText(QStringLiteral("Continue"));

    auto* disabledButton = new AntButton(QStringLiteral("Disabled"));
    auto* disabledConfirm = new AntPopconfirm(page);
    disabledConfirm->setTarget(disabledButton);
    disabledConfirm->setTitle(QStringLiteral("This should not open"));
    disabledConfirm->setDisabled(true);

    variantRow->addWidget(minimalButton);
    variantRow->addWidget(disabledButton);
    variantRow->addStretch();
    layout->addLayout(variantRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createModalPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(16);

    auto* openBasic = new AntButton(QStringLiteral("Open Basic Modal"));
    openBasic->setButtonType(Ant::ButtonType::Primary);
    auto* basicModal = new AntModal(this);
    basicModal->setTitle(QStringLiteral("Delete current draft?"));
    basicModal->setContent(QStringLiteral("This action will remove the local draft and clear the pending review notes."));
    basicModal->setOkText(QStringLiteral("Delete"));
    connect(openBasic, &AntButton::clicked, this, [basicModal]() { basicModal->setOpen(true); });
    connect(basicModal, &AntModal::confirmed, this, [this]() {
        AntMessage::success(QStringLiteral("Draft deleted"), this, 1500);
    });
    connect(basicModal, &AntModal::canceled, this, [this]() {
        AntMessage::info(QStringLiteral("Deletion canceled"), this, 1500);
    });

    auto* openNotice = new AntButton(QStringLiteral("Top Offset"));
    auto* noticeModal = new AntModal(this);
    noticeModal->setTitle(QStringLiteral("Publish release notes"));
    noticeModal->setContent(QStringLiteral("This modal uses a non-centered layout, which is handy for workflows that need more context below."));
    noticeModal->setCentered(false);
    noticeModal->setOkText(QStringLiteral("Publish"));
    connect(openNotice, &AntButton::clicked, this, [noticeModal]() { noticeModal->setOpen(true); });

    basicRow->addWidget(openBasic);
    basicRow->addWidget(openNotice);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Custom Content")));
    auto* customRow = new QHBoxLayout();
    customRow->setSpacing(16);

    auto* inviteButton = new AntButton(QStringLiteral("Invite Teammate"));
    auto* inviteModal = new AntModal(this);
    inviteModal->setTitle(QStringLiteral("Invite a teammate"));
    inviteModal->setOkText(QStringLiteral("Send Invite"));
    inviteModal->setDialogWidth(560);

    auto* inviteContent = new QWidget(inviteModal);
    auto* inviteLayout = new QVBoxLayout(inviteContent);
    inviteLayout->setContentsMargins(0, 0, 0, 0);
    inviteLayout->setSpacing(12);
    auto* inviteHint = new AntTypography(QStringLiteral("Share access with someone who can help review and ship this change."), inviteContent);
    inviteHint->setParagraph(true);
    auto* nameInput = new AntInput(inviteContent);
    nameInput->setPlaceholderText(QStringLiteral("Name"));
    auto* mailInput = new AntInput(inviteContent);
    mailInput->setPlaceholderText(QStringLiteral("Email"));
    inviteLayout->addWidget(inviteHint);
    inviteLayout->addWidget(nameInput);
    inviteLayout->addWidget(mailInput);
    inviteModal->setContentWidget(inviteContent);
    connect(inviteButton, &AntButton::clicked, this, [inviteModal]() { inviteModal->setOpen(true); });
    connect(inviteModal, &AntModal::confirmed, this, [this, nameInput]() {
        const QString name = nameInput->text().trimmed().isEmpty() ? QStringLiteral("teammate") : nameInput->text().trimmed();
        AntNotification::success(QStringLiteral("Invitation queued"),
                                 QStringLiteral("Invite has been prepared for %1.").arg(name),
                                 this,
                                 2200);
    });

    auto* customFooterButton = new AntButton(QStringLiteral("Custom Footer"));
    auto* customFooterModal = new AntModal(this);
    customFooterModal->setTitle(QStringLiteral("Upgrade storage plan"));
    customFooterModal->setContent(QStringLiteral("Your current workspace is close to its attachment limit. Choose how you'd like to continue."));
    customFooterModal->setDialogWidth(560);
    auto* footer = new QWidget(customFooterModal);
    auto* footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->setSpacing(12);
    auto* remindLater = new AntButton(QStringLiteral("Remind Me Later"), footer);
    auto* upgradeNow = new AntButton(QStringLiteral("Upgrade Now"), footer);
    upgradeNow->setButtonType(Ant::ButtonType::Primary);
    footerLayout->addStretch();
    footerLayout->addWidget(remindLater);
    footerLayout->addWidget(upgradeNow);
    customFooterModal->setFooterWidget(footer);
    connect(remindLater, &AntButton::clicked, this, [this, customFooterModal]() {
        customFooterModal->setOpen(false);
        AntMessage::info(QStringLiteral("We will remind you next week"), this, 1600);
    });
    connect(upgradeNow, &AntButton::clicked, this, [this, customFooterModal]() {
        customFooterModal->setOpen(false);
        AntMessage::success(QStringLiteral("Upgrade flow started"), this, 1600);
    });
    connect(customFooterButton, &AntButton::clicked, this, [customFooterModal]() { customFooterModal->setOpen(true); });

    customRow->addWidget(inviteButton);
    customRow->addWidget(customFooterButton);
    customRow->addStretch();
    layout->addLayout(customRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Behavior")));
    auto* behaviorRow = new QHBoxLayout();
    behaviorRow->setSpacing(16);

    auto* protectedButton = new AntButton(QStringLiteral("Protected Action"));
    protectedButton->setDanger(true);
    auto* protectedModal = new AntModal(this);
    protectedModal->setTitle(QStringLiteral("Stop background deployment?"));
    protectedModal->setContent(QStringLiteral("Mask click and close icon are disabled here, so users must make an explicit decision."));
    protectedModal->setMaskClosable(false);
    protectedModal->setClosable(false);
    protectedModal->setOkText(QStringLiteral("Stop Now"));
    protectedModal->setCancelText(QStringLiteral("Keep Running"));
    connect(protectedButton, &AntButton::clicked, this, [protectedModal]() { protectedModal->setOpen(true); });
    connect(protectedModal, &AntModal::confirmed, this, [this]() {
        AntNotification::warning(QStringLiteral("Deployment stopped"),
                                 QStringLiteral("The running deployment has been interrupted."),
                                 this,
                                 2200);
    });

    auto* soloButton = new AntButton(QStringLiteral("Single Action"));
    auto* soloModal = new AntModal(this);
    soloModal->setTitle(QStringLiteral("Session will expire soon"));
    soloModal->setContent(QStringLiteral("There has been no activity for a while. Continue now to keep the workspace active."));
    soloModal->setShowCancel(false);
    soloModal->setOkText(QStringLiteral("Continue Session"));
    connect(soloButton, &AntButton::clicked, this, [soloModal]() { soloModal->setOpen(true); });

    behaviorRow->addWidget(protectedButton);
    behaviorRow->addWidget(soloButton);
    behaviorRow->addStretch();
    layout->addLayout(behaviorRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createPaginationPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basic = new AntPagination();
    basic->setTotal(120);
    basic->setCurrent(3);
    basic->setShowTotal(true);
    layout->addWidget(basic);

    layout->addWidget(createSectionTitle(QStringLiteral("Size and Less Items")));
    auto* sizeRow = new QVBoxLayout();
    sizeRow->setSpacing(12);
    auto* large = new AntPagination();
    large->setPaginationSize(Ant::PaginationSize::Large);
    large->setTotal(260);
    large->setCurrent(8);
    auto* small = new AntPagination();
    small->setPaginationSize(Ant::PaginationSize::Small);
    small->setTotal(260);
    small->setCurrent(8);
    small->setShowLessItems(true);
    sizeRow->addWidget(large);
    sizeRow->addWidget(small);
    layout->addLayout(sizeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Simple and Options")));
    auto* optionRow = new QVBoxLayout();
    optionRow->setSpacing(12);
    auto* simple = new AntPagination();
    simple->setSimple(true);
    simple->setTotal(90);
    simple->setCurrent(4);
    auto* options = new AntPagination();
    options->setTotal(320);
    options->setCurrent(6);
    options->setShowSizeChanger(true);
    options->setShowQuickJumper(true);
    auto* disabled = new AntPagination();
    disabled->setTotal(80);
    disabled->setCurrent(2);
    disabled->setDisabled(true);
    optionRow->addWidget(simple);
    optionRow->addWidget(options);
    optionRow->addWidget(disabled);
    layout->addLayout(optionRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createBadgePage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    auto makeAnchor = [](const QString& text) {
        auto* button = new AntButton(text);
        button->setButtonType(Ant::ButtonType::Default);
        button->setFixedSize(86, 40);
        return button;
    };

    layout->addWidget(createSectionTitle(QStringLiteral("Count")));
    auto* countRow = new QHBoxLayout();
    countRow->setSpacing(28);

    auto* basic = new AntBadge(5);
    basic->setContentWidget(makeAnchor(QStringLiteral("Inbox")));
    countRow->addWidget(basic);

    auto* zero = new AntBadge(0);
    zero->setShowZero(true);
    zero->setContentWidget(makeAnchor(QStringLiteral("Zero")));
    countRow->addWidget(zero);

    auto* overflow = new AntBadge(120);
    overflow->setOverflowCount(99);
    overflow->setContentWidget(makeAnchor(QStringLiteral("Tasks")));
    countRow->addWidget(overflow);
    countRow->addStretch();
    layout->addLayout(countRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Dot and Size")));
    auto* dotRow = new QHBoxLayout();
    dotRow->setSpacing(28);

    auto* dot = new AntBadge();
    dot->setDot(true);
    dot->setContentWidget(makeAnchor(QStringLiteral("Notice")));
    dotRow->addWidget(dot);

    auto* small = new AntBadge(8);
    small->setBadgeSize(Ant::BadgeSize::Small);
    small->setContentWidget(makeAnchor(QStringLiteral("Small")));
    dotRow->addWidget(small);

    auto* color = new AntBadge(3);
    color->setColor(QStringLiteral("success"));
    color->setOffset(QPoint(-8, 6));
    color->setContentWidget(makeAnchor(QStringLiteral("Offset")));
    dotRow->addWidget(color);
    dotRow->addStretch();
    layout->addLayout(dotRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Status")));
    auto* statusCol = new QVBoxLayout();
    statusCol->setSpacing(10);
    const QList<QPair<QString, Ant::BadgeStatus>> statuses = {
        {QStringLiteral("Success"), Ant::BadgeStatus::Success},
        {QStringLiteral("Processing"), Ant::BadgeStatus::Processing},
        {QStringLiteral("Default"), Ant::BadgeStatus::Default},
        {QStringLiteral("Error"), Ant::BadgeStatus::Error},
        {QStringLiteral("Warning"), Ant::BadgeStatus::Warning},
    };
    for (const auto& item : statuses)
    {
        auto* badge = new AntBadge();
        badge->setStatus(item.second);
        badge->setText(item.first);
        statusCol->addWidget(badge);
    }
    layout->addLayout(statusCol);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createAvatarPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(12);
    basicRow->addWidget(new AntAvatar(QStringLiteral("U")));
    auto* icon = new AntAvatar();
    icon->setIconText(QStringLiteral("@"));
    basicRow->addWidget(icon);
    auto* square = new AntAvatar(QStringLiteral("AD"));
    square->setShape(Ant::AvatarShape::Square);
    basicRow->addWidget(square);
    auto* custom = new AntAvatar(QStringLiteral("Qt"));
    custom->setCustomSize(48);
    basicRow->addWidget(custom);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Size")));
    auto* sizeRow = new QHBoxLayout();
    sizeRow->setSpacing(12);
    auto* large = new AntAvatar(QStringLiteral("L"));
    large->setAvatarSize(Ant::AvatarSize::Large);
    auto* middle = new AntAvatar(QStringLiteral("M"));
    middle->setAvatarSize(Ant::AvatarSize::Middle);
    auto* small = new AntAvatar(QStringLiteral("S"));
    small->setAvatarSize(Ant::AvatarSize::Small);
    sizeRow->addWidget(large);
    sizeRow->addWidget(middle);
    sizeRow->addWidget(small);
    sizeRow->addStretch();
    layout->addLayout(sizeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Autoset Font Size")));
    auto* textRow = new QHBoxLayout();
    textRow->setSpacing(12);
    textRow->addWidget(new AntAvatar(QStringLiteral("USER")));
    auto* longName = new AntAvatar(QStringLiteral("DESIGN"));
    longName->setCustomSize(56);
    longName->setGap(6);
    textRow->addWidget(longName);
    auto* tinyGap = new AntAvatar(QStringLiteral("ANTD"));
    tinyGap->setGap(2);
    textRow->addWidget(tinyGap);
    textRow->addStretch();
    layout->addLayout(textRow);

    layout->addWidget(createSectionTitle(QStringLiteral("With Badge")));
    auto* badgeRow = new QHBoxLayout();
    badgeRow->setSpacing(28);
    auto* countBadge = new AntBadge(3);
    countBadge->setContentWidget(new AntAvatar(QStringLiteral("A")));
    badgeRow->addWidget(countBadge);
    auto* dotBadge = new AntBadge();
    dotBadge->setDot(true);
    auto* dotAvatar = new AntAvatar(QStringLiteral("B"));
    dotAvatar->setShape(Ant::AvatarShape::Square);
    dotBadge->setContentWidget(dotAvatar);
    badgeRow->addWidget(dotBadge);
    badgeRow->addStretch();
    layout->addLayout(badgeRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createTagPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(10);
    basicRow->addWidget(new AntTag(QStringLiteral("Tag")));
    auto* closable = new AntTag(QStringLiteral("Closable"));
    closable->setClosable(true);
    auto* icon = new AntTag(QStringLiteral("Icon"));
    icon->setIconText(QStringLiteral("I"));
    basicRow->addWidget(closable);
    basicRow->addWidget(icon);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Color and Variant")));
    auto* colorRow = new QHBoxLayout();
    colorRow->setSpacing(10);
    const QList<QPair<QString, QString>> colors = {
        {QStringLiteral("Success"), QStringLiteral("success")},
        {QStringLiteral("Processing"), QStringLiteral("processing")},
        {QStringLiteral("Warning"), QStringLiteral("warning")},
        {QStringLiteral("Error"), QStringLiteral("error")},
        {QStringLiteral("Magenta"), QStringLiteral("#eb2f96")},
    };
    for (const auto& item : colors)
    {
        auto* tag = new AntTag(item.first);
        tag->setColor(item.second);
        colorRow->addWidget(tag);
    }
    auto* solid = new AntTag(QStringLiteral("Solid"));
    solid->setColor(QStringLiteral("processing"));
    solid->setVariant(Ant::TagVariant::Solid);
    auto* outlined = new AntTag(QStringLiteral("Outlined"));
    outlined->setColor(QStringLiteral("success"));
    outlined->setVariant(Ant::TagVariant::Outlined);
    colorRow->addWidget(solid);
    colorRow->addWidget(outlined);
    colorRow->addStretch();
    layout->addLayout(colorRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Checkable")));
    auto* checkRow = new QHBoxLayout();
    checkRow->setSpacing(10);
    for (const QString& text : {QStringLiteral("Design"), QStringLiteral("Code"), QStringLiteral("Review")})
    {
        auto* tag = new AntTag(text);
        tag->setCheckable(true);
        tag->setChecked(text == QStringLiteral("Code"));
        checkRow->addWidget(tag);
    }
    checkRow->addStretch();
    layout->addLayout(checkRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createProgressPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Line")));
    const QList<int> values = {30, 50, 70, 100};
    for (int value : values)
    {
        auto* progress = new AntProgress();
        progress->setPercent(value);
        if (value == 100)
        {
            progress->setStatus(Ant::ProgressStatus::Success);
        }
        layout->addWidget(progress);
    }

    layout->addWidget(createSectionTitle(QStringLiteral("Status")));
    auto* statusRow = new QHBoxLayout();
    statusRow->setSpacing(18);
    auto* active = new AntProgress();
    active->setPercent(55);
    active->setStatus(Ant::ProgressStatus::Active);
    auto* exception = new AntProgress();
    exception->setPercent(45);
    exception->setStatus(Ant::ProgressStatus::Exception);
    statusRow->addWidget(active);
    statusRow->addWidget(exception);
    layout->addLayout(statusRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Circle")));
    auto* circleRow = new QHBoxLayout();
    circleRow->setSpacing(24);
    auto* circle = new AntProgress();
    circle->setProgressType(Ant::ProgressType::Circle);
    circle->setPercent(75);
    auto* circleSuccess = new AntProgress();
    circleSuccess->setProgressType(Ant::ProgressType::Circle);
    circleSuccess->setPercent(100);
    auto* dashboard = new AntProgress();
    dashboard->setProgressType(Ant::ProgressType::Dashboard);
    dashboard->setPercent(68);
    circleRow->addWidget(circle);
    circleRow->addWidget(circleSuccess);
    circleRow->addWidget(dashboard);
    circleRow->addStretch();
    layout->addLayout(circleRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createRadioPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(24);
    auto* radioA = new AntRadio(QStringLiteral("Option A"));
    auto* radioB = new AntRadio(QStringLiteral("Option B"));
    auto* radioC = new AntRadio(QStringLiteral("Option C"));
    radioA->setChecked(true);
    basicRow->addWidget(radioA);
    basicRow->addWidget(radioB);
    basicRow->addWidget(radioC);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Disabled")));
    auto* disabledRow = new QHBoxLayout();
    disabledRow->setSpacing(24);
    auto* disabled = new AntRadio(QStringLiteral("Disabled"));
    disabled->setEnabled(false);
    auto* disabledChecked = new AntRadio(QStringLiteral("Disabled checked"));
    disabledChecked->setChecked(true);
    disabledChecked->setEnabled(false);
    disabledRow->addWidget(disabled);
    disabledRow->addWidget(disabledChecked);
    disabledRow->addStretch();
    layout->addLayout(disabledRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Vertical Group")));
    auto* group = new QWidget();
    auto* groupLayout = new QVBoxLayout(group);
    groupLayout->setContentsMargins(0, 0, 0, 0);
    groupLayout->setSpacing(12);
    auto* apple = new AntRadio(QStringLiteral("Apple"));
    auto* pear = new AntRadio(QStringLiteral("Pear"));
    auto* orange = new AntRadio(QStringLiteral("Orange"));
    pear->setChecked(true);
    groupLayout->addWidget(apple);
    groupLayout->addWidget(pear);
    groupLayout->addWidget(orange);
    layout->addWidget(group);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createAlertPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(18);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* info = new AntAlert(QStringLiteral("Informational Notes"));
    info->setAlertType(Ant::AlertType::Info);

    auto* success = new AntAlert(QStringLiteral("Success Tips"));
    success->setAlertType(Ant::AlertType::Success);

    auto* warning = new AntAlert(QStringLiteral("Warning"));
    warning->setAlertType(Ant::AlertType::Warning);

    auto* error = new AntAlert(QStringLiteral("Error"));
    error->setAlertType(Ant::AlertType::Error);

    layout->addWidget(info);
    layout->addWidget(success);
    layout->addWidget(warning);
    layout->addWidget(error);

    layout->addWidget(createSectionTitle(QStringLiteral("Description and Icon")));
    auto* described = new AntAlert(QStringLiteral("Data sync in progress"));
    described->setDescription(QStringLiteral("Background synchronization is running. You can keep working while we update the latest records from the server."));
    described->setAlertType(Ant::AlertType::Info);
    described->setShowIcon(true);
    layout->addWidget(described);

    auto* withDescription = new AntAlert(QStringLiteral("Deployment failed"));
    withDescription->setDescription(QStringLiteral("The production deployment stopped because environment variables are incomplete. Please review the release configuration and try again."));
    withDescription->setAlertType(Ant::AlertType::Error);
    withDescription->setShowIcon(true);
    layout->addWidget(withDescription);

    layout->addWidget(createSectionTitle(QStringLiteral("Closable and Action")));
    auto* closable = new AntAlert(QStringLiteral("Update available"));
    closable->setDescription(QStringLiteral("A new desktop package is ready. You can update now or postpone until after your current task."));
    closable->setAlertType(Ant::AlertType::Warning);
    closable->setShowIcon(true);
    closable->setClosable(true);
    connect(closable, &AntAlert::closeRequested, this, [this]() {
        AntMessage::info(QStringLiteral("Alert closed"), this, 1500);
    });
    layout->addWidget(closable);

    auto* actionAlert = new AntAlert(QStringLiteral("Backup completed"));
    actionAlert->setDescription(QStringLiteral("Cloud backup finished successfully. Open the latest snapshot or continue editing."));
    actionAlert->setAlertType(Ant::AlertType::Success);
    actionAlert->setShowIcon(true);
    auto* actionButton = new AntButton(QStringLiteral("Open"));
    actionButton->setButtonType(Ant::ButtonType::Link);
    connect(actionButton, &AntButton::clicked, this, [this]() {
        AntMessage::success(QStringLiteral("Opening backup snapshot"), this, 1500);
    });
    actionAlert->setActionWidget(actionButton);
    layout->addWidget(actionAlert);

    layout->addWidget(createSectionTitle(QStringLiteral("Banner")));
    auto* banner = new AntAlert(QStringLiteral("Scheduled maintenance tonight from 01:00 to 03:00."));
    banner->setBanner(true);
    banner->setClosable(true);
    layout->addWidget(banner);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createInputNumberPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(16);
    auto* basic = new AntInputNumber();
    basic->setValue(3);
    auto* placeholder = new AntInputNumber();
    placeholder->setPlaceholderText(QStringLiteral("Enter number"));
    auto* noControls = new AntInputNumber();
    noControls->setControlsVisible(false);
    noControls->setValue(42);
    basicRow->addWidget(basic);
    basicRow->addWidget(placeholder);
    basicRow->addWidget(noControls);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Sizes")));
    auto* sizeRow = new QHBoxLayout();
    sizeRow->setSpacing(16);
    auto* large = new AntInputNumber();
    large->setInputSize(Ant::InputSize::Large);
    large->setValue(100);
    auto* middle = new AntInputNumber();
    middle->setValue(100);
    auto* small = new AntInputNumber();
    small->setInputSize(Ant::InputSize::Small);
    small->setValue(100);
    sizeRow->addWidget(large);
    sizeRow->addWidget(middle);
    sizeRow->addWidget(small);
    sizeRow->addStretch();
    layout->addLayout(sizeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Status and Variant")));
    auto* statusRow = new QHBoxLayout();
    statusRow->setSpacing(16);
    auto* error = new AntInputNumber();
    error->setStatus(Ant::InputStatus::Error);
    error->setValue(12);
    auto* warning = new AntInputNumber();
    warning->setStatus(Ant::InputStatus::Warning);
    warning->setValue(64);
    auto* filled = new AntInputNumber();
    filled->setVariant(Ant::InputNumberVariant::Filled);
    filled->setValue(128);
    auto* underlined = new AntInputNumber();
    underlined->setVariant(Ant::InputNumberVariant::Underlined);
    underlined->setValue(256);
    statusRow->addWidget(error);
    statusRow->addWidget(warning);
    statusRow->addWidget(filled);
    statusRow->addWidget(underlined);
    statusRow->addStretch();
    layout->addLayout(statusRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Precision and Prefix")));
    auto* formatRow = new QHBoxLayout();
    formatRow->setSpacing(16);
    auto* percent = new AntInputNumber();
    percent->setRange(0, 100);
    percent->setSuffixText(QStringLiteral("%"));
    percent->setValue(85);
    auto* currency = new AntInputNumber();
    currency->setPrefixText(QStringLiteral("$ "));
    currency->setRange(0, 100000);
    currency->setValue(2999);
    auto* decimal = new AntInputNumber();
    decimal->setPrecision(2);
    decimal->setSingleStep(0.25);
    decimal->setValue(1.50);
    formatRow->addWidget(percent);
    formatRow->addWidget(currency);
    formatRow->addWidget(decimal);
    formatRow->addStretch();
    layout->addLayout(formatRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Controlled and Disabled")));
    auto* controlledRow = new QHBoxLayout();
    controlledRow->setSpacing(16);
    auto* quantity = new AntInputNumber();
    quantity->setRange(1, 20);
    quantity->setValue(2);
    auto* summary = new AntTypography(QStringLiteral("Quantity: 2"), page);
    summary->setMinimumWidth(120);
    connect(quantity, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [summary](double value) {
        summary->setText(QStringLiteral("Quantity: %1").arg(value, 0, 'f', 0));
    });
    auto* disabled = new AntInputNumber();
    disabled->setValue(10);
    disabled->setEnabled(false);
    auto* borderless = new AntInputNumber();
    borderless->setVariant(Ant::InputNumberVariant::Borderless);
    borderless->setValue(77);
    controlledRow->addWidget(quantity);
    controlledRow->addWidget(summary);
    controlledRow->addSpacing(12);
    controlledRow->addWidget(disabled);
    controlledRow->addWidget(borderless);
    controlledRow->addStretch();
    layout->addLayout(controlledRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createTooltipPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(16);
    auto* save = new AntButton(QStringLiteral("Save"));
    save->setButtonType(Ant::ButtonType::Primary);
    auto* saveTip = new AntTooltip(page);
    saveTip->setTitle(QStringLiteral("Persist your latest changes"));
    saveTip->setTarget(save);

    auto* deleteButton = new AntButton(QStringLiteral("Delete"));
    deleteButton->setDanger(true);
    auto* deleteTip = new AntTooltip(page);
    deleteTip->setTitle(QStringLiteral("This action cannot be undone"));
    deleteTip->setPlacement(Ant::TooltipPlacement::Bottom);
    deleteTip->setTarget(deleteButton);

    basicRow->addWidget(save);
    basicRow->addWidget(deleteButton);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Placement")));
    auto* placementWrap = new QWidget(page);
    auto* placementLayout = new QVBoxLayout(placementWrap);
    placementLayout->setContentsMargins(0, 0, 0, 0);
    placementLayout->setSpacing(14);

    auto* topRow = new QHBoxLayout();
    topRow->setSpacing(14);
    topRow->addStretch();
    auto* top = new AntButton(QStringLiteral("Top"));
    auto* topTip = new AntTooltip(page);
    topTip->setTitle(QStringLiteral("Shown above the trigger"));
    topTip->setPlacement(Ant::TooltipPlacement::Top);
    topTip->setTarget(top);
    auto* topLeft = new AntButton(QStringLiteral("TopLeft"));
    auto* topLeftTip = new AntTooltip(page);
    topLeftTip->setTitle(QStringLiteral("Aligned with the left edge"));
    topLeftTip->setPlacement(Ant::TooltipPlacement::TopLeft);
    topLeftTip->setTarget(topLeft);
    auto* topRight = new AntButton(QStringLiteral("TopRight"));
    auto* topRightTip = new AntTooltip(page);
    topRightTip->setTitle(QStringLiteral("Aligned with the right edge"));
    topRightTip->setPlacement(Ant::TooltipPlacement::TopRight);
    topRightTip->setTarget(topRight);
    topRow->addWidget(topLeft);
    topRow->addWidget(top);
    topRow->addWidget(topRight);
    topRow->addStretch();
    placementLayout->addLayout(topRow);

    auto* middleRow = new QHBoxLayout();
    middleRow->setSpacing(20);
    auto* left = new AntButton(QStringLiteral("Left"));
    auto* leftTip = new AntTooltip(page);
    leftTip->setTitle(QStringLiteral("Appears on the left side"));
    leftTip->setPlacement(Ant::TooltipPlacement::Left);
    leftTip->setTarget(left);
    auto* center = new QLabel(QStringLiteral("Hover the buttons around this area"), page);
    center->setMinimumSize(220, 64);
    center->setAlignment(Qt::AlignCenter);
    auto* right = new AntButton(QStringLiteral("Right"));
    auto* rightTip = new AntTooltip(page);
    rightTip->setTitle(QStringLiteral("Appears on the right side"));
    rightTip->setPlacement(Ant::TooltipPlacement::Right);
    rightTip->setTarget(right);
    middleRow->addStretch();
    middleRow->addWidget(left);
    middleRow->addWidget(center);
    middleRow->addWidget(right);
    middleRow->addStretch();
    placementLayout->addLayout(middleRow);

    auto* bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(14);
    bottomRow->addStretch();
    auto* bottomLeft = new AntButton(QStringLiteral("BottomLeft"));
    auto* bottomLeftTip = new AntTooltip(page);
    bottomLeftTip->setTitle(QStringLiteral("Bottom placement with left alignment"));
    bottomLeftTip->setPlacement(Ant::TooltipPlacement::BottomLeft);
    bottomLeftTip->setTarget(bottomLeft);
    auto* bottom = new AntButton(QStringLiteral("Bottom"));
    auto* bottomTip = new AntTooltip(page);
    bottomTip->setTitle(QStringLiteral("Shown below the trigger"));
    bottomTip->setPlacement(Ant::TooltipPlacement::Bottom);
    bottomTip->setTarget(bottom);
    auto* bottomRight = new AntButton(QStringLiteral("BottomRight"));
    auto* bottomRightTip = new AntTooltip(page);
    bottomRightTip->setTitle(QStringLiteral("Bottom placement with right alignment"));
    bottomRightTip->setPlacement(Ant::TooltipPlacement::BottomRight);
    bottomRightTip->setTarget(bottomRight);
    bottomRow->addWidget(bottomLeft);
    bottomRow->addWidget(bottom);
    bottomRow->addWidget(bottomRight);
    bottomRow->addStretch();
    placementLayout->addLayout(bottomRow);

    layout->addWidget(placementWrap);

    layout->addWidget(createSectionTitle(QStringLiteral("Arrow and Color")));
    auto* customRow = new QHBoxLayout();
    customRow->setSpacing(16);
    auto* colorful = new AntButton(QStringLiteral("Colorful"));
    auto* colorfulTip = new AntTooltip(page);
    colorfulTip->setTitle(QStringLiteral("Custom background colors adapt text contrast"));
    colorfulTip->setColor(QColor(QStringLiteral("#1677ff")));
    colorfulTip->setTarget(colorful);
    auto* noArrow = new AntButton(QStringLiteral("No Arrow"));
    auto* noArrowTip = new AntTooltip(page);
    noArrowTip->setTitle(QStringLiteral("Tooltip without arrow"));
    noArrowTip->setArrowVisible(false);
    noArrowTip->setTarget(noArrow);
    auto* delayed = new AntButton(QStringLiteral("Delay"));
    auto* delayedTip = new AntTooltip(page);
    delayedTip->setTitle(QStringLiteral("Appears after a longer delay"));
    delayedTip->setOpenDelay(500);
    delayedTip->setTarget(delayed);
    customRow->addWidget(colorful);
    customRow->addWidget(noArrow);
    customRow->addWidget(delayed);
    customRow->addStretch();
    layout->addLayout(customRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createSelectPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(16);
    auto* basic = new AntSelect();
    basic->setPlaceholderText(QStringLiteral("Select a fruit"));
    basic->addOptions({QStringLiteral("Apple"), QStringLiteral("Pear"), QStringLiteral("Orange")});
    auto* withValue = new AntSelect();
    withValue->addOptions({QStringLiteral("China"), QStringLiteral("United States"), QStringLiteral("Japan")});
    withValue->setCurrentIndex(0);
    withValue->setAllowClear(true);
    basicRow->addWidget(basic);
    basicRow->addWidget(withValue);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Size")));
    auto* sizeRow = new QHBoxLayout();
    sizeRow->setSpacing(16);
    auto* large = new AntSelect();
    large->setSelectSize(Ant::SelectSize::Large);
    large->addOptions({QStringLiteral("Large"), QStringLiteral("Option")});
    large->setCurrentIndex(0);
    auto* middle = new AntSelect();
    middle->addOptions({QStringLiteral("Middle"), QStringLiteral("Option")});
    middle->setCurrentIndex(0);
    auto* small = new AntSelect();
    small->setSelectSize(Ant::SelectSize::Small);
    small->addOptions({QStringLiteral("Small"), QStringLiteral("Option")});
    small->setCurrentIndex(0);
    sizeRow->addWidget(large);
    sizeRow->addWidget(middle);
    sizeRow->addWidget(small);
    sizeRow->addStretch();
    layout->addLayout(sizeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Status and Variant")));
    auto* variantRow = new QHBoxLayout();
    variantRow->setSpacing(16);
    auto* error = new AntSelect();
    error->setStatus(Ant::SelectStatus::Error);
    error->setPlaceholderText(QStringLiteral("Error"));
    error->addOptions({QStringLiteral("Invalid option"), QStringLiteral("Valid option")});
    auto* warning = new AntSelect();
    warning->setStatus(Ant::SelectStatus::Warning);
    warning->setPlaceholderText(QStringLiteral("Warning"));
    warning->addOptions({QStringLiteral("Risky option"), QStringLiteral("Safe option")});
    auto* filled = new AntSelect();
    filled->setVariant(Ant::SelectVariant::Filled);
    filled->addOptions({QStringLiteral("Filled"), QStringLiteral("Outlined")});
    filled->setCurrentIndex(0);
    auto* underlined = new AntSelect();
    underlined->setVariant(Ant::SelectVariant::Underlined);
    underlined->addOptions({QStringLiteral("Underlined"), QStringLiteral("Borderless")});
    underlined->setCurrentIndex(0);
    variantRow->addWidget(error);
    variantRow->addWidget(warning);
    variantRow->addWidget(filled);
    variantRow->addWidget(underlined);
    variantRow->addStretch();
    layout->addLayout(variantRow);

    layout->addWidget(createSectionTitle(QStringLiteral("States")));
    auto* stateRow = new QHBoxLayout();
    stateRow->setSpacing(16);
    auto* disabled = new AntSelect();
    disabled->addOptions({QStringLiteral("Disabled")});
    disabled->setCurrentIndex(0);
    disabled->setEnabled(false);
    auto* loading = new AntSelect();
    loading->addOptions({QStringLiteral("Loading"), QStringLiteral("Loaded")});
    loading->setCurrentIndex(0);
    loading->setLoading(true);
    auto* disabledOption = new AntSelect();
    disabledOption->setPlaceholderText(QStringLiteral("Option disabled"));
    disabledOption->addOption(QStringLiteral("Enabled"));
    disabledOption->addOption(QStringLiteral("Disabled option"), QStringLiteral("disabled"), true);
    disabledOption->addOption(QStringLiteral("Another enabled"));
    stateRow->addWidget(disabled);
    stateRow->addWidget(loading);
    stateRow->addWidget(disabledOption);
    stateRow->addStretch();
    layout->addLayout(stateRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createSliderPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(16);
    auto* basic = new AntSlider();
    basic->setValue(30);
    auto* basicValue = new AntTypography(QString::number(basic->value()));
    basicValue->setMinimumWidth(36);
    connect(basic, &AntSlider::valueChanged, this, [basicValue](int value) {
        basicValue->setText(QString::number(value));
    });
    basicRow->addWidget(basic, 1);
    basicRow->addWidget(basicValue);
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Dots and Step")));
    auto* dotsRow = new QHBoxLayout();
    dotsRow->setSpacing(16);
    auto* dots = new AntSlider();
    dots->setRange(0, 50);
    dots->setSingleStep(10);
    dots->setDots(true);
    dots->setValue(20);
    auto* reverse = new AntSlider();
    reverse->setReverse(true);
    reverse->setValue(60);
    dotsRow->addWidget(dots, 1);
    dotsRow->addWidget(reverse, 1);
    layout->addLayout(dotsRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Vertical")));
    auto* verticalRow = new QHBoxLayout();
    verticalRow->setSpacing(28);
    auto* vertical = new AntSlider(Qt::Vertical);
    vertical->setFixedHeight(180);
    vertical->setValue(45);
    auto* verticalReverse = new AntSlider(Qt::Vertical);
    verticalReverse->setFixedHeight(180);
    verticalReverse->setReverse(true);
    verticalReverse->setValue(25);
    verticalRow->addWidget(vertical);
    verticalRow->addWidget(verticalReverse);
    verticalRow->addStretch();
    layout->addLayout(verticalRow);

    layout->addWidget(createSectionTitle(QStringLiteral("States")));
    auto* stateRow = new QHBoxLayout();
    stateRow->setSpacing(16);
    auto* disabled = new AntSlider();
    disabled->setValue(40);
    disabled->setEnabled(false);
    auto* noTrack = new AntSlider();
    noTrack->setValue(70);
    noTrack->setIncluded(false);
    stateRow->addWidget(disabled, 1);
    stateRow->addWidget(noTrack, 1);
    layout->addLayout(stateRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createSpinPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Size")));
    auto* sizeRow = new QHBoxLayout();
    sizeRow->setSpacing(28);
    auto* small = new AntSpin();
    small->setSpinSize(Ant::SpinSize::Small);
    auto* middle = new AntSpin();
    auto* large = new AntSpin();
    large->setSpinSize(Ant::SpinSize::Large);
    sizeRow->addWidget(small);
    sizeRow->addWidget(middle);
    sizeRow->addWidget(large);
    sizeRow->addStretch();
    layout->addLayout(sizeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Description")));
    auto* descRow = new QHBoxLayout();
    descRow->setSpacing(28);
    auto* loading = new AntSpin();
    loading->setSpinSize(Ant::SpinSize::Large);
    loading->setDescription(QStringLiteral("Loading"));
    auto* percent = new AntSpin();
    percent->setSpinSize(Ant::SpinSize::Large);
    percent->setPercent(68);
    percent->setDescription(QStringLiteral("68%"));
    auto* hidden = new AntSpin();
    hidden->setSpinning(false);
    hidden->setDescription(QStringLiteral("Hidden"));
    descRow->addWidget(loading);
    descRow->addWidget(percent);
    descRow->addWidget(hidden);
    descRow->addStretch();
    layout->addLayout(descRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Embedded")));
    auto* card = new AntCard(QStringLiteral("Loading block"));
    card->setMinimumHeight(160);
    auto* cardContent = new QVBoxLayout();
    auto* embedded = new AntSpin();
    embedded->setDescription(QStringLiteral("Fetching data"));
    cardContent->addStretch();
    cardContent->addWidget(embedded, 0, Qt::AlignCenter);
    cardContent->addStretch();
    card->bodyLayout()->addLayout(cardContent);
    layout->addWidget(card);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createStepsPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(28);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basic = new AntSteps(page);
    basic->addStep(QStringLiteral("Create Workspace"), QStringLiteral("Set up name, visibility and project members."));
    basic->addStep(QStringLiteral("Configure Access"), QStringLiteral("Choose environments and grant permissions."));
    basic->addStep(QStringLiteral("Review & Launch"), QStringLiteral("Confirm settings and publish to the team."));
    basic->setCurrentIndex(1);
    layout->addWidget(basic);

    layout->addWidget(createSectionTitle(QStringLiteral("Clickable and Error State")));
    auto* interactive = new AntSteps(page);
    interactive->addStep(QStringLiteral("Draft"), QStringLiteral("Initial content prepared."));
    interactive->addStep(QStringLiteral("Validation"), QStringLiteral("Checks are running on the latest revision."));
    interactive->addStep(QStringLiteral("Approval"), QStringLiteral("A reviewer rejected the request."), QStringLiteral("Needs attention"), Ant::StepStatus::Error);
    interactive->addStep(QStringLiteral("Release"), QStringLiteral("Will proceed after approval."));
    interactive->setCurrentIndex(2);
    auto* summary = new AntTypography(QStringLiteral("Current step: Approval"), page);
    connect(interactive, &AntSteps::stepClicked, this, [summary, interactive](int index) {
        summary->setText(QStringLiteral("Current step: %1").arg(interactive->stepAt(index).title));
    });
    layout->addWidget(interactive);
    layout->addWidget(summary);

    layout->addWidget(createSectionTitle(QStringLiteral("Vertical")));
    auto* vertical = new AntSteps(page);
    vertical->setDirection(Ant::StepsDirection::Vertical);
    vertical->setClickable(false);
    vertical->addStep(QStringLiteral("Submitted"), QStringLiteral("The request has been created and queued."), QStringLiteral("09:12"), Ant::StepStatus::Finish);
    vertical->addStep(QStringLiteral("Security Review"), QStringLiteral("Scanning package and permission changes."), QStringLiteral("10:24"));
    vertical->addStep(QStringLiteral("Deployment"), QStringLiteral("Waiting for review to complete."), QStringLiteral("Pending"));
    vertical->setCurrentIndex(1);
    vertical->setMinimumHeight(280);
    layout->addWidget(vertical);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createSwitchPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(18);
    auto* offSwitch = new AntSwitch();
    auto* onSwitch = new AntSwitch();
    onSwitch->setChecked(true);
    auto* smallSwitch = new AntSwitch();
    smallSwitch->setSwitchSize(Ant::SwitchSize::Small);
    smallSwitch->setChecked(true);
    basicRow->addWidget(offSwitch);
    basicRow->addWidget(onSwitch);
    basicRow->addWidget(smallSwitch);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Text")));
    auto* textRow = new QHBoxLayout();
    textRow->setSpacing(18);
    auto* textSwitch = new AntSwitch();
    textSwitch->setCheckedText(QStringLiteral("ON"));
    textSwitch->setUncheckedText(QStringLiteral("OFF"));
    textSwitch->setChecked(true);
    auto* zhSwitch = new AntSwitch();
    zhSwitch->setCheckedText(QStringLiteral("开"));
    zhSwitch->setUncheckedText(QStringLiteral("关"));
    textRow->addWidget(textSwitch);
    textRow->addWidget(zhSwitch);
    textRow->addStretch();
    layout->addLayout(textRow);

    layout->addWidget(createSectionTitle(QStringLiteral("States")));
    auto* stateRow = new QHBoxLayout();
    stateRow->setSpacing(18);
    auto* loading = new AntSwitch();
    loading->setChecked(true);
    loading->setLoading(true);
    auto* disabledOff = new AntSwitch();
    disabledOff->setEnabled(false);
    auto* disabledOn = new AntSwitch();
    disabledOn->setChecked(true);
    disabledOn->setEnabled(false);
    stateRow->addWidget(loading);
    stateRow->addWidget(disabledOff);
    stateRow->addWidget(disabledOn);
    stateRow->addStretch();
    layout->addLayout(stateRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createTimePickerPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(16);
    auto* basic = new AntTimePicker();
    basic->setPlaceholderText(QStringLiteral("Select time"));
    auto* selected = new AntTimePicker();
    selected->setSelectedTime(QTime(13, 30, 56));
    selected->setAllowClear(true);
    auto* customFormat = new AntTimePicker();
    customFormat->setSelectedTime(QTime(9, 15, 0));
    customFormat->setDisplayFormat(QStringLiteral("HH:mm"));
    customFormat->setSecondStep(10);
    basicRow->addWidget(basic);
    basicRow->addWidget(selected);
    basicRow->addWidget(customFormat);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Size")));
    auto* sizeRow = new QHBoxLayout();
    sizeRow->setSpacing(16);
    auto* large = new AntTimePicker();
    large->setPickerSize(Ant::TimePickerSize::Large);
    large->setSelectedTime(QTime::currentTime());
    auto* middle = new AntTimePicker();
    middle->setSelectedTime(QTime::currentTime());
    auto* small = new AntTimePicker();
    small->setPickerSize(Ant::TimePickerSize::Small);
    small->setSelectedTime(QTime::currentTime());
    sizeRow->addWidget(large);
    sizeRow->addWidget(middle);
    sizeRow->addWidget(small);
    sizeRow->addStretch();
    layout->addLayout(sizeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Status and Variant")));
    auto* variantRow = new QHBoxLayout();
    variantRow->setSpacing(16);
    auto* error = new AntTimePicker();
    error->setStatus(Ant::TimePickerStatus::Error);
    error->setPlaceholderText(QStringLiteral("Error"));
    auto* warning = new AntTimePicker();
    warning->setStatus(Ant::TimePickerStatus::Warning);
    warning->setPlaceholderText(QStringLiteral("Warning"));
    auto* filled = new AntTimePicker();
    filled->setVariant(Ant::TimePickerVariant::Filled);
    filled->setSelectedTime(QTime(8, 0, 0));
    auto* underlined = new AntTimePicker();
    underlined->setVariant(Ant::TimePickerVariant::Underlined);
    underlined->setSelectedTime(QTime(18, 45, 0));
    variantRow->addWidget(error);
    variantRow->addWidget(warning);
    variantRow->addWidget(filled);
    variantRow->addWidget(underlined);
    variantRow->addStretch();
    layout->addLayout(variantRow);

    layout->addWidget(createSectionTitle(QStringLiteral("States and Step")));
    auto* stateRow = new QHBoxLayout();
    stateRow->setSpacing(16);
    auto* disabled = new AntTimePicker();
    disabled->setSelectedTime(QTime(12, 0, 0));
    disabled->setEnabled(false);
    auto* stepped = new AntTimePicker();
    stepped->setMinuteStep(15);
    stepped->setSecondStep(15);
    stepped->setSelectedTime(QTime(10, 30, 30));
    auto* noNow = new AntTimePicker();
    noNow->setShowNow(false);
    noNow->setSelectedTime(QTime(22, 10, 5));
    stateRow->addWidget(disabled);
    stateRow->addWidget(stepped);
    stateRow->addWidget(noNow);
    stateRow->addStretch();
    layout->addLayout(stateRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createCardPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(18);

    auto* row = new QHBoxLayout();
    row->setSpacing(18);
    auto* basic = new AntCard(QStringLiteral("Default card"));
    basic->setExtra(QStringLiteral("More"));
    basic->setMinimumSize(280, 180);
    basic->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Card content\nCard content\nCard content")));

    auto* hoverable = new AntCard(QStringLiteral("Hoverable"));
    hoverable->setHoverable(true);
    hoverable->setMinimumSize(280, 180);
    hoverable->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Move the mouse over this card.")));

    auto* loading = new AntCard(QStringLiteral("Loading"));
    loading->setLoading(true);
    loading->setMinimumSize(280, 180);
    loading->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Loading mask and spinner")));
    row->addWidget(basic);
    row->addWidget(hoverable);
    row->addWidget(loading);
    layout->addLayout(row);

    auto* actionCard = new AntCard(QStringLiteral("Actions"));
    actionCard->setMinimumHeight(180);
    actionCard->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Card with action slots.")));
    actionCard->addActionWidget(new AntTypography(QStringLiteral("Edit")));
    actionCard->addActionWidget(new AntTypography(QStringLiteral("Share")));
    actionCard->addActionWidget(new AntTypography(QStringLiteral("Delete")));
    layout->addWidget(actionCard);
    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createSkeletonPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(20);

    auto* simple = new AntSkeleton(page);
    simple->setFixedWidth(280);
    simple->setParagraphRows(3);

    auto* active = new AntSkeleton(page);
    active->setFixedWidth(280);
    active->setActive(true);
    active->setTitleWidthRatio(0.56);
    active->setParagraphWidthRatios({1.0, 0.92, 0.48});

    auto* staticOne = new AntSkeleton(page);
    staticOne->setFixedWidth(280);
    staticOne->setActive(false);
    staticOne->setParagraphRows(2);
    staticOne->setParagraphWidthRatios({0.88, 0.54});

    basicRow->addWidget(simple);
    basicRow->addWidget(active);
    basicRow->addWidget(staticOne);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Avatar and Round")));
    auto* avatarRow = new QHBoxLayout();
    avatarRow->setSpacing(20);

    auto* article = new AntSkeleton(page);
    article->setFixedWidth(420);
    article->setAvatarVisible(true);
    article->setAvatarShape(Ant::AvatarShape::Circle);
    article->setParagraphRows(3);
    article->setParagraphWidthRatios({0.98, 0.86, 0.58});

    auto* profile = new AntSkeleton(page);
    profile->setFixedWidth(420);
    profile->setAvatarVisible(true);
    profile->setAvatarShape(Ant::AvatarShape::Square);
    profile->setRound(true);
    profile->setTitleWidthRatio(0.38);
    profile->setParagraphRows(4);
    profile->setParagraphWidthRatios({1.0, 0.94, 0.84, 0.52});

    avatarRow->addWidget(article);
    avatarRow->addWidget(profile);
    avatarRow->addStretch();
    layout->addLayout(avatarRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Loading Switch")));
    auto* switchRow = new QHBoxLayout();
    switchRow->setSpacing(20);

    auto* previewCard = new AntCard(QStringLiteral("Workspace Summary"), page);
    previewCard->setFixedWidth(520);
    previewCard->setExtra(QStringLiteral("Ready"));
    previewCard->bodyLayout()->addWidget(new AntTypography(QStringLiteral("3 reviewers assigned"), previewCard));
    previewCard->bodyLayout()->addWidget(new AntTypography(QStringLiteral("12 checklist items completed"), previewCard));
    previewCard->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Next milestone: beta publish"), previewCard));

    auto* wrapped = new AntSkeleton(page);
    wrapped->setFixedWidth(520);
    wrapped->setAvatarVisible(true);
    wrapped->setParagraphRows(4);
    wrapped->setParagraphWidthRatios({0.95, 0.82, 0.76, 0.44});
    wrapped->setContentWidget(previewCard);

    auto* toggle = new AntButton(QStringLiteral("Toggle Loading"));
    toggle->setButtonType(Ant::ButtonType::Primary);
    connect(toggle, &AntButton::clicked, this, [wrapped]() {
        wrapped->setLoading(!wrapped->isLoading());
    });

    switchRow->addWidget(wrapped);
    switchRow->addWidget(toggle, 0, Qt::AlignTop);
    switchRow->addStretch();
    layout->addLayout(switchRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createFormPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(28);

    layout->addWidget(createSectionTitle(QStringLiteral("Horizontal")));
    auto* horizontalForm = new AntForm(page);
    horizontalForm->setLabelWidth(112);

    auto* username = new AntInput(horizontalForm);
    username->setPlaceholderText(QStringLiteral("Enter workspace name"));
    auto* userItem = horizontalForm->addItem(QStringLiteral("Workspace"), username, true);
    userItem->setExtra(QStringLiteral("This name will be visible to your collaborators."));

    auto* role = new AntSelect(horizontalForm);
    role->setPlaceholderText(QStringLiteral("Select owner role"));
    role->addOptions({QStringLiteral("Designer"), QStringLiteral("Engineer"), QStringLiteral("Reviewer")});
    role->setCurrentIndex(1);
    horizontalForm->addItem(QStringLiteral("Owner Role"), role, true);

    auto* notify = new AntSwitch(horizontalForm);
    notify->setChecked(true);
    notify->setCheckedText(QStringLiteral("On"));
    notify->setUncheckedText(QStringLiteral("Off"));
    auto* notifyItem = horizontalForm->addItem(QStringLiteral("Notifications"), notify);
    notifyItem->setExtra(QStringLiteral("Send summary updates to the project channel."));

    layout->addWidget(horizontalForm);

    layout->addWidget(createSectionTitle(QStringLiteral("Validation")));
    auto* validationForm = new AntForm(page);
    validationForm->setLabelWidth(112);

    auto* email = new AntInput(validationForm);
    email->setPlaceholderText(QStringLiteral("name@example.com"));
    email->setStatus(Ant::InputStatus::Error);
    auto* emailItem = validationForm->addItem(QStringLiteral("Email"), email, true);
    emailItem->setValidateStatus(Ant::InputStatus::Error);
    emailItem->setHelpText(QStringLiteral("Please enter a valid email address."));

    auto* apiKey = new AntInput(validationForm);
    apiKey->setPasswordMode(true);
    apiKey->setText(QStringLiteral("temporary-token"));
    apiKey->setStatus(Ant::InputStatus::Warning);
    auto* keyItem = validationForm->addItem(QStringLiteral("API Key"), apiKey, true);
    keyItem->setValidateStatus(Ant::InputStatus::Warning);
    keyItem->setHelpText(QStringLiteral("This token will expire in 3 days. Rotate it soon."));

    auto* agreement = new AntCheckbox(QStringLiteral("I understand this token can access production data"), validationForm);
    auto* agreementItem = validationForm->addItem(QStringLiteral("Agreement"), agreement, true);
    agreementItem->setExtra(QStringLiteral("Store the key in your team's secret manager."));

    layout->addWidget(validationForm);

    layout->addWidget(createSectionTitle(QStringLiteral("Vertical and Inline")));
    auto* bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(28);

    auto* verticalForm = new AntForm(page);
    verticalForm->setFormLayout(Ant::FormLayout::Vertical);
    verticalForm->setLabelAlign(Ant::FormLabelAlign::Left);

    auto* project = new AntInput(verticalForm);
    project->setPlaceholderText(QStringLiteral("Release dashboard"));
    verticalForm->addItem(QStringLiteral("Project Name"), project, true);

    auto* summary = new AntInput(verticalForm);
    summary->setPlaceholderText(QStringLiteral("Short summary for teammates"));
    auto* summaryItem = verticalForm->addItem(QStringLiteral("Summary"), summary);
    summaryItem->setExtra(QStringLiteral("Keep it concise so it fits well in list views."));

    auto* publishButton = new AntButton(QStringLiteral("Create Project"), verticalForm);
    publishButton->setButtonType(Ant::ButtonType::Primary);
    auto* actionItem = verticalForm->addItem(QString(), publishButton);
    actionItem->setColon(false);

    bottomRow->addWidget(verticalForm, 1);

    auto* inlineWrap = new QWidget(page);
    auto* inlineWrapLayout = new QVBoxLayout(inlineWrap);
    inlineWrapLayout->setContentsMargins(0, 0, 0, 0);
    inlineWrapLayout->setSpacing(12);

    auto* inlineHint = new AntTypography(QStringLiteral("Inline layout is handy for compact filters and toolbar forms."), inlineWrap);
    inlineHint->setParagraph(true);
    inlineWrapLayout->addWidget(inlineHint);

    auto* inlineForm = new AntForm(inlineWrap);
    inlineForm->setFormLayout(Ant::FormLayout::Inline);
    inlineForm->setLabelAlign(Ant::FormLabelAlign::Left);
    inlineForm->setLabelWidth(72);

    auto* search = new AntInput(inlineForm);
    search->setPlaceholderText(QStringLiteral("Search issues"));
    inlineForm->addItem(QStringLiteral("Keyword"), search);

    auto* status = new AntSelect(inlineForm);
    status->addOptions({QStringLiteral("Open"), QStringLiteral("In Progress"), QStringLiteral("Done")});
    status->setCurrentIndex(0);
    inlineForm->addItem(QStringLiteral("Status"), status);

    auto* apply = new AntButton(QStringLiteral("Apply"), inlineForm);
    apply->setButtonType(Ant::ButtonType::Primary);
    auto* applyItem = inlineForm->addItem(QString(), apply);
    applyItem->setColon(false);

    inlineWrapLayout->addWidget(inlineForm);
    inlineWrapLayout->addStretch();

    bottomRow->addWidget(inlineWrap, 1);
    layout->addLayout(bottomRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createEmptyPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(28);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(28);

    auto* basic = new AntEmpty(page);
    basic->setFixedWidth(260);

    auto* customText = new AntEmpty(page);
    customText->setFixedWidth(320);
    customText->setDescription(QStringLiteral("No matching issues found. Try adjusting the filters and search terms."));

    auto* simple = new AntEmpty(page);
    simple->setFixedWidth(220);
    simple->setSimple(true);
    simple->setDescription(QStringLiteral("No notifications"));

    basicRow->addWidget(basic);
    basicRow->addWidget(customText);
    basicRow->addWidget(simple);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("With Action")));
    auto* actionRow = new QHBoxLayout();
    actionRow->setSpacing(28);

    auto* createProject = new AntEmpty(page);
    createProject->setFixedWidth(360);
    createProject->setDescription(QStringLiteral("You have not created any projects yet."));
    auto* createButton = new AntButton(QStringLiteral("Create Project"), createProject);
    createButton->setButtonType(Ant::ButtonType::Primary);
    createProject->setExtraWidget(createButton);

    auto* importData = new AntEmpty(page);
    importData->setFixedWidth(360);
    importData->setImageSize(QSize(148, 92));
    importData->setDescription(QStringLiteral("Import data from CSV or connect a remote source to get started."));
    auto* importButton = new AntButton(QStringLiteral("Import Data"), importData);
    importButton->setButtonType(Ant::ButtonType::Default);
    importData->setExtraWidget(importButton);

    actionRow->addWidget(createProject);
    actionRow->addWidget(importData);
    actionRow->addStretch();
    layout->addLayout(actionRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Embedded in Card")));
    auto* cardRow = new QHBoxLayout();
    cardRow->setSpacing(24);

    auto* emptyCard = new AntCard(QStringLiteral("Recent Activity"), page);
    emptyCard->setFixedWidth(420);
    emptyCard->setExtra(QStringLiteral("0 items"));
    auto* cardEmpty = new AntEmpty(emptyCard);
    cardEmpty->setDescription(QStringLiteral("There is no activity in this workspace yet."));
    auto* inviteButton = new AntButton(QStringLiteral("Invite Collaborators"), cardEmpty);
    inviteButton->setButtonType(Ant::ButtonType::Primary);
    cardEmpty->setExtraWidget(inviteButton);
    emptyCard->bodyLayout()->addWidget(cardEmpty);

    auto* placeholderCard = new AntCard(QStringLiteral("Archived Reports"), page);
    placeholderCard->setFixedWidth(420);
    auto* archiveEmpty = new AntEmpty(placeholderCard);
    archiveEmpty->setSimple(true);
    archiveEmpty->setImageVisible(false);
    archiveEmpty->setDescription(QStringLiteral("Archived reports will appear here."));
    placeholderCard->bodyLayout()->addWidget(archiveEmpty);

    cardRow->addWidget(emptyCard);
    cardRow->addWidget(placeholderCard);
    cardRow->addStretch();
    layout->addLayout(cardRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createResultPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(28);

    layout->addWidget(createSectionTitle(QStringLiteral("Status")));
    auto* statusRow = new QHBoxLayout();
    statusRow->setSpacing(28);

    auto* success = new AntResult(page);
    success->setFixedWidth(260);
    success->setStatus(Ant::AlertType::Success);
    success->setTitle(QStringLiteral("Successfully Purchased"));
    success->setSubTitle("Order number: 2018122412345678. Your order has been placed and will be shipped within 24 hours.");
    auto* successBtn = new AntButton(QStringLiteral("View Orders"), success);
    successBtn->setButtonType(Ant::ButtonType::Primary);
    success->setExtraWidget(successBtn);

    auto* warning = new AntResult(page);
    warning->setFixedWidth(260);
    warning->setStatus(Ant::AlertType::Warning);
    warning->setTitle(QStringLiteral("Attention Required"));
    warning->setSubTitle("Your subscription will expire in 3 days. Please renew to avoid service interruption.");
    auto* warningBtn = new AntButton(QStringLiteral("Renew Now"), warning);
    warningBtn->setButtonType(Ant::ButtonType::Primary);
    warning->setExtraWidget(warningBtn);

    auto* error = new AntResult(page);
    error->setFixedWidth(260);
    error->setStatus(Ant::AlertType::Error);
    error->setTitle(QStringLiteral("Submission Failed"));
    error->setSubTitle("Please check the form fields and try again.");
    auto* errorBtn = new AntButton(QStringLiteral("Retry"), error);
    errorBtn->setButtonType(Ant::ButtonType::Primary);
    error->setExtraWidget(errorBtn);

    auto* info = new AntResult(page);
    info->setFixedWidth(260);
    info->setStatus(Ant::AlertType::Info);
    info->setTitle(QStringLiteral("Processing"));
    info->setSubTitle("Your request is being processed. Please wait a moment.");

    statusRow->addWidget(success);
    statusRow->addWidget(warning);
    statusRow->addWidget(error);
    statusRow->addWidget(info);
    statusRow->addStretch();
    layout->addLayout(statusRow);

    layout->addWidget(createSectionTitle(QStringLiteral("With Extra Actions")));
    auto* actionRow = new QHBoxLayout();
    actionRow->setSpacing(28);

    auto* withActions = new AntResult(page);
    withActions->setFixedWidth(400);
    withActions->setStatus(Ant::AlertType::Success);
    withActions->setTitle(QStringLiteral("Payment Complete"));
    withActions->setSubTitle("Thank you for your purchase. A confirmation email has been sent to your inbox.");
    auto* btnRow = new QWidget(withActions);
    auto* btnLayout = new QHBoxLayout(btnRow);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(12);
    auto* goHome = new AntButton(QStringLiteral("Go Home"), btnRow);
    goHome->setButtonType(Ant::ButtonType::Primary);
    auto* viewDetail = new AntButton(QStringLiteral("View Detail"), btnRow);
    btnLayout->addWidget(goHome);
    btnLayout->addWidget(viewDetail);
    withActions->setExtraWidget(btnRow);

    auto* noIcon = new AntResult(page);
    noIcon->setFixedWidth(400);
    noIcon->setIconVisible(false);
    noIcon->setTitle(QStringLiteral("Custom Content Area"));
    noIcon->setSubTitle("This result has no icon. You can place any content here.");
    auto* noIconBtn = new AntButton(QStringLiteral("Back"), noIcon);
    noIconBtn->setButtonType(Ant::ButtonType::Primary);
    noIcon->setExtraWidget(noIconBtn);

    actionRow->addWidget(withActions);
    actionRow->addWidget(noIcon);
    actionRow->addStretch();
    layout->addLayout(actionRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createListPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(28);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicList = new AntList(page);
    basicList->setBordered(true);
    basicList->setFixedWidth(480);

    for (int i = 1; i <= 4; ++i)
    {
        auto* item = new AntListItem(basicList);
        auto* meta = new AntListItemMeta(item);
        meta->setTitle(QStringLiteral("Ant Design List Item %1").arg(i));
        meta->setDescription(QStringLiteral("Description for item %1").arg(i));
        item->setMeta(meta);
        basicList->addItem(item);
    }

    layout->addWidget(basicList);

    layout->addWidget(createSectionTitle(QStringLiteral("With Header and Footer")));
    auto* headerList = new AntList(page);
    headerList->setBordered(true);
    headerList->setFixedWidth(480);

    auto* header = new AntTypography(QStringLiteral("Header"), headerList);
    header->setTitle(true);
    header->setTitleLevel(Ant::TypographyTitleLevel::H5);
    header->setFixedHeight(40);
    headerList->setHeaderWidget(header);

    auto* footer = new AntTypography(QStringLiteral("Footer"), headerList);
    footer->setType(Ant::TypographyType::Secondary);
    footer->setFixedHeight(40);
    headerList->setFooterWidget(footer);

    for (int i = 1; i <= 3; ++i)
    {
        auto* item = new AntListItem(headerList);
        auto* meta = new AntListItemMeta(item);
        meta->setTitle(QStringLiteral("Item %1").arg(i));
        item->setMeta(meta);
        headerList->addItem(item);
    }

    layout->addWidget(headerList);

    layout->addWidget(createSectionTitle(QStringLiteral("Split and Size")));
    auto* splitRow = new QHBoxLayout();
    splitRow->setSpacing(28);

    auto* noSplit = new AntList(page);
    noSplit->setBordered(true);
    noSplit->setSplit(false);
    noSplit->setFixedWidth(240);
    for (int i = 1; i <= 3; ++i)
    {
        auto* item = new AntListItem(noSplit);
        auto* meta = new AntListItemMeta(item);
        meta->setTitle(QStringLiteral("No Split %1").arg(i));
        item->setMeta(meta);
        noSplit->addItem(item);
    }

    auto* smallList = new AntList(page);
    smallList->setBordered(true);
    smallList->setListSize(AntList::Small);
    smallList->setFixedWidth(240);
    for (int i = 1; i <= 3; ++i)
    {
        auto* item = new AntListItem(smallList);
        auto* meta = new AntListItemMeta(item);
        meta->setTitle(QStringLiteral("Small Item %1").arg(i));
        item->setMeta(meta);
        smallList->addItem(item);
    }

    auto* largeList = new AntList(page);
    largeList->setBordered(true);
    largeList->setListSize(AntList::Large);
    largeList->setFixedWidth(240);
    for (int i = 1; i <= 3; ++i)
    {
        auto* item = new AntListItem(largeList);
        auto* meta = new AntListItemMeta(item);
        meta->setTitle(QStringLiteral("Large Item %1").arg(i));
        item->setMeta(meta);
        largeList->addItem(item);
    }

    splitRow->addWidget(noSplit);
    splitRow->addWidget(smallList);
    splitRow->addWidget(largeList);
    splitRow->addStretch();
    layout->addLayout(splitRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createStatisticPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(28);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(48);

    auto* users = new AntStatistic(page);
    users->setTitle(QStringLiteral("Active Users"));
    users->setValue(112893);
    users->setFixedWidth(160);

    auto* balance = new AntStatistic(page);
    balance->setTitle(QStringLiteral("Account Balance (CNY)"));
    balance->setValue(112893.56);
    balance->setPrecision(2);
    balance->setPrefix(QStringLiteral("¥"));
    balance->setFixedWidth(200);

    auto* items = new AntStatistic(page);
    items->setTitle(QStringLiteral("Total Items"));
    items->setValue(28);
    items->setSuffix(QStringLiteral(" items"));
    items->setFixedWidth(160);

    basicRow->addWidget(users);
    basicRow->addWidget(balance);
    basicRow->addWidget(items);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Prefix and Suffix")));
    auto* prefixRow = new QHBoxLayout();
    prefixRow->setSpacing(48);

    auto* feedback = new AntStatistic(page);
    feedback->setTitle(QStringLiteral("Feedback"));
    feedback->setValue(93);
    feedback->setSuffix(QStringLiteral("%"));
    feedback->setFixedWidth(140);

    auto* price = new AntStatistic(page);
    price->setTitle(QStringLiteral("Price"));
    price->setValue(12680.0);
    price->setPrecision(2);
    price->setPrefix(QStringLiteral("$"));
    price->setFixedWidth(180);

    auto* speed = new AntStatistic(page);
    speed->setTitle(QStringLiteral("Speed"));
    speed->setValue(3.5);
    speed->setPrecision(1);
    speed->setSuffix(QStringLiteral(" MB/s"));
    speed->setFixedWidth(160);

    prefixRow->addWidget(feedback);
    prefixRow->addWidget(price);
    prefixRow->addWidget(speed);
    prefixRow->addStretch();
    layout->addLayout(prefixRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Card Style")));
    auto* cardRow = new QHBoxLayout();
    cardRow->setSpacing(24);

    auto* card1 = new AntCard(QStringLiteral("Revenue"), page);
    card1->setFixedWidth(220);
    auto* stat1 = new AntStatistic(card1);
    stat1->setTitle(QStringLiteral("This Week"));
    stat1->setValue(58320);
    stat1->setPrefix(QStringLiteral("¥"));
    card1->bodyLayout()->addWidget(stat1);

    auto* card2 = new AntCard(QStringLiteral("Orders"), page);
    card2->setFixedWidth(220);
    auto* stat2 = new AntStatistic(card2);
    stat2->setTitle(QStringLiteral("This Month"));
    stat2->setValue(1284);
    stat2->setSuffix(QStringLiteral(" orders"));
    card2->bodyLayout()->addWidget(stat2);

    auto* card3 = new AntCard(QStringLiteral("Growth"), page);
    card3->setFixedWidth(220);
    auto* stat3 = new AntStatistic(card3);
    stat3->setTitle(QStringLiteral("Year over Year"));
    stat3->setValue(23.5);
    stat3->setPrecision(1);
    stat3->setSuffix(QStringLiteral("%"));
    card3->bodyLayout()->addWidget(stat3);

    cardRow->addWidget(card1);
    cardRow->addWidget(card2);
    cardRow->addWidget(card3);
    cardRow->addStretch();
    layout->addLayout(cardRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createDividerPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(20);

    layout->addWidget(createSectionTitle(QStringLiteral("Horizontal")));
    layout->addWidget(new AntTypography(QStringLiteral("Ant Design")));
    layout->addWidget(new AntDivider());
    layout->addWidget(new AntTypography(QStringLiteral("Qt Widgets")));

    layout->addWidget(createSectionTitle(QStringLiteral("With Text")));
    layout->addWidget(new AntDivider(QStringLiteral("Center Text")));
    auto* start = new AntDivider(QStringLiteral("Start Text"));
    start->setTitlePlacement(Ant::DividerTitlePlacement::Start);
    layout->addWidget(start);
    auto* end = new AntDivider(QStringLiteral("End Text"));
    end->setTitlePlacement(Ant::DividerTitlePlacement::End);
    end->setPlain(false);
    layout->addWidget(end);

    layout->addWidget(createSectionTitle(QStringLiteral("Variant and Size")));
    auto* dashed = new AntDivider(QStringLiteral("Dashed"));
    dashed->setVariant(Ant::DividerVariant::Dashed);
    dashed->setDividerSize(Ant::DividerSize::Small);
    layout->addWidget(dashed);
    auto* dotted = new AntDivider(QStringLiteral("Dotted"));
    dotted->setVariant(Ant::DividerVariant::Dotted);
    dotted->setDividerSize(Ant::DividerSize::Middle);
    layout->addWidget(dotted);

    layout->addWidget(createSectionTitle(QStringLiteral("Vertical")));
    auto* verticalRow = new QHBoxLayout();
    verticalRow->setSpacing(0);
    verticalRow->addWidget(new AntTypography(QStringLiteral("Text")));
    auto* v1 = new AntDivider();
    v1->setOrientation(Ant::DividerOrientation::Vertical);
    verticalRow->addWidget(v1);
    verticalRow->addWidget(new AntTypography(QStringLiteral("Link")));
    auto* v2 = new AntDivider();
    v2->setOrientation(Ant::DividerOrientation::Vertical);
    v2->setVariant(Ant::DividerVariant::Dashed);
    verticalRow->addWidget(v2);
    verticalRow->addWidget(new AntTypography(QStringLiteral("Action")));
    verticalRow->addStretch();
    layout->addLayout(verticalRow);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::wrapPage(QWidget* page)
{
    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setVerticalScrollBar(new AntScrollBar(Qt::Vertical));
    scroll->setHorizontalScrollBar(new AntScrollBar(Qt::Horizontal));
    scroll->setWidget(page);
    return scroll;
}

AntTypography* ExampleWindow::createSectionTitle(const QString& title)
{
    auto* typo = new AntTypography(title);
    typo->setTitle(true);
    typo->setTitleLevel(Ant::TypographyTitleLevel::H5);
    return typo;
}

void ExampleWindow::addNavButton(const QString& text, int pageIndex)
{
    auto* button = new AntButton(text);
    button->setButtonType(Ant::ButtonType::Text);
    button->setBlock(true);
    connect(button, &AntButton::clicked, this, [this, pageIndex]() {
        m_stack->setCurrentIndex(pageIndex);
    });
    m_navLayout->insertWidget(m_navLayout->count() - 1, button);
}

QWidget* ExampleWindow::createTimelinePage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basic = new AntTimeline();
    basic->addItem(QStringLiteral("Create services"), QStringLiteral("2015-09-01"));
    basic->addItem(QStringLiteral("Solve initial network problems"), QStringLiteral("2015-09-01"));
    basic->addItem(QStringLiteral("Technical testing"), QStringLiteral("2015-09-01"));
    basic->addItem(QStringLiteral("Network problems being solved"), QStringLiteral("2015-09-01"));
    layout->addWidget(basic);

    layout->addWidget(createSectionTitle(QStringLiteral("Color")));
    auto* color = new AntTimeline();
    color->addItem(QStringLiteral("Create services"), QString(), QStringLiteral("green"));
    color->addItem(QStringLiteral("Solve initial network problems"), QString(), QStringLiteral("red"));
    color->addItem(QStringLiteral("Technical testing"), QString(), QStringLiteral("blue"));
    color->addItem(QStringLiteral("Network problems being solved"), QString(), QStringLiteral("gray"));
    layout->addWidget(color);

    layout->addWidget(createSectionTitle(QStringLiteral("Filled")));
    auto* filled = new AntTimeline();
    filled->setDotVariant(Ant::TimelineDotVariant::Filled);
    filled->addItem(QStringLiteral("Create services"), QStringLiteral("2015-09-01"), QStringLiteral("green"));
    filled->addItem(QStringLiteral("Solve initial network problems"), QStringLiteral("2015-09-01"), QStringLiteral("red"));
    filled->addItem(QStringLiteral("Technical testing"), QStringLiteral("2015-09-01"), QStringLiteral("blue"));
    layout->addWidget(filled);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createSpacePage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Horizontal (Small)")));
    auto* hSmall = new AntSpace();
    hSmall->setSize(Ant::SpaceSize::Small);
    for (int i = 0; i < 4; ++i)
    {
        auto* btn = new AntButton(QStringLiteral("Button %1").arg(i + 1));
        btn->setButtonType(Ant::ButtonType::Primary);
        hSmall->addItem(btn);
    }
    layout->addWidget(hSmall);

    layout->addWidget(createSectionTitle(QStringLiteral("Horizontal (Middle)")));
    auto* hMiddle = new AntSpace();
    hMiddle->setSize(Ant::SpaceSize::Middle);
    for (int i = 0; i < 4; ++i)
    {
        auto* btn = new AntButton(QStringLiteral("Button %1").arg(i + 1));
        hMiddle->addItem(btn);
    }
    layout->addWidget(hMiddle);

    layout->addWidget(createSectionTitle(QStringLiteral("Horizontal (Large)")));
    auto* hLarge = new AntSpace();
    hLarge->setSize(Ant::SpaceSize::Large);
    for (int i = 0; i < 3; ++i)
    {
        auto* btn = new AntButton(QStringLiteral("Button %1").arg(i + 1));
        hLarge->addItem(btn);
    }
    layout->addWidget(hLarge);

    layout->addWidget(createSectionTitle(QStringLiteral("Vertical")));
    auto* vSpace = new AntSpace();
    vSpace->setOrientation(Ant::SpaceOrientation::Vertical);
    vSpace->setSize(Ant::SpaceSize::Middle);
    for (int i = 0; i < 3; ++i)
    {
        auto* btn = new AntButton(QStringLiteral("Button %1").arg(i + 1));
        btn->setButtonType(Ant::ButtonType::Primary);
        btn->setBlock(true);
        vSpace->addItem(btn);
    }
    layout->addWidget(vSpace);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createLayoutPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic Layout")));

    auto* basicLayout = new AntLayout();
    basicLayout->setFixedHeight(240);

    auto* header = new AntLayoutHeader();
    header->setStyleSheet(QStringLiteral("background: %1;").arg(antTheme->tokens().colorPrimary.name()));
    auto* headerLabel = new QLabel(QStringLiteral("Header"), header);
    headerLabel->setStyleSheet(QStringLiteral("color: white; padding: 16px; font-weight: bold;"));
    headerLabel->setGeometry(16, 0, 200, 64);
    basicLayout->setHeader(header);

    auto* content = new AntLayoutContent();
    content->setStyleSheet(QStringLiteral("background: %1; padding: 24px;").arg(antTheme->tokens().colorBgContainer.name()));
    auto* contentLabel = new QLabel(QStringLiteral("Content"), content);
    contentLabel->setGeometry(16, 16, 200, 30);
    basicLayout->setContent(content);

    auto* footer = new AntLayoutFooter();
    footer->setStyleSheet(QStringLiteral("background: %1;").arg(antTheme->tokens().colorBgLayout.name()));
    auto* footerLabel = new QLabel(QStringLiteral("Footer"), footer);
    footerLabel->setStyleSheet(QStringLiteral("padding: 12px;"));
    footerLabel->setGeometry(16, 0, 200, 48);
    basicLayout->setFooter(footer);

    layout->addWidget(basicLayout);

    layout->addWidget(createSectionTitle(QStringLiteral("With Sider")));

    auto* siderLayout = new AntLayout();
    siderLayout->setFixedHeight(240);

    auto* siderHeader = new AntLayoutHeader();
    siderHeader->setStyleSheet(QStringLiteral("background: %1;").arg(antTheme->tokens().colorPrimary.name()));
    auto* siderHeaderLabel = new QLabel(QStringLiteral("Header"), siderHeader);
    siderHeaderLabel->setStyleSheet(QStringLiteral("color: white; padding: 16px; font-weight: bold;"));
    siderHeaderLabel->setGeometry(16, 0, 200, 64);
    siderLayout->setHeader(siderHeader);

    auto* sider = new AntLayoutSider();
    sider->setWidth(200);
    sider->setSiderTheme(Ant::LayoutSiderTheme::Light);
    sider->setStyleSheet(QStringLiteral("background: %1;").arg(antTheme->tokens().colorBgContainer.name()));
    auto* siderLabel = new QLabel(QStringLiteral("Sider"), sider);
    siderLabel->setStyleSheet(QStringLiteral("padding: 16px;"));
    siderLabel->setGeometry(0, 8, 200, 30);
    siderLayout->addSider(sider);

    auto* siderContent = new AntLayoutContent();
    siderContent->setStyleSheet(QStringLiteral("background: %1;").arg(antTheme->tokens().colorBgContainer.name()));
    auto* siderContentLabel = new QLabel(QStringLiteral("Content"), siderContent);
    siderContentLabel->setStyleSheet(QStringLiteral("padding: 16px;"));
    siderContentLabel->setGeometry(16, 16, 200, 30);
    siderLayout->setContent(siderContent);

    layout->addWidget(siderLayout);
    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createTypographyPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Title Levels")));
    for (int i = 1; i <= 5; ++i)
    {
        auto* title = new AntTypography(QStringLiteral("Ant Design Title Level %1").arg(i));
        title->setTitle(true);
        title->setTitleLevel(static_cast<Ant::TypographyTitleLevel>(i - 1));
        layout->addWidget(title);
    }

    layout->addWidget(createSectionTitle(QStringLiteral("Text Types")));
    auto* typesRow = new QHBoxLayout();
    typesRow->setSpacing(16);
    auto* primary = new AntTypography(QStringLiteral("Primary Text"));
    auto* secondary = new AntTypography(QStringLiteral("Secondary Text"));
    secondary->setType(Ant::TypographyType::Secondary);
    auto* success = new AntTypography(QStringLiteral("Success Text"));
    success->setType(Ant::TypographyType::Success);
    auto* warning = new AntTypography(QStringLiteral("Warning Text"));
    warning->setType(Ant::TypographyType::Warning);
    auto* danger = new AntTypography(QStringLiteral("Danger Text"));
    danger->setType(Ant::TypographyType::Danger);
    typesRow->addWidget(primary);
    typesRow->addWidget(secondary);
    typesRow->addWidget(success);
    typesRow->addWidget(warning);
    typesRow->addWidget(danger);
    typesRow->addStretch();
    layout->addLayout(typesRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Decorations")));
    auto* strongText = new AntTypography(QStringLiteral("Strong Text"));
    strongText->setStrong(true);
    layout->addWidget(strongText);

    auto* underlineText = new AntTypography(QStringLiteral("Underline Text"));
    underlineText->setUnderline(true);
    layout->addWidget(underlineText);

    auto* deleteText = new AntTypography(QStringLiteral("Delete Text"));
    deleteText->setDelete(true);
    layout->addWidget(deleteText);

    auto* codeText = new AntTypography(QStringLiteral("Code Text"));
    codeText->setCode(true);
    layout->addWidget(codeText);

    auto* markText = new AntTypography(QStringLiteral("Mark Text"));
    markText->setMark(true);
    layout->addWidget(markText);

    auto* italicText = new AntTypography(QStringLiteral("Italic Text"));
    italicText->setItalic(true);
    layout->addWidget(italicText);

    layout->addWidget(createSectionTitle(QStringLiteral("Paragraph")));
    auto* paragraph = new AntTypography(
        QStringLiteral("Ant Design is a design system for enterprise-level products. "
                       "Create an efficient and enjoyable work experience with the design language."));
    paragraph->setParagraph(true);
    layout->addWidget(paragraph);

    layout->addWidget(createSectionTitle(QStringLiteral("Copyable")));
    auto* copyable = new AntTypography(QStringLiteral("This text is copyable. Click to copy!"));
    copyable->setCopyable(true);
    layout->addWidget(copyable);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createTablePage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 16, 24, 16);
    layout->setSpacing(16);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic Table")));

    auto* table = new AntTable(page);
    table->setBordered(true);
    table->addColumn({QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 150});
    table->addColumn({QStringLiteral("Age"), QStringLiteral("age"), QStringLiteral("age"), 80});
    table->addColumn({QStringLiteral("Address"), QStringLiteral("address"), QStringLiteral("address"), 250});

    QVector<AntTableRow> rows;
    rows.push_back({{{QStringLiteral("name"), QStringLiteral("John Brown")},
                     {QStringLiteral("age"), 32},
                     {QStringLiteral("address"), QStringLiteral("New York No. 1 Lake Park")}}});
    rows.push_back({{{QStringLiteral("name"), QStringLiteral("Jim Green")},
                     {QStringLiteral("age"), 42},
                     {QStringLiteral("address"), QStringLiteral("London No. 1 Bridge Street")}}});
    rows.push_back({{{QStringLiteral("name"), QStringLiteral("Joe Black")},
                     {QStringLiteral("age"), 28},
                     {QStringLiteral("address"), QStringLiteral("Sidney No. 1 Oxford Road")}}});
    rows.push_back({{{QStringLiteral("name"), QStringLiteral("Jane White")},
                     {QStringLiteral("age"), 35},
                     {QStringLiteral("address"), QStringLiteral("Tokyo No. 2 Cherry Lane")}}});
    table->setRows(rows);
    layout->addWidget(table);

    layout->addWidget(createSectionTitle(QStringLiteral("Selection Table")));

    auto* selTable = new AntTable(page);
    selTable->setRowSelection(Ant::TableSelectionMode::Checkbox);
    selTable->setBordered(true);
    selTable->addColumn({QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 150});
    selTable->addColumn({QStringLiteral("Age"), QStringLiteral("age"), QStringLiteral("age"), 80});
    selTable->setRows(rows);
    layout->addWidget(selTable);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createTreePage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 16, 24, 16);
    layout->setSpacing(16);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic Tree")));

    auto* tree = new AntTree(page);
    tree->setShowLine(true);
    tree->setCheckable(true);

    QVector<AntTreeNode> treeData;
    AntTreeNode parent1;
    parent1.key = QStringLiteral("parent1");
    parent1.title = QStringLiteral("Parent 1");
    parent1.expanded = true;
    parent1.children.push_back({QStringLiteral("child1-1"), QStringLiteral("Child 1-1")});
    parent1.children.push_back({QStringLiteral("child1-2"), QStringLiteral("Child 1-2")});
    treeData.push_back(parent1);

    AntTreeNode parent2;
    parent2.key = QStringLiteral("parent2");
    parent2.title = QStringLiteral("Parent 2");
    parent2.children.push_back({QStringLiteral("child2-1"), QStringLiteral("Child 2-1")});
    treeData.push_back(parent2);

    tree->setTreeData(treeData);
    layout->addWidget(tree);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createUploadPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 16, 24, 16);
    layout->setSpacing(16);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic Upload")));

    auto* upload = new AntUpload(page);
    upload->addFile({QStringLiteral("file1"), QStringLiteral("document.pdf"), Ant::UploadFileStatus::Done, 100});
    upload->addFile({QStringLiteral("file2"), QStringLiteral("image.png"), Ant::UploadFileStatus::Uploading, 45});
    layout->addWidget(upload);

    layout->addWidget(createSectionTitle(QStringLiteral("Picture Card")));

    auto* picUpload = new AntUpload(page);
    picUpload->setListType(Ant::UploadListType::PictureCard);
    layout->addWidget(picUpload);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createCascaderPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 16, 24, 16);
    layout->setSpacing(16);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic Cascader")));

    auto* cascader = new AntCascader(page);
    cascader->setPlaceholder(QStringLiteral("Please select"));

    QVector<AntCascaderOption> options;
    AntCascaderOption zhejiang;
    zhejiang.value = QStringLiteral("zhejiang");
    zhejiang.label = QStringLiteral("Zhejiang");
    zhejiang.isLeaf = false;
    AntCascaderOption hangzhou;
    hangzhou.value = QStringLiteral("hangzhou");
    hangzhou.label = QStringLiteral("Hangzhou");
    hangzhou.isLeaf = false;
    hangzhou.children.push_back({QStringLiteral("xihu"), QStringLiteral("West Lake")});
    hangzhou.children.push_back({QStringLiteral("xiasha"), QStringLiteral("Xia Sha")});
    zhejiang.children.push_back(hangzhou);
    options.push_back(zhejiang);

    AntCascaderOption jiangsu;
    jiangsu.value = QStringLiteral("jiangsu");
    jiangsu.label = QStringLiteral("Jiangsu");
    jiangsu.isLeaf = false;
    jiangsu.children.push_back({QStringLiteral("nanjing"), QStringLiteral("Nanjing")});
    options.push_back(jiangsu);

    cascader->setOptions(options);
    layout->addWidget(cascader);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createTreeSelectPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 16, 24, 16);
    layout->setSpacing(16);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic TreeSelect")));

    auto* treeSelect = new AntTreeSelect(page);
    treeSelect->setPlaceholder(QStringLiteral("Please select"));

    QVector<AntTreeNode> treeData;
    AntTreeNode parent1;
    parent1.key = QStringLiteral("node1");
    parent1.title = QStringLiteral("Node 1");
    parent1.children.push_back({QStringLiteral("node1-1"), QStringLiteral("Node 1-1")});
    parent1.children.push_back({QStringLiteral("node1-2"), QStringLiteral("Node 1-2")});
    treeData.push_back(parent1);

    AntTreeNode parent2;
    parent2.key = QStringLiteral("node2");
    parent2.title = QStringLiteral("Node 2");
    parent2.children.push_back({QStringLiteral("node2-1"), QStringLiteral("Node 2-1")});
    treeData.push_back(parent2);

    treeSelect->setTreeData(treeData);
    layout->addWidget(treeSelect);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createWindowPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 16, 24, 16);
    layout->setSpacing(16);

    layout->addWidget(createSectionTitle(QStringLiteral("AntWindow - Frameless Window")));

    auto* desc = new AntTypography(QStringLiteral("AntWindow is a frameless window with custom title bar, "
                                                  "draggable, minimize/maximize/close buttons with Ant Design styling. "
                                                  "Click the button below to open a demo window."));
    desc->setParagraph(true);
    layout->addWidget(desc);

    auto* openBtn = new AntButton(QStringLiteral("Open AntWindow"));
    openBtn->setButtonType(Ant::ButtonType::Primary);
    connect(openBtn, &AntButton::clicked, this, [this]() {
        auto* window = new AntWindow();
        window->setWindowTitle(QStringLiteral("AntWindow Demo"));
        window->resize(600, 400);

        auto* central = new QWidget();
        auto* centralLayout = new QVBoxLayout(central);
        centralLayout->setContentsMargins(16, 16, 16, 16);
        auto* label = new AntTypography(QStringLiteral("This is an AntWindow with frameless design.\n"
                                                       "You can drag the title bar to move the window.\n"
                                                       "Double-click the title bar to maximize/restore."));
        label->setParagraph(true);
        centralLayout->addWidget(label);
        centralLayout->addStretch();
        window->setCentralWidget(central);

        window->setAttribute(Qt::WA_DeleteOnClose);
        window->show();
    });
    layout->addWidget(openBtn);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createDrawerPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 16, 24, 16);
    layout->setSpacing(16);

    layout->addWidget(createSectionTitle(QStringLiteral("AntDrawer - Sliding Panel")));

    auto* desc = new AntTypography(QStringLiteral("AntDrawer is a sliding panel that overlays from the edge of the screen. "
                                                  "Supports Left/Right/Top/Bottom placement with animation."));
    desc->setParagraph(true);
    layout->addWidget(desc);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(8);

    auto* rightBtn = new AntButton(QStringLiteral("Open Right Drawer"));
    rightBtn->setButtonType(Ant::ButtonType::Primary);
    auto* drawer = new AntDrawer(page);
    drawer->setTitle(QStringLiteral("Drawer Title"));
    drawer->setPlacement(Ant::DrawerPlacement::Right);
    connect(rightBtn, &AntButton::clicked, drawer, &AntDrawer::open);
    btnLayout->addWidget(rightBtn);

    auto* leftBtn = new AntButton(QStringLiteral("Open Left Drawer"));
    leftBtn->setButtonType(Ant::ButtonType::Default);
    auto* leftDrawer = new AntDrawer(page);
    leftDrawer->setTitle(QStringLiteral("Left Drawer"));
    leftDrawer->setPlacement(Ant::DrawerPlacement::Left);
    connect(leftBtn, &AntButton::clicked, leftDrawer, &AntDrawer::open);
    btnLayout->addWidget(leftBtn);

    auto* bottomBtn = new AntButton(QStringLiteral("Open Bottom Drawer"));
    bottomBtn->setButtonType(Ant::ButtonType::Default);
    auto* bottomDrawer = new AntDrawer(page);
    bottomDrawer->setTitle(QStringLiteral("Bottom Drawer"));
    bottomDrawer->setPlacement(Ant::DrawerPlacement::Bottom);
    bottomDrawer->setDrawerHeight(250);
    connect(bottomBtn, &AntButton::clicked, bottomDrawer, &AntDrawer::open);
    btnLayout->addWidget(bottomBtn);

    layout->addLayout(btnLayout);
    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createStatusBarPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 16, 24, 16);
    layout->setSpacing(16);

    layout->addWidget(createSectionTitle(QStringLiteral("AntStatusBar")));

    auto* desc = new AntTypography(QStringLiteral("AntStatusBar displays status information at the bottom of a window. "
                                                  "Supports regular items, permanent items, message text, and size grip."));
    desc->setParagraph(true);
    layout->addWidget(desc);

    auto* statusBar = new AntStatusBar(page);
    statusBar->addItem(QStringLiteral("Ready"));
    statusBar->addItem(QStringLiteral("Line 1, Col 1"));
    statusBar->addPermanentItem(QStringLiteral("UTF-8"));
    statusBar->addPermanentItem(QStringLiteral("LF"));
    statusBar->setMessage(QStringLiteral("File saved successfully"));
    layout->addWidget(statusBar);

    auto* statusBar2 = new AntStatusBar(page);
    statusBar2->addItem(QStringLiteral("Connected"));
    statusBar2->addItem(QStringLiteral("3 warnings"));
    statusBar2->addPermanentItem(QStringLiteral("v1.0.0"));
    statusBar2->setSizeGrip(false);
    layout->addWidget(statusBar2);

    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::createScrollBarPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 16, 24, 16);
    layout->setSpacing(16);

    layout->addWidget(createSectionTitle(QStringLiteral("AntScrollBar")));

    auto* desc = new AntTypography(QStringLiteral("AntScrollBar is a custom scrollbar with thin (8px) rounded handle, "
                                                  "auto-hide behavior, and transparent groove. No arrow buttons."));
    desc->setParagraph(true);
    layout->addWidget(desc);

    auto* scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBar(new AntScrollBar(Qt::Vertical));
    scrollArea->setHorizontalScrollBar(new AntScrollBar(Qt::Horizontal));

    auto* scrollContent = new QWidget();
    auto* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(8, 8, 8, 8);
    scrollLayout->setSpacing(8);
    for (int i = 0; i < 30; ++i)
    {
        auto* item = new AntTypography(QStringLiteral("Scroll item %1").arg(i + 1));
        item->setFixedHeight(32);
        scrollLayout->addWidget(item);
    }
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea, 1);

    return page;
}

QWidget* ExampleWindow::createRatePage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basic = new AntRate();
    basic->setValue(3.0);
    layout->addWidget(basic);

    layout->addWidget(createSectionTitle(QStringLiteral("Half Star")));
    auto* half = new AntRate();
    half->setAllowHalf(true);
    half->setValue(2.5);
    layout->addWidget(half);

    layout->addWidget(createSectionTitle(QStringLiteral("Size")));
    auto* sizeRow = new QHBoxLayout();
    sizeRow->setSpacing(16);
    auto* smallRate = new AntRate();
    smallRate->setRateSize(Ant::RateSize::Small);
    smallRate->setValue(3.0);
    auto* middleRate = new AntRate();
    middleRate->setValue(3.0);
    auto* largeRate = new AntRate();
    largeRate->setRateSize(Ant::RateSize::Large);
    largeRate->setValue(3.0);
    sizeRow->addWidget(smallRate);
    sizeRow->addWidget(middleRate);
    sizeRow->addWidget(largeRate);
    sizeRow->addStretch();
    layout->addLayout(sizeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Disabled")));
    auto* disabled = new AntRate();
    disabled->setDisabled(true);
    disabled->setValue(2.0);
    layout->addWidget(disabled);

    layout->addWidget(createSectionTitle(QStringLiteral("Allow Clear")));
    auto* clearRow = new QHBoxLayout();
    clearRow->setSpacing(16);
    auto* clearEnabled = new AntRate();
    clearEnabled->setValue(3.0);
    clearEnabled->setAllowClear(true);
    auto* clearDisabled = new AntRate();
    clearDisabled->setValue(3.0);
    clearDisabled->setAllowClear(false);
    clearRow->addWidget(clearEnabled);
    clearRow->addWidget(clearDisabled);
    clearRow->addStretch();
    layout->addLayout(clearRow);

    layout->addStretch();
    return page;
}

QSize ExampleWindow::sizeHint() const
{
    return QSize(1200, 800);
}

void ExampleWindow::applyTheme()
{
    m_themeButton->setText(antTheme->themeMode() == Ant::ThemeMode::Dark
        ? QStringLiteral("Light") : QStringLiteral("Dark"));
}
