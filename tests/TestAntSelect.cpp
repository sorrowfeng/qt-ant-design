#include <QSignalSpy>
#include <QTest>

#include "widgets/AntSelect.h"

class TestAntSelect : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
};

void TestAntSelect::propertiesAndSignals()
{
    auto* sel = new AntSelect;
    QCOMPARE(sel->selectMode(), Ant::SelectMode::Single);
    QCOMPARE(sel->currentIndex(), -1);
    QCOMPARE(sel->isEditable(), false);
    QCOMPARE(sel->isLoading(), false);
    QCOMPARE(sel->count(), 0);
    QCOMPARE(sel->maxTagCount(), 0);
    QCOMPARE(sel->selectSize(), Ant::SelectSize::Middle);
    QCOMPARE(sel->status(), Ant::SelectStatus::Normal);
    QCOMPARE(sel->variant(), Ant::SelectVariant::Outlined);

    // Options
    sel->addOption("Apple", "apple");
    sel->addOption("Banana", "banana");
    sel->addOption("Cherry", "cherry");
    QCOMPARE(sel->count(), 3);

    // Single mode
    QSignalSpy indexSpy(sel, &AntSelect::currentIndexChanged);
    sel->setCurrentIndex(1);
    QCOMPARE(sel->currentIndex(), 1);
    QCOMPARE(sel->currentText(), "Banana");
    QCOMPARE(sel->currentValue().toString(), "banana");
    QCOMPARE(indexSpy.count(), 1);

    // Multiple mode
    QSignalSpy modeSpy(sel, &AntSelect::selectModeChanged);
    sel->setSelectMode(Ant::SelectMode::Multiple);
    QCOMPARE(sel->selectMode(), Ant::SelectMode::Multiple);
    QCOMPARE(modeSpy.count(), 1);

    // Clear any prior selection from single mode before testing multi-select
    sel->clearSelection();
    sel->addSelectedIndex(0);
    sel->addSelectedIndex(2);
    QCOMPARE(sel->selectedIndices(), QList<int>({0, 2}));
    QCOMPARE(sel->selectedTexts(), QStringList({"Apple", "Cherry"}));

    sel->removeSelectedIndex(0);
    QCOMPARE(sel->selectedIndices(), QList<int>({2}));

    sel->clearSelection();
    QCOMPARE(sel->selectedIndices().isEmpty(), true);

    // Tags mode
    sel->setSelectMode(Ant::SelectMode::Tags);
    QCOMPARE(sel->selectMode(), Ant::SelectMode::Tags);

    // maxTagCount
    QSignalSpy maxTagSpy(sel, &AntSelect::maxTagCountChanged);
    sel->setMaxTagCount(3);
    QCOMPARE(sel->maxTagCount(), 3);
    QCOMPARE(maxTagSpy.count(), 1);

    // Other properties
    QSignalSpy sizeSpy(sel, &AntSelect::selectSizeChanged);
    sel->setSelectSize(Ant::SelectSize::Large);
    QCOMPARE(sel->selectSize(), Ant::SelectSize::Large);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy statusSpy(sel, &AntSelect::statusChanged);
    sel->setStatus(Ant::SelectStatus::Error);
    QCOMPARE(sel->status(), Ant::SelectStatus::Error);
    QCOMPARE(statusSpy.count(), 1);

    QSignalSpy variantSpy(sel, &AntSelect::variantChanged);
    sel->setVariant(Ant::SelectVariant::Filled);
    QCOMPARE(sel->variant(), Ant::SelectVariant::Filled);
    QCOMPARE(variantSpy.count(), 1);

    // Option inspection
    AntSelectOption opt0 = sel->optionAt(0);
    QCOMPARE(opt0.label, "Apple");

    sel->clearOptions();
    QCOMPARE(sel->count(), 0);

    // sizeHint
    QSize hint = sel->sizeHint();
    QVERIFY(hint.width() > 0);
    QVERIFY(hint.height() > 0);
}

QTEST_MAIN(TestAntSelect)
#include "TestAntSelect.moc"
