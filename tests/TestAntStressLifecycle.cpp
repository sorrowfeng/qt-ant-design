#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QContextMenuEvent>
#include <QDate>
#include <QDialog>
#include <QElapsedTimer>
#include <QEvent>
#include <QImage>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPointer>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTest>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <functional>

#include "core/AntTheme.h"
#include "widgets/AntAffix.h"
#include "widgets/AntAnchor.h"
#include "widgets/AntButton.h"
#include "widgets/AntCascader.h"
#include "widgets/AntColorPicker.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntDropdown.h"
#include "widgets/AntDockManager.h"
#include "widgets/AntDockWidget.h"
#include "widgets/AntFloatButton.h"
#include "widgets/AntImage.h"
#include "widgets/AntInput.h"
#include "widgets/AntInputNumber.h"
#include "widgets/AntMessage.h"
#include "widgets/AntMenuBar.h"
#include "widgets/AntNotification.h"
#include "widgets/AntSelect.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntTabs.h"
#include "widgets/AntTimePicker.h"
#include "widgets/AntTour.h"
#include "widgets/AntTreeSelect.h"

class TestAntStressLifecycle : public QObject
{
    Q_OBJECT

private slots:
    void repeatedThemeSwitchesKeepCompositeSurfaceAlive();
    void repeatedOwnedPopupOpenCloseCyclesReleaseFrames();
    void transientFeedbackBurstClosesCleanly();
    void controllersSurviveReferencedWidgetDestruction();
    void affixRetargetsDuringDestroyedSignalCleanup();
    void previewAndTourSurviveReverseDestruction();
    void dockDragSurvivesDraggedDockDestruction();
    void menuBarPreservesNativeHoverSemantics();
    void menuBarSurvivesHoveredActionDestruction();
    void synchronousSignalDeletionStopsEventHandlers();
};

namespace
{
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

private:
    Ant::ThemeMode m_originalMode;
};

bool waitUntil(const std::function<bool()>& predicate, int timeoutMs = 1200)
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

