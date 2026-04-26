#include "Pages.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

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
QWidget* createAlertPage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* info = new AntAlert(QStringLiteral("Informational Notes"));
        info->setAlertType(Ant::AlertType::Info);
        auto* success = new AntAlert(QStringLiteral("Success Tips"));
        success->setAlertType(Ant::AlertType::Success);
        auto* warning = new AntAlert(QStringLiteral("Warning"));
        warning->setAlertType(Ant::AlertType::Warning);
        auto* error = new AntAlert(QStringLiteral("Error"));
        error->setAlertType(Ant::AlertType::Error);
        cl->addWidget(info);
        cl->addWidget(success);
        cl->addWidget(warning);
        cl->addWidget(error);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Description and Icon"));
        auto* cl = card->bodyLayout();
        auto* described = new AntAlert(QStringLiteral("Data sync in progress"));
        described->setDescription(QStringLiteral("Background synchronization is running. You can keep working while we update the latest records from the server."));
        described->setAlertType(Ant::AlertType::Info);
        described->setShowIcon(true);
        cl->addWidget(described);
        auto* withDescription = new AntAlert(QStringLiteral("Deployment failed"));
        withDescription->setDescription(QStringLiteral("The production deployment stopped because environment variables are incomplete. Please review the release configuration and try again."));
        withDescription->setAlertType(Ant::AlertType::Error);
        withDescription->setShowIcon(true);
        cl->addWidget(withDescription);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Closable and Action"));
        auto* cl = card->bodyLayout();
        auto* closable = new AntAlert(QStringLiteral("Update available"));
        closable->setDescription(QStringLiteral("A new desktop package is ready. You can update now or postpone until after your current task."));
        closable->setAlertType(Ant::AlertType::Warning);
        closable->setShowIcon(true);
        closable->setClosable(true);
        QObject::connect(closable, &AntAlert::closeRequested, owner, [owner]() {
            AntMessage::info(QStringLiteral("Alert closed"), owner, 1500);
        });
        cl->addWidget(closable);
        auto* actionAlert = new AntAlert(QStringLiteral("Backup completed"));
        actionAlert->setDescription(QStringLiteral("Cloud backup finished successfully. Open the latest snapshot or continue editing."));
        actionAlert->setAlertType(Ant::AlertType::Success);
        actionAlert->setShowIcon(true);
        auto* actionButton = new AntButton(QStringLiteral("Open"));
        actionButton->setButtonType(Ant::ButtonType::Link);
        QObject::connect(actionButton, &AntButton::clicked, owner, [owner]() {
            AntMessage::success(QStringLiteral("Opening backup snapshot"), owner, 1500);
        });
        actionAlert->setActionWidget(actionButton);
        cl->addWidget(actionAlert);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Banner"));
        auto* cl = card->bodyLayout();
        auto* banner = new AntAlert(QStringLiteral("Scheduled maintenance tonight from 01:00 to 03:00."));
        banner->setBanner(true);
        banner->setClosable(true);
        cl->addWidget(banner);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createDrawerPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntDrawer - Sliding Panel"));
        auto* cl = card->bodyLayout();

        auto* desc = new AntTypography(QStringLiteral("AntDrawer is a sliding panel that overlays from the edge of the screen. "
                                                      "Supports Left/Right/Top/Bottom placement with animation."));
        desc->setParagraph(true);
        cl->addWidget(desc);

        auto* btnLayout = new QHBoxLayout();
        btnLayout->setSpacing(8);

        auto* rightBtn = new AntButton(QStringLiteral("Open Right Drawer"));
        rightBtn->setButtonType(Ant::ButtonType::Primary);
        auto* drawer = new AntDrawer(page);
        drawer->setTitle(QStringLiteral("Drawer Title"));
        drawer->setPlacement(Ant::DrawerPlacement::Right);
        QObject::connect(rightBtn, &AntButton::clicked, drawer, &AntDrawer::open);
        btnLayout->addWidget(rightBtn);

        auto* leftBtn = new AntButton(QStringLiteral("Open Left Drawer"));
        leftBtn->setButtonType(Ant::ButtonType::Default);
        auto* leftDrawer = new AntDrawer(page);
        leftDrawer->setTitle(QStringLiteral("Left Drawer"));
        leftDrawer->setPlacement(Ant::DrawerPlacement::Left);
        QObject::connect(leftBtn, &AntButton::clicked, leftDrawer, &AntDrawer::open);
        btnLayout->addWidget(leftBtn);

        auto* bottomBtn = new AntButton(QStringLiteral("Open Bottom Drawer"));
        bottomBtn->setButtonType(Ant::ButtonType::Default);
        auto* bottomDrawer = new AntDrawer(page);
        bottomDrawer->setTitle(QStringLiteral("Bottom Drawer"));
        bottomDrawer->setPlacement(Ant::DrawerPlacement::Bottom);
        bottomDrawer->setDrawerHeight(250);
        QObject::connect(bottomBtn, &AntButton::clicked, bottomDrawer, &AntDrawer::open);
        btnLayout->addWidget(bottomBtn);

        cl->addLayout(btnLayout);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createMessagePage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Types"));
        auto* cl = card->bodyLayout();
        auto* typeRow = new QHBoxLayout();
        typeRow->setSpacing(12);
        auto* info = new AntButton(QStringLiteral("Info"));
        auto* success = new AntButton(QStringLiteral("Success"));
        auto* warning = new AntButton(QStringLiteral("Warning"));
        auto* error = new AntButton(QStringLiteral("Error"));
        auto* loading = new AntButton(QStringLiteral("Loading"));
        QObject::connect(info, &AntButton::clicked, owner, [owner]() { AntMessage::info(QStringLiteral("This is an info message"), owner); });
        QObject::connect(success, &AntButton::clicked, owner, [owner]() { AntMessage::success(QStringLiteral("Saved successfully"), owner); });
        QObject::connect(warning, &AntButton::clicked, owner, [owner]() { AntMessage::warning(QStringLiteral("Please check the warning"), owner); });
        QObject::connect(error, &AntButton::clicked, owner, [owner]() { AntMessage::error(QStringLiteral("Something went wrong"), owner); });
        QObject::connect(loading, &AntButton::clicked, owner, [owner]() { AntMessage::loading(QStringLiteral("Loading data..."), owner, 2500); });
        typeRow->addWidget(info);
        typeRow->addWidget(success);
        typeRow->addWidget(warning);
        typeRow->addWidget(error);
        typeRow->addWidget(loading);
        typeRow->addStretch();
        cl->addLayout(typeRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Duration"));
        auto* cl = card->bodyLayout();
        auto* durationRow = new QHBoxLayout();
        durationRow->setSpacing(12);
        auto* shortMsg = new AntButton(QStringLiteral("1 second"));
        auto* stickyMsg = new AntButton(QStringLiteral("Manual close"));
        QObject::connect(shortMsg, &AntButton::clicked, owner, [owner]() { AntMessage::success(QStringLiteral("This closes quickly"), owner, 1000); });
        QObject::connect(stickyMsg, &AntButton::clicked, owner, [owner]() { AntMessage::info(QStringLiteral("Click this message to close it"), owner, 0); });
        durationRow->addWidget(shortMsg);
        durationRow->addWidget(stickyMsg);
        durationRow->addStretch();
        cl->addLayout(durationRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Placement"));
        auto* cl = card->bodyLayout();
        auto* placementRow = new QHBoxLayout();
        placementRow->setSpacing(12);
        const QList<QPair<QString, Ant::Placement>> placements = {
            {QStringLiteral("Top"), Ant::Placement::Top},
            {QStringLiteral("TopLeft"), Ant::Placement::TopLeft},
            {QStringLiteral("TopRight"), Ant::Placement::TopRight},
            {QStringLiteral("Bottom"), Ant::Placement::Bottom},
            {QStringLiteral("BottomLeft"), Ant::Placement::BottomLeft},
            {QStringLiteral("BottomRight"), Ant::Placement::BottomRight},
        };
        for (const auto& item : placements)
        {
            auto* btn = new AntButton(item.first);
            QObject::connect(btn, &AntButton::clicked, owner, [owner, item]() {
                AntMessage::info(QStringLiteral("Message at %1").arg(item.first), owner, 2000, item.second);
            });
            placementRow->addWidget(btn);
        }
        placementRow->addStretch();
        cl->addLayout(placementRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createModalPage(QWidget* owner)
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

        auto* openBasic = new AntButton(QStringLiteral("Open Basic Modal"));
        openBasic->setButtonType(Ant::ButtonType::Primary);
        auto* basicModal = new AntModal(owner);
        basicModal->setTitle(QStringLiteral("Delete current draft?"));
        basicModal->setContent(QStringLiteral("This action will remove the local draft and clear the pending review notes."));
        basicModal->setOkText(QStringLiteral("Delete"));
        QObject::connect(openBasic, &AntButton::clicked, owner, [basicModal]() { basicModal->setOpen(true); });
        QObject::connect(basicModal, &AntModal::confirmed, owner, [owner]() {
            AntMessage::success(QStringLiteral("Draft deleted"), owner, 1500);
        });
        QObject::connect(basicModal, &AntModal::canceled, owner, [owner]() {
            AntMessage::info(QStringLiteral("Deletion canceled"), owner, 1500);
        });

        auto* openNotice = new AntButton(QStringLiteral("Top Offset"));
        auto* noticeModal = new AntModal(owner);
        noticeModal->setTitle(QStringLiteral("Publish release notes"));
        noticeModal->setContent(QStringLiteral("This modal uses a non-centered layout, which is handy for workflows that need more context below."));
        noticeModal->setCentered(false);
        noticeModal->setOkText(QStringLiteral("Publish"));
        QObject::connect(openNotice, &AntButton::clicked, owner, [noticeModal]() { noticeModal->setOpen(true); });

        basicRow->addWidget(openBasic);
        basicRow->addWidget(openNotice);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Custom Content"));
        auto* cl = card->bodyLayout();
        auto* customRow = new QHBoxLayout();
        customRow->setSpacing(16);

        auto* inviteButton = new AntButton(QStringLiteral("Invite Teammate"));
        auto* inviteModal = new AntModal(owner);
        inviteModal->setTitle(QStringLiteral("Invite a teammate"));
        inviteModal->setOkText(QStringLiteral("Send Invite"));
        inviteModal->setDialogWidth(560);

        auto* inviteContent = new QWidget(inviteModal);
        auto* inviteLayout = new QVBoxLayout(inviteContent);
        inviteLayout->setContentsMargins(0, 0, 0, 0);
        inviteLayout->setSpacing(12);
        auto* inviteHint = new AntTypography(QStringLiteral("Share access with someone who can help review and ship this change."), inviteContent);
        inviteHint->setParagraph(true);
        auto* nameInput = new AntInput(inviteContent);
        nameInput->setPlaceholderText(QStringLiteral("Name"));
        auto* mailInput = new AntInput(inviteContent);
        mailInput->setPlaceholderText(QStringLiteral("Email"));
        inviteLayout->addWidget(inviteHint);
        inviteLayout->addWidget(nameInput);
        inviteLayout->addWidget(mailInput);
        inviteModal->setContentWidget(inviteContent);
        QObject::connect(inviteButton, &AntButton::clicked, owner, [inviteModal]() { inviteModal->setOpen(true); });
        QObject::connect(inviteModal, &AntModal::confirmed, owner, [owner, nameInput]() {
            const QString name = nameInput->text().trimmed().isEmpty() ? QStringLiteral("teammate") : nameInput->text().trimmed();
            AntNotification::success(QStringLiteral("Invitation queued"),
                                     QStringLiteral("Invite has been prepared for %1.").arg(name),
                                     owner,
                                     2200);
        });

        auto* customFooterButton = new AntButton(QStringLiteral("Custom Footer"));
        auto* customFooterModal = new AntModal(owner);
        customFooterModal->setTitle(QStringLiteral("Upgrade storage plan"));
        customFooterModal->setContent(QStringLiteral("Your current workspace is close to its attachment limit. Choose how you'd like to continue."));
        customFooterModal->setDialogWidth(560);
        auto* footer = new QWidget(customFooterModal);
        auto* footerLayout = new QHBoxLayout(footer);
        footerLayout->setContentsMargins(0, 0, 0, 0);
        footerLayout->setSpacing(12);
        auto* remindLater = new AntButton(QStringLiteral("Remind Me Later"), footer);
        auto* upgradeNow = new AntButton(QStringLiteral("Upgrade Now"), footer);
        upgradeNow->setButtonType(Ant::ButtonType::Primary);
        footerLayout->addStretch();
        footerLayout->addWidget(remindLater);
        footerLayout->addWidget(upgradeNow);
        customFooterModal->setFooterWidget(footer);
        QObject::connect(remindLater, &AntButton::clicked, owner, [owner, customFooterModal]() {
            customFooterModal->setOpen(false);
            AntMessage::info(QStringLiteral("We will remind you next week"), owner, 1600);
        });
        QObject::connect(upgradeNow, &AntButton::clicked, owner, [owner, customFooterModal]() {
            customFooterModal->setOpen(false);
            AntMessage::success(QStringLiteral("Upgrade flow started"), owner, 1600);
        });
        QObject::connect(customFooterButton, &AntButton::clicked, owner, [customFooterModal]() { customFooterModal->setOpen(true); });

        customRow->addWidget(inviteButton);
        customRow->addWidget(customFooterButton);
        customRow->addStretch();
        cl->addLayout(customRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Behavior"));
        auto* cl = card->bodyLayout();
        auto* behaviorRow = new QHBoxLayout();
        behaviorRow->setSpacing(16);

        auto* protectedButton = new AntButton(QStringLiteral("Protected Action"));
        protectedButton->setDanger(true);
        auto* protectedModal = new AntModal(owner);
        protectedModal->setTitle(QStringLiteral("Stop background deployment?"));
        protectedModal->setContent(QStringLiteral("Mask click and close icon are disabled here, so users must make an explicit decision."));
        protectedModal->setMaskClosable(false);
        protectedModal->setClosable(false);
        protectedModal->setOkText(QStringLiteral("Stop Now"));
        protectedModal->setCancelText(QStringLiteral("Keep Running"));
        QObject::connect(protectedButton, &AntButton::clicked, owner, [protectedModal]() { protectedModal->setOpen(true); });
        QObject::connect(protectedModal, &AntModal::confirmed, owner, [owner]() {
            AntNotification::warning(QStringLiteral("Deployment stopped"),
                                     QStringLiteral("The running deployment has been interrupted."),
                                     owner,
                                     2200);
        });

        auto* soloButton = new AntButton(QStringLiteral("Single Action"));
        auto* soloModal = new AntModal(owner);
        soloModal->setTitle(QStringLiteral("Session will expire soon"));
        soloModal->setContent(QStringLiteral("There has been no activity for a while. Continue now to keep the workspace active."));
        soloModal->setShowCancel(false);
        soloModal->setOkText(QStringLiteral("Continue Session"));
        QObject::connect(soloButton, &AntButton::clicked, owner, [soloModal]() { soloModal->setOpen(true); });

        behaviorRow->addWidget(protectedButton);
        behaviorRow->addWidget(soloButton);
        behaviorRow->addStretch();
        cl->addLayout(behaviorRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Command API"));
        auto* cl = card->bodyLayout();
        auto* cmdRow = new QHBoxLayout();
        cmdRow->setSpacing(16);
        auto* infoBtn = new AntButton(QStringLiteral("Info"));
        QObject::connect(infoBtn, &AntButton::clicked, owner, [owner]() {
            AntModal::info(QStringLiteral("Information"), QStringLiteral("This is an informational modal dialog."), owner);
        });
        auto* successBtn = new AntButton(QStringLiteral("Success"));
        successBtn->setButtonType(Ant::ButtonType::Primary);
        QObject::connect(successBtn, &AntButton::clicked, owner, [owner]() {
            AntModal::success(QStringLiteral("Success"), QStringLiteral("The operation completed successfully."), owner);
        });
        auto* warningBtn = new AntButton(QStringLiteral("Warning"));
        QObject::connect(warningBtn, &AntButton::clicked, owner, [owner]() {
            AntModal::warning(QStringLiteral("Warning"), QStringLiteral("Please review before proceeding."), owner);
        });
        auto* errorBtn = new AntButton(QStringLiteral("Error"));
        errorBtn->setDanger(true);
        QObject::connect(errorBtn, &AntButton::clicked, owner, [owner]() {
            AntModal::error(QStringLiteral("Error"), QStringLiteral("An unexpected error occurred."), owner);
        });
        auto* confirmBtn = new AntButton(QStringLiteral("Confirm"));
        QObject::connect(confirmBtn, &AntButton::clicked, owner, [owner]() {
            AntModal::confirm(QStringLiteral("Confirm"), QStringLiteral("Are you sure you want to delete this item?"), owner);
        });
        cmdRow->addWidget(infoBtn);
        cmdRow->addWidget(successBtn);
        cmdRow->addWidget(warningBtn);
        cmdRow->addWidget(errorBtn);
        cmdRow->addWidget(confirmBtn);
        cmdRow->addStretch();
        cl->addLayout(cmdRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createNotificationPage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Types"));
        auto* cl = card->bodyLayout();
        auto* typeRow = new QHBoxLayout();
        typeRow->setSpacing(12);
        auto* info = new AntButton(QStringLiteral("Info"));
        auto* success = new AntButton(QStringLiteral("Success"));
        auto* warning = new AntButton(QStringLiteral("Warning"));
        auto* error = new AntButton(QStringLiteral("Error"));
        QObject::connect(info, &AntButton::clicked, owner, [owner]() {
            AntNotification::info(QStringLiteral("Notification"),
                                  QStringLiteral("This is an information notification with a longer description."),
                                  owner);
        });
        QObject::connect(success, &AntButton::clicked, owner, [owner]() {
            AntNotification::success(QStringLiteral("Success"),
                                     QStringLiteral("The operation completed and the result is ready."),
                                     owner);
        });
        QObject::connect(warning, &AntButton::clicked, owner, [owner]() {
            AntNotification::warning(QStringLiteral("Warning"),
                                     QStringLiteral("Please review the pending configuration before continuing."),
                                     owner);
        });
        QObject::connect(error, &AntButton::clicked, owner, [owner]() {
            AntNotification::error(QStringLiteral("Error"),
                                   QStringLiteral("The request failed. Check the connection and try again."),
                                   owner);
        });
        typeRow->addWidget(info);
        typeRow->addWidget(success);
        typeRow->addWidget(warning);
        typeRow->addWidget(error);
        typeRow->addStretch();
        cl->addLayout(typeRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Placement"));
        auto* cl = card->bodyLayout();
        auto* placementRow = new QHBoxLayout();
        placementRow->setSpacing(12);
        const QList<QPair<QString, Ant::Placement>> placements = {
            {QStringLiteral("Top left"), Ant::Placement::TopLeft},
            {QStringLiteral("Top"), Ant::Placement::Top},
            {QStringLiteral("Top right"), Ant::Placement::TopRight},
            {QStringLiteral("Bottom left"), Ant::Placement::BottomLeft},
            {QStringLiteral("Bottom"), Ant::Placement::Bottom},
            {QStringLiteral("Bottom right"), Ant::Placement::BottomRight},
        };
        for (const auto& item : placements)
        {
            auto* button = new AntButton(item.first);
            QObject::connect(button, &AntButton::clicked, owner, [owner, item]() {
                AntNotification::info(item.first,
                                      QStringLiteral("Placement follows the Ant Design notification API."),
                                      owner,
                                      3000,
                                      item.second);
            });
            placementRow->addWidget(button);
        }
        placementRow->addStretch();
        cl->addLayout(placementRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Duration and Progress"));
        auto* cl = card->bodyLayout();
        auto* durationRow = new QHBoxLayout();
        durationRow->setSpacing(12);
        auto* progress = new AntButton(QStringLiteral("Progress"));
        auto* sticky = new AntButton(QStringLiteral("Manual close"));
        auto* closeAll = new AntButton(QStringLiteral("Close all"));
        QObject::connect(progress, &AntButton::clicked, owner, [owner]() {
            auto* notification = AntNotification::success(QStringLiteral("Uploading"),
                                                          QStringLiteral("Progress bar shows the remaining display time."),
                                                          owner,
                                                          5000);
            notification->setShowProgress(true);
        });
        QObject::connect(sticky, &AntButton::clicked, owner, [owner]() {
            AntNotification::warning(QStringLiteral("Manual close"),
                                     QStringLiteral("This notification will stay until the close button is clicked."),
                                     owner,
                                     0);
        });
        QObject::connect(closeAll, &AntButton::clicked, owner, []() { AntNotification::closeAll(); });
        durationRow->addWidget(progress);
        durationRow->addWidget(sticky);
        durationRow->addWidget(closeAll);
        durationRow->addStretch();
        cl->addLayout(durationRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createPopconfirmPage(QWidget* owner)
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

        auto* deleteButton = new AntButton(QStringLiteral("Delete"));
        deleteButton->setDanger(true);
        auto* deleteConfirm = new AntPopconfirm(page);
        deleteConfirm->setTarget(deleteButton);
        deleteConfirm->setTitle(QStringLiteral("Delete this record?"));
        deleteConfirm->setDescription(QStringLiteral("This operation cannot be undone."));
        QObject::connect(deleteConfirm, &AntPopconfirm::confirmRequested, owner, [owner]() {
            AntMessage::success(QStringLiteral("Deleted"), owner, 1400);
        });
        QObject::connect(deleteConfirm, &AntPopconfirm::cancelRequested, owner, [owner]() {
            AntMessage::info(QStringLiteral("Canceled"), owner, 1400);
        });

        auto* archiveButton = new AntButton(QStringLiteral("Archive"));
        auto* archiveConfirm = new AntPopconfirm(page);
        archiveConfirm->setTarget(archiveButton);
        archiveConfirm->setTitle(QStringLiteral("Archive item?"));
        archiveConfirm->setDescription(QStringLiteral("You can restore it later from the archive list."));
        archiveConfirm->setOkText(QStringLiteral("Archive"));
        archiveConfirm->setCancelText(QStringLiteral("Keep"));

        basicRow->addWidget(deleteButton);
        basicRow->addWidget(archiveButton);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Placement"));
        auto* cl = card->bodyLayout();
        auto* placementRow = new QHBoxLayout();
        placementRow->setSpacing(16);

        auto* topButton = new AntButton(QStringLiteral("Top"));
        auto* topConfirm = new AntPopconfirm(page);
        topConfirm->setTarget(topButton);
        topConfirm->setPlacement(Ant::TooltipPlacement::Top);
        topConfirm->setTitle(QStringLiteral("Publish changes now?"));

        auto* rightButton = new AntButton(QStringLiteral("Right"));
        auto* rightConfirm = new AntPopconfirm(page);
        rightConfirm->setTarget(rightButton);
        rightConfirm->setPlacement(Ant::TooltipPlacement::Right);
        rightConfirm->setTitle(QStringLiteral("Move to next step?"));

        auto* bottomLeftButton = new AntButton(QStringLiteral("BottomLeft"));
        auto* bottomLeftConfirm = new AntPopconfirm(page);
        bottomLeftConfirm->setTarget(bottomLeftButton);
        bottomLeftConfirm->setPlacement(Ant::TooltipPlacement::BottomLeft);
        bottomLeftConfirm->setTitle(QStringLiteral("Sign out this device?"));

        placementRow->addWidget(topButton);
        placementRow->addWidget(rightButton);
        placementRow->addWidget(bottomLeftButton);
        placementRow->addStretch();
        cl->addLayout(placementRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Variants"));
        auto* cl = card->bodyLayout();
        auto* variantRow = new QHBoxLayout();
        variantRow->setSpacing(16);

        auto* minimalButton = new AntButton(QStringLiteral("No Cancel"));
        auto* minimalConfirm = new AntPopconfirm(page);
        minimalConfirm->setTarget(minimalButton);
        minimalConfirm->setTitle(QStringLiteral("Proceed with sync?"));
        minimalConfirm->setShowCancel(false);
        minimalConfirm->setOkText(QStringLiteral("Continue"));

        auto* disabledButton = new AntButton(QStringLiteral("Disabled"));
        auto* disabledConfirm = new AntPopconfirm(page);
        disabledConfirm->setTarget(disabledButton);
        disabledConfirm->setTitle(QStringLiteral("This should not open"));
        disabledConfirm->setDisabled(true);

        variantRow->addWidget(minimalButton);
        variantRow->addWidget(disabledButton);
        variantRow->addStretch();
        cl->addLayout(variantRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
