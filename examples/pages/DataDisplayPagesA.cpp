#include "Pages.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QPair>
#include <QPoint>
#include <QSize>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "PageCommon.h"
#include "core/AntTypes.h"
#include "widgets/AntAvatar.h"
#include "widgets/AntBadge.h"
#include "widgets/AntButton.h"
#include "widgets/AntCalendar.h"
#include "widgets/AntCard.h"
#include "widgets/AntCarousel.h"
#include "widgets/AntCollapse.h"
#include "widgets/AntDescriptions.h"
#include "widgets/AntEmpty.h"
#include "widgets/AntImage.h"
#include "widgets/AntList.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntTag.h"
#include "widgets/AntTypography.h"

namespace example::pages
{
QWidget* createAvatarPage(QWidget* /*owner*/)
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

    layout->addWidget(createSectionTitle(QStringLiteral("Group")));
    auto* groupRow = new QHBoxLayout();
    groupRow->setSpacing(28);

    auto* group1 = new AntAvatarGroup(page);
    group1->setMaxCount(3);
    group1->addAvatar(new AntAvatar(QStringLiteral("A")));
    group1->addAvatar(new AntAvatar(QStringLiteral("B")));
    group1->addAvatar(new AntAvatar(QStringLiteral("C")));
    group1->addAvatar(new AntAvatar(QStringLiteral("D")));
    group1->addAvatar(new AntAvatar(QStringLiteral("E")));
    groupRow->addWidget(group1);

    auto* group2 = new AntAvatarGroup(page);
    group2->setMaxCount(4);
    group2->setAvatarSize(Ant::AvatarSize::Small);
    for (const QString& name : {QStringLiteral("X"), QStringLiteral("Y"), QStringLiteral("Z"), QStringLiteral("W"), QStringLiteral("V"), QStringLiteral("U")})
    {
        auto* av = new AntAvatar(name);
        av->setShape(Ant::AvatarShape::Square);
        group2->addAvatar(av);
    }
    groupRow->addWidget(group2);
    groupRow->addStretch();
    layout->addLayout(groupRow);

    layout->addStretch();
    return page;
}

QWidget* createBadgePage(QWidget* /*owner*/)
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

    layout->addWidget(createSectionTitle(QStringLiteral("Ribbon")));
    auto* ribbonRow = new QHBoxLayout();
    ribbonRow->setSpacing(28);

    auto* ribbonCard1 = new AntCard(QStringLiteral("Pushes Open"), page);
    ribbonCard1->setFixedWidth(280);
    ribbonCard1->setMinimumHeight(120);
    ribbonCard1->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Card with a default ribbon badge in the corner.")));
    auto* ribbon1 = new AntBadge();
    ribbon1->setBadgeMode(Ant::BadgeMode::Ribbon);
    ribbon1->setRibbonText(QStringLiteral("Ribbon"));
    ribbon1->setContentWidget(ribbonCard1);
    ribbonRow->addWidget(ribbon1);

    auto* ribbonCard2 = new AntCard(QStringLiteral("Pushes Open"), page);
    ribbonCard2->setFixedWidth(280);
    ribbonCard2->setMinimumHeight(120);
    ribbonCard2->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Card with a colored ribbon badge.")));
    auto* ribbon2 = new AntBadge();
    ribbon2->setBadgeMode(Ant::BadgeMode::Ribbon);
    ribbon2->setRibbonText(QStringLiteral("Success"));
    ribbon2->setRibbonColor(QStringLiteral("success"));
    ribbon2->setContentWidget(ribbonCard2);
    ribbonRow->addWidget(ribbon2);
    ribbonRow->addStretch();
    layout->addLayout(ribbonRow);

    layout->addStretch();
    return page;
}

QWidget* createCalendarPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntCalendar"));
    QFont titleFont = title->font();
    titleFont.setPixelSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    auto* cal = new AntCalendar(page);
    layout->addWidget(cal);

    auto* dateLabel = new QLabel(QStringLiteral("Selected: ") + cal->selectedDate().toString(QStringLiteral("yyyy-MM-dd")));
    QObject::connect(cal, &AntCalendar::clicked, dateLabel, [dateLabel, cal]() {
        dateLabel->setText(QStringLiteral("Selected: ") + cal->selectedDate().toString(QStringLiteral("yyyy-MM-dd")));
    });
    layout->addWidget(dateLabel);

    layout->addStretch();
    return page;
}