void sendMouseEvent(QWidget* target,
                    QEvent::Type type,
                    const QPoint& localPos,
                    Qt::MouseButton button,
                    Qt::MouseButtons buttons)
{
    const QPoint globalPos = target->mapToGlobal(localPos);
    QMouseEvent event(type,
                      QPointF(localPos),
                      QPointF(globalPos),
                      button,
                      buttons,
                      Qt::NoModifier);
    QCoreApplication::sendEvent(target, &event);
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

AntCascaderOption cascaderLeaf()
{
    AntCascaderOption option;
    option.value = QStringLiteral("root");
    option.label = QStringLiteral("Root");
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

template <typename Widget>
void verifyRepeatedChildPopupCycles(Widget* widget, const char* name, int cycles = 6)
{
    for (int i = 0; i < cycles; ++i)
    {
        widget->setOpen(true);

        QWidget* popup = nullptr;
        QVERIFY2(waitUntil([&]() {
                     popup = visiblePopupChild(widget);
                     return widget->isOpen() && popup != nullptr;
                 }),
                 name);

        QPointer<QWidget> popupGuard(popup);
        antTheme->toggleThemeMode();
        QCoreApplication::processEvents();

        QVERIFY2(widget->isOpen(), name);
        QVERIFY2(!popupGuard.isNull(), name);

        widget->setOpen(false);
        QVERIFY2(waitUntil([&]() { return !widget->isOpen(); }), name);
        QVERIFY2(waitUntil([&]() { return popupGuard.isNull() || !popupGuard->isVisible(); }), name);
    }
}
} // namespace

void TestAntStressLifecycle::repeatedThemeSwitchesKeepCompositeSurfaceAlive()
{
    ThemeModeGuard guard;

    QWidget host;
    host.resize(520, 360);

    auto* layout = new QVBoxLayout(&host);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* button = new AntButton(QStringLiteral("Primary"), &host);
    button->setButtonType(Ant::ButtonType::Primary);
    layout->addWidget(button);

    auto* input = new AntInput(&host);
    input->setText(QStringLiteral("stress input"));
    layout->addWidget(input);

    auto* inputNumber = new AntInputNumber(&host);
    inputNumber->setValue(12.0);
    layout->addWidget(inputNumber);

    auto* switcher = new AntSwitch(&host);
    layout->addWidget(switcher);

    auto* tabs = new AntTabs(&host);
    tabs->addTab(new QWidget(tabs), QStringLiteral("one"), QStringLiteral("One"));
    tabs->addTab(new QWidget(tabs), QStringLiteral("two"), QStringLiteral("Two"));
    tabs->setActiveKey(QStringLiteral("two"));
    layout->addWidget(tabs);

    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    QList<QPointer<QWidget>> tracked = {button, input, inputNumber, switcher, tabs};
    QSignalSpy themeSpy(antTheme, &AntTheme::themeChanged);

    for (int i = 0; i < 24; ++i)
    {
        antTheme->toggleThemeMode();
        QCoreApplication::processEvents();

        QImage frame(host.size(), QImage::Format_ARGB32_Premultiplied);
        frame.fill(Qt::transparent);
        host.render(&frame);

        for (const QPointer<QWidget>& widget : tracked)
        {
            QVERIFY(!widget.isNull());
            QVERIFY(widget->isVisible());
        }
    }

    QCOMPARE(themeSpy.count(), 24);
}

void TestAntStressLifecycle::repeatedOwnedPopupOpenCloseCyclesReleaseFrames()
{
    ThemeModeGuard guard;

    QWidget host;
    host.resize(620, 420);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    auto* select = new AntSelect(&host);
    select->addOption(QStringLiteral("Apple"), QStringLiteral("apple"));
    select->addOption(QStringLiteral("Cherry"), QStringLiteral("cherry"));
    select->resize(200, select->sizeHint().height());
    select->move(24, 24);
    select->show();
    verifyRepeatedChildPopupCycles(select, "AntSelect");

    auto* cascader = new AntCascader(&host);
    cascader->setOptions({cascaderLeaf()});
    cascader->resize(200, cascader->sizeHint().height());
    cascader->move(260, 24);
    cascader->show();
    verifyRepeatedChildPopupCycles(cascader, "AntCascader");

    auto* datePicker = new AntDatePicker(&host);
    datePicker->setSelectedDate(QDate(2026, 5, 1));
    datePicker->resize(datePicker->sizeHint());
    datePicker->move(24, 90);
    datePicker->show();
    verifyRepeatedChildPopupCycles(datePicker, "AntDatePicker");

    auto* timePicker = new AntTimePicker(&host);
    timePicker->setSelectedTime(QTime(10, 20, 30));
    timePicker->resize(timePicker->sizeHint());
    timePicker->move(260, 90);
    timePicker->show();
    verifyRepeatedChildPopupCycles(timePicker, "AntTimePicker");

    auto* colorPicker = new AntColorPicker(QColor(0x16, 0x77, 0xff), &host);
    colorPicker->resize(colorPicker->sizeHint());
    colorPicker->move(24, 156);
    colorPicker->show();
    verifyRepeatedChildPopupCycles(colorPicker, "AntColorPicker");

    auto* treeSelect = new AntTreeSelect(&host);
    treeSelect->setTreeData({treeSelectNode()});
    treeSelect->resize(220, treeSelect->sizeHint().height());
    treeSelect->move(260, 156);
    treeSelect->show();
    verifyRepeatedChildPopupCycles(treeSelect, "AntTreeSelect");

    QPushButton target(QStringLiteral("Target"), &host);
    target.resize(120, 32);
    target.move(24, 228);
    target.show();

    auto* dropdown = new AntDropdown(&host);
    dropdown->setTarget(&target);
    dropdown->addItem(QStringLiteral("copy"), QStringLiteral("Copy"));
    dropdown->addItem(QStringLiteral("paste"), QStringLiteral("Paste"));
    verifyRepeatedChildPopupCycles(dropdown, "AntDropdown");

    drainDeferredDeletes();
    QVERIFY(visiblePopupChild(select) == nullptr);
    QVERIFY(visiblePopupChild(cascader) == nullptr);
    QVERIFY(visiblePopupChild(datePicker) == nullptr);
    QVERIFY(visiblePopupChild(timePicker) == nullptr);
    QVERIFY(visiblePopupChild(colorPicker) == nullptr);
    QVERIFY(visiblePopupChild(treeSelect) == nullptr);
    QVERIFY(visiblePopupChild(dropdown) == nullptr);
}

void TestAntStressLifecycle::transientFeedbackBurstClosesCleanly()
{
    QWidget anchor;
    anchor.resize(420, 240);
    anchor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&anchor));

    for (int i = 0; i < 6; ++i)
    {
        QPointer<AntMessage> message(AntMessage::success(QStringLiteral("Saved"), &anchor, 30));
        QVERIFY(waitUntil([&]() { return !message.isNull() && message->isVisible(); }));
        QVERIFY(waitUntil([&]() { return message.isNull(); }, 1000));
    }

    for (int i = 0; i < 6; ++i)
    {
        QPointer<AntNotification> notification(
            AntNotification::info(QStringLiteral("Notice"), QStringLiteral("Stress close"), &anchor, 0));
        QVERIFY(waitUntil([&]() { return !notification.isNull() && notification->isVisible(); }));

        AntNotification::closeAll();
        QVERIFY(waitUntil([&]() { return notification.isNull(); }, 1000));
    }

    drainDeferredDeletes();
}

