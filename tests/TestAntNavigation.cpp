#include <QSignalSpy>
#include <QTest>
#include "widgets/AntBreadcrumb.h"
#include "widgets/AntDropdown.h"
#include "widgets/AntMenu.h"
#include "widgets/AntPagination.h"
#include "widgets/AntSteps.h"
#include "widgets/AntTabs.h"
#include "widgets/AntAnchor.h"

class TestAntNavigation : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
};

void TestAntNavigation::propertiesAndSignals()
{
    // AntBreadcrumb
    auto* bc = new AntBreadcrumb;
    QCOMPARE(bc->separator(), QString("/"));
    QCOMPARE(bc->count(), 0);

    bc->addItem("Home", "/");
    QCOMPARE(bc->count(), 1);
    bc->addItem("Detail", "/detail");
    QCOMPARE(bc->count(), 2);

    QSignalSpy sepSpy(bc, &AntBreadcrumb::separatorChanged);
    bc->setSeparator(">");
    QCOMPARE(bc->separator(), ">");
    QCOMPARE(sepSpy.count(), 1);

    bc->clearItems();
    QCOMPARE(bc->count(), 0);

    // AntDropdown
    auto* dd = new AntDropdown;
    QCOMPARE(dd->placement(), Ant::DropdownPlacement::BottomLeft);
    QCOMPARE(dd->trigger(), Ant::DropdownTrigger::Hover);
    QCOMPARE(dd->arrowVisible(), false);
    QCOMPARE(dd->isOpen(), false);

    QSignalSpy placeSpy(dd, &AntDropdown::placementChanged);
    dd->setPlacement(Ant::DropdownPlacement::BottomRight);
    QCOMPARE(dd->placement(), Ant::DropdownPlacement::BottomRight);
    QCOMPARE(placeSpy.count(), 1);

    QSignalSpy trigSpy(dd, &AntDropdown::triggerChanged);
    dd->setTrigger(Ant::DropdownTrigger::Click);
    QCOMPARE(trigSpy.count(), 1);

    dd->setArrowVisible(false);
    QCOMPARE(dd->arrowVisible(), false);

    dd->addItem("key1", "Label 1");
    dd->addItem("key2", "Label 2");
    dd->clearItems();

    // AntMenu
    auto* menu = new AntMenu;
    QCOMPARE(menu->mode(), Ant::MenuMode::Vertical);
    QCOMPARE(menu->menuTheme(), Ant::MenuTheme::Light);
    QCOMPARE(menu->isInlineCollapsed(), false);
    QCOMPARE(menu->isSelectable(), true);

    QSignalSpy modeSpy(menu, &AntMenu::modeChanged);
    menu->setMode(Ant::MenuMode::Horizontal);
    QCOMPARE(menu->mode(), Ant::MenuMode::Horizontal);
    QCOMPARE(modeSpy.count(), 1);

    QSignalSpy themeSpy(menu, &AntMenu::menuThemeChanged);
    menu->setMenuTheme(Ant::MenuTheme::Dark);
    QCOMPARE(menu->menuTheme(), Ant::MenuTheme::Dark);
    QCOMPARE(themeSpy.count(), 1);

    QSignalSpy collapsedSpy(menu, &AntMenu::inlineCollapsedChanged);
    menu->setInlineCollapsed(true);
    QCOMPARE(menu->isInlineCollapsed(), true);
    QCOMPARE(collapsedSpy.count(), 1);

    menu->addItem("home", "Home");
    menu->addItem("about", "About");
    QSignalSpy selectedSpy(menu, &AntMenu::selectedKeyChanged);
    menu->setSelectedKey("home");
    QCOMPARE(menu->selectedKey(), "home");
    QCOMPARE(selectedSpy.count(), 1);

    menu->clearItems();

    // AntPagination
    auto* pag = new AntPagination;
    QCOMPARE(pag->current(), 1);
    QCOMPARE(pag->pageSize(), 10);
    QCOMPARE(pag->total(), 0);
    QCOMPARE(pag->isDisabled(), false);
    QCOMPARE(pag->isSimple(), false);

    pag->setTotal(100);
    QSignalSpy currentSpy(pag, &AntPagination::currentChanged);
    pag->setCurrent(3);
    QCOMPARE(pag->current(), 3);
    QCOMPARE(currentSpy.count(), 1);

    QSignalSpy pageSizeSpy(pag, &AntPagination::pageSizeChanged);
    pag->setPageSize(20);
    QCOMPARE(pag->pageSize(), 20);
    QCOMPARE(pageSizeSpy.count(), 1);

    QSignalSpy totalSpy(pag, &AntPagination::totalChanged);
    pag->setTotal(200);
    QCOMPARE(pag->total(), 200);
    QCOMPARE(totalSpy.count(), 1);

    QSignalSpy disSpy(pag, &AntPagination::disabledChanged);
    pag->setDisabled(true);
    QCOMPARE(pag->isDisabled(), true);
    QCOMPARE(disSpy.count(), 1);

    QSignalSpy simpleSpy(pag, &AntPagination::simpleChanged);
    pag->setSimple(true);
    QCOMPARE(pag->isSimple(), true);
    QCOMPARE(simpleSpy.count(), 1);

    // AntSteps
    auto* steps = new AntSteps;
    QCOMPARE(steps->currentIndex(), 0);
    QCOMPARE(steps->direction(), Ant::Orientation::Horizontal);
    QCOMPARE(steps->isClickable(), true);
    QCOMPARE(steps->count(), 0);

    steps->addStep("Step 1");
    steps->addStep("Step 2");
    steps->addStep("Step 3");
    QCOMPARE(steps->count(), 3);

    QSignalSpy idxSpy(steps, &AntSteps::currentIndexChanged);
    steps->setCurrentIndex(1);
    QCOMPARE(steps->currentIndex(), 1);
    QCOMPARE(idxSpy.count(), 1);

    QSignalSpy dirSpy(steps, &AntSteps::directionChanged);
    steps->setDirection(Ant::Orientation::Vertical);
    QCOMPARE(steps->direction(), Ant::Orientation::Vertical);
    QCOMPARE(dirSpy.count(), 1);

    QSignalSpy clickSpy(steps, &AntSteps::clickableChanged);
    steps->setClickable(false);
    QCOMPARE(steps->isClickable(), false);
    QCOMPARE(clickSpy.count(), 1);

    steps->clearSteps();
    QCOMPARE(steps->count(), 0);

    // AntTabs
    auto* tabs = new AntTabs;
    QCOMPARE(tabs->tabsType(), Ant::TabsType::Line);
    QCOMPARE(tabs->tabsSize(), Ant::Size::Middle);
    QCOMPARE(tabs->tabPlacement(), Ant::TabsPlacement::Top);
    QCOMPARE(tabs->isCentered(), false);
    QCOMPARE(tabs->isAnimated(), true);

    QSignalSpy typeSpy(tabs, &AntTabs::tabsTypeChanged);
    tabs->setTabsType(Ant::TabsType::Card);
    QCOMPARE(tabs->tabsType(), Ant::TabsType::Card);
    QCOMPARE(typeSpy.count(), 1);

    QSignalSpy tSizeSpy(tabs, &AntTabs::tabsSizeChanged);
    tabs->setTabsSize(Ant::Size::Large);
    QCOMPARE(tabs->tabsSize(), Ant::Size::Large);
    QCOMPARE(tSizeSpy.count(), 1);

    QSignalSpy placeSpy2(tabs, &AntTabs::tabPlacementChanged);
    tabs->setTabPlacement(Ant::TabsPlacement::Bottom);
    QCOMPARE(tabs->tabPlacement(), Ant::TabsPlacement::Bottom);
    QCOMPARE(placeSpy2.count(), 1);

    QSignalSpy centeredSpy(tabs, &AntTabs::centeredChanged);
    tabs->setCentered(true);
    QCOMPARE(tabs->isCentered(), true);
    QCOMPARE(centeredSpy.count(), 1);

    auto* page1 = new QWidget;
    auto* page2 = new QWidget;
    tabs->addTab(page1, "tab1", "Tab 1");
    tabs->addTab(page2, "tab2", "Tab 2");
    QCOMPARE(tabs->activeKey(), "tab1");

    QSignalSpy activeSpy(tabs, &AntTabs::activeKeyChanged);
    tabs->setActiveKey("tab2");
    QCOMPARE(tabs->activeKey(), "tab2");
    QCOMPARE(activeSpy.count(), 1);

    tabs->removeTab("tab1");
    tabs->clearTabs();

    // AntAnchor
    auto* anchor = new AntAnchor;
    QCOMPARE(anchor->activeIndex(), -1);

    anchor->addLink("Section 1", 0);
    anchor->addLink("Section 2", 100);
    QSignalSpy activeIdxSpy(anchor, &AntAnchor::activeIndexChanged);
    Q_UNUSED(activeIdxSpy);
}

QTEST_MAIN(TestAntNavigation)
#include "TestAntNavigation.moc"
