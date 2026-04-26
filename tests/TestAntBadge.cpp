#include <QSignalSpy>
#include <QTest>

#include "widgets/AntBadge.h"

class TestAntBadge : public QObject
{
    Q_OBJECT
private slots:
    void defaultValues();
    void setCount();
    void setText();
    void setColor();
    void setDot();
    void setShowZero();
    void setOverflowCount();
    void setOffset();
    void setBadgeSize();
    void setStatus();
    void setBadgeMode();
    void setRibbonText();
    void setRibbonColor();
    void signalsEmitted();
    void countConstructor();
};

void TestAntBadge::defaultValues()
{
    AntBadge badge;
    QCOMPARE(badge.count(), 0);
    QCOMPARE(badge.text(), QString());
    QCOMPARE(badge.color(), QString());
    QCOMPARE(badge.isDot(), false);
    QCOMPARE(badge.showZero(), false);
    QCOMPARE(badge.overflowCount(), 99);
    QCOMPARE(badge.offset(), QPoint());
    QCOMPARE(badge.badgeSize(), Ant::BadgeSize::Middle);
    QCOMPARE(badge.status(), Ant::BadgeStatus::None);
    QCOMPARE(badge.badgeMode(), Ant::BadgeMode::Default);
    QCOMPARE(badge.ribbonText(), QString());
    QCOMPARE(badge.ribbonColor(), QString());
}

void TestAntBadge::setCount()
{
    AntBadge badge;
    badge.setCount(5);
    QCOMPARE(badge.count(), 5);
    badge.setCount(0);
    QCOMPARE(badge.count(), 0);
    badge.setCount(150);
    QCOMPARE(badge.count(), 150);
}

void TestAntBadge::setText()
{
    AntBadge badge;
    badge.setText("new");
    QCOMPARE(badge.text(), "new");
}

void TestAntBadge::setColor()
{
    AntBadge badge;
    badge.setColor("#f5222d");
    QCOMPARE(badge.color(), "#f5222d");
}

void TestAntBadge::setDot()
{
    AntBadge badge;
    badge.setDot(true);
    QCOMPARE(badge.isDot(), true);
    badge.setDot(false);
    QCOMPARE(badge.isDot(), false);
}

void TestAntBadge::setShowZero()
{
    AntBadge badge;
    badge.setShowZero(true);
    QCOMPARE(badge.showZero(), true);
    badge.setShowZero(false);
    QCOMPARE(badge.showZero(), false);
}

void TestAntBadge::setOverflowCount()
{
    AntBadge badge;
    badge.setOverflowCount(999);
    QCOMPARE(badge.overflowCount(), 999);
}

void TestAntBadge::setOffset()
{
    AntBadge badge;
    badge.setOffset(QPoint(5, -5));
    QCOMPARE(badge.offset(), QPoint(5, -5));
}

void TestAntBadge::setBadgeSize()
{
    AntBadge badge;
    badge.setBadgeSize(Ant::BadgeSize::Small);
    QCOMPARE(badge.badgeSize(), Ant::BadgeSize::Small);
}

void TestAntBadge::setStatus()
{
    AntBadge badge;
    badge.setStatus(Ant::BadgeStatus::Success);
    QCOMPARE(badge.status(), Ant::BadgeStatus::Success);
    badge.setStatus(Ant::BadgeStatus::Error);
    QCOMPARE(badge.status(), Ant::BadgeStatus::Error);
}

void TestAntBadge::setBadgeMode()
{
    AntBadge badge;
    badge.setBadgeMode(Ant::BadgeMode::Dot);
    QCOMPARE(badge.badgeMode(), Ant::BadgeMode::Dot);
    badge.setBadgeMode(Ant::BadgeMode::Ribbon);
    QCOMPARE(badge.badgeMode(), Ant::BadgeMode::Ribbon);
}

void TestAntBadge::setRibbonText()
{
    AntBadge badge;
    badge.setRibbonText("NEW");
    QCOMPARE(badge.ribbonText(), "NEW");
}

void TestAntBadge::setRibbonColor()
{
    AntBadge badge;
    badge.setRibbonColor("green");
    QCOMPARE(badge.ribbonColor(), "green");
}

void TestAntBadge::signalsEmitted()
{
    AntBadge badge;

    QSignalSpy countSpy(&badge, &AntBadge::countChanged);
    badge.setCount(10);
    QCOMPARE(countSpy.count(), 1);
    QCOMPARE(countSpy.at(0).at(0).toInt(), 10);

    QSignalSpy textSpy(&badge, &AntBadge::textChanged);
    badge.setText("hot");
    QCOMPARE(textSpy.count(), 1);

    QSignalSpy colorSpy(&badge, &AntBadge::colorChanged);
    badge.setColor("red");
    QCOMPARE(colorSpy.count(), 1);

    QSignalSpy dotSpy(&badge, &AntBadge::dotChanged);
    badge.setDot(true);
    QCOMPARE(dotSpy.count(), 1);

    QSignalSpy showZeroSpy(&badge, &AntBadge::showZeroChanged);
    badge.setShowZero(true);
    QCOMPARE(showZeroSpy.count(), 1);

    QSignalSpy overflowSpy(&badge, &AntBadge::overflowCountChanged);
    badge.setOverflowCount(199);
    QCOMPARE(overflowSpy.count(), 1);

    QSignalSpy sizeSpy(&badge, &AntBadge::badgeSizeChanged);
    badge.setBadgeSize(Ant::BadgeSize::Small);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy statusSpy(&badge, &AntBadge::statusChanged);
    badge.setStatus(Ant::BadgeStatus::Processing);
    QCOMPARE(statusSpy.count(), 1);

    QSignalSpy modeSpy(&badge, &AntBadge::badgeModeChanged);
    badge.setBadgeMode(Ant::BadgeMode::Dot);
    QCOMPARE(modeSpy.count(), 1);

    QSignalSpy ribbonTextSpy(&badge, &AntBadge::ribbonTextChanged);
    badge.setRibbonText("SALE");
    QCOMPARE(ribbonTextSpy.count(), 1);

    QSignalSpy ribbonColorSpy(&badge, &AntBadge::ribbonColorChanged);
    badge.setRibbonColor("orange");
    QCOMPARE(ribbonColorSpy.count(), 1);
}

void TestAntBadge::countConstructor()
{
    AntBadge badge(42);
    QCOMPARE(badge.count(), 42);
}

QTEST_MAIN(TestAntBadge)
#include "TestAntBadge.moc"
