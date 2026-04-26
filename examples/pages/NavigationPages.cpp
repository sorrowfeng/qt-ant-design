#include "Pages.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QList>
#include <QScrollArea>
#include <QScrollBar>
#include <QString>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "PageCommon.h"
#include "core/AntTheme.h"
#include "core/AntTypes.h"
#include "widgets/AntAffix.h"
#include "widgets/AntAnchor.h"
#include "widgets/AntBreadcrumb.h"
#include "widgets/AntButton.h"
#include "widgets/AntCard.h"
#include "widgets/AntDropdown.h"
#include "widgets/AntMenu.h"
#include "widgets/AntMessage.h"
#include "widgets/AntPagination.h"
#include "widgets/AntScrollBar.h"
#include "widgets/AntSteps.h"
#include "widgets/AntTypography.h"
#include "widgets/AntWidget.h"

namespace example::pages
{
QWidget* createAffixPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Affix to Top (Scroll the area below)")));

    auto* scrollArea = new QScrollArea();
    scrollArea->setFrameShape(QFrame::StyledPanel);
    scrollArea->setFixedHeight(300);
    scrollArea->setVerticalScrollBar(new AntScrollBar(Qt::Vertical));

    auto* scrollContent = new QWidget();
    auto* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(16, 16, 16, 16);
    scrollLayout->setSpacing(12);

    for (int i = 0; i < 6; ++i)
    {
        scrollLayout->addWidget(new AntTypography(QStringLiteral("Scroll down to see the affix effect... Line %1").arg(i + 1)));
    }

    auto* affixedBtn = new AntButton(QStringLiteral("Affixed Button (offsetTop=0)"));
    affixedBtn->setButtonType(Ant::ButtonType::Primary);
    scrollLayout->addWidget(affixedBtn);

    for (int i = 0; i < 10; ++i)
    {
        scrollLayout->addWidget(new AntTypography(QStringLiteral("More content line %1").arg(i + 1)));
    }

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea);

    auto* affix = new AntAffix(page);
    affix->setAffixedWidget(affixedBtn);

    layout->addStretch();
    return page;
}

QWidget* createAnchorPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QHBoxLayout(page);

    auto* scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumHeight(420);
    scrollArea->setVerticalScrollBar(new AntScrollBar(Qt::Vertical));

    auto* content = new QWidget();
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 24, 24, 24);
    contentLayout->setSpacing(24);

    struct SectionInfo
    {
        QString title;
        QString body;
    };
    const QList<SectionInfo> sections = {
        {QStringLiteral("Overview"), QStringLiteral("Anchor tracks the current reading position and highlights the active link.")},
        {QStringLiteral("Workflow"), QStringLiteral("Use it to jump between long sections in forms, docs or settings panels.")},
        {QStringLiteral("Tokens"), QStringLiteral("Spacing, text hierarchy and active color should stay aligned with the current theme.")},
        {QStringLiteral("Integration"), QStringLiteral("Pair it with a scroll area and drive the target scroll position from linkClicked.")},
    };

    QVector<int> offsets;
    int currentY = 0;
    for (const auto& section : sections)
    {
        offsets.append(currentY);

        auto* card = new AntCard(section.title, content);
        auto* bodyLayout = card->bodyLayout();
        bodyLayout->addWidget(new AntTypography(section.body, card));
        bodyLayout->addWidget(new AntTypography(
            QStringLiteral("Section details: this block is intentionally taller so the anchor can track movement while you scroll."),
            card));
        bodyLayout->addWidget(new AntTypography(
            QStringLiteral("The demo keeps the interaction simple: click a link on the right and the content scrolls to that section."),
            card));
        contentLayout->addWidget(card);

        currentY += card->sizeHint().height() + contentLayout->spacing();
    }
    contentLayout->addStretch();
    scrollArea->setWidget(content);

    auto* anchor = new AntAnchor(page);
    anchor->setFixedWidth(220);
    anchor->setScrollArea(scrollArea);
    for (int i = 0; i < sections.size(); ++i)
        anchor->addLink(sections[i].title, offsets[i]);

    QObject::connect(anchor, &AntAnchor::linkClicked, scrollArea, [scrollArea](int, int targetY) {
        scrollArea->verticalScrollBar()->setValue(targetY);
    });

    layout->addWidget(scrollArea, 1);
    layout->addWidget(anchor);
    return page;
}

QWidget* createBreadcrumbPage(QWidget* owner)
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
    QObject::connect(basic, &AntBreadcrumb::itemClicked, owner, [owner](int, const QString& title, const QString&) {
        AntMessage::info(QStringLiteral("Clicked %1").arg(title), owner, 1600);
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

QWidget* createDropdownPage(QWidget* owner)
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
    QObject::connect(hoverDropdown, &AntDropdown::itemTriggered, owner, [owner](const QString& key) {
        AntMessage::info(QStringLiteral("Triggered %1").arg(key), owner, 1400);
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

    auto* contextArea = new AntWidget(page);
    auto* contextLayout = new QVBoxLayout(contextArea);
    contextLayout->setContentsMargins(0, 0, 0, 0);
    auto* contextLabel = new AntTypography(QStringLiteral("Context trigger area"));
    contextLayout->addWidget(contextLabel, 0, Qt::AlignCenter);
    contextArea->setMinimumSize(260, 120);
    contextArea->setStyleSheet(QStringLiteral("border: 1px dashed %1; border-radius: 8px;").arg(antTheme->tokens().colorBorder.name()));
    auto* contextDropdown = new AntDropdown(page);
    contextDropdown->setTrigger(Ant::DropdownTrigger::ContextMenu);
    contextDropdown->setTarget(contextArea);
    contextDropdown->addItem(QStringLiteral("copy"), QStringLiteral("Copy"));
    contextDropdown->addItem(QStringLiteral("paste"), QStringLiteral("Paste"));
    contextDropdown->addItem(QStringLiteral("rename"), QStringLiteral("Rename"));
    QObject::connect(contextDropdown, &AntDropdown::itemTriggered, owner, [owner](const QString& key) {
        AntMessage::success(QStringLiteral("Context action: %1").arg(key), owner, 1400);
    });
    layout->addWidget(contextArea, 0, Qt::AlignLeft);

    layout->addStretch();
    return page;
}

QWidget* createMenuPage(QWidget* /*owner*/)
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

QWidget* createPaginationPage(QWidget* /*owner*/)
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

QWidget* createStepsPage(QWidget* owner)
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
    QObject::connect(interactive, &AntSteps::stepClicked, owner, [summary, interactive](int index) {
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
}
