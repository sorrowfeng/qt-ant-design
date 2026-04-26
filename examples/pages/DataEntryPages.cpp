#include "Pages.h"

#include <QColor>
#include <QDate>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QTime>
#include <QVariant>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "PageCommon.h"
#include "core/AntTypes.h"
#include "widgets/AntAutoComplete.h"
#include "widgets/AntButton.h"
#include "widgets/AntCascader.h"
#include "widgets/AntCheckbox.h"
#include "widgets/AntColorPicker.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntForm.h"
#include "widgets/AntInput.h"
#include "widgets/AntInputNumber.h"
#include "widgets/AntMentions.h"
#include "widgets/AntRadio.h"
#include "widgets/AntRate.h"
#include "widgets/AntSelect.h"
#include "widgets/AntSlider.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntTimePicker.h"
#include "widgets/AntTransfer.h"
#include "widgets/AntTree.h"
#include "widgets/AntTreeSelect.h"
#include "widgets/AntTypography.h"
#include "widgets/AntUpload.h"

namespace example::pages
{
QWidget* createAutoCompletePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntAutoComplete"));
    QFont titleFont = title->font();
    titleFont.setPixelSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    auto* ac = new AntAutoComplete(page);
    ac->setPlaceholderText(QStringLiteral("Type a color..."));
    ac->addSuggestion(QStringLiteral("Red"));
    ac->addSuggestion(QStringLiteral("Blue"));
    ac->addSuggestion(QStringLiteral("Green"));
    ac->addSuggestion(QStringLiteral("Yellow"));
    ac->addSuggestion(QStringLiteral("Orange"));
    ac->addSuggestion(QStringLiteral("Purple"));
    ac->addSuggestion(QStringLiteral("Pink"));
    ac->addSuggestion(QStringLiteral("Cyan"));
    ac->addSuggestion(QStringLiteral("Magenta"));
    ac->addSuggestion(QStringLiteral("Teal"));
    layout->addWidget(ac);

    QObject::connect(ac, &AntAutoComplete::suggestionClicked, page, [](const QString& text, const QVariant&) {
        qDebug() << "Selected:" << text;
    });

    layout->addStretch();
    return page;
}

QWidget* createCascaderPage(QWidget* /*owner*/)
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

QWidget* createCheckboxPage(QWidget* owner)
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
    QObject::connect(controller, &AntCheckbox::clicked, owner, [controller, optionA, optionB]() {
        optionA->setChecked(controller->isChecked());
        optionB->setChecked(controller->isChecked());
    });
    QObject::connect(optionA, &AntCheckbox::checkedChanged, owner, updateController);
    QObject::connect(optionB, &AntCheckbox::checkedChanged, owner, updateController);
    controlledRow->addWidget(controller);
    controlledRow->addSpacing(12);
    controlledRow->addWidget(optionA);
    controlledRow->addWidget(optionB);
    controlledRow->addStretch();
    layout->addLayout(controlledRow);

    layout->addStretch();
    return page;
}

QWidget* createColorPickerPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntColorPicker"));
    QFont titleFont = title->font();
    titleFont.setPixelSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    auto* openBtn = new AntButton(QStringLiteral("Open Color Picker"));
    openBtn->setButtonType(Ant::ButtonType::Primary);
    layout->addWidget(openBtn);

    auto* colorLabel = new QLabel(QStringLiteral("Selected: none"));
    layout->addWidget(colorLabel);

    QObject::connect(openBtn, &QPushButton::clicked, page, [page, colorLabel]() {
        QColor c = AntColorPicker::getColor(Qt::white, page->window(), QStringLiteral("Pick a Color"));
        if (c.isValid())
            colorLabel->setText(QStringLiteral("Selected: ") + c.name());
    });

    layout->addStretch();
    return page;
}

QWidget* createDatePickerPage(QWidget* /*owner*/)
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

QWidget* createFormPage(QWidget* /*owner*/)
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

    layout->addWidget(createSectionTitle(QStringLiteral("Form List")));
    auto* formList = new AntFormList(page);
    formList->setMinCount(1);
    formList->setMaxCount(5);
    formList->setItemFactory([](int index) -> QWidget* {
        auto* item = new AntForm();
        item->setLabelWidth(80);
        auto* nameInput = new AntInput(item);
        nameInput->setPlaceholderText(QStringLiteral("Name #%1").arg(index + 1));
        nameInput->setText(QStringLiteral("Item %1").arg(index + 1));
        item->addItem(QStringLiteral("Name"), nameInput);
        auto* valueInput = new AntInput(item);
        valueInput->setPlaceholderText(QStringLiteral("Value"));
        item->addItem(QStringLiteral("Value"), valueInput);
        return item;
    });
    layout->addWidget(formList);

    layout->addStretch();
    return page;
}

QWidget* createInputPage(QWidget* /*owner*/)
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

    layout->addWidget(createSectionTitle(QStringLiteral("Variants")));
    auto* outlined = new AntInput();
    outlined->setPlaceholderText(QStringLiteral("Outlined (default)"));
    outlined->setVariant(Ant::InputVariant::Outlined);
    auto* filled = new AntInput();
    filled->setPlaceholderText(QStringLiteral("Filled"));
    filled->setVariant(Ant::InputVariant::Filled);
    auto* borderless = new AntInput();
    borderless->setPlaceholderText(QStringLiteral("Borderless"));
    borderless->setVariant(Ant::InputVariant::Borderless);
    auto* underlined = new AntInput();
    underlined->setPlaceholderText(QStringLiteral("Underlined"));
    underlined->setVariant(Ant::InputVariant::Underlined);
    layout->addWidget(outlined);
    layout->addWidget(filled);
    layout->addWidget(borderless);
    layout->addWidget(underlined);

    layout->addStretch();
    return page;
}

