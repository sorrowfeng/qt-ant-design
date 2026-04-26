#include "ExampleWindow.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QStackedWidget>
#include <QString>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "core/AntTheme.h"
#include "core/AntTypes.h"
#include "pages/PageCommon.h"
#include "pages/PageRegistry.h"
#include "widgets/AntButton.h"
#include "widgets/AntScrollBar.h"
#include "widgets/AntTypography.h"
#include "widgets/AntWidget.h"

ExampleWindow::ExampleWindow(QWidget* parent)
    : AntWindow(parent)
{
    setWindowTitle(QStringLiteral("Ant Design Qt Widgets"));

    m_central = new AntWidget(this);
    auto* root = new QHBoxLayout(m_central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    buildSidebar();

    m_content = new AntWidget(m_central);
    auto* contentLayout = new QVBoxLayout(m_content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_stack = new QStackedWidget(m_content);
    contentLayout->addWidget(m_stack, 1);

    buildPages();

    root->addWidget(m_sidebar);
    root->addWidget(m_content, 1);
    setCentralWidget(m_central);
    m_stack->setCurrentIndex(0);

    connect(antTheme, &AntTheme::themeChanged, this, &ExampleWindow::applyTheme);
    applyTheme();
}

void ExampleWindow::buildSidebar()
{
    m_sidebar = new AntWidget(m_central);
    m_sidebar->setFixedWidth(220);
    auto* sideLayout = new QVBoxLayout(m_sidebar);
    sideLayout->setContentsMargins(20, 12, 20, 12);
    sideLayout->setSpacing(8);

    auto* brand = new AntTypography(QStringLiteral("qt-ant-design"), m_sidebar);
    brand->setTitle(true);
    brand->setTitleLevel(Ant::TypographyTitleLevel::H4);
    sideLayout->addWidget(brand);

    m_themeButton = new AntButton(QStringLiteral("Dark"), m_sidebar);
    m_themeButton->setButtonType(Ant::ButtonType::Default);
    m_themeButton->setButtonShape(Ant::ButtonShape::Round);
    m_themeButton->setButtonSize(Ant::ButtonSize::Small);
    connect(m_themeButton, &AntButton::clicked, antTheme, &AntTheme::toggleThemeMode);
    sideLayout->addWidget(m_themeButton);

    auto* navScroll = new QScrollArea(m_sidebar);
    navScroll->setWidgetResizable(true);
    navScroll->setFrameShape(QFrame::NoFrame);
    navScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    navScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    navScroll->setVerticalScrollBar(new AntScrollBar(Qt::Vertical));

    auto* navContainer = new AntWidget();
    m_navLayout = new QVBoxLayout(navContainer);
    m_navLayout->setContentsMargins(0, 0, 4, 0);
    m_navLayout->setSpacing(4);
    m_navLayout->addStretch();

    navScroll->setWidget(navContainer);
    sideLayout->addWidget(navScroll, 1);
}

void ExampleWindow::buildPages()
{
    const QVector<example::pages::PageEntry> registry = example::pages::buildPageRegistry();

    QString currentCategory;
    for (int i = 0; i < registry.size(); ++i)
    {
        const auto& entry = registry.at(i);
        m_stack->addWidget(example::pages::wrapPage(entry.factory(this)));

        if (entry.category != currentCategory)
        {
            addCategoryHeader(entry.category);
            currentCategory = entry.category;
        }
        addNavButton(entry.name, i);
    }
}

void ExampleWindow::addCategoryHeader(const QString& title)
{
    auto* header = new AntTypography(title);
    header->setType(Ant::TypographyType::Secondary);
    header->setStrong(true);
    m_navLayout->insertWidget(m_navLayout->count() - 1, header);
}

void ExampleWindow::addNavButton(const QString& text, int pageIndex)
{
    auto* button = new AntButton(text);
    button->setButtonType(Ant::ButtonType::Text);
    button->setBlock(true);
    connect(button, &AntButton::clicked, this, [this, pageIndex]() {
        m_stack->setCurrentIndex(pageIndex);
    });
    m_navLayout->insertWidget(m_navLayout->count() - 1, button);
}

QSize ExampleWindow::sizeHint() const
{
    return QSize(1200, 800);
}

void ExampleWindow::applyTheme()
{
    m_themeButton->setText(antTheme->themeMode() == Ant::ThemeMode::Dark
        ? QStringLiteral("Light") : QStringLiteral("Dark"));
}
