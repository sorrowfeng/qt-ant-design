#include <QSignalSpy>
#include <QTest>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QEnterEvent>
#include <QHBoxLayout>
#include <QImage>
#include <QLineEdit>
#include <QPainter>
#include <QPalette>
#include <QVariantAnimation>
#include "core/AntTheme.h"
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
    void inputNumberDefaultsToIntegerAndEnablesDecimals();
    void inputNumberLineEditPaletteTracksDarkTheme();
    void inputNumberUsesLayoutFriendlyPolicy();
    void inputNumberCachesMetricsAndScopesControlUpdates();
    void radioCachesLayoutAndScopesStateUpdates();
    void rateCachesStarLayoutAndScopesUpdates();
    void segmentedCachesLayoutAndScopesUpdates();
    void sliderCachesGeometryAndScopesUpdates();
    void sliderMarksReserveLabelHeight();
    void sliderBubbleFloatsAboveMarkedHandle();
    void sliderBubbleArrowJoinsSurface();
    void sliderRangeDragDoesNotPaintPhantomMinimumHandle();
    void segmentedClicksUseFullVisualTrack();
    void rateSelectionStartsScaleAnimation();
    void autoCompleteReusesPopupItemsAndScopesHighlight();
};

namespace
{
QImage renderWidgetImage(QWidget& widget)
{
    widget.ensurePolished();
    QCoreApplication::sendPostedEvents(&widget, QEvent::Polish);
    QCoreApplication::processEvents();

    QImage image(widget.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    widget.render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    return image;
}

int sliderVisualHandleCenterY()
{
    const auto& token = antTheme->tokens();
    const int handleSizeHover = token.controlHeightSM / 2;
    return handleSizeHover / 2 + 2;
}
} // namespace

void TestAntDataEntryA::propertiesAndSignals()
{
    // AntInputNumber
    auto* w1 = new AntInputNumber;
    QCOMPARE(w1->inputSize(), Ant::Size::Middle);
    QCOMPARE(w1->status(), Ant::Status::Normal);
    QCOMPARE(w1->variant(), Ant::Variant::Outlined);
    QCOMPARE(w1->controlsVisible(), true);

    QSignalSpy sizeSpy1(w1, &AntInputNumber::inputSizeChanged);
    w1->setInputSize(Ant::Size::Large);
    QCOMPARE(w1->inputSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy1.count(), 1);

    QSignalSpy statusSpy1(w1, &AntInputNumber::statusChanged);
    w1->setStatus(Ant::Status::Error);
    QCOMPARE(w1->status(), Ant::Status::Error);
    QCOMPARE(statusSpy1.count(), 1);

    QSignalSpy varSpy1(w1, &AntInputNumber::variantChanged);
    w1->setVariant(Ant::Variant::Filled);
    QCOMPARE(w1->variant(), Ant::Variant::Filled);
    QCOMPARE(varSpy1.count(), 1);

    QSignalSpy ctrlSpy1(w1, &AntInputNumber::controlsVisibleChanged);
    w1->setControlsVisible(false);
    QCOMPARE(w1->controlsVisible(), false);
    QCOMPARE(ctrlSpy1.count(), 1);

    QSignalSpy prefixSpy1(w1, &AntInputNumber::prefixTextChanged);
    w1->setPrefixText(QStringLiteral("$"));
    QCOMPARE(w1->prefixText(), QStringLiteral("$"));
    QCOMPARE(w1->prefix(), QStringLiteral("$"));
    QCOMPARE(prefixSpy1.count(), 1);

    QSignalSpy suffixSpy1(w1, &AntInputNumber::suffixTextChanged);
    w1->setSuffixText(QStringLiteral(" kg"));
    QCOMPARE(w1->suffixText(), QStringLiteral(" kg"));
    QCOMPARE(w1->suffix(), QStringLiteral(" kg"));
    QCOMPARE(suffixSpy1.count(), 1);

    QSignalSpy precisionSpy1(w1, &AntInputNumber::precisionChanged);
    w1->setPrecision(3);
    QCOMPARE(w1->precision(), 3);
    QCOMPARE(w1->decimals(), 3);
    QCOMPARE(precisionSpy1.count(), 1);

    w1->setRange(-5.0, 5.0);
    w1->setSingleStep(0.5);
    w1->setValue(4.75);
    w1->stepBy(1);
    QCOMPARE(w1->value(), 5.0);
    w1->stepBy(-2);
    QCOMPARE(w1->value(), 4.0);

    w1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    w1->setInputSize(Ant::Size::Small);
    QCOMPARE(w1->alignment() & Qt::AlignHorizontal_Mask, Qt::AlignRight);
    w1->setVariant(Ant::Variant::Outlined);
    QCOMPARE(w1->alignment() & Qt::AlignHorizontal_Mask, Qt::AlignRight);

    auto* animatedInput = new AntInputNumber;
    QCOMPARE(animatedInput->controlsProgress(), 0.0);
    QEnterEvent enterEvent(QPointF(4, 4), QPointF(4, 4), QPointF(4, 4));
    QCoreApplication::sendEvent(animatedInput, &enterEvent);
    QTRY_VERIFY_WITH_TIMEOUT(animatedInput->controlsProgress() > 0.95, 300);
    QEvent leaveEvent(QEvent::Leave);
    QCoreApplication::sendEvent(animatedInput, &leaveEvent);
    QTRY_VERIFY_WITH_TIMEOUT(animatedInput->controlsProgress() < 0.05, 300);

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

    QSignalSpy radioClickSpy(w2, &AntRadio::clicked);
    w2->setChecked(false);
    w2->click();
    QCOMPARE(w2->isChecked(), true);
    QCOMPARE(radioClickSpy.count(), 1);

    // AntSlider
    auto* w3 = new AntSlider;
    QCOMPARE(w3->minimum(), 0);
    QCOMPARE(w3->maximum(), 100);
    QCOMPARE(w3->value(), 0);
    QCOMPARE(w3->orientation(), Qt::Horizontal);
    QCOMPARE(w3->isReverse(), false);
    QCOMPARE(w3->invertedAppearance(), false);
    QCOMPARE(w3->hasTracking(), true);
    QCOMPARE(w3->pageStep(), 10);
    QCOMPARE(w3->sliderPosition(), 0);
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
    QCOMPARE(w3->invertedAppearance(), true);

    QSignalSpy pageStepSpy(w3, &AntSlider::pageStepChanged);
    w3->setPageStep(12);
    QCOMPARE(w3->pageStep(), 12);
    QCOMPARE(pageStepSpy.count(), 1);

    QSignalSpy trackingSpy(w3, &AntSlider::trackingChanged);
    w3->setTracking(false);
    QCOMPARE(w3->hasTracking(), false);
    QCOMPARE(trackingSpy.count(), 1);

    w3->setSliderPosition(42);
    QCOMPARE(w3->sliderPosition(), 42);
    QCOMPARE(w3->value(), 42);

    auto* sliderWithBubble = new AntSlider;
    sliderWithBubble->resize(220, 44);
    sliderWithBubble->show();
    QVERIFY(QTest::qWaitForWindowExposed(sliderWithBubble));
    QTest::mousePress(sliderWithBubble, Qt::LeftButton, Qt::NoModifier, QPoint(120, 22));
    auto* valueBubble = sliderWithBubble->findChild<QWidget*>(QStringLiteral("antSliderValueBubble"));
    QVERIFY(valueBubble);
    QTRY_VERIFY_WITH_TIMEOUT(valueBubble->isVisible(), 100);
    QTest::mouseRelease(sliderWithBubble, Qt::LeftButton, Qt::NoModifier, QPoint(120, 22));
    QTRY_VERIFY_WITH_TIMEOUT(!valueBubble->isVisible(), 100);

    // AntRate
    auto* w4 = new AntRate;
    QCOMPARE(w4->value(), 0.0);
    QCOMPARE(w4->count(), 5);
    QCOMPARE(w4->allowHalf(), false);
    QCOMPARE(w4->allowClear(), true);
    QCOMPARE(w4->isDisabled(), false);
    QCOMPARE(w4->rateSize(), Ant::Size::Middle);

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
    w4->setRateSize(Ant::Size::Small);
    QCOMPARE(w4->rateSize(), Ant::Size::Small);
    QCOMPARE(sizeSpy4.count(), 1);

    // AntSegmented
    auto* w5 = new AntSegmented;
    QCOMPARE(w5->value(), QString());
    QCOMPARE(w5->currentIndex(), -1);
    QCOMPARE(w5->isBlock(), false);
    QCOMPARE(w5->segmentedSize(), Ant::Size::Middle);
    QCOMPARE(w5->isVertical(), false);
    QCOMPARE(w5->shape(), Ant::SegmentedShape::Default);

    QVector<AntSegmentedOption> opts = {{"A", "A"}, {"B", "B"}, {"C", "C"}};
    w5->setOptions(opts);
    QCOMPARE(w5->options().size(), 3);
    QCOMPARE(w5->currentIndex(), 0);
    QCOMPARE(w5->selectedIndex(), 0);

    QSignalSpy currentIndexSpy5(w5, &AntSegmented::currentIndexChanged);
    w5->setCurrentIndex(2);
    QCOMPARE(w5->currentIndex(), 2);
    QCOMPARE(w5->selectedIndex(), 2);
    QCOMPARE(w5->value(), QStringLiteral("C"));
    QCOMPARE(currentIndexSpy5.count(), 1);
    QCOMPARE(currentIndexSpy5.last().at(0).toInt(), 2);

    w5->setCurrentIndex(2);
    QCOMPARE(currentIndexSpy5.count(), 1);
    w5->setCurrentIndex(-1);
    QCOMPARE(w5->currentIndex(), 2);
    QCOMPARE(currentIndexSpy5.count(), 1);

    AntSegmented disabledSegment;
    disabledSegment.setOptions({{"A", "A"}, {"B", "B", QString(), true}});
    QSignalSpy disabledIndexSpy(&disabledSegment, &AntSegmented::currentIndexChanged);
    disabledSegment.setCurrentIndex(1);
    QCOMPARE(disabledSegment.currentIndex(), 0);
    QCOMPARE(disabledIndexSpy.count(), 0);

    QSignalSpy valueSpy5(w5, &AntSegmented::valueChanged);
    w5->setValue("B");
    QCOMPARE(w5->value(), "B");
    QCOMPARE(w5->currentIndex(), 1);
    QCOMPARE(valueSpy5.count(), 1);
    QCOMPARE(currentIndexSpy5.count(), 2);
    QCOMPARE(currentIndexSpy5.last().at(0).toInt(), 1);

    w5->resize(w5->sizeHint());
    const QPoint thirdSegmentPoint(w5->width() - 8, w5->height() / 2);
    QTest::mousePress(w5, Qt::LeftButton, Qt::NoModifier, thirdSegmentPoint);
    QCOMPARE(w5->pressedIndex(), 2);
    QCOMPARE(w5->value(), "B");
    QTest::mouseRelease(w5, Qt::LeftButton, Qt::NoModifier, thirdSegmentPoint);
    QCOMPARE(w5->pressedIndex(), -1);
    QCOMPARE(w5->value(), "C");
    QCOMPARE(w5->currentIndex(), 2);
    QCOMPARE(valueSpy5.count(), 2);
    QCOMPARE(currentIndexSpy5.count(), 3);
    QCOMPARE(currentIndexSpy5.last().at(0).toInt(), 2);

    QSignalSpy blockSpy(w5, &AntSegmented::blockChanged);
    w5->setBlock(true);
    QCOMPARE(w5->isBlock(), true);
    QCOMPARE(blockSpy.count(), 1);

    QSignalSpy sizeSpy5(w5, &AntSegmented::segmentedSizeChanged);
    w5->setSegmentedSize(Ant::Size::Large);
    QCOMPARE(w5->segmentedSize(), Ant::Size::Large);
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

void TestAntDataEntryA::inputNumberDefaultsToIntegerAndEnablesDecimals()
{
    AntInputNumber input;
    QCOMPARE(input.decimals(), 0);
    QCOMPARE(input.precision(), 0);

    QSignalSpy valueSpy(&input, qOverload<double>(&QDoubleSpinBox::valueChanged));
    input.setRange(-10.0, 10.0);
    input.setSingleStep(0.25);
    input.setValue(1.25);
    QCOMPARE(input.value(), 1.0);
    QCOMPARE(input.cleanText(), QStringLiteral("1"));
    QCOMPARE(valueSpy.count(), 1);

    QSignalSpy precisionSpy(&input, &AntInputNumber::precisionChanged);
    input.setPrecision(2);
    QCOMPARE(input.decimals(), 2);
    QCOMPARE(input.precision(), 2);
    QCOMPARE(precisionSpy.count(), 1);

    input.setValue(1.25);
    QCOMPARE(input.value(), 1.25);
    QCOMPARE(input.cleanText(), QStringLiteral("1.25"));

    input.stepBy(1);
    QCOMPARE(input.value(), 1.5);
    QCOMPARE(input.cleanText(), QStringLiteral("1.50"));

    input.setDecimals(3);
    QCOMPARE(input.decimals(), 3);
    QCOMPARE(input.precision(), 3);
    QCOMPARE(precisionSpy.count(), 2);

    input.setValue(1.125);
    QCOMPARE(input.value(), 1.125);
    QCOMPARE(input.cleanText(), QStringLiteral("1.125"));
}

void TestAntDataEntryA::inputNumberLineEditPaletteTracksDarkTheme()
{
    const Ant::ThemeMode previousMode = antTheme->themeMode();
    antTheme->setThemeMode(Ant::ThemeMode::Dark);

    AntInputNumber input;
    input.setValue(24);
    input.setPlaceholderText(QStringLiteral("Amount"));
    QCoreApplication::processEvents();

    auto* edit = input.findChild<QLineEdit*>();
    QVERIFY(edit);

    const auto& token = antTheme->tokens();
    const QPalette palette = edit->palette();
    QCOMPARE(palette.color(QPalette::Active, QPalette::Text), token.colorText);
    QCOMPARE(palette.color(QPalette::Inactive, QPalette::Text), token.colorText);
    QCOMPARE(palette.color(QPalette::Disabled, QPalette::Text), token.colorTextDisabled);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    QCOMPARE(palette.color(QPalette::Active, QPalette::PlaceholderText), token.colorTextPlaceholder);
    QCOMPARE(palette.color(QPalette::Disabled, QPalette::PlaceholderText), token.colorTextDisabled);
#endif

    input.setEnabled(false);
    QCoreApplication::processEvents();
    QCOMPARE(edit->palette().color(QPalette::Disabled, QPalette::Text), token.colorTextDisabled);

    antTheme->setThemeMode(previousMode);
}

void TestAntDataEntryA::inputNumberUsesLayoutFriendlyPolicy()
{
    QWidget host;
    auto* layout = new QHBoxLayout(&host);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* input = new AntInputNumber;
    input->setValue(42);
    layout->addWidget(input, 1);

    host.resize(260, 80);
    host.show();
    QCoreApplication::processEvents();

    QDoubleSpinBox nativeDoubleSpinBox;
    QCOMPARE(input->sizePolicy().horizontalPolicy(), nativeDoubleSpinBox.sizePolicy().horizontalPolicy());
    QCOMPARE(input->sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
    QCOMPARE(input->height(), input->sizeHint().height());
    QCOMPARE(input->width(), host.width());
}

void TestAntDataEntryA::inputNumberCachesMetricsAndScopesControlUpdates()
{
    AntInputNumber input;
    input.resize(180, input.sizeHint().height());

    const int sizeHintResolvesBefore = input.property("antInputNumberSizeHintResolveCount").toInt();
    input.sizeHint();
    input.minimumSizeHint();
    input.sizeHint();
    QCOMPARE(input.property("antInputNumberSizeHintResolveCount").toInt(), sizeHintResolvesBefore);

    const int metricsResolvesBefore = input.property("antInputNumberMetricsResolveCount").toInt();
    input.sizeHint();
    input.minimumSizeHint();
    QCOMPARE(input.property("antInputNumberMetricsResolveCount").toInt(), metricsResolvesBefore);

    const int editStyleBeforeProgress = input.property("antInputNumberEditStyleApplyCount").toInt();
    const int insetBeforeProgress = input.property("antInputNumberControlsInsetUpdateCount").toInt();
    input.setControlsProgress(0.45);
    QVERIFY(input.property("antInputNumberControlsInsetUpdateCount").toInt() > insetBeforeProgress);
    QCOMPARE(input.property("antInputNumberEditStyleApplyCount").toInt(), editStyleBeforeProgress);

    const int frameUpdatesBefore = input.property("antInputNumberFrameUpdateCount").toInt();
    input.setStatus(Ant::Status::Warning);
    QVERIFY(input.property("antInputNumberFrameUpdateCount").toInt() > frameUpdatesBefore);

    const int regionUpdatesBeforeProgress = input.property("antInputNumberControlsRegionUpdateCount").toInt();
    input.setControlsProgress(0.8);
    QVERIFY(input.property("antInputNumberControlsRegionUpdateCount").toInt() > regionUpdatesBeforeProgress);
    const int regionUpdatesAfterProgress = input.property("antInputNumberControlsRegionUpdateCount").toInt();
    input.setControlsProgress(0.8);
    QCOMPARE(input.property("antInputNumberControlsRegionUpdateCount").toInt(), regionUpdatesAfterProgress);
}

void TestAntDataEntryA::radioCachesLayoutAndScopesStateUpdates()
{
    AntRadio radio(QStringLiteral("Choice"));
    radio.resize(140, radio.sizeHint().height());
    radio.sizeHint();

    const int sizeHintResolvesBefore = radio.property("antRadioSizeHintResolveCount").toInt();
    const int layoutBuildsBefore = radio.property("antRadioLayoutBuildCount").toInt();
    radio.sizeHint();
    radio.minimumSizeHint();
    radio.sizeHint();
    QCOMPARE(radio.property("antRadioSizeHintResolveCount").toInt(), sizeHintResolvesBefore);
    QCOMPARE(radio.property("antRadioLayoutBuildCount").toInt(), layoutBuildsBefore);

    const int regionUpdatesBefore = radio.property("antRadioRegionUpdateCount").toInt();
    radio.setChecked(true);
    QVERIFY(radio.property("antRadioRegionUpdateCount").toInt() > regionUpdatesBefore);
    QCOMPARE(radio.property("antRadioLastUpdateMode").toString(), QStringLiteral("indicator"));

    const int regionUpdatesAfterChecked = radio.property("antRadioRegionUpdateCount").toInt();
    radio.setChecked(true);
    QCOMPARE(radio.property("antRadioRegionUpdateCount").toInt(), regionUpdatesAfterChecked);

    radio.setText(QStringLiteral("Choice B"));
    const int layoutBuildsBeforeTextHint = radio.property("antRadioLayoutBuildCount").toInt();
    radio.sizeHint();
    QVERIFY(radio.property("antRadioLayoutBuildCount").toInt() > layoutBuildsBeforeTextHint);

    QWidget host;
    host.resize(220, 44);
    AntRadio left(QStringLiteral("Left"), &host);
    AntRadio right(QStringLiteral("Right"), &host);
    left.setButtonStyle(true);
    right.setButtonStyle(true);
    left.resize(90, left.sizeHint().height());
    right.resize(90, right.sizeHint().height());
    left.move(0, 0);
    right.move(90, 0);

    left.sizeHint();
    const int edgeResolvesBefore = left.property("antRadioButtonEdgeResolveCount").toInt();
    left.sizeHint();
    left.minimumSizeHint();
    QCOMPARE(left.property("antRadioButtonEdgeResolveCount").toInt(), edgeResolvesBefore);

    const int buttonRegionUpdatesBefore = left.property("antRadioRegionUpdateCount").toInt();
    QEnterEvent enterEvent(QPointF(4, 4), QPointF(4, 4), QPointF(4, 4));
    QCoreApplication::sendEvent(&left, &enterEvent);
    QVERIFY(left.property("antRadioRegionUpdateCount").toInt() > buttonRegionUpdatesBefore);
    QCOMPARE(left.property("antRadioLastUpdateMode").toString(), QStringLiteral("button"));
}

void TestAntDataEntryA::rateCachesStarLayoutAndScopesUpdates()
{
    AntRate rate;
    rate.resize(rate.sizeHint());
    rate.sizeHint();

    const int sizeHintResolvesBefore = rate.property("antRateSizeHintResolveCount").toInt();
    const int layoutBuildsBefore = rate.property("antRateLayoutBuildCount").toInt();
    const int starPathsBefore = rate.property("antRateStarPathBuildCount").toInt();
    rate.sizeHint();
    rate.minimumSizeHint();
    rate.sizeHint();
    QCOMPARE(rate.property("antRateSizeHintResolveCount").toInt(), sizeHintResolvesBefore);
    QCOMPARE(rate.property("antRateLayoutBuildCount").toInt(), layoutBuildsBefore);
    QCOMPARE(rate.property("antRateStarPathBuildCount").toInt(), starPathsBefore);

    const int valueRegionBefore = rate.property("antRateValueRegionUpdateCount").toInt();
    rate.setValue(3.0);
    QVERIFY(rate.property("antRateValueRegionUpdateCount").toInt() > valueRegionBefore);
    QCOMPARE(rate.property("antRateStarPathBuildCount").toInt(), starPathsBefore);

    const int valueRegionAfterSameValue = rate.property("antRateValueRegionUpdateCount").toInt();
    rate.setValue(3.0);
    QCOMPARE(rate.property("antRateValueRegionUpdateCount").toInt(), valueRegionAfterSameValue);

    const auto& token = antTheme->tokens();
    const int starSize = static_cast<int>(std::round(token.controlHeight * 0.625));
    const int gap = token.marginXS;
    const QPoint fourthStarCenter((starSize + gap) * 3 + starSize / 2, rate.height() / 2);
    QMouseEvent moveEvent(QEvent::MouseMove,
                          QPointF(fourthStarCenter),
                          QPointF(rate.mapToGlobal(fourthStarCenter)),
                          Qt::NoButton,
                          Qt::NoButton,
                          Qt::NoModifier);
    QCoreApplication::sendEvent(&rate, &moveEvent);
    QVERIFY(rate.property("antRateValueRegionUpdateCount").toInt() > valueRegionAfterSameValue);

    const int focusUpdatesBefore = rate.property("antRateFocusRegionUpdateCount").toInt();
    QFocusEvent focusEvent(QEvent::FocusIn);
    QCoreApplication::sendEvent(&rate, &focusEvent);
    QVERIFY(rate.property("antRateFocusRegionUpdateCount").toInt() > focusUpdatesBefore);
}

void TestAntDataEntryA::segmentedCachesLayoutAndScopesUpdates()
{
    AntSegmented segmented;
    segmented.setOptions({{QStringLiteral("daily"), QStringLiteral("Daily"), QStringLiteral("appstore")},
                          {QStringLiteral("weekly"), QStringLiteral("Weekly")},
                          {QStringLiteral("monthly"), QStringLiteral("Monthly"), QStringLiteral("setting")}});
    segmented.resize(segmented.sizeHint());
    segmented.sizeHint();

    const int sizeHintResolvesBefore = segmented.property("antSegmentedSizeHintResolveCount").toInt();
    const int layoutBuildsBefore = segmented.property("antSegmentedLayoutBuildCount").toInt();
    const int textMetricsBefore = segmented.property("antSegmentedTextMetricResolveCount").toInt();
    segmented.sizeHint();
    segmented.minimumSizeHint();
    segmented.segmentRects();
    QCOMPARE(segmented.property("antSegmentedSizeHintResolveCount").toInt(), sizeHintResolvesBefore);
    QCOMPARE(segmented.property("antSegmentedLayoutBuildCount").toInt(), layoutBuildsBefore);
    QCOMPARE(segmented.property("antSegmentedTextMetricResolveCount").toInt(), textMetricsBefore);

    const auto rects = segmented.segmentRects();
    QCOMPARE(rects.size(), 3);

    const int regionUpdatesBeforeHover = segmented.property("antSegmentedRegionUpdateCount").toInt();
    const QPoint secondSegmentCenter(qRound(rects.at(1).center().x()), qRound(rects.at(1).center().y()));
    QMouseEvent moveEvent(QEvent::MouseMove,
                          QPointF(secondSegmentCenter),
                          QPointF(segmented.mapToGlobal(secondSegmentCenter)),
                          Qt::NoButton,
                          Qt::NoButton,
                          Qt::NoModifier);
    QCoreApplication::sendEvent(&segmented, &moveEvent);
    QVERIFY(segmented.property("antSegmentedRegionUpdateCount").toInt() > regionUpdatesBeforeHover);
    QCOMPARE(segmented.property("antSegmentedLastUpdateMode").toString(), QStringLiteral("segments"));

    const int thumbUpdatesBeforeSelection = segmented.property("antSegmentedThumbRegionUpdateCount").toInt();
    const int regionUpdatesBeforeSelection = segmented.property("antSegmentedRegionUpdateCount").toInt();
    segmented.setCurrentIndex(2);
    QVERIFY(segmented.property("antSegmentedRegionUpdateCount").toInt() > regionUpdatesBeforeSelection);
    QVERIFY(segmented.property("antSegmentedThumbRegionUpdateCount").toInt() > thumbUpdatesBeforeSelection);

    const int regionUpdatesAfterSameSelection = segmented.property("antSegmentedRegionUpdateCount").toInt();
    segmented.setCurrentIndex(2);
    QCOMPARE(segmented.property("antSegmentedRegionUpdateCount").toInt(), regionUpdatesAfterSameSelection);

    const int layoutBuildsBeforeBlockHint = segmented.property("antSegmentedLayoutBuildCount").toInt();
    segmented.setBlock(true);
    segmented.sizeHint();
    QVERIFY(segmented.property("antSegmentedLayoutBuildCount").toInt() > layoutBuildsBeforeBlockHint);
}

void TestAntDataEntryA::sliderCachesGeometryAndScopesUpdates()
{
    AntSlider slider;
    slider.setSingleStep(10);
    slider.setMarks({{0, QStringLiteral("A")}, {50, QStringLiteral("B")}, {100, QStringLiteral("C")}});
    slider.resize(260, slider.sizeHint().height());
    slider.sizeHint();

    const int sizeHintResolvesBefore = slider.property("antSliderSizeHintResolveCount").toInt();
    const int layoutBuildsBefore = slider.property("antSliderLayoutBuildCount").toInt();
    const int metricsResolvesBefore = slider.property("antSliderMetricsResolveCount").toInt();
    slider.sizeHint();
    slider.minimumSizeHint();
    slider.sizeHint();
    QCOMPARE(slider.property("antSliderSizeHintResolveCount").toInt(), sizeHintResolvesBefore);
    QCOMPARE(slider.property("antSliderLayoutBuildCount").toInt(), layoutBuildsBefore);
    QCOMPARE(slider.property("antSliderMetricsResolveCount").toInt(), metricsResolvesBefore);

    const int regionUpdatesBeforeValue = slider.property("antSliderRegionUpdateCount").toInt();
    slider.setValue(50);
    QVERIFY(slider.property("antSliderRegionUpdateCount").toInt() > regionUpdatesBeforeValue);
    QCOMPARE(slider.property("antSliderLastUpdateMode").toString(), QStringLiteral("value"));

    const int regionUpdatesAfterSameValue = slider.property("antSliderRegionUpdateCount").toInt();
    slider.setValue(50);
    QCOMPARE(slider.property("antSliderRegionUpdateCount").toInt(), regionUpdatesAfterSameValue);

    const int handleUpdatesBefore = slider.property("antSliderHandleRegionUpdateCount").toInt();
    slider.setHandleScale(1.15);
    QVERIFY(slider.property("antSliderHandleRegionUpdateCount").toInt() > handleUpdatesBefore);
    QCOMPARE(slider.property("antSliderLastUpdateMode").toString(), QStringLiteral("handle"));

    const int focusUpdatesBefore = slider.property("antSliderFocusRegionUpdateCount").toInt();
    slider.setFocusProgress(0.7);
    QVERIFY(slider.property("antSliderFocusRegionUpdateCount").toInt() > focusUpdatesBefore);
    QCOMPARE(slider.property("antSliderLastUpdateMode").toString(), QStringLiteral("focus"));

    slider.setRangeMode(true);
    slider.setRangeValues(20, 80);
    const int regionUpdatesBeforeRange = slider.property("antSliderRegionUpdateCount").toInt();
    slider.setRangeValues(30, 70);
    QVERIFY(slider.property("antSliderRegionUpdateCount").toInt() > regionUpdatesBeforeRange);
    QCOMPARE(slider.property("antSliderLastUpdateMode").toString(), QStringLiteral("range"));
}

void TestAntDataEntryA::sliderMarksReserveLabelHeight()
{
    AntSlider slider;
    slider.setSingleStep(25);
    slider.setMarks({{0, QStringLiteral("A")}, {25, QStringLiteral("B")}, {50, QStringLiteral("C")},
                     {75, QStringLiteral("D")}, {100, QStringLiteral("E")}});

    QVERIFY(slider.sizeHint().height() > 0);
    QVERIFY(slider.minimumSizeHint().height() >= slider.sizeHint().height());
}

void TestAntDataEntryA::sliderBubbleFloatsAboveMarkedHandle()
{
    QWidget host;
    host.move(160, 160);
    host.resize(460, 120);

    AntSlider slider(&host);
    slider.setSingleStep(25);
    slider.setMarks({{0, QStringLiteral("A")}, {25, QStringLiteral("B")}, {50, QStringLiteral("C")},
                     {75, QStringLiteral("D")}, {100, QStringLiteral("E")}});
    slider.resize(400, slider.sizeHint().height());
    slider.move(24, 48);
    slider.show();
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    const QPoint visualHandleCenter(slider.width() / 2, sliderVisualHandleCenterY());
    QTest::mousePress(&slider, Qt::LeftButton, Qt::NoModifier, visualHandleCenter);

    auto* valueBubble = slider.findChild<QWidget*>(QStringLiteral("antSliderValueBubble"));
    QVERIFY(valueBubble);
    QTRY_VERIFY_WITH_TIMEOUT(valueBubble->isVisible(), 100);
    QVERIFY(valueBubble->testAttribute(Qt::WA_TransparentForMouseEvents));
    const QPoint visualHandleGlobal = slider.mapToGlobal(visualHandleCenter);
    QVERIFY(valueBubble->geometry().bottom() <= visualHandleGlobal.y() - 4);

    QTest::mouseRelease(&slider, Qt::LeftButton, Qt::NoModifier, visualHandleCenter);
}

void TestAntDataEntryA::sliderBubbleArrowJoinsSurface()
{
    QWidget host;
    host.move(160, 160);
    host.resize(360, 120);

    AntSlider slider(&host);
    slider.resize(280, slider.sizeHint().height());
    slider.move(40, 52);
    slider.show();
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    QTest::mousePress(&slider, Qt::LeftButton, Qt::NoModifier, QPoint(slider.width() / 2, slider.height() / 2));

    auto* valueBubble = slider.findChild<QWidget*>(QStringLiteral("antSliderValueBubble"));
    QVERIFY(valueBubble);
    QTRY_VERIFY_WITH_TIMEOUT(valueBubble->isVisible(), 100);

    const QImage image = renderWidgetImage(*valueBubble);
    QTest::mouseRelease(&slider, Qt::LeftButton, Qt::NoModifier, QPoint(slider.width() / 2, slider.height() / 2));

    const int centerX = image.width() / 2;
    const int joinY = image.height() - 7;
    const QColor body = image.pixelColor(centerX, joinY - 3);
    QVERIFY2(body.alpha() > 240, "value bubble body should be opaque at the arrow center");

    for (int dx = -4; dx <= 4; ++dx)
    {
        const QColor join = image.pixelColor(centerX + dx, joinY);
        QVERIFY2(join.alpha() > 240, "arrow and rounded rectangle should share an opaque joined surface");
        QVERIFY2(qAbs(join.red() - body.red()) <= 2
                     && qAbs(join.green() - body.green()) <= 2
                     && qAbs(join.blue() - body.blue()) <= 2,
                 "arrow join pixels should match the bubble body color without a visible divider");
    }
}

void TestAntDataEntryA::sliderRangeDragDoesNotPaintPhantomMinimumHandle()
{
    AntSlider slider;
    slider.setRangeMode(true);
    slider.setRangeValues(20, 60);
    slider.resize(240, 44);
    slider.show();
    QVERIFY(QTest::qWaitForWindowExposed(&slider));

    const auto& token = antTheme->tokens();
    const int handleSizeHover = token.controlHeightSM / 2;
    const int phantomX = handleSizeHover / 2 + 2;
    const QPoint endHandlePoint(phantomX + qRound((slider.width() - phantomX * 2) * 0.60), slider.height() / 2);

    QTest::mousePress(&slider, Qt::LeftButton, Qt::NoModifier, endHandlePoint);
    QVERIFY(slider.isPressedState());
    const QImage image = renderWidgetImage(slider);
    QTest::mouseRelease(&slider, Qt::LeftButton, Qt::NoModifier, endHandlePoint);

    const QColor phantomPixel = image.pixelColor(phantomX, slider.height() / 2);
    QVERIFY2(phantomPixel.blue() - phantomPixel.red() < 18,
             "range drag should not paint a primary-colored pressed halo at the minimum edge");
}

void TestAntDataEntryA::segmentedClicksUseFullVisualTrack()
{
    AntSegmented segmented;
    segmented.setOptions({{QStringLiteral("daily"), QStringLiteral("Daily")},
                          {QStringLiteral("weekly"), QStringLiteral("Weekly")},
                          {QStringLiteral("monthly"), QStringLiteral("Monthly")}});
    segmented.resize(segmented.sizeHint());
    segmented.show();
    QVERIFY(QTest::qWaitForWindowExposed(&segmented));

    QSignalSpy indexSpy(&segmented, &AntSegmented::currentIndexChanged);
    QSignalSpy valueSpy(&segmented, &AntSegmented::valueChanged);
    const auto rects = segmented.segmentRects();
    QCOMPARE(rects.size(), 3);

    const QPoint topTrackPoint(qRound(rects[1].center().x()), 1);
    QTest::mouseClick(&segmented, Qt::LeftButton, Qt::NoModifier, topTrackPoint);

    QCOMPARE(segmented.currentIndex(), 1);
    QCOMPARE(segmented.value(), QStringLiteral("weekly"));
    QCOMPARE(indexSpy.count(), 1);
    QCOMPARE(indexSpy.takeFirst().at(0).toInt(), 1);
    QCOMPARE(valueSpy.count(), 1);
    QCOMPARE(valueSpy.takeFirst().at(0).toString(), QStringLiteral("weekly"));

    segmented.setVertical(true);
    segmented.resize(segmented.sizeHint());
    QCoreApplication::processEvents();

    const auto verticalRects = segmented.segmentRects();
    QCOMPARE(verticalRects.size(), 3);
    const QPoint leftTrackPoint(1, qRound(verticalRects[2].center().y()));
    QTest::mouseClick(&segmented, Qt::LeftButton, Qt::NoModifier, leftTrackPoint);

    QCOMPARE(segmented.currentIndex(), 2);
    QCOMPARE(segmented.value(), QStringLiteral("monthly"));
    QCOMPARE(indexSpy.count(), 1);
    QCOMPARE(indexSpy.takeFirst().at(0).toInt(), 2);
    QCOMPARE(valueSpy.count(), 1);
    QCOMPARE(valueSpy.takeFirst().at(0).toString(), QStringLiteral("monthly"));
}

void TestAntDataEntryA::rateSelectionStartsScaleAnimation()
{
    AntRate rate;
    rate.resize(rate.sizeHint());
    rate.show();
    QVERIFY(QTest::qWaitForWindowExposed(&rate));

    const auto& token = antTheme->tokens();
    const int starSize = static_cast<int>(std::round(token.controlHeight * 0.625));
    const int gap = token.marginXS;
    const QPoint thirdStarCenter((starSize + gap) * 2 + starSize / 2, rate.height() / 2);

    QTest::mouseClick(&rate, Qt::LeftButton, Qt::NoModifier, thirdStarCenter);
    QCOMPARE(rate.value(), 3.0);

    auto* animation = rate.findChild<QVariantAnimation*>(QStringLiteral("antRateSelectionScaleAnimation"));
    QVERIFY2(animation, "AntRate should start a scale pulse animation when a star is selected");
    QTRY_VERIFY_WITH_TIMEOUT(animation->state() == QAbstractAnimation::Running, 80);
    QTRY_VERIFY_WITH_TIMEOUT(animation->currentValue().toReal() > 1.0, 160);
    QTRY_COMPARE_WITH_TIMEOUT(animation->state(), QAbstractAnimation::Stopped, 500);
    QCOMPARE(animation->currentValue().toReal(), 1.0);
}

void TestAntDataEntryA::autoCompleteReusesPopupItemsAndScopesHighlight()
{
    AntAutoComplete autoComplete;
    autoComplete.setMaxVisibleItems(4);
    for (int i = 0; i < 12; ++i)
    {
        autoComplete.addSuggestion(QStringLiteral("application %1").arg(i), i);
    }
    autoComplete.addSuggestion(QStringLiteral("banana"), QStringLiteral("banana"));
    autoComplete.resize(260, autoComplete.sizeHint().height());
    autoComplete.show();
    QVERIFY(QTest::qWaitForWindowExposed(&autoComplete));

    auto* editor = autoComplete.findChild<QLineEdit*>();
    QVERIFY(editor);
    QTest::mouseClick(editor, Qt::LeftButton);
    QTest::keyClicks(editor, QStringLiteral("app"));

    QTRY_VERIFY(autoComplete.property("antAutoCompleteFilterBuildCount").toInt() > 0);
    QTRY_COMPARE(autoComplete.property("antAutoCompletePopupItemCreateCount").toInt(), 4);

    const int filterBuilds = autoComplete.property("antAutoCompleteFilterBuildCount").toInt();
    const int filterHitsBefore = autoComplete.property("antAutoCompleteFilterCacheHitCount").toInt();
    QVERIFY(QMetaObject::invokeMethod(editor,
                                      "textEdited",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, editor->text())));
    QCOMPARE(autoComplete.property("antAutoCompleteFilterBuildCount").toInt(), filterBuilds);
    QVERIFY(autoComplete.property("antAutoCompleteFilterCacheHitCount").toInt() > filterHitsBefore);

    const int itemCreates = autoComplete.property("antAutoCompletePopupItemCreateCount").toInt();
    const int highlightedUpdatesBefore = autoComplete.property("antAutoCompleteHighlightedRowUpdateCount").toInt();
    QTest::keyClick(editor, Qt::Key_Down);
    QCOMPARE(autoComplete.property("antAutoCompletePopupItemCreateCount").toInt(), itemCreates);
    QVERIFY(autoComplete.property("antAutoCompleteHighlightedRowUpdateCount").toInt() > highlightedUpdatesBefore);

    const int geometrySkipsBefore = autoComplete.property("antAutoCompletePopupGeometrySkipCount").toInt();
    QVERIFY(QMetaObject::invokeMethod(editor,
                                      "textEdited",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, editor->text())));
    QVERIFY(autoComplete.property("antAutoCompletePopupGeometrySkipCount").toInt() > geometrySkipsBefore);
}

QTEST_MAIN(TestAntDataEntryA)
#include "TestAntDataEntryA.moc"