QWidget* createInputNumberPage(QWidget* owner)
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
    QObject::connect(quantity, qOverload<double>(&QDoubleSpinBox::valueChanged), owner, [summary](double value) {
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

QWidget* createMentionsPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntMentions"));
    QFont f = title->font();
    f.setPixelSize(20);
    f.setBold(true);
    title->setFont(f);
    layout->addWidget(title);

    auto* mentions = new AntMentions(page);
    mentions->setPlaceholderText(QStringLiteral("Type @ to mention teammates"));
    mentions->setSuggestions({QStringLiteral("alice"), QStringLiteral("bob"), QStringLiteral("charlie"),
                              QStringLiteral("design-team"), QStringLiteral("frontend"), QStringLiteral("release-bot")});
    layout->addWidget(mentions);

    auto* picked = new QLabel(QStringLiteral("Selected mention: none"), page);
    layout->addWidget(picked);

    QObject::connect(mentions, &AntMentions::mentionSelected, picked, [picked](const QString& text) {
        picked->setText(QStringLiteral("Selected mention: %1").arg(text));
    });

    auto* help = new QLabel(
        QStringLiteral("The popup filters suggestions after the prefix. Click one item to insert the mention and keep typing."),
        page);
    help->setWordWrap(true);
    layout->addWidget(help);
    layout->addStretch();
    return page;
}

QWidget* createRadioPage(QWidget* /*owner*/)
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

QWidget* createRatePage(QWidget* /*owner*/)
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

QWidget* createSelectPage(QWidget* /*owner*/)
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

    layout->addWidget(createSectionTitle(QStringLiteral("Multiple")));
    auto* multiRow = new QHBoxLayout();
    multiRow->setSpacing(16);
    auto* multi = new AntSelect();
    multi->setSelectMode(Ant::SelectMode::Multiple);
    multi->setPlaceholderText(QStringLiteral("Select tags"));
    multi->addOptions({QStringLiteral("Apple"), QStringLiteral("Pear"), QStringLiteral("Orange"), QStringLiteral("Banana"), QStringLiteral("Grape")});
    multi->addSelectedIndex(0);
    multi->addSelectedIndex(2);
    multi->setAllowClear(true);
    multi->setFixedWidth(320);
    multiRow->addWidget(multi);

    auto* multiMax = new AntSelect();
    multiMax->setSelectMode(Ant::SelectMode::Multiple);
    multiMax->setPlaceholderText(QStringLiteral("Max 3 tags"));
    multiMax->addOptions({QStringLiteral("Design"), QStringLiteral("Develop"), QStringLiteral("Test"), QStringLiteral("Deploy"), QStringLiteral("Review")});
    multiMax->addSelectedIndex(0);
    multiMax->addSelectedIndex(1);
    multiMax->addSelectedIndex(2);
    multiMax->addSelectedIndex(3);
    multiMax->setMaxTagCount(3);
    multiMax->setFixedWidth(320);
    multiRow->addWidget(multiMax);
    multiRow->addStretch();
    layout->addLayout(multiRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Tags")));
    auto* tagRow = new QHBoxLayout();
    tagRow->setSpacing(16);
    auto* tags = new AntSelect();
    tags->setSelectMode(Ant::SelectMode::Tags);
    tags->setPlaceholderText(QStringLiteral("Type and press Enter"));
    tags->addOptions({QStringLiteral("Tag 1"), QStringLiteral("Tag 2")});
    tags->addSelectedIndex(0);
    tags->setFixedWidth(320);
    tagRow->addWidget(tags);
    tagRow->addStretch();
    layout->addLayout(tagRow);

    layout->addStretch();
    return page;
}

QWidget* createSliderPage(QWidget* owner)
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
    QObject::connect(basic, &AntSlider::valueChanged, owner, [basicValue](int value) {
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

QWidget* createSwitchPage(QWidget* /*owner*/)
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

QWidget* createTimePickerPage(QWidget* /*owner*/)
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

QWidget* createTransferPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntTransfer"));
    QFont f = title->font();
    f.setPixelSize(20);
    f.setBold(true);
    title->setFont(f);
    layout->addWidget(title);

    auto* transfer = new AntTransfer(page);
    transfer->setSourceItems({QStringLiteral("Dashboard"), QStringLiteral("Analytics"), QStringLiteral("Billing"),
                              QStringLiteral("Settings"), QStringLiteral("Audit Log")});
    transfer->setTargetItems({QStringLiteral("Users")});
    layout->addWidget(transfer);

    auto* summary = new QLabel(page);
    auto updateSummary = [summary, transfer]() {
        summary->setText(QStringLiteral("source=%1, target=%2")
                             .arg(transfer->sourceItems().join(QStringLiteral(", ")))
                             .arg(transfer->targetItems().join(QStringLiteral(", "))));
    };
    updateSummary();
    QObject::connect(transfer, &AntTransfer::itemsChanged, summary, updateSummary);
    layout->addWidget(summary);

    layout->addStretch();
    return page;
}

QWidget* createTreeSelectPage(QWidget* /*owner*/)
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

QWidget* createUploadPage(QWidget* /*owner*/)
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

    layout->addWidget(createSectionTitle(QStringLiteral("Dragger")));

    auto* dragger = new AntUpload(page);
    dragger->setDraggerMode(true);
    dragger->setFixedHeight(180);
    layout->addWidget(dragger);

    layout->addStretch();
    return page;
}
}
