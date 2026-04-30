#include <QSignalSpy>
#include <QTest>

#include "core/AntWave.h"
#include "widgets/AntSwitch.h"

class TestAntSwitch : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
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

QTEST_MAIN(TestAntSwitch)
#include "TestAntSwitch.moc"
