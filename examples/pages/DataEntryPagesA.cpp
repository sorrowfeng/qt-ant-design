#include "Pages.h"

#include <QColor>
#include <QDate>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QTime>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "PageCommon.h"
#include "core/AntTypes.h"
#include "widgets/AntCard.h"
#include "widgets/AntAutoComplete.h"
#include "widgets/AntButton.h"
#include "widgets/AntCascader.h"
#include "widgets/AntCheckBox.h"
#include "widgets/AntColorPicker.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntForm.h"
#include "widgets/AntInput.h"
#include "widgets/AntInputNumber.h"
#include "widgets/AntMentions.h"
#include "widgets/AntPlainTextEdit.h"
#include "widgets/AntSelect.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntTypography.h"

namespace example::pages
{
QWidget* createAutoCompletePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    auto* card = new AntCard(QStringLiteral("Basic"));
    auto* cl = card->bodyLayout();
    cl->setAlignment(Qt::AlignTop);

    auto* ac = new AntAutoComplete(card);
    ac->setFixedWidth(280);
    ac->setPlaceholderText(QStringLiteral("Type to search..."));
    ac->addSuggestion(QStringLiteral("a"));
    ac->addSuggestion(QStringLiteral("aa"));
    ac->addSuggestion(QStringLiteral("a!"));
    cl->addWidget(ac, 0, Qt::AlignLeft);
    layout->addWidget(card);

    layout->addStretch();
    return page;
}