QWidget* createCardPage(QWidget* /*owner*/)
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
    basic->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Card content\nCard content\nCard content")));

    auto* hoverable = new AntCard(QStringLiteral("Hoverable"));
    hoverable->setHoverable(true);
    hoverable->setMinimumSize(280, 180);
    hoverable->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Move the mouse over this card.")));

    auto* loading = new AntCard(QStringLiteral("Loading"));
    loading->setLoading(true);
    loading->setMinimumSize(280, 180);
    loading->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Loading mask and spinner")));
    row->addWidget(basic);
    row->addWidget(hoverable);
    row->addWidget(loading);
    layout->addLayout(row);

    auto* actionCard = new AntCard(QStringLiteral("Actions"));
    actionCard->setMinimumHeight(180);
    actionCard->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Card with action slots.")));
    actionCard->addActionWidget(new AntTypography(QStringLiteral("Edit")));
    actionCard->addActionWidget(new AntTypography(QStringLiteral("Share")));
    actionCard->addActionWidget(new AntTypography(QStringLiteral("Delete")));
    layout->addWidget(actionCard);

    layout->addWidget(createSectionTitle(QStringLiteral("Meta")));
    auto* metaRow = new QHBoxLayout();
    metaRow->setSpacing(18);

    auto* metaCard = new AntCard(QStringLiteral("Project Card"), page);
    metaCard->setFixedWidth(320);
    metaCard->setMetaTitle(QStringLiteral("Ant Design"));
    metaCard->setMetaDescription(QStringLiteral("A design system for enterprise-level products."));
    metaCard->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Card body content goes here.")));
    metaRow->addWidget(metaCard);

    auto* metaCard2 = new AntCard(QStringLiteral("With Avatar"), page);
    metaCard2->setFixedWidth(320);
    auto* metaAvatar = new AntAvatar(QStringLiteral("AD"));
    metaAvatar->setAvatarSize(Ant::AvatarSize::Small);
    metaCard2->setMetaAvatar(metaAvatar);
    metaCard2->setMetaTitle(QStringLiteral("Design Team"));
    metaCard2->setMetaDescription(QStringLiteral("Shared component library and design tokens."));
    metaRow->addWidget(metaCard2);
    metaRow->addStretch();
    layout->addLayout(metaRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Grid")));
    auto* gridCard = new AntCard(QStringLiteral("Grid Layout"), page);
    gridCard->setMinimumHeight(200);
    gridCard->addGridItem(new AntTypography(QStringLiteral("Grid Item 1")));
    gridCard->addGridItem(new AntTypography(QStringLiteral("Grid Item 2")));
    gridCard->addGridItem(new AntTypography(QStringLiteral("Grid Item 3")));
    gridCard->addGridItem(new AntTypography(QStringLiteral("Grid Item 4")));
    gridCard->addGridItem(new AntTypography(QStringLiteral("Grid Item 5")));
    gridCard->addGridItem(new AntTypography(QStringLiteral("Grid Item 6")));
    layout->addWidget(gridCard);

    layout->addStretch();
    return page;
}

