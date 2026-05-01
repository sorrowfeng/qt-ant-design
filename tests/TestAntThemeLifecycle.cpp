#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QFrame>
#include <QPointer>
#include <QPushButton>
#include <QTest>
#include <QWidget>

#include <functional>

#include "core/AntTheme.h"
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
#include "widgets/AntCheckbox.h"
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
#include "widgets/AntTooltip.h"
#include "widgets/AntTour.h"
#include "widgets/AntTransfer.h"
#include "widgets/AntTree.h"
#include "widgets/AntTreeSelect.h"
#include "widgets/AntTypography.h"
#include "widgets/AntUpload.h"
#include "widgets/AntWatermark.h"
#include "widgets/AntWidget.h"
#include "widgets/AntWindow.h"

class TestAntThemeLifecycle : public QObject
{
    Q_OBJECT

private slots:
    void allControlsSurviveThemeSwitchAndDestroyCleanly();
    void openPopupsSurviveThemeSwitch();
};

namespace
{
struct ObjectCase
{
    const char* name;
    std::function<QObject*(QWidget*)> create;
};

class ThemeModeGuard
{
public:
    ThemeModeGuard()
        : m_originalMode(antTheme->themeMode())
    {
    }

    ~ThemeModeGuard()
    {
        antTheme->setThemeMode(m_originalMode);
        QCoreApplication::processEvents();
    }

    Ant::ThemeMode alternateMode() const
    {
        return m_originalMode == Ant::ThemeMode::Dark ? Ant::ThemeMode::Default : Ant::ThemeMode::Dark;
    }

private:
    Ant::ThemeMode m_originalMode;
};

QList<ObjectCase> objectCases()
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
        {"AntCheckbox", [](QWidget* parent) { return new AntCheckbox(parent); }},
        {"AntCollapsePanel", [](QWidget* parent) { return new AntCollapsePanel(QStringLiteral("Panel"), parent); }},
        {"AntCollapse", [](QWidget* parent) { return new AntCollapse(parent); }},
        {"AntColorPicker", [](QWidget* parent) { return new AntColorPicker(parent); }},
        {"AntConfigProvider", [](QWidget* parent) { return new AntConfigProvider(parent); }},
        {"AntDatePicker", [](QWidget* parent) { return new AntDatePicker(parent); }},
        {"AntDescriptionsItem", [](QWidget* parent) { return new AntDescriptionsItem(parent); }},
        {"AntDescriptions", [](QWidget* parent) { return new AntDescriptions(parent); }},
        {"AntDivider", [](QWidget* parent) { return new AntDivider(parent); }},
        {"AntDockWidget", [](QWidget* parent) { return new AntDockWidget(parent); }},
        {"AntDrawer", [](QWidget* parent) { return new AntDrawer(parent); }},
        {"AntDropdown", [](QWidget* parent) { return new AntDropdown(parent); }},
        {"AntEmpty", [](QWidget* parent) { return new AntEmpty(parent); }},
        {"AntFlex", [](QWidget* parent) { return new AntFlex(parent); }},
        {"AntFloatButton", [](QWidget* parent) { return new AntFloatButton(parent); }},
        {"AntFormItem", [](QWidget* parent) { return new AntFormItem(parent); }},
        {"AntFormProvider", [](QWidget* parent) { return new AntFormProvider(parent); }},
        {"AntForm", [](QWidget* parent) { return new AntForm(parent); }},
        {"AntFormList", [](QWidget* parent) { return new AntFormList(parent); }},
        {"AntCol", [](QWidget* parent) { return new AntCol(12, parent); }},
        {"AntRow", [](QWidget* parent) { return new AntRow(parent); }},
        {"AntIcon", [](QWidget* parent) { return new AntIcon(parent); }},
        {"AntImage", [](QWidget* parent) { return new AntImage(parent); }},
        {"AntInput", [](QWidget* parent) { return new AntInput(parent); }},
        {"AntInputNumber", [](QWidget* parent) { return new AntInputNumber(parent); }},
        {"AntLayoutHeader", [](QWidget* parent) { return new AntLayoutHeader(parent); }},
        {"AntLayoutFooter", [](QWidget* parent) { return new AntLayoutFooter(parent); }},
        {"AntLayoutContent", [](QWidget* parent) { return new AntLayoutContent(parent); }},
        {"AntLayoutSider", [](QWidget* parent) { return new AntLayoutSider(parent); }},
        {"AntLayout", [](QWidget* parent) { return new AntLayout(parent); }},
        {"AntListItemMeta", [](QWidget* parent) { return new AntListItemMeta(parent); }},
        {"AntListItem", [](QWidget* parent) { return new AntListItem(parent); }},
        {"AntList", [](QWidget* parent) { return new AntList(parent); }},
        {"AntLog", [](QWidget* parent) { return new AntLog(parent); }},
        {"AntMasonry", [](QWidget* parent) { return new AntMasonry(parent); }},
        {"AntMentions", [](QWidget* parent) { return new AntMentions(parent); }},
        {"AntMenu", [](QWidget* parent) { return new AntMenu(parent); }},
        {"AntMenuBar", [](QWidget* parent) { return new AntMenuBar(parent); }},
        {"AntMessage", [](QWidget* parent) { return new AntMessage(parent); }},
        {"AntModal", [](QWidget* parent) { return new AntModal(parent); }},
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
        {"AntSelect", [](QWidget* parent) { return new AntSelect(parent); }},
        {"AntSkeleton", [](QWidget* parent) { return new AntSkeleton(parent); }},
        {"AntSlider", [](QWidget* parent) { return new AntSlider(parent); }},
        {"AntSpace", [](QWidget* parent) { return new AntSpace(parent); }},
        {"AntSpin", [](QWidget* parent) { return new AntSpin(parent); }},
        {"AntSplitter", [](QWidget* parent) { return new AntSplitter(parent); }},
        {"AntStatistic", [](QWidget* parent) { return new AntStatistic(parent); }},
        {"AntStatusBar", [](QWidget* parent) { return new AntStatusBar(parent); }},
        {"AntSteps", [](QWidget* parent) { return new AntSteps(parent); }},
        {"AntSwitch", [](QWidget* parent) { return new AntSwitch(parent); }},
        {"AntTable", [](QWidget* parent) { return new AntTable(parent); }},
        {"AntTabs", [](QWidget* parent) { return new AntTabs(parent); }},
        {"AntTag", [](QWidget* parent) { return new AntTag(parent); }},
        {"AntTimeline", [](QWidget* parent) { return new AntTimeline(parent); }},
        {"AntTimePicker", [](QWidget* parent) { return new AntTimePicker(parent); }},
        {"AntToolBar", [](QWidget* parent) { return new AntToolBar(parent); }},
        {"AntToolButton", [](QWidget* parent) { return new AntToolButton(parent); }},
        {"AntTooltip", [](QWidget* parent) { return new AntTooltip(parent); }},
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

bool waitUntil(const std::function<bool()>& predicate, int timeoutMs = 700)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeoutMs)
    {
        QCoreApplication::processEvents();
        if (predicate())
        {
            return true;
        }
        QTest::qWait(10);
    }
    QCoreApplication::processEvents();
    return predicate();
}

