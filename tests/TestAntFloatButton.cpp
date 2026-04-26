#include <QSignalSpy>
#include <QTest>
#include "widgets/AntFloatButton.h"

class TestAntFloatButton : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
};

void TestAntFloatButton::propertiesAndSignals()
{
    auto* w = new AntFloatButton;
    QCOMPARE(w->floatButtonType(), Ant::FloatButtonType::Default);
    QCOMPARE(w->floatButtonShape(), Ant::FloatButtonShape::Circle);
    QCOMPARE(w->placement(), Ant::FloatButtonPlacement::BottomRight);
    QCOMPARE(w->isOpen(), false);
    QCOMPARE(w->isBackTop(), false);
    QCOMPARE(w->badgeDot(), false);
    QCOMPARE(w->badgeCount(), 0);

    QSignalSpy typeSpy(w, &AntFloatButton::floatButtonTypeChanged);
    w->setFloatButtonType(Ant::FloatButtonType::Primary);
    QCOMPARE(w->floatButtonType(), Ant::FloatButtonType::Primary);
    QCOMPARE(typeSpy.count(), 1);

    QSignalSpy shapeSpy(w, &AntFloatButton::floatButtonShapeChanged);
    w->setFloatButtonShape(Ant::FloatButtonShape::Square);
    QCOMPARE(w->floatButtonShape(), Ant::FloatButtonShape::Square);
    QCOMPARE(shapeSpy.count(), 1);

    QSignalSpy placeSpy(w, &AntFloatButton::placementChanged);
    w->setPlacement(Ant::FloatButtonPlacement::TopLeft);
    QCOMPARE(w->placement(), Ant::FloatButtonPlacement::TopLeft);
    QCOMPARE(placeSpy.count(), 1);

    QSignalSpy iconSpy(w, &AntFloatButton::iconChanged);
    w->setIcon("search");
    QCOMPARE(w->icon(), "search");
    QCOMPARE(iconSpy.count(), 1);

    QSignalSpy contentSpy(w, &AntFloatButton::contentChanged);
    w->setContent("Help");
    QCOMPARE(w->content(), "Help");
    QCOMPARE(contentSpy.count(), 1);

    QSignalSpy openSpy(w, &AntFloatButton::openChanged);
    w->setOpen(true);
    QCOMPARE(w->isOpen(), true);
    QCOMPARE(openSpy.count(), 1);

    w->setBadgeDot(true);
    QCOMPARE(w->badgeDot(), true);
    w->setBadgeCount(5);
    QCOMPARE(w->badgeCount(), 5);
}

QTEST_MAIN(TestAntFloatButton)
#include "TestAntFloatButton.moc"
