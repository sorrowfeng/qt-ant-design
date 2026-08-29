#include "Pages.h"

#include <QColor>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

#include "PageCommon.h"
#include "core/AntTheme.h"
#include "core/AntTypes.h"
#include "widgets/AntApp.h"
#include "widgets/AntBorderBeam.h"
#include "widgets/AntButton.h"
#include "widgets/AntCard.h"
#include "widgets/AntConfigProvider.h"
#include "widgets/AntFloatButton.h"
#include "widgets/AntMessage.h"
#include "widgets/AntNotification.h"
#include "widgets/AntTypography.h"
#include "widgets/AntWidget.h"

namespace example::pages
{
QWidget* createAppPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* appContext = new AntApp(page, page);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("App Component"));
        auto* cl = card->bodyLayout();

        auto* desc = makeParagraph(QStringLiteral("AntApp binds feedback to a visible host widget and exposes real Message, Modal, and Notification entry points. It also reports presentation failures instead of silently creating feedback for a hidden or zero-size host."),
                                   page,
                                   Ant::TypographyType::Secondary);
        cl->addWidget(desc);

        auto* actions = new QHBoxLayout();
        actions->setSpacing(8);
        auto* messageButton = new AntButton(QStringLiteral("Show message"), page);
        messageButton->setButtonType(Ant::ButtonType::Primary);
        auto* modalButton = new AntButton(QStringLiteral("Show modal"), page);
        auto* notificationButton = new AntButton(QStringLiteral("Show notification"), page);
        actions->addWidget(messageButton);
        actions->addWidget(modalButton);
        actions->addWidget(notificationButton);
        actions->addStretch();
        cl->addLayout(actions);

        auto* status = makeParagraph(QStringLiteral("Use the buttons to invoke AntApp::showMessage(), showModal(), and showNotification()."),
                                     page,
                                     Ant::TypographyType::Secondary);
        cl->addWidget(status);

        QObject::connect(messageButton, &AntButton::clicked, appContext, [appContext]() {
            appContext->showMessage(QStringLiteral("Message presented by AntApp"));
        });
        QObject::connect(modalButton, &AntButton::clicked, appContext, [appContext, status]() {
            appContext->showModal(
                QStringLiteral("AntApp modal"),
                QStringLiteral("The callbacks are wired through AntApp::showModal()."),
                [status]() { status->setText(QStringLiteral("Modal confirmed.")); },
                [status]() { status->setText(QStringLiteral("Modal canceled.")); });
        });
        QObject::connect(notificationButton, &AntButton::clicked, appContext, [appContext]() {
            appContext->showNotification(QStringLiteral("AntApp notification"),
                                         QStringLiteral("Notification presented against this page."));
        });
        QObject::connect(appContext, &AntApp::feedbackFailed, status,
                         [status](const QString& feedbackType) {
            status->setText(QStringLiteral("Unable to present %1: the feedback host is not visible or has no usable size.")
                                .arg(feedbackType));
        });

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createConfigProviderPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* provider = new AntConfigProvider(page);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("ConfigProvider"));
        auto* cl = card->bodyLayout();

        auto* note = makeParagraph(
            QStringLiteral("AntConfigProvider stages theme mode, primary color, base font size, and base border radius. apply() publishes all four token families atomically and emits the generic theme lifecycle signals used by every themed component."),
            page,
            Ant::TypographyType::Secondary);
        cl->addWidget(note);

        auto* actions = new QHBoxLayout();
        actions->setSpacing(8);
        auto* presetButton = new AntButton(QStringLiteral("Apply purple preset"), page);
        presetButton->setButtonType(Ant::ButtonType::Primary);
        auto* resetButton = new AntButton(QStringLiteral("Restore defaults"), page);
        actions->addWidget(presetButton);
        actions->addWidget(resetButton);
        actions->addStretch();
        cl->addLayout(actions);

        auto* status = makeParagraph(QStringLiteral("The preset is applied only when apply() is called."),
                                     page,
                                     Ant::TypographyType::Secondary);
        cl->addWidget(status);

        QObject::connect(presetButton, &AntButton::clicked, provider, [provider, status]() {
            provider->setThemeMode(Ant::ThemeMode::Dark);
            provider->setPrimaryColor(QColor(QStringLiteral("#722ed1")));
            provider->setFontSize(15);
            provider->setBorderRadius(8);
            provider->apply();
            status->setText(QStringLiteral("Applied Dark / #722ed1 / 15 px / 8 px atomically."));
        });
        QObject::connect(resetButton, &AntButton::clicked, provider, [provider, status]() {
            provider->setThemeMode(Ant::ThemeMode::Default);
            provider->setPrimaryColor(QColor());
            provider->setFontSize(Ant::FontSize);
            provider->setBorderRadius(Ant::BorderRadius);
            provider->apply();
            status->setText(QStringLiteral("Restored the built-in theme token defaults."));
        });

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createBorderBeamPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    auto makeBeamContent = [](const QString& text, QWidget* parent) {
        auto* content = new AntWidget(parent);
        auto* cl = new QVBoxLayout(content);
        cl->setContentsMargins(16, 12, 16, 12);
        cl->addWidget(makeParagraph(text, content));
        return content;
    };

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* beam = new AntBorderBeam(page);
        beam->setContentWidget(makeBeamContent(QStringLiteral("AntBorderBeam draws an animated light beam along the container border, useful for highlighting key cards or call-to-action areas."), beam));
        cl->addWidget(beam);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Count & Color"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* row = new QHBoxLayout();
        row->setSpacing(16);

        auto* triple = new AntBorderBeam(page);
        triple->setCount(3);
        triple->setColor(QColor(QStringLiteral("#722ed1")));
        triple->setContentWidget(makeBeamContent(QStringLiteral("count = 3"), triple));
        auto* fast = new AntBorderBeam(page);
        fast->setDuration(1200);
        fast->setBeamLength(96);
        fast->setColor(QColor(QStringLiteral("#13c2c2")));
        fast->setContentWidget(makeBeamContent(QStringLiteral("duration = 1200, beamLength = 96"), fast));

        row->addWidget(triple);
        row->addWidget(fast);
        cl->addLayout(row);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Hover To Run"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* hoverBeam = new AntBorderBeam(page);
        hoverBeam->setActiveOnHover(true);
        hoverBeam->setLineWidth(3);
        hoverBeam->setContentWidget(makeBeamContent(QStringLiteral("Hover this card to start the beam animation."), hoverBeam));
        cl->addWidget(hoverBeam);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createFloatButtonPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* desc = new AntTypography(QStringLiteral("FloatButton appears in the bottom-right corner."));
        desc->setType(Ant::TypographyType::Secondary);
        cl->addWidget(desc);

        layout->addWidget(card);
    }

    auto* fab1 = new AntFloatButton(page);
    fab1->setIcon(QStringLiteral("home"));
    fab1->setPlacement(Ant::Placement::BottomRight);
    fab1->show();
    fab1->raise();

    layout->addStretch();
    return page;
}
}