void TestAntStressLifecycle::controllersSurviveReferencedWidgetDestruction()
{
    AntAffix affix;
    auto* scrollArea = new QScrollArea;
    auto* scrollContent = new QWidget;
    auto* affixedWidget = new QWidget(scrollContent);
    scrollArea->setWidget(scrollContent);

    affix.setAffixedWidget(affixedWidget);
    affix.setScrollTarget(scrollArea);
    affix.setOffsetTop(8);
    QVERIFY(affix.property("antAffixCheckQueued").toBool());

    QPointer<QScrollArea> scrollAreaGuard(scrollArea);
    QPointer<QWidget> affixedWidgetGuard(affixedWidget);
    scrollArea->deleteLater();
    drainDeferredDeletes();
    QCoreApplication::processEvents();

    QVERIFY(scrollAreaGuard.isNull());
    QVERIFY(affixedWidgetGuard.isNull());
    QVERIFY(affix.scrollTarget() == nullptr);
    QVERIFY(affix.affixedWidget() == nullptr);
    QVERIFY(!affix.isAffixed());
    QVERIFY(!affix.property("antAffixCheckQueued").toBool());
    affix.setScrollTarget(nullptr);
    affix.setAffixedWidget(nullptr);

    QWidget borrowedHost;
    borrowedHost.resize(240, 120);
    borrowedHost.show();
    auto* borrowedWidget = new QWidget(&borrowedHost);
    borrowedWidget->resize(80, 24);
    borrowedWidget->show();
    auto* firstIndependentArea = new QScrollArea;
    firstIndependentArea->resize(240, 120);
    firstIndependentArea->show();
    auto* secondIndependentArea = new QScrollArea;
    secondIndependentArea->resize(240, 120);
    secondIndependentArea->show();

    AntAffix borrowedAffix;
    borrowedAffix.setAffixedWidget(borrowedWidget);
    borrowedAffix.setScrollTarget(firstIndependentArea);
    borrowedAffix.setOffsetTop(10000);
    QTRY_VERIFY(borrowedAffix.isAffixed());
    QCOMPARE(borrowedWidget->parentWidget(), firstIndependentArea->viewport());

    QPointer<QWidget> borrowedGuard(borrowedWidget);
    borrowedAffix.setScrollTarget(secondIndependentArea);
    QVERIFY(!borrowedAffix.isAffixed());
    QCOMPARE(borrowedWidget->parentWidget(), &borrowedHost);
    delete firstIndependentArea;
    QVERIFY(!borrowedGuard.isNull());

    QTRY_VERIFY(borrowedAffix.isAffixed());
    QCOMPARE(borrowedWidget->parentWidget(), secondIndependentArea->viewport());

    QPointer<QWidget> oldViewportGuard(secondIndependentArea->viewport());
    auto* replacementViewport = new QWidget;
    secondIndependentArea->setViewport(replacementViewport);
    QTRY_VERIFY(oldViewportGuard.isNull());
    QVERIFY(!borrowedGuard.isNull());
    QCOMPARE(borrowedWidget->parentWidget(), &borrowedHost);
    QVERIFY(!borrowedAffix.isAffixed());

    borrowedAffix.setScrollTarget(secondIndependentArea);
    QTRY_VERIFY(borrowedAffix.isAffixed());
    QCOMPARE(borrowedWidget->parentWidget(), replacementViewport);

    delete secondIndependentArea;
    QVERIFY(!borrowedGuard.isNull());
    QCOMPARE(borrowedWidget->parentWidget(), &borrowedHost);
    QVERIFY(!borrowedAffix.isAffixed());

    auto* topLevelBorrowed = new QWidget;
    topLevelBorrowed->resize(80, 24);
    topLevelBorrowed->show();
    auto* topLevelArea = new QScrollArea;
    topLevelArea->resize(240, 120);
    topLevelArea->show();
    AntAffix topLevelAffix;
    topLevelAffix.setAffixedWidget(topLevelBorrowed);
    topLevelAffix.setScrollTarget(topLevelArea);
    topLevelAffix.setOffsetTop(10000);
    QTRY_VERIFY(topLevelAffix.isAffixed());
    QPointer<QWidget> topLevelBorrowedGuard(topLevelBorrowed);
    delete topLevelArea;
    QVERIFY(!topLevelBorrowedGuard.isNull());
    QVERIFY(topLevelBorrowed->parentWidget() == nullptr);
    QVERIFY(!topLevelAffix.isAffixed());
    delete topLevelBorrowed;

    AntAnchor anchor;
    auto* firstAnchorArea = new QScrollArea;
    anchor.setScrollArea(firstAnchorArea);
    QPointer<QScrollArea> firstAnchorGuard(firstAnchorArea);
    firstAnchorArea->deleteLater();
    drainDeferredDeletes();
    QVERIFY(firstAnchorGuard.isNull());
    anchor.setScrollArea(nullptr);

    auto* secondAnchorArea = new QScrollArea;
    anchor.setScrollArea(secondAnchorArea);
    secondAnchorArea->verticalScrollBar()->setValue(12);
    anchor.setScrollArea(nullptr);
    delete secondAnchorArea;

    QWidget floatHost;
    auto* group = new AntFloatButton(&floatHost);
    auto* child = new AntFloatButton(&floatHost);
    group->addChild(child);
    group->setOpen(true);
    QCOMPARE(group->childButtons().size(), 1);

    QPointer<AntFloatButton> childGuard(child);
    child->deleteLater();
    drainDeferredDeletes();
    QVERIFY(childGuard.isNull());
    QVERIFY(group->childButtons().isEmpty());
    QVERIFY(!group->isOpen());

    auto* backTop = new AntFloatButton(&floatHost);
    backTop->setBackTop(true);
    auto* firstBackTopArea = new QScrollArea;
    backTop->setScrollTarget(firstBackTopArea);
    QPointer<QScrollArea> firstBackTopGuard(firstBackTopArea);
    firstBackTopArea->deleteLater();
    drainDeferredDeletes();
    QVERIFY(firstBackTopGuard.isNull());
    QVERIFY(backTop->scrollTarget() == nullptr);

    auto* secondBackTopArea = new QScrollArea;
    backTop->setScrollTarget(secondBackTopArea);
    QCOMPARE(backTop->scrollTarget(), secondBackTopArea);
    backTop->setScrollTarget(nullptr);
    delete secondBackTopArea;
}

