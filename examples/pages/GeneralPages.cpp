#include "Pages.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QList>
#include <QPainterPath>
#include <QPair>
#include <QVBoxLayout>
#include <QWidget>

#include "core/AntTheme.h"
#include "widgets/AntCard.h"
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
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    // Types — Primary / Default / Dashed / Text / Link
    {
        auto* card = new AntCard(QStringLiteral("Button Types"));
        auto* cl = card->bodyLayout();
        auto* row = new QHBoxLayout();
        row->setSpacing(12);
        auto* primary = new AntButton(QStringLiteral("Primary"));
        primary->setButtonType(Ant::ButtonType::Primary);
        auto* def = new AntButton(QStringLiteral("Default"));
        def->setButtonType(Ant::ButtonType::Default);
        auto* dashed = new AntButton(QStringLiteral("Dashed"));
        dashed->setButtonType(Ant::ButtonType::Dashed);
        auto* text = new AntButton(QStringLiteral("Text"));
        text->setButtonType(Ant::ButtonType::Text);
        auto* link = new AntButton(QStringLiteral("Link"));
        link->setButtonType(Ant::ButtonType::Link);
        row->addWidget(primary);
        row->addWidget(def);
        row->addWidget(dashed);
        row->addWidget(text);
        row->addWidget(link);
        row->addStretch();
        cl->addLayout(row);
        layout->addWidget(card);
    }

    // Danger & Disabled
    {
        auto* card = new AntCard(QStringLiteral("Danger & Disabled"));
        auto* cl = card->bodyLayout();
        auto* row = new QHBoxLayout();
        row->setSpacing(12);
        auto* primaryDanger = new AntButton(QStringLiteral("Primary Danger"));
        primaryDanger->setButtonType(Ant::ButtonType::Primary);
        primaryDanger->setDanger(true);
        auto* defaultDanger = new AntButton(QStringLiteral("Default Danger"));
        defaultDanger->setButtonType(Ant::ButtonType::Default);
        defaultDanger->setDanger(true);
        auto* disabled = new AntButton(QStringLiteral("Disabled"));
        disabled->setButtonType(Ant::ButtonType::Primary);
        disabled->setEnabled(false);
        row->addWidget(primaryDanger);
        row->addWidget(defaultDanger);
        row->addWidget(disabled);
        row->addStretch();
        cl->addLayout(row);
        layout->addWidget(card);
    }

    // Sizes — Large / Middle / Small
    {
        auto* card = new AntCard(QStringLiteral("Sizes"));
        auto* cl = card->bodyLayout();
        auto* row = new QHBoxLayout();
        row->setSpacing(12);
        auto* large = new AntButton(QStringLiteral("Large"));
        large->setButtonType(Ant::ButtonType::Primary);
        large->setButtonSize(Ant::Size::Large);
        auto* middle = new AntButton(QStringLiteral("Middle"));
        middle->setButtonType(Ant::ButtonType::Primary);
        auto* small = new AntButton(QStringLiteral("Small"));
        small->setButtonType(Ant::ButtonType::Primary);
        small->setButtonSize(Ant::Size::Small);
        row->addWidget(large);
        row->addWidget(middle);
        row->addWidget(small);
        row->addStretch();
        cl->addLayout(row);
        layout->addWidget(card);
    }

    // Shapes — Round / Circle / Ghost / Loading
    {
        auto* card = new AntCard(QStringLiteral("Shapes"));
        auto* cl = card->bodyLayout();
        auto* row = new QHBoxLayout();
        row->setSpacing(12);
        auto* round = new AntButton(QStringLiteral("Round"));
        round->setButtonType(Ant::ButtonType::Primary);
        round->setButtonShape(Ant::ButtonShape::Round);
        auto* circle = new AntButton();
        circle->setButtonType(Ant::ButtonType::Primary);
        circle->setButtonShape(Ant::ButtonShape::Circle);
        circle->setButtonIconType(Ant::IconType::Search);
        auto* ghost = new AntButton(QStringLiteral("Ghost"));
        ghost->setButtonType(Ant::ButtonType::Primary);
        ghost->setGhost(true);
        auto* loading = new AntButton(QStringLiteral("Loading"));
        loading->setButtonType(Ant::ButtonType::Primary);
        loading->setLoading(true);
        row->addWidget(round);
        row->addWidget(circle);
        row->addWidget(ghost);
        row->addWidget(loading);
        row->addStretch();
        cl->addLayout(row);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createIconPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    auto createIconBlock = [](const QString& title, AntIcon* icon) {
        auto* block = new QWidget();
        block->setFixedWidth(72);
        auto* blockLayout = new QVBoxLayout(block);
        blockLayout->setContentsMargins(0, 0, 0, 0);
        blockLayout->setSpacing(4);
        blockLayout->addWidget(icon, 0, Qt::AlignHCenter);
        auto* label = new AntTypography(title, block);
        label->setType(Ant::TypographyType::Secondary);
        blockLayout->addWidget(label, 0, Qt::AlignHCenter);
        return block;
    };

    {
        auto* card = new AntCard(QStringLiteral("Outlined Icons"));
        auto* cl = card->bodyLayout();
        auto* grid = new QGridLayout();
        grid->setContentsMargins(0, 0, 0, 0);
        grid->setHorizontalSpacing(20);
        grid->setVerticalSpacing(16);
        const QList<QPair<QString, Ant::IconType>> basics = {
            {QStringLiteral("Home"), Ant::IconType::Home},
            {QStringLiteral("User"), Ant::IconType::User},
            {QStringLiteral("Search"), Ant::IconType::Search},
            {QStringLiteral("Setting"), Ant::IconType::Setting},
            {QStringLiteral("Star"), Ant::IconType::Star},
            {QStringLiteral("Heart"), Ant::IconType::Heart},
            {QStringLiteral("Bell"), Ant::IconType::Bell},
            {QStringLiteral("Mail"), Ant::IconType::Mail},
            {QStringLiteral("Calendar"), Ant::IconType::Calendar},
            {QStringLiteral("Clock"), Ant::IconType::ClockCircle},
            {QStringLiteral("Check"), Ant::IconType::Check},
            {QStringLiteral("Close"), Ant::IconType::Close},
            {QStringLiteral("Plus"), Ant::IconType::Plus},
            {QStringLiteral("Edit"), Ant::IconType::Edit},
            {QStringLiteral("Delete"), Ant::IconType::Delete},
            {QStringLiteral("Upload"), Ant::IconType::CloudUpload},
        };
        for (int i = 0; i < basics.size(); ++i)
        {
            const auto& item = basics.at(i);
            auto* icon = new AntIcon(item.second);
            icon->setIconSize(22);
            grid->addWidget(createIconBlock(item.first, icon), i / 10, i % 10);
        }
        cl->addLayout(grid);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Themes and Colors"));
        auto* cl = card->bodyLayout();
        auto* row = new QHBoxLayout();
        row->setSpacing(24);
        auto* outlined = new AntIcon(Ant::IconType::Star);
        outlined->setIconSize(28);
        outlined->setIconTheme(Ant::IconTheme::Outlined);
        row->addWidget(createIconBlock(QStringLiteral("Outlined"), outlined));

        auto* filled = new AntIcon(Ant::IconType::Star);
        filled->setIconSize(28);
        filled->setIconTheme(Ant::IconTheme::Filled);
        filled->setColor(antTheme->tokens().colorWarning);
        row->addWidget(createIconBlock(QStringLiteral("Filled"), filled));

        auto* twoTone = new AntIcon(Ant::IconType::InfoCircle);
        twoTone->setIconSize(28);
        twoTone->setIconTheme(Ant::IconTheme::TwoTone);
        twoTone->setTwoToneColor(antTheme->tokens().colorPrimary);
        row->addWidget(createIconBlock(QStringLiteral("TwoTone"), twoTone));

        auto* error = new AntIcon(Ant::IconType::CloseCircle);
        error->setIconSize(28);
        error->setIconTheme(Ant::IconTheme::Filled);
        error->setColor(antTheme->tokens().colorError);
        row->addWidget(createIconBlock(QStringLiteral("Status"), error));
        row->addStretch();
        cl->addLayout(row);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Rotate and Spin"));
        auto* cl = card->bodyLayout();
        auto* row = new QHBoxLayout();
        row->setSpacing(24);
        auto* loading = new AntIcon(Ant::IconType::Loading);
        loading->setIconSize(28);
        loading->setColor(antTheme->tokens().colorPrimary);
        loading->setSpin(true);
        row->addWidget(createIconBlock(QStringLiteral("Loading"), loading));

        auto* rotate90 = new AntIcon(Ant::IconType::Right);
        rotate90->setIconSize(28);
        rotate90->setRotate(90);
        row->addWidget(createIconBlock(QStringLiteral("Rotate 90"), rotate90));

        auto* rotate180 = new AntIcon(Ant::IconType::Down);
        rotate180->setIconSize(28);
        rotate180->setRotate(180);
        row->addWidget(createIconBlock(QStringLiteral("Rotate 180"), rotate180));
        row->addStretch();
        cl->addLayout(row);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Custom Path"));
        auto* cl = card->bodyLayout();
        auto* row = new QHBoxLayout();
        row->setSpacing(24);
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
        row->addWidget(createIconBlock(QStringLiteral("Custom"), customIcon));

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
        row->addWidget(inlineWrap, 1);
        cl->addLayout(row);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createTypographyPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Title Levels"));
        auto* cl = card->bodyLayout();
        for (int i = 1; i <= 5; ++i)
        {
            auto* title = new AntTypography(QStringLiteral("Ant Design Title Level %1").arg(i));
            title->setTitle(true);
            title->setTitleLevel(static_cast<Ant::TypographyTitleLevel>(i - 1));
            cl->addWidget(title);
        }
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Text Types"));
        auto* cl = card->bodyLayout();
        auto* row = new QHBoxLayout();
        row->setSpacing(16);
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
        row->addWidget(primary);
        row->addWidget(secondary);
        row->addWidget(success);
        row->addWidget(warning);
        row->addWidget(danger);
        row->addWidget(link);
        row->addStretch();
        cl->addLayout(row);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Decorations"));
        auto* cl = card->bodyLayout();
        auto* strongText = new AntTypography(QStringLiteral("Strong Text"));
        strongText->setStrong(true);
        cl->addWidget(strongText);
        auto* underlineText = new AntTypography(QStringLiteral("Underline Text"));
        underlineText->setUnderline(true);
        cl->addWidget(underlineText);
        auto* deleteText = new AntTypography(QStringLiteral("Delete Text"));
        deleteText->setDelete(true);
        cl->addWidget(deleteText);
        auto* codeText = new AntTypography(QStringLiteral("Code Text"));
        codeText->setCode(true);
        cl->addWidget(codeText);
        auto* markText = new AntTypography(QStringLiteral("Mark Text"));
        markText->setMark(true);
        cl->addWidget(markText);
        auto* italicText = new AntTypography(QStringLiteral("Italic Text"));
        italicText->setItalic(true);
        cl->addWidget(italicText);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Paragraph"));
        auto* cl = card->bodyLayout();
        auto* paragraph = new AntTypography(
            QStringLiteral("Ant Design is a design system for enterprise-level products. "
                           "Create an efficient and enjoyable work experience with the design language."));
        paragraph->setParagraph(true);
        cl->addWidget(paragraph);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Copyable"));
        auto* cl = card->bodyLayout();
        auto* copyable = new AntTypography(QStringLiteral("This text is copyable. Click to copy!"));
        copyable->setCopyable(true);
        cl->addWidget(copyable);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
