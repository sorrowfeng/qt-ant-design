#include <QSignalSpy>
#include <QTest>
#include <QAbstractItemView>
#include <QColor>
#include <QCoreApplication>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>
#include "core/AntTheme.h"
#include "widgets/AntList.h"
#include "widgets/AntListWidget.h"
#include "widgets/AntTable.h"
#include "widgets/AntTree.h"
#include "widgets/AntTimeline.h"
#include "widgets/AntDescriptions.h"
#include "widgets/AntQRCode.h"
#include "widgets/AntWatermark.h"
#include "widgets/AntCarousel.h"
#include "widgets/AntCollapse.h"

class TestAntDataDisplayB : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
    void listWidgetCompatibilityApis();
    void listBulkInsertionCoalescesLayout();
    void listInternalScrolling();
    void listUsesExpandingLayoutPolicy();
    void treeCachesFlattenedVisibleNodes();
    void tableHoverUsesRowScopedUpdates();
    void tableSelectionCompatibilityApis();
    void carouselPausesAutoplayAndScopesTransitionWork();
    void collapseCachesSizeHintsAndScopesAnimationUpdates();
    void qrCodeReusesRenderedModuleCache();
    void watermarkCachesRenderedPixmap();
    void timelineCachesPaintLayoutAndColors();
    void descriptionsUpdatesTextWithoutGridRebuildAndCachesSizeHint();
    void listSelectionHighlightUsesBalancedInset();
};

