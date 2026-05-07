#include <QPalette>
#include <QPlainTextEdit>
#include <QSignalSpy>
#include <QTest>
#include <QToolButton>
#include "core/AntTheme.h"
#include "widgets/AntApp.h"
#include "widgets/AntConfigProvider.h"
#include "widgets/AntForm.h"
#include "widgets/AntLog.h"
#include "widgets/AntMasonry.h"
#include "widgets/AntPlainTextEdit.h"
#include "widgets/AntScrollArea.h"
#include "widgets/AntScrollBar.h"
#include "widgets/AntSplitter.h"
#include "widgets/AntStatusBar.h"
#include "widgets/AntToolButton.h"
#include "widgets/AntToolBar.h"
#include "widgets/AntMenuBar.h"
#include "widgets/AntDockWidget.h"
#include "widgets/AntWidget.h"
#include "widgets/AntWindow.h"
#include "widgets/AntColorPicker.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#endif

class TestAntQtExtensions : public QObject
{
    Q_OBJECT
private slots:
    void app();
    void configProvider();
    void formItem();
    void form();
    void formList();
    void log();
    void masonry();
    void plainTextEdit();
    void scrollArea();
    void scrollBar();
    void splitter();
    void statusBar();
    void toolButton();
    void toolBar();
    void menuBar();
    void dockWidget();
    void widget();
    void window();
    void windowTitleBarButtonsHandleChildDeliveredClicks();
    void windowTitleBarButtonsTriggerOnRelease();
    void windowNativeHitTestSupportsSnapZones();
    void colorPicker();
};

void TestAntQtExtensions::app()
{
    auto* root = new QWidget;
    auto* w = new AntApp(root);
    QCOMPARE(w->rootWidget(), root);
}

void TestAntQtExtensions::configProvider()
{
    auto* w = new AntConfigProvider;
    QCOMPARE(w->themeMode(), Ant::ThemeMode::Default);
    QCOMPARE(w->fontSize(), 14);
    QCOMPARE(w->borderRadius(), 6);

    QSignalSpy themeSpy(w, &AntConfigProvider::themeModeChanged);
    w->setThemeMode(Ant::ThemeMode::Dark);
    QCOMPARE(w->themeMode(), Ant::ThemeMode::Dark);
    QCOMPARE(themeSpy.count(), 1);

    QSignalSpy colorSpy(w, &AntConfigProvider::primaryColorChanged);
    w->setPrimaryColor(Qt::blue);
    QCOMPARE(w->primaryColor(), QColor(Qt::blue));
    QCOMPARE(colorSpy.count(), 1);

    QSignalSpy fontSpy(w, &AntConfigProvider::fontSizeChanged);
    w->setFontSize(16);
    QCOMPARE(w->fontSize(), 16);
    QCOMPARE(fontSpy.count(), 1);

    QSignalSpy radiusSpy(w, &AntConfigProvider::borderRadiusChanged);
    w->setBorderRadius(8);
    QCOMPARE(w->borderRadius(), 8);
    QCOMPARE(radiusSpy.count(), 1);
}

void TestAntQtExtensions::formItem()
{
    auto* w = new AntFormItem;
    QCOMPARE(w->label(), QString());
    QCOMPARE(w->helpText(), QString());
    QCOMPARE(w->extra(), QString());
    QCOMPARE(w->isRequired(), false);
    QCOMPARE(w->colon(), true);
    QCOMPARE(w->validateStatus(), Ant::Status::Normal);

    QSignalSpy labelSpy(w, &AntFormItem::labelChanged);
    w->setLabel("Username");
    QCOMPARE(w->label(), "Username");
    QCOMPARE(labelSpy.count(), 1);

    QSignalSpy helpSpy(w, &AntFormItem::helpTextChanged);
    w->setHelpText("Enter your username");
    QCOMPARE(w->helpText(), "Enter your username");
    QCOMPARE(helpSpy.count(), 1);

    QSignalSpy extraSpy(w, &AntFormItem::extraChanged);
    w->setExtra("Required");
    QCOMPARE(w->extra(), "Required");
    QCOMPARE(extraSpy.count(), 1);

    QSignalSpy reqSpy(w, &AntFormItem::requiredChanged);
    w->setRequired(true);
    QCOMPARE(w->isRequired(), true);
    QCOMPARE(reqSpy.count(), 1);

    QSignalSpy colonSpy(w, &AntFormItem::colonChanged);
    w->setColon(false);
    QCOMPARE(w->colon(), false);
    QCOMPARE(colonSpy.count(), 1);

    QSignalSpy statusSpy(w, &AntFormItem::validateStatusChanged);
    w->setValidateStatus(Ant::Status::Error);
    QCOMPARE(w->validateStatus(), Ant::Status::Error);
    QCOMPARE(statusSpy.count(), 1);
}

