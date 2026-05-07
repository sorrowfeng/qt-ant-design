#include <QCoreApplication>
#include <QEvent>
#include <QPointer>
#include <QTest>
#include <QWidget>

#include "widgets/AntAffix.h"
#include "widgets/AntAlert.h"
#include "widgets/AntAnchor.h"
#include "widgets/AntApp.h"
#include "widgets/AntAutoComplete.h"
#include "widgets/AntAvatar.h"
#include "widgets/AntBadge.h"
#include "widgets/AntBreadcrumb.h"
#include "widgets/AntButton.h"
#include "widgets/AntCalendar.h"
#include "widgets/AntCard.h"
#include "widgets/AntCarousel.h"
#include "widgets/AntCascader.h"
#include "widgets/AntCheckBox.h"
#include "widgets/AntCollapse.h"
#include "widgets/AntColorPicker.h"
#include "widgets/AntConfigProvider.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntDescriptions.h"
#include "widgets/AntDivider.h"
#include "widgets/AntDockWidget.h"
#include "widgets/AntDrawer.h"
#include "widgets/AntDropdown.h"
#include "widgets/AntEmpty.h"
#include "widgets/AntFlex.h"
#include "widgets/AntFloatButton.h"
#include "widgets/AntForm.h"
#include "widgets/AntGrid.h"
#include "widgets/AntIcon.h"
#include "widgets/AntImage.h"
#include "widgets/AntInput.h"
#include "widgets/AntInputNumber.h"
#include "widgets/AntLayout.h"
#include "widgets/AntList.h"
#include "widgets/AntLog.h"
#include "widgets/AntMasonry.h"
#include "widgets/AntMentions.h"
#include "widgets/AntMenu.h"
#include "widgets/AntMenuBar.h"
#include "widgets/AntMessage.h"
#include "widgets/AntModal.h"
#include "widgets/AntNavItem.h"
#include "widgets/AntNotification.h"
#include "widgets/AntPagination.h"
#include "widgets/AntPlainTextEdit.h"
#include "widgets/AntPopconfirm.h"
#include "widgets/AntPopover.h"
#include "widgets/AntProgress.h"
#include "widgets/AntQRCode.h"
#include "widgets/AntRadio.h"
#include "widgets/AntRate.h"
#include "widgets/AntResult.h"
#include "widgets/AntScrollArea.h"
#include "widgets/AntScrollBar.h"
#include "widgets/AntSegmented.h"
#include "widgets/AntSelect.h"
#include "widgets/AntSkeleton.h"
#include "widgets/AntSlider.h"
#include "widgets/AntSpace.h"
#include "widgets/AntSpin.h"
#include "widgets/AntSplitter.h"
#include "widgets/AntStatistic.h"
#include "widgets/AntStatusBar.h"
#include "widgets/AntSteps.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntTable.h"
#include "widgets/AntTabs.h"
#include "widgets/AntTag.h"
#include "widgets/AntTimeline.h"
#include "widgets/AntTimePicker.h"
#include "widgets/AntToolBar.h"
#include "widgets/AntToolButton.h"
#include "widgets/AntToolTip.h"
#include "widgets/AntTour.h"
#include "widgets/AntTransfer.h"
#include "widgets/AntTree.h"
#include "widgets/AntTreeSelect.h"
#include "widgets/AntTypography.h"
#include "widgets/AntUpload.h"
#include "widgets/AntWatermark.h"
#include "widgets/AntWidget.h"
#include "widgets/AntWindow.h"

class TestAntObjectTree : public QObject
{
    Q_OBJECT

private slots:
    void parentOwnsWidgetsAndStyles();
};