void TestAntDataDisplayB::propertiesAndSignals()
{
    // AntList
    auto* list = new AntList;
    QCOMPARE(list->isBordered(), false);
    QCOMPARE(list->isSplit(), true);
    QCOMPARE(list->listSize(), static_cast<int>(AntList::Default));
    QCOMPARE(list->itemCount(), 0);

    QSignalSpy borderSpy(list, &AntList::borderedChanged);
    list->setBordered(true);
    QCOMPARE(list->isBordered(), true);
    QCOMPARE(borderSpy.count(), 1);

    QSignalSpy splitSpy(list, &AntList::splitChanged);
    list->setSplit(false);
    QCOMPARE(list->isSplit(), false);
    QCOMPARE(splitSpy.count(), 1);

    auto* item = new AntListItem;
    list->addItem(item);
    QCOMPARE(list->itemCount(), 1);
    QCOMPARE(list->count(), 1);
    QCOMPARE(list->isEmpty(), false);
    QCOMPARE(list->itemAt(0), item);

    auto* insertedItem = new AntListItem;
    list->insertItem(0, insertedItem);
    QCOMPARE(list->itemAt(0), insertedItem);
    QCOMPARE(list->itemAt(1), item);

    QCOMPARE(list->takeItem(0), insertedItem);
    QCOMPARE(insertedItem->parentWidget(), nullptr);
    QCOMPARE(list->count(), 1);

    list->clear();
    QCOMPARE(list->count(), 0);
    QCOMPARE(list->isEmpty(), true);
    QCOMPARE(item->parentWidget(), nullptr);

    // AntTable
    auto* tbl = new AntTable;
    QCOMPARE(tbl->isBordered(), true);
    QCOMPARE(tbl->tableSize(), Ant::Size::Middle);
    QCOMPARE(tbl->isLoading(), false);
    QCOMPARE(tbl->rowSelection(), Ant::TableSelectionMode::None);
    QCOMPARE(tbl->currentPage(), 1);
    QCOMPARE(tbl->pageSize(), 10);
    QCOMPARE(tbl->rowCount(), 0);
    QCOMPARE(tbl->columnCount(), 0);

    QSignalSpy tBorderSpy(tbl, &AntTable::borderedChanged);
    tbl->setBordered(false);
    QCOMPARE(tbl->isBordered(), false);
    QCOMPARE(tBorderSpy.count(), 1);

    QSignalSpy tSizeSpy(tbl, &AntTable::tableSizeChanged);
    tbl->setTableSize(Ant::Size::Small);
    QCOMPARE(tbl->tableSize(), Ant::Size::Small);
    QCOMPARE(tSizeSpy.count(), 1);

    QSignalSpy tLoadSpy(tbl, &AntTable::loadingChanged);
    tbl->setLoading(true);
    QCOMPARE(tbl->isLoading(), true);
    QCOMPARE(tLoadSpy.count(), 1);

    QSignalSpy selSpy(tbl, &AntTable::rowSelectionChanged);
    tbl->setRowSelection(Ant::TableSelectionMode::Checkbox);
    QCOMPARE(tbl->rowSelection(), Ant::TableSelectionMode::Checkbox);
    QCOMPARE(selSpy.count(), 1);

    QSignalSpy tableColumnsSpy(tbl, &AntTable::columnsChanged);
    tbl->addColumn({QStringLiteral("ID"), QStringLiteral("id"), QStringLiteral("id"), 80});
    QCOMPARE(tbl->columnCount(), 1);
    QCOMPARE(tbl->columnAt(0).key, QStringLiteral("id"));
    QCOMPARE(tbl->headerLabels(), QStringList({QStringLiteral("ID")}));
    QCOMPARE(tableColumnsSpy.count(), 1);

    // Add rows first so pagination works
    QSignalSpy tableRowsSpy(tbl, &AntTable::rowsChanged);
    for (int i = 0; i < 25; ++i) {
        AntTableRow r;
        r.data["id"] = i;
        tbl->addRow(r);
    }
    QCOMPARE(tableRowsSpy.count(), 25);
    QCOMPARE(tbl->cellData(0, QStringLiteral("id")).toInt(), 0);

    QSignalSpy cellSpy(tbl, &AntTable::cellDataChanged);
    tbl->setData(0, QStringLiteral("id"), 100);
    QCOMPARE(tbl->cellData(0, QStringLiteral("id")).toInt(), 100);
    QCOMPARE(cellSpy.count(), 1);

    QSignalSpy pageSpy(tbl, &AntTable::pageChanged);
    tbl->setCurrentPage(2);
    QCOMPARE(tbl->currentPage(), 2);
    QCOMPARE(pageSpy.count(), 1);

    QSignalSpy pageSizeSpy(tbl, &AntTable::pageSizeChanged);
    tbl->setPageSize(20);
    QCOMPARE(tbl->pageSize(), 20);
    QCOMPARE(pageSizeSpy.count(), 1);

    tbl->clearRows();
    QCOMPARE(tbl->rowCount(), 0);
    AntTableRow row;
    row.data["name"] = "Alice";
    tbl->addRow(row);
    QCOMPARE(tbl->rowCount(), 1);
    tbl->clearRows();
    QCOMPARE(tbl->rowCount(), 0);

    auto* sortTbl = new AntTable;
    sortTbl->resize(360, 220);
    sortTbl->addColumn({QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 120});
    AntTableColumn sortAgeColumn{QStringLiteral("Age"), QStringLiteral("age"), QStringLiteral("age"), 120};
    sortAgeColumn.sorter = true;
    sortTbl->addColumn(sortAgeColumn);

    QVector<AntTableRow> sortRows;
    sortRows.push_back({{{QStringLiteral("name"), QStringLiteral("John")}, {QStringLiteral("age"), 28}}});
    sortRows.push_back({{{QStringLiteral("name"), QStringLiteral("Jane")}, {QStringLiteral("age"), 32}}});
    sortRows.push_back({{{QStringLiteral("name"), QStringLiteral("Joe")}, {QStringLiteral("age"), 24}}});
    sortTbl->setRows(sortRows);
    QCOMPARE(sortTbl->rowAt(0).data.value(QStringLiteral("name")).toString(), QStringLiteral("John"));

    QSignalSpy sortSpy(sortTbl, &AntTable::sortChanged);
    QTest::mouseClick(sortTbl, Qt::LeftButton, Qt::NoModifier, QPoint(170, 20));
    QCOMPARE(sortTbl->sortOrder(), Ant::TableSortOrder::Ascending);
    QCOMPARE(sortTbl->rowAt(0).data.value(QStringLiteral("name")).toString(), QStringLiteral("Joe"));
    QTest::mouseClick(sortTbl, Qt::LeftButton, Qt::NoModifier, QPoint(170, 20));
    QCOMPARE(sortTbl->sortOrder(), Ant::TableSortOrder::Descending);
    QCOMPARE(sortTbl->rowAt(0).data.value(QStringLiteral("name")).toString(), QStringLiteral("Jane"));
    QTest::mouseClick(sortTbl, Qt::LeftButton, Qt::NoModifier, QPoint(170, 20));
    QCOMPARE(sortTbl->sortOrder(), Ant::TableSortOrder::None);
    QCOMPARE(sortTbl->currentSortColumn(), QString());
    QCOMPARE(sortTbl->rowAt(0).data.value(QStringLiteral("name")).toString(), QStringLiteral("John"));
    QCOMPARE(sortSpy.count(), 3);

    // AntTree
    auto* tree = new AntTree;
    QCOMPARE(tree->isSelectable(), true);
    QCOMPARE(tree->isCheckable(), true);
    QCOMPARE(tree->isShowLine(), false);
    QCOMPARE(tree->isShowIcon(), true);
    QCOMPARE(tree->isMultiple(), false);

    QSignalSpy treeSelSpy(tree, &AntTree::selectableChanged);
    tree->setSelectable(false);
    QCOMPARE(tree->isSelectable(), false);
    QCOMPARE(treeSelSpy.count(), 1);

    QSignalSpy treeCheckSpy(tree, &AntTree::checkableChanged);
    tree->setCheckable(false);
    QCOMPARE(tree->isCheckable(), false);
    QCOMPARE(treeCheckSpy.count(), 1);

    QSignalSpy lineSpy(tree, &AntTree::showLineChanged);
    tree->setShowLine(true);
    QCOMPARE(tree->isShowLine(), true);
    QCOMPARE(lineSpy.count(), 1);

    QSignalSpy iconSpy(tree, &AntTree::showIconChanged);
    tree->setShowIcon(false);
    QCOMPARE(tree->isShowIcon(), false);
    QCOMPARE(iconSpy.count(), 1);

    QSignalSpy multiSpy(tree, &AntTree::multipleChanged);
    tree->setMultiple(true);
    QCOMPARE(tree->isMultiple(), true);
    QCOMPARE(multiSpy.count(), 1);

    QVector<AntTreeNode> treeData;
    AntTreeNode root;
    root.key = QStringLiteral("root");
    root.title = QStringLiteral("Root");
    root.children.push_back({QStringLiteral("child"), QStringLiteral("Child")});
    treeData.push_back(root);
    tree->setTreeData(treeData);
    QCOMPARE(tree->treeData().size(), 1);
    QCOMPARE(tree->topLevelNodeCount(), 1);
    QCOMPARE(tree->nodeCount(), 2);
    QCOMPARE(tree->containsNode(QStringLiteral("child")), true);

    QSignalSpy expandedSpy(tree, &AntTree::nodeExpanded);
    tree->setNodeExpanded(QStringLiteral("root"), true);
    QCOMPARE(tree->expandedKeys(), QStringList({QStringLiteral("root")}));
    QCOMPARE(expandedSpy.count(), 1);

    QSignalSpy checkedSpy(tree, &AntTree::nodeChecked);
    tree->setNodeChecked(QStringLiteral("child"), true);
    QVERIFY(tree->checkedKeys().contains(QStringLiteral("child")));
    QCOMPARE(checkedSpy.count(), 1);

    tree->clear();
    QCOMPARE(tree->nodeCount(), 0);
    QCOMPARE(tree->selectedKeys().isEmpty(), true);
    QCOMPARE(tree->checkedKeys().isEmpty(), true);

    // AntTimeline
    auto* tl = new AntTimeline;
    QCOMPARE(tl->mode(), Ant::TimelineMode::Start);
    QCOMPARE(tl->orientation(), Ant::TimelineOrientation::Vertical);
    QCOMPARE(tl->dotVariant(), Ant::TimelineDotVariant::Outlined);
    QCOMPARE(tl->isReverse(), false);
    QCOMPARE(tl->count(), 0);

    QSignalSpy modeSpy(tl, &AntTimeline::modeChanged);
    tl->setMode(Ant::TimelineMode::Alternate);
    QCOMPARE(tl->mode(), Ant::TimelineMode::Alternate);
    QCOMPARE(modeSpy.count(), 1);

    QSignalSpy orientSpy(tl, &AntTimeline::orientationChanged);
    tl->setOrientation(Ant::TimelineOrientation::Horizontal);
    QCOMPARE(tl->orientation(), Ant::TimelineOrientation::Horizontal);
    QCOMPARE(orientSpy.count(), 1);

    QSignalSpy dotSpy(tl, &AntTimeline::dotVariantChanged);
    tl->setDotVariant(Ant::TimelineDotVariant::Filled);
    QCOMPARE(tl->dotVariant(), Ant::TimelineDotVariant::Filled);
    QCOMPARE(dotSpy.count(), 1);

    QSignalSpy revSpy(tl, &AntTimeline::reverseChanged);
    tl->setReverse(true);
    QCOMPARE(tl->isReverse(), true);
    QCOMPARE(revSpy.count(), 1);

    tl->addItem("Step 1", "Content 1");
    tl->addItem("Step 2", "Content 2", "green");
    QCOMPARE(tl->count(), 2);
    QCOMPARE(tl->itemAt(0).title, "Step 1");
    QCOMPARE(tl->itemAt(1).color, "green");
    tl->clearItems();
    QCOMPARE(tl->count(), 0);

    // AntDescriptions
    auto* desc = new AntDescriptions;
    QCOMPARE(desc->title(), QString());
    QCOMPARE(desc->extra(), QString());
    QCOMPARE(desc->columnCount(), 3);
    QCOMPARE(desc->isBordered(), false);
    QCOMPARE(desc->isVertical(), false);

    QSignalSpy dTitleSpy(desc, &AntDescriptions::titleChanged);
    desc->setTitle("User Info");
    QCOMPARE(desc->title(), "User Info");
    QCOMPARE(dTitleSpy.count(), 1);

    QSignalSpy dExtraSpy(desc, &AntDescriptions::extraChanged);
    desc->setExtra("Edit");
    QCOMPARE(desc->extra(), "Edit");
    QCOMPARE(dExtraSpy.count(), 1);

    QSignalSpy colSpy(desc, &AntDescriptions::columnCountChanged);
    desc->setColumnCount(2);
    QCOMPARE(desc->columnCount(), 2);
    QCOMPARE(colSpy.count(), 1);

    QSignalSpy dBorderSpy(desc, &AntDescriptions::borderedChanged);
    desc->setBordered(true);
    QCOMPARE(desc->isBordered(), true);
    QCOMPARE(dBorderSpy.count(), 1);

    QSignalSpy vertSpy(desc, &AntDescriptions::verticalChanged);
    desc->setVertical(true);
    QCOMPARE(desc->isVertical(), true);
    QCOMPARE(vertSpy.count(), 1);

    auto* di = new AntDescriptionsItem("Name", "Alice");
    desc->addItem(di);
    QCOMPARE(desc->items().size(), 1);
    desc->addItem("Age", "30", 2);
    QCOMPARE(desc->items().size(), 2);
    desc->clearItems();
    QCOMPARE(desc->items().size(), 0);

    // AntQRCode
    auto* qr = new AntQRCode;
    QCOMPARE(qr->value(), QStringLiteral("https://github.com/sorrowfeng/qt-ant-design"));
    QVERIFY(!qr->qrMatrix().isEmpty());
    QCOMPARE(qr->qrSize(), 160);
    QCOMPARE(qr->errorLevel(), Ant::QRCodeErrorLevel::M);
    QCOMPARE(qr->iconSize(), 40);
    QCOMPARE(qr->isBordered(), true);
    QCOMPARE(qr->status(), Ant::QRCodeStatus::Active);

    QSignalSpy valSpy(qr, &AntQRCode::valueChanged);
    const auto defaultMatrix = qr->qrMatrix();
    qr->setValue("https://example.com");
    QCOMPARE(qr->value(), "https://example.com");
    QVERIFY(qr->qrMatrix() != defaultMatrix);
    QCOMPARE(valSpy.count(), 1);

    QSignalSpy qrSizeSpy(qr, &AntQRCode::qrSizeChanged);
    qr->setQrSize(200);
    QCOMPARE(qr->qrSize(), 200);
    QCOMPARE(qrSizeSpy.count(), 1);

    QSignalSpy colorSpy(qr, &AntQRCode::colorChanged);
    qr->setColor(Qt::black);
    QCOMPARE(qr->color(), QColor(Qt::black));
    QCOMPARE(colorSpy.count(), 1);

    QSignalSpy bgSpy(qr, &AntQRCode::bgColorChanged);
    qr->setBgColor(Qt::white);
    QCOMPARE(qr->bgColor(), QColor(Qt::white));
    QCOMPARE(bgSpy.count(), 1);

    QSignalSpy levelSpy(qr, &AntQRCode::errorLevelChanged);
    qr->setErrorLevel(Ant::QRCodeErrorLevel::H);
    QCOMPARE(qr->errorLevel(), Ant::QRCodeErrorLevel::H);
    QCOMPARE(levelSpy.count(), 1);

    QSignalSpy qrBorderSpy(qr, &AntQRCode::borderedChanged);
    qr->setBordered(false);
    QCOMPARE(qr->isBordered(), false);
    QCOMPARE(qrBorderSpy.count(), 1);

    QSignalSpy statusSpy(qr, &AntQRCode::statusChanged);
    qr->setStatus(Ant::QRCodeStatus::Expired);
    QCOMPARE(qr->status(), Ant::QRCodeStatus::Expired);
    QCOMPARE(statusSpy.count(), 1);

    // AntWatermark
    auto* wm = new AntWatermark;
    QCOMPARE(wm->content(), QStringList());
    QCOMPARE(wm->rotate(), -22.0);

    QSignalSpy contentSpy(wm, &AntWatermark::contentChanged);
    wm->setContent({"Secret", "Draft"});
    QCOMPARE(wm->content().size(), 2);
    QCOMPARE(contentSpy.count(), 1);

    QSignalSpy rotSpy(wm, &AntWatermark::rotateChanged);
    wm->setRotate(-30.0);
    QCOMPARE(wm->rotate(), -30.0);
    QCOMPARE(rotSpy.count(), 1);

    wm->setContent("Single line");
    QCOMPARE(wm->content().size(), 1);

    // AntCarousel
    auto* car = new AntCarousel;
    QCOMPARE(car->autoPlay(), true);
    QCOMPARE(car->interval(), 3000);
    QCOMPARE(car->showDots(), true);
    QCOMPARE(car->currentIndex(), 0);
    QCOMPARE(car->count(), 0);

    QSignalSpy autoSpy(car, &AntCarousel::autoPlayChanged);
    car->setAutoPlay(false);
    QCOMPARE(car->autoPlay(), false);
    QCOMPARE(autoSpy.count(), 1);

    QSignalSpy intSpy(car, &AntCarousel::intervalChanged);
    car->setInterval(5000);
    QCOMPARE(car->interval(), 5000);
    QCOMPARE(intSpy.count(), 1);

    QSignalSpy dotsSpy(car, &AntCarousel::showDotsChanged);
    car->setShowDots(false);
    QCOMPARE(car->showDots(), false);
    QCOMPARE(dotsSpy.count(), 1);

    auto* s1 = new QWidget;
    auto* s2 = new QWidget;
    car->resize(240, 160);
    car->addSlide(s1);
    car->addSlide(s2);
    QCOMPARE(car->count(), 2);

    QSignalSpy idxSpy(car, &AntCarousel::currentIndexChanged);
    car->setCurrentIndex(1);
    QCOMPARE(car->currentIndex(), 1);
    QCOMPARE(idxSpy.count(), 1);
    QCOMPARE(car->transitionProgress(), 0.0);
    QVERIFY(!s1->isHidden());
    QVERIFY(!s2->isHidden());
    QCOMPARE(s1->geometry(), car->rect());
    QCOMPARE(s2->geometry().left(), car->width());
    QTRY_VERIFY_WITH_TIMEOUT(s1->isHidden(), 1000);
    QCOMPARE(s2->geometry(), car->rect());

    car->clearSlides();
    QCOMPARE(car->count(), 0);

    // AntCollapse
    auto* col = new AntCollapse;
    QCOMPARE(col->accordion(), false);
    QCOMPARE(col->bordered(), true);

    QSignalSpy accSpy(col, &AntCollapse::accordionChanged);
    col->setAccordion(true);
    QCOMPARE(col->accordion(), true);
    QCOMPARE(accSpy.count(), 1);

    QSignalSpy cbSpy(col, &AntCollapse::borderedChanged);
    col->setBordered(false);
    QCOMPARE(col->bordered(), false);
    QCOMPARE(cbSpy.count(), 1);

    auto* p = col->addPanel("Panel 1");
    QCOMPARE(p->title(), "Panel 1");
    QCOMPARE(p->isExpanded(), false);

    QSignalSpy pTitleSpy(p, &AntCollapsePanel::titleChanged);
    p->setTitle("Updated");
    QCOMPARE(p->title(), "Updated");
    QCOMPARE(pTitleSpy.count(), 1);

    QSignalSpy expSpy(p, &AntCollapsePanel::expandedChanged);
    p->setExpanded(true);
    QCOMPARE(p->isExpanded(), true);
    QCOMPARE(expSpy.count(), 1);
}

