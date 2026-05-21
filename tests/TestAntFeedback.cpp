#include <QCoreApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QMoveEvent>
#include <QSignalSpy>
#include <QPointer>
#include <QTest>
#include "widgets/AntAlert.h"
#include "widgets/AntButton.h"
#include "widgets/AntDrawer.h"
#include "widgets/AntMessage.h"
#include "widgets/AntNotification.h"
#include "widgets/AntPopconfirm.h"
#include "widgets/AntPopover.h"
#include "widgets/AntProgress.h"
#include "widgets/AntResult.h"
#include "widgets/AntSkeleton.h"
#include "widgets/AntSpin.h"
#include "widgets/AntToolTip.h"
#include "widgets/AntTour.h"

class TestAntFeedback : public QObject
{
    Q_OBJECT
private slots:
    void alert();
    void alertCachesLayoutAndScopesCloseHover();
    void drawer();
    void drawerCachesGeometryAndScopesAnimationUpdates();
    void message();
    void messageCachesLayoutShadowAndScopesUpdates();
    void notification();
    void notificationCachesLayoutShadowAndScopesUpdates();
    void notificationLoadingProgressAutoCloses();
    void popconfirm();
    void popconfirmReusesActionWidgetAndSkipsRebuilds();
    void popover();
    void popoverCachesLayoutAndSkipsPlacementWork();
    void progress();
    void progressCachesGeometryAndScopesUpdates();
    void result();
    void resultCachesLayoutIconAndExtraGeometry();
    void skeleton();
    void spin();
    void tooltip();
    void tour();
};

void TestAntFeedback::alert()
{
    auto* w = new AntAlert;
    QCOMPARE(w->title(), QString());
    QCOMPARE(w->description(), QString());
    QCOMPARE(w->alertType(), Ant::AlertType::Info);
    QCOMPARE(w->showIcon(), false);
    QCOMPARE(w->isClosable(), false);
    QCOMPARE(w->isBanner(), false);

    QSignalSpy titleSpy(w, &AntAlert::titleChanged);
    w->setTitle("Warning");
    QCOMPARE(w->title(), "Warning");
    QCOMPARE(titleSpy.count(), 1);

    QSignalSpy descSpy(w, &AntAlert::descriptionChanged);
    w->setDescription("Something happened");
    QCOMPARE(w->description(), "Something happened");
    QCOMPARE(descSpy.count(), 1);

    QSignalSpy typeSpy(w, &AntAlert::alertTypeChanged);
    w->setAlertType(Ant::AlertType::Warning);
    QCOMPARE(w->alertType(), Ant::AlertType::Warning);
    QCOMPARE(typeSpy.count(), 1);

    QSignalSpy iconSpy(w, &AntAlert::showIconChanged);
    w->setShowIcon(true);
    QCOMPARE(w->showIcon(), true);
    QCOMPARE(iconSpy.count(), 1);

    QSignalSpy closeSpy(w, &AntAlert::closableChanged);
    w->setClosable(true);
    QCOMPARE(w->isClosable(), true);
    QCOMPARE(closeSpy.count(), 1);

    QSignalSpy bannerSpy(w, &AntAlert::bannerChanged);
    w->setBanner(true);
    QCOMPARE(w->isBanner(), true);
    QCOMPARE(bannerSpy.count(), 1);

    auto* w2 = new AntAlert("Title");
    QCOMPARE(w2->title(), "Title");
}