void drainDeferredDeletes()
{
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
}

void sizeAndShowIfWidget(QObject* object)
{
    auto* widget = qobject_cast<QWidget*>(object);
    if (!widget)
    {
        return;
    }
    QSize size = widget->sizeHint();
    if (!size.isValid() || size.isEmpty())
    {
        size = QSize(120, 48);
    }
    widget->resize(size.expandedTo(QSize(80, 32)));
    widget->ensurePolished();
    widget->show();
}

QWidget* directVisiblePopupChild(QWidget* owner)
{
    const auto children = owner->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget* child : children)
    {
        if (child && child->isVisible())
        {
            const Qt::WindowType type = child->windowType();
            if (type == Qt::Popup || type == Qt::ToolTip)
            {
                return child;
            }
        }
    }
    return nullptr;
}

AntCascaderOption cascaderOption()
{
    AntCascaderOption option;
    option.value = QStringLiteral("root");
    option.label = QStringLiteral("Root");
    option.isLeaf = true;
    return option;
}

AntTreeNode treeNode()
{
    AntTreeNode node;
    node.key = QStringLiteral("root");
    node.title = QStringLiteral("Root");
    return node;
}
} // namespace

void TestAntThemeLifecycle::allControlsSurviveThemeSwitchAndDestroyCleanly()
{
    ThemeModeGuard guard;
    auto* root = new QWidget;
    root->resize(960, 720);
    root->show();
    QVERIFY(QTest::qWaitForWindowExposed(root));

    QList<QPointer<QObject>> tracked;
    for (const ObjectCase& objectCase : objectCases())
    {
        QObject* object = objectCase.create(root);
        QVERIFY2(object != nullptr, objectCase.name);
        tracked.append(QPointer<QObject>(object));
        sizeAndShowIfWidget(object);
    }

    antTheme->setThemeMode(guard.alternateMode());
    QCoreApplication::processEvents();
    for (const QPointer<QObject>& object : std::as_const(tracked))
    {
        QVERIFY(!object.isNull());
    }

    antTheme->setThemeMode(guard.alternateMode() == Ant::ThemeMode::Dark ? Ant::ThemeMode::Default : Ant::ThemeMode::Dark);
    QCoreApplication::processEvents();
    for (const QPointer<QObject>& object : std::as_const(tracked))
    {
        QVERIFY(!object.isNull());
    }

    delete root;
    drainDeferredDeletes();
    for (const QPointer<QObject>& object : std::as_const(tracked))
    {
        QVERIFY(object.isNull());
    }

    antTheme->toggleThemeMode();
    QCoreApplication::processEvents();
}

