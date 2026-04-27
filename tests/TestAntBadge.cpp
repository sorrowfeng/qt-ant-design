#include <QSignalSpy>
#include <QTest>

#include "widgets/AntBadge.h"

class TestAntBadge : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
};

void TestAntBadge::propertiesAndSignals()
{
    auto* badge = new AntBadge;
    QCOMPARE(badge->count(), 0);
    QCOMPARE(badge->text(), QString());
    QCOMPARE(badge->color(), QString());
    QCOMPARE(badge->isDot(), false);
    QCOMPARE(badge->showZero(), false);
    QCOMPARE(badge->overflowCount(), 99);
    QCOMPARE(badge->offset(), QPoint());
    QCOMPARE(badge->badgeSize(), Ant::Size::Middle);
    QCOMPARE(badge->status(), Ant::BadgeStatus::None);
    QCOMPARE(badge->badgeMode(), Ant::BadgeMode::Default);
    QCOMPARE(badge->ribbonText(), QString());
    QCOMPARE(badge->ribbonColor(), QString());

    QSignalSpy countSpy(badge, &AntBadge::countChanged);
    badge->setCount(10);
    QCOMPARE(badge->count(), 10);
    QCOMPARE(countSpy.count(), 1);
    QCOMPARE(countSpy.at(0).at(0).toInt(), 10);

    QSignalSpy textSpy(badge, &AntBadge::textChanged);
    badge->setText("hot");
    QCOMPARE(badge->text(), "hot");
    QCOMPARE(textSpy.count(), 1);

    QSignalSpy colorSpy(badge, &AntBadge::colorChanged);
    badge->setColor("red");
    QCOMPARE(badge->color(), "red");
    QCOMPARE(colorSpy.count(), 1);

    QSignalSpy dotSpy(badge, &AntBadge::dotChanged);
    badge->setDot(true);
    QCOMPARE(badge->isDot(), true);
    QCOMPARE(dotSpy.count(), 1);

    QSignalSpy showZeroSpy(badge, &AntBadge::showZeroChanged);
    badge->setShowZero(true);
    QCOMPARE(badge->showZero(), true);
    QCOMPARE(showZeroSpy.count(), 1);

    QSignalSpy overflowSpy(badge, &AntBadge::overflowCountChanged);
    badge->setOverflowCount(199);
    QCOMPARE(badge->overflowCount(), 199);
    QCOMPARE(overflowSpy.count(), 1);

    QSignalSpy offsetSpy(badge, &AntBadge::offsetChanged);
    badge->setOffset(QPoint(5, -5));
    QCOMPARE(badge->offset(), QPoint(5, -5));
    QCOMPARE(offsetSpy.count(), 1);

    QSignalSpy sizeSpy(badge, &AntBadge::badgeSizeChanged);
    badge->setBadgeSize(Ant::Size::Small);
    QCOMPARE(badge->badgeSize(), Ant::Size::Small);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy statusSpy(badge, &AntBadge::statusChanged);
    badge->setStatus(Ant::BadgeStatus::Processing);
    QCOMPARE(badge->status(), Ant::BadgeStatus::Processing);
    QCOMPARE(badge->processingPulseProgress(), 0.0);
    QCOMPARE(statusSpy.count(), 1);

    QSignalSpy modeSpy(badge, &AntBadge::badgeModeChanged);
    badge->setBadgeMode(Ant::BadgeMode::Dot);
    QCOMPARE(badge->badgeMode(), Ant::BadgeMode::Dot);
    QCOMPARE(modeSpy.count(), 1);

    QSignalSpy ribbonTextSpy(badge, &AntBadge::ribbonTextChanged);
    badge->setRibbonText("SALE");
    QCOMPARE(badge->ribbonText(), "SALE");
    QCOMPARE(ribbonTextSpy.count(), 1);

    QSignalSpy ribbonColorSpy(badge, &AntBadge::ribbonColorChanged);
    badge->setRibbonColor("orange");
    QCOMPARE(badge->ribbonColor(), "orange");
    QCOMPARE(ribbonColorSpy.count(), 1);

    auto* countBadge = new AntBadge(42);
    QCOMPARE(countBadge->count(), 42);
}

QTEST_MAIN(TestAntBadge)
#include "TestAntBadge.moc"
