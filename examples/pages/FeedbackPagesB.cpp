#include "Pages.h"

#include <QColor>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

#include "PageCommon.h"
#include "core/AntLocale.h"
#include "core/AntTheme.h"
#include "core/AntTypes.h"
#include "widgets/AntApp.h"
#include "widgets/AntBorderBeam.h"
#include "widgets/AntButton.h"
#include "widgets/AntCard.h"
#include "widgets/AntConfigProvider.h"
#include "widgets/AntFloatButton.h"
#include "widgets/AntMessage.h"
#include "widgets/AntModal.h"
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
        auto* card = makeCard(layout, QStringLiteral("App Component"));
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
        auto* card = makeCard(layout, QStringLiteral("ConfigProvider"));
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

    }

    {
        auto* card = makeCard(layout, QStringLiteral("Compact Density"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* note = makeParagraph(
            QStringLiteral("AntConfigProvider::density applies the compactAlgorithm: controlHeight reduced by 4, padding/margin tokens scale to 75%."),
            page,
            Ant::TypographyType::Secondary);
        cl->addWidget(note);
        auto* btnRow = new QHBoxLayout();
        btnRow->setSpacing(8);
        auto* compactBtn = new AntButton(QStringLiteral("Compact"), page);
        compactBtn->setButtonType(Ant::ButtonType::Primary);
        auto* defaultBtn = new AntButton(QStringLiteral("Default"), page);
        btnRow->addWidget(compactBtn);
        btnRow->addWidget(defaultBtn);
        btnRow->addStretch();
        cl->addLayout(btnRow);
        auto* sampleRow = new QHBoxLayout();
        sampleRow->setSpacing(12);
        for (int i = 1; i <= 3; ++i)
        {
            sampleRow->addWidget(new AntButton(QStringLiteral("Button %1").arg(i)));
        }
        sampleRow->addStretch();
        cl->addSpacing(12);
        cl->addLayout(sampleRow);
        auto* status = makeSecondaryText(QStringLiteral("Current density: Default"), page);
        cl->addWidget(status);
        QObject::connect(compactBtn, &AntButton::clicked, provider, [provider, status]() {
            provider->setDensity(Ant::ThemeDensity::Compact);
            provider->apply();
            status->setText(QStringLiteral("Current density: Compact (controlHeight -4, padding 75%)"));
        });
        QObject::connect(defaultBtn, &AntButton::clicked, provider, [provider, status]() {
            provider->setDensity(Ant::ThemeDensity::Default);
            provider->apply();
            status->setText(QStringLiteral("Current density: Default"));
        });

    }

    {
        auto* card = makeCard(layout, QStringLiteral("RTL Direction"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* note = makeParagraph(
            QStringLiteral("AntConfigProvider::direction sets QApplication layout direction (LeftToRight / RightToLeft)."),
            page,
            Ant::TypographyType::Secondary);
        cl->addWidget(note);
        auto* btnRow = new QHBoxLayout();
        btnRow->setSpacing(8);
        auto* ltrBtn = new AntButton(QStringLiteral("LTR"), page);
        auto* rtlBtn = new AntButton(QStringLiteral("RTL"), page);
        rtlBtn->setButtonType(Ant::ButtonType::Primary);
        btnRow->addWidget(ltrBtn);
        btnRow->addWidget(rtlBtn);
        btnRow->addStretch();
        cl->addLayout(btnRow);
        auto* status = makeSecondaryText(QStringLiteral("Current: LeftToRight"), page);
        cl->addWidget(status);
        QObject::connect(ltrBtn, &AntButton::clicked, provider, [provider, status]() {
            provider->setDirection(Qt::LeftToRight);
            provider->apply();
            status->setText(QStringLiteral("Current: LeftToRight"));
        });
        QObject::connect(rtlBtn, &AntButton::clicked, provider, [provider, status]() {
            provider->setDirection(Qt::RightToLeft);
            provider->apply();
            status->setText(QStringLiteral("Current: RightToLeft"));
        });

    }

    {
        auto* card = makeCard(layout, QStringLiteral("Locale"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* note = makeParagraph(
            QStringLiteral("AntLocale drives built-in text (Modal/Popconfirm buttons, etc.). Set language to switch all uncustomized defaults."),
            page,
            Ant::TypographyType::Secondary);
        cl->addWidget(note);
        auto* btnRow = new QHBoxLayout();
        btnRow->setSpacing(8);
        auto* enBtn = new AntButton(QStringLiteral("English"), page);
        auto* zhBtn = new AntButton(QStringLiteral("中文"), page);
        zhBtn->setButtonType(Ant::ButtonType::Primary);
        btnRow->addWidget(enBtn);
        btnRow->addWidget(zhBtn);
        btnRow->addStretch();
        cl->addLayout(btnRow);
        auto* preview = new AntModal(page);
        preview->setClosable(false);
        preview->setShowCancel(true);
        preview->setOpen(true);
        cl->addSpacing(12);
        cl->addWidget(preview);
        auto* status = makeSecondaryText(QStringLiteral("Current locale: English"), page);
        cl->addWidget(status);
        QObject::connect(enBtn, &AntButton::clicked, page, [preview, status]() {
            antLocale->setLanguage(Ant::LocaleLanguage::English);
            status->setText(QStringLiteral("Current locale: English"));
        });
        QObject::connect(zhBtn, &AntButton::clicked, page, [preview, status]() {
            antLocale->setLanguage(Ant::LocaleLanguage::ChineseSimplified);
            status->setText(QStringLiteral("当前语言：简体中文"));
        });

    }

    {
        auto* card = makeCard(layout, QStringLiteral("Component Token Override"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* note = makeParagraph(
            QStringLiteral("antTheme->setComponentToken(\"Button\", \"borderRadius\", value) overrides the radius for all AntButtons globally. Clear to restore default."),
            page,
            Ant::TypographyType::Secondary);
        cl->addWidget(note);
        auto* btnRow = new QHBoxLayout();
        btnRow->setSpacing(8);
        auto* sharpBtn = new AntButton(QStringLiteral("Radius 0"), page);
        sharpBtn->setButtonType(Ant::ButtonType::Primary);
        auto* roundBtn = new AntButton(QStringLiteral("Radius 24"), page);
        auto* resetBtn = new AntButton(QStringLiteral("Restore"), page);
        btnRow->addWidget(sharpBtn);
        btnRow->addWidget(roundBtn);
        btnRow->addWidget(resetBtn);
        btnRow->addStretch();
        cl->addLayout(btnRow);
        auto* sampleRow = new QHBoxLayout();
        sampleRow->setSpacing(12);
        for (const QString& label : {QStringLiteral("Primary"), QStringLiteral("Default"), QStringLiteral("Dashed")})
        {
            auto* btn = new AntButton(label, page);
            if (label == QStringLiteral("Primary"))
            {
                btn->setButtonType(Ant::ButtonType::Primary);
            }
            else if (label == QStringLiteral("Dashed"))
            {
                btn->setButtonType(Ant::ButtonType::Dashed);
            }
            sampleRow->addWidget(btn);
        }
        sampleRow->addStretch();
        cl->addSpacing(12);
        cl->addLayout(sampleRow);
        auto* status = makeSecondaryText(QStringLiteral("Button.borderRadius: default (token)"), page);
        cl->addWidget(status);
        QObject::connect(sharpBtn, &AntButton::clicked, page, [status]() {
            antTheme->setComponentToken(QStringLiteral("Button"), QStringLiteral("borderRadius"), 0);
            status->setText(QStringLiteral("Button.borderRadius: 0 (sharp)"));
        });
        QObject::connect(roundBtn, &AntButton::clicked, page, [status]() {
            antTheme->setComponentToken(QStringLiteral("Button"), QStringLiteral("borderRadius"), 24);
            status->setText(QStringLiteral("Button.borderRadius: 24 (round)"));
        });
        QObject::connect(resetBtn, &AntButton::clicked, page, [status]() {
            antTheme->clearComponentTokens(QStringLiteral("Button"));
            status->setText(QStringLiteral("Button.borderRadius: default (token)"));
        });

    }

    layout->addStretch();
    return page;
}

QWidget* createBorderBeamPage(QWidget* /*owner*/)
{
    auto [page, layout] = makePage();

    auto makeBeamContent = [](const QString& text, QWidget* parent) {
        auto* content = new AntWidget(parent);
        auto* cl = new QVBoxLayout(content);
        cl->setContentsMargins(16, 12, 16, 12);
        cl->addWidget(makeParagraph(text, content));
        return content;
    };

    {
        auto* card = makeCard(layout, QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* beam = new AntBorderBeam(page);
        beam->setContentWidget(makeBeamContent(QStringLiteral("AntBorderBeam draws an animated light beam along the container border, useful for highlighting key cards or call-to-action areas."), beam));
        cl->addWidget(beam);

    }

    {
        auto* card = makeCard(layout, QStringLiteral("Count & Color"));
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

    }

    {
        auto* card = makeCard(layout, QStringLiteral("Hover To Run"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* hoverBeam = new AntBorderBeam(page);
        hoverBeam->setActiveOnHover(true);
        hoverBeam->setLineWidth(3);
        hoverBeam->setContentWidget(makeBeamContent(QStringLiteral("Hover this card to start the beam animation."), hoverBeam));
        cl->addWidget(hoverBeam);

    }

    layout->addStretch();
    return page;
}

QWidget* createFloatButtonPage(QWidget* /*owner*/)
{
    auto [page, layout] = makePage();

    {
        auto* card = makeCard(layout, QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* desc = new AntTypography(QStringLiteral("FloatButton appears in the bottom-right corner."));
        desc->setType(Ant::TypographyType::Secondary);
        cl->addWidget(desc);

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
