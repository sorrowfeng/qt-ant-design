#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QPointer>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>
#include <QWidget>

#include <functional>

#include "widgets/AntCascader.h"
#include "widgets/AntColorPicker.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntDropdown.h"
#include "widgets/AntPopconfirm.h"
#include "widgets/AntPopover.h"
#include "widgets/AntSelect.h"
#include "widgets/AntTimePicker.h"
#include "widgets/AntToolTip.h"
#include "widgets/AntTreeSelect.h"

class TestAntPopupLifecycle : public QObject
{
    Q_OBJECT

private slots:
    void ownedPopupControlsCloseAndDestroySafely();
    void targetDrivenPopupsCloseAndDestroySafely();
};

namespace
{
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

QWidget* visiblePopupChild(QWidget* owner)
{
    const auto children = owner->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget* child : children)
    {
        if (!child || !child->isVisible())
        {
            continue;
        }
        const Qt::WindowType type = child->windowType();
        if (type == Qt::Popup || type == Qt::ToolTip)
        {
            return child;
        }
    }
    return nullptr;
}

template <typename Widget>
void verifyChildPopupClosesAndOwnerDeleteCleansUp(Widget* widget, const char* name)
{
    QPointer<Widget> ownerGuard(widget);
    QSignalSpy openSpy(widget, SIGNAL(openChanged(bool)));

    widget->setOpen(true);
    QVERIFY2(waitUntil([&]() { return widget->isOpen(); }), name);

    QWidget* popup = nullptr;
    QVERIFY2(waitUntil([&]() {
                 popup = visiblePopupChild(widget);
                 return popup != nullptr;
             }),
             name);
    QPointer<QWidget> popupGuard(popup);

    widget->setOpen(false);
    QVERIFY2(waitUntil([&]() { return !widget->isOpen(); }), name);
    QVERIFY2(waitUntil([&]() { return popupGuard.isNull() || !popupGuard->isVisible(); }), name);
    QVERIFY2(openSpy.count() >= 2, name);

    widget->setOpen(true);
    QVERIFY2(waitUntil([&]() {
                 popup = visiblePopupChild(widget);
                 return widget->isOpen() && popup != nullptr;
             }),
             name);
    popupGuard = popup;

    delete widget;
    drainDeferredDeletes();

    QVERIFY2(ownerGuard.isNull(), name);
    QVERIFY2(popupGuard.isNull(), name);
}

AntCascaderOption cascaderLeaf()
{
    AntCascaderOption option;
    option.value = QStringLiteral("zhejiang");
    option.label = QStringLiteral("Zhejiang");
    option.isLeaf = true;
    return option;
}

AntTreeNode treeSelectNode()
{
    AntTreeNode node;
    node.key = QStringLiteral("root");
    node.title = QStringLiteral("Root");
    return node;
}
} // namespace

void TestAntPopupLifecycle::ownedPopupControlsCloseAndDestroySafely()
{
    QWidget host;
    host.resize(420, 320);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    auto* select = new AntSelect(&host);
    select->addOption(QStringLiteral("Apple"), QStringLiteral("apple"));
    select->addOption(QStringLiteral("Cherry"), QStringLiteral("cherry"));
    select->resize(180, select->sizeHint().height());
    select->show();
    verifyChildPopupClosesAndOwnerDeleteCleansUp(select, "AntSelect");

    auto* cascader = new AntCascader(&host);
    cascader->setOptions({cascaderLeaf()});
    cascader->resize(180, cascader->sizeHint().height());
    cascader->show();
    verifyChildPopupClosesAndOwnerDeleteCleansUp(cascader, "AntCascader");

    auto* datePicker = new AntDatePicker(&host);
    datePicker->setSelectedDate(QDate(2026, 5, 1));
    datePicker->resize(datePicker->sizeHint());
    datePicker->show();
    verifyChildPopupClosesAndOwnerDeleteCleansUp(datePicker, "AntDatePicker");

    auto* timePicker = new AntTimePicker(&host);
    timePicker->setSelectedTime(QTime(10, 20, 30));
    timePicker->resize(timePicker->sizeHint());
    timePicker->show();
    verifyChildPopupClosesAndOwnerDeleteCleansUp(timePicker, "AntTimePicker");

    auto* colorPicker = new AntColorPicker(QColor(0x16, 0x77, 0xff), &host);
    colorPicker->resize(colorPicker->sizeHint());
    colorPicker->show();
    verifyChildPopupClosesAndOwnerDeleteCleansUp(colorPicker, "AntColorPicker");

    auto* treeSelect = new AntTreeSelect(&host);
    treeSelect->setTreeData({treeSelectNode()});
    treeSelect->resize(220, treeSelect->sizeHint().height());
    treeSelect->show();
    verifyChildPopupClosesAndOwnerDeleteCleansUp(treeSelect, "AntTreeSelect");
}

