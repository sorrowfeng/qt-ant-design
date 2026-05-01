#include <QCoreApplication>
#include <QDate>
#include <QElapsedTimer>
#include <QEvent>
#include <QImage>
#include <QPointer>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>
#include <QTime>
#include <QVBoxLayout>
#include <QWidget>

#include <functional>

#include "core/AntTheme.h"
#include "widgets/AntButton.h"
#include "widgets/AntCascader.h"
#include "widgets/AntColorPicker.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntDropdown.h"
#include "widgets/AntInput.h"
#include "widgets/AntInputNumber.h"
#include "widgets/AntMessage.h"
#include "widgets/AntNotification.h"
#include "widgets/AntSelect.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntTabs.h"
#include "widgets/AntTimePicker.h"
#include "widgets/AntTreeSelect.h"

class TestAntStressLifecycle : public QObject
{
    Q_OBJECT

private slots:
    void repeatedThemeSwitchesKeepCompositeSurfaceAlive();
    void repeatedOwnedPopupOpenCloseCyclesReleaseFrames();
    void transientFeedbackBurstClosesCleanly();
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

QTEST_MAIN(TestAntStressLifecycle)
#include "TestAntStressLifecycle.moc"
