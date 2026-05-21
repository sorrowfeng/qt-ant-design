#include <QSignalSpy>
#include <QTest>
#include <QComboBox>
#include <QCoreApplication>
#include <QLineEdit>
#include <QVBoxLayout>

#include "widgets/AntSelect.h"

class TestAntSelect : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
    void defaultsToFirstOptionWhenPopulated();
    void layoutKeepsSelectControlHeight();
    void popupReusesRowsAndScopesUpdates();
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
    QCOMPARE(sel->selectSize(), Ant::Size::Middle);
    QCOMPARE(sel->status(), Ant::Status::Normal);
    QCOMPARE(sel->variant(), Ant::Variant::Outlined);

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
    sel->setSelectSize(Ant::Size::Large);
    QCOMPARE(sel->selectSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy statusSpy(sel, &AntSelect::statusChanged);
    sel->setStatus(Ant::Status::Error);
    QCOMPARE(sel->status(), Ant::Status::Error);
    QCOMPARE(statusSpy.count(), 1);

    QSignalSpy variantSpy(sel, &AntSelect::variantChanged);
    sel->setVariant(Ant::Variant::Filled);
    QCOMPARE(sel->variant(), Ant::Variant::Filled);
    QCOMPARE(variantSpy.count(), 1);

    // Option inspection
    AntSelectOption opt0 = sel->optionAt(0);
    QCOMPARE(opt0.label, "Apple");

    sel->clearOptions();
    QCOMPARE(sel->count(), 0);

    // QComboBox-style compatibility helpers
    auto* combo = new AntSelect;
    combo->addItem(QStringLiteral("One"), 1);
    combo->addItems({QStringLiteral("Two"), QStringLiteral("Three")});
    combo->insertItem(1, QStringLiteral("Inserted"), 99);
    QCOMPARE(combo->count(), 4);
    QCOMPARE(combo->itemText(1), QStringLiteral("Inserted"));
    QCOMPARE(combo->itemData(1).toInt(), 99);
    QCOMPARE(combo->findText(QStringLiteral("Two")), 2);
    QCOMPARE(combo->findData(99), 1);
    QCOMPARE(combo->optionData(1).toInt(), 99);

    combo->setOptionText(1, QStringLiteral("Option Renamed"));
    QCOMPARE(combo->itemText(1), QStringLiteral("Option Renamed"));
    combo->setItemText(1, QStringLiteral("Renamed"));
    combo->setItemData(1, 100);
    QCOMPARE(combo->itemText(1), QStringLiteral("Renamed"));
    QCOMPARE(combo->itemData(1).toInt(), 100);
    QCOMPARE(combo->optionData(1).toInt(), 100);

    QSignalSpy comboIndexSpy(combo, &AntSelect::currentIndexChanged);
    combo->setCurrentText(QStringLiteral("Three"));
    QCOMPARE(combo->currentIndex(), 3);
    QCOMPARE(combo->currentData().toString(), QStringLiteral("Three"));
    QCOMPARE(comboIndexSpy.count(), 1);

    combo->removeOption(3);
    QCOMPARE(combo->count(), 3);
    QCOMPARE(combo->findText(QStringLiteral("Three")), -1);

    combo->clear();
    QCOMPARE(combo->count(), 0);

    auto* activatedSelect = new AntSelect;
    activatedSelect->addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
    QSignalSpy activatedSpy(activatedSelect, &AntSelect::activated);
    QSignalSpy textActivatedSpy(activatedSelect, &AntSelect::textActivated);
    QSignalSpy highlightedSpy(activatedSelect, &AntSelect::highlighted);
    QSignalSpy textHighlightedSpy(activatedSelect, &AntSelect::textHighlighted);
    activatedSelect->setOpen(true);
    QTest::keyClick(activatedSelect, Qt::Key_Down);
    QVERIFY(highlightedSpy.count() >= 1);
    QVERIFY(textHighlightedSpy.count() >= 1);
    QTest::keyClick(activatedSelect, Qt::Key_Return);
    QCOMPARE(activatedSpy.count(), 1);
    QCOMPARE(textActivatedSpy.count(), 1);
    QCOMPARE(activatedSpy.takeFirst().at(0).toInt(), activatedSelect->currentIndex());
    QCOMPARE(textActivatedSpy.takeFirst().at(0).toString(), activatedSelect->currentText());

    // sizeHint
    QSize hint = sel->sizeHint();
    QVERIFY(hint.width() > 0);
    QVERIFY(hint.height() > 0);
}

