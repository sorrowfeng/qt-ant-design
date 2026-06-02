#include "ExampleWindow.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QScrollArea>
#include <QString>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"
#include "pages/PageCommon.h"
#include "pages/PageRegistry.h"
#include "widgets/AntImage.h"
#include "widgets/AntNav.h"
#include "widgets/AntStackedWidget.h"
#include "widgets/AntTypography.h"
#include "widgets/AntWidget.h"

namespace
{
QString exampleLogoResource()
{
    return QStringLiteral(":/qt-ant-design-example/logo.png");
}
}

ExampleWindow::ExampleWindow(QWidget* parent)
    : AntWindow(parent)
{
    setWindowTitle(QStringLiteral("Ant Design Qt Widgets"));
    setWindowIcon(QIcon(exampleLogoResource()));
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
    m_content->setObjectName(QStringLiteral("ExampleContent"));
    auto* contentLayout = new QVBoxLayout(m_content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_stack = new AntStackedWidget(m_content);
    m_stack->setObjectName(QStringLiteral("ExamplePageStack"));
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
    m_sidebar->setObjectName(QStringLiteral("ExampleSidebar"));
    m_sidebar->setFixedWidth(220);

    auto* sideLayout = new QVBoxLayout(m_sidebar);
    sideLayout->setSpacing(0);

    auto* brandArea = new QWidget(m_sidebar);
    auto* brandLayout = new QHBoxLayout(brandArea);
    brandLayout->setContentsMargins(8, 12, 8, 12);
    brandLayout->setSpacing(6);

    auto* logo = new AntImage(brandArea);
    logo->setObjectName(QStringLiteral("ExampleBrandLogo"));
    logo->setSrc(exampleLogoResource());
    logo->setAlt(QStringLiteral("qt-ant-design"));
    logo->setPreview(false);
    logo->setImgWidth(24);
    logo->setImgHeight(24);
    logo->setFixedSize(24, 24);
    brandLayout->addWidget(logo);

    auto* brand = new AntTypography(QStringLiteral("qt-ant-design"), brandArea);
    brand->setPixelSize(14);
    brand->setWordWrap(false);
    brand->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    brandLayout->addWidget(brand, 1);

    sideLayout->addWidget(brandArea);

    m_nav = new AntNav(m_sidebar);
    m_nav->setObjectName(QStringLiteral("ExampleNavigation"));
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
        m_pageNames.append(entry.name);

        if (entry.category != currentCategory)
        {
            m_nav->addCategory(entry.category);
            currentCategory = entry.category;
        }
        m_nav->addItem(entry.name, i);
    }
}

int ExampleWindow::examplePageCount() const
{
    return m_stack ? m_stack->count() : 0;
}

QString ExampleWindow::examplePageName(int index) const
{
    return index >= 0 && index < m_pageNames.size() ? m_pageNames.at(index) : QString();
}

QWidget* ExampleWindow::examplePageWidget(int index) const
{
    return m_stack && index >= 0 && index < m_stack->count() ? m_stack->widget(index) : nullptr;
}

bool ExampleWindow::setExamplePageIndex(int index)
{
    if (!m_stack || !m_nav || index < 0 || index >= m_stack->count())
    {
        return false;
    }

    m_nav->setCurrentIndex(index);
    if (m_stack->currentIndex() != index)
    {
        m_stack->setCurrentIndex(index);
    }
    return m_stack->currentIndex() == index;
}

QSize ExampleWindow::sizeHint() const
{
    return QSize(1200, 800);
}
