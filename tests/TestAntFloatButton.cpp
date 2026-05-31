#include <QSignalSpy>
#include <QTest>
#include <QCoreApplication>
#include <QEvent>
#include <QImage>
#include <QPainter>
#include <QWidget>

#include "core/AntWave.h"
#include "widgets/AntFloatButton.h"

namespace
{

int maxAlphaInRect(const QImage& image, const QRect& rect)
{
    int maxAlpha = 0;
    const QRect bounded = rect.intersected(image.rect());
    for (int y = bounded.top(); y <= bounded.bottom(); ++y)
    {
        for (int x = bounded.left(); x <= bounded.right(); ++x)
        {
            maxAlpha = qMax(maxAlpha, qAlpha(image.pixel(x, y)));
        }
    }
    return maxAlpha;
}

} // namespace

class TestAntFloatButton : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
    void shadowMarginAndClickAnimation();
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

    auto* host = new QWidget;
    host->resize(240, 240);
    auto* group = new AntFloatButton(host);
    auto* child = new AntFloatButton;
    group->addChild(child);
    host->show();
    group->show();
    QVERIFY(QTest::qWaitForWindowExposed(host));
    QTRY_VERIFY(group->property("antFloatButtonPositionApplyCount").toInt() > 0);

    group->setOpen(true);
    QCOMPARE(group->isOpen(), true);
    QVERIFY(child->isVisible());
    const int childLayoutApplies = group->property("antFloatButtonChildLayoutApplyCount").toInt();
    QVERIFY(childLayoutApplies > 0);

    const int positionSkips = group->property("antFloatButtonPositionSkipCount").toInt();
    QEvent showAgain(QEvent::Show);
    QCoreApplication::sendEvent(host, &showAgain);
    QTRY_VERIFY(group->property("antFloatButtonPositionSkipCount").toInt() > positionSkips);
    QCOMPARE(group->property("antFloatButtonChildLayoutApplyCount").toInt(), childLayoutApplies);

    const int positionApplies = group->property("antFloatButtonPositionApplyCount").toInt();
    host->resize(260, 260);
    QTRY_VERIFY(group->property("antFloatButtonPositionApplyCount").toInt() > positionApplies);
    QVERIFY(group->property("antFloatButtonChildLayoutApplyCount").toInt() > childLayoutApplies);

    const int contentSizeApplies = group->property("antFloatButtonContentSizeApplyCount").toInt();
    group->setFloatButtonShape(Ant::FloatButtonShape::Square);
    group->setContent(QStringLiteral("Support"));
    QVERIFY(group->property("antFloatButtonContentSizeApplyCount").toInt() > contentSizeApplies);
    const int contentSizeAfterText = group->property("antFloatButtonContentSizeApplyCount").toInt();
    group->setContent(QStringLiteral("Support"));
    QCOMPARE(group->property("antFloatButtonContentSizeApplyCount").toInt(), contentSizeAfterText);
}

void TestAntFloatButton::shadowMarginAndClickAnimation()
{
    QWidget host;
    host.resize(180, 180);

    auto* button = new AntFloatButton(&host);
    button->move(40, 40);
    button->show();
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    const QRect visualRect = button->buttonRect();
    QCOMPARE(visualRect.size(), QSize(AntFloatButton::VisualButtonSize, AntFloatButton::VisualButtonSize));
    QVERIFY(button->sizeHint().width() > visualRect.width());
    QVERIFY(button->sizeHint().height() > visualRect.height());

    QImage image(button->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    {
        QPainter painter(&image);
        button->render(&painter);
    }

    const QRect topShadowRect(visualRect.left(), 0, visualRect.width(), visualRect.top());
    QVERIFY(maxAlphaInRect(image, topShadowRect) > 0);
    QVERIFY(qAlpha(image.pixel(visualRect.center())) > 200);

    QSignalSpy clickSpy(button, &AntFloatButton::clicked);
    QTest::mouseClick(button, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QCOMPARE(clickSpy.count(), 0);

    QTest::mousePress(button, Qt::LeftButton, Qt::NoModifier, visualRect.center());
    QTRY_VERIFY(button->pressProgress() > 0.0);
    QTest::mouseRelease(button, Qt::LeftButton, Qt::NoModifier, visualRect.center());
    QCOMPARE(clickSpy.count(), 1);
    QTRY_VERIFY(!host.findChildren<AntWave*>().isEmpty());
    QTRY_VERIFY(button->pressProgress() < 0.05);
}

QTEST_MAIN(TestAntFloatButton)
#include "TestAntFloatButton.moc"