void TestAntSelect::defaultsToFirstOptionWhenPopulated()
{
    AntSelect added;
    QSignalSpy addedIndexSpy(&added, &AntSelect::currentIndexChanged);
    QSignalSpy addedTextSpy(&added, &AntSelect::currentTextChanged);
    QSignalSpy addedValueSpy(&added, &AntSelect::currentValueChanged);

    added.addOption(QStringLiteral("First"), QStringLiteral("first"));
    QCOMPARE(added.currentIndex(), 0);
    QCOMPARE(added.currentText(), QStringLiteral("First"));
    QCOMPARE(added.currentValue().toString(), QStringLiteral("first"));
    QCOMPARE(addedIndexSpy.count(), 1);
    QCOMPARE(addedTextSpy.count(), 1);
    QCOMPARE(addedValueSpy.count(), 1);

    added.addOption(QStringLiteral("Second"), QStringLiteral("second"));
    QCOMPARE(added.currentIndex(), 0);
    QCOMPARE(addedIndexSpy.count(), 1);

    AntSelect inserted;
    inserted.insertItem(0, QStringLiteral("Inserted"), QStringLiteral("inserted"));
    QCOMPARE(inserted.currentIndex(), 0);
    QCOMPARE(inserted.currentText(), QStringLiteral("Inserted"));
    QCOMPARE(inserted.currentValue().toString(), QStringLiteral("inserted"));
}

void TestAntSelect::layoutKeepsSelectControlHeight()
{
    QWidget host;
    auto* layout = new QVBoxLayout(&host);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* select = new AntSelect;
    select->addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
    layout->addWidget(select, 1);

    host.resize(260, 180);
    host.show();
    QCoreApplication::processEvents();

    QComboBox nativeComboBox;
    QCOMPARE(select->sizePolicy().horizontalPolicy(), nativeComboBox.sizePolicy().horizontalPolicy());
    QCOMPARE(select->sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
    QCOMPARE(select->height(), select->sizeHint().height());
    QCOMPARE(select->width(), host.width());
}

void TestAntSelect::popupReusesRowsAndScopesUpdates()
{
    AntSelect select;
    select.addItems({QStringLiteral("Alpha"),
                     QStringLiteral("Beta"),
                     QStringLiteral("Gamma"),
                     QStringLiteral("Delta")});
    select.resize(select.sizeHint());
    select.setOpen(true);
    QCoreApplication::processEvents();

    const int rowCreatesAfterOpen = select.property("antSelectPopupRowCreateCount").toInt();
    QVERIFY(rowCreatesAfterOpen >= 4);
    QVERIFY(select.property("antSelectPopupGeometryApplyCount").toInt() >= 1);
    QVERIFY(select.property("antSelectPopupGeometrySkipCount").toInt() >= 1);

    const int metricsBefore = select.property("antSelectMetricsResolveCount").toInt();
    const int sizeHintsBefore = select.property("antSelectSizeHintResolveCount").toInt();
    select.sizeHint();
    select.minimumSizeHint();
    select.sizeHint();
    QCOMPARE(select.property("antSelectMetricsResolveCount").toInt(), metricsBefore);
    QCOMPARE(select.property("antSelectSizeHintResolveCount").toInt(), sizeHintsBefore);

    const int rowUpdatesBeforeSelection = select.property("antSelectPopupRowUpdateCount").toInt();
    select.setCurrentIndex(2);
    QVERIFY(select.property("antSelectPopupRowUpdateCount").toInt() > rowUpdatesBeforeSelection);
    QCOMPARE(select.property("antSelectPopupRowCreateCount").toInt(), rowCreatesAfterOpen);

    const int rowUpdatesBeforeHighlight = select.property("antSelectPopupRowUpdateCount").toInt();
    QTest::keyClick(&select, Qt::Key_Down);
    QVERIFY(select.property("antSelectPopupRowUpdateCount").toInt() > rowUpdatesBeforeHighlight);

    const int rebuildsBeforeFilter = select.property("antSelectPopupRebuildCount").toInt();
    select.setEditable(true);
    auto* edit = select.findChild<QLineEdit*>();
    QVERIFY(edit);
    edit->setText(QStringLiteral("alp"));
    QCoreApplication::processEvents();
    QVERIFY(select.property("antSelectPopupRebuildCount").toInt() > rebuildsBeforeFilter);
    QCOMPARE(select.property("antSelectPopupRowCreateCount").toInt(), rowCreatesAfterOpen);
}

QTEST_MAIN(TestAntSelect)
#include "TestAntSelect.moc"
