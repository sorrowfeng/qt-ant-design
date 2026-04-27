#include "Pages.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QTime>
#include <QVBoxLayout>
#include <QWidget>

#include "PageCommon.h"
#include "core/AntTypes.h"
#include "widgets/AntCard.h"
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
QWidget* createRadioPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Disabled"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(disabledRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Vertical Group"));
        auto* cl = card->bodyLayout();
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
        cl->addWidget(group);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createRatePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basic = new AntRate();
        basic->setValue(3.0);
        cl->addWidget(basic);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Half Star"));
        auto* cl = card->bodyLayout();
        auto* half = new AntRate();
        half->setAllowHalf(true);
        half->setValue(2.5);
        cl->addWidget(half);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Size"));
        auto* cl = card->bodyLayout();
        auto* sizeRow = new QHBoxLayout();
        sizeRow->setSpacing(16);
        auto* smallRate = new AntRate();
        smallRate->setRateSize(Ant::Size::Small);
        smallRate->setValue(3.0);
        auto* middleRate = new AntRate();
        middleRate->setValue(3.0);
        auto* largeRate = new AntRate();
        largeRate->setRateSize(Ant::Size::Large);
        largeRate->setValue(3.0);
        sizeRow->addWidget(smallRate);
        sizeRow->addWidget(middleRate);
        sizeRow->addWidget(largeRate);
        sizeRow->addStretch();
        cl->addLayout(sizeRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Disabled"));
        auto* cl = card->bodyLayout();
        auto* disabled = new AntRate();
        disabled->setDisabled(true);
        disabled->setValue(2.0);
        cl->addWidget(disabled);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Allow Clear"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(clearRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createSelectPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Size"));
        auto* cl = card->bodyLayout();
        auto* sizeRow = new QHBoxLayout();
        sizeRow->setSpacing(16);
        auto* large = new AntSelect();
        large->setSelectSize(Ant::Size::Large);
        large->addOptions({QStringLiteral("Large"), QStringLiteral("Option")});
        large->setCurrentIndex(0);
        auto* middle = new AntSelect();
        middle->addOptions({QStringLiteral("Middle"), QStringLiteral("Option")});
        middle->setCurrentIndex(0);
        auto* small = new AntSelect();
        small->setSelectSize(Ant::Size::Small);
        small->addOptions({QStringLiteral("Small"), QStringLiteral("Option")});
        small->setCurrentIndex(0);
        sizeRow->addWidget(large);
        sizeRow->addWidget(middle);
        sizeRow->addWidget(small);
        sizeRow->addStretch();
        cl->addLayout(sizeRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Status and Variant"));
        auto* cl = card->bodyLayout();
        auto* variantRow = new QHBoxLayout();
        variantRow->setSpacing(16);
        auto* error = new AntSelect();
        error->setStatus(Ant::Status::Error);
        error->setPlaceholderText(QStringLiteral("Error"));
        error->addOptions({QStringLiteral("Invalid option"), QStringLiteral("Valid option")});
        auto* warning = new AntSelect();
        warning->setStatus(Ant::Status::Warning);
        warning->setPlaceholderText(QStringLiteral("Warning"));
        warning->addOptions({QStringLiteral("Risky option"), QStringLiteral("Safe option")});
        auto* filled = new AntSelect();
        filled->setVariant(Ant::Variant::Filled);
        filled->addOptions({QStringLiteral("Filled"), QStringLiteral("Outlined")});
        filled->setCurrentIndex(0);
        auto* underlined = new AntSelect();
        underlined->setVariant(Ant::Variant::Underlined);
        underlined->addOptions({QStringLiteral("Underlined"), QStringLiteral("Borderless")});
        underlined->setCurrentIndex(0);
        variantRow->addWidget(error);
        variantRow->addWidget(warning);
        variantRow->addWidget(filled);
        variantRow->addWidget(underlined);
        variantRow->addStretch();
        cl->addLayout(variantRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("States"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(stateRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Multiple"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(multiRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Tags"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(tagRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createSliderPage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Dots and Step"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(dotsRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Vertical"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(verticalRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("States"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(stateRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createSwitchPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(18);
        auto* offSwitch = new AntSwitch();
        auto* onSwitch = new AntSwitch();
        onSwitch->setChecked(true);
        auto* smallSwitch = new AntSwitch();
        smallSwitch->setSwitchSize(Ant::Size::Small);
        smallSwitch->setChecked(true);
        basicRow->addWidget(offSwitch);
        basicRow->addWidget(onSwitch);
        basicRow->addWidget(smallSwitch);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Text"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(textRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("States"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(stateRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createTimePickerPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
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
        auto* range = new AntTimePicker();
        range->setRangeMode(true);
        range->setStartTime(QTime(9, 0, 0));
        range->setEndTime(QTime(18, 0, 0));
        range->setDisplayFormat(QStringLiteral("HH:mm"));
        basicRow->addWidget(basic);
        basicRow->addWidget(selected);
        basicRow->addWidget(customFormat);
        basicRow->addWidget(range);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Size"));
        auto* cl = card->bodyLayout();
        auto* sizeRow = new QHBoxLayout();
        sizeRow->setSpacing(16);
        auto* large = new AntTimePicker();
        large->setPickerSize(Ant::Size::Large);
        large->setSelectedTime(QTime::currentTime());
        auto* middle = new AntTimePicker();
        middle->setSelectedTime(QTime::currentTime());
        auto* small = new AntTimePicker();
        small->setPickerSize(Ant::Size::Small);
        small->setSelectedTime(QTime::currentTime());
        sizeRow->addWidget(large);
        sizeRow->addWidget(middle);
        sizeRow->addWidget(small);
        sizeRow->addStretch();
        cl->addLayout(sizeRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Status and Variant"));
        auto* cl = card->bodyLayout();
        auto* variantRow = new QHBoxLayout();
        variantRow->setSpacing(16);
        auto* error = new AntTimePicker();
        error->setStatus(Ant::Status::Error);
        error->setPlaceholderText(QStringLiteral("Error"));
        auto* warning = new AntTimePicker();
        warning->setStatus(Ant::Status::Warning);
        warning->setPlaceholderText(QStringLiteral("Warning"));
        auto* filled = new AntTimePicker();
        filled->setVariant(Ant::Variant::Filled);
        filled->setSelectedTime(QTime(8, 0, 0));
        auto* underlined = new AntTimePicker();
        underlined->setVariant(Ant::Variant::Underlined);
        underlined->setSelectedTime(QTime(18, 45, 0));
        variantRow->addWidget(error);
        variantRow->addWidget(warning);
        variantRow->addWidget(filled);
        variantRow->addWidget(underlined);
        variantRow->addStretch();
        cl->addLayout(variantRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("States and Step"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(stateRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createTransferPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new AntTypography(QStringLiteral("AntTransfer"));
    title->setTitle(true);
    title->setTitleLevel(Ant::TypographyTitleLevel::H3);
    layout->addWidget(title);

    auto* transfer = new AntTransfer(page);
    transfer->setSourceItems({QStringLiteral("Dashboard"), QStringLiteral("Analytics"), QStringLiteral("Billing"),
                              QStringLiteral("Settings"), QStringLiteral("Audit Log")});
    transfer->setTargetItems({QStringLiteral("Users")});
    layout->addWidget(transfer);

    auto* summary = makeText(QString(), page);
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

    {
        auto* card = new AntCard(QStringLiteral("Basic TreeSelect"));
        auto* cl = card->bodyLayout();

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
        cl->addWidget(treeSelect);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createUploadPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 16, 24, 16);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic Upload"));
        auto* cl = card->bodyLayout();

        auto* upload = new AntUpload(page);
        upload->addFile({QStringLiteral("file1"), QStringLiteral("document.pdf"), Ant::UploadFileStatus::Done, 100});
        upload->addFile({QStringLiteral("file2"), QStringLiteral("image.png"), Ant::UploadFileStatus::Uploading, 45});
        cl->addWidget(upload);

        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Picture Card"));
        auto* cl = card->bodyLayout();

        auto* picUpload = new AntUpload(page);
        picUpload->setListType(Ant::UploadListType::PictureCard);
        cl->addWidget(picUpload);

        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Dragger"));
        auto* cl = card->bodyLayout();

        auto* dragger = new AntUpload(page);
        dragger->setDraggerMode(true);
        dragger->setFixedHeight(180);
        cl->addWidget(dragger);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