void TestAntStressLifecycle::affixRetargetsDuringDestroyedSignalCleanup()
{
    QWidget host;
    host.resize(240, 120);
    host.show();

    auto* targetDestroyedWidget = new QWidget(&host);
    targetDestroyedWidget->resize(80, 24);
    targetDestroyedWidget->show();
    auto* firstArea = new QScrollArea;
    firstArea->resize(240, 120);
    firstArea->show();
    auto* replacementArea = new QScrollArea;
    replacementArea->resize(240, 120);
    replacementArea->show();

    AntAffix targetAffix;
    targetAffix.setAffixedWidget(targetDestroyedWidget);
    targetAffix.setScrollTarget(firstArea);
    targetAffix.setOffsetTop(10000);
    QTRY_VERIFY(targetAffix.isAffixed());

    bool targetRetargeted = false;
    connect(&targetAffix, &AntAffix::affixStateChanged, &targetAffix,
            [&](bool affixed) {
                if (!affixed && !targetRetargeted)
                {
                    targetRetargeted = true;
                    targetAffix.setScrollTarget(replacementArea);
                }
            });

    delete firstArea;
    QVERIFY(targetRetargeted);
    QCOMPARE(targetAffix.scrollTarget(), replacementArea);
    QTRY_VERIFY(targetAffix.isAffixed());
    QCOMPARE(targetDestroyedWidget->parentWidget(), replacementArea->viewport());

    targetAffix.setScrollTarget(nullptr);
    QCOMPARE(targetDestroyedWidget->parentWidget(), &host);
    delete replacementArea;

    auto* viewportDestroyedWidget = new QWidget(&host);
    viewportDestroyedWidget->resize(80, 24);
    viewportDestroyedWidget->show();
    auto* viewportArea = new QScrollArea;
    viewportArea->resize(240, 120);
    viewportArea->show();

    AntAffix viewportAffix;
    viewportAffix.setAffixedWidget(viewportDestroyedWidget);
    viewportAffix.setScrollTarget(viewportArea);
    viewportAffix.setOffsetTop(10000);
    QTRY_VERIFY(viewportAffix.isAffixed());

    bool viewportRetargeted = false;
    connect(&viewportAffix, &AntAffix::affixStateChanged, &viewportAffix,
            [&](bool affixed) {
                if (!affixed && !viewportRetargeted)
                {
                    viewportRetargeted = true;
                    viewportAffix.setScrollTarget(viewportArea);
                }
            });

    QPointer<QWidget> oldViewport(viewportArea->viewport());
    auto* replacementViewport = new QWidget;
    viewportArea->setViewport(replacementViewport);
    QTRY_VERIFY(oldViewport.isNull());
    QVERIFY(viewportRetargeted);
    QCOMPARE(viewportAffix.scrollTarget(), viewportArea);
    QTRY_VERIFY(viewportAffix.isAffixed());
    QCOMPARE(viewportDestroyedWidget->parentWidget(), replacementViewport);

    viewportAffix.setScrollTarget(nullptr);
    QCOMPARE(viewportDestroyedWidget->parentWidget(), &host);
    delete viewportArea;
}