void TestAntPopupLifecycle::targetDrivenPopupsCloseAndDestroySafely()
{
    QWidget host;
    host.resize(420, 320);

    QPushButton target(QStringLiteral("Target"), &host);
    target.resize(120, 32);
    target.move(40, 40);

    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    auto* dropdown = new AntDropdown(&host);
    dropdown->setTarget(&target);
    dropdown->addItem(QStringLiteral("copy"), QStringLiteral("Copy"));
    verifyChildPopupClosesAndOwnerDeleteCleansUp(dropdown, "AntDropdown");
    QTest::mouseClick(&target, Qt::LeftButton, Qt::NoModifier, target.rect().center());

    auto* popconfirm = new AntPopconfirm(&host);
    popconfirm->setTarget(&target);
    popconfirm->setTitle(QStringLiteral("Delete?"));
    popconfirm->setDescription(QStringLiteral("This cannot be undone."));
    verifyChildPopupClosesAndOwnerDeleteCleansUp(popconfirm, "AntPopconfirm");
    QTest::mouseClick(&target, Qt::LeftButton, Qt::NoModifier, target.rect().center());

    auto* popover = new AntPopover(&host);
    popover->setTarget(&target);
    popover->setTrigger(Ant::PopoverTrigger::Click);
    popover->setTitle(QStringLiteral("Title"));
    popover->setContent(QStringLiteral("Content"));
    QPointer<AntPopover> popoverGuard(popover);
    QSignalSpy popoverOpenSpy(popover, &AntPopover::openChanged);
    popover->setOpen(true);
    QVERIFY(waitUntil([&]() { return popover->isOpen() && popover->isVisible(); }));
    popover->setOpen(false);
    QVERIFY(waitUntil([&]() { return !popover->isOpen() && !popover->isVisible(); }));
    QVERIFY(popoverOpenSpy.count() >= 2);
    popover->setOpen(true);
    QVERIFY(waitUntil([&]() { return popover->isOpen() && popover->isVisible(); }));
    delete popover;
    drainDeferredDeletes();
    QVERIFY(popoverGuard.isNull());
    QTest::mouseClick(&target, Qt::LeftButton, Qt::NoModifier, target.rect().center());

    auto* tooltip = new AntToolTip(&host);
    tooltip->setTarget(&target);
    tooltip->setTitle(QStringLiteral("Tooltip"));
    QPointer<AntToolTip> tooltipGuard(tooltip);
    tooltip->showTooltip();
    QVERIFY(waitUntil([&]() { return tooltip->isVisible(); }));
    tooltip->hideTooltip();
    QVERIFY(waitUntil([&]() { return !tooltip->isVisible(); }));
    tooltip->showTooltip();
    QVERIFY(waitUntil([&]() { return tooltip->isVisible(); }));
    delete tooltip;
    drainDeferredDeletes();
    QVERIFY(tooltipGuard.isNull());
    QTest::mouseClick(&target, Qt::LeftButton, Qt::NoModifier, target.rect().center());
}

QTEST_MAIN(TestAntPopupLifecycle)
#include "TestAntPopupLifecycle.moc"
