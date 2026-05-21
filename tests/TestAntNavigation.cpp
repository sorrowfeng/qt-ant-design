#include <QSignalSpy>
#include <QTest>
#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QMargins>
#include <QMouseEvent>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QStyle>
#include <QVBoxLayout>
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
    void dropdownCachesPopupGeometry();
    void menuCachesLayoutAndScopesRows();
    void paginationQuickJumperEditsCurrentPage();
    void paginationCachesButtonGeometry();
    void stepsCachesLayoutAndScopesUpdates();
    void tabsCachesLayoutAndScopesUpdates();
    void anchorCachesLayoutAndCoalescesScroll();
    void breadcrumbCachesLayoutAndScopesHover();
    void tabsNormalizePageLayoutMargins();
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
    QCOMPARE(dd->renderPlacement(), Ant::DropdownPlacement::BottomLeft);
    QCOMPARE(dd->trigger(), Ant::DropdownTrigger::Hover);
    QCOMPARE(dd->arrowVisible(), false);
    QCOMPARE(dd->isOpen(), false);

    QSignalSpy placeSpy(dd, &AntDropdown::placementChanged);
    dd->setPlacement(Ant::DropdownPlacement::BottomRight);
    QCOMPARE(dd->placement(), Ant::DropdownPlacement::BottomRight);
    QCOMPARE(dd->renderPlacement(), Ant::DropdownPlacement::BottomRight);
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
    menu->setMode(Ant::MenuMode::Inline);
    menu->setInlineCollapsed(false);
    QSignalSpy openSpy(menu, &AntMenu::openKeysChanged);
    menu->addSubMenu("sub", "Navigation");
    menu->addSubItem("sub", "s1", "Sub 1");
    QCOMPARE(menu->openKeys(), QStringList());
    menu->setOpenKeys(QStringList{"sub"});
    QCOMPARE(menu->openKeys(), QStringList{"sub"});
    QCOMPARE(openSpy.count(), 1);
    menu->clearItems();

    auto* actionMenu = new AntMenu;
    auto* openAction = new QAction(QStringLiteral("&Open"), actionMenu);
    openAction->setData(QStringLiteral("open"));
    openAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    actionMenu->addAction(openAction);
    QCOMPARE(actionMenu->itemCount(), 1);
    QCOMPARE(actionMenu->itemAt(0).key, QStringLiteral("open"));
    QCOMPARE(actionMenu->itemAt(0).label, QStringLiteral("Open"));
    QCOMPARE(actionMenu->itemAt(0).extra, openAction->shortcut().toString(QKeySequence::NativeText));

    openAction->setText(QStringLiteral("&Open File"));
    openAction->setEnabled(false);
    QCOMPARE(actionMenu->itemAt(0).label, QStringLiteral("Open File"));
    QCOMPARE(actionMenu->itemAt(0).disabled, true);

    openAction->setEnabled(true);
    actionMenu->resize(actionMenu->sizeHint());
    QSignalSpy openTriggeredSpy(openAction, &QAction::triggered);
    QSignalSpy actionItemSpy(actionMenu, &AntMenu::itemClicked);
    QTest::mouseClick(actionMenu, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QCOMPARE(openTriggeredSpy.count(), 1);
    QCOMPARE(actionItemSpy.count(), 1);
    QCOMPARE(actionItemSpy.takeFirst().at(0).toString(), QStringLiteral("open"));

    actionMenu->removeAction(openAction);
    QCOMPARE(actionMenu->itemCount(), 0);

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
    QSignalSpy tabCurrentSpy(tabs, &AntTabs::currentChanged);
    tabs->setTabEnabled("tab2", false);
    QCOMPARE(tabs->activeKey(), "tab1");
    QCOMPARE(activeSpy.count(), 2);
    QCOMPARE(tabCurrentSpy.count(), 1);
    QCOMPARE(tabCurrentSpy.takeFirst().at(0).toInt(), 0);
    tabs->setActiveKey("tab2");
    QCOMPARE(tabs->activeKey(), "tab1");
    QCOMPARE(activeSpy.count(), 2);
    tabs->setTabEnabled("tab2", true);
    tabs->setActiveKey("tab2");
    QCOMPARE(tabs->activeKey(), "tab2");
    QCOMPARE(activeSpy.count(), 3);

    auto* lineTabs = new AntTabs;
    lineTabs->resize(320, 120);
    lineTabs->setAnimated(false);
    lineTabs->addTab(new QWidget, "one", "Tab 1");
    lineTabs->addTab(new QWidget, "two", "Tab 2");
    const QRectF firstIndicator = lineTabs->indicatorRect();
    QVERIFY(!firstIndicator.isNull());
    lineTabs->setActiveKey("two");
    QVERIFY(lineTabs->indicatorRect().left() > firstIndicator.left());
    QCOMPARE(lineTabs->indicatorRect().height(), 2.0);
    lineTabs->setActiveKey("one");
    QCOMPARE(lineTabs->activeKey(), "one");
    lineTabs->setTabEnabled("one", false);
    QCOMPARE(lineTabs->activeKey(), "two");

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

void TestAntNavigation::dropdownCachesPopupGeometry()
{
    QWidget host;
    host.resize(420, 260);
    QPushButton target(QStringLiteral("Actions"), &host);
    target.resize(120, 32);
    target.move(40, 40);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    AntDropdown dropdown(&host);
    dropdown.setTarget(&target);
    dropdown.setTrigger(Ant::DropdownTrigger::Click);
    dropdown.addItem(QStringLiteral("download"), QStringLiteral("Download"));
    dropdown.addItem(QStringLiteral("archive"), QStringLiteral("Archive"));

    dropdown.setOpen(true);
    QTRY_VERIFY_WITH_TIMEOUT(dropdown.isOpen(), 300);
    QTRY_VERIFY_WITH_TIMEOUT(dropdown.menu()->isVisible(), 300);
    QVERIFY(dropdown.property("antDropdownGeometryApplyCount").toInt() > 0);
    QVERIFY(dropdown.property("antDropdownContentWidthCacheMissCount").toInt() > 0);

    const int contentHitsBefore = dropdown.property("antDropdownContentWidthCacheHitCount").toInt();
    const int geometrySkipsBefore = dropdown.property("antDropdownGeometrySkipCount").toInt();
    QResizeEvent sameResize(target.size(), target.size());
    QCoreApplication::sendEvent(&target, &sameResize);
    QVERIFY(dropdown.property("antDropdownContentWidthCacheHitCount").toInt() > contentHitsBefore);
    QVERIFY(dropdown.property("antDropdownGeometrySkipCount").toInt() > geometrySkipsBefore);

    const int marginAppliesBefore = dropdown.property("antDropdownMarginApplyCount").toInt();
    dropdown.setArrowVisible(true);
    QVERIFY(dropdown.property("antDropdownMarginApplyCount").toInt() > marginAppliesBefore);

    const int contentMissesBefore = dropdown.property("antDropdownContentWidthCacheMissCount").toInt();
    dropdown.addItem(QStringLiteral("settings"), QStringLiteral("Longer settings label"));
    QVERIFY(dropdown.property("antDropdownContentWidthCacheMissCount").toInt() > contentMissesBefore);

    dropdown.setOpen(false);
    QTRY_VERIFY_WITH_TIMEOUT(!dropdown.isOpen(), 300);
}

void TestAntNavigation::menuCachesLayoutAndScopesRows()
{
    AntMenu menu;
    menu.addItem(QStringLiteral("home"), QStringLiteral("Home"));
    menu.addItem(QStringLiteral("about"), QStringLiteral("About"));
    menu.addSubMenu(QStringLiteral("more"), QStringLiteral("More"));
    menu.addSubItem(QStringLiteral("more"), QStringLiteral("settings"), QStringLiteral("Settings"));
    menu.resize(240, 160);
    menu.show();
    QVERIFY(QTest::qWaitForWindowExposed(&menu));

    (void)menu.sizeHint();
    const int cacheHitsBefore = menu.property("antMenuVisibleLayoutCacheHitCount").toInt();
    (void)menu.sizeHint();
    QVERIFY(menu.property("antMenuVisibleLayoutCacheHitCount").toInt() > cacheHitsBefore);

    const int scopedBeforeHover = menu.property("antMenuScopedItemUpdateCount").toInt();
    QMouseEvent firstHover(QEvent::MouseMove,
                           QPointF(20, 20),
                           QPointF(menu.mapToGlobal(QPoint(20, 20))),
                           Qt::NoButton,
                           Qt::NoButton,
                           Qt::NoModifier);
    QCoreApplication::sendEvent(&menu, &firstHover);
    QVERIFY(menu.property("antMenuScopedItemUpdateCount").toInt() > scopedBeforeHover);

    const int scopedBeforeSecondHover = menu.property("antMenuScopedItemUpdateCount").toInt();
    QMouseEvent secondHover(QEvent::MouseMove,
                            QPointF(20, 60),
                            QPointF(menu.mapToGlobal(QPoint(20, 60))),
                            Qt::NoButton,
                            Qt::NoButton,
                            Qt::NoModifier);
    QCoreApplication::sendEvent(&menu, &secondHover);
    QVERIFY(menu.property("antMenuScopedItemUpdateCount").toInt() > scopedBeforeSecondHover);

    const int buildsBeforeSelection = menu.property("antMenuVisibleLayoutBuildCount").toInt();
    const int scopedBeforeSelection = menu.property("antMenuScopedItemUpdateCount").toInt();
    menu.setSelectedKey(QStringLiteral("about"));
    QCOMPARE(menu.property("antMenuVisibleLayoutBuildCount").toInt(), buildsBeforeSelection);
    QVERIFY(menu.property("antMenuScopedItemUpdateCount").toInt() > scopedBeforeSelection);

    AntMenu actionMenu;
    auto* action = new QAction(QStringLiteral("&Open"), &actionMenu);
    action->setData(QStringLiteral("open"));
    actionMenu.addAction(action);
    actionMenu.resize(240, 80);
    (void)actionMenu.sizeHint();

    const int actionBuildsBefore = actionMenu.property("antMenuVisibleLayoutBuildCount").toInt();
    const int actionScopedBefore = actionMenu.property("antMenuScopedItemUpdateCount").toInt();
    action->setEnabled(false);
    QCOMPARE(actionMenu.property("antMenuVisibleLayoutBuildCount").toInt(), actionBuildsBefore);
    QVERIFY(actionMenu.property("antMenuScopedItemUpdateCount").toInt() > actionScopedBefore);
}

void TestAntNavigation::paginationQuickJumperEditsCurrentPage()
{
    AntPagination pagination;
    pagination.setTotal(200);
    pagination.setPageSize(10);
    pagination.setShowQuickJumper(true);
    pagination.resize(pagination.sizeHint());
    pagination.show();
    QVERIFY(QTest::qWaitForWindowExposed(&pagination));

    auto* jumper = pagination.findChild<QLineEdit*>(QStringLiteral("AntPaginationQuickJumper"));
    QVERIFY(jumper);
    QTRY_VERIFY(jumper->isVisible());

    QSignalSpy currentSpy(&pagination, &AntPagination::currentChanged);
    QSignalSpy changeSpy(&pagination, &AntPagination::change);

    QTest::mouseClick(jumper, Qt::LeftButton);
    QTest::keyClicks(jumper, "12");
    QTest::keyClick(jumper, Qt::Key_Return);

    QCOMPARE(pagination.current(), 12);
    QCOMPARE(currentSpy.count(), 1);
    QCOMPARE(changeSpy.count(), 1);

    jumper->clear();
    QTest::keyClicks(jumper, "999");
    QTest::keyClick(jumper, Qt::Key_Return);

    QCOMPARE(pagination.current(), pagination.pageCount());
}

void TestAntNavigation::paginationCachesButtonGeometry()
{
    AntPagination pagination;
    pagination.setTotal(200);
    pagination.setCurrent(5);
    pagination.resize(pagination.sizeHint());
    pagination.show();
    QVERIFY(QTest::qWaitForWindowExposed(&pagination));

    (void)pagination.sizeHint();
    const int cacheHitsBefore = pagination.property("antPaginationPageItemsCacheHitCount").toInt();
    (void)pagination.sizeHint();
    QVERIFY(pagination.property("antPaginationPageItemsCacheHitCount").toInt() > cacheHitsBefore);

    const int scopedBeforeHover = pagination.property("antPaginationScopedItemUpdateCount").toInt();
    QMouseEvent firstHover(QEvent::MouseMove,
                           QPointF(20, pagination.height() / 2.0),
                           QPointF(pagination.mapToGlobal(QPoint(20, pagination.height() / 2))),
                           Qt::NoButton,
                           Qt::NoButton,
                           Qt::NoModifier);
    QCoreApplication::sendEvent(&pagination, &firstHover);
    QVERIFY(pagination.property("antPaginationScopedItemUpdateCount").toInt() > scopedBeforeHover);

    const int scopedBeforeSameHover = pagination.property("antPaginationScopedItemUpdateCount").toInt();
    QCoreApplication::sendEvent(&pagination, &firstHover);
    QCOMPARE(pagination.property("antPaginationScopedItemUpdateCount").toInt(), scopedBeforeSameHover);

    const int scopedBeforeSecondHover = pagination.property("antPaginationScopedItemUpdateCount").toInt();
    QMouseEvent secondHover(QEvent::MouseMove,
                            QPointF(56, pagination.height() / 2.0),
                            QPointF(pagination.mapToGlobal(QPoint(56, pagination.height() / 2))),
                            Qt::NoButton,
                            Qt::NoButton,
                            Qt::NoModifier);
    QCoreApplication::sendEvent(&pagination, &secondHover);
    QVERIFY(pagination.property("antPaginationScopedItemUpdateCount").toInt() > scopedBeforeSecondHover);

    const int buildsBeforeCurrent = pagination.property("antPaginationPageItemsBuildCount").toInt();
    pagination.setCurrent(6);
    QVERIFY(pagination.property("antPaginationPageItemsBuildCount").toInt() > buildsBeforeCurrent);

    const int buildsBeforeQuickJumper = pagination.property("antPaginationPageItemsBuildCount").toInt();
    pagination.setShowQuickJumper(true);
    pagination.resize(pagination.sizeHint());
    QVERIFY(pagination.property("antPaginationPageItemsBuildCount").toInt() > buildsBeforeQuickJumper);

    auto* jumper = pagination.findChild<QLineEdit*>(QStringLiteral("AntPaginationQuickJumper"));
    QVERIFY(jumper);
    QTRY_VERIFY(jumper->isVisible());
}

void TestAntNavigation::stepsCachesLayoutAndScopesUpdates()
{
    AntSteps steps;
    steps.addStep(QStringLiteral("First"), QStringLiteral("Prepare"));
    steps.addStep(QStringLiteral("Second"), QStringLiteral("Run"));
    steps.addStep(QStringLiteral("Third"), QStringLiteral("Review"));
    steps.resize(480, steps.sizeHint().height());
    steps.show();
    QVERIFY(QTest::qWaitForWindowExposed(&steps));
    QCoreApplication::processEvents();

    QVERIFY(steps.property("antStepsLayoutBuildCount").toInt() > 0);
    const int cacheHitsBefore = steps.property("antStepsLayoutCacheHitCount").toInt();
    QMouseEvent firstHover(QEvent::MouseMove,
                           QPointF(20, 20),
                           QPointF(steps.mapToGlobal(QPoint(20, 20))),
                           Qt::NoButton,
                           Qt::NoButton,
                           Qt::NoModifier);
    QCoreApplication::sendEvent(&steps, &firstHover);
    QVERIFY(steps.property("antStepsLayoutCacheHitCount").toInt() > cacheHitsBefore);

    const int scopedBeforeSameHover = steps.property("antStepsScopedStepUpdateCount").toInt();
    QCoreApplication::sendEvent(&steps, &firstHover);
    QCOMPARE(steps.property("antStepsScopedStepUpdateCount").toInt(), scopedBeforeSameHover);

    const int scopedBeforeSecondHover = steps.property("antStepsScopedStepUpdateCount").toInt();
    QMouseEvent secondHover(QEvent::MouseMove,
                            QPointF(180, 20),
                            QPointF(steps.mapToGlobal(QPoint(180, 20))),
                            Qt::NoButton,
                            Qt::NoButton,
                            Qt::NoModifier);
    QCoreApplication::sendEvent(&steps, &secondHover);
    QVERIFY(steps.property("antStepsScopedStepUpdateCount").toInt() > scopedBeforeSecondHover);

    const int buildsBeforeCurrent = steps.property("antStepsLayoutBuildCount").toInt();
    const int scopedBeforeCurrent = steps.property("antStepsScopedStepUpdateCount").toInt();
    steps.setCurrentIndex(2);
    QCOMPARE(steps.property("antStepsLayoutBuildCount").toInt(), buildsBeforeCurrent);
    QVERIFY(steps.property("antStepsScopedStepUpdateCount").toInt() > scopedBeforeCurrent);

    const int scopedBeforeStatus = steps.property("antStepsScopedStepUpdateCount").toInt();
    steps.setStepStatus(1, Ant::StepStatus::Error);
    QVERIFY(steps.property("antStepsScopedStepUpdateCount").toInt() > scopedBeforeStatus);

    const int buildsBeforeDirection = steps.property("antStepsLayoutBuildCount").toInt();
    steps.setDirection(Ant::Orientation::Vertical);
    QMouseEvent verticalHover(QEvent::MouseMove,
                              QPointF(20, 90),
                              QPointF(steps.mapToGlobal(QPoint(20, 90))),
                              Qt::NoButton,
                              Qt::NoButton,
                              Qt::NoModifier);
    QCoreApplication::sendEvent(&steps, &verticalHover);
    QVERIFY(steps.property("antStepsLayoutBuildCount").toInt() > buildsBeforeDirection);
}

void TestAntNavigation::tabsCachesLayoutAndScopesUpdates()
{
    AntTabs tabs;
    tabs.addTab(new QWidget, QStringLiteral("one"), QStringLiteral("One"));
    tabs.addTab(new QWidget, QStringLiteral("two"), QStringLiteral("Two"));
    tabs.addTab(new QWidget, QStringLiteral("three"), QStringLiteral("Three"));
    tabs.resize(360, 160);
    tabs.show();
    QVERIFY(QTest::qWaitForWindowExposed(&tabs));
    QCoreApplication::processEvents();

    QVERIFY(tabs.property("antTabsTabLayoutBuildCount").toInt() > 0);
    const int cacheHitsBeforeHover = tabs.property("antTabsTabLayoutCacheHitCount").toInt();
    const int scopedBeforeHover = tabs.property("antTabsScopedUpdateCount").toInt();
    QMouseEvent firstHover(QEvent::MouseMove,
                           QPointF(10, 20),
                           QPointF(tabs.mapToGlobal(QPoint(10, 20))),
                           Qt::NoButton,
                           Qt::NoButton,
                           Qt::NoModifier);
    QCoreApplication::sendEvent(&tabs, &firstHover);
    QVERIFY(tabs.property("antTabsTabLayoutCacheHitCount").toInt() > cacheHitsBeforeHover);
    QVERIFY(tabs.property("antTabsScopedUpdateCount").toInt() > scopedBeforeHover);

    const int scopedBeforeSameHover = tabs.property("antTabsScopedUpdateCount").toInt();
    QCoreApplication::sendEvent(&tabs, &firstHover);
    QCOMPARE(tabs.property("antTabsScopedUpdateCount").toInt(), scopedBeforeSameHover);

    const int buildsBeforeActive = tabs.property("antTabsTabLayoutBuildCount").toInt();
    const int scopedBeforeActive = tabs.property("antTabsScopedUpdateCount").toInt();
    tabs.setAnimated(false);
    tabs.setActiveKey(QStringLiteral("three"));
    QCOMPARE(tabs.property("antTabsTabLayoutBuildCount").toInt(), buildsBeforeActive);
    QVERIFY(tabs.property("antTabsScopedUpdateCount").toInt() > scopedBeforeActive);

    const int buildsBeforeLabel = tabs.property("antTabsTabLayoutBuildCount").toInt();
    tabs.setTabText(QStringLiteral("three"), QStringLiteral("Longer third tab"));
    QVERIFY(tabs.property("antTabsTabLayoutBuildCount").toInt() > buildsBeforeLabel);

    AntTabs editable;
    editable.setTabsType(Ant::TabsType::EditableCard);
    editable.addTab(new QWidget, QStringLiteral("one"), QStringLiteral("One"));
    editable.addTab(new QWidget, QStringLiteral("two"), QStringLiteral("Two"));
    editable.addTab(new QWidget, QStringLiteral("three"), QStringLiteral("Three"));
    editable.resize(360, 160);
    editable.show();
    QVERIFY(QTest::qWaitForWindowExposed(&editable));
    QCoreApplication::processEvents();

    const int editableBuilds = editable.property("antTabsTabLayoutBuildCount").toInt();
    const int editableScopedBeforeHover = editable.property("antTabsScopedUpdateCount").toInt();
    QMouseEvent addHover(QEvent::MouseMove,
                         QPointF(245, 20),
                         QPointF(editable.mapToGlobal(QPoint(245, 20))),
                         Qt::NoButton,
                         Qt::NoButton,
                         Qt::NoModifier);
    QCoreApplication::sendEvent(&editable, &addHover);
    QCOMPARE(editable.property("antTabsTabLayoutBuildCount").toInt(), editableBuilds);
    QVERIFY(editable.property("antTabsScopedUpdateCount").toInt() > editableScopedBeforeHover);
}

void TestAntNavigation::anchorCachesLayoutAndCoalescesScroll()
{
    AntAnchor anchor;
    anchor.addLink(QStringLiteral("Section 1"), 0);
    anchor.addLink(QStringLiteral("Section 2"), 100);
    anchor.addLink(QStringLiteral("Section 3"), 200);
    anchor.resize(160, 120);
    anchor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&anchor));

    const auto labels = anchor.findChildren<QLabel*>();
    QCOMPARE(labels.size(), 3);

    QTest::mouseClick(labels.at(0), Qt::LeftButton);
    QCOMPARE(anchor.activeIndex(), 0);
    QVERIFY(anchor.property("antAnchorLinkRectCacheMissCount").toInt() > 0);

    const int cacheHitsBefore = anchor.property("antAnchorLinkRectCacheHitCount").toInt();
    QTest::mouseClick(labels.at(1), Qt::LeftButton);
    QCOMPARE(anchor.activeIndex(), 1);
    QVERIFY(anchor.property("antAnchorLinkRectCacheHitCount").toInt() > cacheHitsBefore);

    const int labelVisualApplies = anchor.property("antAnchorLabelVisualApplyCount").toInt();
    anchor.update();
    QCoreApplication::processEvents();
    QCOMPARE(anchor.property("antAnchorLabelVisualApplyCount").toInt(), labelVisualApplies);

    QScrollArea area;
    auto* content = new QWidget;
    content->resize(100, 500);
    area.setWidget(content);
    area.resize(100, 120);
    area.show();
    QVERIFY(QTest::qWaitForWindowExposed(&area));

    AntAnchor scrollAnchor;
    scrollAnchor.addLink(QStringLiteral("Intro"), 0);
    scrollAnchor.addLink(QStringLiteral("Middle"), 100);
    scrollAnchor.addLink(QStringLiteral("End"), 200);
    scrollAnchor.setScrollArea(&area);
    QCoreApplication::processEvents();

    const int scrollResolvesBefore = scrollAnchor.property("antAnchorScrollResolveCount").toInt();
    area.verticalScrollBar()->setValue(40);
    area.verticalScrollBar()->setValue(140);
    area.verticalScrollBar()->setValue(240);
    QTRY_COMPARE(scrollAnchor.activeIndex(), 2);
    QCOMPARE(scrollAnchor.property("antAnchorScrollResolveCount").toInt(), scrollResolvesBefore + 1);
}