void TestAntFeedback::alertCachesLayoutAndScopesCloseHover()
{
    AntAlert alert;
    alert.setTitle(QStringLiteral("Cached alert"));
    alert.setDescription(QStringLiteral("Repeated paints should reuse layout and icon pixmaps."));
    alert.setShowIcon(true);
    alert.setClosable(true);
    alert.resize(420, 96);
    alert.show();
    QVERIFY(QTest::qWaitForWindowExposed(&alert));

    alert.grab();
    const int layoutBuilds = alert.property("antAlertLayoutBuildCount").toInt();
    const int layoutHits = alert.property("antAlertLayoutCacheHitCount").toInt();
    const int iconBuilds = alert.property("antAlertIconPixmapBuildCount").toInt();
    const int iconHits = alert.property("antAlertIconPixmapCacheHitCount").toInt();
    QVERIFY(layoutBuilds > 0);
    QVERIFY(iconBuilds > 0);

    alert.grab();
    QCOMPARE(alert.property("antAlertLayoutBuildCount").toInt(), layoutBuilds);
    QCOMPARE(alert.property("antAlertIconPixmapBuildCount").toInt(), iconBuilds);
    QVERIFY(alert.property("antAlertLayoutCacheHitCount").toInt() > layoutHits);
    QVERIFY(alert.property("antAlertIconPixmapCacheHitCount").toInt() > iconHits);

    auto sendMove = [&alert](const QPoint& point) {
        QMouseEvent event(QEvent::MouseMove, QPointF(point), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        QCoreApplication::sendEvent(&alert, &event);
    };

    const int closeUpdates = alert.property("antAlertCloseRegionUpdateCount").toInt();
    sendMove(QPoint(alert.width() - 26, 22));
    QCOMPARE(alert.property("antAlertLastUpdateMode").toString(), QStringLiteral("closeHover"));
    QVERIFY(alert.property("antAlertCloseRegionUpdateCount").toInt() > closeUpdates);

    const int closeUpdatesAfterEnter = alert.property("antAlertCloseRegionUpdateCount").toInt();
    sendMove(QPoint(16, alert.height() - 12));
    QCOMPARE(alert.property("antAlertLastUpdateMode").toString(), QStringLiteral("closeHover"));
    QVERIFY(alert.property("antAlertCloseRegionUpdateCount").toInt() > closeUpdatesAfterEnter);

    const int layoutBuildsBeforeChange = alert.property("antAlertLayoutBuildCount").toInt();
    alert.setDescription(QStringLiteral("Changing content invalidates layout once and repaints the alert body."));
    QCOMPARE(alert.property("antAlertLastUpdateMode").toString(), QStringLiteral("description"));
    alert.grab();
    QVERIFY(alert.property("antAlertLayoutBuildCount").toInt() > layoutBuildsBeforeChange);
}

void TestAntFeedback::drawer()
{
    auto* w = new AntDrawer;
    QCOMPARE(w->title(), QString());
    QCOMPARE(w->placement(), Ant::DrawerPlacement::Right);
    QCOMPARE(w->drawerWidth(), 378);
    QCOMPARE(w->drawerHeight(), 378);
    QCOMPARE(w->isClosable(), true);
    QCOMPARE(w->isMaskClosable(), true);
    QCOMPARE(w->isOpen(), false);

    QSignalSpy titleSpy(w, &AntDrawer::titleChanged);
    w->setTitle("Settings");
    QCOMPARE(w->title(), "Settings");
    QCOMPARE(titleSpy.count(), 1);

    QSignalSpy placeSpy(w, &AntDrawer::placementChanged);
    w->setPlacement(Ant::DrawerPlacement::Left);
    QCOMPARE(w->placement(), Ant::DrawerPlacement::Left);
    QCOMPARE(placeSpy.count(), 1);

    QSignalSpy wSpy(w, &AntDrawer::drawerWidthChanged);
    w->setDrawerWidth(400);
    QCOMPARE(w->drawerWidth(), 400);
    QCOMPARE(wSpy.count(), 1);

    QSignalSpy hSpy(w, &AntDrawer::drawerHeightChanged);
    w->setDrawerHeight(500);
    QCOMPARE(w->drawerHeight(), 500);
    QCOMPARE(hSpy.count(), 1);

    QSignalSpy closeSpy(w, &AntDrawer::closableChanged);
    w->setClosable(false);
    QCOMPARE(w->isClosable(), false);
    QCOMPARE(closeSpy.count(), 1);

    QSignalSpy maskSpy(w, &AntDrawer::maskClosableChanged);
    w->setMaskClosable(false);
    QCOMPARE(w->isMaskClosable(), false);
    QCOMPARE(maskSpy.count(), 1);

    QSignalSpy openSpy(w, &AntDrawer::openChanged);
    w->setOpen(true);
    QCOMPARE(w->isOpen(), true);
    QCOMPARE(openSpy.count(), 1);
}

void TestAntFeedback::drawerCachesGeometryAndScopesAnimationUpdates()
{
    QWidget host;
    host.resize(480, 320);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    AntDrawer drawer(&host);
    drawer.setTitle(QStringLiteral("Drawer"));
    drawer.setDrawerWidth(220);
    drawer.setBodyWidget(new QLabel(QStringLiteral("Drawer body")));

    QSignalSpy openedSpy(&drawer, &AntDrawer::opened);
    drawer.setOpen(true);
    QTRY_COMPARE_WITH_TIMEOUT(openedSpy.count(), 1, 700);
    QVERIFY(drawer.isOpen());
    QVERIFY(drawer.property("antDrawerAnimationRegionUpdateCount").toInt() > 0);
    QVERIFY(drawer.property("antDrawerMaskRegionUpdateCount").toInt() > 0);
    QVERIFY(drawer.property("antDrawerPanelRegionUpdateCount").toInt() > 0);
    QVERIFY(drawer.property("antDrawerGeometryBuildCount").toInt() > 0);

    const int geometryHits = drawer.property("antDrawerGeometryCacheHitCount").toInt();
    (void)drawer.maskProgress();
    QVERIFY(drawer.property("antDrawerGeometryCacheHitCount").toInt() > geometryHits);

    const int panelApplies = drawer.property("antDrawerPanelGeometryApplyCount").toInt();
    const int themeSkips = drawer.property("antDrawerThemeSkipCount").toInt();
    drawer.setDrawerWidth(260);
    QVERIFY(drawer.property("antDrawerPanelGeometryApplyCount").toInt() > panelApplies);
    QVERIFY(drawer.property("antDrawerThemeSkipCount").toInt() > themeSkips);

    QSignalSpy closedSpy(&drawer, &AntDrawer::closed);
    drawer.setOpen(false);
    QTRY_COMPARE_WITH_TIMEOUT(closedSpy.count(), 1, 700);
    QVERIFY(!drawer.isOpen());
}

void TestAntFeedback::message()
{
    auto* w = new AntMessage;
    QCOMPARE(w->text(), QString());
    QCOMPARE(w->messageType(), Ant::MessageType::Info);
    QCOMPARE(w->duration(), 3000);
    QCOMPARE(w->pauseOnHover(), true);
    QCOMPARE(w->loadingAngle(), 0);

    QSignalSpy textSpy(w, &AntMessage::textChanged);
    w->setText("Saved!");
    QCOMPARE(w->text(), "Saved!");
    QCOMPARE(textSpy.count(), 1);

    QSignalSpy typeSpy(w, &AntMessage::messageTypeChanged);
    w->setMessageType(Ant::MessageType::Success);
    QCOMPARE(w->messageType(), Ant::MessageType::Success);
    QCOMPARE(typeSpy.count(), 1);

    QSignalSpy durSpy(w, &AntMessage::durationChanged);
    w->setDuration(5000);
    QCOMPARE(w->duration(), 5000);
    QCOMPARE(durSpy.count(), 1);

    QSignalSpy pauseSpy(w, &AntMessage::pauseOnHoverChanged);
    w->setPauseOnHover(false);
    QCOMPARE(w->pauseOnHover(), false);
    QCOMPARE(pauseSpy.count(), 1);
}

void TestAntFeedback::messageCachesLayoutShadowAndScopesUpdates()
{
    AntMessage message;
    message.setText(QStringLiteral("Saved"));
    message.setMessageType(Ant::MessageType::Success);

    message.grab();
    const int layoutBuilds = message.property("antMessageLayoutBuildCount").toInt();
    const int layoutHits = message.property("antMessageLayoutCacheHitCount").toInt();
    const int shadowBuilds = message.property("antMessageShadowBuildCount").toInt();
    const int shadowHits = message.property("antMessageShadowCacheHitCount").toInt();
    QVERIFY(layoutBuilds > 0);
    QVERIFY(shadowBuilds > 0);

    message.grab();
    QCOMPARE(message.property("antMessageLayoutBuildCount").toInt(), layoutBuilds);
    QCOMPARE(message.property("antMessageShadowBuildCount").toInt(), shadowBuilds);
    QVERIFY(message.property("antMessageLayoutCacheHitCount").toInt() > layoutHits);
    QVERIFY(message.property("antMessageShadowCacheHitCount").toInt() > shadowHits);

    message.setMessageType(Ant::MessageType::Loading);
    QCOMPARE(message.property("antMessageLastUpdateMode").toString(), QStringLiteral("type"));
    message.show();
    QVERIFY(QTest::qWaitForWindowExposed(&message));
    const int spinnerUpdates = message.property("antMessageSpinnerRegionUpdateCount").toInt();
    QTRY_VERIFY_WITH_TIMEOUT(message.property("antMessageSpinnerRegionUpdateCount").toInt() > spinnerUpdates, 300);
    QCOMPARE(message.property("antMessageLastUpdateMode").toString(), QStringLiteral("loading"));
    message.hide();

    QWidget anchor;
    anchor.resize(480, 240);
    anchor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&anchor));

    QPointer<AntMessage> first = AntMessage::success(QStringLiteral("First"), &anchor, 0, Ant::Placement::Top);
    QVERIFY(first);
    QTRY_VERIFY_WITH_TIMEOUT(first->isVisible(), 300);
    QTest::qWait(260);
    const int firstRelayoutSkips = first->property("antMessageRelayoutSkipCount").toInt();

    QPointer<AntMessage> second = AntMessage::info(QStringLiteral("Second"), &anchor, 0, Ant::Placement::Top);
    QVERIFY(second);
    QTRY_VERIFY_WITH_TIMEOUT(second->isVisible(), 300);
    QVERIFY(first->property("antMessageRelayoutSkipCount").toInt() > firstRelayoutSkips);

    if (second)
    {
        second->close();
    }
    if (first)
    {
        first->close();
    }
}

