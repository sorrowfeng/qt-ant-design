#include <QSignalSpy>
#include <QTest>
#include "widgets/AntCascader.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntTimePicker.h"
#include "widgets/AntMentions.h"
#include "widgets/AntTransfer.h"
#include "widgets/AntTreeSelect.h"
#include "widgets/AntUpload.h"

class TestAntDataEntryB : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
};

void TestAntDataEntryB::propertiesAndSignals()
{
    // AntCascader
    auto* w1 = new AntCascader;
    QCOMPARE(w1->cascaderSize(), Ant::Size::Middle);
    QCOMPARE(w1->status(), Ant::Status::Normal);
    QCOMPARE(w1->variant(), Ant::Variant::Outlined);
    QCOMPARE(w1->expandTrigger(), Ant::Trigger::Click);
    QCOMPARE(w1->isOpen(), false);
    QCOMPARE(w1->allowClear(), false);

    QSignalSpy sizeSpy1(w1, &AntCascader::cascaderSizeChanged);
    w1->setCascaderSize(Ant::Size::Large);
    QCOMPARE(w1->cascaderSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy1.count(), 1);

    QSignalSpy statusSpy1(w1, &AntCascader::statusChanged);
    w1->setStatus(Ant::Status::Error);
    QCOMPARE(w1->status(), Ant::Status::Error);
    QCOMPARE(statusSpy1.count(), 1);

    QSignalSpy triggerSpy(w1, &AntCascader::expandTriggerChanged);
    w1->setExpandTrigger(Ant::Trigger::Hover);
    QCOMPARE(w1->expandTrigger(), Ant::Trigger::Hover);
    QCOMPARE(triggerSpy.count(), 1);

    QSignalSpy clearSpy1(w1, &AntCascader::allowClearChanged);
    w1->setAllowClear(true);
    QCOMPARE(w1->allowClear(), true);
    QCOMPARE(clearSpy1.count(), 1);

    w1->setPlaceholder("Select...");
    QCOMPARE(w1->placeholder(), "Select...");

    // AntDatePicker
    auto* w2 = new AntDatePicker;
    QCOMPARE(w2->pickerSize(), Ant::Size::Middle);
    QCOMPARE(w2->status(), Ant::Status::Normal);
    QCOMPARE(w2->variant(), Ant::Variant::Outlined);
    QCOMPARE(w2->allowClear(), true);
    QCOMPARE(w2->isOpen(), false);
    QCOMPARE(w2->hasSelectedDate(), false);
    QCOMPARE(w2->isHoveredState(), false);

    QSignalSpy sizeSpy2(w2, &AntDatePicker::pickerSizeChanged);
    w2->setPickerSize(Ant::Size::Large);
    QCOMPARE(w2->pickerSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy2.count(), 1);

    QSignalSpy statusSpy2(w2, &AntDatePicker::statusChanged);
    w2->setStatus(Ant::Status::Error);
    QCOMPARE(w2->status(), Ant::Status::Error);
    QCOMPARE(statusSpy2.count(), 1);

    QSignalSpy dateSpy(w2, &AntDatePicker::selectedDateChanged);
    w2->setSelectedDate(QDate(2026, 4, 26));
    QCOMPARE(w2->selectedDate(), QDate(2026, 4, 26));
    QCOMPARE(w2->hasSelectedDate(), true);
    QCOMPARE(dateSpy.count(), 1);

    QSignalSpy clearSpy2(w2, &AntDatePicker::allowClearChanged);
    w2->setAllowClear(false);
    QCOMPARE(w2->allowClear(), false);
    QCOMPARE(clearSpy2.count(), 1);

    w2->setDisplayFormat("yyyy-MM-dd");
    QCOMPARE(w2->displayFormat(), "yyyy-MM-dd");

    auto* dateRange = new AntDatePicker;
    dateRange->setRangeMode(true);
    dateRange->setStartDate(QDate(2026, 4, 1));
    dateRange->setEndDate(QDate(2026, 4, 8));
    QCOMPARE(dateRange->hasSelectedDate(), true);
    QCOMPARE(dateRange->dateString(), QStringLiteral("2026-04-01 - 2026-04-08"));
    dateRange->clear();
    QCOMPARE(dateRange->hasSelectedDate(), false);

    // AntTimePicker
    auto* w3 = new AntTimePicker;
    QCOMPARE(w3->pickerSize(), Ant::Size::Middle);
    QCOMPARE(w3->status(), Ant::Status::Normal);
    QCOMPARE(w3->variant(), Ant::Variant::Outlined);
    QCOMPARE(w3->allowClear(), true);
    QCOMPARE(w3->isOpen(), false);
    QCOMPARE(w3->hasSelectedTime(), false);
    QCOMPARE(w3->isHoveredState(), false);

    QSignalSpy sizeSpy3(w3, &AntTimePicker::pickerSizeChanged);
    w3->setPickerSize(Ant::Size::Small);
    QCOMPARE(w3->pickerSize(), Ant::Size::Small);
    QCOMPARE(sizeSpy3.count(), 1);

    QSignalSpy timeSpy(w3, &AntTimePicker::selectedTimeChanged);
    w3->setSelectedTime(QTime(14, 30, 0));
    QCOMPARE(w3->selectedTime(), QTime(14, 30, 0));
    QCOMPARE(w3->hasSelectedTime(), true);
    QCOMPARE(timeSpy.count(), 1);

    QSignalSpy hourSpy(w3, &AntTimePicker::hourStepChanged);
    w3->setHourStep(2);
    QCOMPARE(w3->hourStep(), 2);
    QCOMPARE(hourSpy.count(), 1);

    QSignalSpy minSpy(w3, &AntTimePicker::minuteStepChanged);
    w3->setMinuteStep(15);
    QCOMPARE(w3->minuteStep(), 15);
    QCOMPARE(minSpy.count(), 1);

    auto* timeRange = new AntTimePicker;
    timeRange->setRangeMode(true);
    timeRange->setDisplayFormat(QStringLiteral("HH:mm"));
    timeRange->setStartTime(QTime(9, 0));
    timeRange->setEndTime(QTime(18, 30));
    QCOMPARE(timeRange->hasSelectedTime(), true);
    QCOMPARE(timeRange->timeString(), QStringLiteral("09:00 - 18:30"));
    timeRange->clear();
    QCOMPARE(timeRange->hasSelectedTime(), false);

    // AntMentions
    auto* w4 = new AntMentions;
    QCOMPARE(w4->text(), QString());
    QCOMPARE(w4->prefix(), "@");

    QSignalSpy prefixSpy(w4, &AntMentions::prefixChanged);
    w4->setPrefix("#");
    QCOMPARE(w4->prefix(), "#");
    QCOMPARE(prefixSpy.count(), 1);

    w4->addSuggestion("alice");
    w4->addSuggestion("bob");
    w4->setSuggestions({"charlie", "dave"});

    // AntTransfer
    auto* w5 = new AntTransfer;
    QCOMPARE(w5->sourceItems().isEmpty(), true);
    QCOMPARE(w5->targetItems().isEmpty(), true);

    w5->setSourceItems({"A", "B", "C"});
    QCOMPARE(w5->sourceItems().size(), 3);

    QSignalSpy itemsSpy(w5, &AntTransfer::itemsChanged);
    w5->setTargetItems({"A"});
    QCOMPARE(w5->targetItems().size(), 1);
    QVERIFY(itemsSpy.count() >= 1);

    // AntTreeSelect
    auto* w6 = new AntTreeSelect;
    QCOMPARE(w6->selectSize(), Ant::Size::Middle);
    QCOMPARE(w6->status(), Ant::Status::Normal);
    QCOMPARE(w6->variant(), Ant::Variant::Outlined);
    QCOMPARE(w6->isMultiple(), false);
    QCOMPARE(w6->isTreeCheckable(), false);
    QCOMPARE(w6->isOpen(), false);

    QSignalSpy sizeSpy6(w6, &AntTreeSelect::selectSizeChanged);
    w6->setSelectSize(Ant::Size::Large);
    QCOMPARE(w6->selectSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy6.count(), 1);

    QSignalSpy multiSpy6(w6, &AntTreeSelect::multipleChanged);
    w6->setMultiple(true);
    QCOMPARE(w6->isMultiple(), true);
    QCOMPARE(multiSpy6.count(), 1);

    QSignalSpy checkSpy6(w6, &AntTreeSelect::treeCheckableChanged);
    w6->setTreeCheckable(true);
    QCOMPARE(w6->isTreeCheckable(), true);
    QCOMPARE(checkSpy6.count(), 1);

    w6->setPlaceholder("Select tree...");
    QCOMPARE(w6->placeholder(), "Select tree...");

    // AntUpload
    auto* w7 = new AntUpload;
    QCOMPARE(w7->isMultiple(), false);
    QCOMPARE(w7->maxCount(), 0);
    QCOMPARE(w7->isDisabled(), false);
    QCOMPARE(w7->listType(), Ant::UploadListType::Text);
    QCOMPARE(w7->isDraggerMode(), false);

    QSignalSpy multiSpy7(w7, &AntUpload::multipleChanged);
    w7->setMultiple(true);
    QCOMPARE(w7->isMultiple(), true);
    QCOMPARE(multiSpy7.count(), 1);

    QSignalSpy maxSpy7(w7, &AntUpload::maxCountChanged);
    w7->setMaxCount(5);
    QCOMPARE(w7->maxCount(), 5);
    QCOMPARE(maxSpy7.count(), 1);

    QSignalSpy disabledSpy7(w7, &AntUpload::disabledChanged);
    w7->setDisabled(true);
    QCOMPARE(w7->isDisabled(), true);
    QCOMPARE(disabledSpy7.count(), 1);

    QSignalSpy typeSpy7(w7, &AntUpload::listTypeChanged);
    w7->setListType(Ant::UploadListType::PictureCard);
    QCOMPARE(w7->listType(), Ant::UploadListType::PictureCard);
    QCOMPARE(typeSpy7.count(), 1);

    QSignalSpy draggerSpy(w7, &AntUpload::draggerModeChanged);
    w7->setDraggerMode(true);
    QCOMPARE(w7->isDraggerMode(), true);
    QCOMPARE(draggerSpy.count(), 1);

    w7->setAccept(".jpg,.png");
    QCOMPARE(w7->accept(), ".jpg,.png");
}

QTEST_MAIN(TestAntDataEntryB)
#include "TestAntDataEntryB.moc"
