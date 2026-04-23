#include "ExampleWindow.h"

#include <QFrame>
#include <QDate>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QMouseEvent>
#include <QPoint>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTime>
#include <QVBoxLayout>

#include "core/AntTheme.h"
#include "widgets/AntAvatar.h"
#include "widgets/AntBadge.h"
#include "widgets/AntBreadcrumb.h"
#include "widgets/AntButton.h"
#include "widgets/AntCard.h"
#include "widgets/AntCheckbox.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntInput.h"
#include "widgets/AntMessage.h"
#include "widgets/AntMenu.h"
#include "widgets/AntNotification.h"
#include "widgets/AntPagination.h"
#include "widgets/AntProgress.h"
#include "widgets/AntRadio.h"
#include "widgets/AntSelect.h"
#include "widgets/AntSlider.h"
#include "widgets/AntSpin.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntTabs.h"
#include "widgets/AntTag.h"
#include "widgets/AntTimePicker.h"

ExampleWindow::ExampleWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground, false);

    m_central = new QWidget(this);
    auto* root = new QHBoxLayout(m_central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_sidebar = new QWidget(m_central);
    m_sidebar->setFixedWidth(220);
    auto* sideLayout = new QVBoxLayout(m_sidebar);
    sideLayout->setContentsMargins(20, 20, 20, 20);
    sideLayout->setSpacing(12);

    auto* brand = new QLabel(QStringLiteral("qt-ant-design"), m_sidebar);
    QFont brandFont = brand->font();
    brandFont.setPixelSize(20);
    brandFont.setWeight(QFont::DemiBold);
    brand->setFont(brandFont);
    sideLayout->addWidget(brand);

    m_navLayout = new QVBoxLayout();
    m_navLayout->setSpacing(8);
    sideLayout->addLayout(m_navLayout);
    sideLayout->addStretch();

    m_content = new QWidget(m_central);
    auto* contentLayout = new QVBoxLayout(m_content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_titleBar = new QWidget(m_content);
    m_titleBar->setFixedHeight(64);
    auto* titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(28, 0, 28, 0);
    auto* title = new QLabel(QStringLiteral("Ant Design Qt Widgets"), m_titleBar);
    QFont titleFont = title->font();
    titleFont.setPixelSize(18);
    titleFont.setWeight(QFont::DemiBold);
    title->setFont(titleFont);
    titleLayout->addWidget(title);
    titleLayout->addStretch();

    m_themeButton = new AntButton(QStringLiteral("Dark"), m_titleBar);
    m_themeButton->setButtonType(Ant::ButtonType::Default);
    m_themeButton->setButtonShape(Ant::ButtonShape::Round);
    connect(m_themeButton, &AntButton::clicked, antTheme, &AntTheme::toggleThemeMode);
    titleLayout->addWidget(m_themeButton);

    m_stack = new QStackedWidget(m_content);
    contentLayout->addWidget(m_titleBar);
    contentLayout->addWidget(m_stack, 1);

    m_stack->addWidget(wrapPage(createButtonPage()));
    m_stack->addWidget(wrapPage(createBreadcrumbPage()));
    m_stack->addWidget(wrapPage(createCheckboxPage()));
    m_stack->addWidget(wrapPage(createDatePickerPage()));
    m_stack->addWidget(wrapPage(createInputPage()));
    m_stack->addWidget(wrapPage(createMessagePage()));
    m_stack->addWidget(wrapPage(createMenuPage()));
    m_stack->addWidget(wrapPage(createTabsPage()));
    m_stack->addWidget(wrapPage(createBadgePage()));
    m_stack->addWidget(wrapPage(createAvatarPage()));
    m_stack->addWidget(wrapPage(createTagPage()));
    m_stack->addWidget(wrapPage(createNotificationPage()));
    m_stack->addWidget(wrapPage(createPaginationPage()));
    m_stack->addWidget(wrapPage(createProgressPage()));
    m_stack->addWidget(wrapPage(createRadioPage()));
    m_stack->addWidget(wrapPage(createSelectPage()));
    m_stack->addWidget(wrapPage(createSliderPage()));
    m_stack->addWidget(wrapPage(createSpinPage()));
    m_stack->addWidget(wrapPage(createSwitchPage()));
    m_stack->addWidget(wrapPage(createTimePickerPage()));
    m_stack->addWidget(wrapPage(createCardPage()));
    addNavButton(QStringLiteral("Button"), 0);
    addNavButton(QStringLiteral("Breadcrumb"), 1);
    addNavButton(QStringLiteral("Checkbox"), 2);
    addNavButton(QStringLiteral("DatePicker"), 3);
    addNavButton(QStringLiteral("Input"), 4);
    addNavButton(QStringLiteral("Message"), 5);
    addNavButton(QStringLiteral("Menu"), 6);
    addNavButton(QStringLiteral("Tabs"), 7);
    addNavButton(QStringLiteral("Badge"), 8);
    addNavButton(QStringLiteral("Avatar"), 9);
    addNavButton(QStringLiteral("Tag"), 10);
    addNavButton(QStringLiteral("Notification"), 11);
    addNavButton(QStringLiteral("Pagination"), 12);
    addNavButton(QStringLiteral("Progress"), 13);
    addNavButton(QStringLiteral("Radio"), 14);
    addNavButton(QStringLiteral("Select"), 15);
    addNavButton(QStringLiteral("Slider"), 16);
    addNavButton(QStringLiteral("Spin"), 17);
    addNavButton(QStringLiteral("Switch"), 18);
    addNavButton(QStringLiteral("TimePicker"), 19);
    addNavButton(QStringLiteral("Card"), 20);

    root->addWidget(m_sidebar);
    root->addWidget(m_content, 1);
    setCentralWidget(m_central);

    connect(antTheme, &AntTheme::themeChanged, this, &ExampleWindow::applyTheme);
    applyTheme();
}

void ExampleWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_titleBar->geometry().contains(m_content->mapFrom(this, event->pos())))
    {
        m_dragging = true;
        m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
        return;
    }
    QMainWindow::mousePressEvent(event);
}

void ExampleWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging)
    {
        move(event->globalPosition().toPoint() - m_dragOffset);
        event->accept();
        return;
    }
    QMainWindow::mouseMoveEvent(event);
}

void ExampleWindow::mouseReleaseEvent(QMouseEvent* event)
{
    m_dragging = false;
    QMainWindow::mouseReleaseEvent(event);
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
        auto* label = new QLabel(text);
        label->setWordWrap(true);
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
    auto* basicValue = new QLabel(QString::number(basic->value()));
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
    basic->bodyLayout()->addWidget(new QLabel(QStringLiteral("Card content\nCard content\nCard content")));

    auto* hoverable = new AntCard(QStringLiteral("Hoverable"));
    hoverable->setHoverable(true);
    hoverable->setMinimumSize(280, 180);
    hoverable->bodyLayout()->addWidget(new QLabel(QStringLiteral("Move the mouse over this card.")));

    auto* loading = new AntCard(QStringLiteral("Loading"));
    loading->setLoading(true);
    loading->setMinimumSize(280, 180);
    loading->bodyLayout()->addWidget(new QLabel(QStringLiteral("Loading mask and spinner")));
    row->addWidget(basic);
    row->addWidget(hoverable);
    row->addWidget(loading);
    layout->addLayout(row);

    auto* actionCard = new AntCard(QStringLiteral("Actions"));
    actionCard->setMinimumHeight(180);
    actionCard->bodyLayout()->addWidget(new QLabel(QStringLiteral("Card with action slots.")));
    actionCard->addActionWidget(new QLabel(QStringLiteral("Edit")));
    actionCard->addActionWidget(new QLabel(QStringLiteral("Share")));
    actionCard->addActionWidget(new QLabel(QStringLiteral("Delete")));
    layout->addWidget(actionCard);
    layout->addStretch();
    return page;
}

QWidget* ExampleWindow::wrapPage(QWidget* page)
{
    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(page);
    return scroll;
}

QLabel* ExampleWindow::createSectionTitle(const QString& title)
{
    auto* label = new QLabel(title);
    QFont font = label->font();
    font.setPixelSize(16);
    font.setWeight(QFont::DemiBold);
    label->setFont(font);
    return label;
}

void ExampleWindow::addNavButton(const QString& text, int pageIndex)
{
    auto* button = new AntButton(text, m_sidebar);
    button->setButtonType(Ant::ButtonType::Text);
    button->setBlock(true);
    connect(button, &AntButton::clicked, this, [this, pageIndex]() {
        m_stack->setCurrentIndex(pageIndex);
    });
    m_navLayout->addWidget(button);
}

void ExampleWindow::applyTheme()
{
    const auto& token = antTheme->tokens();
    m_themeButton->setText(antTheme->themeMode() == Ant::ThemeMode::Dark ? QStringLiteral("Light") : QStringLiteral("Dark"));
    setStyleSheet(QStringLiteral(
        "QMainWindow, QWidget { background: %1; color: %2; }"
        "QScrollArea { border: none; background: %1; }"
        "QScrollBar:vertical { width: 10px; background: transparent; }"
        "QScrollBar::handle:vertical { background: %3; border-radius: 5px; min-height: 32px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }")
                      .arg(token.colorBgLayout.name(QColor::HexArgb),
                           token.colorText.name(QColor::HexArgb),
                           token.colorBorder.name(QColor::HexArgb)));
    m_sidebar->setStyleSheet(QStringLiteral("QWidget { background: %1; } QLabel { color: %2; background: transparent; }")
                                 .arg(token.colorBgContainer.name(QColor::HexArgb), token.colorText.name(QColor::HexArgb)));
    m_titleBar->setStyleSheet(QStringLiteral("QWidget { background: %1; } QLabel { color: %2; background: transparent; }")
                                  .arg(token.colorBgContainer.name(QColor::HexArgb), token.colorText.name(QColor::HexArgb)));
}