void TestAntObjectTree::parentOwnsWidgetsAndStyles()
{
    auto* root = new QWidget;
    QList<QPointer<QObject>> tracked;

    auto trackObject = [&](QObject* object) {
        QVERIFY(object != nullptr);
        QCOMPARE(object->parent(), root);
        tracked.append(QPointer<QObject>(object));
    };

    auto trackWidget = [&](QWidget* widget, bool hasAntStyle) {
        QVERIFY(widget != nullptr);
        QCOMPARE(widget->parentWidget(), root);
        tracked.append(QPointer<QObject>(widget));
        if (hasAntStyle)
        {
            QVERIFY(widget->style() != nullptr);
            QCOMPARE(widget->style()->parent(), widget);
        }
    };

    trackObject(new AntAffix(root));
    trackObject(new AntApp(root, root));
    trackObject(new AntConfigProvider(root));
    trackObject(new AntTour(root));

    trackWidget(new AntAlert(root), true);
    trackWidget(new AntAnchor(root), false);
    trackWidget(new AntAutoComplete(root), true);
    trackWidget(new AntAvatarGroup(root), false);
    trackWidget(new AntAvatar(root), true);
    trackWidget(new AntBadge(root), true);
    trackWidget(new AntBreadcrumb(root), true);
    trackWidget(new AntButton(root), true);
    trackWidget(new AntCalendar(root), true);
    trackWidget(new AntCard(root), true);
    trackWidget(new AntCarousel(root), false);
    trackWidget(new AntCascader(root), true);
    trackWidget(new AntCheckBox(root), true);
    trackWidget(new AntCollapsePanel(QStringLiteral("Panel"), root), false);
    trackWidget(new AntCollapse(root), false);
    trackWidget(new AntColorPicker(root), false);
    trackWidget(new AntDatePicker(root), true);
    trackWidget(new AntDescriptionsItem(root), false);
    trackWidget(new AntDescriptions(root), true);
    trackWidget(new AntDivider(root), true);
    trackWidget(new AntDockWidget(root), false);
    trackWidget(new AntDrawer(root), true);
    trackWidget(new AntDropdown(root), true);
    trackWidget(new AntEmpty(root), true);
    trackWidget(new AntFlex(root), false);
    trackWidget(new AntFloatButton(root), true);
    trackWidget(new AntFormItem(root), false);
    trackWidget(new AntFormProvider(root), false);
    trackWidget(new AntForm(root), true);
    trackWidget(new AntFormList(root), false);
    trackWidget(new AntCol(12, root), false);
    trackWidget(new AntRow(root), false);
    trackWidget(new AntIcon(root), true);
    trackWidget(new AntImage(root), false);
    trackWidget(new AntInput(root), true);
    trackWidget(new AntInputNumber(root), true);
    trackWidget(new AntLayoutHeader(root), false);
    trackWidget(new AntLayoutFooter(root), false);
    trackWidget(new AntLayoutContent(root), false);
    trackWidget(new AntLayoutSider(root), false);
    trackWidget(new AntLayout(root), true);
    trackWidget(new AntListItemMeta(root), false);
    trackWidget(new AntListItem(root), false);
    trackWidget(new AntList(root), true);
    trackWidget(new AntLog(root), false);
    trackWidget(new AntMasonry(root), false);
    trackWidget(new AntMentions(root), true);
    trackWidget(new AntMenu(root), true);
    trackWidget(new AntMenuBar(root), true);
    trackWidget(new AntMessage(root), true);
    trackWidget(new AntModal(root), true);
    trackWidget(new AntNavItem(QStringLiteral("Nav"), root), false);
    trackWidget(new AntNotification(root), true);
    trackWidget(new AntPagination(root), true);
    trackWidget(new AntPlainTextEdit(root), true);
    trackWidget(new AntPopconfirm(root), true);
    trackWidget(new AntPopover(root), true);
    trackWidget(new AntProgress(root), true);
    trackWidget(new AntQRCode(root), true);
    trackWidget(new AntRadio(root), true);
    trackWidget(new AntRate(root), true);
    trackWidget(new AntResult(root), true);
    trackWidget(new AntScrollArea(root), false);
    trackWidget(new AntScrollBar(root), true);
    trackWidget(new AntSegmented(root), true);
    trackWidget(new AntSelect(root), true);
    trackWidget(new AntSkeleton(root), true);
    trackWidget(new AntSlider(root), true);
    trackWidget(new AntSpace(root), true);
    trackWidget(new AntSpin(root), true);
    trackWidget(new AntSplitter(root), false);
    trackWidget(new AntStatistic(root), true);
    trackWidget(new AntStatusBar(root), true);
    trackWidget(new AntSteps(root), true);
    trackWidget(new AntSwitch(root), true);
    trackWidget(new AntTable(root), true);
    trackWidget(new AntTabs(root), true);
    trackWidget(new AntTag(root), true);
    trackWidget(new AntTimeline(root), true);
    trackWidget(new AntTimePicker(root), true);
    trackWidget(new AntToolBar(root), true);
    trackWidget(new AntToolButton(root), true);
    trackWidget(new AntToolTip(root), true);
    trackWidget(new AntTransfer(root), false);
    trackWidget(new AntTree(root), true);
    trackWidget(new AntTreeSelect(root), true);
    trackWidget(new AntTypography(root), true);
    trackWidget(new AntUpload(root), true);
    trackWidget(new AntWatermark(root), true);
    trackWidget(new AntWidget(root), false);
    trackWidget(new AntWindow(root), true);

    delete root;
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    for (const auto& object : std::as_const(tracked))
    {
        QVERIFY(object.isNull());
    }
}

QTEST_MAIN(TestAntObjectTree)
#include "TestAntObjectTree.moc"