void TestAntFeedback::notification()
{
    auto* w = new AntNotification;
    QCOMPARE(w->title(), QString());
    QCOMPARE(w->description(), QString());
    QCOMPARE(w->notificationType(), Ant::MessageType::Info);
    QCOMPARE(w->placement(), Ant::Placement::TopRight);
    QCOMPARE(w->duration(), 4500);
    QCOMPARE(w->pauseOnHover(), true);
    QCOMPARE(w->showProgress(), false);
    QCOMPARE(w->isClosable(), true);
    QCOMPARE(w->iconVisible(), true);
    QCOMPARE(w->spinnerAngle(), 0);
    QVERIFY(qFuzzyCompare(w->progressRatio(), 1.0));

    QSignalSpy titleSpy(w, &AntNotification::titleChanged);
    w->setTitle("Update");
    QCOMPARE(w->title(), "Update");
    QCOMPARE(titleSpy.count(), 1);

    QSignalSpy descSpy(w, &AntNotification::descriptionChanged);
    w->setDescription("New version available");
    QCOMPARE(w->description(), "New version available");
    QCOMPARE(descSpy.count(), 1);

    QSignalSpy typeSpy(w, &AntNotification::notificationTypeChanged);
    w->setNotificationType(Ant::MessageType::Warning);
    QCOMPARE(w->notificationType(), Ant::MessageType::Warning);
    QCOMPARE(typeSpy.count(), 1);

    QSignalSpy iconSpy(w, &AntNotification::iconVisibleChanged);
    w->setIconVisible(false);
    QCOMPARE(w->iconVisible(), false);
    QCOMPARE(iconSpy.count(), 1);

    QSignalSpy placeSpy(w, &AntNotification::placementChanged);
    w->setPlacement(Ant::Placement::BottomLeft);
    QCOMPARE(w->placement(), Ant::Placement::BottomLeft);
    QCOMPARE(placeSpy.count(), 1);

    QSignalSpy durSpy(w, &AntNotification::durationChanged);
    w->setDuration(6000);
    QCOMPARE(w->duration(), 6000);
    QCOMPARE(durSpy.count(), 1);

    QSignalSpy progressSpy(w, &AntNotification::showProgressChanged);
    w->setShowProgress(true);
    QCOMPARE(w->showProgress(), true);
    QCOMPARE(progressSpy.count(), 1);

    QSignalSpy closeSpy(w, &AntNotification::closableChanged);
    w->setClosable(false);
    QCOMPARE(w->isClosable(), false);
    QCOMPARE(closeSpy.count(), 1);
}