void TestAntDataDisplayB::carouselPausesAutoplayAndScopesTransitionWork()
{
    AntCarousel carousel;
    carousel.resize(260, 160);
    carousel.setInterval(1000);
    auto* first = new QWidget;
    first->setObjectName(QStringLiteral("first-slide"));
    first->setStyleSheet(QStringLiteral("background: #1677ff;"));
    auto* second = new QWidget;
    second->setObjectName(QStringLiteral("second-slide"));
    second->setStyleSheet(QStringLiteral("background: #52c41a;"));
    carousel.addSlide(first);
    carousel.addSlide(second);

    QVERIFY(!carousel.property("antCarouselAutoPlayTimerActive").toBool());
    carousel.show();
    QVERIFY(QTest::qWaitForWindowExposed(&carousel));
    QTRY_VERIFY(carousel.property("antCarouselAutoPlayTimerActive").toBool());

    const int indexBeforeHide = carousel.currentIndex();
    carousel.hide();
    QCoreApplication::processEvents();
    QVERIFY(!carousel.property("antCarouselAutoPlayTimerActive").toBool());
    QTest::qWait(120);
    QCOMPARE(carousel.currentIndex(), indexBeforeHide);

    carousel.setAutoPlay(false);
    carousel.show();
    QVERIFY(QTest::qWaitForWindowExposed(&carousel));
    QVERIFY(!carousel.property("antCarouselAutoPlayTimerActive").toBool());

    const int dotsPaintBefore = carousel.property("antCarouselDotsPaintUpdateCount").toInt();
    carousel.setCurrentIndex(1);
    QCOMPARE(carousel.currentIndex(), 1);
    QCOMPARE(carousel.property("antCarouselLastUpdateMode").toString(), QStringLiteral("transition"));
    const int dotsPaintAfterStart = carousel.property("antCarouselDotsPaintUpdateCount").toInt();
    const int dotsGeometryAfterStart = carousel.property("antCarouselDotsGeometryUpdateCount").toInt();
    const int slideGeometryAfterStart = carousel.property("antCarouselSlideGeometryUpdateCount").toInt();
    QVERIFY(dotsPaintAfterStart > dotsPaintBefore);

    QTest::qWait(120);
    QVERIFY(carousel.property("antCarouselSlideGeometryUpdateCount").toInt() > slideGeometryAfterStart);
    QCOMPARE(carousel.property("antCarouselDotsPaintUpdateCount").toInt(), dotsPaintAfterStart);
    QCOMPARE(carousel.property("antCarouselDotsGeometryUpdateCount").toInt(), dotsGeometryAfterStart);

    QTRY_VERIFY_WITH_TIMEOUT(first->isHidden(), 1000);
    QCOMPARE(second->geometry(), carousel.rect());
}

