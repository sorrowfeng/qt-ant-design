#include "Pages.h"

#include <QHBoxLayout>
#include <QList>
#include <QPainterPath>
#include <QPair>
#include <QVBoxLayout>
#include <QWidget>

#include "PageCommon.h"
#include "core/AntTheme.h"
#include "core/AntTypes.h"
#include "widgets/AntButton.h"
#include "widgets/AntIcon.h"
#include "widgets/AntTypography.h"

namespace example::pages
{
QWidget* createButtonPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Types")));
    auto* typeRow = new QHBoxLayout();
    typeRow->setSpacing(12);
    const QList<QPair<QString, Ant::ButtonType>> types = {
        {QStringLiteral("Primary"), Ant::ButtonType::Primary},
        {QStringLiteral("Default"), Ant::ButtonType::Default},
        {QStringLiteral("Dashed"), Ant::ButtonType::Dashed},
        {QStringLiteral("Text"), Ant::ButtonType::Text},
        {QStringLiteral("Link"), Ant::ButtonType::Link},
    };
    for (const auto& item : types)
    {
        auto* button = new AntButton(item.first);
        button->setButtonType(item.second);
        typeRow->addWidget(button);
    }
    typeRow->addStretch();
    layout->addLayout(typeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Size and Shape")));
    auto* shapeRow = new QHBoxLayout();
    shapeRow->setSpacing(12);
    auto* large = new AntButton(QStringLiteral("Large"));
    large->setButtonType(Ant::ButtonType::Primary);
    large->setButtonSize(Ant::ButtonSize::Large);
    auto* middle = new AntButton(QStringLiteral("Middle"));
    middle->setButtonType(Ant::ButtonType::Primary);
    auto* small = new AntButton(QStringLiteral("Small"));
    small->setButtonType(Ant::ButtonType::Primary);
    small->setButtonSize(Ant::ButtonSize::Small);
    auto* round = new AntButton(QStringLiteral("Round"));
    round->setButtonShape(Ant::ButtonShape::Round);
    auto* circle = new AntButton(QStringLiteral("A"));
    circle->setButtonShape(Ant::ButtonShape::Circle);
    shapeRow->addWidget(large);
    shapeRow->addWidget(middle);
    shapeRow->addWidget(small);
    shapeRow->addWidget(round);
    shapeRow->addWidget(circle);
    shapeRow->addStretch();
    layout->addLayout(shapeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("States")));
    auto* stateRow = new QHBoxLayout();
    stateRow->setSpacing(12);
    auto* danger = new AntButton(QStringLiteral("Danger"));
    danger->setDanger(true);
    auto* ghost = new AntButton(QStringLiteral("Ghost"));
    ghost->setButtonType(Ant::ButtonType::Primary);
    ghost->setGhost(true);
    auto* loading = new AntButton(QStringLiteral("Loading"));
    loading->setButtonType(Ant::ButtonType::Primary);
    loading->setLoading(true);
    auto* disabled = new AntButton(QStringLiteral("Disabled"));
    disabled->setEnabled(false);
    stateRow->addWidget(danger);
    stateRow->addWidget(ghost);
    stateRow->addWidget(loading);
    stateRow->addWidget(disabled);
    stateRow->addStretch();
    layout->addLayout(stateRow);

    auto* block = new AntButton(QStringLiteral("Block Button"));
    block->setButtonType(Ant::ButtonType::Primary);
    block->setBlock(true);
    layout->addWidget(block);
    layout->addStretch();
    return page;
}

