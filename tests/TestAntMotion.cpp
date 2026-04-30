#include <QSignalSpy>
#include <QTest>
#include <QWidget>

#include <cmath>

#include "core/AntPopupMotion.h"
#include "core/AntWave.h"
#include "widgets/AntRadio.h"

class TestAntMotion : public QObject
{
    Q_OBJECT

private slots:
    void popupMotionShowHideAndClose();
    void popupMotionPlacementMapping();
    void radioButtonStyleClickCreatesWave();
};

namespace
{
bool opacityRestored(const QWidget& widget)
{
    return std::abs(widget.windowOpacity() - 1.0) < 0.05;
}
} // namespace

void TestAntMotion::popupMotionShowHideAndClose()
{
    QWidget popup;
    popup.resize(80, 40);
    popup.move(200, 160);

    AntPopupMotion::show(&popup, AntPopupMotion::Placement::Bottom, 10);
    QVERIFY(popup.isVisible());
    QTRY_COMPARE_WITH_TIMEOUT(popup.pos(), QPoint(200, 160), 500);
    QVERIFY(opacityRestored(popup));
    QVERIFY(!AntPopupMotion::isClosing(&popup));

    AntPopupMotion::hide(&popup, AntPopupMotion::Placement::Right, 8);
    QVERIFY(AntPopupMotion::isClosing(&popup));
    QTRY_VERIFY_WITH_TIMEOUT(!popup.isVisible(), 500);
    QCOMPARE(popup.pos(), QPoint(200, 160));
    QVERIFY(opacityRestored(popup));
    QVERIFY(!AntPopupMotion::isClosing(&popup));

    popup.move(120, 90);
    AntPopupMotion::show(&popup, AntPopupMotion::Placement::Top, 6);
    QTRY_COMPARE_WITH_TIMEOUT(popup.pos(), QPoint(120, 90), 500);

    AntPopupMotion::close(&popup, AntPopupMotion::Placement::Left, 8);
    QVERIFY(AntPopupMotion::isClosing(&popup));
    QTRY_VERIFY_WITH_TIMEOUT(!popup.isVisible(), 500);
    QCOMPARE(popup.pos(), QPoint(120, 90));
    QVERIFY(opacityRestored(popup));
    QVERIFY(!AntPopupMotion::isClosing(&popup));
}

void TestAntMotion::popupMotionPlacementMapping()
{
    QCOMPARE(AntPopupMotion::fromPlacement(Ant::Placement::TopLeft), AntPopupMotion::Placement::Top);
    QCOMPARE(AntPopupMotion::fromPlacement(Ant::Placement::Top), AntPopupMotion::Placement::Top);
    QCOMPARE(AntPopupMotion::fromPlacement(Ant::Placement::BottomRight), AntPopupMotion::Placement::Bottom);

    QCOMPARE(AntPopupMotion::fromDropdownPlacement(Ant::DropdownPlacement::TopRight), AntPopupMotion::Placement::Top);
    QCOMPARE(AntPopupMotion::fromDropdownPlacement(Ant::DropdownPlacement::BottomLeft), AntPopupMotion::Placement::Bottom);

    QCOMPARE(AntPopupMotion::fromTooltipPlacement(Ant::TooltipPlacement::Left), AntPopupMotion::Placement::Left);
    QCOMPARE(AntPopupMotion::fromTooltipPlacement(Ant::TooltipPlacement::Right), AntPopupMotion::Placement::Right);
    QCOMPARE(AntPopupMotion::fromTooltipPlacement(Ant::TooltipPlacement::Top), AntPopupMotion::Placement::Top);
    QCOMPARE(AntPopupMotion::fromTooltipPlacement(Ant::TooltipPlacement::Bottom), AntPopupMotion::Placement::Bottom);
}

void TestAntMotion::radioButtonStyleClickCreatesWave()
{
    QWidget host;
    host.resize(180, 80);

    auto* radio = new AntRadio(QStringLiteral("Option"), &host);
    radio->setButtonStyle(true);
    radio->resize(120, radio->sizeHint().height());
    radio->move(24, 24);

    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    QSignalSpy clickedSpy(radio, &AntRadio::clicked);
    QTest::mouseClick(radio, Qt::LeftButton, Qt::NoModifier, radio->rect().center());

    QCOMPARE(clickedSpy.count(), 1);
    QCOMPARE(radio->isChecked(), true);
    QTRY_VERIFY_WITH_TIMEOUT(!host.findChildren<AntWave*>().isEmpty(), 100);
}

QTEST_MAIN(TestAntMotion)
#include "TestAntMotion.moc"
