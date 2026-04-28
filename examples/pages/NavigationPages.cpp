#include "Pages.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QPainter>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "core/AntTypes.h"
#include "widgets/AntAffix.h"
#include "widgets/AntAnchor.h"
#include "widgets/AntBreadcrumb.h"
#include "widgets/AntButton.h"
#include "widgets/AntCard.h"
#include "widgets/AntDropdown.h"
#include "widgets/AntMenu.h"
#include "widgets/AntPagination.h"
#include "widgets/AntSteps.h"
#include "widgets/AntTypography.h"

namespace
{
class AnchorContentBlock : public QFrame
{
public:
    AnchorContentBlock(const QString& text, const QColor& bg, QWidget* parent = nullptr)
        : QFrame(parent), m_bg(bg)
    {
        setFixedHeight(100);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        layout->addWidget(new AntTypography(text, this));
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(m_bg);
        p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 4, 4);
    }

private:
    QColor m_bg;
};

QVBoxLayout* topAlignedBody(AntCard* card)
{
    auto* layout = card->bodyLayout();
    layout->setAlignment(Qt::AlignTop);
    return layout;
}
}

namespace example::pages
{
QWidget* createAffixPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Affix"));
        auto* cl = topAlignedBody(card);
        cl->setSpacing(16);

        auto* affixedBtn = new AntButton(QStringLiteral("Affix Top 10px"));
        affixedBtn->setButtonType(Ant::ButtonType::Primary);
        cl->addWidget(affixedBtn, 0, Qt::AlignLeft);

        auto* note = new AntTypography(QStringLiteral("Scroll the page to see the button stick to the top."));
        note->setType(Ant::TypographyType::Secondary);
        cl->addWidget(note);

        auto* affix = new AntAffix(page);
        affix->setOffsetTop(10);
        affix->setAffixedWidget(affixedBtn);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createAnchorPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    auto* card = new AntCard(QStringLiteral("Anchor"));
    auto* cl = topAlignedBody(card);
    cl->setSpacing(12);

    auto* anchor = new AntAnchor(card);
    anchor->addLink(QStringLiteral("Part 1"), 0);
    anchor->addLink(QStringLiteral("Part 2"), 100);
    anchor->addLink(QStringLiteral("Part 3"), 200);
    cl->addWidget(anchor, 0, Qt::AlignLeft);

    cl->addWidget(new AnchorContentBlock(QStringLiteral("Part 1 content"), QColor(QStringLiteral("#f0f5ff")), card));
    cl->addWidget(new AnchorContentBlock(QStringLiteral("Part 2 content"), QColor(QStringLiteral("#f6ffed")), card));
    cl->addWidget(new AnchorContentBlock(QStringLiteral("Part 3 content"), QColor(QStringLiteral("#fff7e6")), card));
    layout->addWidget(card);

    layout->addStretch();
    return page;
}

