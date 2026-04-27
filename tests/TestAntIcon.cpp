#include <QSignalSpy>
#include <QTest>
#include "widgets/AntIcon.h"

class TestAntIcon : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
};

void TestAntIcon::propertiesAndSignals()
{
    auto* w = new AntIcon;
    QCOMPARE(w->iconType(), Ant::IconType::None);
    QCOMPARE(w->iconTheme(), Ant::IconTheme::Outlined);
    QVERIFY(w->iconSize() > 0);
    QCOMPARE(w->rotate(), 0);
    QCOMPARE(w->isSpin(), false);

    QSignalSpy typeSpy(w, &AntIcon::iconTypeChanged);
    w->setIconType(Ant::IconType::Search);
    QCOMPARE(w->iconType(), Ant::IconType::Search);
    QCOMPARE(typeSpy.count(), 1);

    QSignalSpy themeSpy(w, &AntIcon::iconThemeChanged);
    w->setIconTheme(Ant::IconTheme::Filled);
    QCOMPARE(w->iconTheme(), Ant::IconTheme::Filled);
    QCOMPARE(themeSpy.count(), 1);

    QSignalSpy sizeSpy(w, &AntIcon::iconSizeChanged);
    w->setIconSize(24);
    QCOMPARE(w->iconSize(), 24);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy spinSpy(w, &AntIcon::spinChanged);
    w->setSpin(true);
    QCOMPARE(w->isSpin(), true);
    QCOMPARE(spinSpy.count(), 1);

    QSignalSpy rotateSpy(w, &AntIcon::rotateChanged);
    w->setRotate(90);
    QCOMPARE(w->rotate(), 90);
    QCOMPARE(rotateSpy.count(), 1);

    QPainterPath primaryPath;
    primaryPath.addRect(QRectF(0, 0, 32, 32));
    QPainterPath secondaryPath;
    secondaryPath.addEllipse(QRectF(8, 8, 16, 16));
    w->setCustomPath(primaryPath, secondaryPath);
    QVERIFY(w->hasCustomPath());
    QVERIFY(!w->customPrimaryPath().isEmpty());
    QVERIFY(!w->customSecondaryPath().isEmpty());
    w->clearCustomPath();
    QVERIFY(!w->hasCustomPath());
}

QTEST_MAIN(TestAntIcon)
#include "TestAntIcon.moc"
