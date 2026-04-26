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
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Affix to Top (Scroll the area below)"));
        auto* cl = card->bodyLayout();
        auto* scrollArea = new QScrollArea();
        scrollArea->setFrameShape(QFrame::StyledPanel);
        scrollArea->setFixedHeight(300);
        scrollArea->setVerticalScrollBar(new AntScrollBar(Qt::Vertical));
        auto* scrollContent = new QWidget();
        auto* scrollLayout = new QVBoxLayout(scrollContent);
        scrollLayout->setContentsMargins(16, 16, 16, 16);
        scrollLayout->setSpacing(12);
        for (int i = 0; i < 6; ++i)
            scrollLayout->addWidget(new AntTypography(QStringLiteral("Scroll down to see the affix effect... Line %1").arg(i + 1)));
        auto* affixedBtn = new AntButton(QStringLiteral("Affixed Button (offsetTop=0)"));
        affixedBtn->setButtonType(Ant::ButtonType::Primary);
        scrollLayout->addWidget(affixedBtn);
        for (int i = 0; i < 10; ++i)
            scrollLayout->addWidget(new AntTypography(QStringLiteral("More content line %1").arg(i + 1)));
        scrollLayout->addStretch();
        scrollArea->setWidget(scrollContent);
        cl->addWidget(scrollArea);
        auto* affix = new AntAffix(page);
        affix->setAffixedWidget(affixedBtn);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createAnchorPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    auto* inner = new QWidget();
    auto* innerLayout = new QHBoxLayout(inner);
    innerLayout->setContentsMargins(0, 0, 0, 0);

    auto* scrollArea = new QScrollArea(inner);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumHeight(420);
    scrollArea->setVerticalScrollBar(new AntScrollBar(Qt::Vertical));

    auto* content = new QWidget();
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 24, 24, 24);
    contentLayout->setSpacing(24);

    struct SectionInfo { QString title; QString body; };
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
            QStringLiteral("Section details: this block is intentionally taller so the anchor can track movement while you scroll."), card));
        contentLayout->addWidget(card);
        currentY += card->sizeHint().height() + contentLayout->spacing();
    }
    contentLayout->addStretch();
    scrollArea->setWidget(content);

    auto* anchor = new AntAnchor(inner);
    anchor->setFixedWidth(220);
    anchor->setScrollArea(scrollArea);
    for (int i = 0; i < sections.size(); ++i)
        anchor->addLink(sections[i].title, offsets[i]);
    QObject::connect(anchor, &AntAnchor::linkClicked, scrollArea, [scrollArea](int, int targetY) {
        scrollArea->verticalScrollBar()->setValue(targetY);
    });

    innerLayout->addWidget(scrollArea, 1);
    innerLayout->addWidget(anchor);

    auto* card = new AntCard(QStringLiteral("Anchor"));
    auto* cl = card->bodyLayout();
    cl->addWidget(inner);
    layout->addWidget(card);

    layout->addStretch();
    return page;
}