QWidget* createBreadcrumbPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = topAlignedBody(card);
        auto* basic = new AntBreadcrumb();
        basic->addItem(QStringLiteral("Home"));
        basic->addItem(QStringLiteral("Products"));
        basic->addItem(QStringLiteral("Detail"));
        cl->addWidget(basic);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Separator"));
        auto* cl = topAlignedBody(card);
        auto* custom = new AntBreadcrumb();
        custom->setSeparator(QStringLiteral(">"));
        custom->addItem(QStringLiteral("Home"));
        custom->addItem(QStringLiteral("Application"));
        custom->addItem(QStringLiteral("List"));
        cl->addWidget(custom);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createDropdownPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = topAlignedBody(card);
        auto* row = new QHBoxLayout();
        row->setSpacing(16);

        auto* hoverButton = new AntButton(QStringLiteral("Hover me"));
        hoverButton->setButtonIconType(Ant::IconType::Down);
        auto* hoverDropdown = new AntDropdown(page);
        hoverDropdown->setTarget(hoverButton);
        hoverDropdown->addItem(QStringLiteral("1"), QStringLiteral("Menu Item 1"));
        hoverDropdown->addItem(QStringLiteral("2"), QStringLiteral("Menu Item 2"));
        hoverDropdown->addDivider();
        hoverDropdown->addItem(QStringLiteral("3"), QStringLiteral("Menu Item 3"));

        auto* contextButton = new AntButton(QStringLiteral("Right Click"));
        contextButton->setButtonType(Ant::ButtonType::Primary);
        auto* contextDropdown = new AntDropdown(page);
        contextDropdown->setTrigger(Ant::DropdownTrigger::ContextMenu);
        contextDropdown->setTarget(contextButton);
        contextDropdown->addItem(QStringLiteral("1"), QStringLiteral("Action 1"));
        contextDropdown->addItem(QStringLiteral("2"), QStringLiteral("Action 2"));
        contextDropdown->addItem(QStringLiteral("3"), QStringLiteral("Action 3"));

        row->addWidget(hoverButton);
        row->addWidget(contextButton);
        row->addStretch();
        cl->addLayout(row);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createMenuPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Horizontal"));
        auto* cl = topAlignedBody(card);
        auto* horizontal = new AntMenu();
        horizontal->setMode(Ant::MenuMode::Horizontal);
        horizontal->setSelectedKey(QStringLiteral("mail"));
        horizontal->addItem(QStringLiteral("mail"), QStringLiteral("Mail"), Ant::IconType::Mail);
        horizontal->addItem(QStringLiteral("app"), QStringLiteral("App"), QStringLiteral("A"));
        horizontal->addSubMenu(QStringLiteral("sub"), QStringLiteral("Sub Menu"));
        horizontal->addSubItem(QStringLiteral("sub"), QStringLiteral("s1"), QStringLiteral("Option 1"));
        horizontal->addSubItem(QStringLiteral("sub"), QStringLiteral("s2"), QStringLiteral("Option 2"));
        horizontal->setMinimumHeight(48);
        cl->addWidget(horizontal);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Vertical / Inline"));
        auto* cl = topAlignedBody(card);
        auto* inlineMenu = new AntMenu();
        inlineMenu->setMode(Ant::MenuMode::Inline);
        inlineMenu->setSelectedKey(QStringLiteral("1"));
        inlineMenu->addItem(QStringLiteral("1"), QStringLiteral("Option 1"), Ant::IconType::Home);
        inlineMenu->addItem(QStringLiteral("2"), QStringLiteral("Option 2"), Ant::IconType::User);
        inlineMenu->addSubMenu(QStringLiteral("sub"), QStringLiteral("Navigation"), Ant::IconType::Setting);
        inlineMenu->addSubItem(QStringLiteral("sub"), QStringLiteral("s1"), QStringLiteral("Sub 1"));
        inlineMenu->addSubItem(QStringLiteral("sub"), QStringLiteral("s2"), QStringLiteral("Sub 2"));
        inlineMenu->setFixedWidth(200);
        inlineMenu->setMinimumHeight(200);
        cl->addWidget(inlineMenu, 0, Qt::AlignLeft);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createPaginationPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = topAlignedBody(card);
        auto* basic = new AntPagination();
        basic->setTotal(50);
        basic->setCurrent(1);
        cl->addWidget(basic);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("More Options"));
        auto* cl = topAlignedBody(card);
        auto* options = new AntPagination();
        options->setTotal(200);
        options->setCurrent(1);
        options->setShowSizeChanger(true);
        options->setShowQuickJumper(true);
        options->setShowTotal(true);
        cl->addWidget(options);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Simple"));
        auto* cl = topAlignedBody(card);
        auto* simple = new AntPagination();
        simple->setSimple(true);
        simple->setTotal(50);
        simple->setCurrent(1);
        cl->addWidget(simple);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createStepsPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = topAlignedBody(card);
        auto* basic = new AntSteps(page);
        basic->addStep(QStringLiteral("Finished"));
        basic->addStep(QStringLiteral("In Progress"));
        basic->addStep(QStringLiteral("Waiting"));
        basic->setCurrentIndex(1);
        cl->addWidget(basic);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Description"));
        auto* cl = topAlignedBody(card);
        auto* vertical = new AntSteps(page);
        vertical->setDirection(Ant::Orientation::Vertical);
        vertical->addStep(QStringLiteral("Finished"), QStringLiteral("This is a description."));
        vertical->addStep(QStringLiteral("In Progress"), QStringLiteral("This is a description."));
        vertical->addStep(QStringLiteral("Waiting"), QStringLiteral("This is a description."));
        vertical->setCurrentIndex(1);
        vertical->setMinimumHeight(280);
        cl->addWidget(vertical);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Error"));
        auto* cl = topAlignedBody(card);
        auto* error = new AntSteps(page);
        error->addStep(QStringLiteral("Finished"));
        error->addStep(QStringLiteral("Error"), QStringLiteral("This is an error."), QString(), Ant::StepStatus::Error);
        error->addStep(QStringLiteral("Waiting"));
        error->setCurrentIndex(1);
        cl->addWidget(error);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