QWidget* createCarouselPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntCarousel"));
    QFont f = title->font();
    f.setPixelSize(20);
    f.setBold(true);
    title->setFont(f);
    layout->addWidget(title);

    auto* carousel = new AntCarousel(page);
    carousel->setMinimumHeight(220);
    carousel->setInterval(2200);

    const QList<QPair<QString, QString>> slides = {
        {QStringLiteral("#1677ff"), QStringLiteral("Release Status")},
        {QStringLiteral("#13c2c2"), QStringLiteral("Design Tokens")},
        {QStringLiteral("#52c41a"), QStringLiteral("Gallery Coverage")},
    };

    for (const auto& slide : slides)
    {
        auto* card = new QWidget();
        card->setStyleSheet(QStringLiteral("background:%1; border-radius:12px;").arg(slide.first));
        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(24, 24, 24, 24);
        auto* heading = new QLabel(slide.second, card);
        QFont headingFont = heading->font();
        headingFont.setPixelSize(24);
        headingFont.setBold(true);
        heading->setFont(headingFont);
        heading->setStyleSheet(QStringLiteral("color:white;"));
        cardLayout->addWidget(heading);
        auto* body = new QLabel(QStringLiteral("Click the left/right side of the carousel to switch slides. Dots stay in sync with the current page."), card);
        body->setWordWrap(true);
        body->setStyleSheet(QStringLiteral("color:white;"));
        cardLayout->addWidget(body);
        cardLayout->addStretch();
        carousel->addSlide(card);
    }

    auto* current = new QLabel(QStringLiteral("Current slide: 1 / %1").arg(carousel->count()), page);
    QObject::connect(carousel, &AntCarousel::currentIndexChanged, current, [current, carousel](int index) {
        current->setText(QStringLiteral("Current slide: %1 / %2").arg(index + 1).arg(carousel->count()));
    });

    auto* controls = new QHBoxLayout();
    auto* autoplay = new AntSwitch();
    autoplay->setChecked(carousel->autoPlay());
    auto* dots = new AntSwitch();
    dots->setChecked(carousel->showDots());
    controls->addWidget(new QLabel(QStringLiteral("Auto play")));
    controls->addWidget(autoplay);
    controls->addSpacing(16);
    controls->addWidget(new QLabel(QStringLiteral("Show dots")));
    controls->addWidget(dots);
    controls->addStretch();

    QObject::connect(autoplay, &AntSwitch::checkedChanged, carousel, &AntCarousel::setAutoPlay);
    QObject::connect(dots, &AntSwitch::checkedChanged, carousel, &AntCarousel::setShowDots);

    layout->addWidget(carousel);
    layout->addWidget(current);
    layout->addLayout(controls);
    layout->addStretch();
    return page;
}

QWidget* createCollapsePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntCollapse"));
    QFont f = title->font();
    f.setPixelSize(20);
    f.setBold(true);
    title->setFont(f);
    layout->addWidget(title);

    auto* collapse = new AntCollapse(page);
    collapse->setAccordion(true);

    auto* p1 = collapse->addPanel(QStringLiteral("Panel 1 — First Item"));
    auto* c1 = new QWidget();
    auto* l1 = new QVBoxLayout(c1);
    l1->addWidget(new QLabel(QStringLiteral("Content for panel 1. This area expands and collapses.")));
    p1->setContentWidget(c1);

    auto* p2 = collapse->addPanel(QStringLiteral("Panel 2 — Second Item"));
    auto* c2 = new QWidget();
    auto* l2 = new QVBoxLayout(c2);
    l2->addWidget(new QLabel(QStringLiteral("Panel 2 content here.")));
    p2->setContentWidget(c2);

    auto* p3 = collapse->addPanel(QStringLiteral("Panel 3 — Third Item (Disabled)"));
    p3->setDisabled(true);
    auto* c3 = new QWidget();
    auto* l3 = new QVBoxLayout(c3);
    l3->addWidget(new QLabel(QStringLiteral("This panel is disabled.")));
    p3->setContentWidget(c3);

    layout->addWidget(collapse);
    layout->addStretch();
    return page;
}