void TestAntDataDisplayB::collapseCachesSizeHintsAndScopesAnimationUpdates()
{
    AntCollapse collapse;
    collapse.resize(320, 180);
    auto* panel = collapse.addPanel(QStringLiteral("Details"));
    auto* content = new QLabel(QStringLiteral("Cached animated content"));
    content->setMinimumHeight(64);
    panel->setContentWidget(content);

    const QSize firstPanelHint = panel->sizeHint();
    const int panelBuilds = panel->property("antCollapsePanelSizeHintBuildCount").toInt();
    const int panelHits = panel->property("antCollapsePanelSizeHintHitCount").toInt();
    QVERIFY(firstPanelHint.height() >= 46);
    QVERIFY(panelBuilds > 0);
    QCOMPARE(panel->sizeHint(), firstPanelHint);
    QCOMPARE(panel->property("antCollapsePanelSizeHintBuildCount").toInt(), panelBuilds);
    QVERIFY(panel->property("antCollapsePanelSizeHintHitCount").toInt() > panelHits);

    const QSize firstCollapseHint = collapse.sizeHint();
    const int collapseBuilds = collapse.property("antCollapseSizeHintBuildCount").toInt();
    const int collapseHits = collapse.property("antCollapseSizeHintHitCount").toInt();
    QCOMPARE(collapse.sizeHint(), firstCollapseHint);
    QCOMPARE(collapse.property("antCollapseSizeHintBuildCount").toInt(), collapseBuilds);
    QVERIFY(collapse.property("antCollapseSizeHintHitCount").toInt() > collapseHits);

    collapse.show();
    QVERIFY(QTest::qWaitForWindowExposed(&collapse));
    const int contentUpdates = panel->property("antCollapsePanelContentRegionUpdateCount").toInt();
    const int layoutUpdates = panel->property("antCollapsePanelLayoutUpdateCount").toInt();
    const int buildsBeforeExpand = panel->property("antCollapsePanelSizeHintBuildCount").toInt();
    panel->setExpanded(true);
    QCOMPARE(panel->isExpanded(), true);
    QTRY_VERIFY(panel->property("antCollapsePanelContentRegionUpdateCount").toInt() > contentUpdates);
    QVERIFY(panel->property("antCollapsePanelLayoutUpdateCount").toInt() > layoutUpdates);
    QTRY_VERIFY_WITH_TIMEOUT(panel->sizeHint().height() > firstPanelHint.height(), 1000);
    QVERIFY(panel->property("antCollapsePanelSizeHintBuildCount").toInt() > buildsBeforeExpand);

    const int updatesAfterExpand = panel->property("antCollapsePanelContentRegionUpdateCount").toInt();
    QTRY_VERIFY_WITH_TIMEOUT(panel->contentWidget()->isVisible(), 1000);
    QTRY_COMPARE_WITH_TIMEOUT(panel->property("antCollapsePanelLastUpdateMode").toString(), QStringLiteral("animation"), 1000);
    panel->setExpanded(false);
    QTRY_VERIFY(panel->property("antCollapsePanelContentRegionUpdateCount").toInt() > updatesAfterExpand);

    const int headerUpdates = panel->property("antCollapsePanelRegionUpdateCount").toInt();
    QEnterEvent enterEvent(QPointF(12, 12), QPointF(12, 12), QPointF(panel->mapToGlobal(QPoint(12, 12))));
    QCoreApplication::sendEvent(panel, &enterEvent);
    QCOMPARE(panel->property("antCollapsePanelLastUpdateMode").toString(), QStringLiteral("hover"));
    QVERIFY(panel->property("antCollapsePanelRegionUpdateCount").toInt() > headerUpdates);
}