void TestAntStressLifecycle::previewAndTourSurviveReverseDestruction()
{
    {
        QWidget imageHost;
        imageHost.resize(380, 180);
        auto* survivor = new AntImage(&imageHost);
        survivor->setSrc(QStringLiteral(":/qt-ant-design/images/image-basic.png"));
        survivor->setGeometry(20, 20, 150, 100);
        auto* removedMember = new AntImage(&imageHost);
        removedMember->setSrc(QStringLiteral(":/qt-ant-design/images/image-basic.png"));
        removedMember->setGeometry(200, 20, 150, 100);
        survivor->setPreviewGroup({survivor, removedMember});

        imageHost.show();
        QVERIFY(QTest::qWaitForWindowExposed(&imageHost));

        QPointer<AntImage> removedMemberGuard(removedMember);
        removedMember->deleteLater();
        drainDeferredDeletes();
        QVERIFY(removedMemberGuard.isNull());

        QSignalSpy clickSpy(survivor, &AntImage::clicked);
        QPointer<AntImage> survivorGuard(survivor);
        QTimer::singleShot(0, qApp, [survivorGuard]() {
            if (QWidget* modal = QApplication::activeModalWidget())
            {
                if (survivorGuard)
                {
                    survivorGuard->setProperty("antImagePreviewOpenedForLifecycleTest", true);
                }
                modal->close();
            }
        });
        QTest::mouseClick(survivor, Qt::LeftButton, Qt::NoModifier, survivor->rect().center());
        QCOMPARE(clickSpy.count(), 1);
        QVERIFY(survivor->property("antImagePreviewOpenedForLifecycleTest").toBool());
    }

    auto* tourHost = new QWidget;
    tourHost->resize(420, 260);
    auto* target = new QPushButton(QStringLiteral("Target"), tourHost);
    target->setGeometry(40, 40, 120, 32);
    tourHost->show();
    QVERIFY(QTest::qWaitForWindowExposed(tourHost));
    tourHost->raise();
    tourHost->activateWindow();
    QTRY_VERIFY(QApplication::activeWindow() == tourHost);

    AntTour tour;
    tour.addStep({target, QStringLiteral("Step"), QStringLiteral("Description"), Qt::AlignBottom});
    tour.start();
    QCOMPARE(tour.property("antTourCurrentIndex").toInt(), 0);
    QVERIFY(tourHost->findChild<QDialog*>() != nullptr);

    QPointer<QPushButton> targetGuard(target);
    target->deleteLater();
    drainDeferredDeletes();
    QVERIFY(targetGuard.isNull());
    tour.start(0);
    tour.next();
    drainDeferredDeletes();

    tour.start(0);
    QPointer<QWidget> tourHostGuard(tourHost);
    tourHost->deleteLater();
    drainDeferredDeletes();
    QVERIFY(tourHostGuard.isNull());

    tour.next();
    tour.start(0);
    tour.close();
    drainDeferredDeletes();

    auto* controllerHost = new QWidget;
    controllerHost->resize(360, 220);
    auto* controllerTarget = new QPushButton(QStringLiteral("Controller target"), controllerHost);
    controllerTarget->setGeometry(32, 32, 140, 32);
    controllerHost->show();
    QVERIFY(QTest::qWaitForWindowExposed(controllerHost));
    controllerHost->raise();
    controllerHost->activateWindow();
    QTRY_VERIFY(QApplication::activeWindow() == controllerHost);

    auto* controller = new AntTour;
    controller->addStep({controllerTarget,
                         QStringLiteral("Controller"),
                         QStringLiteral("Controller is destroyed first"),
                         Qt::AlignBottom});
    int controllerFinishedCount = 0;
    connect(controller, &AntTour::finished, controllerHost, [&controllerFinishedCount]() {
        ++controllerFinishedCount;
    });
    controller->start();
    QPointer<QDialog> controllerOverlay(controllerHost->findChild<QDialog*>());
    QVERIFY(!controllerOverlay.isNull());

    delete controller;
    QVERIFY(controllerOverlay.isNull());
    QCOMPARE(controllerFinishedCount, 0);
    delete controllerHost;
}