QWidget* createDescriptionsPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(28);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basic = new AntDescriptions(page);
    basic->setTitle(QStringLiteral("User Info"));
    basic->setColumnCount(3);
    basic->addItem(QStringLiteral("Name"), QStringLiteral("Liam Parker"));
    basic->addItem(QStringLiteral("Status"), QStringLiteral("Active"));
    basic->addItem(QStringLiteral("Region"), QStringLiteral("Singapore"));
    basic->addItem(QStringLiteral("Address"), QStringLiteral("88 Market Street, Floor 12"), 2);
    basic->addItem(QStringLiteral("Remarks"), QStringLiteral("Owns release workflow and deployment sign-off."));
    layout->addWidget(basic);

    layout->addWidget(createSectionTitle(QStringLiteral("Bordered")));
    auto* bordered = new AntDescriptions(page);
    bordered->setTitle(QStringLiteral("Order Summary"));
    bordered->setExtra(QStringLiteral("Processing"));
    bordered->setBordered(true);
    bordered->setColumnCount(2);
    bordered->addItem(QStringLiteral("Order No."), QStringLiteral("#SO-2048"));
    bordered->addItem(QStringLiteral("Placed At"), QStringLiteral("2026-04-24 14:30"));
    bordered->addItem(QStringLiteral("Amount"), QStringLiteral("$ 2,499.00"));
    auto* tagItem = new AntDescriptionsItem(QStringLiteral("Priority"), QString(), bordered);
    auto* priority = new AntTag(QStringLiteral("High"), bordered);
    priority->setColor(QStringLiteral("#fa541c"));
    priority->setVariant(Ant::TagVariant::Solid);
    tagItem->setContentWidget(priority);
    bordered->addItem(tagItem);
    bordered->addItem(QStringLiteral("Notes"), QStringLiteral("Customer requested expedited shipping and proactive status updates."), 2);
    layout->addWidget(bordered);

    layout->addWidget(createSectionTitle(QStringLiteral("Vertical")));
    auto* vertical = new AntDescriptions(page);
    vertical->setTitle(QStringLiteral("Deployment Details"));
    vertical->setVertical(true);
    vertical->setBordered(true);
    vertical->setColumnCount(3);
    vertical->addItem(QStringLiteral("Environment"), QStringLiteral("Production"));
    vertical->addItem(QStringLiteral("Version"), QStringLiteral("v2.8.1"));
    vertical->addItem(QStringLiteral("Triggered By"), QStringLiteral("CI Pipeline"));
    vertical->addItem(QStringLiteral("Rollback Plan"), QStringLiteral("Automatic rollback after 5 minutes of elevated error rate."), 2);
    vertical->addItem(QStringLiteral("Approval"), QStringLiteral("Ready"));
    layout->addWidget(vertical);

    layout->addStretch();
    return page;
}

QWidget* createEmptyPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(28);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(28);

    auto* basic = new AntEmpty(page);
    basic->setFixedWidth(260);

    auto* customText = new AntEmpty(page);
    customText->setFixedWidth(320);
    customText->setDescription(QStringLiteral("No matching issues found. Try adjusting the filters and search terms."));

    auto* simple = new AntEmpty(page);
    simple->setFixedWidth(220);
    simple->setSimple(true);
    simple->setDescription(QStringLiteral("No notifications"));

    basicRow->addWidget(basic);
    basicRow->addWidget(customText);
    basicRow->addWidget(simple);
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("With Action")));
    auto* actionRow = new QHBoxLayout();
    actionRow->setSpacing(28);

    auto* createProject = new AntEmpty(page);
    createProject->setFixedWidth(360);
    createProject->setDescription(QStringLiteral("You have not created any projects yet."));
    auto* createButton = new AntButton(QStringLiteral("Create Project"), createProject);
    createButton->setButtonType(Ant::ButtonType::Primary);
    createProject->setExtraWidget(createButton);

    auto* importData = new AntEmpty(page);
    importData->setFixedWidth(360);
    importData->setImageSize(QSize(148, 92));
    importData->setDescription(QStringLiteral("Import data from CSV or connect a remote source to get started."));
    auto* importButton = new AntButton(QStringLiteral("Import Data"), importData);
    importButton->setButtonType(Ant::ButtonType::Default);
    importData->setExtraWidget(importButton);

    actionRow->addWidget(createProject);
    actionRow->addWidget(importData);
    actionRow->addStretch();
    layout->addLayout(actionRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Embedded in Card")));
    auto* cardRow = new QHBoxLayout();
    cardRow->setSpacing(24);

    auto* emptyCard = new AntCard(QStringLiteral("Recent Activity"), page);
    emptyCard->setFixedWidth(420);
    emptyCard->setExtra(QStringLiteral("0 items"));
    auto* cardEmpty = new AntEmpty(emptyCard);
    cardEmpty->setDescription(QStringLiteral("There is no activity in this workspace yet."));
    auto* inviteButton = new AntButton(QStringLiteral("Invite Collaborators"), cardEmpty);
    inviteButton->setButtonType(Ant::ButtonType::Primary);
    cardEmpty->setExtraWidget(inviteButton);
    emptyCard->bodyLayout()->addWidget(cardEmpty);

    auto* placeholderCard = new AntCard(QStringLiteral("Archived Reports"), page);
    placeholderCard->setFixedWidth(420);
    auto* archiveEmpty = new AntEmpty(placeholderCard);
    archiveEmpty->setSimple(true);
    archiveEmpty->setImageVisible(false);
    archiveEmpty->setDescription(QStringLiteral("Archived reports will appear here."));
    placeholderCard->bodyLayout()->addWidget(archiveEmpty);

    cardRow->addWidget(emptyCard);
    cardRow->addWidget(placeholderCard);
    cardRow->addStretch();
    layout->addLayout(cardRow);

    layout->addStretch();
    return page;
}

