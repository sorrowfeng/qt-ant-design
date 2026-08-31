#pragma once

// 权威控件清单（single source of truth）。
//
// 全库公开控件在此登记一次，供下列表驱动测试共享，避免"新增控件要改 N 处"：
//   - TestAntMetaProperties  （元属性读写覆盖）
//   - TestAntThemeLifecycle  （主题切换存活覆盖）
//   - TestAntObjectTree      （父子归属覆盖）
//   - TestAntRenderSmoke     （渲染非空覆盖，需额外的 configure/size 语义）
//
// 新增一个公开控件时，只需在本清单追加一项（并在对应测试的语义覆盖层补齐
// 该控件特有的 configure/豁免标记）。

#include <QLineEdit>
#include <QList>
#include <QWidget>

#include <functional>

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
#include "widgets/AntDialog.h"
#include "widgets/AntBorderBeam.h"
#include "widgets/AntDivider.h"
#include "widgets/AntDockManager.h"
#include "widgets/AntDockWidget.h"
#include "widgets/AntDrawer.h"
#include "widgets/AntDropdown.h"
#include "widgets/AntEmpty.h"
#include "widgets/AntFlex.h"
#include "widgets/AntFloatButton.h"
#include "widgets/AntFileDialog.h"
#include "widgets/AntForm.h"
#include "widgets/AntGrid.h"
#include "widgets/AntIcon.h"
#include "widgets/AntImage.h"
#include "widgets/AntInput.h"
#include "widgets/AntInputDialog.h"
#include "widgets/AntInputNumber.h"
#include "widgets/AntLayout.h"
#include "widgets/AntList.h"
#include "widgets/AntListy.h"
#include "widgets/AntLog.h"
#include "widgets/AntMasonry.h"
#include "widgets/AntMentions.h"
#include "widgets/AntMenu.h"
#include "widgets/AntMenuBar.h"
#include "widgets/AntMessage.h"
#include "widgets/AntModal.h"
#include "widgets/AntNav.h"
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
#include "widgets/AntStackedWidget.h"
#include "widgets/AntStatistic.h"
#include "widgets/AntStatusBar.h"
#include "widgets/AntRibbon.h"
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
#include "widgets/AntWindowFrame.h"