void TestAntThemeLifecycle::openPopupsSurviveThemeSwitch()
{
    ThemeModeGuard guard;
    QWidget host;
    host.resize(640, 360);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    QPushButton target(QStringLiteral("Target"), &host);
    target.resize(120, 32);
    target.move(24, 24);
    target.show();

    AntSelect select(&host);
    select.addOption(QStringLiteral("Apple"), QStringLiteral("apple"));
    select.resize(180, select.sizeHint().height());
    select.move(24, 72);
    select.show();
    select.setOpen(true);
    QVERIFY(waitUntil([&]() { return select.isOpen() && directVisiblePopupChild(&select); }));

    AntCascader cascader(&host);
    cascader.setOptions({cascaderOption()});
    cascader.resize(180, cascader.sizeHint().height());
    cascader.move(220, 72);
    cascader.show();
    cascader.setOpen(true);
    QVERIFY(waitUntil([&]() { return cascader.isOpen() && directVisiblePopupChild(&cascader); }));

    AntColorPicker colorPicker(&host);
    colorPicker.resize(colorPicker.sizeHint());
    colorPicker.move(24, 128);
    colorPicker.show();
    colorPicker.setOpen(true);
    QVERIFY(waitUntil([&]() { return colorPicker.isOpen() && directVisiblePopupChild(&colorPicker); }));

    AntTreeSelect treeSelect(&host);
    treeSelect.setTreeData({treeNode()});
    treeSelect.resize(180, treeSelect.sizeHint().height());
    treeSelect.move(220, 128);
    treeSelect.show();
    treeSelect.setOpen(true);
    QVERIFY(waitUntil([&]() { return treeSelect.isOpen() && directVisiblePopupChild(&treeSelect); }));

    AntDropdown dropdown(&host);
    dropdown.setTarget(&target);
    dropdown.addItem(QStringLiteral("copy"), QStringLiteral("Copy"));
    dropdown.setOpen(true);
    QVERIFY(waitUntil([&]() { return dropdown.isOpen() && directVisiblePopupChild(&dropdown); }));

    AntPopover popover(&host);
    popover.setTarget(&target);
    popover.setTitle(QStringLiteral("Title"));
    popover.setContent(QStringLiteral("Content"));
    popover.setOpen(true);
    QVERIFY(waitUntil([&]() { return popover.isOpen() && popover.isVisible(); }));

    AntTooltip tooltip(&host);
    tooltip.setTarget(&target);
    tooltip.setTitle(QStringLiteral("Tooltip"));
    tooltip.showTooltip();
    QVERIFY(waitUntil([&]() { return tooltip.isVisible(); }));

    antTheme->setThemeMode(guard.alternateMode());
    QCoreApplication::processEvents();

    QVERIFY(select.isOpen());
    QVERIFY(cascader.isOpen());
    QVERIFY(colorPicker.isOpen());
    QVERIFY(treeSelect.isOpen());
    QVERIFY(dropdown.isOpen());
    QVERIFY(popover.isOpen());
    QVERIFY(tooltip.isVisible());

    select.setOpen(false);
    cascader.setOpen(false);
    colorPicker.setOpen(false);
    treeSelect.setOpen(false);
    dropdown.setOpen(false);
    popover.setOpen(false);
    tooltip.hideTooltip();
}

QTEST_MAIN(TestAntThemeLifecycle)
#include "TestAntThemeLifecycle.moc"