QWidget* createBreadcrumbPage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basic = new AntBreadcrumb();
        basic->addItem(QStringLiteral("Home"), QStringLiteral("/"));
        basic->addItem(QStringLiteral("Application Center"), QStringLiteral("/apps"));
        basic->addItem(QStringLiteral("Application List"), QStringLiteral("/apps/list"));
        basic->addItem(QStringLiteral("An Application"));
        QObject::connect(basic, &AntBreadcrumb::itemClicked, owner, [owner](int, const QString& title, const QString&) {
            AntMessage::info(QStringLiteral("Clicked %1").arg(title), owner, 1600);
        });
        cl->addWidget(basic);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Icon and Custom Separator"));
        auto* cl = card->bodyLayout();
        auto* custom = new AntBreadcrumb();
        custom->setSeparator(QStringLiteral(">"));
        custom->addItem(QStringLiteral("Home"), QStringLiteral("/"), QStringLiteral("H"));
        custom->addItem(QStringLiteral("Library"), QStringLiteral("/library"), QStringLiteral("L"));
        custom->addItem(QStringLiteral("Data"));
        cl->addWidget(custom);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Disabled"));
        auto* cl = card->bodyLayout();
        auto* disabled = new AntBreadcrumb();
        disabled->addItem(QStringLiteral("Workspace"), QStringLiteral("/workspace"));
        disabled->addItem(QStringLiteral("Disabled link"), QStringLiteral("/disabled"), QString(), true);
        disabled->addItem(QStringLiteral("Current"));
        cl->addWidget(disabled);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createDropdownPage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* row = new QHBoxLayout();
        row->setSpacing(16);
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
        row->addWidget(hoverButton);
        row->addWidget(clickButton);
        row->addStretch();
        cl->addLayout(row);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Placement"));
        auto* cl = card->bodyLayout();
        auto* row = new QHBoxLayout();
        row->setSpacing(16);
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
        row->addWidget(bottomLeftButton);
        row->addWidget(bottomRightButton);
        row->addWidget(topButton);
        row->addStretch();
        cl->addLayout(row);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Context Menu"));
        auto* cl = card->bodyLayout();
        auto* contextArea = new AntWidget(page);
        auto* contextLayout = new QVBoxLayout(contextArea);
        contextLayout->setContentsMargins(0, 0, 0, 0);
        auto* contextLabel = new AntTypography(QStringLiteral("Right click to open context dropdown"));
        contextLayout->addWidget(contextLabel, 0, Qt::AlignCenter);
        contextArea->setMinimumSize(260, 120);
        auto* contextDropdown = new AntDropdown(page);
        contextDropdown->setTrigger(Ant::DropdownTrigger::ContextMenu);
        contextDropdown->setTarget(contextArea);
        contextDropdown->addItem(QStringLiteral("copy"), QStringLiteral("Copy"));
        contextDropdown->addItem(QStringLiteral("paste"), QStringLiteral("Paste"));
        contextDropdown->addItem(QStringLiteral("rename"), QStringLiteral("Rename"));
        QObject::connect(contextDropdown, &AntDropdown::itemTriggered, owner, [owner](const QString& key) {
            AntMessage::success(QStringLiteral("Context action: %1").arg(key), owner, 1400);
        });
        cl->addWidget(contextArea, 0, Qt::AlignLeft);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createMenuPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Inline"));
        auto* cl = card->bodyLayout();
        auto* row = new QHBoxLayout();
        row->setSpacing(18);
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
        row->addWidget(inlineMenu);
        auto* collapsedMenu = new AntMenu();
        collapsedMenu->setMode(Ant::MenuMode::Inline);
        collapsedMenu->setInlineCollapsed(true);
        collapsedMenu->setSelectedKey(QStringLiteral("projects"));
        collapsedMenu->addItem(QStringLiteral("dashboard"), QStringLiteral("Dashboard"), QStringLiteral("D"));
        collapsedMenu->addItem(QStringLiteral("projects"), QStringLiteral("Projects"), QStringLiteral("P"));
        collapsedMenu->addItem(QStringLiteral("settings"), QStringLiteral("Settings"), QStringLiteral("S"));
        collapsedMenu->setFixedWidth(80);
        collapsedMenu->setMinimumHeight(250);
        row->addWidget(collapsedMenu);
        row->addStretch();
        cl->addLayout(row);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Horizontal"));
        auto* cl = card->bodyLayout();
        auto* horizontal = new AntMenu();
        horizontal->setMode(Ant::MenuMode::Horizontal);
        horizontal->setSelectedKey(QStringLiteral("mail"));
        horizontal->addItem(QStringLiteral("mail"), QStringLiteral("Mail"), QStringLiteral("M"));
        horizontal->addItem(QStringLiteral("calendar"), QStringLiteral("Calendar"), QStringLiteral("C"));
        horizontal->addSubMenu(QStringLiteral("reports"), QStringLiteral("Reports"), QStringLiteral("R"));
        horizontal->addItem(QStringLiteral("disabled"), QStringLiteral("Disabled"), QStringLiteral("X"), QString(), true);
        horizontal->setMinimumHeight(48);
        cl->addWidget(horizontal);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Dark and Danger"));
        auto* cl = card->bodyLayout();
        auto* darkMenu = new AntMenu();
        darkMenu->setMenuTheme(Ant::MenuTheme::Dark);
        darkMenu->setMode(Ant::MenuMode::Vertical);
        darkMenu->setSelectedKey(QStringLiteral("profile"));
        darkMenu->addItem(QStringLiteral("profile"), QStringLiteral("Profile"), QStringLiteral("P"));
        darkMenu->addItem(QStringLiteral("billing"), QStringLiteral("Billing"), QStringLiteral("B"));
        darkMenu->addItem(QStringLiteral("danger"), QStringLiteral("Danger zone"), QStringLiteral("!"), QString(), false, true);
        darkMenu->addItem(QStringLiteral("disabled-dark"), QStringLiteral("Disabled"), QStringLiteral("X"), QString(), true);
        darkMenu->setMinimumHeight(200);
        cl->addWidget(darkMenu);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createPaginationPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basic = new AntPagination();
        basic->setTotal(120);
        basic->setCurrent(3);
        basic->setShowTotal(true);
        cl->addWidget(basic);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Size and Less Items"));
        auto* cl = card->bodyLayout();
        auto* large = new AntPagination();
        large->setPaginationSize(Ant::Size::Large);
        large->setTotal(260);
        large->setCurrent(8);
        cl->addWidget(large);
        auto* small = new AntPagination();
        small->setPaginationSize(Ant::Size::Small);
        small->setTotal(260);
        small->setCurrent(8);
        small->setShowLessItems(true);
        cl->addWidget(small);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Simple and Options"));
        auto* cl = card->bodyLayout();
        auto* simple = new AntPagination();
        simple->setSimple(true);
        simple->setTotal(90);
        simple->setCurrent(4);
        cl->addWidget(simple);
        auto* options = new AntPagination();
        options->setTotal(320);
        options->setCurrent(6);
        options->setShowSizeChanger(true);
        options->setShowQuickJumper(true);
        cl->addWidget(options);
        auto* disabled = new AntPagination();
        disabled->setTotal(80);
        disabled->setCurrent(2);
        disabled->setDisabled(true);
        cl->addWidget(disabled);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createStepsPage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basic = new AntSteps(page);
        basic->addStep(QStringLiteral("Create Workspace"), QStringLiteral("Set up name, visibility and project members."));
        basic->addStep(QStringLiteral("Configure Access"), QStringLiteral("Choose environments and grant permissions."));
        basic->addStep(QStringLiteral("Review & Launch"), QStringLiteral("Confirm settings and publish to the team."));
        basic->setCurrentIndex(1);
        cl->addWidget(basic);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Clickable and Error State"));
        auto* cl = card->bodyLayout();
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
        cl->addWidget(interactive);
        cl->addWidget(summary);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Vertical"));
        auto* cl = card->bodyLayout();
        auto* vertical = new AntSteps(page);
        vertical->setDirection(Ant::Orientation::Vertical);
        vertical->setClickable(false);
        vertical->addStep(QStringLiteral("Submitted"), QStringLiteral("The request has been created and queued."), QStringLiteral("09:12"), Ant::StepStatus::Finish);
        vertical->addStep(QStringLiteral("Security Review"), QStringLiteral("Scanning package and permission changes."), QStringLiteral("10:24"));
        vertical->addStep(QStringLiteral("Deployment"), QStringLiteral("Waiting for review to complete."), QStringLiteral("Pending"));
        vertical->setCurrentIndex(1);
        vertical->setMinimumHeight(280);
        cl->addWidget(vertical);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
