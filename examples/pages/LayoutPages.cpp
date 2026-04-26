#include "Pages.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "PageCommon.h"
#include "core/AntTypes.h"
#include "widgets/AntButton.h"
#include "widgets/AntDivider.h"
#include "widgets/AntFlex.h"
#include "widgets/AntGrid.h"
#include "widgets/AntLayout.h"
#include "widgets/AntSpace.h"
#include "widgets/AntSplitter.h"
#include "widgets/AntTypography.h"

namespace example::pages
{
QWidget* createDividerPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(20);

    layout->addWidget(createSectionTitle(QStringLiteral("Horizontal")));
    layout->addWidget(new AntTypography(QStringLiteral("Ant Design")));
    layout->addWidget(new AntDivider());
    layout->addWidget(new AntTypography(QStringLiteral("Qt Widgets")));

    layout->addWidget(createSectionTitle(QStringLiteral("With Text")));
    layout->addWidget(new AntDivider(QStringLiteral("Center Text")));
    auto* start = new AntDivider(QStringLiteral("Start Text"));
    start->setTitlePlacement(Ant::DividerTitlePlacement::Start);
    layout->addWidget(start);
    auto* end = new AntDivider(QStringLiteral("End Text"));
    end->setTitlePlacement(Ant::DividerTitlePlacement::End);
    end->setPlain(false);
    layout->addWidget(end);

    layout->addWidget(createSectionTitle(QStringLiteral("Variant and Size")));
    auto* dashed = new AntDivider(QStringLiteral("Dashed"));
    dashed->setVariant(Ant::DividerVariant::Dashed);
    dashed->setDividerSize(Ant::Size::Small);
    layout->addWidget(dashed);
    auto* dotted = new AntDivider(QStringLiteral("Dotted"));
    dotted->setVariant(Ant::DividerVariant::Dotted);
    dotted->setDividerSize(Ant::Size::Middle);
    layout->addWidget(dotted);

    layout->addWidget(createSectionTitle(QStringLiteral("Vertical")));
    auto* verticalRow = new QHBoxLayout();
    verticalRow->setSpacing(0);
    verticalRow->addWidget(new AntTypography(QStringLiteral("Text")));
    auto* v1 = new AntDivider();
    v1->setOrientation(Ant::Orientation::Vertical);
    verticalRow->addWidget(v1);
    verticalRow->addWidget(new AntTypography(QStringLiteral("Link")));
    auto* v2 = new AntDivider();
    v2->setOrientation(Ant::Orientation::Vertical);
    v2->setVariant(Ant::DividerVariant::Dashed);
    verticalRow->addWidget(v2);
    verticalRow->addWidget(new AntTypography(QStringLiteral("Action")));
    verticalRow->addStretch();
    layout->addLayout(verticalRow);

    layout->addStretch();
    return page;
}

QWidget* createFlexPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntFlex"));
    QFont f = title->font();
    f.setPixelSize(20);
    f.setBold(true);
    title->setFont(f);
    layout->addWidget(title);

    auto makeChip = [](const QString& text, const QString& color) {
        auto* label = new QLabel(text);
        label->setAlignment(Qt::AlignCenter);
        label->setMinimumHeight(40);
        label->setStyleSheet(QStringLiteral("background:%1; color:white; border-radius:8px; padding:0 14px;").arg(color));
        return label;
    };

    auto* horizontal = new AntFlex(page);
    horizontal->setGap(12);
    horizontal->addWidget(makeChip(QStringLiteral("Design"), QStringLiteral("#1677ff")));
    horizontal->addWidget(makeChip(QStringLiteral("Build"), QStringLiteral("#13c2c2")));
    horizontal->addWidget(makeChip(QStringLiteral("Review"), QStringLiteral("#52c41a")));
    horizontal->addStretch();

    auto* vertical = new AntFlex(page);
    vertical->setVertical(true);
    vertical->setGap(10);
    vertical->addWidget(makeChip(QStringLiteral("Token sync"), QStringLiteral("#722ed1")));
    vertical->addWidget(makeChip(QStringLiteral("Motion polish"), QStringLiteral("#eb2f96")));
    vertical->addWidget(makeChip(QStringLiteral("Example coverage"), QStringLiteral("#fa8c16")));

    layout->addWidget(new QLabel(QStringLiteral("Horizontal gap layout")));
    layout->addWidget(horizontal);
    layout->addSpacing(16);
    layout->addWidget(new QLabel(QStringLiteral("Vertical layout")));
    layout->addWidget(vertical);
    layout->addStretch();
    return page;
}

QWidget* createGridPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntGrid"));
    QFont f = title->font();
    f.setPixelSize(20);
    f.setBold(true);
    title->setFont(f);
    layout->addWidget(title);

    auto makeBlock = [](const QString& text, const QString& color) {
        auto* label = new QLabel(text);
        label->setAlignment(Qt::AlignCenter);
        label->setMinimumHeight(48);
        label->setStyleSheet(QStringLiteral("background:%1; color:white; border-radius:8px;").arg(color));
        return label;
    };

    auto* row1 = new AntRow(page);
    row1->setGutter(12);
    row1->addWidget(makeBlock(QStringLiteral("span 8"), QStringLiteral("#1677ff")), 8);
    row1->addWidget(makeBlock(QStringLiteral("span 8"), QStringLiteral("#13c2c2")), 8);
    row1->addWidget(makeBlock(QStringLiteral("span 8"), QStringLiteral("#52c41a")), 8);

    auto* row2 = new AntRow(page);
    row2->setGutter(12);
    row2->addWidget(makeBlock(QStringLiteral("span 6"), QStringLiteral("#722ed1")), 6);
    row2->addWidget(makeBlock(QStringLiteral("span 6 offset 2"), QStringLiteral("#eb2f96")), 6, 2);
    row2->addWidget(makeBlock(QStringLiteral("span 10"), QStringLiteral("#fa8c16")), 10);

    layout->addWidget(new QLabel(QStringLiteral("24-column layout with span and offset")));
    layout->addWidget(row1);
    layout->addWidget(row2);
    layout->addStretch();
    return page;
}