void TestAntQtExtensions::form()
{
    auto* w = new AntForm;
    QCOMPARE(w->formLayout(), Ant::FormLayout::Horizontal);
    QCOMPARE(w->labelAlign(), Ant::FormLabelAlign::Right);
    QCOMPARE(w->colon(), true);
    QCOMPARE(w->requiredMark(), true);
    QCOMPARE(w->labelWidth(), 96);
    QCOMPARE(w->itemSpacing(), 16);

    QSignalSpy layoutSpy(w, &AntForm::formLayoutChanged);
    w->setFormLayout(Ant::FormLayout::Vertical);
    QCOMPARE(w->formLayout(), Ant::FormLayout::Vertical);
    QCOMPARE(layoutSpy.count(), 1);

    QSignalSpy alignSpy(w, &AntForm::labelAlignChanged);
    w->setLabelAlign(Ant::FormLabelAlign::Left);
    QCOMPARE(w->labelAlign(), Ant::FormLabelAlign::Left);
    QCOMPARE(alignSpy.count(), 1);

    QSignalSpy colonSpy(w, &AntForm::colonChanged);
    w->setColon(false);
    QCOMPARE(w->colon(), false);
    QCOMPARE(colonSpy.count(), 1);

    QSignalSpy markSpy(w, &AntForm::requiredMarkChanged);
    w->setRequiredMark(false);
    QCOMPARE(w->requiredMark(), false);
    QCOMPARE(markSpy.count(), 1);

    QSignalSpy widthSpy(w, &AntForm::labelWidthChanged);
    w->setLabelWidth(120);
    QCOMPARE(w->labelWidth(), 120);
    QCOMPARE(widthSpy.count(), 1);

    QSignalSpy spacingSpy(w, &AntForm::itemSpacingChanged);
    w->setItemSpacing(24);
    QCOMPARE(w->itemSpacing(), 24);
    QCOMPARE(spacingSpy.count(), 1);

    auto* item = new AntFormItem;
    item->setLabel("Name");
    w->addItem(item);
    QCOMPARE(w->items().size(), 1);

    w->addItem("Email", new QWidget, true);
    QCOMPARE(w->items().size(), 2);

    w->clearItems();
    QCOMPARE(w->items().size(), 0);
}

void TestAntQtExtensions::formList()
{
    auto* w = new AntFormList;
    QCOMPARE(w->minCount(), 0);
    QCOMPARE(w->maxCount(), 0);
    QCOMPARE(w->count(), 0);

    QSignalSpy minSpy(w, &AntFormList::minCountChanged);
    w->setMinCount(1);
    QCOMPARE(w->minCount(), 1);
    QCOMPARE(minSpy.count(), 1);

    QSignalSpy maxSpy(w, &AntFormList::maxCountChanged);
    w->setMaxCount(5);
    QCOMPARE(w->maxCount(), 5);
    QCOMPARE(maxSpy.count(), 1);
}

