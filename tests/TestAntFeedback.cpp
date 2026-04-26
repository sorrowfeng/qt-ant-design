#include <QSignalSpy>
#include <QTest>
#include "widgets/AntAlert.h"
#include "widgets/AntDrawer.h"
#include "widgets/AntMessage.h"
#include "widgets/AntNotification.h"
#include "widgets/AntPopconfirm.h"
#include "widgets/AntPopover.h"
#include "widgets/AntProgress.h"
#include "widgets/AntResult.h"
#include "widgets/AntSkeleton.h"
#include "widgets/AntSpin.h"
#include "widgets/AntTooltip.h"
#include "widgets/AntTour.h"

class TestAntFeedback : public QObject
{
    Q_OBJECT
private slots:
    void alert();
    void drawer();
    void message();
    void notification();
    void popconfirm();
    void popover();
    void progress();
    void result();
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

void TestAntFeedback::drawer()
{
    auto* w = new AntDrawer;
    QCOMPARE(w->title(), QString());
    QCOMPARE(w->placement(), Ant::DrawerPlacement::Right);
    QCOMPARE(w->drawerWidth(), 300);
    QCOMPARE(w->drawerHeight(), 300);
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

void TestAntFeedback::message()
{
    auto* w = new AntMessage;
    QCOMPARE(w->text(), QString());
    QCOMPARE(w->messageType(), Ant::MessageType::Info);
    QCOMPARE(w->duration(), 3000);
    QCOMPARE(w->pauseOnHover(), true);

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

void TestAntFeedback::notification()
{
    auto* w = new AntNotification;
    QCOMPARE(w->title(), QString());
    QCOMPARE(w->description(), QString());
    QCOMPARE(w->notificationType(), Ant::MessageType::Info);
    QCOMPARE(w->placement(), Ant::NotificationPlacement::TopRight);
    QCOMPARE(w->duration(), 4500);
    QCOMPARE(w->pauseOnHover(), true);
    QCOMPARE(w->showProgress(), false);
    QCOMPARE(w->isClosable(), true);

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

    QSignalSpy placeSpy(w, &AntNotification::placementChanged);
    w->setPlacement(Ant::NotificationPlacement::BottomLeft);
    QCOMPARE(w->placement(), Ant::NotificationPlacement::BottomLeft);
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

    QSignalSpy disSpy(w, &AntPopconfirm::disabledChanged);
    w->setDisabled(true);
    QCOMPARE(w->isDisabled(), true);
    QCOMPARE(disSpy.count(), 1);
}

void TestAntFeedback::popover()
{
    auto* w = new AntPopover;
    QCOMPARE(w->title(), QString());
    QCOMPARE(w->content(), QString());
    QCOMPARE(w->placement(), Ant::TooltipPlacement::Top);
    QCOMPARE(w->trigger(), Ant::PopoverTrigger::Hover);
    QCOMPARE(w->arrowVisible(), true);
    QCOMPARE(w->isOpen(), false);

    QSignalSpy titleSpy(w, &AntPopover::titleChanged);
    w->setTitle("Popover Title");
    QCOMPARE(w->title(), "Popover Title");
    QCOMPARE(titleSpy.count(), 1);

    QSignalSpy contentSpy(w, &AntPopover::contentChanged);
    w->setContent("Popover content");
    QCOMPARE(w->content(), "Popover content");
    QCOMPARE(contentSpy.count(), 1);

    QSignalSpy placeSpy(w, &AntPopover::placementChanged);
    w->setPlacement(Ant::TooltipPlacement::Bottom);
    QCOMPARE(w->placement(), Ant::TooltipPlacement::Bottom);
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

void TestAntFeedback::progress()
{
    auto* w = new AntProgress;
    QCOMPARE(w->percent(), 0);
    QCOMPARE(w->progressType(), Ant::ProgressType::Line);
    QCOMPARE(w->status(), Ant::ProgressStatus::Normal);
    QCOMPARE(w->showInfo(), true);
    QCOMPARE(w->strokeWidth(), 8);
    QCOMPARE(w->circleSize(), 120);

    QSignalSpy pctSpy(w, &AntProgress::percentChanged);
    w->setPercent(50);
    QCOMPARE(w->percent(), 50);
    QCOMPARE(pctSpy.count(), 1);

    QSignalSpy typeSpy(w, &AntProgress::progressTypeChanged);
    w->setProgressType(Ant::ProgressType::Circle);
    QCOMPARE(w->progressType(), Ant::ProgressType::Circle);
    QCOMPARE(typeSpy.count(), 1);

    QSignalSpy statusSpy(w, &AntProgress::statusChanged);
    w->setStatus(Ant::ProgressStatus::Success);
    QCOMPARE(w->status(), Ant::ProgressStatus::Success);
    QCOMPARE(statusSpy.count(), 1);

    QSignalSpy infoSpy(w, &AntProgress::showInfoChanged);
    w->setShowInfo(false);
    QCOMPARE(w->showInfo(), false);
    QCOMPARE(infoSpy.count(), 1);

    QSignalSpy strokeSpy(w, &AntProgress::strokeWidthChanged);
    w->setStrokeWidth(12);
    QCOMPARE(w->strokeWidth(), 12);
    QCOMPARE(strokeSpy.count(), 1);

    QSignalSpy circleSpy(w, &AntProgress::circleSizeChanged);
    w->setCircleSize(160);
    QCOMPARE(w->circleSize(), 160);
    QCOMPARE(circleSpy.count(), 1);
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
}

void TestAntFeedback::spin()
{
    auto* w = new AntSpin;
    QCOMPARE(w->isSpinning(), true);
    QCOMPARE(w->spinSize(), Ant::SpinSize::Middle);
    QCOMPARE(w->description(), QString());
    QCOMPARE(w->delay(), 0);

    QSignalSpy spinSpy(w, &AntSpin::spinningChanged);
    w->setSpinning(false);
    QCOMPARE(w->isSpinning(), false);
    QCOMPARE(spinSpy.count(), 1);

    QSignalSpy sizeSpy(w, &AntSpin::spinSizeChanged);
    w->setSpinSize(Ant::SpinSize::Large);
    QCOMPARE(w->spinSize(), Ant::SpinSize::Large);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy descSpy(w, &AntSpin::descriptionChanged);
    w->setDescription("Loading...");
    QCOMPARE(w->description(), "Loading...");
    QCOMPARE(descSpy.count(), 1);

    QSignalSpy delaySpy(w, &AntSpin::delayChanged);
    w->setDelay(500);
    QCOMPARE(w->delay(), 500);
    QCOMPARE(delaySpy.count(), 1);
}

void TestAntFeedback::tooltip()
{
    auto* w = new AntTooltip;
    QCOMPARE(w->title(), QString());
    QCOMPARE(w->placement(), Ant::TooltipPlacement::Top);
    QCOMPARE(w->arrowVisible(), true);
    QCOMPARE(w->openDelay(), 120);

    QSignalSpy titleSpy(w, &AntTooltip::titleChanged);
    w->setTitle("Tooltip text");
    QCOMPARE(w->title(), "Tooltip text");
    QCOMPARE(titleSpy.count(), 1);

    QSignalSpy placeSpy(w, &AntTooltip::placementChanged);
    w->setPlacement(Ant::TooltipPlacement::Bottom);
    QCOMPARE(w->placement(), Ant::TooltipPlacement::Bottom);
    QCOMPARE(placeSpy.count(), 1);

    QSignalSpy colorSpy(w, &AntTooltip::colorChanged);
    w->setColor(Qt::red);
    QCOMPARE(w->color(), QColor(Qt::red));
    QCOMPARE(colorSpy.count(), 1);

    QSignalSpy arrowSpy(w, &AntTooltip::arrowVisibleChanged);
    w->setArrowVisible(false);
    QCOMPARE(w->arrowVisible(), false);
    QCOMPARE(arrowSpy.count(), 1);

    QSignalSpy delaySpy(w, &AntTooltip::openDelayChanged);
    w->setOpenDelay(200);
    QCOMPARE(w->openDelay(), 200);
    QCOMPARE(delaySpy.count(), 1);
}

void TestAntFeedback::tour()
{
    auto* w = new AntTour;
    QSignalSpy finishedSpy(w, &AntTour::finished);
    QSignalSpy stepSpy(w, &AntTour::stepChanged);
    Q_UNUSED(finishedSpy);
    Q_UNUSED(stepSpy);
    // Tour is step-based, just verify it instantiates
}

QTEST_MAIN(TestAntFeedback)
#include "TestAntFeedback.moc"