void TestAntFeedback::notificationCachesLayoutShadowAndScopesUpdates()
{
    AntNotification notification;
    notification.setTitle(QStringLiteral("Cached notice"));
    notification.setDescription(QStringLiteral("Repeated paints should reuse notification layout and soft shadow."));
    notification.setNotificationType(Ant::MessageType::Success);
    notification.setDuration(1200);
    notification.setShowProgress(true);
    notification.resize(notification.sizeHint());

    notification.grab();
    const int layoutBuilds = notification.property("antNotificationLayoutBuildCount").toInt();
    const int layoutHits = notification.property("antNotificationLayoutCacheHitCount").toInt();
    const int shadowBuilds = notification.property("antNotificationShadowBuildCount").toInt();
    const int shadowHits = notification.property("antNotificationShadowCacheHitCount").toInt();
    QVERIFY(layoutBuilds > 0);
    QVERIFY(shadowBuilds > 0);

    notification.grab();
    QCOMPARE(notification.property("antNotificationShadowBuildCount").toInt(), shadowBuilds);
    QVERIFY(notification.property("antNotificationLayoutCacheHitCount").toInt() > layoutHits);
    QVERIFY(notification.property("antNotificationShadowCacheHitCount").toInt() > shadowHits);

    notification.show();
    QVERIFY(QTest::qWaitForWindowExposed(&notification));

    const int progressUpdates = notification.property("antNotificationProgressRegionUpdateCount").toInt();
    QTRY_VERIFY_WITH_TIMEOUT(notification.property("antNotificationProgressRegionUpdateCount").toInt() > progressUpdates,
                             250);
    QCOMPARE(notification.property("antNotificationLastUpdateMode").toString(), QStringLiteral("progress"));

    notification.setNotificationType(Ant::MessageType::Loading);
    const int spinnerUpdates = notification.property("antNotificationSpinnerRegionUpdateCount").toInt();
    QTRY_VERIFY_WITH_TIMEOUT(notification.property("antNotificationSpinnerRegionUpdateCount").toInt() > spinnerUpdates,
                             300);

    auto sendMove = [&notification](const QPoint& point) {
        QMouseEvent event(QEvent::MouseMove, QPointF(point), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        QCoreApplication::sendEvent(&notification, &event);
    };

    const int closeUpdates = notification.property("antNotificationCloseRegionUpdateCount").toInt();
    sendMove(QPoint(notification.width() - 52, 42));
    QCOMPARE(notification.property("antNotificationLastUpdateMode").toString(), QStringLiteral("closeHover"));
    QVERIFY(notification.property("antNotificationCloseRegionUpdateCount").toInt() > closeUpdates);

    const int closeUpdatesAfterEnter = notification.property("antNotificationCloseRegionUpdateCount").toInt();
    sendMove(QPoint(24, notification.height() - 20));
    QCOMPARE(notification.property("antNotificationLastUpdateMode").toString(), QStringLiteral("closeHover"));
    QVERIFY(notification.property("antNotificationCloseRegionUpdateCount").toInt() > closeUpdatesAfterEnter);
    notification.hide();

    QWidget anchor;
    anchor.resize(520, 360);
    anchor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&anchor));

    QPointer<AntNotification> first =
        AntNotification::info(QStringLiteral("First"), QStringLiteral("Cached stack"), &anchor, 0);
    QVERIFY(first);
    QTRY_VERIFY_WITH_TIMEOUT(first->isVisible(), 300);
    QTest::qWait(260);
    const int relayoutSkips = first->property("antNotificationRelayoutSkipCount").toInt();

    QPointer<AntNotification> second =
        AntNotification::info(QStringLiteral("Second"), QStringLiteral("Cached stack"), &anchor, 0);
    QVERIFY(second);
    QTRY_VERIFY_WITH_TIMEOUT(second->isVisible(), 300);
    QVERIFY(first->property("antNotificationRelayoutSkipCount").toInt() > relayoutSkips);

    if (second)
    {
        second->close();
    }
    if (first)
    {
        first->close();
    }
}

void TestAntFeedback::notificationLoadingProgressAutoCloses()
{
    QWidget anchor;
    anchor.resize(480, 320);
    anchor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&anchor));

    QPointer<AntNotification> notification = AntNotification::open(QStringLiteral("Loading"),
                                                                    QStringLiteral("Processing request"),
                                                                    Ant::MessageType::Loading,
                                                                    &anchor,
                                                                    600,
                                                                    Ant::Placement::TopRight);
    QVERIFY(notification);
    notification->setPauseOnHover(false);
    notification->setShowProgress(true);

    QTRY_VERIFY_WITH_TIMEOUT(notification && notification->isVisible(), 300);
    QCOMPARE(notification->notificationType(), Ant::MessageType::Loading);
    QCOMPARE(notification->showProgress(), true);
    QVERIFY(notification->progressRatio() <= 1.0);
    QTRY_VERIFY_WITH_TIMEOUT(notification && notification->progressRatio() < 0.98, 350);
    QTRY_VERIFY_WITH_TIMEOUT(notification.isNull(), 1600);
}

void TestAntFeedback::popconfirm()
{
    auto* w = new AntPopconfirm;
    QCOMPARE(w->title(), QString());
    QCOMPARE(w->description(), QString());
    QCOMPARE(w->okText(), QStringLiteral("OK"));
    QCOMPARE(w->cancelText(), QStringLiteral("Cancel"));
    QCOMPARE(w->showCancel(), true);
    QCOMPARE(w->isDisabled(), false);
    QCOMPARE(w->isOpen(), false);

    QSignalSpy titleSpy(w, &AntPopconfirm::titleChanged);
    w->setTitle("Are you sure?");
    QCOMPARE(w->title(), "Are you sure?");
    QCOMPARE(titleSpy.count(), 1);

    QSignalSpy descSpy(w, &AntPopconfirm::descriptionChanged);
    w->setDescription("This cannot be undone");
    QCOMPARE(w->description(), "This cannot be undone");
    QCOMPARE(descSpy.count(), 1);

    QSignalSpy okSpy(w, &AntPopconfirm::okTextChanged);
    w->setOkText("Yes");
    QCOMPARE(w->okText(), "Yes");
    QCOMPARE(okSpy.count(), 1);

    QSignalSpy cancelSpy(w, &AntPopconfirm::cancelTextChanged);
    w->setCancelText("No");
    QCOMPARE(w->cancelText(), "No");
    QCOMPARE(cancelSpy.count(), 1);

    QSignalSpy showSpy(w, &AntPopconfirm::showCancelChanged);
    w->setShowCancel(false);
    QCOMPARE(w->showCancel(), false);
    QCOMPARE(showSpy.count(), 1);

    auto* target = new QWidget;
    target->resize(80, 32);
    w->setTarget(target);
    QCOMPARE(w->target(), target);

    QSignalSpy disSpy(w, &AntPopconfirm::disabledChanged);
    w->setDisabled(true);
    QCOMPARE(w->isDisabled(), true);
    QCOMPARE(disSpy.count(), 1);
    w->setOpen(true);
    QCOMPARE(w->isOpen(), false);
    QTest::mouseClick(target, Qt::LeftButton);
    QCOMPARE(w->isOpen(), false);
}