void TestAntQtExtensions::log()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    auto* w = new AntLog;
    QCOMPARE(w->maxEntries(), 5000);
    QCOMPARE(w->autoScroll(), true);
    auto* view = w->findChild<QPlainTextEdit*>();
    QVERIFY(view != nullptr);
    QCOMPARE(view->palette().color(QPalette::Base), antTheme->tokens().colorFillQuaternary);

    QSignalSpy maxSpy(w, &AntLog::maxEntriesChanged);
    w->setMaxEntries(1000);
    QCOMPARE(w->maxEntries(), 1000);
    QCOMPARE(maxSpy.count(), 1);

    QSignalSpy scrollSpy(w, &AntLog::autoScrollChanged);
    w->setAutoScroll(false);
    QCOMPARE(w->autoScroll(), false);
    QCOMPARE(scrollSpy.count(), 1);

    w->info("test message");
    w->warning("warning message");
    w->error("error message");

    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCOMPARE(view->palette().color(QPalette::Base), antTheme->tokens().colorFillQuaternary);

    w->clear();
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::masonry()
{
    auto* w = new AntMasonry;
    QCOMPARE(w->columns(), 3);
    QCOMPARE(w->spacing(), 8);

    QSignalSpy columnsSpy(w, &AntMasonry::columnsChanged);
    w->setColumns(2);
    QCOMPARE(w->columns(), 2);
    QCOMPARE(columnsSpy.count(), 1);

    QSignalSpy spacingSpy(w, &AntMasonry::spacingChanged);
    w->setSpacing(12);
    QCOMPARE(w->spacing(), 12);
    QCOMPARE(spacingSpy.count(), 1);

    w->resize(212, 200);
    auto* first = new QWidget;
    first->setMinimumHeight(120);
    auto* second = new QWidget;
    second->setMinimumHeight(60);
    auto* third = new QWidget;
    third->setMinimumHeight(80);

    w->addWidget(first);
    w->addWidget(second);
    w->addWidget(third);

    QCOMPARE(first->geometry(), QRect(0, 0, 100, 120));
    QCOMPARE(second->geometry(), QRect(112, 0, 100, 60));
    QCOMPARE(third->geometry(), QRect(112, 72, 100, 80));
    QCOMPARE(w->minimumHeight(), 152);

    w->clear();
    QCOMPARE(w->minimumHeight(), 0);
}

void TestAntQtExtensions::plainTextEdit()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    auto* w = new AntPlainTextEdit;
    QCOMPARE(w->variant(), Ant::Variant::Outlined);

    QSignalSpy varSpy(w, &AntPlainTextEdit::variantChanged);
    w->setVariant(Ant::Variant::Filled);
    QCOMPARE(w->variant(), Ant::Variant::Filled);
    QCOMPARE(varSpy.count(), 1);

    QSignalSpy phSpy(w, &AntPlainTextEdit::placeholderTextChanged);
    w->setPlaceholderText("Type here...");
    QCOMPARE(w->placeholderText(), "Type here...");
    QCOMPARE(phSpy.count(), 1);

    w->setEnabled(false);
    QCOMPARE(w->palette().color(QPalette::Disabled, QPalette::Text), antTheme->tokens().colorTextDisabled);

    auto* resizable = new AntPlainTextEdit;
    resizable->setFixedSize(180, 80);
    resizable->show();
    QVERIFY(QTest::qWaitForWindowExposed(resizable));
    const QPoint grip = resizable->viewport()->mapFrom(resizable, QPoint(resizable->width() - 4, resizable->height() - 4));
    QTest::mousePress(resizable->viewport(), Qt::LeftButton, Qt::NoModifier, grip);
    QTest::mouseMove(resizable->viewport(), grip + QPoint(32, 18));
    QTest::mouseRelease(resizable->viewport(), Qt::LeftButton, Qt::NoModifier, grip + QPoint(32, 18));
    QCOMPARE(resizable->size(), QSize(212, 98));

    auto* w2 = new AntPlainTextEdit("Initial text");
    QCOMPARE(w2->toPlainText(), "Initial text");
}