namespace AntTestUtils
{

// 一个公开控件的默认构造工厂。name 是控件名（用于断言消息），
// create 返回一个以 parent 为父对象的 QObject（绝大多数是 QWidget）。
struct WidgetFactoryCase
{
    const char* name;
    std::function<QObject*(QWidget*)> create;
};

// 权威清单：所有公开控件 + 其默认构造。
inline QList<WidgetFactoryCase> allWidgetFactoryCases()
{
    return {
            {"AntAffix", [](QWidget* parent) { return new AntAffix(parent); }},
            {"AntAlert", [](QWidget* parent) { return new AntAlert(parent); }},
            {"AntAnchor", [](QWidget* parent) { return new AntAnchor(parent); }},
            {"AntApp", [](QWidget* parent) { return new AntApp(parent, parent); }},
            {"AntAutoComplete", [](QWidget* parent) { return new AntAutoComplete(parent); }},
            {"AntAvatarGroup", [](QWidget* parent) { return new AntAvatarGroup(parent); }},
            {"AntAvatar", [](QWidget* parent) { return new AntAvatar(parent); }},
            {"AntBadge", [](QWidget* parent) { return new AntBadge(parent); }},
            {"AntBreadcrumb", [](QWidget* parent) { return new AntBreadcrumb(parent); }},
            {"AntButton", [](QWidget* parent) { return new AntButton(parent); }},
            {"AntCalendar", [](QWidget* parent) { return new AntCalendar(parent); }},
            {"AntCard", [](QWidget* parent) { return new AntCard(parent); }},
            {"AntCarousel", [](QWidget* parent) { return new AntCarousel(parent); }},
            {"AntCascader", [](QWidget* parent) { return new AntCascader(parent); }},
            {"AntCheckBox", [](QWidget* parent) { return new AntCheckBox(parent); }},
            {"AntCollapsePanel", [](QWidget* parent) { return new AntCollapsePanel(QStringLiteral("Panel"), parent); }},
            {"AntCollapse", [](QWidget* parent) { return new AntCollapse(parent); }},
            {"AntColorPicker", [](QWidget* parent) { return new AntColorPicker(parent); }},
            {"AntConfigProvider", [](QWidget* parent) { return new AntConfigProvider(parent); }},
            {"AntDatePicker", [](QWidget* parent) { return new AntDatePicker(parent); }},
            {"AntDescriptionsItem", [](QWidget* parent) { return new AntDescriptionsItem(parent); }},
            {"AntDescriptions", [](QWidget* parent) { return new AntDescriptions(parent); }},
            {"AntDialog", [](QWidget* parent) { return new AntDialog(parent); }},
                    {"AntDivider", [](QWidget* parent) { return new AntDivider(parent); }},
            {"AntBorderBeam", [](QWidget* parent) { return new AntBorderBeam(parent); }},
            {"AntDockManager", [](QWidget* parent) { return new AntDockManager(parent); }},
            {"AntDockWidget", [](QWidget* parent) { return new AntDockWidget(parent); }},
            {"AntDrawer", [](QWidget* parent) { return new AntDrawer(parent); }},
            {"AntDropdown", [](QWidget* parent) { return new AntDropdown(parent); }},
            {"AntEmpty", [](QWidget* parent) { return new AntEmpty(parent); }},
            {"AntFlex", [](QWidget* parent) { return new AntFlex(parent); }},
            {"AntFloatButton", [](QWidget* parent) { return new AntFloatButton(parent); }},
            {"AntFileDialog", [](QWidget* parent) { return new AntFileDialog(parent); }},
            {"AntFormItem", [](QWidget* parent) {
                 auto* item = new AntFormItem(parent);
                 item->setFieldWidget(new QLineEdit);
                 return item;
             }},
            {"AntFormProvider", [](QWidget* parent) { return new AntFormProvider(parent); }},
            {"AntForm", [](QWidget* parent) { return new AntForm(parent); }},
            {"AntFormList", [](QWidget* parent) { return new AntFormList(parent); }},
            {"AntCol", [](QWidget* parent) { return new AntCol(12, parent); }},
            {"AntRow", [](QWidget* parent) { return new AntRow(parent); }},
            {"AntIcon", [](QWidget* parent) { return new AntIcon(parent); }},
            {"AntImage", [](QWidget* parent) { return new AntImage(parent); }},
            {"AntInput", [](QWidget* parent) { return new AntInput(parent); }},
            {"AntInputDialog", [](QWidget* parent) { return new AntInputDialog(parent); }},
            {"AntInputNumber", [](QWidget* parent) { return new AntInputNumber(parent); }},
            {"AntLayoutHeader", [](QWidget* parent) { return new AntLayoutHeader(parent); }},
            {"AntLayoutFooter", [](QWidget* parent) { return new AntLayoutFooter(parent); }},
            {"AntLayoutContent", [](QWidget* parent) { return new AntLayoutContent(parent); }},
            {"AntLayoutSider", [](QWidget* parent) { return new AntLayoutSider(parent); }},
            {"AntLayout", [](QWidget* parent) { return new AntLayout(parent); }},
            {"AntListItemMeta", [](QWidget* parent) { return new AntListItemMeta(parent); }},
            {"AntListItem", [](QWidget* parent) { return new AntListItem(parent); }},
            {"AntList", [](QWidget* parent) { return new AntList(parent); }},
            {"AntListy", [](QWidget* parent) { return new AntListy(parent); }},
            {"AntLog", [](QWidget* parent) { return new AntLog(parent); }},
            {"AntMasonry", [](QWidget* parent) { return new AntMasonry(parent); }},
            {"AntMentions", [](QWidget* parent) { return new AntMentions(parent); }},
            {"AntMenu", [](QWidget* parent) { return new AntMenu(parent); }},
            {"AntMenuBar", [](QWidget* parent) { return new AntMenuBar(parent); }},
            {"AntMessage", [](QWidget* parent) { return new AntMessage(parent); }},
            {"AntModal", [](QWidget* parent) { return new AntModal(parent); }},
            {"AntNav", [](QWidget* parent) { return new AntNav(parent); }},
            {"AntNavItem", [](QWidget* parent) { return new AntNavItem(QStringLiteral("Nav"), parent); }},
            {"AntNotification", [](QWidget* parent) { return new AntNotification(parent); }},
            {"AntPagination", [](QWidget* parent) { return new AntPagination(parent); }},
            {"AntPlainTextEdit", [](QWidget* parent) { return new AntPlainTextEdit(parent); }},
            {"AntPopconfirm", [](QWidget* parent) { return new AntPopconfirm(parent); }},
            {"AntPopover", [](QWidget* parent) { return new AntPopover(parent); }},
            {"AntProgress", [](QWidget* parent) { return new AntProgress(parent); }},
            {"AntQRCode", [](QWidget* parent) { return new AntQRCode(parent); }},
            {"AntRadio", [](QWidget* parent) { return new AntRadio(parent); }},
            {"AntRate", [](QWidget* parent) { return new AntRate(parent); }},
            {"AntResult", [](QWidget* parent) { return new AntResult(parent); }},
            {"AntScrollArea", [](QWidget* parent) { return new AntScrollArea(parent); }},
            {"AntScrollBar", [](QWidget* parent) { return new AntScrollBar(parent); }},
            {"AntSegmented", [](QWidget* parent) { return new AntSegmented(parent); }},
            {"AntSelect", [](QWidget* parent) {
                 auto* select = new AntSelect(parent);
                 select->addOption(QStringLiteral("Meta Value"), QStringLiteral("meta-value"));
                 return select;
             }},
            {"AntSkeleton", [](QWidget* parent) { return new AntSkeleton(parent); }},
            {"AntSlider", [](QWidget* parent) { return new AntSlider(parent); }},
            {"AntSpace", [](QWidget* parent) { return new AntSpace(parent); }},
            {"AntSpin", [](QWidget* parent) { return new AntSpin(parent); }},
            {"AntSplitter", [](QWidget* parent) { return new AntSplitter(parent); }},
            {"AntStackedWidget", [](QWidget* parent) { return new AntStackedWidget(parent); }},
            {"AntStatistic", [](QWidget* parent) { return new AntStatistic(parent); }},
            {"AntStatusBar", [](QWidget* parent) { return new AntStatusBar(parent); }},
            {"AntRibbon", [](QWidget* parent) { return new AntRibbon(parent); }},
            {"AntRibbonPage", [](QWidget* parent) { return new AntRibbonPage(QStringLiteral("Page"), QStringLiteral("page"), parent); }},
            {"AntRibbonGroup", [](QWidget* parent) { return new AntRibbonGroup(QStringLiteral("Group"), parent); }},
            {"AntSteps", [](QWidget* parent) { return new AntSteps(parent); }},
            {"AntSwitch", [](QWidget* parent) { return new AntSwitch(parent); }},
            {"AntTable", [](QWidget* parent) { return new AntTable(parent); }},
            {"AntTabs", [](QWidget* parent) { return new AntTabs(parent); }},
            {"AntTag", [](QWidget* parent) { return new AntTag(parent); }},
            {"AntTimeline", [](QWidget* parent) { return new AntTimeline(parent); }},
            {"AntTimePicker", [](QWidget* parent) { return new AntTimePicker(parent); }},
            {"AntToolBar", [](QWidget* parent) { return new AntToolBar(parent); }},
            {"AntToolButton", [](QWidget* parent) { return new AntToolButton(parent); }},
            {"AntToolTip", [](QWidget* parent) { return new AntToolTip(parent); }},
            {"AntTour", [](QWidget* parent) { return new AntTour(parent); }},
            {"AntTransfer", [](QWidget* parent) { return new AntTransfer(parent); }},
            {"AntTree", [](QWidget* parent) { return new AntTree(parent); }},
            {"AntTreeSelect", [](QWidget* parent) { return new AntTreeSelect(parent); }},
            {"AntTypography", [](QWidget* parent) { return new AntTypography(parent); }},
            {"AntUpload", [](QWidget* parent) { return new AntUpload(parent); }},
            {"AntWatermark", [](QWidget* parent) { return new AntWatermark(parent); }},
            {"AntWidget", [](QWidget* parent) { return new AntWidget(parent); }},
            {"AntWindow", [](QWidget* parent) { return new AntWindow(parent); }},
        };
}

} // namespace AntTestUtils