QWidget* createLayoutPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic Layout")));

    auto* basicLayout = new AntLayout();
    basicLayout->setFixedHeight(240);

    auto* header = new AntLayoutHeader();
    auto* headerLabel = new AntTypography(QStringLiteral("Header"), header);
    headerLabel->setType(Ant::TypographyType::LightSolid);
    headerLabel->setStrong(true);
    headerLabel->setGeometry(16, 16, 200, 32);
    basicLayout->setHeader(header);

    auto* content = new AntLayoutContent();
    auto* contentLabel = new AntTypography(QStringLiteral("Content"), content);
    contentLabel->setGeometry(16, 16, 200, 30);
    basicLayout->setContent(content);

    auto* footer = new AntLayoutFooter();
    auto* footerLabel = new AntTypography(QStringLiteral("Footer"), footer);
    footerLabel->setGeometry(16, 8, 200, 32);
    basicLayout->setFooter(footer);

    layout->addWidget(basicLayout);

    layout->addWidget(createSectionTitle(QStringLiteral("With Sider")));

    auto* siderLayout = new AntLayout();
    siderLayout->setFixedHeight(240);

    auto* siderHeader = new AntLayoutHeader();
    auto* siderHeaderLabel = new AntTypography(QStringLiteral("Header"), siderHeader);
    siderHeaderLabel->setType(Ant::TypographyType::LightSolid);
    siderHeaderLabel->setStrong(true);
    siderHeaderLabel->setGeometry(16, 16, 200, 32);
    siderLayout->setHeader(siderHeader);

    auto* sider = new AntLayoutSider();
    sider->setWidth(200);
    sider->setSiderTheme(Ant::LayoutSiderTheme::Light);
    auto* siderLabel = new AntTypography(QStringLiteral("Sider"), sider);
    siderLabel->setGeometry(16, 8, 200, 30);
    siderLayout->addSider(sider);

    auto* siderContent = new AntLayoutContent();
    auto* siderContentLabel = new AntTypography(QStringLiteral("Content"), siderContent);
    siderContentLabel->setGeometry(16, 16, 200, 30);
    siderLayout->setContent(siderContent);

    layout->addWidget(siderLayout);
    layout->addStretch();
    return page;
}

QWidget* createSpacePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Horizontal (Small)")));
    auto* hSmall = new AntSpace();
    hSmall->setSize(Ant::Size::Small);
    for (int i = 0; i < 4; ++i)
    {
        auto* btn = new AntButton(QStringLiteral("Button %1").arg(i + 1));
        btn->setButtonType(Ant::ButtonType::Primary);
        hSmall->addItem(btn);
    }
    layout->addWidget(hSmall);

    layout->addWidget(createSectionTitle(QStringLiteral("Horizontal (Middle)")));
    auto* hMiddle = new AntSpace();
    hMiddle->setSize(Ant::Size::Middle);
    for (int i = 0; i < 4; ++i)
    {
        auto* btn = new AntButton(QStringLiteral("Button %1").arg(i + 1));
        hMiddle->addItem(btn);
    }
    layout->addWidget(hMiddle);

    layout->addWidget(createSectionTitle(QStringLiteral("Horizontal (Large)")));
    auto* hLarge = new AntSpace();
    hLarge->setSize(Ant::Size::Large);
    for (int i = 0; i < 3; ++i)
    {
        auto* btn = new AntButton(QStringLiteral("Button %1").arg(i + 1));
        hLarge->addItem(btn);
    }
    layout->addWidget(hLarge);

    layout->addWidget(createSectionTitle(QStringLiteral("Vertical")));
    auto* vSpace = new AntSpace();
    vSpace->setOrientation(Ant::Orientation::Vertical);
    vSpace->setSize(Ant::Size::Middle);
    for (int i = 0; i < 3; ++i)
    {
        auto* btn = new AntButton(QStringLiteral("Button %1").arg(i + 1));
        btn->setButtonType(Ant::ButtonType::Primary);
        btn->setBlock(true);
        vSpace->addItem(btn);
    }
    layout->addWidget(vSpace);

    layout->addStretch();
    return page;
}

QWidget* createSplitterPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntSplitter"));
    QFont f = title->font();
    f.setPixelSize(20);
    f.setBold(true);
    title->setFont(f);
    layout->addWidget(title);

    auto* splitter = new AntSplitter(Qt::Horizontal, page);
    auto* left = new QTextEdit(splitter);
    left->setPlainText(QStringLiteral("Left panel"));
    auto* right = new QTextEdit(splitter);
    right->setPlainText(QStringLiteral("Right panel"));
    splitter->addWidget(left);
    splitter->addWidget(right);
    splitter->setSizes({200, 200});

    layout->addWidget(splitter);
    return page;
}
}