QWidget* createIconPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    auto createIconBlock = [](const QString& title, AntIcon* icon) {
        auto* block = new QWidget();
        auto* blockLayout = new QVBoxLayout(block);
        blockLayout->setContentsMargins(0, 0, 0, 0);
        blockLayout->setSpacing(8);
        blockLayout->addWidget(icon, 0, Qt::AlignHCenter);

        auto* label = new AntTypography(title, block);
        blockLayout->addWidget(label, 0, Qt::AlignHCenter);
        return block;
    };

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(24);
    const QList<QPair<QString, Ant::IconType>> basics = {
        {QStringLiteral("Search"), Ant::IconType::Search},
        {QStringLiteral("Home"), Ant::IconType::Home},
        {QStringLiteral("User"), Ant::IconType::User},
        {QStringLiteral("Calendar"), Ant::IconType::Calendar},
        {QStringLiteral("Clock"), Ant::IconType::ClockCircle},
        {QStringLiteral("Star"), Ant::IconType::Star},
    };
    for (const auto& item : basics)
    {
        auto* icon = new AntIcon(item.second);
        icon->setIconSize(24);
        basicRow->addWidget(createIconBlock(item.first, icon));
    }
    basicRow->addStretch();
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Themes and Colors")));
    auto* themeRow = new QHBoxLayout();
    themeRow->setSpacing(24);
    auto* outlined = new AntIcon(Ant::IconType::Star);
    outlined->setIconSize(28);
    outlined->setIconTheme(Ant::IconTheme::Outlined);
    themeRow->addWidget(createIconBlock(QStringLiteral("Outlined"), outlined));

    auto* filled = new AntIcon(Ant::IconType::Star);
    filled->setIconSize(28);
    filled->setIconTheme(Ant::IconTheme::Filled);
    filled->setColor(antTheme->tokens().colorWarning);
    themeRow->addWidget(createIconBlock(QStringLiteral("Filled"), filled));

    auto* twoTone = new AntIcon(Ant::IconType::InfoCircle);
    twoTone->setIconSize(28);
    twoTone->setIconTheme(Ant::IconTheme::TwoTone);
    twoTone->setTwoToneColor(antTheme->tokens().colorPrimary);
    themeRow->addWidget(createIconBlock(QStringLiteral("TwoTone"), twoTone));

    auto* error = new AntIcon(Ant::IconType::CloseCircle);
    error->setIconSize(28);
    error->setIconTheme(Ant::IconTheme::Filled);
    error->setColor(antTheme->tokens().colorError);
    themeRow->addWidget(createIconBlock(QStringLiteral("Status"), error));
    themeRow->addStretch();
    layout->addLayout(themeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Rotate and Spin")));
    auto* motionRow = new QHBoxLayout();
    motionRow->setSpacing(24);
    auto* loading = new AntIcon(Ant::IconType::Loading);
    loading->setIconSize(28);
    loading->setColor(antTheme->tokens().colorPrimary);
    loading->setSpin(true);
    motionRow->addWidget(createIconBlock(QStringLiteral("Loading"), loading));

    auto* rotate90 = new AntIcon(Ant::IconType::Right);
    rotate90->setIconSize(28);
    rotate90->setRotate(90);
    motionRow->addWidget(createIconBlock(QStringLiteral("Rotate 90"), rotate90));

    auto* rotate180 = new AntIcon(Ant::IconType::Down);
    rotate180->setIconSize(28);
    rotate180->setRotate(180);
    motionRow->addWidget(createIconBlock(QStringLiteral("Rotate 180"), rotate180));
    motionRow->addStretch();
    layout->addLayout(motionRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Custom Path")));
    auto* customRow = new QHBoxLayout();
    customRow->setSpacing(24);
    auto* customIcon = new AntIcon();
    customIcon->setIconSize(30);
    customIcon->setIconTheme(Ant::IconTheme::TwoTone);
    customIcon->setTwoToneColor(QColor(QStringLiteral("#eb2f96")));
    QPainterPath heart;
    heart.moveTo(16, 28);
    heart.cubicTo(7, 22, 3, 16, 3, 10.5);
    heart.cubicTo(3, 6.5, 6.2, 4, 10, 4);
    heart.cubicTo(12.8, 4, 15, 5.4, 16, 7.6);
    heart.cubicTo(17, 5.4, 19.2, 4, 22, 4);
    heart.cubicTo(25.8, 4, 29, 6.5, 29, 10.5);
    heart.cubicTo(29, 16, 25, 22, 16, 28);
    QPainterPath shine;
    shine.addEllipse(QRectF(10, 8, 4, 4));
    customIcon->setCustomPath(heart, shine);
    customRow->addWidget(createIconBlock(QStringLiteral("Custom"), customIcon));

    auto* inlineText = new AntTypography(QStringLiteral("Use icons inside rows for status, action and navigation."), page);
    auto* inlineWrap = new QWidget(page);
    auto* inlineLayout = new QHBoxLayout(inlineWrap);
    inlineLayout->setContentsMargins(0, 0, 0, 0);
    inlineLayout->setSpacing(8);
    auto* checkIcon = new AntIcon(Ant::IconType::CheckCircle);
    checkIcon->setIconTheme(Ant::IconTheme::Filled);
    checkIcon->setColor(antTheme->tokens().colorSuccess);
    checkIcon->setIconSize(18);
    inlineLayout->addWidget(checkIcon);
    inlineLayout->addWidget(inlineText);
    inlineLayout->addStretch();
    customRow->addWidget(inlineWrap, 1);
    layout->addLayout(customRow);

    layout->addStretch();
    return page;
}

QWidget* createTypographyPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Title Levels")));
    for (int i = 1; i <= 5; ++i)
    {
        auto* title = new AntTypography(QStringLiteral("Ant Design Title Level %1").arg(i));
        title->setTitle(true);
        title->setTitleLevel(static_cast<Ant::TypographyTitleLevel>(i - 1));
        layout->addWidget(title);
    }

    layout->addWidget(createSectionTitle(QStringLiteral("Text Types")));
    auto* typesRow = new QHBoxLayout();
    typesRow->setSpacing(16);
    auto* primary = new AntTypography(QStringLiteral("Primary Text"));
    auto* secondary = new AntTypography(QStringLiteral("Secondary Text"));
    secondary->setType(Ant::TypographyType::Secondary);
    auto* success = new AntTypography(QStringLiteral("Success Text"));
    success->setType(Ant::TypographyType::Success);
    auto* warning = new AntTypography(QStringLiteral("Warning Text"));
    warning->setType(Ant::TypographyType::Warning);
    auto* danger = new AntTypography(QStringLiteral("Danger Text"));
    danger->setType(Ant::TypographyType::Danger);
    auto* link = new AntTypography(QStringLiteral("Link Text"));
    link->setType(Ant::TypographyType::Link);
    link->setHref(QStringLiteral("https://ant.design"));
    typesRow->addWidget(primary);
    typesRow->addWidget(secondary);
    typesRow->addWidget(success);
    typesRow->addWidget(warning);
    typesRow->addWidget(danger);
    typesRow->addWidget(link);
    typesRow->addStretch();
    layout->addLayout(typesRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Decorations")));
    auto* strongText = new AntTypography(QStringLiteral("Strong Text"));
    strongText->setStrong(true);
    layout->addWidget(strongText);

    auto* underlineText = new AntTypography(QStringLiteral("Underline Text"));
    underlineText->setUnderline(true);
    layout->addWidget(underlineText);

    auto* deleteText = new AntTypography(QStringLiteral("Delete Text"));
    deleteText->setDelete(true);
    layout->addWidget(deleteText);

    auto* codeText = new AntTypography(QStringLiteral("Code Text"));
    codeText->setCode(true);
    layout->addWidget(codeText);

    auto* markText = new AntTypography(QStringLiteral("Mark Text"));
    markText->setMark(true);
    layout->addWidget(markText);

    auto* italicText = new AntTypography(QStringLiteral("Italic Text"));
    italicText->setItalic(true);
    layout->addWidget(italicText);

    layout->addWidget(createSectionTitle(QStringLiteral("Paragraph")));
    auto* paragraph = new AntTypography(
        QStringLiteral("Ant Design is a design system for enterprise-level products. "
                       "Create an efficient and enjoyable work experience with the design language."));
    paragraph->setParagraph(true);
    layout->addWidget(paragraph);

    layout->addWidget(createSectionTitle(QStringLiteral("Copyable")));
    auto* copyable = new AntTypography(QStringLiteral("This text is copyable. Click to copy!"));
    copyable->setCopyable(true);
    layout->addWidget(copyable);

    layout->addStretch();
    return page;
}
}
