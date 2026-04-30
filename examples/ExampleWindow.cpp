#include "ExampleWindow.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
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
#include "widgets/AntNavItem.h"
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
    sideLayout->setSpacing(0);

    auto* brandArea = new QWidget(m_sidebar);
    auto* brandLayout = new QVBoxLayout(brandArea);
    brandLayout->setContentsMargins(20, 16, 20, 16);
    brandLayout->setSpacing(8);

    auto* brand = new AntTypography(QStringLiteral("qt-ant-design"), brandArea);
    brand->setTitle(true);
    brand->setTitleLevel(Ant::TypographyTitleLevel::H4);
    brandLayout->addWidget(brand);

    m_themeButton = new AntButton(QStringLiteral("Dark"), brandArea);
    m_themeButton->setButtonType(Ant::ButtonType::Default);
    m_themeButton->setButtonShape(Ant::ButtonShape::Round);
    m_themeButton->setButtonSize(Ant::Size::Small);
    connect(m_themeButton, &AntButton::clicked, antTheme, &AntTheme::toggleThemeMode);
    brandLayout->addWidget(m_themeButton);

    sideLayout->addWidget(brandArea);

    auto* navScroll = new QScrollArea(m_sidebar);
    navScroll->setWidgetResizable(true);
    navScroll->setFrameShape(QFrame::NoFrame);
    navScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    navScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    navScroll->setVerticalScrollBar(new AntScrollBar(Qt::Vertical));

    auto* navContainer = new QWidget();
    m_navLayout = new QVBoxLayout(navContainer);
    m_navLayout->setContentsMargins(0, 8, 0, 0);
    m_navLayout->setSpacing(0);
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
    auto* header = new AntTypography(title.toUpper());
    header->setTitle(true);
    header->setTitleLevel(Ant::TypographyTitleLevel::H5);
    header->setContentsMargins(20, 12, 20, 4);
    m_navLayout->insertWidget(m_navLayout->count() - 1, header);
}

void ExampleWindow::addNavButton(const QString& text, int pageIndex)
{
    auto* item = new AntNavItem(text);
    m_navLayout->insertWidget(m_navLayout->count() - 1, item);
    m_navItems.append(item);

    connect(item, &AntNavItem::clicked, this, [this, item]() {
        int idx = m_navItems.indexOf(item);
        if (idx >= 0)
        {
            m_stack->setCurrentIndex(idx);
            setActiveNav(idx);
        }
    });

    connect(item, &QObject::destroyed, this, [this, item]() {
        m_navItems.removeOne(item);
    });

    if (pageIndex == 0)
    {
        setActiveNav(0);
    }
}

void ExampleWindow::setActiveNav(int index)
{
    if (index < 0 || index >= m_navItems.size())
    {
        return;
    }

    for (int i = 0; i < m_navItems.size(); ++i)
    {
        m_navItems[i]->setActive(i == index);
    }
    m_activeIndex = index;
}

QSize ExampleWindow::sizeHint() const
{
    return QSize(1200, 800);
}

void ExampleWindow::applyTheme()
{
    const bool isDark = antTheme->themeMode() == Ant::ThemeMode::Dark;

    m_themeButton->setText(isDark ? QStringLiteral("Light") : QStringLiteral("Dark"));

    setActiveNav(m_activeIndex);
}