QWidget* createCascaderPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* cascader = new AntCascader(card);
        cascader->setFixedWidth(280);
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
        zhejiang.children.push_back(hangzhou);
        options.push_back(zhejiang);

        AntCascaderOption jiangsu;
        jiangsu.value = QStringLiteral("jiangsu");
        jiangsu.label = QStringLiteral("Jiangsu");
        jiangsu.isLeaf = false;
        AntCascaderOption nanjing;
        nanjing.value = QStringLiteral("nanjing");
        nanjing.label = QStringLiteral("Nanjing");
        nanjing.isLeaf = false;
        nanjing.children.push_back({QStringLiteral("zhonghua"), QStringLiteral("Zhonghua")});
        jiangsu.children.push_back(nanjing);
        options.push_back(jiangsu);

        cascader->setOptions(options);
        cl->addWidget(cascader, 0, Qt::AlignLeft);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createCheckboxPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(16);
        auto* apple = new AntCheckBox(QStringLiteral("Apple"));
        auto* banana = new AntCheckBox(QStringLiteral("Banana"));
        banana->setChecked(true);
        auto* orange = new AntCheckBox(QStringLiteral("Orange"));
        basicRow->addWidget(apple);
        basicRow->addWidget(banana);
        basicRow->addWidget(orange);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Group"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* groupRow = new QHBoxLayout();
        groupRow->setSpacing(16);
        auto* apple = new AntCheckBox(QStringLiteral("Apple"));
        apple->setChecked(true);
        auto* banana = new AntCheckBox(QStringLiteral("Banana"));
        auto* orange = new AntCheckBox(QStringLiteral("Orange"));
        groupRow->addWidget(apple);
        groupRow->addWidget(banana);
        groupRow->addWidget(orange);
        groupRow->addStretch();
        cl->addLayout(groupRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createColorPickerPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* row = new QHBoxLayout();
        row->setSpacing(20);
        auto* picker = new AntColorPicker(QColor(QStringLiteral("#1677ff")), card);
        row->addWidget(picker);
        row->addWidget(makeText(QStringLiteral("Pick a color"), card));
        row->addStretch();
        cl->addLayout(row);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Text"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* picker = new AntColorPicker(QColor(QStringLiteral("#52c41a")), card);
        picker->setShowText(true);
        cl->addWidget(picker, 0, Qt::AlignLeft);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createDatePickerPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(24);
        auto* basic = new AntDatePicker();
        auto* range = new AntDatePicker();
        range->setRangeMode(true);
        basicRow->addWidget(basic);
        basicRow->addWidget(range);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Sizes"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* sizeRow = new QHBoxLayout();
        sizeRow->setSpacing(12);
        auto* large = new AntDatePicker();
        large->setPickerSize(Ant::Size::Large);
        large->setPlaceholderText(QStringLiteral("Large"));
        auto* middle = new AntDatePicker();
        middle->setPlaceholderText(QStringLiteral("Middle"));
        auto* small = new AntDatePicker();
        small->setPickerSize(Ant::Size::Small);
        small->setPlaceholderText(QStringLiteral("Small"));
        sizeRow->addWidget(large);
        sizeRow->addWidget(middle);
        sizeRow->addWidget(small);
        sizeRow->addStretch();
        cl->addLayout(sizeRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createFormPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    auto* card = new AntCard(QStringLiteral("Basic Form"));
    auto* cl = card->bodyLayout();
    cl->setAlignment(Qt::AlignTop);

    auto* form = new AntForm(card);
    form->setFixedWidth(480);
    form->setLabelWidth(112);
    form->setItemSpacing(24);

    auto* username = new AntInput(form);
    username->setPlaceholderText(QStringLiteral("Enter username"));
    form->addItem(QStringLiteral("Username"), username, true);

    auto* password = new AntInput(form);
    password->setPasswordMode(true);
    password->setPlaceholderText(QStringLiteral("Enter password"));
    form->addItem(QStringLiteral("Password"), password, true);

    auto* gender = new AntSelect(form);
    gender->setPlaceholderText(QStringLiteral("Select"));
    gender->addOptions({QStringLiteral("Male"), QStringLiteral("Female")});
    form->addItem(QStringLiteral("Gender"), gender);

    auto* remember = new AntCheckBox(QStringLiteral("Remember me"), form);
    form->addItem(QStringLiteral("Remember"), remember);

    auto* submit = new AntButton(QStringLiteral("Submit"), form);
    submit->setButtonType(Ant::ButtonType::Primary);
    submit->setFixedWidth(submit->sizeHint().width());
    auto* submitItem = form->addItem(QStringLiteral(" "), submit);
    submitItem->setColon(false);

    cl->addWidget(form, 0, Qt::AlignLeft);
    layout->addWidget(card);

    layout->addStretch();
    return page;
}

QWidget* createInputPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(12);
        auto* basic = new AntInput();
        basic->setPlaceholderText(QStringLiteral("Basic input"));
        basic->setFixedWidth(240);
        auto* disabled = new AntInput();
        disabled->setPlaceholderText(QStringLiteral("Disabled"));
        disabled->setFixedWidth(240);
        disabled->setEnabled(false);
        basicRow->addWidget(basic);
        basicRow->addWidget(disabled);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Sizes"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* sizeRow = new QHBoxLayout();
        sizeRow->setSpacing(12);
        auto* large = new AntInput();
        large->setPlaceholderText(QStringLiteral("Large"));
        large->setInputSize(Ant::Size::Large);
        large->setFixedWidth(180);
        auto* middle = new AntInput();
        middle->setPlaceholderText(QStringLiteral("Middle"));
        middle->setFixedWidth(180);
        auto* small = new AntInput();
        small->setPlaceholderText(QStringLiteral("Small"));
        small->setInputSize(Ant::Size::Small);
        small->setFixedWidth(180);
        sizeRow->addWidget(large);
        sizeRow->addWidget(middle);
        sizeRow->addWidget(small);
        sizeRow->addStretch();
        cl->addLayout(sizeRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Search & Password"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* searchRow = new QHBoxLayout();
        searchRow->setSpacing(12);
        auto* search = new AntInput();
        search->setPlaceholderText(QStringLiteral("Search"));
        search->setSearchMode(true);
        search->setAllowClear(true);
        search->setFixedWidth(280);
        auto* password = new AntInput();
        password->setPlaceholderText(QStringLiteral("Password"));
        password->setPasswordMode(true);
        password->setFixedWidth(200);
        searchRow->addWidget(search);
        searchRow->addWidget(password);
        searchRow->addStretch();
        cl->addLayout(searchRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("TextArea"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* textArea = new AntPlainTextEdit(card);
        textArea->setPlaceholderText(QStringLiteral("Enter text..."));
        textArea->setFixedSize(400, 78);
        textArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        textArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        cl->addWidget(textArea, 0, Qt::AlignLeft);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Addon"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* addon = new AntInput();
        addon->setAddonBefore(QStringLiteral("http://"));
        addon->setAddonAfter(QStringLiteral(".com"));
        addon->setText(QStringLiteral("mysite"));
        addon->setFixedWidth(300);
        cl->addWidget(addon, 0, Qt::AlignLeft);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createInputNumberPage(QWidget* owner)
{
    Q_UNUSED(owner)
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* basic = new AntInputNumber();
        basic->setRange(1, 10);
        basic->setControlsVisible(false);
        basic->setValue(3);
        basic->setFixedWidth(90);
        cl->addWidget(basic, 0, Qt::AlignLeft);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Prefix/Suffix"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* formatRow = new QHBoxLayout();
        formatRow->setSpacing(12);
        auto* currency = new AntInputNumber();
        currency->setPrefixText(QStringLiteral("$ "));
        currency->setRange(0, 100000);
        currency->setControlsVisible(false);
        currency->setValue(100);
        currency->setFixedWidth(160);
        auto* percent = new AntInputNumber();
        percent->setRange(0, 100);
        percent->setAddonAfterText(QStringLiteral("%"));
        percent->setControlsVisible(false);
        percent->setValue(50);
        percent->setFixedWidth(160);
        formatRow->addWidget(currency);
        formatRow->addWidget(percent);
        formatRow->addStretch();
        cl->addLayout(formatRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createMentionsPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    auto* card = new AntCard(QStringLiteral("Basic"));
    auto* cl = card->bodyLayout();
    cl->setAlignment(Qt::AlignTop);

    auto* mentions = new AntMentions(card);
    mentions->setPlaceholderText(QStringLiteral("Type @ to mention"));
    mentions->setSuggestions({QStringLiteral("afc163"), QStringLiteral("zombieJ")});
    mentions->setRows(3);
    mentions->setFixedWidth(400);
    cl->addWidget(mentions, 0, Qt::AlignLeft);
    layout->addWidget(card);

    layout->addStretch();
    return page;
}
}
