#include "ExampleWindow.h"

#include <QFrame>
#include <QDate>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTime>
#include <QVBoxLayout>

#include "core/AntTheme.h"
#include "widgets/AntButton.h"
#include "widgets/AntCard.h"
#include "widgets/AntCheckbox.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntInput.h"
#include "widgets/AntRadio.h"
#include "widgets/AntSelect.h"
#include "widgets/AntSlider.h"
#include "widgets/AntSpin.h"
#include "widgets/AntSwitch.h"
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
    m_stack->addWidget(wrapPage(createCheckboxPage()));
    m_stack->addWidget(wrapPage(createDatePickerPage()));
    m_stack->addWidget(wrapPage(createInputPage()));
    m_stack->addWidget(wrapPage(createRadioPage()));
    m_stack->addWidget(wrapPage(createSelectPage()));
    m_stack->addWidget(wrapPage(createSliderPage()));
    m_stack->addWidget(wrapPage(createSpinPage()));
    m_stack->addWidget(wrapPage(createSwitchPage()));
    m_stack->addWidget(wrapPage(createTimePickerPage()));
    m_stack->addWidget(wrapPage(createCardPage()));
    addNavButton(QStringLiteral("Button"), 0);
    addNavButton(QStringLiteral("Checkbox"), 1);
    addNavButton(QStringLiteral("DatePicker"), 2);
    addNavButton(QStringLiteral("Input"), 3);
    addNavButton(QStringLiteral("Radio"), 4);
    addNavButton(QStringLiteral("Select"), 5);
    addNavButton(QStringLiteral("Slider"), 6);
    addNavButton(QStringLiteral("Spin"), 7);
    addNavButton(QStringLiteral("Switch"), 8);
    addNavButton(QStringLiteral("TimePicker"), 9);
    addNavButton(QStringLiteral("Card"), 10);

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