void TestAntFeedback::popconfirmReusesActionWidgetAndSkipsRebuilds()
{
    AntPopconfirm popconfirm;
    auto* popover = popconfirm.findChild<AntPopover*>();
    QVERIFY(popover != nullptr);
    QWidget* actionWidget = popover->actionWidget();
    QVERIFY(actionWidget != nullptr);
    auto* cancelButton = actionWidget->findChild<AntButton*>(QStringLiteral("AntPopconfirmCancelButton"));
    auto* okButton = actionWidget->findChild<AntButton*>(QStringLiteral("AntPopconfirmOkButton"));
    QVERIFY(cancelButton != nullptr);
    QVERIFY(okButton != nullptr);

    const int initialBuilds = popconfirm.property("antPopconfirmActionBuildCount").toInt();
    const int initialAttaches = popconfirm.property("antPopconfirmActionAttachCount").toInt();
    QVERIFY(initialBuilds > 0);
    QCOMPARE(initialAttaches, 1);

    popconfirm.setOkText(QStringLiteral("Proceed"));
    QCOMPARE(popover->actionWidget(), actionWidget);
    QCOMPARE(actionWidget->findChild<AntButton*>(QStringLiteral("AntPopconfirmOkButton")), okButton);
    QCOMPARE(okButton->text(), QStringLiteral("Proceed"));
    QCOMPARE(popconfirm.property("antPopconfirmActionBuildCount").toInt(), initialBuilds);
    QCOMPARE(popconfirm.property("antPopconfirmActionAttachCount").toInt(), initialAttaches);
    QVERIFY(popconfirm.property("antPopconfirmActionSyncApplyCount").toInt() > 1);

    popconfirm.setCancelText(QStringLiteral("Back"));
    QCOMPARE(popover->actionWidget(), actionWidget);
    QCOMPARE(actionWidget->findChild<AntButton*>(QStringLiteral("AntPopconfirmCancelButton")), cancelButton);
    QCOMPARE(cancelButton->text(), QStringLiteral("Back"));
    QCOMPARE(popconfirm.property("antPopconfirmActionBuildCount").toInt(), initialBuilds);

    popconfirm.setShowCancel(false);
    QCOMPARE(popover->actionWidget(), actionWidget);
    QCOMPARE(actionWidget->findChild<AntButton*>(QStringLiteral("AntPopconfirmCancelButton")), cancelButton);
    QVERIFY(!cancelButton->isVisibleTo(actionWidget));
    QCOMPARE(popconfirm.property("antPopconfirmActionBuildCount").toInt(), initialBuilds);

    popconfirm.setShowCancel(true);
    QCOMPARE(popover->actionWidget(), actionWidget);
    QCOMPARE(actionWidget->findChild<AntButton*>(QStringLiteral("AntPopconfirmCancelButton")), cancelButton);
    QVERIFY(cancelButton->isVisibleTo(actionWidget));
    QCOMPARE(popconfirm.property("antPopconfirmActionBuildCount").toInt(), initialBuilds);

    popconfirm.setTitle(QStringLiteral("Delete?"));
    popconfirm.setDescription(QStringLiteral("This operation cannot be undone."));
    QVERIFY(popconfirm.property("antPopconfirmContentSyncApplyCount").toInt() >= 2);
    QCOMPARE(popconfirm.property("antPopconfirmLastSyncMode").toString(), QStringLiteral("content"));
}

void TestAntFeedback::popover()
{
    auto* w = new AntPopover;
    QCOMPARE(w->title(), QString());
    QCOMPARE(w->titleIconType(), Ant::IconType::None);
    QCOMPARE(w->content(), QString());
    QCOMPARE(w->placement(), Ant::TooltipPlacement::Top);
    QCOMPARE(w->renderPlacement(), Ant::TooltipPlacement::Top);
    QCOMPARE(w->trigger(), Ant::PopoverTrigger::Hover);
    QCOMPARE(w->arrowVisible(), true);
    QCOMPARE(w->isOpen(), false);

    QSignalSpy titleSpy(w, &AntPopover::titleChanged);
    w->setTitle("Popover Title");
    QCOMPARE(w->title(), "Popover Title");
    QCOMPARE(titleSpy.count(), 1);

    w->setTitleIconType(Ant::IconType::ExclamationCircle);
    QCOMPARE(w->titleIconType(), Ant::IconType::ExclamationCircle);

    QSignalSpy contentSpy(w, &AntPopover::contentChanged);
    w->setContent("Popover content");
    QCOMPARE(w->content(), "Popover content");
    QCOMPARE(contentSpy.count(), 1);

    QSignalSpy placeSpy(w, &AntPopover::placementChanged);
    w->setPlacement(Ant::TooltipPlacement::Bottom);
    QCOMPARE(w->placement(), Ant::TooltipPlacement::Bottom);
    QCOMPARE(w->renderPlacement(), Ant::TooltipPlacement::Bottom);
    QCOMPARE(placeSpy.count(), 1);

    QSignalSpy trigSpy(w, &AntPopover::triggerChanged);
    w->setTrigger(Ant::PopoverTrigger::Click);
    QCOMPARE(w->trigger(), Ant::PopoverTrigger::Click);
    QCOMPARE(trigSpy.count(), 1);

    QSignalSpy arrowSpy(w, &AntPopover::arrowVisibleChanged);
    w->setArrowVisible(false);
    QCOMPARE(w->arrowVisible(), false);
    QCOMPARE(arrowSpy.count(), 1);

    QSignalSpy openSpy(w, &AntPopover::openChanged);
    w->setOpen(true);
    QCOMPARE(w->isOpen(), true);
    QCOMPARE(openSpy.count(), 1);
}