QWidget* createImagePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntImage"));
    QFont f = title->font();
    f.setPixelSize(20);
    f.setBold(true);
    title->setFont(f);
    layout->addWidget(title);

    auto* img = new AntImage(page);
    img->setAlt(QStringLiteral("No image loaded"));
    img->setImgWidth(200);
    img->setImgHeight(200);
    layout->addWidget(img);

    auto* tip = new QLabel(QStringLiteral("Set src to load an image. Click to preview."));
    layout->addWidget(tip);

    layout->addStretch();
    return page;
}

QWidget* createListPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(28);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicList = new AntList(page);
    basicList->setBordered(true);
    basicList->setFixedWidth(480);

    for (int i = 1; i <= 4; ++i)
    {
        auto* item = new AntListItem(basicList);
        auto* meta = new AntListItemMeta(item);
        meta->setTitle(QStringLiteral("Ant Design List Item %1").arg(i));
        meta->setDescription(QStringLiteral("Description for item %1").arg(i));
        item->setMeta(meta);
        basicList->addItem(item);
    }

    layout->addWidget(basicList);

    layout->addWidget(createSectionTitle(QStringLiteral("With Header and Footer")));
    auto* headerList = new AntList(page);
    headerList->setBordered(true);
    headerList->setFixedWidth(480);

    auto* header = new AntTypography(QStringLiteral("Header"), headerList);
    header->setTitle(true);
    header->setTitleLevel(Ant::TypographyTitleLevel::H5);
    header->setFixedHeight(40);
    headerList->setHeaderWidget(header);

    auto* footer = new AntTypography(QStringLiteral("Footer"), headerList);
    footer->setType(Ant::TypographyType::Secondary);
    footer->setFixedHeight(40);
    headerList->setFooterWidget(footer);

    for (int i = 1; i <= 3; ++i)
    {
        auto* item = new AntListItem(headerList);
        auto* meta = new AntListItemMeta(item);
        meta->setTitle(QStringLiteral("Item %1").arg(i));
        item->setMeta(meta);
        headerList->addItem(item);
    }

    layout->addWidget(headerList);

    layout->addWidget(createSectionTitle(QStringLiteral("Split and Size")));
    auto* splitRow = new QHBoxLayout();
    splitRow->setSpacing(28);

    auto* noSplit = new AntList(page);
    noSplit->setBordered(true);
    noSplit->setSplit(false);
    noSplit->setFixedWidth(240);
    for (int i = 1; i <= 3; ++i)
    {
        auto* item = new AntListItem(noSplit);
        auto* meta = new AntListItemMeta(item);
        meta->setTitle(QStringLiteral("No Split %1").arg(i));
        item->setMeta(meta);
        noSplit->addItem(item);
    }

    auto* smallList = new AntList(page);
    smallList->setBordered(true);
    smallList->setListSize(AntList::Small);
    smallList->setFixedWidth(240);
    for (int i = 1; i <= 3; ++i)
    {
        auto* item = new AntListItem(smallList);
        auto* meta = new AntListItemMeta(item);
        meta->setTitle(QStringLiteral("Small Item %1").arg(i));
        item->setMeta(meta);
        smallList->addItem(item);
    }

    auto* largeList = new AntList(page);
    largeList->setBordered(true);
    largeList->setListSize(AntList::Large);
    largeList->setFixedWidth(240);
    for (int i = 1; i <= 3; ++i)
    {
        auto* item = new AntListItem(largeList);
        auto* meta = new AntListItemMeta(item);
        meta->setTitle(QStringLiteral("Large Item %1").arg(i));
        item->setMeta(meta);
        largeList->addItem(item);
    }

    splitRow->addWidget(noSplit);
    splitRow->addWidget(smallList);
    splitRow->addWidget(largeList);
    splitRow->addStretch();
    layout->addLayout(splitRow);

    layout->addStretch();
    return page;
}
}