void TestAntDataDisplayB::listWidgetCompatibilityApis()
{
    AntListWidget list;
    list.addItem(QStringLiteral("Alpha"));
    list.addItems({QStringLiteral("Beta"), QStringLiteral("Gamma")});
    list.insertItem(1, QStringLiteral("Inserted"));

    QCOMPARE(list.count(), 4);
    QCOMPARE(list.item(0)->text(), QStringLiteral("Alpha"));
    QCOMPARE(list.item(1)->text(), QStringLiteral("Inserted"));
    QCOMPARE(list.row(list.item(2)), 2);
    QCOMPARE(list.itemAt(2), list.item(2));

    QSignalSpy itemChangedSpy(&list, &AntList::itemChanged);
    list.item(0)->setData(Qt::UserRole, QStringLiteral("alpha-id"));
    QCOMPARE(list.item(0)->data(Qt::UserRole).toString(), QStringLiteral("alpha-id"));
    list.item(0)->setText(QStringLiteral("Alpha Prime"));
    QCOMPARE(list.item(0)->text(), QStringLiteral("Alpha Prime"));
    QVERIFY(itemChangedSpy.count() >= 2);

    const QList<AntListItem*> found = list.findItems(QStringLiteral("Gamma"), Qt::MatchExactly);
    QCOMPARE(found.size(), 1);
    QCOMPARE(found.first(), list.item(3));

    QSignalSpy currentItemSpy(&list, &AntList::currentItemChanged);
    QSignalSpy currentRowSpy(&list, &AntList::currentRowChanged);
    QSignalSpy selectionSpy(&list, &AntList::itemSelectionChanged);
    list.setCurrentRow(2);
    QCOMPARE(list.currentRow(), 2);
    QCOMPARE(list.currentItem(), list.item(2));
    QCOMPARE(list.selectedItems(), QList<AntListItem*>{list.item(2)});
    QCOMPARE(currentItemSpy.count(), 1);
    QCOMPARE(currentRowSpy.count(), 1);
    QVERIFY(selectionSpy.count() >= 1);

    list.setSelectionMode(QAbstractItemView::MultiSelection);
    list.setItemSelected(list.item(0), true);
    list.setItemSelected(list.item(2), true);
    QCOMPARE(list.selectedItems().size(), 2);

    list.sortItems(Qt::AscendingOrder);
    QCOMPARE(list.item(0)->text(), QStringLiteral("Alpha Prime"));

    list.resize(240, list.sizeHint().height());
    list.show();
    QVERIFY(QTest::qWaitForWindowExposed(&list));

    QSignalSpy clickedSpy(&list, &AntList::itemClicked);
    QTest::mouseClick(&list, Qt::LeftButton, Qt::NoModifier, list.visualItemRect(list.item(0)).center());
    QCOMPARE(clickedSpy.count(), 1);
    QCOMPARE(clickedSpy.takeFirst().at(0).value<AntListItem*>(), list.item(0));

    AntListItem* taken = list.takeItem(1);
    QVERIFY(taken != nullptr);
    QCOMPARE(taken->parentWidget(), nullptr);
    delete taken;
    QCOMPARE(list.count(), 3);
}

void TestAntDataDisplayB::listBulkInsertionCoalescesLayout()
{
    AntList list;
    QStringList labels;
    for (int i = 0; i < 40; ++i)
    {
        labels.append(QStringLiteral("Item %1").arg(i));
    }

    list.addItems(labels);
    QCOMPARE(list.count(), labels.size());
    QCOMPARE(list.item(0)->text(), QStringLiteral("Item 0"));
    QCOMPARE(list.item(39)->text(), QStringLiteral("Item 39"));
    QCOMPARE(list.property("antListLastBulkOperation").toString(), QStringLiteral("addItems"));
    QCOMPARE(list.property("antListLastBulkItemCount").toInt(), labels.size());
    QCOMPARE(list.property("antListLastBulkLayoutCount").toInt(), 1);

    list.insertItems(2, {QStringLiteral("Inserted A"), QStringLiteral("Inserted B")});
    QCOMPARE(list.count(), 42);
    QCOMPARE(list.item(2)->text(), QStringLiteral("Inserted A"));
    QCOMPARE(list.item(3)->text(), QStringLiteral("Inserted B"));
    QCOMPARE(list.property("antListLastBulkOperation").toString(), QStringLiteral("insertItems"));
    QCOMPARE(list.property("antListLastBulkItemCount").toInt(), 2);
    QCOMPARE(list.property("antListLastBulkLayoutCount").toInt(), 1);

    list.resize(240, 160);
    list.show();
    QVERIFY(QTest::qWaitForWindowExposed(&list));
    QVERIFY(list.maximumScrollOffset() > 0);
    QVERIFY(list.visualItemRect(list.item(0)).isValid());
}

