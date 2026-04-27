#include "Pages.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

#include "PageCommon.h"
#include "core/AntTypes.h"
#include "widgets/AntAlert.h"
#include "widgets/AntButton.h"
#include "widgets/AntCard.h"
#include "widgets/AntDrawer.h"
#include "widgets/AntInput.h"
#include "widgets/AntMessage.h"
#include "widgets/AntModal.h"
#include "widgets/AntNotification.h"
#include "widgets/AntPopconfirm.h"
#include "widgets/AntProgress.h"
#include "widgets/AntResult.h"
#include "widgets/AntSelect.h"
#include "widgets/AntSkeleton.h"
#include "widgets/AntSpin.h"
#include "widgets/AntTour.h"
#include "widgets/AntTypography.h"
#include "widgets/AntWatermark.h"

namespace example::pages
{
QWidget* createProgressPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Line"));
        auto* cl = card->bodyLayout();
        const QList<int> values = {30, 50, 70, 100};
        for (int value : values)
        {
            auto* progress = new AntProgress();
            progress->setPercent(value);
            if (value == 100)
            {
                progress->setStatus(Ant::ProgressStatus::Success);
            }
            cl->addWidget(progress);
        }
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Status"));
        auto* cl = card->bodyLayout();
        auto* statusRow = new QHBoxLayout();
        statusRow->setSpacing(16);
        auto* active = new AntProgress();
        active->setPercent(55);
        active->setStatus(Ant::ProgressStatus::Active);
        auto* exception = new AntProgress();
        exception->setPercent(45);
        exception->setStatus(Ant::ProgressStatus::Exception);
        statusRow->addWidget(active);
        statusRow->addWidget(exception);
        cl->addLayout(statusRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Circle"));
        auto* cl = card->bodyLayout();
        auto* circleRow = new QHBoxLayout();
        circleRow->setSpacing(16);
        auto* circle = new AntProgress();
        circle->setProgressType(Ant::ProgressType::Circle);
        circle->setPercent(75);
        auto* circleSuccess = new AntProgress();
        circleSuccess->setProgressType(Ant::ProgressType::Circle);
        circleSuccess->setPercent(100);
        auto* dashboard = new AntProgress();
        dashboard->setProgressType(Ant::ProgressType::Dashboard);
        dashboard->setPercent(68);
        circleRow->addWidget(circle);
        circleRow->addWidget(circleSuccess);
        circleRow->addWidget(dashboard);
        circleRow->addStretch();
        cl->addLayout(circleRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createResultPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Status"));
        auto* cl = card->bodyLayout();
        auto* statusRow = new QHBoxLayout();
        statusRow->setSpacing(16);

        auto* success = new AntResult(page);
        success->setFixedWidth(260);
        success->setStatus(Ant::AlertType::Success);
        success->setTitle(QStringLiteral("Successfully Purchased"));
        success->setSubTitle("Order number: 2018122412345678. Your order has been placed and will be shipped within 24 hours.");
        auto* successBtn = new AntButton(QStringLiteral("View Orders"), success);
        successBtn->setButtonType(Ant::ButtonType::Primary);
        success->setExtraWidget(successBtn);

        auto* warning = new AntResult(page);
        warning->setFixedWidth(260);
        warning->setStatus(Ant::AlertType::Warning);
        warning->setTitle(QStringLiteral("Attention Required"));
        warning->setSubTitle("Your subscription will expire in 3 days. Please renew to avoid service interruption.");
        auto* warningBtn = new AntButton(QStringLiteral("Renew Now"), warning);
        warningBtn->setButtonType(Ant::ButtonType::Primary);
        warning->setExtraWidget(warningBtn);

        auto* error = new AntResult(page);
        error->setFixedWidth(260);
        error->setStatus(Ant::AlertType::Error);
        error->setTitle(QStringLiteral("Submission Failed"));
        error->setSubTitle("Please check the form fields and try again.");
        auto* errorBtn = new AntButton(QStringLiteral("Retry"), error);
        errorBtn->setButtonType(Ant::ButtonType::Primary);
        error->setExtraWidget(errorBtn);

        auto* info = new AntResult(page);
        info->setFixedWidth(260);
        info->setStatus(Ant::AlertType::Info);
        info->setTitle(QStringLiteral("Processing"));
        info->setSubTitle("Your request is being processed. Please wait a moment.");

        statusRow->addWidget(success);
        statusRow->addWidget(warning);
        statusRow->addWidget(error);
        statusRow->addWidget(info);
        statusRow->addStretch();
        cl->addLayout(statusRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Extra Actions"));
        auto* cl = card->bodyLayout();
        auto* actionRow = new QHBoxLayout();
        actionRow->setSpacing(16);

        auto* withActions = new AntResult(page);
        withActions->setFixedWidth(400);
        withActions->setStatus(Ant::AlertType::Success);
        withActions->setTitle(QStringLiteral("Payment Complete"));
        withActions->setSubTitle("Thank you for your purchase. A confirmation email has been sent to your inbox.");
        auto* btnRow = new QWidget(withActions);
        auto* btnLayout = new QHBoxLayout(btnRow);
        btnLayout->setContentsMargins(0, 0, 0, 0);
        btnLayout->setSpacing(12);
        auto* goHome = new AntButton(QStringLiteral("Go Home"), btnRow);
        goHome->setButtonType(Ant::ButtonType::Primary);
        auto* viewDetail = new AntButton(QStringLiteral("View Detail"), btnRow);
        btnLayout->addWidget(goHome);
        btnLayout->addWidget(viewDetail);
        withActions->setExtraWidget(btnRow);

        auto* noIcon = new AntResult(page);
        noIcon->setFixedWidth(400);
        noIcon->setIconVisible(false);
        noIcon->setTitle(QStringLiteral("Custom Content Area"));
        noIcon->setSubTitle("This result has no icon. You can place any content here.");
        auto* noIconBtn = new AntButton(QStringLiteral("Back"), noIcon);
        noIconBtn->setButtonType(Ant::ButtonType::Primary);
        noIcon->setExtraWidget(noIconBtn);

        actionRow->addWidget(withActions);
        actionRow->addWidget(noIcon);
        actionRow->addStretch();
        cl->addLayout(actionRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createSkeletonPage(QWidget* /*owner*/)
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

        auto* simple = new AntSkeleton(page);
        simple->setFixedWidth(280);
        simple->setParagraphRows(3);

        auto* active = new AntSkeleton(page);
        active->setFixedWidth(280);
        active->setActive(true);
        active->setTitleWidthRatio(0.56);
        active->setParagraphWidthRatios({1.0, 0.92, 0.48});

        auto* staticOne = new AntSkeleton(page);
        staticOne->setFixedWidth(280);
        staticOne->setActive(false);
        staticOne->setParagraphRows(2);
        staticOne->setParagraphWidthRatios({0.88, 0.54});

        basicRow->addWidget(simple);
        basicRow->addWidget(active);
        basicRow->addWidget(staticOne);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Avatar and Round"));
        auto* cl = card->bodyLayout();
        auto* avatarRow = new QHBoxLayout();
        avatarRow->setSpacing(16);

        auto* article = new AntSkeleton(page);
        article->setFixedWidth(420);
        article->setAvatarVisible(true);
        article->setAvatarShape(Ant::AvatarShape::Circle);
        article->setParagraphRows(3);
        article->setParagraphWidthRatios({0.98, 0.86, 0.58});

        auto* profile = new AntSkeleton(page);
        profile->setFixedWidth(420);
        profile->setAvatarVisible(true);
        profile->setAvatarShape(Ant::AvatarShape::Square);
        profile->setRound(true);
        profile->setTitleWidthRatio(0.38);
        profile->setParagraphRows(4);
        profile->setParagraphWidthRatios({1.0, 0.94, 0.84, 0.52});

        avatarRow->addWidget(article);
        avatarRow->addWidget(profile);
        avatarRow->addStretch();
        cl->addLayout(avatarRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Loading Switch"));
        auto* cl = card->bodyLayout();
        auto* switchRow = new QHBoxLayout();
        switchRow->setSpacing(16);

        auto* previewCard = new AntCard(QStringLiteral("Workspace Summary"), page);
        previewCard->setFixedWidth(520);
        previewCard->setExtra(QStringLiteral("Ready"));
        previewCard->bodyLayout()->addWidget(new AntTypography(QStringLiteral("3 reviewers assigned"), previewCard));
        previewCard->bodyLayout()->addWidget(new AntTypography(QStringLiteral("12 checklist items completed"), previewCard));
        previewCard->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Next milestone: beta publish"), previewCard));

        auto* wrapped = new AntSkeleton(page);
        wrapped->setFixedWidth(520);
        wrapped->setAvatarVisible(true);
        wrapped->setParagraphRows(4);
        wrapped->setParagraphWidthRatios({0.95, 0.82, 0.76, 0.44});
        wrapped->setContentWidget(previewCard);

        auto* toggle = new AntButton(QStringLiteral("Toggle Loading"));
        toggle->setButtonType(Ant::ButtonType::Primary);
        QObject::connect(toggle, &AntButton::clicked, wrapped, [wrapped]() {
            wrapped->setLoading(!wrapped->isLoading());
        });

        switchRow->addWidget(wrapped);
        switchRow->addWidget(toggle, 0, Qt::AlignTop);
        switchRow->addStretch();
        cl->addLayout(switchRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Element Variants"));
        auto* cl = card->bodyLayout();
        auto* elementRow = new QHBoxLayout();
        elementRow->setSpacing(16);

        auto* btnSkeleton = new AntSkeleton(page);
        btnSkeleton->setElement(Ant::SkeletonElement::Button);
        btnSkeleton->setFixedWidth(120);

        auto* avatarSkeleton = new AntSkeleton(page);
        avatarSkeleton->setElement(Ant::SkeletonElement::Avatar);

        auto* inputSkeleton = new AntSkeleton(page);
        inputSkeleton->setElement(Ant::SkeletonElement::Input);
        inputSkeleton->setFixedWidth(200);

        auto* imageSkeleton = new AntSkeleton(page);
        imageSkeleton->setElement(Ant::SkeletonElement::Image);
        imageSkeleton->setFixedSize(160, 160);

        elementRow->addWidget(btnSkeleton);
        elementRow->addWidget(avatarSkeleton);
        elementRow->addWidget(inputSkeleton);
        elementRow->addWidget(imageSkeleton);
        elementRow->addStretch();
        cl->addLayout(elementRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createSpinPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Size"));
        auto* cl = card->bodyLayout();
        auto* sizeRow = new QHBoxLayout();
        sizeRow->setSpacing(16);
        auto* small = new AntSpin();
        small->setSpinSize(Ant::Size::Small);
        auto* middle = new AntSpin();
        auto* large = new AntSpin();
        large->setSpinSize(Ant::Size::Large);
        sizeRow->addWidget(small);
        sizeRow->addWidget(middle);
        sizeRow->addWidget(large);
        sizeRow->addStretch();
        cl->addLayout(sizeRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Description"));
        auto* cl = card->bodyLayout();
        auto* descRow = new QHBoxLayout();
        descRow->setSpacing(16);
        auto* loading = new AntSpin();
        loading->setSpinSize(Ant::Size::Large);
        loading->setDescription(QStringLiteral("Loading"));
        auto* percent = new AntSpin();
        percent->setSpinSize(Ant::Size::Large);
        percent->setPercent(68);
        percent->setDescription(QStringLiteral("68%"));
        auto* hidden = new AntSpin();
        hidden->setSpinning(false);
        hidden->setDescription(QStringLiteral("Hidden"));
        descRow->addWidget(loading);
        descRow->addWidget(percent);
        descRow->addWidget(hidden);
        descRow->addStretch();
        cl->addLayout(descRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Embedded"));
        auto* cl = card->bodyLayout();
        auto* embeddedCard = new AntCard(QStringLiteral("Loading block"));
        embeddedCard->setMinimumHeight(160);
        auto* cardContent = new QVBoxLayout();
        auto* embedded = new AntSpin();
        embedded->setDescription(QStringLiteral("Fetching data"));
        cardContent->addStretch();
        cardContent->addWidget(embedded, 0, Qt::AlignCenter);
        cardContent->addStretch();
        embeddedCard->bodyLayout()->addLayout(cardContent);
        cl->addWidget(embeddedCard);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createTourPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntTour"));
        auto* cl = card->bodyLayout();

        auto* checklistCard = new AntCard(QStringLiteral("Release Checklist"), page);
        auto* bodyLayout = checklistCard->bodyLayout();
        auto* trigger = new AntButton(QStringLiteral("Start Tour"), checklistCard);
        trigger->setButtonType(Ant::ButtonType::Primary);
        auto* targetA = new AntInput(checklistCard);
        targetA->setPlaceholderText(QStringLiteral("Version"));
        auto* targetB = new AntSelect(checklistCard);
        targetB->addOption(QStringLiteral("Stable"));
        targetB->addOption(QStringLiteral("Beta"));
        auto* targetC = new AntButton(QStringLiteral("Publish"), checklistCard);
        bodyLayout->addWidget(trigger);
        bodyLayout->addWidget(targetA);
        bodyLayout->addWidget(targetB);
        bodyLayout->addWidget(targetC);
        cl->addWidget(checklistCard);

        auto* status = makeText(QStringLiteral("Tour not started"), page);
        cl->addWidget(status);

        auto* tour = new AntTour(page);
        tour->addStep({trigger, QStringLiteral("Start"), QStringLiteral("Launch the walkthrough from here.")});
        tour->addStep({targetA, QStringLiteral("Version"), QStringLiteral("Choose the version you want to publish.")});
        tour->addStep({targetB, QStringLiteral("Channel"), QStringLiteral("Pick a release channel before publishing.")});
        tour->addStep({targetC, QStringLiteral("Publish"), QStringLiteral("Finish by confirming the action.")});

        QObject::connect(trigger, &AntButton::clicked, page, [tour]() { tour->start(); });
        QObject::connect(tour, &AntTour::stepChanged, status, [status](int index) {
            status->setText(QStringLiteral("Tour step: %1").arg(index + 1));
        });
        QObject::connect(tour, &AntTour::finished, status, [status]() {
            status->setText(QStringLiteral("Tour finished"));
        });

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createWatermarkPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic Text Watermark"));
        auto* cl = card->bodyLayout();
        auto* wm1 = new AntWatermark();
        wm1->setContent(QStringLiteral("Ant Design Confidential"));
        wm1->setFixedHeight(160);
        auto* wm1Layout = new QVBoxLayout(wm1);
        wm1Layout->setContentsMargins(16, 16, 16, 16);
        auto* info = new AntTypography(QStringLiteral("This area is watermarked with repeated text."));
        info->setType(Ant::TypographyType::Secondary);
        wm1Layout->addWidget(info);
        cl->addWidget(wm1);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Multi-line Watermark"));
        auto* cl = card->bodyLayout();
        auto* wm2 = new AntWatermark();
        wm2->setContent(QStringList{QStringLiteral("Ant Design"), QStringLiteral("Internal Use Only")});
        wm2->setFixedHeight(160);
        auto* wm2Layout = new QVBoxLayout(wm2);
        wm2Layout->setContentsMargins(16, 16, 16, 16);
        auto* info2 = new AntTypography(QStringLiteral("Multi-line watermark with custom text."));
        info2->setType(Ant::TypographyType::Secondary);
        wm2Layout->addWidget(info2);
        cl->addWidget(wm2);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Custom Rotation"));
        auto* cl = card->bodyLayout();
        auto* wm3 = new AntWatermark();
        wm3->setContent(QStringLiteral("ROTATED"));
        wm3->setRotate(-45);
        wm3->setFixedHeight(160);
        auto* wm3Layout = new QVBoxLayout(wm3);
        wm3Layout->setContentsMargins(16, 16, 16, 16);
        wm3Layout->addWidget(new AntTypography(QStringLiteral("This watermark is rotated -45 degrees.")));
        cl->addWidget(wm3);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
