#include <QSignalSpy>
#include <QTest>

#include "core/AntWave.h"
#include "widgets/AntSwitch.h"

class TestAntSwitch : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
    void cachesLayoutAndScopesUpdates();
    void loadingTimerPausesWhenHidden();
};

void TestAntSwitch::propertiesAndSignals()
{
    auto* sw = new AntSwitch;
    QCOMPARE(sw->isChecked(), false);
    QCOMPARE(sw->switchSize(), Ant::Size::Middle);
    QCOMPARE(sw->isLoading(), false);
    QCOMPARE(sw->checkedText(), QString());
    QCOMPARE(sw->uncheckedText(), QString());

    QSignalSpy checkedSpy(sw, &AntSwitch::checkedChanged);
    sw->setChecked(true);
    QCOMPARE(sw->isChecked(), true);
    QCOMPARE(checkedSpy.count(), 1);

    QSignalSpy sizeSpy(sw, &AntSwitch::switchSizeChanged);
    sw->setSwitchSize(Ant::Size::Small);
    QCOMPARE(sw->switchSize(), Ant::Size::Small);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy loadingSpy(sw, &AntSwitch::loadingChanged);
    sw->setLoading(true);
    QCOMPARE(sw->isLoading(), true);
    QCOMPARE(loadingSpy.count(), 1);

    QSignalSpy checkedTextSpy(sw, &AntSwitch::checkedTextChanged);
    sw->setCheckedText("ON");
    QCOMPARE(sw->checkedText(), "ON");
    QCOMPARE(checkedTextSpy.count(), 1);

    QSignalSpy uncheckedTextSpy(sw, &AntSwitch::uncheckedTextChanged);
    sw->setUncheckedText("OFF");
    QCOMPARE(sw->uncheckedText(), "OFF");
    QCOMPARE(uncheckedTextSpy.count(), 1);

    QSize hint = sw->sizeHint();
    QVERIFY(hint.width() > 0);
    QVERIFY(hint.height() > 0);

    auto* host = new QWidget;
    host->resize(120, 80);
    auto* clickable = new AntSwitch(host);
    clickable->move(24, 24);
    host->show();
    QVERIFY(QTest::qWaitForWindowExposed(host));
    QTest::mouseClick(clickable, Qt::LeftButton, Qt::NoModifier, clickable->rect().center());
    QTRY_VERIFY_WITH_TIMEOUT(!host->findChildren<AntWave*>().isEmpty(), 100);
}

void TestAntSwitch::cachesLayoutAndScopesUpdates()
{
    AntSwitch sw;
    sw.setCheckedText(QStringLiteral("ON"));
    sw.setUncheckedText(QStringLiteral("OFF"));
    sw.resize(sw.sizeHint());
    sw.sizeHint();

    const int sizeHintResolvesBefore = sw.property("antSwitchSizeHintResolveCount").toInt();
    const int layoutBuildsBefore = sw.property("antSwitchLayoutBuildCount").toInt();
    const int metricsResolvesBefore = sw.property("antSwitchMetricsResolveCount").toInt();
    sw.sizeHint();
    sw.minimumSizeHint();
    sw.sizeHint();
    QCOMPARE(sw.property("antSwitchSizeHintResolveCount").toInt(), sizeHintResolvesBefore);
    QCOMPARE(sw.property("antSwitchLayoutBuildCount").toInt(), layoutBuildsBefore);
    QCOMPARE(sw.property("antSwitchMetricsResolveCount").toInt(), metricsResolvesBefore);

    const int handleUpdatesBeforeProgress = sw.property("antSwitchHandleRegionUpdateCount").toInt();
    sw.setHandleProgress(0.5);
    QVERIFY(sw.property("antSwitchHandleRegionUpdateCount").toInt() > handleUpdatesBeforeProgress);
    QCOMPARE(sw.property("antSwitchLastUpdateMode").toString(), QStringLiteral("handle"));

    const int regionUpdatesAfterSameProgress = sw.property("antSwitchRegionUpdateCount").toInt();
    sw.setHandleProgress(0.5);
    QCOMPARE(sw.property("antSwitchRegionUpdateCount").toInt(), regionUpdatesAfterSameProgress);

    const int handleUpdatesBeforeStretch = sw.property("antSwitchHandleRegionUpdateCount").toInt();
    sw.setHandleStretch(1.0);
    QVERIFY(sw.property("antSwitchHandleRegionUpdateCount").toInt() > handleUpdatesBeforeStretch);
    QCOMPARE(sw.property("antSwitchLastUpdateMode").toString(), QStringLiteral("handle"));

    const int regionUpdatesBeforeChecked = sw.property("antSwitchRegionUpdateCount").toInt();
    sw.setChecked(true);
    QVERIFY(sw.property("antSwitchRegionUpdateCount").toInt() > regionUpdatesBeforeChecked);
}

void TestAntSwitch::loadingTimerPausesWhenHidden()
{
    AntSwitch sw;
    sw.resize(sw.sizeHint());

    sw.setLoading(true);
    QVERIFY(sw.isLoading());
    QVERIFY(!sw.property("antSwitchLoadingTimerActive").toBool());

    sw.show();
    QVERIFY(QTest::qWaitForWindowExposed(&sw));
    QTRY_VERIFY_WITH_TIMEOUT(sw.property("antSwitchLoadingTimerActive").toBool(), 100);

    const int loadingUpdatesBefore = sw.property("antSwitchLoadingRegionUpdateCount").toInt();
    QTest::qWait(120);
    QVERIFY(sw.property("antSwitchLoadingRegionUpdateCount").toInt() > loadingUpdatesBefore);
    QCOMPARE(sw.property("antSwitchLastUpdateMode").toString(), QStringLiteral("loading"));

    sw.hide();
    QTRY_VERIFY_WITH_TIMEOUT(!sw.property("antSwitchLoadingTimerActive").toBool(), 100);
}

QTEST_MAIN(TestAntSwitch)
#include "TestAntSwitch.moc"
