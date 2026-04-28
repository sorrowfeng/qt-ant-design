#include "Pages.h"

#include <QColor>
#include <QDateTime>
#include <QFrame>
#include <QHBoxLayout>
#include <QList>
#include <QPair>
#include <QString>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"
#include "widgets/AntButton.h"
#include "widgets/AntCard.h"
#include "widgets/AntPopover.h"
#include "widgets/AntQRCode.h"
#include "widgets/AntSegmented.h"
#include "widgets/AntStatistic.h"
#include "widgets/AntTable.h"
#include <QDateTime>
#include "widgets/AntTabs.h"
#include "widgets/AntTag.h"
#include "widgets/AntTimeline.h"
#include "widgets/AntTooltip.h"
#include "widgets/AntTree.h"
#include "widgets/AntTypography.h"

namespace example::pages
{
QWidget* createPopoverPage(QWidget* /*owner*/)
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

        auto* hoverButton = new AntButton(QStringLiteral("Hover me"));
        auto* hoverPopover = new AntPopover(page);
        hoverPopover->setTarget(hoverButton);
        hoverPopover->setTitle(QStringLiteral("Title"));
        hoverPopover->setContent(QStringLiteral("Popover content"));

        auto* clickButton = new AntButton(QStringLiteral("Click me"));
        auto* clickPopover = new AntPopover(page);
        clickPopover->setTrigger(Ant::PopoverTrigger::Click);
        clickPopover->setTarget(clickButton);
        clickPopover->setTitle(QStringLiteral("Title"));
        clickPopover->setContent(QStringLiteral("Click content"));

