#include <QSignalSpy>
#include <QTest>

#include "widgets/AntButton.h"

class TestAntButton : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
};

void TestAntButton::propertiesAndSignals()
{
    // Heap-allocate to avoid QProxyStyle destructor crash in test environment
    auto* btn = new AntButton;
    QCOMPARE(btn->buttonType(), Ant::ButtonType::Default);
    QCOMPARE(btn->buttonSize(), Ant::Size::Middle);
    QCOMPARE(btn->buttonShape(), Ant::ButtonShape::Default);
    QCOMPARE(btn->isLoading(), false);
    QCOMPARE(btn->isDanger(), false);
    QCOMPARE(btn->isGhost(), false);
    QCOMPARE(btn->isBlock(), false);

    QSignalSpy typeSpy(btn, &AntButton::buttonTypeChanged);
    btn->setButtonType(Ant::ButtonType::Primary);
    QCOMPARE(btn->buttonType(), Ant::ButtonType::Primary);
    QCOMPARE(typeSpy.count(), 1);
    QCOMPARE(typeSpy.at(0).at(0).value<Ant::ButtonType>(), Ant::ButtonType::Primary);

    QSignalSpy sizeSpy(btn, &AntButton::buttonSizeChanged);
    btn->setButtonSize(Ant::Size::Large);
    QCOMPARE(btn->buttonSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy shapeSpy(btn, &AntButton::buttonShapeChanged);
    btn->setButtonShape(Ant::ButtonShape::Circle);
    QCOMPARE(btn->buttonShape(), Ant::ButtonShape::Circle);
    QCOMPARE(shapeSpy.count(), 1);

    QSignalSpy dangerSpy(btn, &AntButton::dangerChanged);
    btn->setDanger(true);
    QCOMPARE(btn->isDanger(), true);
    QCOMPARE(dangerSpy.count(), 1);

    QSignalSpy ghostSpy(btn, &AntButton::ghostChanged);
    btn->setGhost(true);
    QCOMPARE(btn->isGhost(), true);
    QCOMPARE(ghostSpy.count(), 1);

    QSignalSpy blockSpy(btn, &AntButton::blockChanged);
    btn->setBlock(true);
    QCOMPARE(btn->isBlock(), true);
    QCOMPARE(blockSpy.count(), 1);

    QSignalSpy loadingSpy(btn, &AntButton::loadingChanged);
    btn->setLoading(true);
    QCOMPARE(btn->isLoading(), true);
    QCOMPARE(loadingSpy.count(), 1);

    QSize hint = btn->sizeHint();
    QVERIFY(hint.width() > 0);
    QVERIFY(hint.height() > 0);

    auto* btn2 = new AntButton("Click Me");
    QCOMPARE(btn2->text(), "Click Me");
}

QTEST_MAIN(TestAntButton)
#include "TestAntButton.moc"
