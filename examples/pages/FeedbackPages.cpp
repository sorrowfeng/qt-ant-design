#include "Pages.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
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
QWidget* createAlertPage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(18);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* info = new AntAlert(QStringLiteral("Informational Notes"));
    info->setAlertType(Ant::AlertType::Info);

    auto* success = new AntAlert(QStringLiteral("Success Tips"));
    success->setAlertType(Ant::AlertType::Success);

    auto* warning = new AntAlert(QStringLiteral("Warning"));
    warning->setAlertType(Ant::AlertType::Warning);

    auto* error = new AntAlert(QStringLiteral("Error"));
    error->setAlertType(Ant::AlertType::Error);

    layout->addWidget(info);
    layout->addWidget(success);
    layout->addWidget(warning);
    layout->addWidget(error);

    layout->addWidget(createSectionTitle(QStringLiteral("Description and Icon")));
    auto* described = new AntAlert(QStringLiteral("Data sync in progress"));
    described->setDescription(QStringLiteral("Background synchronization is running. You can keep working while we update the latest records from the server."));
    described->setAlertType(Ant::AlertType::Info);
    described->setShowIcon(true);
    layout->addWidget(described);

    auto* withDescription = new AntAlert(QStringLiteral("Deployment failed"));
    withDescription->setDescription(QStringLiteral("The production deployment stopped because environment variables are incomplete. Please review the release configuration and try again."));
    withDescription->setAlertType(Ant::AlertType::Error);
    withDescription->setShowIcon(true);
    layout->addWidget(withDescription);

    layout->addWidget(createSectionTitle(QStringLiteral("Closable and Action")));
    auto* closable = new AntAlert(QStringLiteral("Update available"));
    closable->setDescription(QStringLiteral("A new desktop package is ready. You can update now or postpone until after your current task."));
    closable->setAlertType(Ant::AlertType::Warning);
    closable->setShowIcon(true);
    closable->setClosable(true);
    QObject::connect(closable, &AntAlert::closeRequested, owner, [owner]() {
        AntMessage::info(QStringLiteral("Alert closed"), owner, 1500);
    });
    layout->addWidget(closable);

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
    layout->addWidget(actionAlert);

    layout->addWidget(createSectionTitle(QStringLiteral("Banner")));
    auto* banner = new AntAlert(QStringLiteral("Scheduled maintenance tonight from 01:00 to 03:00."));
    banner->setBanner(true);
    banner->setClosable(true);
    layout->addWidget(banner);

    layout->addStretch();
    return page;
}

QWidget* createDrawerPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 16, 24, 16);
    layout->setSpacing(16);

    layout->addWidget(createSectionTitle(QStringLiteral("AntDrawer - Sliding Panel")));

    auto* desc = new AntTypography(QStringLiteral("AntDrawer is a sliding panel that overlays from the edge of the screen. "
                                                  "Supports Left/Right/Top/Bottom placement with animation."));
    desc->setParagraph(true);
    layout->addWidget(desc);

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

    layout->addLayout(btnLayout);
    layout->addStretch();
    return page;
}

QWidget* createMessagePage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Types")));
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
    layout->addLayout(typeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Duration")));
    auto* durationRow = new QHBoxLayout();
    durationRow->setSpacing(12);
    auto* shortMsg = new AntButton(QStringLiteral("1 second"));
    auto* stickyMsg = new AntButton(QStringLiteral("Manual close"));
    QObject::connect(shortMsg, &AntButton::clicked, owner, [owner]() { AntMessage::success(QStringLiteral("This closes quickly"), owner, 1000); });
    QObject::connect(stickyMsg, &AntButton::clicked, owner, [owner]() { AntMessage::info(QStringLiteral("Click this message to close it"), owner, 0); });
    durationRow->addWidget(shortMsg);
    durationRow->addWidget(stickyMsg);
    durationRow->addStretch();
    layout->addLayout(durationRow);

    layout->addStretch();
    return page;
}

QWidget* createModalPage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
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
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Custom Content")));
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
    layout->addLayout(customRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Behavior")));
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
    layout->addLayout(behaviorRow);

    layout->addStretch();
    return page;
}

