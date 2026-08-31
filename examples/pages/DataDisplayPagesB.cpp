#include "Pages.h"

#include "PageCommon.h"

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
#include "widgets/AntInput.h"
#include "widgets/AntPopover.h"
#include "widgets/AntScrollArea.h"
#include "widgets/AntQRCode.h"
#include "widgets/AntSegmented.h"
#include "widgets/AntStatistic.h"
#include "widgets/AntTable.h"
#include <QDateTime>
#include "widgets/AntTabs.h"
#include "widgets/AntTag.h"
#include "widgets/AntTimeline.h"
#include "widgets/AntToolTip.h"
#include "widgets/AntTree.h"
#include "widgets/AntTypography.h"

namespace example::pages
{
QWidget* createPopoverPage(QWidget* /*owner*/)
{
    auto [page, layout] = makePage();

    {
        auto* card = makeCard(layout, QStringLiteral("Basic"));
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

    }

    layout->addStretch();
    return page;
}

QWidget* createQRCodePage(QWidget* /*owner*/)
{
    auto [page, layout] = makePage();

    {
        auto* card = makeCard(layout, QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* qr1 = new AntQRCode();
        qr1->setFixedSize(160, 160);
        cl->addWidget(qr1, 0, Qt::AlignLeft);

        auto* controlRow = new QHBoxLayout();
        controlRow->setSpacing(8);

        auto* valueInput = new AntInput();
        valueInput->setText(qr1->value());
        valueInput->setPlaceholderText(QStringLiteral("QR code content"));
        valueInput->setMinimumWidth(360);

        auto* regenerate = new AntButton(QStringLiteral("Regenerate"));
        regenerate->setButtonType(Ant::ButtonType::Primary);

        auto regenerateQRCode = [qr1, valueInput]() {
            qr1->setValue(valueInput->text());
        };
        QObject::connect(regenerate, &AntButton::clicked, qr1, regenerateQRCode);
        QObject::connect(valueInput, &AntInput::returnPressed, qr1, regenerateQRCode);

        controlRow->addWidget(valueInput);
        controlRow->addWidget(regenerate);
        controlRow->addStretch();
        cl->addLayout(controlRow);

    }

    layout->addStretch();
    return page;
}

QWidget* createSegmentedPage(QWidget* /*owner*/)
{
    auto [page, layout] = makePage();

    const QVector<AntSegmentedOption> basicOptions = {
        {QStringLiteral("daily"), QStringLiteral("Daily")},
        {QStringLiteral("weekly"), QStringLiteral("Weekly")},
        {QStringLiteral("monthly"), QStringLiteral("Monthly")},
    };

    {
        auto* card = makeCard(layout, QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basic = new AntSegmented();
        basic->setOptions(basicOptions);
        basic->setValue(QStringLiteral("daily"));
        cl->addWidget(basic, 0, Qt::AlignLeft | Qt::AlignTop);

    }

    {
        auto* card = makeCard(layout, QStringLiteral("With Icons"));
        auto* cl = card->bodyLayout();
        auto* withIcons = new AntSegmented();
        withIcons->setOptions({
            {QStringLiteral("list"), QStringLiteral("List"), QStringLiteral("appstore")},
            {QStringLiteral("kanban"), QStringLiteral("Kanban"), QStringLiteral("setting")},
        });
        withIcons->setValue(QStringLiteral("list"));
        cl->addWidget(withIcons, 0, Qt::AlignLeft | Qt::AlignTop);

    }

    layout->addStretch();
    return page;
}

QWidget* createStatisticPage(QWidget* /*owner*/)
{
    auto [page, layout] = makePage();

    {
        auto* card = makeCard(layout, QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(40);

        auto* users = new AntStatistic(page);
        users->setTitle(QStringLiteral("Active Users"));
        users->setValue(112893);
        users->setFixedWidth(120);

        auto* balance = new AntStatistic(page);
        balance->setTitle(QStringLiteral("Account Balance"));
        balance->setValue(112893);
        balance->setPrecision(2);
        balance->setPrefix(QStringLiteral("$"));
        balance->setFixedWidth(180);

        basicRow->addWidget(users);
        basicRow->addWidget(balance);
        basicRow->addStretch();
        cl->addLayout(basicRow);

    }

    {
        auto* card = makeCard(layout, QStringLiteral("Countdown"));
        auto* cl = card->bodyLayout();
        auto* countdown = new AntStatistic(page);
        countdown->setTitle(QStringLiteral("Countdown"));
        countdown->setCountdownMode(true);
        countdown->setCountdownFormat(QStringLiteral("HH:mm:ss"));
        countdown->setValue(QDateTime::currentDateTime().addDays(2).toMSecsSinceEpoch() / 1000.0);
        countdown->setFixedWidth(180);
        cl->addWidget(countdown);

    }

    layout->addStretch();
    return page;
}

QWidget* createTablePage(QWidget* /*owner*/)
{
    auto [page, layout] = makePage();

    auto makeRows = [](int count) {
        static const char* const kNames[] = {
            "John", "Jane", "Joe", "Jill", "Jack", "Jenny", "Jim", "Julia",
            "Jacob", "Jasmine", "Jerry", "Joyce", "Jared", "Jade", "Jonas",
        };
        static const char* const kCities[] = {
            "New York", "London", "Sydney", "Tokyo", "Paris", "Berlin",
            "Toronto", "Seoul", "Shanghai", "Singapore", "Dubai", "Rome",
            "Madrid", "Vienna", "Oslo",
        };
        QVector<AntTableRow> rows;
        for (int i = 0; i < count; ++i)
        {
            rows.push_back({{{QStringLiteral("name"), QString::fromLatin1(kNames[i % 15])},
                             {QStringLiteral("age"), 20 + (i % 30)},
                             {QStringLiteral("address"), QString::fromLatin1(kCities[i % 15])}}});
        }
        return rows;
    };

    // AntTable is a self-painted widget with a fixed total column width and
    // no internal horizontal scrolling, so when the table's sizeHint() exceeds
    // the host container's width the rightmost columns would be clipped.
    // Pass the table through this helper to wrap it in a non-resizable
    // AntScrollArea that exposes a horizontal scrollbar instead.
    auto wrapTableForHorizontalScroll = [](AntTable* table) {
        auto* scroll = new AntScrollArea();
        scroll->setWidgetResizable(false);
        scroll->setAutoHideScrollBar(false);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setWidget(table);
        return scroll;
    };

    {
        auto* card = makeCard(layout, QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();

        // Column widths are sized so the table fits within a 1200px window
        // without horizontal clipping; AntTable does not autoscale column
        // widths to its container, so too-wide tables get wrapped in
        // wrapTableForHorizontalScroll() below instead.
        auto* table = new AntTable(page);
        table->setBordered(false);
        table->addColumn({QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 220});
        AntTableColumn ageColumn{QStringLiteral("Age"), QStringLiteral("age"), QStringLiteral("age"), 180};
        ageColumn.sorter = true;
        table->addColumn(ageColumn);
        table->addColumn({QStringLiteral("Address"), QStringLiteral("address"), QStringLiteral("address"), 300});
        table->setRows(makeRows(3));
        cl->addWidget(table);

    }

    {
        auto* card = makeCard(layout, QStringLiteral("Row selection"));
        auto* cl = card->bodyLayout();

        auto* table = new AntTable(page);
        table->setRowSelection(Ant::TableSelectionMode::Checkbox);
        table->addColumn({QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 220});
        table->addColumn({QStringLiteral("Age"), QStringLiteral("age"), QStringLiteral("age"), 140});
        table->addColumn({QStringLiteral("Address"), QStringLiteral("address"), QStringLiteral("address"), 280});
        table->setRows(makeRows(5));

        auto* hint = new AntTypography(QStringLiteral("Selected: none"), page);
        hint->setType(Ant::TypographyType::Secondary);
        QObject::connect(table, &AntTable::selectionChanged, hint,
                         [hint](const QStringList& keys) {
                             hint->setText(QStringLiteral("Selected: %1")
                                               .arg(keys.isEmpty()
                                                        ? QStringLiteral("none")
                                                        : keys.join(QStringLiteral(", "))));
                         });
        cl->addWidget(table);
        cl->addSpacing(8);
        cl->addWidget(hint);

    }

    {
        auto* card = makeCard(layout, QStringLiteral("Pagination & border"));
        auto* cl = card->bodyLayout();

        auto* table = new AntTable(page);
        table->setBordered(true);
        table->setPageSize(5);
        table->addColumn({QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 220});
        table->addColumn({QStringLiteral("Age"), QStringLiteral("age"), QStringLiteral("age"), 140});
        table->addColumn({QStringLiteral("Address"), QStringLiteral("address"), QStringLiteral("address"), 280});
        table->setRows(makeRows(15));
        cl->addWidget(table);

    }

    layout->addStretch();
    return page;
}

QWidget* createTabsPage(QWidget* /*owner*/)
{
    auto [page, layout] = makePage();

    auto makePane = [](const QString& text) {
        auto* pane = new QWidget();
        auto* paneLayout = new QVBoxLayout(pane);
        paneLayout->setContentsMargins(0, 8, 0, 0);
        auto* label = new AntTypography(text);
        label->setParagraph(true);
        paneLayout->addWidget(label);
        paneLayout->addStretch();
        return pane;
    };

    {
        auto* card = makeCard(layout, QStringLiteral("Line"));
        auto* cl = card->bodyLayout();
        auto* lineTabs = new AntTabs();
        lineTabs->setMinimumHeight(110);
        lineTabs->addTab(makePane(QStringLiteral("Content 1")), QStringLiteral("1"), QStringLiteral("Tab 1"));
        lineTabs->addTab(makePane(QStringLiteral("Content 2")), QStringLiteral("2"), QStringLiteral("Tab 2"));
        lineTabs->addTab(makePane(QStringLiteral("Content 3")), QStringLiteral("3"), QStringLiteral("Tab 3"));
        lineTabs->setActiveKey(QStringLiteral("1"));
        cl->addWidget(lineTabs);

    }

    {
        auto* card = makeCard(layout, QStringLiteral("Card"));
        auto* cl = card->bodyLayout();
        auto* cardTabs = new AntTabs();
        cardTabs->setTabsType(Ant::TabsType::Card);
        cardTabs->setMinimumHeight(110);
        cardTabs->addTab(makePane(QStringLiteral("Content A")), QStringLiteral("1"), QStringLiteral("Card A"));
        cardTabs->addTab(makePane(QStringLiteral("Content B")), QStringLiteral("2"), QStringLiteral("Card B"));
        cardTabs->setActiveKey(QStringLiteral("1"));
        cl->addWidget(cardTabs);

    }

    layout->addStretch();
    return page;
}

QWidget* createTagPage(QWidget* /*owner*/)
{
    auto [page, layout] = makePage();

    {
        auto* card = makeCard(layout, QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(10);
        for (const auto& item : {QPair<QString, QString>{QStringLiteral("Blue"), QStringLiteral("blue")},
                                  QPair<QString, QString>{QStringLiteral("Green"), QStringLiteral("green")},
                                  QPair<QString, QString>{QStringLiteral("Orange"), QStringLiteral("orange")},
                                  QPair<QString, QString>{QStringLiteral("Red"), QStringLiteral("red")},
                                  QPair<QString, QString>{QStringLiteral("Purple"), QStringLiteral("purple")}})
        {
            auto* tag = new AntTag(item.first);
            tag->setColor(item.second);
            basicRow->addWidget(tag);
        }
        basicRow->addStretch();
        cl->addLayout(basicRow);

    }

    {
        auto* card = makeCard(layout, QStringLiteral("Checkable & Closable"));
        auto* cl = card->bodyLayout();
        auto* checkRow = new QHBoxLayout();
        checkRow->setSpacing(10);

        auto* checkable = new AntTag(QStringLiteral("Checkable"));
        checkable->setCheckable(true);
        checkable->setChecked(true);
        checkRow->addWidget(checkable);

        auto* closable = new AntTag(QStringLiteral("Closable"));
        closable->setClosable(true);
        checkRow->addWidget(closable);

        auto* processing = new AntTag(QStringLiteral("Processing"));
        processing->setColor(QStringLiteral("processing"));
        checkRow->addWidget(processing);

        auto* success = new AntTag(QStringLiteral("Success"));
        success->setColor(QStringLiteral("success"));
        checkRow->addWidget(success);

        checkRow->addStretch();
        cl->addLayout(checkRow);

    }

    layout->addStretch();
    return page;
}

QWidget* createTimelinePage(QWidget* /*owner*/)
{
    auto [page, layout] = makePage();

    {
        auto* card = makeCard(layout, QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basic = new AntTimeline();
        basic->addItem(QStringLiteral("Step 1 - Create services site 2024-01-01"), QString(), QStringLiteral("blue"));
        basic->addItem(QStringLiteral("Step 2 - Initial program coding 2024-01-02"));
        basic->addItem(QStringLiteral("Step 3 - Testing and verification 2024-01-03"), QString(), QStringLiteral("green"));
        basic->addItem(QStringLiteral("Step 4 - Network problems resolved 2024-01-04"), QString(), QStringLiteral("red"));
        cl->addWidget(basic);

    }

    layout->addStretch();
    return page;
}

QWidget* createTooltipPage(QWidget* /*owner*/)
{
    auto [page, layout] = makePage();

    {
        auto* card = makeCard(layout, QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(16);
        const QList<QPair<QString, Ant::Placement>> placements = {
            {QStringLiteral("Top"), Ant::Placement::Top},
            {QStringLiteral("Bottom"), Ant::Placement::Bottom},
            {QStringLiteral("Left"), Ant::Placement::Left},
            {QStringLiteral("Right"), Ant::Placement::Right},
        };
        for (const auto& item : placements)
        {
            auto* button = new AntButton(item.first);
            auto* tooltip = new AntToolTip(page);
            tooltip->setTitle(QStringLiteral("Prompt Text"));
            tooltip->setPlacement(item.second);
            tooltip->setTarget(button);
            basicRow->addWidget(button);
        }
        basicRow->addStretch();
        cl->addLayout(basicRow);

    }

    layout->addStretch();
    return page;
}

QWidget* createTreePage(QWidget* /*owner*/)
{
    auto [page, layout] = makePage();

    {
        auto* card = makeCard(layout, QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();

        auto* tree = new AntTree(page);
        tree->setShowIcon(false);
        tree->setCheckable(false);

        QVector<AntTreeNode> treeData;
        AntTreeNode parent1;
        parent1.key = QStringLiteral("0-0");
        parent1.title = QStringLiteral("Parent 1");
        parent1.expanded = true;

        AntTreeNode child1;
        child1.key = QStringLiteral("0-0-0");
        child1.title = QStringLiteral("Child 1-1");
        child1.expanded = true;
        child1.children.push_back({QStringLiteral("0-0-0-0"), QStringLiteral("Leaf")});
        parent1.children.push_back(child1);
        parent1.children.push_back({QStringLiteral("0-0-1"), QStringLiteral("Child 1-2")});
        treeData.push_back(parent1);

        tree->setTreeData(treeData);
        cl->addWidget(tree);

    }

    layout->addStretch();
    return page;
}
}
