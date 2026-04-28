#include <QSignalSpy>
#include <QTest>
#include "widgets/AntList.h"
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
    QCOMPARE(list->itemAt(0), item);

    // AntTable
    auto* tbl = new AntTable;
    QCOMPARE(tbl->isBordered(), true);
    QCOMPARE(tbl->tableSize(), Ant::Size::Middle);
    QCOMPARE(tbl->isLoading(), false);
    QCOMPARE(tbl->rowSelection(), Ant::TableSelectionMode::None);
    QCOMPARE(tbl->currentPage(), 1);
    QCOMPARE(tbl->pageSize(), 10);
    QCOMPARE(tbl->rowCount(), 0);

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

    // Add rows first so pagination works
    for (int i = 0; i < 25; ++i) {
        AntTableRow r;
        r.data["id"] = i;
        tbl->addRow(r);
    }

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
    QCOMPARE(qr->value(), QString());
    QCOMPARE(qr->qrSize(), 160);
    QCOMPARE(qr->errorLevel(), Ant::QRCodeErrorLevel::M);
    QCOMPARE(qr->iconSize(), 40);
    QCOMPARE(qr->isBordered(), true);
    QCOMPARE(qr->status(), Ant::QRCodeStatus::Active);

    QSignalSpy valSpy(qr, &AntQRCode::valueChanged);
    qr->setValue("https://example.com");
    QCOMPARE(qr->value(), "https://example.com");
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
    car->addSlide(s1);
    car->addSlide(s2);
    QCOMPARE(car->count(), 2);

    QSignalSpy idxSpy(car, &AntCarousel::currentIndexChanged);
    car->setCurrentIndex(1);
    QCOMPARE(car->currentIndex(), 1);
    QCOMPARE(idxSpy.count(), 1);

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

QTEST_MAIN(TestAntDataDisplayB)
#include "TestAntDataDisplayB.moc"
