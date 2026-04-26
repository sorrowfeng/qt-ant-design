#include <QSignalSpy>
#include <QTest>

#include "widgets/AntCheckbox.h"

class TestAntCheckbox : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
};

void TestAntCheckbox::propertiesAndSignals()
{
    // Heap-allocate to avoid QProxyStyle destructor crash in test environment
    auto* cb = new AntCheckbox;
    QCOMPARE(cb->isChecked(), false);
    QCOMPARE(cb->isIndeterminate(), false);
    QCOMPARE(cb->text(), QString());

    QSignalSpy checkedSpy(cb, &AntCheckbox::checkedChanged);
    cb->setChecked(true);
    QCOMPARE(cb->isChecked(), true);
    QCOMPARE(checkedSpy.count(), 1);
    QCOMPARE(checkedSpy.at(0).at(0).toBool(), true);

    QSignalSpy indeterminateSpy(cb, &AntCheckbox::indeterminateChanged);
    cb->setIndeterminate(true);
    QCOMPARE(cb->isIndeterminate(), true);
    QCOMPARE(indeterminateSpy.count(), 1);

    QSignalSpy textSpy(cb, &AntCheckbox::textChanged);
    cb->setText("Option A");
    QCOMPARE(cb->text(), "Option A");
    QCOMPARE(textSpy.count(), 1);

    QSignalSpy toggledSpy(cb, &AntCheckbox::toggled);
    cb->setChecked(false);
    QCOMPARE(toggledSpy.count(), 1);

    QSize hint = cb->sizeHint();
    QVERIFY(hint.width() > 0);
    QVERIFY(hint.height() > 0);

    auto* cb2 = new AntCheckbox("My Checkbox");
    QCOMPARE(cb2->text(), "My Checkbox");
}

QTEST_MAIN(TestAntCheckbox)
#include "TestAntCheckbox.moc"
