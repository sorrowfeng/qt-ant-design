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
        cl->setAlignment(Qt::AlignTop);
        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(16);
        auto* radioA = new AntRadio(QStringLiteral("A"));
        auto* radioB = new AntRadio(QStringLiteral("B"));
        auto* radioC = new AntRadio(QStringLiteral("C"));
        radioA->setChecked(true);
        basicRow->addWidget(radioA);
        basicRow->addWidget(radioB);
        basicRow->addWidget(radioC);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Button Style"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* buttonRow = new QHBoxLayout();
        buttonRow->setSpacing(0);
        auto* daily = new AntRadio(QStringLiteral("Daily"));
        daily->setButtonStyle(true);
        daily->setChecked(true);
        auto* weekly = new AntRadio(QStringLiteral("Weekly"));
        weekly->setButtonStyle(true);
        auto* monthly = new AntRadio(QStringLiteral("Monthly"));
        monthly->setButtonStyle(true);
        buttonRow->addWidget(daily);
        buttonRow->addWidget(weekly);
        buttonRow->addWidget(monthly);
        buttonRow->addStretch();
        cl->addLayout(buttonRow);
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
        cl->setAlignment(Qt::AlignTop);
        auto* basic = new AntRate();
        basic->setValue(3.0);
        cl->addWidget(basic, 0, Qt::AlignLeft);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Half & Disabled"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* row = new QHBoxLayout();
        row->setSpacing(16);
        auto* half = new AntRate();
        half->setAllowHalf(true);
        half->setValue(2.5);
        auto* disabled = new AntRate();
        disabled->setDisabled(true);
        disabled->setValue(3.0);
        row->addWidget(half);
        row->addWidget(disabled);
        row->addStretch();
        cl->addLayout(row);
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
        cl->setAlignment(Qt::AlignTop);
        auto* basic = new AntSelect();
        basic->addOptions({QStringLiteral("Apple"), QStringLiteral("Banana"), QStringLiteral("Orange")});
        basic->setCurrentIndex(0);
        basic->setFixedWidth(160);
        cl->addWidget(basic, 0, Qt::AlignLeft);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Multiple"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* multi = new AntSelect();
        multi->setSelectMode(Ant::SelectMode::Multiple);
        multi->addOptions({QStringLiteral("Apple"), QStringLiteral("Banana"), QStringLiteral("Orange")});
        multi->addSelectedIndex(0);
        multi->setFixedWidth(280);
        cl->addWidget(multi, 0, Qt::AlignLeft);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Sizes"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* sizeRow = new QHBoxLayout();
        sizeRow->setSpacing(12);
        auto* large = new AntSelect();
        large->setSelectSize(Ant::Size::Large);
        large->addOptions({QStringLiteral("Large"), QStringLiteral("B")});
        large->setCurrentIndex(0);
        large->setFixedWidth(120);
        auto* middle = new AntSelect();
        middle->addOptions({QStringLiteral("Middle"), QStringLiteral("B")});
        middle->setCurrentIndex(0);
        middle->setFixedWidth(120);
        auto* small = new AntSelect();
        small->setSelectSize(Ant::Size::Small);
        small->addOptions({QStringLiteral("Small"), QStringLiteral("B")});
        small->setCurrentIndex(0);
        small->setFixedWidth(120);
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

QWidget* createSliderPage(QWidget* owner)
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
        auto* basic = new AntSlider();
        basic->setFixedWidth(400);
        basic->setValue(30);
        cl->addWidget(basic, 0, Qt::AlignLeft);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Range"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* range = new AntSlider();
        range->setRangeMode(true);
        range->setRangeValues(20, 60);
        range->setFixedWidth(400);
        cl->addWidget(range, 0, Qt::AlignLeft);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Step"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* marks = new AntSlider();
        marks->setSingleStep(25);
        marks->setMarks({{0, QStringLiteral("A")}, {25, QStringLiteral("B")}, {50, QStringLiteral("C")},
                         {75, QStringLiteral("D")}, {100, QStringLiteral("E")}});
        marks->setValue(50);
        marks->setFixedSize(400, 46);
        cl->addWidget(marks, 0, Qt::AlignLeft);
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
        cl->setAlignment(Qt::AlignTop);
        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(16);
        auto* onSwitch = new AntSwitch();
        onSwitch->setChecked(true);
        auto* offSwitch = new AntSwitch();
        basicRow->addWidget(onSwitch);
        basicRow->addWidget(offSwitch);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Text"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* textRow = new QHBoxLayout();
        textRow->setSpacing(16);
        auto* textSwitch = new AntSwitch();
        textSwitch->setCheckedText(QStringLiteral("ON"));
        textSwitch->setUncheckedText(QStringLiteral("OFF"));
        textSwitch->setChecked(true);
        auto* digitSwitch = new AntSwitch();
        digitSwitch->setCheckedText(QStringLiteral("1"));
        digitSwitch->setUncheckedText(QStringLiteral("0"));
        textRow->addWidget(textSwitch);
        textRow->addWidget(digitSwitch);
        textRow->addStretch();
        cl->addLayout(textRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Disabled"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* disabledOn = new AntSwitch();
        disabledOn->setChecked(true);
        disabledOn->setEnabled(false);
        cl->addWidget(disabledOn, 0, Qt::AlignLeft);
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
        cl->setAlignment(Qt::AlignTop);
        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(12);
        auto* basic = new AntTimePicker();
        auto* range = new AntTimePicker();
        range->setRangeMode(true);
        basicRow->addWidget(basic);
        basicRow->addWidget(range);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createTransferPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    auto* card = new AntCard(QStringLiteral("Basic"));
    auto* cl = card->bodyLayout();
    cl->setAlignment(Qt::AlignTop);

    auto* transfer = new AntTransfer(card);
    transfer->setSourceItems({QStringLiteral("Item 0"), QStringLiteral("Item 1"), QStringLiteral("Item 2"),
                              QStringLiteral("Item 3"), QStringLiteral("Item 4"), QStringLiteral("Item 5"),
                              QStringLiteral("Item 6"), QStringLiteral("Item 7"), QStringLiteral("Item 8"),
                              QStringLiteral("Item 9")});
    transfer->setTargetItems({});
    cl->addWidget(transfer, 0, Qt::AlignLeft);
    layout->addWidget(card);

    layout->addStretch();
    return page;
}

QWidget* createTreeSelectPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* treeSelect = new AntTreeSelect(card);
        treeSelect->setPlaceholder(QStringLiteral("Please select"));
        treeSelect->setFixedWidth(240);

        QVector<AntTreeNode> treeData;
        AntTreeNode parent1;
        parent1.key = QStringLiteral("1");
        parent1.title = QStringLiteral("Node 1");
        parent1.children.push_back({QStringLiteral("1-1"), QStringLiteral("Child 1-1")});
        parent1.children.push_back({QStringLiteral("1-2"), QStringLiteral("Child 1-2")});
        treeData.push_back(parent1);

        AntTreeNode parent2;
        parent2.key = QStringLiteral("2");
        parent2.title = QStringLiteral("Node 2");
        treeData.push_back(parent2);

        treeSelect->setTreeData(treeData);
        cl->addWidget(treeSelect, 0, Qt::AlignLeft);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createUploadPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Default"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* upload = new AntUpload(card);
        upload->setFixedWidth(180);
        cl->addWidget(upload, 0, Qt::AlignLeft);

        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Dragger"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* dragger = new AntUpload(card);
        dragger->setDraggerMode(true);
        dragger->setMultiple(true);
        dragger->setFixedSize(480, 150);
        cl->addWidget(dragger, 0, Qt::AlignLeft);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