void TestAntNavigation::breadcrumbCachesLayoutAndScopesHover()
{
    AntBreadcrumb breadcrumb;
    breadcrumb.addItem(QStringLiteral("Home"), QStringLiteral("/"));
    breadcrumb.addItem(QStringLiteral("Library"), QStringLiteral("/library"));
    breadcrumb.addItem(QStringLiteral("Data"));
    breadcrumb.resize(240, breadcrumb.sizeHint().height());
    breadcrumb.show();
    QVERIFY(QTest::qWaitForWindowExposed(&breadcrumb));

    const int missesBefore = breadcrumb.property("antBreadcrumbLayoutCacheMissCount").toInt();
    const QSize firstHint = breadcrumb.sizeHint();
    QVERIFY(firstHint.width() > 0);
    QVERIFY(breadcrumb.property("antBreadcrumbLayoutCacheMissCount").toInt() >= missesBefore);

    const int hitsBefore = breadcrumb.property("antBreadcrumbLayoutCacheHitCount").toInt();
    QCOMPARE(breadcrumb.sizeHint(), firstHint);
    QVERIFY(breadcrumb.property("antBreadcrumbLayoutCacheHitCount").toInt() > hitsBefore);

    const int hoverUpdatesBefore = breadcrumb.property("antBreadcrumbHoverScopedUpdateCount").toInt();
    QMouseEvent hoverMove(QEvent::MouseMove,
                          QPointF(16, breadcrumb.height() / 2),
                          Qt::NoButton,
                          Qt::NoButton,
                          Qt::NoModifier);
    QCoreApplication::sendEvent(&breadcrumb, &hoverMove);
    QVERIFY(breadcrumb.property("antBreadcrumbHoverScopedUpdateCount").toInt() > hoverUpdatesBefore);

    const int missesBeforeSeparator = breadcrumb.property("antBreadcrumbLayoutCacheMissCount").toInt();
    breadcrumb.setSeparator(QStringLiteral(">"));
    (void)breadcrumb.sizeHint();
    QVERIFY(breadcrumb.property("antBreadcrumbLayoutCacheMissCount").toInt() > missesBeforeSeparator);
}