void TestAntFeedback::popoverCachesLayoutAndSkipsPlacementWork()
{
    QWidget host;
    host.resize(520, 320);

    QWidget target(&host);
    target.resize(120, 32);
    target.move(180, 92);

    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    AntPopover popover;
    popover.setTarget(&target);
    popover.setTrigger(Ant::PopoverTrigger::Click);
    popover.setPlacement(Ant::TooltipPlacement::Bottom);
    popover.setTitle(QStringLiteral("Cached popover"));
    popover.setTitleIconType(Ant::IconType::InfoCircle);
    popover.setContent(QStringLiteral("Repeated paints and target events should reuse popup geometry."));

    auto* action = new QWidget;
    action->setFixedSize(92, 28);
    popover.setActionWidget(action);
    popover.setOpen(true);
    QTRY_VERIFY_WITH_TIMEOUT(popover.isVisible(), 300);

    popover.grab();
    const int layoutBuilds = popover.property("antPopoverLayoutBuildCount").toInt();
    const int layoutHits = popover.property("antPopoverLayoutCacheHitCount").toInt();
    QVERIFY(layoutBuilds > 0);

    popover.grab();
    QCOMPARE(popover.property("antPopoverLayoutBuildCount").toInt(), layoutBuilds);
    QVERIFY(popover.property("antPopoverLayoutCacheHitCount").toInt() > layoutHits);

    const int resolveSkips = popover.property("antPopoverPositionResolveSkipCount").toInt();
    const int sizeSkips = popover.property("antPopoverSizeSkipCount").toInt();
    QMoveEvent moveEvent(target.pos(), target.pos());
    QCoreApplication::sendEvent(&target, &moveEvent);
    QVERIFY(popover.property("antPopoverPositionResolveSkipCount").toInt() > resolveSkips);
    QVERIFY(popover.property("antPopoverSizeSkipCount").toInt() > sizeSkips);
    const int positionSkips = popover.property("antPopoverPositionSkipCount").toInt();
    QMoveEvent repeatedMoveEvent(target.pos(), target.pos());
    QCoreApplication::sendEvent(&target, &repeatedMoveEvent);
    QVERIFY(popover.property("antPopoverPositionSkipCount").toInt() > positionSkips);

    const int buildsBeforeContentChange = popover.property("antPopoverLayoutBuildCount").toInt();
    popover.setContent(QStringLiteral("Changing content invalidates the cached layout exactly through the content path."));
    QCOMPARE(popover.property("antPopoverLastUpdateMode").toString(), QStringLiteral("content"));
    popover.grab();
    QVERIFY(popover.property("antPopoverLayoutBuildCount").toInt() > buildsBeforeContentChange);

    popover.setOpen(false);
}

void TestAntFeedback::progress()
{
    auto* w = new AntProgress;
    QCOMPARE(w->percent(), 0);
    QCOMPARE(w->minimum(), 0);
    QCOMPARE(w->maximum(), 100);
    QCOMPARE(w->value(), 0);
    QCOMPARE(w->textVisible(), true);
    QCOMPARE(w->progressType(), Ant::ProgressType::Line);
    QCOMPARE(w->status(), Ant::ProgressStatus::Normal);
    QCOMPARE(w->showInfo(), true);
    QCOMPARE(w->strokeWidth(), 8);
    QCOMPARE(w->circleSize(), 120);

    QSignalSpy pctSpy(w, &AntProgress::percentChanged);
    w->setPercent(50);
    QCOMPARE(w->percent(), 50);
    QCOMPARE(w->value(), 50);
    QCOMPARE(pctSpy.count(), 1);

    w->setRange(10, 110);
    QCOMPARE(w->minimum(), 10);
    QCOMPARE(w->maximum(), 110);

    QSignalSpy valueSpy(w, &AntProgress::valueChanged);
    w->setValue(60);
    QCOMPARE(w->value(), 60);
    QCOMPARE(w->percent(), 50);
    QCOMPARE(valueSpy.count(), 1);

    QSignalSpy typeSpy(w, &AntProgress::progressTypeChanged);
    w->setProgressType(Ant::ProgressType::Circle);
    QCOMPARE(w->progressType(), Ant::ProgressType::Circle);
    QCOMPARE(typeSpy.count(), 1);

    QSignalSpy statusSpy(w, &AntProgress::statusChanged);
    w->setStatus(Ant::ProgressStatus::Success);
    QCOMPARE(w->status(), Ant::ProgressStatus::Success);
    QCOMPARE(statusSpy.count(), 1);

    QSignalSpy infoSpy(w, &AntProgress::showInfoChanged);
    w->setTextVisible(false);
    QCOMPARE(w->showInfo(), false);
    QCOMPARE(w->textVisible(), false);
    QCOMPARE(infoSpy.count(), 1);

    w->reset();
    QCOMPARE(w->value(), 10);
    QCOMPARE(w->percent(), 0);

    QSignalSpy strokeSpy(w, &AntProgress::strokeWidthChanged);
    w->setStrokeWidth(12);
    QCOMPARE(w->strokeWidth(), 12);
    QCOMPARE(strokeSpy.count(), 1);

    QSignalSpy circleSpy(w, &AntProgress::circleSizeChanged);
    w->setCircleSize(160);
    QCOMPARE(w->circleSize(), 160);
    QCOMPARE(circleSpy.count(), 1);
}