void TestAntQtExtensions::scrollArea()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    auto* w = new AntScrollArea;
    QCOMPARE(w->autoHideScrollBar(), true);
    QCOMPARE(w->isGestureEnabled(), true);

    QSignalSpy hideSpy(w, &AntScrollArea::autoHideScrollBarChanged);
    w->setAutoHideScrollBar(false);
    QCOMPARE(w->autoHideScrollBar(), false);
    QCOMPARE(hideSpy.count(), 1);

    QSignalSpy gestureSpy(w, &AntScrollArea::enableGestureChanged);
    w->setEnableGesture(false);
    QCOMPARE(w->isGestureEnabled(), false);
    QCOMPARE(gestureSpy.count(), 1);

    auto* content = new QWidget;
    w->setWidget(content);
    QCOMPARE(w->viewport()->palette().color(QPalette::Base), antTheme->tokens().colorBgContainer);
    QCOMPARE(content->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);

    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCOMPARE(w->viewport()->palette().color(QPalette::Base), antTheme->tokens().colorBgContainer);
    QCOMPARE(content->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::scrollBar()
{
    auto* w = new AntScrollBar;
    QCOMPARE(w->autoHide(), true);

    QSignalSpy hideSpy(w, &AntScrollBar::autoHideChanged);
    w->setAutoHide(false);
    QCOMPARE(w->autoHide(), false);
    QCOMPARE(hideSpy.count(), 1);

    auto* w2 = new AntScrollBar(Qt::Horizontal);
    QCOMPARE(w2->orientation(), Qt::Horizontal);
}

void TestAntQtExtensions::splitter()
{
    auto* w = new AntSplitter;
    QCOMPARE(w->orientation(), Qt::Horizontal);
    QCOMPARE(w->childrenCollapsible(), false);
    QCOMPARE(w->handleWidth(), 4);

    auto* w2 = new AntSplitter(Qt::Vertical);
    QCOMPARE(w2->orientation(), Qt::Vertical);

    auto* c1 = new QWidget;
    auto* c2 = new QWidget;
    w->addWidget(c1);
    w->addWidget(c2);
    QCOMPARE(w->count(), 2);
}

void TestAntQtExtensions::statusBar()
{
    auto* w = new AntStatusBar;
    QCOMPARE(w->message(), QString());
    QCOMPARE(w->hasSizeGrip(), true);
    QCOMPARE(w->itemCount(), 0);
    QCOMPARE(w->permanentItemCount(), 0);

    QSignalSpy msgSpy(w, &AntStatusBar::messageChanged);
    w->setMessage("Ready");
    QCOMPARE(w->message(), "Ready");
    QCOMPARE(msgSpy.count(), 1);

    QSignalSpy gripSpy(w, &AntStatusBar::sizeGripChanged);
    w->setSizeGrip(false);
    QCOMPARE(w->hasSizeGrip(), false);
    QCOMPARE(gripSpy.count(), 1);

    w->addItem("Item 1");
    QCOMPARE(w->itemCount(), 1);
    w->addPermanentItem("Perm 1");
    QCOMPARE(w->permanentItemCount(), 1);

    auto item = w->itemAt(0);
    QCOMPARE(item.text, "Item 1");
}

void TestAntQtExtensions::toolButton()
{
    auto* w = new AntToolButton;
    QCOMPARE(w->buttonType(), Ant::ButtonType::Default);
    QCOMPARE(w->buttonSize(), Ant::Size::Middle);
    QCOMPARE(w->isDanger(), false);
    QCOMPARE(w->isLoading(), false);
    QCOMPARE(w->arrowRotation(), 0.0);

    QSignalSpy typeSpy(w, &AntToolButton::buttonTypeChanged);
    w->setButtonType(Ant::ButtonType::Primary);
    QCOMPARE(w->buttonType(), Ant::ButtonType::Primary);
    QCOMPARE(typeSpy.count(), 1);

    QSignalSpy sizeSpy(w, &AntToolButton::buttonSizeChanged);
    w->setButtonSize(Ant::Size::Large);
    QCOMPARE(w->buttonSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy dangerSpy(w, &AntToolButton::dangerChanged);
    w->setDanger(true);
    QCOMPARE(w->isDanger(), true);
    QCOMPARE(dangerSpy.count(), 1);

    QSignalSpy loadSpy(w, &AntToolButton::loadingChanged);
    w->setLoading(true);
    QCOMPARE(w->isLoading(), true);
    QCOMPARE(loadSpy.count(), 1);

    QSignalSpy arrowSpy(w, &AntToolButton::arrowRotationChanged);
    w->setArrowRotation(90.0);
    QCOMPARE(w->arrowRotation(), 90.0);
    QCOMPARE(arrowSpy.count(), 1);

    auto* w2 = new AntToolButton("Click");
    QCOMPARE(w2->text(), "Click");
}

void TestAntQtExtensions::toolBar()
{
    auto* w = new AntToolBar;
    QVERIFY(w != nullptr);
    auto* action = w->addAction("New");
    auto* button = qobject_cast<QToolButton*>(w->widgetForAction(action));
    QVERIFY(button != nullptr);
    QVERIFY(button->property("antToolBarButton").toBool());
    QCOMPARE(button->style(), w->style());

    auto* w2 = new AntToolBar("My Toolbar");
    QVERIFY(w2 != nullptr);
}

void TestAntQtExtensions::menuBar()
{
    auto* w = new AntMenuBar;
    QVERIFY(w->actions().isEmpty());

    auto* menu = w->addMenu("File");
    QVERIFY(menu != nullptr);
    QVERIFY(!w->actions().isEmpty());
}

void TestAntQtExtensions::dockWidget()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    auto* w = new AntDockWidget;
    QCOMPARE(w->windowTitle(), QString());

    auto* w2 = new AntDockWidget("Properties");
    QCOMPARE(w2->windowTitle(), "Properties");

    auto* content = new QWidget;
    w2->setWidget(content);
    QCOMPARE(content->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);

    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCOMPARE(w2->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);
    QCOMPARE(content->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::widget()
{
    auto* w = new AntWidget;
    QVERIFY(w->currentTheme() == Ant::ThemeMode::Default ||
            w->currentTheme() == Ant::ThemeMode::Dark);
}

void TestAntQtExtensions::window()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    auto* w = new AntWindow;
    QCOMPARE(w->isMaximized(), false);
    QCOMPARE(w->TitleBarHeight, 40);
    QCOMPARE(w->TitleBarButtonWidth, 46);
    QCOMPARE(w->isAlwaysOnTop(), false);
    QCOMPARE(w->isPinButtonVisible(), true);
    QCOMPARE(w->isThemeButtonVisible(), true);
    QCOMPARE(w->isMinimizeButtonVisible(), true);
    QCOMPARE(w->isMaximizeButtonVisible(), true);
    QCOMPARE(w->isCloseButtonVisible(), true);
    QCOMPARE(w->isTitleBarButtonVisible(AntWindow::TitleBarButton::Pin), true);
    QCOMPARE(w->isTitleBarButtonVisible(AntWindow::TitleBarButton::Theme), true);
    QCOMPARE(w->isTitleBarButtonVisible(AntWindow::TitleBarButton::Minimize), true);
    QCOMPARE(w->isTitleBarButtonVisible(AntWindow::TitleBarButton::Maximize), true);
    QCOMPARE(w->isTitleBarButtonVisible(AntWindow::TitleBarButton::Close), true);

    QSignalSpy titleSpy(w, &AntWindow::windowTitleChanged);
    w->setWindowTitle("Example");
    QCOMPARE(w->windowTitle(), "Example");
    QCOMPARE(titleSpy.count(), 1);

    w->resize(640, 400);
    QVERIFY(!w->titleBarButtonRect(AntWindow::TitleBarButton::Pin).isNull());
    QVERIFY(!w->titleBarButtonRect(AntWindow::TitleBarButton::Theme).isNull());
    QVERIFY(!w->titleBarButtonRect(AntWindow::TitleBarButton::Minimize).isNull());
    QVERIFY(!w->titleBarButtonRect(AntWindow::TitleBarButton::Maximize).isNull());
    QVERIFY(!w->titleBarButtonRect(AntWindow::TitleBarButton::Close).isNull());

    QSignalSpy visibilitySpy(w, &AntWindow::titleBarButtonVisibilityChanged);
    QSignalSpy pinVisibilitySpy(w, &AntWindow::pinButtonVisibleChanged);
    w->setPinButtonVisible(false);
    QCOMPARE(w->isPinButtonVisible(), false);
    QCOMPARE(w->isTitleBarButtonVisible(AntWindow::TitleBarButton::Pin), false);
    QVERIFY(w->titleBarButtonRect(AntWindow::TitleBarButton::Pin).isNull());
    QCOMPARE(pinVisibilitySpy.count(), 1);
    QCOMPARE(visibilitySpy.count(), 1);
    w->setTitleBarButtonVisible(AntWindow::TitleBarButton::Pin, true);
    QCOMPARE(w->isPinButtonVisible(), true);
    QCOMPARE(pinVisibilitySpy.count(), 2);
    QCOMPARE(visibilitySpy.count(), 2);

    w->setThemeButtonVisible(false);
    w->setMinimizeButtonVisible(false);
    w->setMaximizeButtonVisible(false);
    w->setCloseButtonVisible(false);
    QCOMPARE(w->isThemeButtonVisible(), false);
    QCOMPARE(w->isMinimizeButtonVisible(), false);
    QCOMPARE(w->isMaximizeButtonVisible(), false);
    QCOMPARE(w->isCloseButtonVisible(), false);
    QVERIFY(w->titleBarButtonRect(AntWindow::TitleBarButton::Theme).isNull());
    QVERIFY(w->titleBarButtonRect(AntWindow::TitleBarButton::Minimize).isNull());
    QVERIFY(w->titleBarButtonRect(AntWindow::TitleBarButton::Maximize).isNull());
    QVERIFY(w->titleBarButtonRect(AntWindow::TitleBarButton::Close).isNull());
    w->setTitleBarButtonVisible(AntWindow::TitleBarButton::Theme, true);
    w->setTitleBarButtonVisible(AntWindow::TitleBarButton::Minimize, true);
    w->setTitleBarButtonVisible(AntWindow::TitleBarButton::Maximize, true);
    w->setTitleBarButtonVisible(AntWindow::TitleBarButton::Close, true);

    QSignalSpy alwaysOnTopSpy(w, &AntWindow::alwaysOnTopChanged);
    w->setAlwaysOnTop(true);
    QCOMPARE(w->isAlwaysOnTop(), true);
    QVERIFY(w->windowFlags().testFlag(Qt::WindowStaysOnTopHint));
    QCOMPARE(alwaysOnTopSpy.count(), 1);
    w->setAlwaysOnTop(false);
    QCOMPARE(w->isAlwaysOnTop(), false);
    QVERIFY(!w->windowFlags().testFlag(Qt::WindowStaysOnTopHint));
    QCOMPARE(alwaysOnTopSpy.count(), 2);

    const Ant::ThemeMode beforeClickMode = antTheme->themeMode();
    QTest::mouseClick(w, Qt::LeftButton, Qt::NoModifier, w->titleBarButtonRect(AntWindow::TitleBarButton::Theme).center());
    QCOMPARE(antTheme->themeMode(),
             beforeClickMode == Ant::ThemeMode::Dark ? Ant::ThemeMode::Default : Ant::ThemeMode::Dark);
    QTest::mouseClick(w, Qt::LeftButton, Qt::NoModifier, w->titleBarButtonRect(AntWindow::TitleBarButton::Pin).center());
    QCOMPARE(w->isAlwaysOnTop(), true);
    w->setAlwaysOnTop(false);

    auto* content = new QWidget;
    w->setCentralWidget(content);
    QCOMPARE(content->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);

    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCOMPARE(content->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::windowTitleBarButtonsHandleChildDeliveredClicks()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    AntWindow window;
    window.resize(640, 400);
    window.setCentralWidget(new QWidget);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QPoint themeGlobal = window.mapToGlobal(window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center());
    QWidget* themeTarget = window.centralWidget();
    QVERIFY(themeTarget != nullptr);
    const Ant::ThemeMode beforeClickMode = antTheme->themeMode();
    QTest::mouseClick(themeTarget, Qt::LeftButton, Qt::NoModifier, themeTarget->mapFromGlobal(themeGlobal));
    QTRY_COMPARE(antTheme->themeMode(),
                 beforeClickMode == Ant::ThemeMode::Dark ? Ant::ThemeMode::Default : Ant::ThemeMode::Dark);

    const QPoint pinGlobal = window.mapToGlobal(window.titleBarButtonRect(AntWindow::TitleBarButton::Pin).center());
    QWidget* pinTarget = window.centralWidget();
    QVERIFY(pinTarget != nullptr);
    QTest::mouseClick(pinTarget, Qt::LeftButton, Qt::NoModifier, pinTarget->mapFromGlobal(pinGlobal));
    QTRY_COMPARE(window.isAlwaysOnTop(), true);

    window.setAlwaysOnTop(false);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::windowTitleBarButtonsTriggerOnRelease()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    AntWindow window;
    window.resize(640, 400);
    const QPoint themePoint = window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center();
    QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, themePoint);
    QCOMPARE(antTheme->themeMode(), Ant::ThemeMode::Default);
    QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, themePoint);
    QCOMPARE(antTheme->themeMode(), Ant::ThemeMode::Dark);

    const QPoint pinPoint = window.titleBarButtonRect(AntWindow::TitleBarButton::Pin).center();
    QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, pinPoint);
    QCOMPARE(window.isAlwaysOnTop(), false);
    QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, pinPoint);
    QCOMPARE(window.isAlwaysOnTop(), true);

    window.setAlwaysOnTop(false);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::windowNativeHitTestSupportsSnapZones()
{
#ifndef Q_OS_WIN
    QSKIP("Windows native hit testing is only available on Windows.");
#else
    class NativeHitTestWindow : public AntWindow
    {
    public:
        using AntWindow::nativeEvent;
    };

    NativeHitTestWindow window;
    window.resize(640, 420);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const HWND hwnd = reinterpret_cast<HWND>(window.winId());
    QVERIFY(hwnd != nullptr);

    auto hitTest = [&](const QPoint& localPos) -> qintptr {
        MSG msg{};
        msg.hwnd = hwnd;
        msg.message = WM_NCHITTEST;
        const QPoint globalPos = window.mapToGlobal(localPos);
        msg.lParam = MAKELPARAM(globalPos.x(), globalPos.y());
        qintptr result = 0;
        if (!window.nativeEvent("windows_generic_MSG", &msg, &result))
        {
            return -1;
        }
        return result;
    };

    QCOMPARE(hitTest(QPoint(2, 2)), static_cast<qintptr>(HTTOPLEFT));
    QCOMPARE(hitTest(QPoint(window.width() - 3, 2)), static_cast<qintptr>(HTTOPRIGHT));
    QCOMPARE(hitTest(QPoint(2, window.height() - 3)), static_cast<qintptr>(HTBOTTOMLEFT));
    QCOMPARE(hitTest(QPoint(window.width() - 3, window.height() - 3)), static_cast<qintptr>(HTBOTTOMRIGHT));
    QCOMPARE(hitTest(QPoint(2, AntWindow::TitleBarHeight + 30)), static_cast<qintptr>(HTLEFT));
    QCOMPARE(hitTest(QPoint(window.width() - 3, AntWindow::TitleBarHeight + 30)), static_cast<qintptr>(HTRIGHT));
    QCOMPARE(hitTest(QPoint(80, 2)), static_cast<qintptr>(HTTOP));
    QCOMPARE(hitTest(QPoint(80, window.height() - 3)), static_cast<qintptr>(HTBOTTOM));
    QCOMPARE(hitTest(QPoint(80, AntWindow::TitleBarHeight / 2)), static_cast<qintptr>(HTCAPTION));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Close).center()),
             static_cast<qintptr>(HTCLIENT));

    window.showMaximized();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QCoreApplication::processEvents();

    QCOMPARE(hitTest(QPoint(80, AntWindow::TitleBarHeight / 2)), static_cast<qintptr>(HTCAPTION));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Close).center()),
             static_cast<qintptr>(HTCLIENT));
#endif
}

void TestAntQtExtensions::colorPicker()
{
    auto* w = new AntColorPicker;
    QCOMPARE(w->currentColor(), QColor(Qt::white));
    QCOMPARE(w->showText(), false);

    QSignalSpy colorSpy(w, &AntColorPicker::currentColorChanged);
    w->setCurrentColor(Qt::red);
    QCOMPARE(w->currentColor(), QColor(Qt::red));
    QCOMPARE(colorSpy.count(), 1);

    QSignalSpy textSpy(w, &AntColorPicker::showTextChanged);
    w->setShowText(true);
    QCOMPARE(w->showText(), true);
    QCOMPARE(textSpy.count(), 1);

    QSignalSpy openSpy(w, &AntColorPicker::openChanged);
    w->setOpen(true);
    QCOMPARE(w->isOpen(), true);
    QCOMPARE(openSpy.count(), 1);
    w->setOpen(false);
    QCOMPARE(w->isOpen(), false);
    QCOMPARE(openSpy.count(), 2);

    auto* w2 = new AntColorPicker(Qt::blue);
    QCOMPARE(w2->currentColor(), QColor(Qt::blue));
}

QTEST_MAIN(TestAntQtExtensions)
#include "TestAntQtExtensions.moc"