QWidget* createNotificationPage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Types")));
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
    layout->addLayout(typeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Placement")));
    auto* placementRow = new QHBoxLayout();
    placementRow->setSpacing(12);
    const QList<QPair<QString, Ant::NotificationPlacement>> placements = {
        {QStringLiteral("Top left"), Ant::NotificationPlacement::TopLeft},
        {QStringLiteral("Top"), Ant::NotificationPlacement::Top},
        {QStringLiteral("Top right"), Ant::NotificationPlacement::TopRight},
        {QStringLiteral("Bottom left"), Ant::NotificationPlacement::BottomLeft},
        {QStringLiteral("Bottom"), Ant::NotificationPlacement::Bottom},
        {QStringLiteral("Bottom right"), Ant::NotificationPlacement::BottomRight},
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
    layout->addLayout(placementRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Duration and Progress")));
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
    layout->addLayout(durationRow);

    layout->addStretch();
    return page;
}

QWidget* createPopconfirmPage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
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
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Placement")));
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
    layout->addLayout(placementRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Variants")));
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
    layout->addLayout(variantRow);

    layout->addStretch();
    return page;
}

QWidget* createProgressPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Line")));
    const QList<int> values = {30, 50, 70, 100};
    for (int value : values)
    {
        auto* progress = new AntProgress();
        progress->setPercent(value);
        if (value == 100)
        {
            progress->setStatus(Ant::ProgressStatus::Success);
        }
        layout->addWidget(progress);
    }

    layout->addWidget(createSectionTitle(QStringLiteral("Status")));
    auto* statusRow = new QHBoxLayout();
    statusRow->setSpacing(18);
    auto* active = new AntProgress();
    active->setPercent(55);
    active->setStatus(Ant::ProgressStatus::Active);
    auto* exception = new AntProgress();
    exception->setPercent(45);
    exception->setStatus(Ant::ProgressStatus::Exception);
    statusRow->addWidget(active);
    statusRow->addWidget(exception);
    layout->addLayout(statusRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Circle")));
    auto* circleRow = new QHBoxLayout();
    circleRow->setSpacing(24);
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
    layout->addLayout(circleRow);

    layout->addStretch();
    return page;
}

QWidget* createResultPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(28);

    layout->addWidget(createSectionTitle(QStringLiteral("Status")));
    auto* statusRow = new QHBoxLayout();
    statusRow->setSpacing(28);

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
    layout->addLayout(statusRow);

    layout->addWidget(createSectionTitle(QStringLiteral("With Extra Actions")));
    auto* actionRow = new QHBoxLayout();
    actionRow->setSpacing(28);

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
    layout->addLayout(actionRow);

    layout->addStretch();
    return page;
}

QWidget* createSkeletonPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic")));
    auto* basicRow = new QHBoxLayout();
    basicRow->setSpacing(20);

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
    layout->addLayout(basicRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Avatar and Round")));
    auto* avatarRow = new QHBoxLayout();
    avatarRow->setSpacing(20);

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
    layout->addLayout(avatarRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Loading Switch")));
    auto* switchRow = new QHBoxLayout();
    switchRow->setSpacing(20);

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
    layout->addLayout(switchRow);

    layout->addStretch();
    return page;
}

QWidget* createSpinPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Size")));
    auto* sizeRow = new QHBoxLayout();
    sizeRow->setSpacing(28);
    auto* small = new AntSpin();
    small->setSpinSize(Ant::SpinSize::Small);
    auto* middle = new AntSpin();
    auto* large = new AntSpin();
    large->setSpinSize(Ant::SpinSize::Large);
    sizeRow->addWidget(small);
    sizeRow->addWidget(middle);
    sizeRow->addWidget(large);
    sizeRow->addStretch();
    layout->addLayout(sizeRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Description")));
    auto* descRow = new QHBoxLayout();
    descRow->setSpacing(28);
    auto* loading = new AntSpin();
    loading->setSpinSize(Ant::SpinSize::Large);
    loading->setDescription(QStringLiteral("Loading"));
    auto* percent = new AntSpin();
    percent->setSpinSize(Ant::SpinSize::Large);
    percent->setPercent(68);
    percent->setDescription(QStringLiteral("68%"));
    auto* hidden = new AntSpin();
    hidden->setSpinning(false);
    hidden->setDescription(QStringLiteral("Hidden"));
    descRow->addWidget(loading);
    descRow->addWidget(percent);
    descRow->addWidget(hidden);
    descRow->addStretch();
    layout->addLayout(descRow);

    layout->addWidget(createSectionTitle(QStringLiteral("Embedded")));
    auto* card = new AntCard(QStringLiteral("Loading block"));
    card->setMinimumHeight(160);
    auto* cardContent = new QVBoxLayout();
    auto* embedded = new AntSpin();
    embedded->setDescription(QStringLiteral("Fetching data"));
    cardContent->addStretch();
    cardContent->addWidget(embedded, 0, Qt::AlignCenter);
    cardContent->addStretch();
    card->bodyLayout()->addLayout(cardContent);
    layout->addWidget(card);

    layout->addStretch();
    return page;
}

QWidget* createTourPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntTour"));
    QFont f = title->font();
    f.setPixelSize(20);
    f.setBold(true);
    title->setFont(f);
    layout->addWidget(title);

    auto* card = new AntCard(QStringLiteral("Release Checklist"), page);
    auto* bodyLayout = card->bodyLayout();
    auto* trigger = new AntButton(QStringLiteral("Start Tour"), card);
    trigger->setButtonType(Ant::ButtonType::Primary);
    auto* targetA = new AntInput(card);
    targetA->setPlaceholderText(QStringLiteral("Version"));
    auto* targetB = new AntSelect(card);
    targetB->addOption(QStringLiteral("Stable"));
    targetB->addOption(QStringLiteral("Beta"));
    auto* targetC = new AntButton(QStringLiteral("Publish"), card);
    bodyLayout->addWidget(trigger);
    bodyLayout->addWidget(targetA);
    bodyLayout->addWidget(targetB);
    bodyLayout->addWidget(targetC);
    layout->addWidget(card);

    auto* status = new QLabel(QStringLiteral("Tour not started"), page);
    layout->addWidget(status);

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

    layout->addStretch();
    return page;
}

QWidget* createWatermarkPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(24);

    layout->addWidget(createSectionTitle(QStringLiteral("Basic Text Watermark")));
    auto* wm1 = new AntWatermark();
    wm1->setContent(QStringLiteral("Ant Design Confidential"));
    wm1->setFixedHeight(160);
    auto* wm1Layout = new QVBoxLayout(wm1);
    wm1Layout->setContentsMargins(16, 16, 16, 16);
    auto* info = new AntTypography(QStringLiteral("This area is watermarked with repeated text."));
    info->setType(Ant::TypographyType::Secondary);
    wm1Layout->addWidget(info);
    layout->addWidget(wm1);

    layout->addWidget(createSectionTitle(QStringLiteral("Multi-line Watermark")));
    auto* wm2 = new AntWatermark();
    wm2->setContent(QStringList{QStringLiteral("Ant Design"), QStringLiteral("Internal Use Only")});
    wm2->setFixedHeight(160);
    auto* wm2Layout = new QVBoxLayout(wm2);
    wm2Layout->setContentsMargins(16, 16, 16, 16);
    auto* info2 = new AntTypography(QStringLiteral("Multi-line watermark with custom text."));
    info2->setType(Ant::TypographyType::Secondary);
    wm2Layout->addWidget(info2);
    layout->addWidget(wm2);

    layout->addWidget(createSectionTitle(QStringLiteral("Custom Rotation")));
    auto* wm3 = new AntWatermark();
    wm3->setContent(QStringLiteral("ROTATED"));
    wm3->setRotate(-45);
    wm3->setFixedHeight(160);
    auto* wm3Layout = new QVBoxLayout(wm3);
    wm3Layout->setContentsMargins(16, 16, 16, 16);
    wm3Layout->addWidget(new AntTypography(QStringLiteral("This watermark is rotated -45 degrees.")));
    layout->addWidget(wm3);

    layout->addStretch();
    return page;
}
}
