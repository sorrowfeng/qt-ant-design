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
QWidget* createAppPage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntApp"));
        auto* cl = card->bodyLayout();

        auto* desc = makeParagraph(
            QStringLiteral("AntApp provides an application-level wrapper around message, modal and notification context. "
                           "In the current port the wrapper exists and tracks the root widget, while feedback calls are still minimal."),
            page);
        cl->addWidget(desc);

        auto* app = new AntApp(page, page);
        auto* status = makeText(QStringLiteral("Instance root: %1").arg(app->rootWidget() == page ? QStringLiteral("attached") : QStringLiteral("missing")), page);
        cl->addWidget(status);

        auto* buttonRow = new QHBoxLayout();
        auto* msgBtn = new AntButton(QStringLiteral("Show Message"));
        auto* modalBtn = new AntButton(QStringLiteral("Show Modal"));
        auto* notificationBtn = new AntButton(QStringLiteral("Show Notification"));
        buttonRow->addWidget(msgBtn);
        buttonRow->addWidget(modalBtn);
        buttonRow->addWidget(notificationBtn);
        buttonRow->addStretch();
        cl->addLayout(buttonRow);

        QObject::connect(msgBtn, &AntButton::clicked, page, [owner, app]() {
            app->showMessage(QStringLiteral("Build started"));
            AntMessage::info(QStringLiteral("AntApp message API is wired to the current root widget."), owner, 1600);
        });
        QObject::connect(modalBtn, &AntButton::clicked, page, [owner, app]() {
            app->showModal(QStringLiteral("Publish"), QStringLiteral("AntApp modal wrapper placeholder"));
            AntNotification::info(QStringLiteral("AntApp"),
                                  QStringLiteral("Modal wrapper exists; full context-driven implementation can be expanded later."),
                                  owner);
        });
        QObject::connect(notificationBtn, &AntButton::clicked, page, [owner, app]() {
            app->showNotification(QStringLiteral("Deploy"), QStringLiteral("Notification placeholder"));
            AntNotification::success(QStringLiteral("AntApp"),
                                     QStringLiteral("Singleton instance is available and ready to host global feedback APIs."),
                                     owner);
        });

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createConfigProviderPage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntConfigProvider"));
        auto* cl = card->bodyLayout();

        auto* provider = new AntConfigProvider(page);
        provider->setThemeMode(antTheme->themeMode());
        provider->setPrimaryColor(QColor(QStringLiteral("#1677ff")));
        provider->setFontSize(14);
        provider->setBorderRadius(6);

        auto* summary = makeText(QString(), page);
        auto updateSummary = [summary, provider]() {
            summary->setText(QStringLiteral("theme=%1, primary=%2, fontSize=%3, radius=%4")
                                 .arg(provider->themeMode() == Ant::ThemeMode::Dark ? QStringLiteral("dark") : QStringLiteral("light"))
                                 .arg(provider->primaryColor().name())
                                 .arg(provider->fontSize())
                                 .arg(provider->borderRadius()));
        };
        updateSummary();
        cl->addWidget(summary);

        auto* buttonRow = new QHBoxLayout();
        auto* lightBtn = new AntButton(QStringLiteral("Light"));
        auto* darkBtn = new AntButton(QStringLiteral("Dark"));
        auto* applyBtn = new AntButton(QStringLiteral("Apply Theme Mode"));
        applyBtn->setButtonType(Ant::ButtonType::Primary);
        buttonRow->addWidget(lightBtn);
        buttonRow->addWidget(darkBtn);
        buttonRow->addWidget(applyBtn);
        buttonRow->addStretch();
        cl->addLayout(buttonRow);

        QObject::connect(lightBtn, &AntButton::clicked, page, [provider, updateSummary]() {
            provider->setThemeMode(Ant::ThemeMode::Default);
            updateSummary();
        });
        QObject::connect(darkBtn, &AntButton::clicked, page, [provider, updateSummary]() {
            provider->setThemeMode(Ant::ThemeMode::Dark);
            updateSummary();
        });
        QObject::connect(applyBtn, &AntButton::clicked, page, [owner, provider]() {
            provider->apply();
            AntMessage::success(QStringLiteral("ConfigProvider applied theme mode"), owner, 1400);
        });

        auto* note = makeParagraph(
            QStringLiteral("This demo focuses on the current C++ API surface. The provider already stores theme, primary color, font size and radius settings; theme mode application is live."),
            page);
        cl->addWidget(note);

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

        auto* desc = new AntTypography(QStringLiteral("Floating buttons appear in the bottom-right corner. Use them in a real app."));
        desc->setType(Ant::TypographyType::Secondary);
        cl->addWidget(desc);

        auto* scene = new AntWidget();
        scene->setFixedHeight(200);

        auto* fab1 = new AntFloatButton(scene);
        fab1->setIcon(QStringLiteral("✦"));
        fab1->setFloatButtonType(Ant::FloatButtonType::Primary);
        fab1->setPlacement(Ant::FloatButtonPlacement::BottomRight);
        fab1->show();

        cl->addWidget(scene);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Square Shape with Text"));
        auto* cl = card->bodyLayout();
        auto* scene2 = new AntWidget();
        scene2->setFixedHeight(80);
        auto* fab2 = new AntFloatButton(scene2);
        fab2->setIcon(QStringLiteral("✦"));
        fab2->setContent(QStringLiteral("Ask"));
        fab2->setFloatButtonShape(Ant::FloatButtonShape::Square);
        fab2->setPlacement(Ant::FloatButtonPlacement::BottomRight);
        fab2->show();
        cl->addWidget(scene2);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Badge"));
        auto* cl = card->bodyLayout();
        auto* scene3 = new AntWidget();
        scene3->setFixedHeight(80);
        auto* fab3 = new AntFloatButton(scene3);
        fab3->setIcon(QStringLiteral("✉"));
        fab3->setBadgeDot(true);
        fab3->setPlacement(Ant::FloatButtonPlacement::BottomRight);
        fab3->show();
        cl->addWidget(scene3);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
