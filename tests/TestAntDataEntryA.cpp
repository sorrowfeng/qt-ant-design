#include <QSignalSpy>
#include <QTest>
#include "widgets/AntInputNumber.h"
#include "widgets/AntRadio.h"
#include "widgets/AntSlider.h"
#include "widgets/AntRate.h"
#include "widgets/AntSegmented.h"
#include "widgets/AntAutoComplete.h"

class TestAntDataEntryA : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
};

void TestAntDataEntryA::propertiesAndSignals()
{
    // AntInputNumber
    auto* w1 = new AntInputNumber;
    QCOMPARE(w1->inputSize(), Ant::InputSize::Middle);
    QCOMPARE(w1->status(), Ant::InputStatus::Normal);
    QCOMPARE(w1->variant(), Ant::InputNumberVariant::Outlined);
    QCOMPARE(w1->controlsVisible(), true);

    QSignalSpy sizeSpy1(w1, &AntInputNumber::inputSizeChanged);
    w1->setInputSize(Ant::InputSize::Large);
    QCOMPARE(w1->inputSize(), Ant::InputSize::Large);
    QCOMPARE(sizeSpy1.count(), 1);

    QSignalSpy statusSpy1(w1, &AntInputNumber::statusChanged);
    w1->setStatus(Ant::InputStatus::Error);
    QCOMPARE(w1->status(), Ant::InputStatus::Error);
    QCOMPARE(statusSpy1.count(), 1);

    QSignalSpy varSpy1(w1, &AntInputNumber::variantChanged);
    w1->setVariant(Ant::InputNumberVariant::Filled);
    QCOMPARE(w1->variant(), Ant::InputNumberVariant::Filled);
    QCOMPARE(varSpy1.count(), 1);

    QSignalSpy ctrlSpy1(w1, &AntInputNumber::controlsVisibleChanged);
    w1->setControlsVisible(false);
    QCOMPARE(w1->controlsVisible(), false);
    QCOMPARE(ctrlSpy1.count(), 1);

    // AntRadio
    auto* w2 = new AntRadio;
    QCOMPARE(w2->isChecked(), false);
    QCOMPARE(w2->text(), QString());
    QCOMPARE(w2->autoExclusive(), true);

    QSignalSpy checkedSpy(w2, &AntRadio::checkedChanged);
    w2->setChecked(true);
    QCOMPARE(w2->isChecked(), true);
    QCOMPARE(checkedSpy.count(), 1);

    QSignalSpy textSpy(w2, &AntRadio::textChanged);
    w2->setText("Option A");
    QCOMPARE(w2->text(), "Option A");
    QCOMPARE(textSpy.count(), 1);

    QSignalSpy valueSpy(w2, &AntRadio::valueChanged);
    w2->setValue(42);
    QCOMPARE(w2->value().toInt(), 42);
    QCOMPARE(valueSpy.count(), 1);

    // AntSlider
    auto* w3 = new AntSlider;
    QCOMPARE(w3->minimum(), 0);
    QCOMPARE(w3->maximum(), 100);
    QCOMPARE(w3->value(), 0);
    QCOMPARE(w3->orientation(), Qt::Horizontal);
    QCOMPARE(w3->isReverse(), false);
    QCOMPARE(w3->dots(), false);

    QSignalSpy valSpy3(w3, &AntSlider::valueChanged);
    w3->setValue(50);
    QCOMPARE(w3->value(), 50);
    QCOMPARE(valSpy3.count(), 1);

    QSignalSpy rangeSpy(w3, &AntSlider::rangeChanged);
    w3->setRange(10, 90);
    QCOMPARE(w3->minimum(), 10);
    QCOMPARE(w3->maximum(), 90);
    QCOMPARE(rangeSpy.count(), 1);

    QSignalSpy orientSpy(w3, &AntSlider::orientationChanged);
    w3->setOrientation(Qt::Vertical);
    QCOMPARE(w3->orientation(), Qt::Vertical);
    QCOMPARE(orientSpy.count(), 1);

    w3->setDots(true);
    QCOMPARE(w3->dots(), true);
    w3->setReverse(true);
    QCOMPARE(w3->isReverse(), true);

    // AntRate
    auto* w4 = new AntRate;
    QCOMPARE(w4->value(), 0.0);
    QCOMPARE(w4->count(), 5);
    QCOMPARE(w4->allowHalf(), false);
    QCOMPARE(w4->allowClear(), true);
    QCOMPARE(w4->isDisabled(), false);
    QCOMPARE(w4->rateSize(), Ant::RateSize::Middle);

    QSignalSpy halfSpy(w4, &AntRate::allowHalfChanged);
    w4->setAllowHalf(true);
    QCOMPARE(w4->allowHalf(), true);
    QCOMPARE(halfSpy.count(), 1);

    QSignalSpy valSpy4(w4, &AntRate::valueChanged);
    w4->setValue(3.5);
    QCOMPARE(w4->value(), 3.5);
    QCOMPARE(valSpy4.count(), 1);

    QSignalSpy countSpy(w4, &AntRate::countChanged);
    w4->setCount(10);
    QCOMPARE(w4->count(), 10);
    QCOMPARE(countSpy.count(), 1);

    QSignalSpy disabledSpy4(w4, &AntRate::disabledChanged);
    w4->setDisabled(true);
    QCOMPARE(w4->isDisabled(), true);
    QCOMPARE(disabledSpy4.count(), 1);

    QSignalSpy sizeSpy4(w4, &AntRate::rateSizeChanged);
    w4->setRateSize(Ant::RateSize::Small);
    QCOMPARE(w4->rateSize(), Ant::RateSize::Small);
    QCOMPARE(sizeSpy4.count(), 1);

    // AntSegmented
    auto* w5 = new AntSegmented;
    QCOMPARE(w5->value(), QString());
    QCOMPARE(w5->isBlock(), false);
    QCOMPARE(w5->segmentedSize(), Ant::SegmentedSize::Middle);
    QCOMPARE(w5->isVertical(), false);
    QCOMPARE(w5->shape(), Ant::SegmentedShape::Default);

    QVector<AntSegmentedOption> opts = {{"A", "A"}, {"B", "B"}, {"C", "C"}};
    w5->setOptions(opts);
    QCOMPARE(w5->options().size(), 3);

    QSignalSpy valueSpy5(w5, &AntSegmented::valueChanged);
    w5->setValue("B");
    QCOMPARE(w5->value(), "B");
    QCOMPARE(valueSpy5.count(), 1);

    QSignalSpy blockSpy(w5, &AntSegmented::blockChanged);
    w5->setBlock(true);
    QCOMPARE(w5->isBlock(), true);
    QCOMPARE(blockSpy.count(), 1);

    QSignalSpy sizeSpy5(w5, &AntSegmented::segmentedSizeChanged);
    w5->setSegmentedSize(Ant::SegmentedSize::Large);
    QCOMPARE(w5->segmentedSize(), Ant::SegmentedSize::Large);
    QCOMPARE(sizeSpy5.count(), 1);

    QSignalSpy vertSpy(w5, &AntSegmented::verticalChanged);
    w5->setVertical(true);
    QCOMPARE(w5->isVertical(), true);
    QCOMPARE(vertSpy.count(), 1);

    QSignalSpy shapeSpy(w5, &AntSegmented::shapeChanged);
    w5->setShape(Ant::SegmentedShape::Round);
    QCOMPARE(w5->shape(), Ant::SegmentedShape::Round);
    QCOMPARE(shapeSpy.count(), 1);

    // AntAutoComplete
    auto* w6 = new AntAutoComplete;
    QCOMPARE(w6->text(), QString());
    QCOMPARE(w6->maxVisibleItems(), 8);

    w6->setText("test");
    QCOMPARE(w6->text(), "test");
    // Note: textChanged signal is only emitted on user input (textEdited), not programmatic setText

    QSignalSpy maxSpy(w6, &AntAutoComplete::maxVisibleItemsChanged);
    w6->setMaxVisibleItems(10);
    QCOMPARE(w6->maxVisibleItems(), 10);
    QCOMPARE(maxSpy.count(), 1);

    w6->addSuggestion("Apple", "apple");
    w6->addSuggestion("Banana", "banana");
    QCOMPARE(w6->suggestionCount(), 2);

    w6->clearSuggestions();
    QCOMPARE(w6->suggestionCount(), 0);
}

QTEST_MAIN(TestAntDataEntryA)
#include "TestAntDataEntryA.moc"