void TestAntNavigation::tabsNormalizePageLayoutMargins()
{
    AntTabs tabs;
    const QMargins zeroMargins(0, 0, 0, 0);

    auto styleMarginsFor = [](QWidget* widget) {
        QStyle* style = widget->style();
        return QMargins(style->pixelMetric(QStyle::PM_LayoutLeftMargin, nullptr, widget),
                        style->pixelMetric(QStyle::PM_LayoutTopMargin, nullptr, widget),
                        style->pixelMetric(QStyle::PM_LayoutRightMargin, nullptr, widget),
                        style->pixelMetric(QStyle::PM_LayoutBottomMargin, nullptr, widget));
    };

    auto* defaultPage = new QWidget;
    auto* defaultLayout = new QVBoxLayout(defaultPage);
    defaultLayout->setContentsMargins(styleMarginsFor(defaultPage));
    QVERIFY(defaultLayout->contentsMargins() != zeroMargins);

    QCOMPARE(tabs.addTab(defaultPage, QStringLiteral("default"), QStringLiteral("Default")), 0);
    QCOMPARE(defaultLayout->contentsMargins(), zeroMargins);

    auto* customPage = new QWidget;
    auto* customLayout = new QVBoxLayout(customPage);
    const QMargins customMargins(21, 22, 23, 24);
    customLayout->setContentsMargins(customMargins);

    QCOMPARE(tabs.addTab(customPage, QStringLiteral("custom"), QStringLiteral("Custom")), 1);
    QCOMPARE(customLayout->contentsMargins(), customMargins);

    AntTabs::useTabContentLayout(customPage);
    QCOMPARE(customLayout->contentsMargins(), zeroMargins);

    auto* detachedLayout = new QVBoxLayout;
    detachedLayout->setContentsMargins(8, 9, 10, 11);
    AntTabs::useTabContentLayout(detachedLayout);
    QCOMPARE(detachedLayout->contentsMargins(), zeroMargins);
    delete detachedLayout;
}

QTEST_MAIN(TestAntNavigation)
#include "TestAntNavigation.moc"