void TestAntFeedback::progressCachesGeometryAndScopesUpdates()
{
    AntProgress progress;
    progress.resize(320, 40);
    progress.show();
    QVERIFY(QTest::qWaitForWindowExposed(&progress));

    progress.grab();
    const int layoutBuilds = progress.property("antProgressLayoutBuildCount").toInt();
    const int layoutHits = progress.property("antProgressLayoutCacheHitCount").toInt();
    QVERIFY(layoutBuilds > 0);

    progress.grab();
    QCOMPARE(progress.property("antProgressLayoutBuildCount").toInt(), layoutBuilds);
    QVERIFY(progress.property("antProgressLayoutCacheHitCount").toInt() > layoutHits);

    const int valueUpdates = progress.property("antProgressValueRegionUpdateCount").toInt();
    progress.setPercent(42);
    QCOMPARE(progress.property("antProgressLastUpdateMode").toString(), QStringLiteral("value"));
    QVERIFY(progress.property("antProgressValueRegionUpdateCount").toInt() > valueUpdates);

    progress.setStatus(Ant::ProgressStatus::Active);
    const int activeUpdates = progress.property("antProgressActiveRegionUpdateCount").toInt();
    QTRY_VERIFY_WITH_TIMEOUT(progress.property("antProgressActiveRegionUpdateCount").toInt() > activeUpdates, 220);
    QCOMPARE(progress.property("antProgressLastUpdateMode").toString(), QStringLiteral("active"));

    progress.hide();
    const int hiddenActiveUpdates = progress.property("antProgressActiveRegionUpdateCount").toInt();
    QTest::qWait(120);
    QCOMPARE(progress.property("antProgressActiveRegionUpdateCount").toInt(), hiddenActiveUpdates);

    progress.setStatus(Ant::ProgressStatus::Normal);
    progress.setProgressType(Ant::ProgressType::Circle);
    progress.resize(progress.sizeHint());
    progress.show();
    QVERIFY(QTest::qWaitForWindowExposed(&progress));
    progress.grab();
    const int circleValueUpdates = progress.property("antProgressValueRegionUpdateCount").toInt();
    progress.setPercent(68);
    QCOMPARE(progress.property("antProgressLastUpdateMode").toString(), QStringLiteral("value"));
    QVERIFY(progress.property("antProgressValueRegionUpdateCount").toInt() > circleValueUpdates);
}

void TestAntFeedback::result()
{
    auto* w = new AntResult;
    QCOMPARE(w->status(), Ant::AlertType::Info);
    QCOMPARE(w->title(), QString());
    QCOMPARE(w->subTitle(), QString());
    QCOMPARE(w->isIconVisible(), true);

    QSignalSpy statusSpy(w, &AntResult::statusChanged);
    w->setStatus(Ant::AlertType::Success);
    QCOMPARE(w->status(), Ant::AlertType::Success);
    QCOMPARE(statusSpy.count(), 1);

    QSignalSpy titleSpy(w, &AntResult::titleChanged);
    w->setTitle("Success");
    QCOMPARE(w->title(), "Success");
    QCOMPARE(titleSpy.count(), 1);

    QSignalSpy subSpy(w, &AntResult::subTitleChanged);
    w->setSubTitle("Operation completed");
    QCOMPARE(w->subTitle(), "Operation completed");
    QCOMPARE(subSpy.count(), 1);

    QSignalSpy iconSpy(w, &AntResult::iconVisibleChanged);
    w->setIconVisible(false);
    QCOMPARE(w->isIconVisible(), false);
    QCOMPARE(iconSpy.count(), 1);

    auto* w2 = new AntResult("Title");
    QCOMPARE(w2->title(), "Title");
}

void TestAntFeedback::resultCachesLayoutIconAndExtraGeometry()
{
    AntResult result;
    result.setStatus(Ant::AlertType::Success);
    result.setTitle(QStringLiteral("Cached result"));
    result.setSubTitle(QStringLiteral("Repeated paints should reuse result text layout and the status icon pixmap."));
    result.resize(360, 220);
    result.show();
    QVERIFY(QTest::qWaitForWindowExposed(&result));

    result.grab();
    const int layoutBuilds = result.property("antResultLayoutBuildCount").toInt();
    const int layoutHits = result.property("antResultLayoutCacheHitCount").toInt();
    const int iconBuilds = result.property("antResultIconPixmapBuildCount").toInt();
    const int iconHits = result.property("antResultIconPixmapCacheHitCount").toInt();
    QVERIFY(layoutBuilds > 0);
    QVERIFY(iconBuilds > 0);

    result.grab();
    QCOMPARE(result.property("antResultLayoutBuildCount").toInt(), layoutBuilds);
    QCOMPARE(result.property("antResultIconPixmapBuildCount").toInt(), iconBuilds);
    QVERIFY(result.property("antResultLayoutCacheHitCount").toInt() > layoutHits);
    QVERIFY(result.property("antResultIconPixmapCacheHitCount").toInt() > iconHits);

    const int iconBuildsBeforeStatus = result.property("antResultIconPixmapBuildCount").toInt();
    result.setStatus(Ant::AlertType::Error);
    QCOMPARE(result.property("antResultLastUpdateMode").toString(), QStringLiteral("status"));
    result.grab();
    QVERIFY(result.property("antResultIconPixmapBuildCount").toInt() > iconBuildsBeforeStatus);

    const int layoutBuildsBeforeTitle = result.property("antResultLayoutBuildCount").toInt();
    result.setTitle(QStringLiteral("Updated cached result title"));
    QCOMPARE(result.property("antResultLastUpdateMode").toString(), QStringLiteral("title"));
    result.grab();
    QVERIFY(result.property("antResultLayoutBuildCount").toInt() > layoutBuildsBeforeTitle);

    auto* extra = new QWidget;
    extra->setFixedSize(96, 32);
    const int geometryApplies = result.property("antResultExtraGeometryApplyCount").toInt();
    result.setExtraWidget(extra);
    QVERIFY(result.property("antResultExtraGeometryApplyCount").toInt() > geometryApplies);

    QVERIFY(!extra->geometry().isEmpty());
}

