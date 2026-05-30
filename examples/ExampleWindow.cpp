#include "ExampleWindow.h"

#include <QHBoxLayout>
#include <QScrollArea>
#include <QString>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"
#include "pages/PageCommon.h"
#include "pages/PageRegistry.h"
#include "widgets/AntNav.h"
#include "widgets/AntStackedWidget.h"
#include "widgets/AntTypography.h"
#include "widgets/AntWidget.h"

ExampleWindow::ExampleWindow(QWidget* parent)
    : AntWindow(parent)
{
    setWindowTitle(QStringLiteral("Ant Design Qt Widgets"));
    setPinButtonVisible(true);
    setThemeButtonVisible(true);
    setMinimizeButtonVisible(true);
    setMaximizeButtonVisible(true);
    setCloseButtonVisible(true);

    m_central = new AntWidget(this);
    auto* root = new QHBoxLayout(m_central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    buildSidebar();

    m_content = new AntWidget(m_central);
    auto* contentLayout = new QVBoxLayout(m_content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_stack = new AntStackedWidget(m_content);
    m_stack->setVariant(Ant::Variant::Borderless);
    contentLayout->addWidget(m_stack, 1);

    connect(m_nav, &AntNav::currentIndexChanged, this, [this](int index) {
        if (m_stack && index >= 0 && index < m_stack->count())
        {
            m_stack->setCurrentIndex(index);
        }
    });

    buildPages();

    root->addWidget(m_sidebar);
    root->addWidget(m_content, 1);
    setCentralWidget(m_central);
    m_stack->setCurrentIndex(0);
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

    sideLayout->addWidget(brandArea);

    m_nav = new AntNav(m_sidebar);
    sideLayout->addWidget(m_nav, 1);
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
            m_nav->addCategory(entry.category);
            currentCategory = entry.category;
        }
        m_nav->addItem(entry.name, i);
    }
}

QSize ExampleWindow::sizeHint() const
{
    return QSize(1200, 800);
}