void TestAntDataDisplayB::listInternalScrolling()
{
    AntListWidget list;
    for (int i = 0; i < 12; ++i)
    {
        list.addItem(QStringLiteral("Item %1").arg(i));
    }
    list.resize(240, 96);
    list.show();
    QVERIFY(QTest::qWaitForWindowExposed(&list));

    QVERIFY(list.maximumScrollOffset() > 0);
    QCOMPARE(list.verticalScrollOffset(), 0);

    QWheelEvent wheelEvent(QPointF(list.rect().center()),
                           QPointF(list.mapToGlobal(list.rect().center())),
                           QPoint(),
                           QPoint(0, -120),
                           Qt::NoButton,
                           Qt::NoModifier,
                           Qt::NoScrollPhase,
                           false);
    QCoreApplication::sendEvent(&list, &wheelEvent);
    QVERIFY(list.verticalScrollOffset() > 0);
    QVERIFY(list.visualItemRect(list.item(0)).top() < 0);

    list.scrollToItem(list.item(list.count() - 1));
    QVERIFY(list.visualItemRect(list.item(list.count() - 1)).bottom() <= list.rect().bottom());
}

void TestAntDataDisplayB::listUsesExpandingLayoutPolicy()
{
    QWidget host;
    auto* layout = new QVBoxLayout(&host);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* list = new AntList;
    list->setBordered(true);
    for (int i = 0; i < 8; ++i)
    {
        list->addItem(QStringLiteral("Item %1").arg(i + 1));
    }
    layout->addWidget(list, 1);

    host.resize(260, 150);
    host.show();
    QCoreApplication::processEvents();

    QCOMPARE(list->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    QCOMPARE(list->sizePolicy().verticalPolicy(), QSizePolicy::Expanding);
    QCOMPARE(list->width(), host.width());
    QCOMPARE(list->height(), host.height());
    QVERIFY(list->maximumScrollOffset() > 0);
}

void TestAntDataDisplayB::treeCachesFlattenedVisibleNodes()
{
    QVector<AntTreeNode> roots;
    for (int i = 0; i < 4; ++i)
    {
        AntTreeNode root;
        root.key = QStringLiteral("root-%1").arg(i);
        root.title = QStringLiteral("Root %1").arg(i);
        root.expanded = true;
        for (int j = 0; j < 6; ++j)
        {
            AntTreeNode child;
            child.key = QStringLiteral("root-%1-child-%2").arg(i).arg(j);
            child.title = QStringLiteral("Child %1").arg(j);
            child.isLeaf = true;
            root.children.push_back(child);
        }
        roots.push_back(root);
    }

    AntTree tree;
    tree.resize(320, 160);
    tree.setTreeData(roots);
    QCOMPARE(tree.property("antTreeFlatCacheHit").toBool(), false);
    QCOMPARE(tree.property("antTreeFlatNodeCount").toInt(), 28);
    const int initialBuilds = tree.property("antTreeFlatCacheBuildCount").toInt();
    QVERIFY(initialBuilds > 0);

    auto renderTree = [&tree]() {
        QImage image(tree.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        tree.render(&painter);
    };

    renderTree();
    QCOMPARE(tree.property("antTreeFlatCacheHit").toBool(), true);
    QCOMPARE(tree.property("antTreeFlatCacheBuildCount").toInt(), initialBuilds);

    QMouseEvent moveToFirstRow(QEvent::MouseMove, QPointF(120, 14), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&tree, &moveToFirstRow);
    QCOMPARE(tree.property("antTreeFlatCacheHit").toBool(), true);
    QCOMPARE(tree.property("antTreeLastUpdateMode").toString(), QStringLiteral("row"));
    QCOMPARE(tree.property("antTreeLastRowUpdateCount").toInt(), 1);

    QMouseEvent moveToSecondRow(QEvent::MouseMove, QPointF(120, 42), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&tree, &moveToSecondRow);
    QCOMPARE(tree.property("antTreeFlatCacheHit").toBool(), true);
    QCOMPARE(tree.property("antTreeLastUpdateMode").toString(), QStringLiteral("row"));
    QCOMPARE(tree.property("antTreeLastRowUpdateCount").toInt(), 2);
    QCOMPARE(tree.property("antTreeFlatCacheBuildCount").toInt(), initialBuilds);

    tree.setNodeExpanded(QStringLiteral("root-0"), false);
    const int collapsedBuilds = tree.property("antTreeFlatCacheBuildCount").toInt();
    QVERIFY(collapsedBuilds > initialBuilds);
    QCOMPARE(tree.property("antTreeFlatCacheHit").toBool(), false);
    QCOMPARE(tree.property("antTreeFlatNodeCount").toInt(), 22);

    renderTree();
    QCOMPARE(tree.property("antTreeFlatCacheHit").toBool(), true);
    QCOMPARE(tree.property("antTreeFlatCacheBuildCount").toInt(), collapsedBuilds);
}

void TestAntDataDisplayB::tableHoverUsesRowScopedUpdates()
{
    AntTable table;
    table.resize(360, 240);
    table.addColumn({QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 140});
    table.addColumn({QStringLiteral("Age"), QStringLiteral("age"), QStringLiteral("age"), 80});
    for (int i = 0; i < 5; ++i)
    {
        AntTableRow row;
        row.data[QStringLiteral("name")] = QStringLiteral("User %1").arg(i);
        row.data[QStringLiteral("age")] = i + 20;
        table.addRow(row);
    }

    const auto sendMouseMove = [&table](const QPoint& pos) {
        QMouseEvent event(QEvent::MouseMove, QPointF(pos), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        QCoreApplication::sendEvent(&table, &event);
    };

    sendMouseMove(QPoint(120, 72));
    QCOMPARE(table.property("antTableLastUpdateMode").toString(), QStringLiteral("row"));
    QCOMPARE(table.property("antTableLastRowUpdateCount").toInt(), 1);

    sendMouseMove(QPoint(120, 128));
    QCOMPARE(table.property("antTableLastUpdateMode").toString(), QStringLiteral("row"));
    QCOMPARE(table.property("antTableLastRowUpdateCount").toInt(), 2);

    QEvent leaveEvent(QEvent::Leave);
    QCoreApplication::sendEvent(&table, &leaveEvent);
    QCOMPARE(table.property("antTableLastUpdateMode").toString(), QStringLiteral("row"));
    QCOMPARE(table.property("antTableLastRowUpdateCount").toInt(), 1);
}

void TestAntDataDisplayB::tableSelectionCompatibilityApis()
{
    AntTable table;
    table.addColumn({QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 120});

    QVector<AntTableRow> rows;
    AntTableRow alice;
    alice.data[QStringLiteral("name")] = QStringLiteral("Alice");
    alice.tooltip = QStringLiteral("Firmware A - 12 KB");
    rows.push_back(alice);
    AntTableRow bob;
    bob.data[QStringLiteral("name")] = QStringLiteral("Bob");
    rows.push_back(bob);
    table.setRows(rows);

    QCOMPARE(table.rows().size(), 2);
    QCOMPARE(table.rows().at(0).tooltip, QStringLiteral("Firmware A - 12 KB"));
    QCOMPARE(table.currentRowIndex(), -1);

    QSignalSpy selectionSpy(&table, &AntTable::selectionChanged);
    table.selectRow(1);
    QCOMPARE(table.currentRowIndex(), 1);
    QCOMPARE(table.rowAt(1).selected, true);
    QCOMPARE(table.rowAt(0).selected, false);
    QCOMPARE(table.selectedRowKeys(), QStringList({QStringLiteral("Bob")}));
    QCOMPARE(selectionSpy.count(), 1);

    table.selectRow(0);
    QCOMPARE(table.currentRowIndex(), 0);
    QCOMPARE(table.rowAt(0).selected, true);
    QCOMPARE(table.rowAt(1).selected, false);
    QCOMPARE(table.selectedRowKeys(), QStringList({QStringLiteral("Alice")}));
    QCOMPARE(selectionSpy.count(), 2);

    table.setRows(rows);
    QCOMPARE(table.currentRowIndex(), -1);
}

void TestAntDataDisplayB::timelineCachesPaintLayoutAndColors()
{
    AntTimeline timeline;
    timeline.resize(420, 260);
    timeline.addItem(QStringLiteral("Created"),
                     QStringLiteral("A longer description that wraps and exercises cached vertical item metrics."),
                     QStringLiteral("blue"));
    timeline.addItem(QStringLiteral("Reviewed"), QStringLiteral("Short note."), QStringLiteral("green"));
    timeline.addItem(QStringLiteral("Released"),
                     QStringLiteral("Another wrapped line so repeated paints do not remeasure every item."),
                     QStringLiteral("#722ed1"));

    auto renderTimeline = [&timeline]() {
        QImage image(timeline.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        timeline.render(&painter);
    };

    renderTimeline();
    QCOMPARE(timeline.property("antTimelineLayoutCacheHit").toBool(), false);
    QCOMPARE(timeline.property("antTimelineColorCacheHit").toBool(), false);
    QCOMPARE(timeline.property("antTimelineCachedItemCount").toInt(), 3);

    renderTimeline();
    QCOMPARE(timeline.property("antTimelineLayoutCacheHit").toBool(), true);
    QCOMPARE(timeline.property("antTimelineColorCacheHit").toBool(), true);

    timeline.setReverse(true);
    renderTimeline();
    QCOMPARE(timeline.property("antTimelineLayoutCacheHit").toBool(), false);
    QCOMPARE(timeline.property("antTimelineColorCacheHit").toBool(), true);

    renderTimeline();
    QCOMPARE(timeline.property("antTimelineLayoutCacheHit").toBool(), true);
    QCOMPARE(timeline.property("antTimelineColorCacheHit").toBool(), true);

    timeline.addItem(QStringLiteral("Archived"), QStringLiteral("Cache invalidates after data changes."),
                     QStringLiteral("red"));
    renderTimeline();
    QCOMPARE(timeline.property("antTimelineLayoutCacheHit").toBool(), false);
    QCOMPARE(timeline.property("antTimelineColorCacheHit").toBool(), false);
    QCOMPARE(timeline.property("antTimelineCachedItemCount").toInt(), 4);
}

void TestAntDataDisplayB::qrCodeReusesRenderedModuleCache()
{
    AntQRCode qr;
    const QSize moduleSize(132, 132);
    const QColor foreground(QStringLiteral("#1677ff"));

    const QPixmap first = qr.cachedQrPixmap(moduleSize, 1.0, foreground);
    QVERIFY(!first.isNull());
    const qint64 firstKey = first.cacheKey();

    const QPixmap second = qr.cachedQrPixmap(moduleSize, 1.0, foreground);
    QCOMPARE(second.cacheKey(), firstKey);

    qr.setStatus(Ant::QRCodeStatus::Expired);
    const QPixmap statusOnly = qr.cachedQrPixmap(moduleSize, 1.0, foreground);
    QCOMPARE(statusOnly.cacheKey(), firstKey);

    const QPixmap highDpi = qr.cachedQrPixmap(moduleSize, 2.0, foreground);
    QVERIFY(!highDpi.isNull());
    QVERIFY(highDpi.cacheKey() != firstKey);
    QCOMPARE(qRound(highDpi.devicePixelRatio() * 1000.0), 2000);

    qr.setValue(QStringLiteral("https://example.com/cache-bust"));
    const QPixmap changedPayload = qr.cachedQrPixmap(moduleSize, 1.0, foreground);
    QVERIFY(changedPayload.cacheKey() != firstKey);
}

void TestAntDataDisplayB::watermarkCachesRenderedPixmap()
{
    AntWatermark watermark;
    watermark.resize(260, 180);
    watermark.setContent({QStringLiteral("Secret"), QStringLiteral("Draft")});
    watermark.show();
    QVERIFY(QTest::qWaitForWindowExposed(&watermark));

    watermark.grab();
    const int pixmapBuilds = watermark.property("antWatermarkPixmapBuildCount").toInt();
    const int pixmapHits = watermark.property("antWatermarkPixmapCacheHitCount").toInt();
    QVERIFY(pixmapBuilds > 0);

    watermark.grab();
    QCOMPARE(watermark.property("antWatermarkPixmapBuildCount").toInt(), pixmapBuilds);
    QVERIFY(watermark.property("antWatermarkPixmapCacheHitCount").toInt() > pixmapHits);

    const int buildsBeforeOffset = watermark.property("antWatermarkPixmapBuildCount").toInt();
    watermark.setOffset(QPoint(16, 12));
    QCOMPARE(watermark.property("antWatermarkLastUpdateMode").toString(), QStringLiteral("offset"));
    watermark.grab();
    QVERIFY(watermark.property("antWatermarkPixmapBuildCount").toInt() > buildsBeforeOffset);

    AntWatermarkFont font = watermark.watermarkFont();
    font.fontSize = 18;
    font.color = QColor("#1677ff");
    const int buildsBeforeFont = watermark.property("antWatermarkPixmapBuildCount").toInt();
    watermark.setWatermarkFont(font);
    QCOMPARE(watermark.property("antWatermarkLastUpdateMode").toString(), QStringLiteral("font"));
    watermark.grab();
    QVERIFY(watermark.property("antWatermarkPixmapBuildCount").toInt() > buildsBeforeFont);
}

void TestAntDataDisplayB::descriptionsUpdatesTextWithoutGridRebuildAndCachesSizeHint()
{
    AntDescriptions descriptions;
    descriptions.setTitle(QStringLiteral("User Info"));
    descriptions.setColumnCount(2);
    descriptions.setBordered(true);
    auto* nameItem = descriptions.addItem(QStringLiteral("Name"), QStringLiteral("Alice"));
    descriptions.addItem(QStringLiteral("Role"), QStringLiteral("Designer"));
    descriptions.resize(420, descriptions.sizeHint().height());
    descriptions.show();
    QVERIFY(QTest::qWaitForWindowExposed(&descriptions));

    const QSize firstHint = descriptions.sizeHint();
    QVERIFY(firstHint.width() > 0);
    QVERIFY(descriptions.property("antDescriptionsSizeHintBuildCount").toInt() > 0);
    const int cacheHitsBefore = descriptions.property("antDescriptionsSizeHintCacheHitCount").toInt();
    QCOMPARE(descriptions.sizeHint(), firstHint);
    QVERIFY(descriptions.property("antDescriptionsSizeHintCacheHitCount").toInt() > cacheHitsBefore);

    const int rebuildsBeforeText = descriptions.property("antDescriptionsGridRebuildCount").toInt();
    const int textUpdatesBefore = descriptions.property("antDescriptionsItemTextUpdateCount").toInt();
    nameItem->setContent(QStringLiteral("Alice Chen"));
    QCOMPARE(descriptions.property("antDescriptionsGridRebuildCount").toInt(), rebuildsBeforeText);
    QVERIFY(descriptions.property("antDescriptionsItemTextUpdateCount").toInt() > textUpdatesBefore);

    const int labelUpdatesBefore = descriptions.property("antDescriptionsItemTextUpdateCount").toInt();
    nameItem->setLabel(QStringLiteral("Full name"));
    QCOMPARE(descriptions.property("antDescriptionsGridRebuildCount").toInt(), rebuildsBeforeText);
    QVERIFY(descriptions.property("antDescriptionsItemTextUpdateCount").toInt() > labelUpdatesBefore);

    const int rebuildsBeforeHeader = descriptions.property("antDescriptionsGridRebuildCount").toInt();
    descriptions.setTitle(QStringLiteral("Profile"));
    QCOMPARE(descriptions.property("antDescriptionsGridRebuildCount").toInt(), rebuildsBeforeHeader);

    const int rebuildsBeforeSpan = descriptions.property("antDescriptionsGridRebuildCount").toInt();
    nameItem->setSpan(2);
    QVERIFY(descriptions.property("antDescriptionsGridRebuildCount").toInt() > rebuildsBeforeSpan);
}

void TestAntDataDisplayB::listSelectionHighlightUsesBalancedInset()
{
    AntListWidget list;
    list.setBordered(true);
    list.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta"), QStringLiteral("Gamma")});
    list.setCurrentRow(1);
    list.resize(260, list.sizeHint().height());
    list.show();
    QVERIFY(QTest::qWaitForWindowExposed(&list));

    QImage image(list.size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    {
        QPainter painter(&image);
        list.render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    }

    const QRect rowRect = list.visualItemRect(list.item(1));
    const QColor selectedColor = antTheme->tokens().colorPrimaryBg;
    auto nearColor = [](const QColor& actual, const QColor& expected) {
        return qAbs(actual.red() - expected.red()) <= 18 &&
               qAbs(actual.green() - expected.green()) <= 18 &&
               qAbs(actual.blue() - expected.blue()) <= 18;
    };

    const QColor leftInset = image.pixelColor(rowRect.left() + 1, rowRect.center().y());
    const QColor rightInset = image.pixelColor(rowRect.right() - 1, rowRect.center().y());
    const QColor topInset = image.pixelColor(rowRect.center().x(), rowRect.top() + 1);
    const QColor bottomInset = image.pixelColor(rowRect.center().x(), rowRect.bottom() - 1);
    const QColor leftFill = image.pixelColor(rowRect.left() + 3, rowRect.center().y());
    const QColor rightFill = image.pixelColor(rowRect.right() - 3, rowRect.center().y());
    const QColor topFill = image.pixelColor(rowRect.center().x(), rowRect.top() + 3);
    const QColor bottomFill = image.pixelColor(rowRect.center().x(), rowRect.bottom() - 3);

    QVERIFY2(!nearColor(leftInset, selectedColor), "selected list row should leave the same left inset as the vertical inset");
    QVERIFY2(!nearColor(rightInset, selectedColor), "selected list row should leave the same right inset as the vertical inset");
    QVERIFY2(!nearColor(topInset, selectedColor), "selected list row should keep its top inset");
    QVERIFY2(!nearColor(bottomInset, selectedColor), "selected list row should keep its bottom inset");
    QVERIFY2(nearColor(leftFill, selectedColor), "selected list row should fill immediately inside the left inset");
    QVERIFY2(nearColor(rightFill, selectedColor), "selected list row should fill immediately inside the right inset");
    QVERIFY2(nearColor(topFill, selectedColor), "selected list row should fill immediately inside the top inset");
    QVERIFY2(nearColor(bottomFill, selectedColor), "selected list row should fill immediately inside the bottom inset");
}

QTEST_MAIN(TestAntDataDisplayB)
#include "TestAntDataDisplayB.moc"