void TestAntStressLifecycle::dockDragSurvivesDraggedDockDestruction()
{
    AntDockManager manager;
    manager.resize(720, 460);

    auto* doomedDock = new AntDockWidget(QStringLiteral("Doomed"));
    doomedDock->setWidget(new QWidget);
    auto* survivorDock = new AntDockWidget(QStringLiteral("Survivor"));
    survivorDock->setWidget(new QWidget);
    manager.addDockWidget(Qt::LeftDockWidgetArea, doomedDock);
    manager.addDockWidget(Qt::RightDockWidgetArea, survivorDock);

    manager.show();
    QVERIFY(QTest::qWaitForWindowExposed(&manager));

    QWidget* doomedTitle = doomedDock->titleBarWidget();
    QVERIFY(doomedTitle != nullptr);
    const QPoint doomedPress = doomedTitle->rect().center();
    sendMouseEvent(doomedTitle, QEvent::MouseButtonPress, doomedPress, Qt::LeftButton, Qt::LeftButton);
    const QPoint firstDragGlobal = manager.mapToGlobal(manager.rect().center());
    sendMouseEvent(doomedTitle,
                   QEvent::MouseMove,
                   doomedTitle->mapFromGlobal(firstDragGlobal),
                   Qt::NoButton,
                   Qt::LeftButton);
    QVERIFY(manager.property("antDockDragActivated").toBool());

    QPointer<AntDockWidget> doomedGuard(doomedDock);
    doomedDock->deleteLater();
    drainDeferredDeletes();
    QVERIFY(doomedGuard.isNull());
    QVERIFY(!manager.property("antDockDragActivated").toBool());
    QVERIFY(!manager.isDropPreviewVisible());
    QCOMPARE(manager.dockWidgets().size(), 1);

    QKeyEvent escapeAfterDelete(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QCoreApplication::sendEvent(qApp, &escapeAfterDelete);

    QWidget* survivorTitle = survivorDock->titleBarWidget();
    QVERIFY(survivorTitle != nullptr);
    const QPoint survivorPress = survivorTitle->rect().center();
    sendMouseEvent(survivorTitle, QEvent::MouseButtonPress, survivorPress, Qt::LeftButton, Qt::LeftButton);
    const QPoint secondDragGlobal = manager.mapToGlobal(manager.rect().center() + QPoint(40, 20));
    sendMouseEvent(survivorTitle,
                   QEvent::MouseMove,
                   survivorTitle->mapFromGlobal(secondDragGlobal),
                   Qt::NoButton,
                   Qt::LeftButton);
    QVERIFY(manager.property("antDockDragActivated").toBool());

    QKeyEvent escapeSecondDrag(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QCoreApplication::sendEvent(qApp, &escapeSecondDrag);
    QVERIFY(!manager.property("antDockDragActivated").toBool());
    QVERIFY(!manager.isDropPreviewVisible());

    QPointer<AntDockWidget> survivorGuard(survivorDock);
    delete survivorDock;
    QVERIFY(survivorGuard.isNull());
    QVERIFY(manager.dockWidgets().isEmpty());

    auto* transientManager = new AntDockManager;
    transientManager->resize(640, 400);
    auto* transientDock = new AntDockWidget(QStringLiteral("Transient"));
    transientDock->setWidget(new QWidget);
    transientManager->addDockWidget(Qt::LeftDockWidgetArea, transientDock);
    transientManager->show();
    QVERIFY(QTest::qWaitForWindowExposed(transientManager));

    QWidget* transientTitle = transientDock->titleBarWidget();
    QVERIFY(transientTitle != nullptr);
    const QPoint transientPress = transientTitle->rect().center();
    sendMouseEvent(transientTitle,
                   QEvent::MouseButtonPress,
                   transientPress,
                   Qt::LeftButton,
                   Qt::LeftButton);
    sendMouseEvent(transientTitle,
                   QEvent::MouseMove,
                   transientTitle->mapFromGlobal(
                       transientManager->mapToGlobal(transientManager->rect().center())),
                   Qt::NoButton,
                   Qt::LeftButton);
    QVERIFY(transientManager->property("antDockDragActivated").toBool());

    QPointer<AntDockManager> transientManagerGuard(transientManager);
    delete transientManager;
    QVERIFY(transientManagerGuard.isNull());

    QKeyEvent escapeAfterManagerDelete(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QCoreApplication::sendEvent(qApp, &escapeAfterManagerDelete);
    QMouseEvent moveAfterManagerDelete(QEvent::MouseMove,
                                       QPointF(10, 10),
                                       QPointF(10, 10),
                                       Qt::NoButton,
                                       Qt::LeftButton,
                                       Qt::NoModifier);
    QCoreApplication::sendEvent(qApp, &moveAfterManagerDelete);
}

void TestAntStressLifecycle::menuBarSurvivesHoveredActionDestruction()
{
    AntMenuBar menuBar;
    menuBar.resize(360, 32);
    auto* removedAction = new QAction(QStringLiteral("Removed"), &menuBar);
    menuBar.addAction(removedAction);
    menuBar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&menuBar));

    const QRect removedGeometry = menuBar.actionGeometry(removedAction);
    QVERIFY(removedGeometry.isValid());
    sendMouseEvent(&menuBar,
                   QEvent::MouseMove,
                   removedGeometry.center(),
                   Qt::NoButton,
                   Qt::NoButton);

    QPointer<QAction> removedActionGuard(removedAction);
    removedAction->deleteLater();
    drainDeferredDeletes();
    QVERIFY(removedActionGuard.isNull());

    QEvent leaveEvent(QEvent::Leave);
    QCoreApplication::sendEvent(&menuBar, &leaveEvent);

    auto* nextAction = new QAction(QStringLiteral("Next"), &menuBar);
    menuBar.addAction(nextAction);
    const QRect nextGeometry = menuBar.actionGeometry(nextAction);
    QVERIFY(nextGeometry.isValid());
    sendMouseEvent(&menuBar,
                   QEvent::MouseMove,
                   nextGeometry.center(),
                   Qt::NoButton,
                   Qt::NoButton);

    auto* synchronouslyDeletedAction = new QAction(QStringLiteral("Synchronous"), &menuBar);
    menuBar.addAction(synchronouslyDeletedAction);
    const QRect synchronousGeometry = menuBar.actionGeometry(synchronouslyDeletedAction);
    QVERIFY(synchronousGeometry.isValid());
    QPointer<QAction> synchronousGuard(synchronouslyDeletedAction);
    connect(&menuBar, &QMenuBar::hovered, &menuBar,
            [synchronouslyDeletedAction](QAction* hovered) {
        if (hovered == synchronouslyDeletedAction)
        {
            delete synchronouslyDeletedAction;
        }
    });
    sendMouseEvent(&menuBar,
                   QEvent::MouseMove,
                   synchronousGeometry.center(),
                   Qt::NoButton,
                   Qt::NoButton);
    QVERIFY(synchronousGuard.isNull());

    auto* directlyDeletedAction = new QAction(QStringLiteral("Direct action"), &menuBar);
    menuBar.addAction(directlyDeletedAction);
    const QRect directGeometry = menuBar.actionGeometry(directlyDeletedAction);
    QVERIFY(directGeometry.isValid());
    QPointer<QAction> directGuard(directlyDeletedAction);
    connect(directlyDeletedAction, &QAction::hovered, &menuBar,
            [directlyDeletedAction]() { delete directlyDeletedAction; });
    sendMouseEvent(&menuBar,
                   QEvent::MouseMove,
                   directGeometry.center(),
                   Qt::NoButton,
                   Qt::NoButton);
    QVERIFY(directGuard.isNull());
    QCoreApplication::sendEvent(&menuBar, &leaveEvent);
}

void TestAntStressLifecycle::menuBarPreservesNativeHoverSemantics()
{
    AntMenuBar menuBar;
    menuBar.resize(520, 32);

    auto* normalAction = menuBar.addAction(QStringLiteral("Normal"));
    auto* disabledAction = menuBar.addAction(QStringLiteral("Disabled"));
    disabledAction->setEnabled(false);
    auto* synthesizedAction = menuBar.addAction(QStringLiteral("Synthesized"));
    auto* recoveryAction = menuBar.addAction(QStringLiteral("Recovery"));

    menuBar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&menuBar));

    QSignalSpy menuHoveredSpy(&menuBar, &QMenuBar::hovered);
    QSignalSpy normalHoveredSpy(normalAction, &QAction::hovered);
    QSignalSpy disabledHoveredSpy(disabledAction, &QAction::hovered);
    QSignalSpy synthesizedHoveredSpy(synthesizedAction, &QAction::hovered);
    QSignalSpy recoveryHoveredSpy(recoveryAction, &QAction::hovered);

    const QRect normalGeometry = menuBar.actionGeometry(normalAction);
    const QRect disabledGeometry = menuBar.actionGeometry(disabledAction);
    const QRect synthesizedGeometry = menuBar.actionGeometry(synthesizedAction);
    const QRect recoveryGeometry = menuBar.actionGeometry(recoveryAction);
    QVERIFY(normalGeometry.isValid());
    QVERIFY(disabledGeometry.isValid());
    QVERIFY(synthesizedGeometry.isValid());
    QVERIFY(recoveryGeometry.isValid());

    sendMouseEvent(&menuBar,
                   QEvent::MouseMove,
                   normalGeometry.center(),
                   Qt::NoButton,
                   Qt::NoButton);
    QCOMPARE(normalHoveredSpy.count(), 1);
    QCOMPARE(menuHoveredSpy.count(), 1);

    sendMouseEvent(&menuBar,
                   QEvent::MouseMove,
                   disabledGeometry.center(),
                   Qt::NoButton,
                   Qt::NoButton);
    QCOMPARE(disabledHoveredSpy.count(), 0);
    QCOMPARE(menuHoveredSpy.count(), 1);

    const QPoint synthesizedPos = synthesizedGeometry.center();
    const QPoint synthesizedGlobalPos = menuBar.mapToGlobal(synthesizedPos);
    QMouseEvent synthesizedMove(QEvent::MouseMove,
                                QPointF(synthesizedPos),
                                QPointF(synthesizedPos),
                                QPointF(synthesizedGlobalPos),
                                Qt::NoButton,
                                Qt::NoButton,
                                Qt::NoModifier,
                                Qt::MouseEventSynthesizedBySystem);
    QCoreApplication::sendEvent(&menuBar, &synthesizedMove);
    QCOMPARE(synthesizedHoveredSpy.count(), 0);
    QCOMPARE(menuHoveredSpy.count(), 1);

    sendMouseEvent(&menuBar,
                   QEvent::MouseMove,
                   recoveryGeometry.center(),
                   Qt::NoButton,
                   Qt::NoButton);
    QCOMPARE(recoveryHoveredSpy.count(), 1);
    QCOMPARE(menuHoveredSpy.count(), 2);
}