        basicRow->addWidget(hoverButton);
        basicRow->addWidget(clickButton);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createQRCodePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* qr1 = new AntQRCode();
        qr1->setValue(QStringLiteral("https://ant.design"));
        qr1->setFixedSize(160, 160);
        cl->addWidget(qr1);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Color"));
        auto* cl = card->bodyLayout();
        auto* colorRow = new QHBoxLayout();
        colorRow->setSpacing(24);
        auto* qrDefault = new AntQRCode();
        qrDefault->setValue(QStringLiteral("https://ant.design"));
        qrDefault->setFixedSize(120, 120);
        auto* qrRed = new AntQRCode();
        qrRed->setValue(QStringLiteral("https://ant.design"));
        qrRed->setColor(QColor(QStringLiteral("#ff4d4f")));
        qrRed->setFixedSize(120, 120);
        auto* qrBlue = new AntQRCode();
        qrBlue->setValue(QStringLiteral("https://ant.design"));
        qrBlue->setColor(QColor(QStringLiteral("#1677ff")));
        qrBlue->setFixedSize(120, 120);
        colorRow->addWidget(qrDefault);
        colorRow->addWidget(qrRed);
        colorRow->addWidget(qrBlue);
        colorRow->addStretch();
        cl->addLayout(colorRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("No Border"));
        auto* cl = card->bodyLayout();
        auto* qrNoBorder = new AntQRCode();
        qrNoBorder->setValue(QStringLiteral("https://ant.design"));
        qrNoBorder->setBordered(false);
        qrNoBorder->setFixedSize(120, 120);
        cl->addWidget(qrNoBorder);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Status Overlay"));
        auto* cl = card->bodyLayout();
        auto* statusRow = new QHBoxLayout();
        statusRow->setSpacing(12);
        auto* qrExpired = new AntQRCode();
        qrExpired->setValue(QStringLiteral("https://ant.design"));
        qrExpired->setStatus(Ant::QRCodeStatus::Expired);
        qrExpired->setFixedSize(120, 120);
        auto* qrLoading = new AntQRCode();
        qrLoading->setValue(QStringLiteral("https://ant.design"));
        qrLoading->setStatus(Ant::QRCodeStatus::Loading);
        qrLoading->setFixedSize(120, 120);
        auto* qrScanned = new AntQRCode();
        qrScanned->setValue(QStringLiteral("https://ant.design"));
        qrScanned->setStatus(Ant::QRCodeStatus::Scanned);
        qrScanned->setFixedSize(120, 120);
        statusRow->addWidget(qrExpired);
        statusRow->addWidget(qrLoading);
        statusRow->addWidget(qrScanned);
        statusRow->addStretch();
        cl->addLayout(statusRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createSegmentedPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    QVector<AntSegmentedOption> opts1 = {
        {QStringLiteral("daily"), QStringLiteral("Daily")},
        {QStringLiteral("weekly"), QStringLiteral("Weekly")},
        {QStringLiteral("monthly"), QStringLiteral("Monthly")},
        {QStringLiteral("yearly"), QStringLiteral("Yearly")},
    };

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* seg1 = new AntSegmented();
        seg1->setOptions(opts1);
        seg1->setValue(QStringLiteral("daily"));
        cl->addWidget(seg1);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Block"));
        auto* cl = card->bodyLayout();
        auto* seg2 = new AntSegmented();
        seg2->setOptions(opts1);
        seg2->setValue(QStringLiteral("weekly"));
        seg2->setBlock(true);
        cl->addWidget(seg2);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Size"));
        auto* cl = card->bodyLayout();
        auto* sizeRow = new QHBoxLayout();
        sizeRow->setSpacing(12);
        auto* segSm = new AntSegmented();
        segSm->setOptions({{QStringLiteral("s"), QStringLiteral("S")}, {QStringLiteral("m"), QStringLiteral("M")}});
        segSm->setSegmentedSize(Ant::Size::Small);
        auto* segMd = new AntSegmented();
        segMd->setOptions({{QStringLiteral("a"), QStringLiteral("A")}, {QStringLiteral("b"), QStringLiteral("B")}});
        auto* segLg = new AntSegmented();
        segLg->setOptions({{QStringLiteral("x"), QStringLiteral("X")}, {QStringLiteral("y"), QStringLiteral("Y")}});
        segLg->setSegmentedSize(Ant::Size::Large);
        sizeRow->addWidget(segSm);
        sizeRow->addWidget(segMd);
        sizeRow->addWidget(segLg);
        sizeRow->addStretch();
        cl->addLayout(sizeRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Vertical"));
        auto* cl = card->bodyLayout();
        auto* segVert = new AntSegmented();
        segVert->setOptions({{QStringLiteral("map"), QStringLiteral("Map")}, {QStringLiteral("list"), QStringLiteral("List")}, {QStringLiteral("grid"), QStringLiteral("Grid")}});
        segVert->setVertical(true);
        cl->addWidget(segVert);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Round Shape"));
        auto* cl = card->bodyLayout();
        auto* segRound = new AntSegmented();
        segRound->setOptions({{QStringLiteral("a"), QStringLiteral("A")}, {QStringLiteral("b"), QStringLiteral("B")}, {QStringLiteral("c"), QStringLiteral("C")}});
        segRound->setShape(Ant::SegmentedShape::Round);
        cl->addWidget(segRound);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Disabled Option"));
        auto* cl = card->bodyLayout();
        auto* segDisabled = new AntSegmented();
        segDisabled->setOptions({
            {QStringLiteral("a"), QStringLiteral("Enabled")},
            {QStringLiteral("b"), QStringLiteral("Disabled"), QString(), true},
            {QStringLiteral("c"), QStringLiteral("Active")},
        });
        segDisabled->setValue(QStringLiteral("c"));
        cl->addWidget(segDisabled);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createStatisticPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Prefix and Suffix"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(prefixRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Card Style"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(cardRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Countdown"));
        auto* cl = card->bodyLayout();
        auto* countdownRow = new QHBoxLayout();
        countdownRow->setSpacing(48);

        auto* countdown = new AntStatistic(page);
        countdown->setTitle(QStringLiteral("Countdown"));
        countdown->setCountdownMode(true);
        countdown->setCountdownFormat(QStringLiteral("HH:mm:ss"));
        countdown->setValue(QDateTime::currentDateTime().addSecs(3661).toMSecsSinceEpoch() / 1000.0);
        countdown->setFixedWidth(180);

        auto* countdown2 = new AntStatistic(page);
        countdown2->setTitle(QStringLiteral("Days Left"));
        countdown2->setCountdownMode(true);
        countdown2->setCountdownFormat(QStringLiteral("DD HH:mm:ss"));
        countdown2->setValue(QDateTime::currentDateTime().addDays(2).addSecs(7200).toMSecsSinceEpoch() / 1000.0);
        countdown2->setFixedWidth(220);

        countdownRow->addWidget(countdown);
        countdownRow->addWidget(countdown2);
        countdownRow->addStretch();
        cl->addLayout(countdownRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createTablePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

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

    {
        auto* card = new AntCard(QStringLiteral("Basic Table"));
        auto* cl = card->bodyLayout();

        auto* table = new AntTable(page);
        table->setBordered(true);
        table->addColumn({QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 150});
        table->addColumn({QStringLiteral("Age"), QStringLiteral("age"), QStringLiteral("age"), 80});
        table->addColumn({QStringLiteral("Address"), QStringLiteral("address"), QStringLiteral("address"), 250});
        table->setRows(rows);
        cl->addWidget(table);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Selection Table"));
        auto* cl = card->bodyLayout();

        auto* selTable = new AntTable(page);
        selTable->setRowSelection(Ant::TableSelectionMode::Checkbox);
        selTable->setBordered(true);
        selTable->addColumn({QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 150});
        selTable->addColumn({QStringLiteral("Age"), QStringLiteral("age"), QStringLiteral("age"), 80});
        selTable->setRows(rows);
        cl->addWidget(selTable);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Empty Table"));
        auto* cl = card->bodyLayout();

        auto* emptyTable = new AntTable(page);
        emptyTable->setBordered(true);
        emptyTable->addColumn({QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 150});
        emptyTable->addColumn({QStringLiteral("Age"), QStringLiteral("age"), QStringLiteral("age"), 80});
        emptyTable->addColumn({QStringLiteral("Address"), QStringLiteral("address"), QStringLiteral("address"), 250});
        emptyTable->setFixedHeight(220);
        cl->addWidget(emptyTable);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createTabsPage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

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

    {
        auto* card = new AntCard(QStringLiteral("Line"));
        auto* cl = card->bodyLayout();
        auto* lineTabs = new AntTabs();
        lineTabs->setMinimumHeight(180);
        lineTabs->addTab(makePane(QStringLiteral("Content of Tab 1")), QStringLiteral("tab1"), QStringLiteral("Tab 1"));
        lineTabs->addTab(makePane(QStringLiteral("Content of Tab 2")), QStringLiteral("tab2"), QStringLiteral("Tab 2"));
        lineTabs->addTab(makePane(QStringLiteral("Disabled tab content")), QStringLiteral("disabled"), QStringLiteral("Disabled"), QString(), true);
        lineTabs->setActiveKey(QStringLiteral("tab1"));
        cl->addWidget(lineTabs);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Centered and Placement"));
        auto* cl = card->bodyLayout();
        auto* placementRow = new QHBoxLayout();
        placementRow->setSpacing(18);
        auto* centered = new AntTabs();
        centered->setCentered(true);
        centered->setTabsSize(Ant::Size::Large);
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
        cl->addLayout(placementRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Card and Editable"));
        auto* cl = card->bodyLayout();
        auto* cardRow = new QHBoxLayout();
        cardRow->setSpacing(18);
        auto* cardTabs = new AntTabs();
        cardTabs->setTabsType(Ant::TabsType::Card);
        cardTabs->setTabsSize(Ant::Size::Small);
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
        QObject::connect(editableTabs, &AntTabs::tabAddRequested, owner, [editableTabs, makePane]() {
            const QString key = QStringLiteral("new-%1").arg(QDateTime::currentMSecsSinceEpoch());
            editableTabs->addTab(makePane(QStringLiteral("New editable tab")), key, QStringLiteral("New Tab"));
            editableTabs->setActiveKey(key);
        });
        cardRow->addWidget(editableTabs, 1);
        cl->addLayout(cardRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createTagPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Status Colors"));
        auto* cl = card->bodyLayout();
        auto* statusRow = new QHBoxLayout();
        statusRow->setSpacing(10);
        const QList<QPair<QString, QString>> statusColors = {
            {QStringLiteral("Success"), QStringLiteral("success")},
            {QStringLiteral("Processing"), QStringLiteral("processing")},
            {QStringLiteral("Warning"), QStringLiteral("warning")},
            {QStringLiteral("Error"), QStringLiteral("error")},
        };
        for (const auto& item : statusColors)
        {
            auto* tag = new AntTag(item.first);
            tag->setColor(item.second);
            statusRow->addWidget(tag);
        }
        statusRow->addStretch();
        cl->addLayout(statusRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Preset Colors"));
        auto* cl = card->bodyLayout();
        auto* presetRow1 = new QHBoxLayout();
        presetRow1->setSpacing(10);
        for (const QString& name : {QStringLiteral("magenta"), QStringLiteral("red"), QStringLiteral("volcano"),
                                     QStringLiteral("orange"), QStringLiteral("gold"), QStringLiteral("lime"),
                                     QStringLiteral("green")})
        {
            auto* tag = new AntTag(name);
            tag->setColor(name);
            presetRow1->addWidget(tag);
        }
        presetRow1->addStretch();
        cl->addLayout(presetRow1);

        auto* presetRow2 = new QHBoxLayout();
        presetRow2->setSpacing(10);
        for (const QString& name : {QStringLiteral("cyan"), QStringLiteral("blue"), QStringLiteral("geekblue"),
                                     QStringLiteral("purple"), QStringLiteral("pink"), QStringLiteral("yellow")})
        {
            auto* tag = new AntTag(name);
            tag->setColor(name);
            presetRow2->addWidget(tag);
        }
        presetRow2->addStretch();
        cl->addLayout(presetRow2);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Variant"));
        auto* cl = card->bodyLayout();
        auto* variantRow = new QHBoxLayout();
        variantRow->setSpacing(10);
        const QList<QPair<QString, QString>> statusColors = {
            {QStringLiteral("Success"), QStringLiteral("success")},
            {QStringLiteral("Processing"), QStringLiteral("processing")},
            {QStringLiteral("Warning"), QStringLiteral("warning")},
            {QStringLiteral("Error"), QStringLiteral("error")},
        };
        for (const auto& item : statusColors)
        {
            auto* tag = new AntTag(QStringLiteral("Solid ") + item.first);
            tag->setColor(item.second);
            tag->setVariant(Ant::TagVariant::Solid);
            variantRow->addWidget(tag);
        }
        for (const auto& item : statusColors)
        {
            auto* tag = new AntTag(QStringLiteral("Outlined ") + item.first);
            tag->setColor(item.second);
            tag->setVariant(Ant::TagVariant::Outlined);
            variantRow->addWidget(tag);
        }
        variantRow->addStretch();
        cl->addLayout(variantRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Checkable"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(checkRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createTimelinePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basic = new AntTimeline();
        basic->addItem(QStringLiteral("Create services"), QStringLiteral("2015-09-01"));
        basic->addItem(QStringLiteral("Solve initial network problems"), QStringLiteral("2015-09-01"));
        basic->addItem(QStringLiteral("Technical testing"), QStringLiteral("2015-09-01"));
        basic->addItem(QStringLiteral("Network problems being solved"), QStringLiteral("2015-09-01"));
        cl->addWidget(basic);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Color"));
        auto* cl = card->bodyLayout();
        auto* color = new AntTimeline();
        color->addItem(QStringLiteral("Create services"), QString(), QStringLiteral("green"));
        color->addItem(QStringLiteral("Solve initial network problems"), QString(), QStringLiteral("red"));
        color->addItem(QStringLiteral("Technical testing"), QString(), QStringLiteral("blue"));
        color->addItem(QStringLiteral("Network problems being solved"), QString(), QStringLiteral("gray"));
        cl->addWidget(color);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Filled"));
        auto* cl = card->bodyLayout();
        auto* filled = new AntTimeline();
        filled->setDotVariant(Ant::TimelineDotVariant::Filled);
        filled->addItem(QStringLiteral("Create services"), QStringLiteral("2015-09-01"), QStringLiteral("green"));
        filled->addItem(QStringLiteral("Solve initial network problems"), QStringLiteral("2015-09-01"), QStringLiteral("red"));
        filled->addItem(QStringLiteral("Technical testing"), QStringLiteral("2015-09-01"), QStringLiteral("blue"));
        cl->addWidget(filled);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createTooltipPage(QWidget* /*owner*/)
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
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Placement"));
        auto* cl = card->bodyLayout();
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
        auto* center = new AntTypography(QStringLiteral("Hover the buttons around this area"), page);
        center->setMinimumSize(220, 64);
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

        cl->addWidget(placementWrap);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Arrow and Color"));
        auto* cl = card->bodyLayout();
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
        cl->addLayout(customRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createTreePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic Tree"));
        auto* cl = card->bodyLayout();

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
        cl->addWidget(tree);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
