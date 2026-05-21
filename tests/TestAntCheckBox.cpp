#include <QCoreApplication>
#include <QEnterEvent>
#include <QSignalSpy>
#include <QTest>

#include "widgets/AntCheckBox.h"

class TestAntCheckBox : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
    void cachesLayoutAndScopesIndicatorUpdates();
};

void TestAntCheckBox::propertiesAndSignals()
{
    // Heap-allocate to avoid QProxyStyle destructor crash in test environment
    auto* cb = new AntCheckBox;
    QCOMPARE(cb->isChecked(), false);
    QCOMPARE(cb->isIndeterminate(), false);
    QCOMPARE(cb->checkState(), Qt::Unchecked);
    QCOMPARE(cb->isTristate(), false);
    QCOMPARE(cb->text(), QString());

    QSignalSpy checkedSpy(cb, &AntCheckBox::checkedChanged);
    cb->setChecked(true);
    QCOMPARE(cb->isChecked(), true);
    QCOMPARE(checkedSpy.count(), 1);
    QCOMPARE(checkedSpy.at(0).at(0).toBool(), true);

    QSignalSpy indeterminateSpy(cb, &AntCheckBox::indeterminateChanged);
    cb->setTristate(true);
    QCOMPARE(cb->isTristate(), true);
    cb->setCheckState(Qt::PartiallyChecked);
    QCOMPARE(cb->isIndeterminate(), true);
    QCOMPARE(cb->checkState(), Qt::PartiallyChecked);
    QCOMPARE(indeterminateSpy.count(), 1);

    QSignalSpy textSpy(cb, &AntCheckBox::textChanged);
    cb->setText("Option A");
    QCOMPARE(cb->text(), "Option A");
    QCOMPARE(textSpy.count(), 1);

    QSignalSpy toggledSpy(cb, &AntCheckBox::toggled);
    cb->setChecked(false);
    QCOMPARE(toggledSpy.count(), 1);

    cb->setTristate(false);
    QCOMPARE(cb->isTristate(), false);
    cb->toggle();
    QCOMPARE(cb->isChecked(), true);
    cb->click();
    QCOMPARE(cb->isChecked(), false);

    QSize hint = cb->sizeHint();
    QVERIFY(hint.width() > 0);
    QVERIFY(hint.height() > 0);

    auto* cb2 = new AntCheckBox("My Checkbox");
    QCOMPARE(cb2->text(), "My Checkbox");
}

void TestAntCheckBox::cachesLayoutAndScopesIndicatorUpdates()
{
    AntCheckBox cb(QStringLiteral("Cached option"));
    cb.resize(180, 32);
    cb.show();
    QVERIFY(QTest::qWaitForWindowExposed(&cb));

    const QSize firstHint = cb.sizeHint();
    QVERIFY(firstHint.width() > 0);
    QVERIFY(cb.property("antCheckBoxLayoutBuildCount").toInt() > 0);

    const int cacheHitsBefore = cb.property("antCheckBoxLayoutCacheHitCount").toInt();
    QCOMPARE(cb.sizeHint(), firstHint);
    QVERIFY(cb.property("antCheckBoxLayoutCacheHitCount").toInt() > cacheHitsBefore);

    const int scopedBeforeHover = cb.property("antCheckBoxIndicatorScopedUpdateCount").toInt();
    QEnterEvent enterEvent(QPointF(4, 4), QPointF(4, 4), QPointF(cb.mapToGlobal(QPoint(4, 4))));
    QCoreApplication::sendEvent(&cb, &enterEvent);
    QVERIFY(cb.property("antCheckBoxIndicatorScopedUpdateCount").toInt() > scopedBeforeHover);

    const int scopedBeforeChecked = cb.property("antCheckBoxIndicatorScopedUpdateCount").toInt();
    cb.setChecked(true);
    QVERIFY(cb.property("antCheckBoxIndicatorScopedUpdateCount").toInt() > scopedBeforeChecked);

    const int scopedBeforeSameChecked = cb.property("antCheckBoxIndicatorScopedUpdateCount").toInt();
    cb.setChecked(true);
    QCOMPARE(cb.property("antCheckBoxIndicatorScopedUpdateCount").toInt(), scopedBeforeSameChecked);

    const int buildsBeforeText = cb.property("antCheckBoxLayoutBuildCount").toInt();
    cb.setText(QStringLiteral("Cached option with longer text"));
    (void)cb.sizeHint();
    QVERIFY(cb.property("antCheckBoxLayoutBuildCount").toInt() > buildsBeforeText);
}

QTEST_MAIN(TestAntCheckBox)
#include "TestAntCheckBox.moc"