void TestAntStressLifecycle::synchronousSignalDeletionStopsEventHandlers()
{
    auto* image = new AntImage;
    image->resize(120, 80);
    image->show();
    QPointer<AntImage> imageGuard(image);
    connect(image, &AntImage::clicked, image, [image]() { delete image; });
    sendMouseEvent(image,
                   QEvent::MouseButtonPress,
                   image->rect().center(),
                   Qt::LeftButton,
                   Qt::LeftButton);
    QVERIFY(imageGuard.isNull());

    QWidget previewHost;
    previewHost.resize(320, 200);
    previewHost.show();
    auto* previewImage = new AntImage(&previewHost);
    const QString previewSource = QStringLiteral(":/qt-ant-design/images/image-basic.png");
    QVERIFY(!QPixmap(previewSource).isNull());
    previewImage->setSrc(previewSource);
    previewImage->resize(160, 100);
    previewImage->show();
    QPointer<AntImage> previewGuard(previewImage);
    bool previewImageDeleted = false;
    QTimer::singleShot(0, &previewHost, [&previewGuard, &previewImageDeleted]() {
        delete previewGuard.data();
        previewImageDeleted = true;
    });
    QTimer::singleShot(1, &previewHost, []() {
        if (QWidget* modal = QApplication::activeModalWidget())
        {
            modal->close();
        }
    });
    sendMouseEvent(previewImage,
                   QEvent::MouseButtonPress,
                   previewImage->rect().center(),
                   Qt::LeftButton,
                   Qt::LeftButton);
    QVERIFY(previewImageDeleted);
    QVERIFY(previewGuard.isNull());

    auto* floatButton = new AntFloatButton;
    floatButton->setBackTop(true);
    floatButton->resize(floatButton->sizeHint());
    floatButton->show();
    QPointer<AntFloatButton> floatGuard(floatButton);
    connect(floatButton, &AntFloatButton::backTopClicked, floatButton,
            [floatButton]() { delete floatButton; });
    const QPoint floatCenter = floatButton->rect().center();
    sendMouseEvent(floatButton,
                   QEvent::MouseButtonPress,
                   floatCenter,
                   Qt::LeftButton,
                   Qt::LeftButton);
    sendMouseEvent(floatButton,
                   QEvent::MouseButtonRelease,
                   floatCenter,
                   Qt::LeftButton,
                   Qt::NoButton);
    QVERIFY(floatGuard.isNull());

    QWidget affixHost;
    affixHost.resize(240, 120);
    auto* affixedWidget = new QWidget(&affixHost);
    affixedWidget->resize(80, 24);
    auto* firstArea = new QScrollArea;
    firstArea->resize(240, 120);
    firstArea->show();
    auto* secondArea = new QScrollArea;
    secondArea->resize(240, 120);
    secondArea->show();
    auto* affix = new AntAffix;
    affix->setAffixedWidget(affixedWidget);
    affix->setScrollTarget(firstArea);
    affix->setOffsetTop(10000);
    QTRY_VERIFY(affix->isAffixed());
    QPointer<AntAffix> affixGuard(affix);
    connect(affix, &AntAffix::affixStateChanged, affix, [affix](bool affixed) {
        if (!affixed)
        {
            delete affix;
        }
    });
    affix->setScrollTarget(secondArea);
    QVERIFY(affixGuard.isNull());
    QCOMPARE(affixedWidget->parentWidget(), &affixHost);
    delete firstArea;
    delete secondArea;

    auto* dockManager = new AntDockManager;
    dockManager->resize(480, 320);
    auto* dock = new AntDockWidget(QStringLiteral("Context"));
    dock->setWidget(new QWidget);
    dockManager->addDockWidget(Qt::LeftDockWidgetArea, dock);
    dockManager->show();
    QWidget* titleBar = dock->titleBarWidget();
    QVERIFY(titleBar != nullptr);
    QPointer<AntDockManager> managerGuard(dockManager);
    QPointer<AntDockWidget> dockGuard(dock);
    connect(dockManager, &AntDockManager::dockWidgetContextMenuRequested,
            dockManager, [dockManager](AntDockWidget*, const QPoint&) {
        delete dockManager;
    });
    const QPoint titlePoint = titleBar->rect().center();
    QContextMenuEvent contextEvent(QContextMenuEvent::Mouse,
                                   titlePoint,
                                   titleBar->mapToGlobal(titlePoint));
    QCoreApplication::sendEvent(titleBar, &contextEvent);
    QTRY_VERIFY(managerGuard.isNull());
    QVERIFY(dockGuard.isNull());
}

QTEST_MAIN(TestAntStressLifecycle)
#include "TestAntStressLifecycle.moc"