void TestAntFeedback::skeleton()
{
    auto* w = new AntSkeleton;
    QCOMPARE(w->isActive(), true);
    QCOMPARE(w->isLoading(), true);
    QCOMPARE(w->isRound(), false);
    QCOMPARE(w->avatarVisible(), false);
    QCOMPARE(w->titleVisible(), true);
    QCOMPARE(w->paragraphVisible(), true);
    QCOMPARE(w->paragraphRows(), 3);

    QSignalSpy activeSpy(w, &AntSkeleton::activeChanged);
    w->setActive(false);
    QCOMPARE(w->isActive(), false);
    QCOMPARE(activeSpy.count(), 1);

    QSignalSpy loadSpy(w, &AntSkeleton::loadingChanged);
    w->setLoading(false);
    QCOMPARE(w->isLoading(), false);
    QCOMPARE(loadSpy.count(), 1);

    QSignalSpy roundSpy(w, &AntSkeleton::roundChanged);
    w->setRound(true);
    QCOMPARE(w->isRound(), true);
    QCOMPARE(roundSpy.count(), 1);

    QSignalSpy avatarSpy(w, &AntSkeleton::avatarVisibleChanged);
    w->setAvatarVisible(true);
    QCOMPARE(w->avatarVisible(), true);
    QCOMPARE(avatarSpy.count(), 1);

    QSignalSpy titleSpy(w, &AntSkeleton::titleVisibleChanged);
    w->setTitleVisible(false);
    QCOMPARE(w->titleVisible(), false);
    QCOMPARE(titleSpy.count(), 1);

    QSignalSpy paraSpy(w, &AntSkeleton::paragraphVisibleChanged);
    w->setParagraphVisible(false);
    QCOMPARE(w->paragraphVisible(), false);
    QCOMPARE(paraSpy.count(), 1);

    QSignalSpy rowsSpy(w, &AntSkeleton::paragraphRowsChanged);
    w->setParagraphRows(5);
    QCOMPARE(w->paragraphRows(), 5);
    QCOMPARE(rowsSpy.count(), 1);

    auto* moving = new AntSkeleton;
    moving->resize(320, 80);
    const int firstOffset = moving->shimmerOffset();
    QTest::qWait(90);
    QVERIFY(moving->shimmerOffset() != firstOffset);
    moving->setActive(false);
    const int pausedOffset = moving->shimmerOffset();
    QTest::qWait(90);
    QCOMPARE(moving->shimmerOffset(), pausedOffset);
}

void TestAntFeedback::spin()
{
    auto* w = new AntSpin;
    QCOMPARE(w->isSpinning(), true);
    QCOMPARE(w->spinSize(), Ant::Size::Middle);
    QCOMPARE(w->description(), QString());
    QCOMPARE(w->delay(), 0);

    QSignalSpy spinSpy(w, &AntSpin::spinningChanged);
    w->setSpinning(false);
    QCOMPARE(w->isSpinning(), false);
    QCOMPARE(spinSpy.count(), 1);

    QSignalSpy sizeSpy(w, &AntSpin::spinSizeChanged);
    w->setSpinSize(Ant::Size::Large);
    QCOMPARE(w->spinSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy descSpy(w, &AntSpin::descriptionChanged);
    w->setDescription("Loading...");
    QCOMPARE(w->description(), "Loading...");
    QCOMPARE(descSpy.count(), 1);

    QSignalSpy delaySpy(w, &AntSpin::delayChanged);
    w->setDelay(500);
    QCOMPARE(w->delay(), 500);
    QCOMPARE(delaySpy.count(), 1);

    auto* animated = new AntSpin;
    animated->show();
    const int startAngle = animated->angle();
    QTest::qWait(55);
    const int angleDelta = (animated->angle() - startAngle + 360) % 360;
    QVERIFY(angleDelta > 0);
    QVERIFY(angleDelta <= 25);
    animated->hide();
}

void TestAntFeedback::tooltip()
{
    auto* w = new AntToolTip;
    QCOMPARE(w->title(), QString());
    QCOMPARE(w->placement(), Ant::TooltipPlacement::Top);
    QCOMPARE(w->renderPlacement(), Ant::TooltipPlacement::Top);
    QCOMPARE(w->arrowVisible(), true);
    QCOMPARE(w->openDelay(), 120);

    QSignalSpy titleSpy(w, &AntToolTip::titleChanged);
    w->setTitle("Tooltip text");
    QCOMPARE(w->title(), "Tooltip text");
    QCOMPARE(titleSpy.count(), 1);

    QSignalSpy placeSpy(w, &AntToolTip::placementChanged);
    w->setPlacement(Ant::TooltipPlacement::Bottom);
    QCOMPARE(w->placement(), Ant::TooltipPlacement::Bottom);
    QCOMPARE(w->renderPlacement(), Ant::TooltipPlacement::Bottom);
    QCOMPARE(placeSpy.count(), 1);

    QSignalSpy colorSpy(w, &AntToolTip::colorChanged);
    w->setColor(Qt::red);
    QCOMPARE(w->color(), QColor(Qt::red));
    QCOMPARE(colorSpy.count(), 1);

    QSignalSpy arrowSpy(w, &AntToolTip::arrowVisibleChanged);
    w->setArrowVisible(false);
    QCOMPARE(w->arrowVisible(), false);
    QCOMPARE(arrowSpy.count(), 1);

    QSignalSpy delaySpy(w, &AntToolTip::openDelayChanged);
    w->setOpenDelay(200);
    QCOMPARE(w->openDelay(), 200);
    QCOMPARE(delaySpy.count(), 1);
}

void TestAntFeedback::tour()
{
    QWidget host;
    host.resize(360, 180);
    AntButton step1(QStringLiteral("Step 1"), &host);
    AntButton step2(QStringLiteral("Step 2"), &host);
    AntButton step3(QStringLiteral("Step 3"), &host);
    step1.move(24, 48);
    step2.move(124, 48);
    step3.move(224, 48);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    auto* w = new AntTour(&host);
    w->addStep({&step1, QStringLiteral("Step 1"), QStringLiteral("First step")});
    w->addStep({&step2, QStringLiteral("Step 2"), QStringLiteral("Second step")});
    w->addStep({&step3, QStringLiteral("Step 3"), QStringLiteral("Third step")});

    QSignalSpy finishedSpy(w, &AntTour::finished);
    QSignalSpy stepSpy(w, &AntTour::stepChanged);

    w->start(1);
    QCOMPARE(stepSpy.count(), 1);
    QCOMPARE(stepSpy.takeFirst().at(0).toInt(), 1);

    w->next();
    QCOMPARE(stepSpy.count(), 1);
    QCOMPARE(stepSpy.takeFirst().at(0).toInt(), 2);

    w->prev();
    QCOMPARE(stepSpy.count(), 1);
    QCOMPARE(stepSpy.takeFirst().at(0).toInt(), 1);

    w->close();
    QCOMPARE(finishedSpy.count(), 1);
}

QTEST_MAIN(TestAntFeedback)
#include "TestAntFeedback.moc"
