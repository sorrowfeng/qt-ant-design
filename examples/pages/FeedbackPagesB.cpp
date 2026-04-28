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
QWidget* createAppPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("App Component"));
        auto* cl = card->bodyLayout();

        auto* desc = makeParagraph(QStringLiteral("App provides static methods like App.useApp() for Message, Notification, and Modal."),
                                   page,
                                   Ant::TypographyType::Secondary);
        cl->addWidget(desc);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createConfigProviderPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("ConfigProvider"));
        auto* cl = card->bodyLayout();

        auto* note = makeParagraph(
            QStringLiteral("ConfigProvider provides global configuration for Ant Design components (theme, locale, etc.)."),
            page,
            Ant::TypographyType::Secondary);
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
        cl->setAlignment(Qt::AlignTop);

        auto* desc = new AntTypography(QStringLiteral("FloatButton appears in the bottom-right corner."));
        desc->setType(Ant::TypographyType::Secondary);
        cl->addWidget(desc);

        layout->addWidget(card);
    }

    auto* fab1 = new AntFloatButton(page);
    fab1->setIcon(QStringLiteral("home"));
    fab1->setPlacement(Ant::FloatButtonPlacement::BottomRight);
    fab1->show();
    fab1->raise();

    layout->addStretch();
    return page;
}
}
