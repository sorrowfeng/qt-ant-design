#include "Pages.h"

#include <QFrame>
#include <QHBoxLayout>
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
QWidget* createAlertPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Types"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* success = new AntAlert(QStringLiteral("Success Text"));
        success->setAlertType(Ant::AlertType::Success);
        success->setShowIcon(true);
        success->setClosable(true);

        auto* info = new AntAlert(QStringLiteral("Info Text"));
        info->setAlertType(Ant::AlertType::Info);
        info->setShowIcon(true);
        info->setClosable(true);

        auto* warning = new AntAlert(QStringLiteral("Warning Text"));
        warning->setAlertType(Ant::AlertType::Warning);
        warning->setShowIcon(true);
        warning->setClosable(true);

        auto* error = new AntAlert(QStringLiteral("Error Text"));
        error->setAlertType(Ant::AlertType::Error);
        error->setShowIcon(true);
        error->setClosable(true);

        cl->addWidget(success);
        cl->addWidget(info);
        cl->addWidget(warning);
        cl->addWidget(error);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Banner"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* banner = new AntAlert(QStringLiteral("Warning text"));
        banner->setAlertType(Ant::AlertType::Warning);
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
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* openButton = new AntButton(QStringLiteral("Open Drawer"));
        openButton->setButtonType(Ant::ButtonType::Primary);
        auto* drawer = new AntDrawer(page);
        drawer->setTitle(QStringLiteral("Basic Drawer"));

        auto* body = new QWidget();
        auto* bodyLayout = new QVBoxLayout(body);
        bodyLayout->setContentsMargins(0, 0, 0, 0);
        bodyLayout->setSpacing(0);
        auto* content = new AntTypography(QStringLiteral("Drawer content here."));
        content->setParagraph(true);
        bodyLayout->addWidget(content);
        bodyLayout->addStretch();
        drawer->setBodyWidget(body);

        QObject::connect(openButton, &AntButton::clicked, drawer, &AntDrawer::open);

        cl->addWidget(openButton, 0, Qt::AlignLeft);
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
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(12);
        auto* success = new AntButton(QStringLiteral("Success"));
        auto* info = new AntButton(QStringLiteral("Info"));
        auto* warning = new AntButton(QStringLiteral("Warning"));
        auto* error = new AntButton(QStringLiteral("Error"));
        QObject::connect(success, &AntButton::clicked, owner, [owner]() { AntMessage::success(QStringLiteral("Success!"), owner); });
        QObject::connect(info, &AntButton::clicked, owner, [owner]() { AntMessage::info(QStringLiteral("Info!"), owner); });
        QObject::connect(warning, &AntButton::clicked, owner, [owner]() { AntMessage::warning(QStringLiteral("Warning!"), owner); });
        QObject::connect(error, &AntButton::clicked, owner, [owner]() { AntMessage::error(QStringLiteral("Error!"), owner); });
        basicRow->addWidget(success);
        basicRow->addWidget(info);
        basicRow->addWidget(warning);
        basicRow->addWidget(error);
        basicRow->addStretch();
        cl->addLayout(basicRow);
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
        cl->setAlignment(Qt::AlignTop);
        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(12);

        auto* openBasic = new AntButton(QStringLiteral("Open Modal"));
        openBasic->setButtonType(Ant::ButtonType::Primary);
        auto* basicModal = new AntModal(owner);
        basicModal->setTitle(QStringLiteral("Basic Modal"));
        basicModal->setContent(QStringLiteral("Modal content here."));
        basicModal->setCentered(false);
        QObject::connect(openBasic, &AntButton::clicked, owner, [basicModal]() {
            basicModal->setOpen(true);
        });

        basicRow->addWidget(openBasic);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Confirm"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* cmdRow = new QHBoxLayout();
        cmdRow->setSpacing(12);
        auto* infoBtn = new AntButton(QStringLiteral("Info"));
        QObject::connect(infoBtn, &AntButton::clicked, owner, [owner]() {
            AntModal::info(QStringLiteral("Info"), QStringLiteral("Info content"), owner);
        });
        auto* successBtn = new AntButton(QStringLiteral("Success"));
        QObject::connect(successBtn, &AntButton::clicked, owner, [owner]() {
            AntModal::success(QStringLiteral("Success"), QStringLiteral("Success content"), owner);
        });
        auto* warningBtn = new AntButton(QStringLiteral("Warning"));
        QObject::connect(warningBtn, &AntButton::clicked, owner, [owner]() {
            AntModal::warning(QStringLiteral("Warning"), QStringLiteral("Warning content"), owner);
        });
        auto* errorBtn = new AntButton(QStringLiteral("Error"));
        QObject::connect(errorBtn, &AntButton::clicked, owner, [owner]() {
            AntModal::error(QStringLiteral("Error"), QStringLiteral("Error content"), owner);
        });
        auto* confirmBtn = new AntButton(QStringLiteral("Confirm"));
        QObject::connect(confirmBtn, &AntButton::clicked, owner, [owner]() {
            AntModal::confirm(QStringLiteral("Confirm"), QStringLiteral("Are you sure?"), owner);
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
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(12);
        auto* open = new AntButton(QStringLiteral("Open"));
        auto* success = new AntButton(QStringLiteral("Success"));
        QObject::connect(open, &AntButton::clicked, owner, [owner]() {
            AntNotification::open(QStringLiteral("Notification Title"),
                                  QStringLiteral("Notification description."),
                                  owner);
        });
        QObject::connect(success, &AntButton::clicked, owner, [owner]() {
            AntNotification::success(QStringLiteral("Success"),
                                     QStringLiteral("Success notification."),
                                     owner);
        });
        basicRow->addWidget(open);
        basicRow->addWidget(success);
        basicRow->addStretch();
        cl->addLayout(basicRow);
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
        cl->setAlignment(Qt::AlignTop);
        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(12);

        auto* deleteButton = new AntButton(QStringLiteral("Delete"));
        deleteButton->setDanger(true);
        auto* deleteConfirm = new AntPopconfirm(page);
        deleteConfirm->setTarget(deleteButton);
        deleteConfirm->setTitle(QStringLiteral("Are you sure?"));
        deleteConfirm->setDescription(QStringLiteral("This action cannot be undone."));
        deleteConfirm->setOkText(QStringLiteral("Yes"));
        deleteConfirm->setCancelText(QStringLiteral("No"));
        QObject::connect(deleteConfirm, &AntPopconfirm::confirmRequested, owner, [owner]() {
            AntMessage::success(QStringLiteral("Deleted"), owner, 1400);
        });
        QObject::connect(deleteConfirm, &AntPopconfirm::cancelRequested, owner, [owner]() {
            AntMessage::info(QStringLiteral("Canceled"), owner, 1400);
        });

        basicRow->addWidget(deleteButton);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
