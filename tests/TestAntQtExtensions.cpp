#include <QPalette>
#include <QApplication>
#include <QPainter>
#include <QCoreApplication>
#include <QContextMenuEvent>
#include <QEventLoop>
#include <QGraphicsOpacityEffect>
#include <QGuiApplication>
#include <QImage>
#include <QLabel>
#include <QMargins>
#include <QMainWindow>
#include <QMetaType>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QAction>
#include <QHideEvent>
#include <QSignalSpy>
#include <QTabBar>
#include <QTabWidget>
#include <QHoverEvent>
#include <QTest>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWindow>
#include "core/AntTheme.h"
#include "core/AntWave.h"
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
#include "widgets/AntRibbon.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntToolButton.h"
#include "widgets/AntToolBar.h"
#include "widgets/AntMenu.h"
#include "widgets/AntMenuBar.h"
#include "widgets/AntDockManager.h"
#include "widgets/AntDockWidget.h"
#include "widgets/AntWidget.h"
#include "widgets/AntWindow.h"
#include "widgets/AntColorPicker.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#endif

namespace
{
bool colorNearForExtensionTest(const QColor& actual, const QColor& expected, int tolerance = 12)
{
    return std::abs(actual.red() - expected.red()) <= tolerance &&
           std::abs(actual.green() - expected.green()) <= tolerance &&
           std::abs(actual.blue() - expected.blue()) <= tolerance;
}

QString colorStringForExtensionTest(const QColor& color)
{
    return QStringLiteral("rgba(%1,%2,%3,%4)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(color.alpha());
}

QImage renderForExtensionTest(QWidget* widget)
{
    QImage image(widget->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    widget->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    return image;
}

QWidget* topLevelWidgetForExtensionTest(const QString& objectName)
{
    const auto widgets = QApplication::topLevelWidgets();
    for (QWidget* widget : widgets)
    {
        if (widget && widget->objectName() == objectName)
        {
            return widget;
        }
    }
    return nullptr;
}

QLabel* ribbonTitleLabelForExtensionTest(AntRibbonGroup* group, const QString& text)
{
    const auto labels = group->findChildren<QLabel*>();
    for (QLabel* label : labels)
    {
        if (label && label->text() == text)
        {
            return label;
        }
    }
    return nullptr;
}

QTabWidget* dockAreaForExtensionTest(AntDockWidget* dock)
{
    QWidget* parent = dock ? dock->parentWidget() : nullptr;
    while (parent)
    {
        if (auto* tabWidget = qobject_cast<QTabWidget*>(parent))
        {
            return tabWidget;
        }
        parent = parent->parentWidget();
    }
    return nullptr;
}

QTabBar* dockTabBarForExtensionTest(AntDockWidget* dock)
{
    QTabWidget* area = dockAreaForExtensionTest(dock);
    return area ? area->findChild<QTabBar*>() : nullptr;
}

QPoint dockTabCenterForExtensionTest(AntDockWidget* dock)
{
    QTabWidget* area = dockAreaForExtensionTest(dock);
    QTabBar* tabBar = dockTabBarForExtensionTest(dock);
    if (!area || !tabBar) return QPoint();

    const int tab = area->indexOf(dock);
    return tab >= 0 ? tabBar->tabRect(tab).center() : QPoint();
}

#ifdef Q_OS_WIN
using RtlGetVersionFn = LONG(WINAPI*)(OSVERSIONINFOW*);

int windowsBuildNumberForTest()
{
    if (HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll"))
    {
        auto* rtlGetVersion = reinterpret_cast<RtlGetVersionFn>(::GetProcAddress(ntdll, "RtlGetVersion"));
        if (rtlGetVersion)
        {
            OSVERSIONINFOW version{};
            version.dwOSVersionInfoSize = sizeof(version);
            if (rtlGetVersion(&version) == 0)
            {
                return static_cast<int>(version.dwBuildNumber);
            }
        }
    }
    return 0;
}

bool supportsNativeCaptionSnapLayoutsForTest()
{
    constexpr int kWindows11Build = 22000;
    return windowsBuildNumberForTest() >= kWindows11Build;
}

bool nativeMouseInputAvailableForExtensionTest()
{
    const QString platform = QGuiApplication::platformName().toLower();
    return !platform.contains(QStringLiteral("offscreen")) &&
           !platform.contains(QStringLiteral("minimal"));
}

HWND rootHwndForExtensionTest(QWidget* widget)
{
    if (!widget)
    {
        return nullptr;
    }

    QWidget* root = widget->window();
    if (!root)
    {
        return nullptr;
    }
    if (!root->windowHandle())
    {
        root->winId();
    }

    HWND hwnd = reinterpret_cast<HWND>(root->winId());
    return hwnd ? ::GetAncestor(hwnd, GA_ROOT) : nullptr;
}

bool bringWidgetToFrontForExtensionTest(QWidget* widget)
{
    if (!nativeMouseInputAvailableForExtensionTest())
    {
        return false;
    }

    QWidget* root = widget ? widget->window() : nullptr;
    HWND hwnd = rootHwndForExtensionTest(widget);
    if (!root || !hwnd)
    {
        return false;
    }

    if (::IsIconic(hwnd))
    {
        ::ShowWindow(hwnd, SW_RESTORE);
    }
    else
    {
        ::ShowWindow(hwnd, SW_SHOWNORMAL);
    }

    root->show();
    root->raise();
    root->activateWindow();
    ::SetWindowPos(hwnd,
                   HWND_TOPMOST,
                   0,
                   0,
                   0,
                   0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    const DWORD currentThreadId = ::GetCurrentThreadId();
    DWORD foregroundThreadId = currentThreadId;
    if (HWND foreground = ::GetForegroundWindow())
    {
        foregroundThreadId = ::GetWindowThreadProcessId(foreground, nullptr);
    }
    const BOOL attached = foregroundThreadId != currentThreadId
        ? ::AttachThreadInput(currentThreadId, foregroundThreadId, TRUE)
        : FALSE;
    ::SetForegroundWindow(hwnd);
    ::SetActiveWindow(hwnd);
    ::SetFocus(hwnd);
    if (attached)
    {
        ::AttachThreadInput(currentThreadId, foregroundThreadId, FALSE);
    }
    QTest::qWait(80);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 80);
    HWND foreground = ::GetForegroundWindow();
    return ::IsWindowVisible(hwnd) && (!foreground || ::GetAncestor(foreground, GA_ROOT) == hwnd);
}

void releaseTopMostForExtensionTest(QWidget* widget)
{
    if (HWND hwnd = rootHwndForExtensionTest(widget))
    {
        ::SetWindowPos(hwnd,
                       HWND_NOTOPMOST,
                       0,
                       0,
                       0,
                       0,
                       SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
}

bool sendMouseInputForExtensionTest(DWORD flags)
{
    INPUT input{};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = flags;
    return ::SendInput(1, &input, sizeof(INPUT)) == 1;
}

QPoint nativePointForExtensionTest(QWidget* referenceWidget, const QPoint& globalPos)
{
    QWidget* root = referenceWidget ? referenceWidget->window() : nullptr;
    if (root)
    {
        if (HWND hwnd = rootHwndForExtensionTest(root))
        {
            POINT clientOrigin{0, 0};
            if (::ClientToScreen(hwnd, &clientOrigin))
            {
                const QPoint rootGlobal = root->mapToGlobal(QPoint(0, 0));
                const qreal dpr = qMax<qreal>(1.0, root->devicePixelRatioF());
                return QPoint(clientOrigin.x + qRound(static_cast<qreal>(globalPos.x() - rootGlobal.x()) * dpr),
                              clientOrigin.y + qRound(static_cast<qreal>(globalPos.y() - rootGlobal.y()) * dpr));
            }
        }
    }

    if (QScreen* screen = QGuiApplication::screenAt(globalPos))
    {
        const qreal dpr = qMax<qreal>(1.0, screen->devicePixelRatio());
        const QPoint topLeft = screen->geometry().topLeft();
        return topLeft + QPoint(qRound(static_cast<qreal>(globalPos.x() - topLeft.x()) * dpr),
                                qRound(static_cast<qreal>(globalPos.y() - topLeft.y()) * dpr));
    }

    return globalPos;
}

bool nativeMoveMouseForExtensionTest(QWidget* referenceWidget, const QPoint& globalPos)
{
    const QPoint nativePos = nativePointForExtensionTest(referenceWidget, globalPos);
    const int virtualLeft = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
    const int virtualTop = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
    const int virtualWidth = qMax(1, ::GetSystemMetrics(SM_CXVIRTUALSCREEN));
    const int virtualHeight = qMax(1, ::GetSystemMetrics(SM_CYVIRTUALSCREEN));

    INPUT input{};
    input.type = INPUT_MOUSE;
    input.mi.dx = static_cast<LONG>(
        qRound(static_cast<qreal>(nativePos.x() - virtualLeft) * 65535.0 /
               static_cast<qreal>(qMax(1, virtualWidth - 1))));
    input.mi.dy = static_cast<LONG>(
        qRound(static_cast<qreal>(nativePos.y() - virtualTop) * 65535.0 /
               static_cast<qreal>(qMax(1, virtualHeight - 1))));
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;
    return ::SendInput(1, &input, sizeof(INPUT)) == 1;
}

bool nativeMouseClickForExtensionTest(QWidget* focusWidget, const QPoint& globalPos)
{
    if (!bringWidgetToFrontForExtensionTest(focusWidget))
    {
        return false;
    }

    if (!nativeMoveMouseForExtensionTest(focusWidget, globalPos))
    {
        releaseTopMostForExtensionTest(focusWidget);
        return false;
    }
    QTest::qWait(40);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 40);

    const bool sent = sendMouseInputForExtensionTest(MOUSEEVENTF_LEFTDOWN) &&
                      sendMouseInputForExtensionTest(MOUSEEVENTF_LEFTUP);
    QTest::qWait(80);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 80);
    releaseTopMostForExtensionTest(focusWidget);
    return sent;
}

bool nativeMouseDragForExtensionTest(QWidget* focusWidget, const QPoint& startGlobal, const QPoint& endGlobal)
{
    if (!bringWidgetToFrontForExtensionTest(focusWidget))
    {
        return false;
    }

    if (!nativeMoveMouseForExtensionTest(focusWidget, startGlobal))
    {
        releaseTopMostForExtensionTest(focusWidget);
        return false;
    }
    QTest::qWait(60);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 60);

    if (!sendMouseInputForExtensionTest(MOUSEEVENTF_LEFTDOWN))
    {
        releaseTopMostForExtensionTest(focusWidget);
        return false;
    }
    QTest::qWait(80);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 80);

    constexpr int kSteps = 18;
    for (int step = 1; step <= kSteps; ++step)
    {
        const qreal t = static_cast<qreal>(step) / static_cast<qreal>(kSteps);
        const QPoint point(qRound(startGlobal.x() + (endGlobal.x() - startGlobal.x()) * t),
                           qRound(startGlobal.y() + (endGlobal.y() - startGlobal.y()) * t));
        if (!nativeMoveMouseForExtensionTest(focusWidget, point))
        {
            sendMouseInputForExtensionTest(MOUSEEVENTF_LEFTUP);
            releaseTopMostForExtensionTest(focusWidget);
            return false;
        }
        QTest::qWait(14);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 14);
    }

    const bool released = sendMouseInputForExtensionTest(MOUSEEVENTF_LEFTUP);
    QTest::qWait(120);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 120);
    releaseTopMostForExtensionTest(focusWidget);
    return released;
}
#endif
} // namespace

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
    void ribbon();
    void toolButton();
    void toolBar();
    void menuBar();
    void dockWidget();
    void dockManager();
    void widget();
    void window();
    void windowTitleBarButtonsHandleChildDeliveredClicks();
    void windowTitleBarButtonsTriggerOnRelease();
    void windowAlwaysOnTopDoesNotRecreateVisibleWindow();
    void windowTitleBarHoverStateClearsOnLeave();
    void windowThemeButtonShowsTransitionOverlay();
    void windowThemeTransitionOverlayKeepsOldFrameScale();
    void windowThemeTransitionRevealsNewFrameWithoutBlackHole();
    void windowNativeHitTestSupportsSnapZones();
    void windowDwmFrameMarginsPreserveShadow();
    void windowLegacyFramePolicyRestoresShadowAfterResize();
    void windowMaximizedNcCalcKeepsFullWorkArea();
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
    QCOMPARE(w->currentMessage(), "Ready");
    QCOMPARE(msgSpy.count(), 1);

    w->clearMessage();
    QCOMPARE(w->currentMessage(), QString());
    w->showMessage(QStringLiteral("Saving"), 30);
    QCOMPARE(w->currentMessage(), QStringLiteral("Saving"));
    QTRY_COMPARE(w->currentMessage(), QString());

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

    auto* actionButton = new AntToolButton;
    auto* runAction = new QAction(QStringLiteral("Run"), actionButton);
    actionButton->setDefaultAction(runAction);
    QCOMPARE(actionButton->defaultAction(), runAction);
    QCOMPARE(actionButton->text(), QStringLiteral("Run"));
    QSignalSpy runSpy(runAction, &QAction::triggered);
    actionButton->resize(actionButton->sizeHint());
    QTest::mouseClick(actionButton, Qt::LeftButton, Qt::NoModifier, actionButton->rect().center());
    QCOMPARE(runSpy.count(), 1);
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
    QSignalSpy actionSpy(action, &QAction::triggered);
    QTest::mouseClick(button, Qt::LeftButton, Qt::NoModifier, button->rect().center());
    QCOMPARE(actionSpy.count(), 1);

    auto* w2 = new AntToolBar("My Toolbar");
    QVERIFY(w2 != nullptr);
}

void TestAntQtExtensions::ribbon()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    auto* ribbon = new AntRibbon;
    QCOMPARE(ribbon->pageCount(), 0);
    QCOMPARE(ribbon->currentPageIndex(), -1);
    QCOMPARE(ribbon->currentPageKey(), QString());
    QCOMPARE(ribbon->isCollapsed(), false);
    QCOMPARE(ribbon->isCollapseButtonVisible(), true);

    QSignalSpy currentSpy(ribbon, &AntRibbon::currentPageChanged);
    QSignalSpy currentKeySpy(ribbon, &AntRibbon::currentPageKeyChanged);
    auto* file = ribbon->addPage(QStringLiteral("File"), QStringLiteral("file"));
    QVERIFY(file != nullptr);
    QCOMPARE(ribbon->pageCount(), 1);
    QCOMPARE(ribbon->pageAt(0), file);
    QCOMPARE(ribbon->pageByKey(QStringLiteral("file")), file);
    QCOMPARE(ribbon->currentPageIndex(), 0);
    QCOMPARE(ribbon->currentPageKey(), QStringLiteral("file"));
    QCOMPARE(currentSpy.count(), 1);
    QCOMPARE(currentKeySpy.count(), 1);

    auto* edit = ribbon->addPage(QStringLiteral("Edit"), QStringLiteral("edit"));
    QVERIFY(edit != nullptr);
    const QRectF fileIndicator = ribbon->indicatorRect();
    ribbon->setCurrentPageKey(QStringLiteral("edit"));
    QCOMPARE(ribbon->currentPageIndex(), 1);
    QCOMPARE(ribbon->currentPageKey(), QStringLiteral("edit"));
    QVERIFY(ribbon->indicatorRect().isValid());
    QVERIFY(qAbs(ribbon->indicatorRect().left() - fileIndicator.left()) < 0.5);
    QTRY_VERIFY(qAbs(ribbon->indicatorRect().left() - fileIndicator.left()) > 1.0);
    ribbon->setCurrentPageIndex(0);
    QCOMPARE(ribbon->currentPageKey(), QStringLiteral("file"));

    auto* group = file->addGroup(QStringLiteral("Clipboard"));
    QVERIFY(group != nullptr);
    QCOMPARE(file->groupCount(), 1);
    QCOMPARE(file->groupAt(0), group);

    QSignalSpy groupActionSpy(group, &AntRibbonGroup::actionTriggered);
    QSignalSpy ribbonActionSpy(ribbon, &AntRibbon::actionTriggered);
    auto* pasteAction = new QAction(QIcon(), QStringLiteral("Paste"), ribbon);
    group->addLargeAction(pasteAction);
    group->addSmallAction(new QAction(QStringLiteral("Copy"), ribbon));
    auto* combo = new QComboBox;
    combo->addItem(QStringLiteral("Mode A"));
    group->addWidget(combo, Ant::RibbonItemSize::Small);
    group->addWidget(new QWidget, Ant::RibbonItemSize::Large);
    QCOMPARE(group->itemCount(), 4);

    auto* controls = file->addGroup(QStringLiteral("Ant Controls"));
    QVERIFY(controls != nullptr);
    auto* modeSelect = new QComboBox;
    modeSelect->addItem(QStringLiteral("Advanced"));
    controls->addWidget(modeSelect, Ant::RibbonItemSize::Small);
    controls->addWidget(new QComboBox, Ant::RibbonItemSize::Small);
    controls->addWidget(new QWidget, Ant::RibbonItemSize::Large);
    QCOMPARE(file->groupCount(), 2);
    QVERIFY(group->sizeHint().height() >= 158);
    QVERIFY(ribbon->sizeHint().height() >= 210);
    QVERIFY(ribbon->property("indicatorRect").isValid());
    QVERIFY(ribbon->property("contentHeight").isValid());
    pasteAction->trigger();
    QCOMPARE(groupActionSpy.count(), 1);
    QCOMPARE(ribbonActionSpy.count(), 1);

    ribbon->resize(640, ribbon->sizeHint().height());
    ribbon->show();
    QVERIFY(QTest::qWaitForWindowExposed(ribbon));
    auto* clipboardTitle = ribbonTitleLabelForExtensionTest(group, QStringLiteral("Clipboard"));
    auto* controlsTitle = ribbonTitleLabelForExtensionTest(controls, QStringLiteral("Ant Controls"));
    QVERIFY(clipboardTitle != nullptr);
    QVERIFY(controlsTitle != nullptr);
    QCOMPARE(clipboardTitle->mapTo(ribbon, QPoint(0, 0)).y(), controlsTitle->mapTo(ribbon, QPoint(0, 0)).y());
    ribbon->setCurrentPageIndex(1);
    QTRY_COMPARE(ribbon->currentPageIndex(), 1);
    QTest::mouseMove(ribbon, QPoint(28, 20));
    QCoreApplication::processEvents();
    const QImage hoverImage = renderForExtensionTest(ribbon);
    const QColor hoveredTabBackground = hoverImage.pixelColor(18, 14);
    QVERIFY2(colorNearForExtensionTest(hoveredTabBackground, antTheme->tokens().colorBgElevated),
             qPrintable(QStringLiteral("Hovered tab painted a filled background: sampled %1 expected near %2")
                            .arg(colorStringForExtensionTest(hoveredTabBackground),
                                 colorStringForExtensionTest(antTheme->tokens().colorBgElevated))));
    ribbon->setCurrentPageIndex(0);

    QSignalSpy collapsedSpy(ribbon, &AntRibbon::collapsedChanged);
    ribbon->setCollapsed(true);
    QCOMPARE(ribbon->isCollapsed(), true);
    QCOMPARE(collapsedSpy.count(), 1);
    QVERIFY(ribbon->property("contentHeight").toReal() > 0.0);
    QTRY_VERIFY(ribbon->sizeHint().height() < 80);
    ribbon->setCollapsed(false);
    QCOMPARE(ribbon->isCollapsed(), false);
    QCOMPARE(collapsedSpy.count(), 2);
    QVERIFY(ribbon->property("contentHeight").toReal() < 176.0);
    QTRY_VERIFY(ribbon->sizeHint().height() >= 210);
    ribbon->setCollapsed(true);
    QCOMPARE(collapsedSpy.count(), 3);
    QTRY_VERIFY(ribbon->sizeHint().height() < 80);
    ribbon->resize(640, ribbon->sizeHint().height());
    QTest::mouseClick(ribbon, Qt::LeftButton, Qt::NoModifier, QPoint(28, 20));
    auto* popup = ribbon->findChild<QWidget*>(QStringLiteral("AntRibbonPopup"));
    QVERIFY(popup != nullptr);
    QTRY_VERIFY(popup->isVisible());
    popup->hide();
    ribbon->hide();

    AntWindow window;
    window.resize(700, 420);
    auto* windowRibbon = new AntRibbon;
    window.setRibbon(windowRibbon);
    QCOMPARE(window.ribbon(), windowRibbon);
    QCOMPARE(window.isRibbonVisible(), true);
    window.setRibbonVisible(false);
    QCOMPARE(window.isRibbonVisible(), false);
    QVERIFY(!windowRibbon->isVisible());
    window.setCentralWidget(new QWidget);
    QCOMPARE(window.ribbon(), windowRibbon);
    window.setRibbonVisible(true);
    QCOMPARE(window.isRibbonVisible(), true);
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

    QMainWindow host;
    host.resize(360, 260);
    auto* lockedFloat = new AntDockWidget(QStringLiteral("Locked Float"));
    lockedFloat->setWidget(new QWidget);
    lockedFloat->setFeatures(lockedFloat->features() & ~QDockWidget::DockWidgetFloatable);
    host.addDockWidget(Qt::LeftDockWidgetArea, lockedFloat);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    QVERIFY(!lockedFloat->isFloating());
    QWidget* titleBar = lockedFloat->titleBarWidget();
    QVERIFY(titleBar != nullptr);
    QTRY_VERIFY(titleBar->isVisible());
    const QPoint titlePoint = titleBar->rect().center();
    QMouseEvent blockedDoubleClick(QEvent::MouseButtonDblClick,
                                   QPointF(titlePoint),
                                   QPointF(titleBar->mapToGlobal(titlePoint)),
                                   Qt::LeftButton,
                                   Qt::LeftButton,
                                   Qt::NoModifier);
    QCoreApplication::sendEvent(titleBar, &blockedDoubleClick);
    QCoreApplication::processEvents();
    QVERIFY(!lockedFloat->isFloating());

    lockedFloat->setFeatures(lockedFloat->features() | QDockWidget::DockWidgetFloatable);
    QMouseEvent allowedDoubleClick(QEvent::MouseButtonDblClick,
                                   QPointF(titlePoint),
                                   QPointF(titleBar->mapToGlobal(titlePoint)),
                                   Qt::LeftButton,
                                   Qt::LeftButton,
                                   Qt::NoModifier);
    QCoreApplication::sendEvent(titleBar, &allowedDoubleClick);
    QTRY_VERIFY(lockedFloat->isFloating());
    lockedFloat->hide();
}

void TestAntQtExtensions::dockManager()
{
    qRegisterMetaType<AntDockWidget*>("AntDockWidget*");
    qRegisterMetaType<AntDockManager::DockPlacement>("AntDockManager::DockPlacement");
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    auto* manager = new AntDockManager;
    manager->resize(640, 420);
    QCOMPARE(manager->dockOptions().testFlag(QMainWindow::AllowNestedDocks), true);
    QCOMPARE(manager->dockOptions().testFlag(QMainWindow::AllowTabbedDocks), true);
    QCOMPARE(manager->palette().color(QPalette::Window), antTheme->tokens().colorBgLayout);
    QCOMPARE(manager->isDropGuideEnabled(), true);
    QCOMPARE(manager->isDropGuideVisible(), true);
    QCOMPARE(manager->activeDropGuide(), AntDockManager::DockPlacement::None);
    QCOMPARE(manager->isDropPreviewVisible(), false);
    QVERIFY(manager->dropPreviewRect().isEmpty());

    auto* explorer = new AntDockWidget(QStringLiteral("Explorer"));
    explorer->setWidget(new QWidget);
    auto* inspector = new AntDockWidget(QStringLiteral("Inspector"));
    inspector->setWidget(new QWidget);
    auto* preview = new AntDockWidget(QStringLiteral("Preview"));
    preview->setWidget(new QWidget);

    QSignalSpy addedSpy(manager, &AntDockManager::dockWidgetAdded);
    manager->addDockWidget(Qt::LeftDockWidgetArea, explorer);
    QCOMPARE(addedSpy.count(), 1);
    QVERIFY(manager->dockWidgets().contains(explorer));
    QCOMPARE(manager->dockWidgetArea(explorer), Qt::LeftDockWidgetArea);
    QVERIFY(!explorer->objectName().isEmpty());

    manager->splitDockWidget(explorer, inspector, Qt::Horizontal);
    QCOMPARE(addedSpy.count(), 2);
    QVERIFY(manager->dockWidgets().contains(inspector));

    manager->addDockWidget(preview, inspector, AntDockManager::DockPlacement::Center);
    QCOMPARE(addedSpy.count(), 3);
    QVERIFY(manager->dockWidgets().contains(preview));
    QVERIFY(manager->tabifiedDockWidgets(inspector).contains(preview));

    QTabWidget* inspectorArea = dockAreaForExtensionTest(inspector);
    QVERIFY(inspectorArea != nullptr);
    auto* properties = new AntDockWidget(QStringLiteral("Properties"));
    properties->setWidget(new QWidget);
    manager->addDockWidget(properties, inspector, AntDockManager::DockPlacement::Right);
    QCOMPARE(addedSpy.count(), 4);
    QTabWidget* propertiesArea = dockAreaForExtensionTest(properties);
    QVERIFY(propertiesArea != nullptr);
    QVERIFY(propertiesArea != inspectorArea);

    QSignalSpy featureSpy(manager, &AntDockManager::dockWidgetFeatureChanged);
    QSignalSpy tabMovedSpy(manager, &AntDockManager::dockWidgetTabMoved);
    QSignalSpy contextMenuSpy(manager, &AntDockManager::dockWidgetContextMenuRequested);
    QSignalSpy floatedSpy(manager, &AntDockManager::dockWidgetFloated);
    QSignalSpy dockedSpy(manager, &AntDockManager::dockWidgetDocked);
    QSignalSpy layoutSpy(manager, &AntDockManager::dockLayoutChanged);

    QVERIFY(manager->isDockWidgetClosable(explorer));
    manager->setDockWidgetClosable(explorer, false);
    QCOMPARE(manager->isDockWidgetClosable(explorer), false);
    manager->setDockWidgetClosable(explorer, true);
    QCOMPARE(manager->isDockWidgetClosable(explorer), true);
    QCOMPARE(featureSpy.count(), 2);

    QVERIFY(manager->isDockWidgetFloatable(preview));
    manager->setDockWidgetFloatable(preview, false);
    QCOMPARE(manager->isDockWidgetFloatable(preview), false);
    manager->setDockWidgetFloating(preview, true);
    QVERIFY(!manager->isDockWidgetFloating(preview));
    manager->setDockWidgetFloatable(preview, true);
    QCOMPARE(manager->isDockWidgetFloatable(preview), true);

    auto* preconfiguredDock = new AntDockWidget(QStringLiteral("Preconfigured"));
    preconfiguredDock->setWidget(new QWidget);
    preconfiguredDock->setFeatures(QDockWidget::DockWidgetClosable);
    manager->addDockWidget(Qt::BottomDockWidgetArea, preconfiguredDock);
    QCOMPARE(manager->isDockWidgetClosable(preconfiguredDock), true);
    QCOMPARE(manager->isDockWidgetFloatable(preconfiguredDock), false);
    QCOMPARE(manager->isDockWidgetMovable(preconfiguredDock), false);
    manager->setDockWidgetFloating(preconfiguredDock, true);
    QVERIFY(!manager->isDockWidgetFloating(preconfiguredDock));
    manager->removeDockWidget(preconfiguredDock);
    delete preconfiguredDock;

    auto* detachedLockedFloating = new AntDockWidget(QStringLiteral("Detached Locked Floating"));
    detachedLockedFloating->setWidget(new QWidget);
    detachedLockedFloating->setFeatures(detachedLockedFloating->features() & ~QDockWidget::DockWidgetFloatable);
    const int addedBeforeLockedDetachedFloat = addedSpy.count();
    const int floatedBeforeLockedDetachedFloat = floatedSpy.count();
    manager->setDockWidgetFloating(detachedLockedFloating,
                                   true,
                                   QRect(manager->mapToGlobal(QPoint(96, 72)), QSize(240, 160)));
    QCOMPARE(addedSpy.count(), addedBeforeLockedDetachedFloat);
    QCOMPARE(floatedSpy.count(), floatedBeforeLockedDetachedFloat);
    QVERIFY(!manager->dockWidgets().contains(detachedLockedFloating));
    QVERIFY(!detachedLockedFloating->isVisible());
    delete detachedLockedFloating;

    auto* detachedFloating = new AntDockWidget(QStringLiteral("Detached Floating"));
    detachedFloating->setWidget(new QWidget);
    const int addedBeforeDetachedFloat = addedSpy.count();
    const int floatedBeforeDetachedFloat = floatedSpy.count();
    manager->setDockWidgetFloating(detachedFloating,
                                   true,
                                   QRect(manager->mapToGlobal(QPoint(120, 96)), QSize(260, 180)));
    QCOMPARE(addedSpy.count(), addedBeforeDetachedFloat + 1);
    QCOMPARE(floatedSpy.count(), floatedBeforeDetachedFloat + 1);
    QVERIFY(manager->dockWidgets().contains(detachedFloating));
    QVERIFY(manager->isDockWidgetFloating(detachedFloating));
    QVERIFY(manager->floatingDockWidgets().contains(detachedFloating));
    QTRY_VERIFY(detachedFloating->isVisible());
    manager->removeDockWidget(detachedFloating);
    QTRY_VERIFY(!detachedFloating->isVisible());
    delete detachedFloating;

    auto* lockedCloseFloating = new AntDockWidget(QStringLiteral("Locked Close Floating"));
    lockedCloseFloating->setWidget(new QWidget);
    lockedCloseFloating->setFeatures(lockedCloseFloating->features() & ~QDockWidget::DockWidgetClosable);
    manager->setDockWidgetFloating(lockedCloseFloating,
                                   true,
                                   QRect(manager->mapToGlobal(QPoint(132, 108)), QSize(260, 180)));
    QVERIFY(manager->dockWidgets().contains(lockedCloseFloating));
    QVERIFY(manager->isDockWidgetFloating(lockedCloseFloating));
    QTRY_VERIFY(lockedCloseFloating->isVisible());
    QWidget* lockedCloseButton =
        lockedCloseFloating->titleBarWidget()->findChild<QWidget*>(QStringLiteral("AntDockTitleCloseButton"));
    QVERIFY(lockedCloseButton != nullptr);
    QVERIFY(!lockedCloseButton->isVisible());
    manager->setDockWidgetClosable(lockedCloseFloating, true);
    QTRY_VERIFY(lockedCloseButton->isVisible());
    manager->setDockWidgetClosable(lockedCloseFloating, false);
    QTRY_VERIFY(!lockedCloseButton->isVisible());
    lockedCloseFloating->close();
    QCoreApplication::processEvents();
    QVERIFY(manager->dockWidgets().contains(lockedCloseFloating));
    QVERIFY(manager->floatingDockWidgets().contains(lockedCloseFloating));
    QVERIFY(lockedCloseFloating->isVisible());
    manager->removeDockWidget(lockedCloseFloating);
    QTRY_VERIFY(!lockedCloseFloating->isVisible());
    delete lockedCloseFloating;

    auto* directCloseFloating = new AntDockWidget(QStringLiteral("Direct Close Floating"));
    directCloseFloating->setWidget(new QWidget);
    QSignalSpy directCloseRemovedSpy(manager, &AntDockManager::dockWidgetRemoved);
    manager->setDockWidgetFloating(directCloseFloating,
                                   true,
                                   QRect(manager->mapToGlobal(QPoint(144, 120)), QSize(260, 180)));
    QVERIFY(manager->dockWidgets().contains(directCloseFloating));
    QVERIFY(manager->floatingDockWidgets().contains(directCloseFloating));
    QTRY_VERIFY(directCloseFloating->isVisible());
    directCloseFloating->close();
    QCoreApplication::processEvents();
    QCOMPARE(directCloseRemovedSpy.count(), 1);
    QVERIFY(!manager->dockWidgets().contains(directCloseFloating));
    QVERIFY(!manager->floatingDockWidgets().contains(directCloseFloating));
    QVERIFY(!directCloseFloating->isVisible());
    delete directCloseFloating;

    {
        auto* cleanupManager = new AntDockManager;
        cleanupManager->setGeometry(80, 80, 420, 280);
        cleanupManager->show();
        QVERIFY(QTest::qWaitForWindowExposed(cleanupManager));

        auto* cleanupFloating = new AntDockWidget(QStringLiteral("Cleanup Floating"));
        cleanupFloating->setWidget(new QWidget);
        cleanupManager->setDockWidgetFloating(
            cleanupFloating,
            true,
            QRect(cleanupManager->mapToGlobal(QPoint(96, 72)), QSize(240, 160)));
        QVERIFY(cleanupManager->dockWidgets().contains(cleanupFloating));
        QVERIFY(cleanupManager->floatingDockWidgets().contains(cleanupFloating));
        QTRY_VERIFY(cleanupFloating->isVisible());
        QCOMPARE(cleanupFloating->property("antDockFloatingOwnedByManager").toBool(), true);

        delete cleanupManager;
        QCoreApplication::processEvents();
        QVERIFY(!cleanupFloating->isVisible());
        QCOMPARE(cleanupFloating->property("antDockFloatingOwnedByManager").toBool(), false);
#if defined(Q_OS_WIN)
        QCOMPARE(cleanupFloating->property("antDockFloatingNativeOwnedByManager").toBool(), false);
#endif
        delete cleanupFloating;
    }

    auto* readdedFloating = new AntDockWidget(QStringLiteral("Readded Floating"));
    readdedFloating->setWidget(new QWidget);
    manager->addDockWidget(Qt::BottomDockWidgetArea, readdedFloating);
    QVERIFY(manager->dockWidgets().contains(readdedFloating));
    manager->removeDockWidget(readdedFloating);
    QVERIFY(!manager->dockWidgets().contains(readdedFloating));
    QCoreApplication::processEvents();
    const int ownerApplyBeforeReadd = readdedFloating->property("antDockFloatingOwnerApplyCount").toInt();
    manager->setDockWidgetFloating(
        readdedFloating,
        true,
        QRect(manager->mapToGlobal(QPoint(156, 132)), QSize(260, 180)));
    QVERIFY(manager->dockWidgets().contains(readdedFloating));
    QVERIFY(manager->floatingDockWidgets().contains(readdedFloating));
    QTRY_VERIFY(readdedFloating->isVisible());
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    const int ownerApplyDelta =
        readdedFloating->property("antDockFloatingOwnerApplyCount").toInt() - ownerApplyBeforeReadd;
    QVERIFY2(ownerApplyDelta >= 2 && ownerApplyDelta <= 3,
             qPrintable(QStringLiteral("unexpected owner apply count after re-add: %1").arg(ownerApplyDelta)));
    manager->removeDockWidget(readdedFloating);
    QTRY_VERIFY(!readdedFloating->isVisible());
    delete readdedFloating;

    auto* areaConnectionDock = new AntDockWidget(QStringLiteral("Area Connection"));
    areaConnectionDock->setWidget(new QWidget);
    manager->addDockWidget(areaConnectionDock, inspector, AntDockManager::DockPlacement::Center);
    QTabWidget* areaConnectionTabArea = dockAreaForExtensionTest(areaConnectionDock);
    QVERIFY(areaConnectionTabArea != nullptr);
    manager->removeDockWidget(areaConnectionDock);
    QVERIFY(!manager->dockWidgets().contains(areaConnectionDock));
    manager->addDockWidget(areaConnectionDock, inspector, AntDockManager::DockPlacement::Center);
    QCOMPARE(dockAreaForExtensionTest(areaConnectionDock), areaConnectionTabArea);
    const int titleSyncBeforeReaddTitleChange =
        areaConnectionDock->property("antDockAreaTitleSyncCount").toInt();
    areaConnectionDock->setWindowTitle(QStringLiteral("Area Connection Updated"));
    QCoreApplication::processEvents();
    QCOMPARE(areaConnectionDock->property("antDockAreaTitleSyncCount").toInt() -
                 titleSyncBeforeReaddTitleChange,
             1);
    manager->removeDockWidget(areaConnectionDock);
    delete areaConnectionDock;

    QVERIFY(manager->isDockWidgetMovable(preview));
    manager->setDockWidgetMovable(preview, false);
    QCOMPARE(manager->isDockWidgetMovable(preview), false);
    QVERIFY(!manager->moveDockWidgetTab(preview, 0));
    manager->setDockWidgetMovable(preview, true);
    QCOMPARE(manager->isDockWidgetMovable(preview), true);
    QCOMPARE(featureSpy.count(), 8);

    QVERIFY(manager->moveDockWidgetTab(preview, 0));
    QCOMPARE(manager->dockWidgetTabIndex(preview), 0);
    QCOMPARE(tabMovedSpy.count(), 1);
    QVERIFY(manager->moveDockWidgetTab(inspector, 0));
    QCOMPARE(manager->dockWidgetTabIndex(inspector), 0);
    QCOMPARE(tabMovedSpy.count(), 2);
    QVERIFY(layoutSpy.count() >= 2);

    QSignalSpy guideEnabledSpy(manager, &AntDockManager::dropGuideEnabledChanged);
    QSignalSpy guideVisibleSpy(manager, &AntDockManager::dropGuideVisibleChanged);
    manager->setDropGuideEnabled(false);
    QCOMPARE(manager->isDropGuideEnabled(), false);
    QCOMPARE(manager->isDropGuideVisible(), false);
    QCOMPARE(guideEnabledSpy.count(), 1);
    QCOMPARE(guideVisibleSpy.count(), 1);
    manager->setDropGuideVisible(true);
    QCOMPARE(manager->isDropGuideEnabled(), true);
    QCOMPARE(manager->isDropGuideVisible(), true);
    QCOMPARE(guideEnabledSpy.count(), 2);
    QCOMPARE(guideVisibleSpy.count(), 2);

    QSignalSpy savedSpy(manager, &AntDockManager::perspectiveSaved);
    QSignalSpy restoredSpy(manager, &AntDockManager::perspectiveRestored);
    QSignalSpy removedPerspectiveSpy(manager, &AntDockManager::perspectiveRemoved);
    QVERIFY(manager->savePerspective(QStringLiteral("default")));
    QCOMPARE(savedSpy.count(), 1);
    QVERIFY(manager->perspectiveNames().contains(QStringLiteral("default")));
    QVERIFY(!manager->perspectiveState(QStringLiteral("default")).isEmpty());
    manager->addDockWidget(properties, inspector, AntDockManager::DockPlacement::Center);
    QTRY_VERIFY(manager->tabifiedDockWidgets(inspector).contains(properties));
    QTRY_VERIFY(dockAreaForExtensionTest(properties) == dockAreaForExtensionTest(inspector));
    QVERIFY(manager->restorePerspective(QStringLiteral("default")));
    QCOMPARE(restoredSpy.count(), 1);
    inspectorArea = dockAreaForExtensionTest(inspector);
    propertiesArea = dockAreaForExtensionTest(properties);
    QVERIFY(inspectorArea != nullptr);
    QVERIFY(propertiesArea != nullptr);
    QVERIFY(propertiesArea != inspectorArea);
    QVERIFY(manager->tabifiedDockWidgets(inspector).contains(preview));
    QVERIFY(!manager->tabifiedDockWidgets(inspector).contains(properties));
    QVERIFY(manager->removePerspective(QStringLiteral("default")));
    QCOMPARE(removedPerspectiveSpy.count(), 1);
    QVERIFY(!manager->perspectiveNames().contains(QStringLiteral("default")));

    manager->show();
    QVERIFY(QTest::qWaitForWindowExposed(manager));
    const QRect inspectorAreaRect = QRect(inspectorArea->mapToGlobal(QPoint(0, 0)), inspectorArea->size());
    const QRect propertiesAreaRect = QRect(propertiesArea->mapToGlobal(QPoint(0, 0)), propertiesArea->size());
    QVERIFY2(propertiesAreaRect.left() >= inspectorAreaRect.center().x(),
             "DockPlacement::Right must create a real custom layout area to the target's right.");
    QVERIFY(explorer->titleBarWidget() != nullptr);
    QVERIFY(properties->titleBarWidget() != nullptr);
    QVERIFY(!explorer->titleBarWidget()->isVisible());
    QVERIFY(!properties->titleBarWidget()->isVisible());
    QTabBar* explorerTabBar = dockTabBarForExtensionTest(explorer);
    QVERIFY(explorerTabBar != nullptr);
    QSignalSpy previewVisibleSpy(manager, &AntDockManager::dropPreviewVisibleChanged);

    QTabBar* inspectorTabBar = dockTabBarForExtensionTest(inspector);
    QVERIFY(inspectorTabBar != nullptr);
    const QPoint inspectorTabPoint = dockTabCenterForExtensionTest(inspector);
    QVERIFY(!inspectorTabPoint.isNull());
    const auto menuItemByKey = [](AntMenu* menu, const QString& key) {
        for (int i = 0; menu && i < menu->itemCount(); ++i)
        {
            const AntMenuItem item = menu->itemAt(i);
            if (item.key == key)
            {
                return item;
            }
        }
        return AntMenuItem{};
    };
    const auto openInspectorMenu = [&]() {
        QContextMenuEvent contextEvent(QContextMenuEvent::Mouse,
                                       inspectorTabPoint,
                                       inspectorTabBar->mapToGlobal(inspectorTabPoint));
        QCoreApplication::sendEvent(inspectorTabBar, &contextEvent);
        QWidget* popup = nullptr;
        for (QWidget* widget : QApplication::topLevelWidgets())
        {
            if (widget && widget->objectName() == QStringLiteral("AntDockContextMenuPopup") && widget->isVisible())
            {
                popup = widget;
                break;
            }
        }
        return popup;
    };
    manager->setDockWidgetClosable(preview, false);
    QWidget* popup = openInspectorMenu();
    QCOMPARE(contextMenuSpy.count(), 1);
    QVERIFY(popup != nullptr);
    QCOMPARE(popup->objectName(), QStringLiteral("AntDockContextMenuPopup"));
    QVERIFY(popup->testAttribute(Qt::WA_TranslucentBackground));
    auto* dockMenu = popup->findChild<AntMenu*>(QStringLiteral("AntDockContextMenu"));
    QVERIFY(dockMenu != nullptr);
    QCOMPARE(dockMenu->menuTheme(),
             antTheme->themeMode() == Ant::ThemeMode::Dark ? Ant::MenuTheme::Dark : Ant::MenuTheme::Light);
    QCOMPARE(dockMenu->isCompact(), true);
    QCOMPARE(dockMenu->isSelectable(), false);
    QVERIFY(dockMenu->itemCount() >= 10);
    QCOMPARE(dockMenu->itemAt(0).key, QStringLiteral("float"));
    QCOMPARE(dockMenu->itemAt(0).label, QStringLiteral("Float"));
    QCOMPARE(dockMenu->itemAt(0).disabled, false);
    QCOMPARE(menuItemByKey(dockMenu, QStringLiteral("close-others")).disabled, true);
    popup->close();
    QCoreApplication::processEvents();

    manager->setDockWidgetClosable(preview, true);
    manager->setDockWidgetClosable(inspector, false);
    popup = openInspectorMenu();
    QCOMPARE(contextMenuSpy.count(), 2);
    QVERIFY(popup != nullptr);
    dockMenu = popup->findChild<AntMenu*>(QStringLiteral("AntDockContextMenu"));
    QVERIFY(dockMenu != nullptr);
    QCOMPARE(menuItemByKey(dockMenu, QStringLiteral("close-others")).disabled, false);
    QCOMPARE(menuItemByKey(dockMenu, QStringLiteral("close")).disabled, true);
    popup->close();
    QCoreApplication::processEvents();
    manager->setDockWidgetClosable(inspector, true);

    QTabBar* movableCancelTabBar = dockTabBarForExtensionTest(preview);
    QVERIFY(movableCancelTabBar != nullptr);
    const QPoint movableCancelStart = dockTabCenterForExtensionTest(preview);
    QVERIFY(!movableCancelStart.isNull());
    const QPoint movableCancelGlobal = movableCancelTabBar->mapToGlobal(movableCancelStart);
    QMouseEvent movableCancelPress(QEvent::MouseButtonPress,
                                   QPointF(movableCancelStart),
                                   QPointF(movableCancelGlobal),
                                   Qt::LeftButton,
                                   Qt::LeftButton,
                                   Qt::NoModifier);
    QCoreApplication::sendEvent(movableCancelTabBar, &movableCancelPress);
    manager->setDockWidgetMovable(preview, false);
    QCOMPARE(manager->isDockWidgetMovable(preview), false);
    const QPoint canceledMoveGlobal = propertiesAreaRect.center();
    QMouseEvent canceledMove(QEvent::MouseMove,
                             QPointF(movableCancelTabBar->mapFromGlobal(canceledMoveGlobal)),
                             QPointF(canceledMoveGlobal),
                             Qt::NoButton,
                             Qt::LeftButton,
                             Qt::NoModifier);
    QCoreApplication::sendEvent(movableCancelTabBar, &canceledMove);
    QCOMPARE(manager->property("antDockDragActivated").toBool(), false);
    QVERIFY(!manager->isDropPreviewVisible());
    QCOMPARE(manager->activeDropGuide(), AntDockManager::DockPlacement::None);
    QMouseEvent movableCancelRelease(QEvent::MouseButtonRelease,
                                     QPointF(movableCancelStart),
                                     QPointF(movableCancelGlobal),
                                     Qt::LeftButton,
                                     Qt::NoButton,
                                     Qt::NoModifier);
    QCoreApplication::sendEvent(movableCancelTabBar, &movableCancelRelease);
    manager->setDockWidgetMovable(preview, true);
    QCOMPARE(manager->isDockWidgetMovable(preview), true);

    QMouseEvent directFeatureCancelPress(QEvent::MouseButtonPress,
                                         QPointF(movableCancelStart),
                                         QPointF(movableCancelGlobal),
                                         Qt::LeftButton,
                                         Qt::LeftButton,
                                         Qt::NoModifier);
    QCoreApplication::sendEvent(movableCancelTabBar, &directFeatureCancelPress);
    const int directSetFeaturesSignalCount = featureSpy.count();
    preview->setFeatures(preview->features() & ~QDockWidget::DockWidgetMovable);
    QCOMPARE(manager->isDockWidgetMovable(preview), false);
    QCOMPARE(featureSpy.count(), directSetFeaturesSignalCount + 1);
    QMouseEvent directFeatureCanceledMove(QEvent::MouseMove,
                                          QPointF(movableCancelTabBar->mapFromGlobal(canceledMoveGlobal)),
                                          QPointF(canceledMoveGlobal),
                                          Qt::NoButton,
                                          Qt::LeftButton,
                                          Qt::NoModifier);
    QCoreApplication::sendEvent(movableCancelTabBar, &directFeatureCanceledMove);
    QCOMPARE(manager->property("antDockDragActivated").toBool(), false);
    QVERIFY(!manager->isDropPreviewVisible());
    QCOMPARE(manager->activeDropGuide(), AntDockManager::DockPlacement::None);
    QMouseEvent directFeatureCancelRelease(QEvent::MouseButtonRelease,
                                           QPointF(movableCancelStart),
                                           QPointF(movableCancelGlobal),
                                           Qt::LeftButton,
                                           Qt::NoButton,
                                           Qt::NoModifier);
    QCoreApplication::sendEvent(movableCancelTabBar, &directFeatureCancelRelease);
    preview->setFeatures(preview->features() | QDockWidget::DockWidgetMovable);
    QCOMPARE(manager->isDockWidgetMovable(preview), true);
    QCOMPARE(featureSpy.count(), directSetFeaturesSignalCount + 2);

    const QPoint previewTabPoint = dockTabCenterForExtensionTest(preview);
    QVERIFY(!previewTabPoint.isNull());
    const QPoint previewTabGlobal = inspectorTabBar->mapToGlobal(previewTabPoint);
    QMouseEvent tabReorderPress(QEvent::MouseButtonPress,
                                QPointF(previewTabPoint),
                                QPointF(previewTabGlobal),
                                Qt::LeftButton,
                                Qt::LeftButton,
                                Qt::NoModifier);
    QCoreApplication::sendEvent(inspectorTabBar, &tabReorderPress);
    QMouseEvent tabReorderMove(QEvent::MouseMove,
                               QPointF(inspectorTabPoint),
                               QPointF(inspectorTabBar->mapToGlobal(inspectorTabPoint)),
                               Qt::NoButton,
                               Qt::LeftButton,
                               Qt::NoModifier);
    QCoreApplication::sendEvent(inspectorTabBar, &tabReorderMove);
    QMouseEvent tabReorderRelease(QEvent::MouseButtonRelease,
                                  QPointF(inspectorTabPoint),
                                  QPointF(inspectorTabBar->mapToGlobal(inspectorTabPoint)),
                                  Qt::LeftButton,
                                  Qt::NoButton,
                                  Qt::NoModifier);
    QCoreApplication::sendEvent(inspectorTabBar, &tabReorderRelease);
    QTRY_COMPARE(manager->dockWidgetTabIndex(preview), 0);
    QVERIFY(!manager->isDropPreviewVisible());
    QVERIFY(tabMovedSpy.count() >= 3);
    QVERIFY(manager->moveDockWidgetTab(inspector, 0));
    QCOMPARE(manager->dockWidgetTabIndex(inspector), 0);

    const QPoint dragStart = dockTabCenterForExtensionTest(explorer);
    QVERIFY(!dragStart.isNull());
    const QPoint propertiesCenterGlobal = propertiesAreaRect.center();
    const QPoint propertiesCenterMove = explorerTabBar->mapFromGlobal(propertiesCenterGlobal);

    QMouseEvent pressEvent(QEvent::MouseButtonPress,
                           QPointF(dragStart),
                           QPointF(explorerTabBar->mapToGlobal(dragStart)),
                           Qt::LeftButton,
                           Qt::LeftButton,
                           Qt::NoModifier);
    QCoreApplication::sendEvent(explorerTabBar, &pressEvent);
    QMouseEvent moveToPropertiesEvent(QEvent::MouseMove,
                                      QPointF(propertiesCenterMove),
                                      QPointF(propertiesCenterGlobal),
                                      Qt::NoButton,
                                      Qt::LeftButton,
                                      Qt::NoModifier);
    QCoreApplication::sendEvent(explorerTabBar, &moveToPropertiesEvent);
    QTRY_VERIFY(manager->isDropPreviewVisible());
#if defined(Q_OS_WIN)
    QWidget* dragPreviewWindow = nullptr;
    QWidget* dropPreviewWindow = nullptr;
    QTRY_VERIFY((dragPreviewWindow = topLevelWidgetForExtensionTest(QStringLiteral("AntDockDragPreviewWindow"))) != nullptr);
    QTRY_VERIFY((dropPreviewWindow = topLevelWidgetForExtensionTest(QStringLiteral("AntDockDropPreviewWindow"))) != nullptr);
    QVERIFY(dragPreviewWindow->testAttribute(Qt::WA_TransparentForMouseEvents));
    QVERIFY(dropPreviewWindow->testAttribute(Qt::WA_TransparentForMouseEvents));
    QTRY_COMPARE(dragPreviewWindow->property("antDockTransparentToolWindowClickThrough").toBool(), true);
    QTRY_COMPARE(dropPreviewWindow->property("antDockTransparentToolWindowClickThrough").toBool(), true);
#endif
    QTRY_COMPARE(manager->activeDropGuide(), AntDockManager::DockPlacement::Center);
    QVERIFY2(propertiesAreaRect.contains(manager->dropPreviewRect().center()),
             "Dock-area guide must follow the dock area under the cursor instead of staying at the container center.");

    const QPoint rightGuideGlobal = manager->mapToGlobal(QPoint(manager->width() - 33, manager->height() / 2));
    const QPoint rightGuideMove = explorerTabBar->mapFromGlobal(rightGuideGlobal);
    QMouseEvent moveEvent(QEvent::MouseMove,
                          QPointF(rightGuideMove),
                          QPointF(rightGuideGlobal),
                          Qt::NoButton,
                          Qt::LeftButton,
                          Qt::NoModifier);
    QCoreApplication::sendEvent(explorerTabBar, &moveEvent);
    QTRY_VERIFY(manager->isDropPreviewVisible());
    QTRY_COMPARE(manager->activeDropGuide(), AntDockManager::DockPlacement::Right);
    QVERIFY(!manager->dropPreviewRect().isEmpty());
    const QRect managerGlobalRect = QRect(manager->mapToGlobal(QPoint(0, 0)), manager->size());
    QVERIFY2(manager->dropPreviewRect().left() >= managerGlobalRect.center().x(),
             "Right edge guide preview must target the right half of the dock container.");
    QMouseEvent releaseRightEvent(QEvent::MouseButtonRelease,
                                  QPointF(rightGuideMove),
                                  QPointF(rightGuideGlobal),
                                  Qt::LeftButton,
                                  Qt::NoButton,
                                  Qt::NoModifier);
    QCoreApplication::sendEvent(explorerTabBar, &releaseRightEvent);
    QTRY_VERIFY(!manager->isDropPreviewVisible());
    QTRY_VERIFY([&]() {
        QTabWidget* explorerArea = dockAreaForExtensionTest(explorer);
        if (!explorerArea) return false;
        const QRect explorerAreaRect = QRect(explorerArea->mapToGlobal(QPoint(0, 0)), explorerArea->size());
        return explorerAreaRect.left() >= managerGlobalRect.center().x();
    }());

    const QRect inspectorDropAreaRect = QRect(inspectorArea->mapToGlobal(QPoint(0, 0)), inspectorArea->size());
    const QPoint centerGuideGlobal = inspectorDropAreaRect.center();
    QTabBar* propertiesTabBar = dockTabBarForExtensionTest(properties);
    QVERIFY(propertiesTabBar != nullptr);
    const QPoint propertiesDragStart = dockTabCenterForExtensionTest(properties);
    QVERIFY(!propertiesDragStart.isNull());
    const QPoint centerGuideMove = propertiesTabBar->mapFromGlobal(centerGuideGlobal);
    QMouseEvent pressCenterEvent(QEvent::MouseButtonPress,
                                 QPointF(propertiesDragStart),
                                 QPointF(propertiesTabBar->mapToGlobal(propertiesDragStart)),
                                 Qt::LeftButton,
                                 Qt::LeftButton,
                                 Qt::NoModifier);
    QCoreApplication::sendEvent(propertiesTabBar, &pressCenterEvent);
    QMouseEvent guideMoveEvent(QEvent::MouseMove,
                               QPointF(centerGuideMove),
                               QPointF(centerGuideGlobal),
                               Qt::NoButton,
                               Qt::LeftButton,
                               Qt::NoModifier);
    QCoreApplication::sendEvent(propertiesTabBar, &guideMoveEvent);
    QTRY_COMPARE(manager->activeDropGuide(), AntDockManager::DockPlacement::Center);
    auto* opacityEffect = qobject_cast<QGraphicsOpacityEffect*>(properties->graphicsEffect());
    QVERIFY(opacityEffect != nullptr);
    QVERIFY(qAbs(opacityEffect->opacity() - 0.68) < 0.01);
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
                             QPointF(centerGuideMove),
                             QPointF(centerGuideGlobal),
                             Qt::LeftButton,
                             Qt::NoButton,
                             Qt::NoModifier);
    QCoreApplication::sendEvent(propertiesTabBar, &releaseEvent);
    QTRY_VERIFY(!manager->isDropPreviewVisible());
    QCOMPARE(manager->activeDropGuide(), AntDockManager::DockPlacement::None);
    QTRY_VERIFY(properties->graphicsEffect() == nullptr);
    QTRY_VERIFY(manager->tabifiedDockWidgets(preview).contains(properties) ||
                manager->tabifiedDockWidgets(inspector).contains(properties) ||
                manager->tabifiedDockWidgets(explorer).contains(properties) ||
                manager->tabifiedDockWidgets(properties).contains(preview) ||
                manager->tabifiedDockWidgets(properties).contains(inspector) ||
                manager->tabifiedDockWidgets(properties).contains(explorer));
    QVERIFY(previewVisibleSpy.count() >= 4);

    QTabWidget* explorerAreaBeforeDisabledGuideDrag = dockAreaForExtensionTest(explorer);
    QTabWidget* disabledGuideTargetArea = dockAreaForExtensionTest(inspector);
    QVERIFY(explorerAreaBeforeDisabledGuideDrag != nullptr);
    QVERIFY(disabledGuideTargetArea != nullptr);
    QVERIFY(disabledGuideTargetArea != explorerAreaBeforeDisabledGuideDrag);
    const QRect disabledGuideTargetRect = QRect(disabledGuideTargetArea->mapToGlobal(QPoint(0, 0)),
                                                disabledGuideTargetArea->size());
    const QPoint disabledGuideTargetGlobal = disabledGuideTargetRect.center();
    explorerTabBar = dockTabBarForExtensionTest(explorer);
    QVERIFY(explorerTabBar != nullptr);
    const QPoint disabledGuideDragStart = dockTabCenterForExtensionTest(explorer);
    QVERIFY(!disabledGuideDragStart.isNull());
    const QPoint disabledGuideMove = explorerTabBar->mapFromGlobal(disabledGuideTargetGlobal);
    manager->setDropGuideEnabled(false);
    QMouseEvent disabledPressEvent(QEvent::MouseButtonPress,
                                   QPointF(disabledGuideDragStart),
                                   QPointF(explorerTabBar->mapToGlobal(disabledGuideDragStart)),
                                   Qt::LeftButton,
                                   Qt::LeftButton,
                                   Qt::NoModifier);
    QCoreApplication::sendEvent(explorerTabBar, &disabledPressEvent);
    QMouseEvent disabledMoveEvent(QEvent::MouseMove,
                                  QPointF(disabledGuideMove),
                                  QPointF(disabledGuideTargetGlobal),
                                  Qt::NoButton,
                                  Qt::LeftButton,
                                  Qt::NoModifier);
    QCoreApplication::sendEvent(explorerTabBar, &disabledMoveEvent);
    QTRY_VERIFY(manager->isDropPreviewVisible());
    QCOMPARE(manager->activeDropGuide(), AntDockManager::DockPlacement::None);
    QVERIFY2(disabledGuideTargetRect.contains(manager->dropPreviewRect().center()),
             "Disabling drop guide squares must keep implicit docking preview and target selection active.");
    QMouseEvent disabledReleaseEvent(QEvent::MouseButtonRelease,
                                     QPointF(disabledGuideMove),
                                     QPointF(disabledGuideTargetGlobal),
                                     Qt::LeftButton,
                                     Qt::NoButton,
                                     Qt::NoModifier);
    QCoreApplication::sendEvent(explorerTabBar, &disabledReleaseEvent);
    QTRY_VERIFY(!manager->isDropPreviewVisible());
    QTRY_VERIFY(explorer->graphicsEffect() == nullptr);
    QTRY_VERIFY(dockAreaForExtensionTest(explorer) != explorerAreaBeforeDisabledGuideDrag);
    QTRY_VERIFY(manager->tabifiedDockWidgets(inspector).contains(explorer) ||
                manager->tabifiedDockWidgets(explorer).contains(inspector));
    manager->setDropGuideEnabled(true);

    auto dragFloatingDockBack = [&](AntDockWidget* dock, AntDockWidget* target) {
        QWidget* titleBar = dock ? dock->titleBarWidget() : nullptr;
        QTabWidget* targetArea = dockAreaForExtensionTest(target);
        QVERIFY(titleBar != nullptr);
        QVERIFY(targetArea != nullptr);
        const QRect targetRect(targetArea->mapToGlobal(QPoint(0, 0)), targetArea->size());
        const QPoint titleStart = titleBar->rect().center();
        const QPoint targetGlobal = targetRect.center();
        const QPoint targetLocal = titleBar->mapFromGlobal(targetGlobal);
        QMouseEvent pressFloatingEvent(QEvent::MouseButtonPress,
                                       QPointF(titleStart),
                                       QPointF(titleBar->mapToGlobal(titleStart)),
                                       Qt::LeftButton,
                                       Qt::LeftButton,
                                       Qt::NoModifier);
        QCoreApplication::sendEvent(titleBar, &pressFloatingEvent);
        QMouseEvent moveFloatingEvent(QEvent::MouseMove,
                                      QPointF(targetLocal),
                                      QPointF(targetGlobal),
                                      Qt::NoButton,
                                      Qt::LeftButton,
                                      Qt::NoModifier);
        QCoreApplication::sendEvent(titleBar, &moveFloatingEvent);
        QMouseEvent releaseFloatingEvent(QEvent::MouseButtonRelease,
                                         QPointF(targetLocal),
                                         QPointF(targetGlobal),
                                         Qt::LeftButton,
                                         Qt::NoButton,
                                         Qt::NoModifier);
        QCoreApplication::sendEvent(titleBar, &releaseFloatingEvent);
    };

#if defined(Q_OS_WIN)
    auto dragFloatingDockBackWithNativeMouse = [&](AntDockWidget* dock, AntDockWidget* target) {
        QWidget* titleBar = dock ? dock->titleBarWidget() : nullptr;
        QTabWidget* targetArea = dockAreaForExtensionTest(target);
        QVERIFY(titleBar != nullptr);
        QVERIFY(targetArea != nullptr);
        const QRect targetRect(targetArea->mapToGlobal(QPoint(0, 0)), targetArea->size());
        const QPoint startGlobal = titleBar->mapToGlobal(titleBar->rect().center());
        const QPoint targetGlobal = targetRect.center();
        QVERIFY(nativeMouseDragForExtensionTest(dock, startGlobal, targetGlobal));
    };
#endif

    QTabBar* floatingSourceTabBar = dockTabBarForExtensionTest(explorer);
    QVERIFY(floatingSourceTabBar != nullptr);
    const QPoint floatDragStart = dockTabCenterForExtensionTest(explorer);
    QVERIFY(!floatDragStart.isNull());
    const QPoint outsideManagerGlobal = manager->mapToGlobal(QPoint(-260, -180));
    const QPoint outsideManagerLocal = floatingSourceTabBar->mapFromGlobal(outsideManagerGlobal);
    QMouseEvent floatPressEvent(QEvent::MouseButtonPress,
                                QPointF(floatDragStart),
                                QPointF(floatingSourceTabBar->mapToGlobal(floatDragStart)),
                                Qt::LeftButton,
                                Qt::LeftButton,
                                Qt::NoModifier);
    QCoreApplication::sendEvent(floatingSourceTabBar, &floatPressEvent);
    QMouseEvent floatMoveEvent(QEvent::MouseMove,
                               QPointF(outsideManagerLocal),
                               QPointF(outsideManagerGlobal),
                               Qt::NoButton,
                               Qt::LeftButton,
                               Qt::NoModifier);
    QCoreApplication::sendEvent(floatingSourceTabBar, &floatMoveEvent);
    QMouseEvent floatReleaseEvent(QEvent::MouseButtonRelease,
                                  QPointF(outsideManagerLocal),
                                  QPointF(outsideManagerGlobal),
                                  Qt::LeftButton,
                                  Qt::NoButton,
                                  Qt::NoModifier);
    QCoreApplication::sendEvent(floatingSourceTabBar, &floatReleaseEvent);
    QTRY_VERIFY(explorer->isFloating());
    QTRY_VERIFY(explorer->titleBarWidget()->isVisible());
    QCOMPARE(explorer->property("antDockFloatingFrame").toBool(), true);
    QCOMPARE(explorer->property("antDockFloatingShadowMargin").toInt(), 0);
    QVERIFY(explorer->property("antDockFloatingCornerRadius").toInt() >= 8);
    QCOMPARE(explorer->property("antDockFloatingTitleBarHeight").toInt(), 40);
    QCOMPARE(explorer->titleBarWidget()->property("antDockTitleBarHeight").toInt(), 40);
    QCOMPARE(explorer->windowType(), Qt::Window);
    QVERIFY(explorer->windowFlags().testFlag(Qt::FramelessWindowHint));
    QVERIFY(!explorer->testAttribute(Qt::WA_TranslucentBackground));
    QVERIFY(explorer->windowHandle() != nullptr);
    QVERIFY(manager->windowHandle() != nullptr);
    QCOMPARE(explorer->windowHandle()->transientParent(), manager->windowHandle());
    QCOMPARE(explorer->property("antDockFloatingOwnedByManager").toBool(), true);
#if defined(Q_OS_WIN)
    QTRY_VERIFY(explorer->property("antDockNativeWindowFrameEnabled").toBool());
    QVERIFY(explorer->property("antDockDwmFrameApplyCount").toInt() > 0);
    QCOMPARE(explorer->property("antDockFloatingNativeOwnedByManager").toBool(), true);
#endif
    QVERIFY(dockAreaForExtensionTest(explorer) == nullptr);
    QVERIFY(manager->isDockWidgetFloating(explorer));
    QVERIFY(manager->floatingDockWidgets().contains(explorer));
    const QSize floatingResize = explorer->size().expandedTo(QSize(280, 180));
    explorer->setGeometry(QRect(manager->mapToGlobal(QPoint(64, 64)), floatingResize));
    QCOMPARE(explorer->size(), floatingResize);

    manager->setDockWidgetFloatable(explorer, false);
    QWidget* floatingTitleBarForMenu = explorer->titleBarWidget();
    QVERIFY(floatingTitleBarForMenu != nullptr);
    const QPoint floatingMenuPoint = floatingTitleBarForMenu->rect().center();
    QContextMenuEvent floatingContextEvent(
        QContextMenuEvent::Mouse,
        floatingMenuPoint,
        floatingTitleBarForMenu->mapToGlobal(floatingMenuPoint));
    QCoreApplication::sendEvent(floatingTitleBarForMenu, &floatingContextEvent);
    QWidget* floatingPopup = nullptr;
    for (QWidget* widget : QApplication::topLevelWidgets())
    {
        if (widget && widget->objectName() == QStringLiteral("AntDockContextMenuPopup") && widget->isVisible())
        {
            floatingPopup = widget;
            break;
        }
    }
    QVERIFY(floatingPopup != nullptr);
    auto* floatingDockMenu = floatingPopup->findChild<AntMenu*>(QStringLiteral("AntDockContextMenu"));
    QVERIFY(floatingDockMenu != nullptr);
    const AntMenuItem floatingAction = menuItemByKey(floatingDockMenu, QStringLiteral("float"));
    QCOMPARE(floatingAction.label, QStringLiteral("Dock to workspace"));
    QCOMPARE(floatingAction.disabled, false);
    floatingPopup->close();
    QCoreApplication::processEvents();
    manager->setDockWidgetFloatable(explorer, true);

    auto* floatingClickSwitch = new AntSwitch(manager);
    floatingClickSwitch->setObjectName(QStringLiteral("dockFloatingClickSwitch"));
    floatingClickSwitch->setGeometry(16, 16, 92, 28);
    floatingClickSwitch->show();
    floatingClickSwitch->raise();
    QSignalSpy floatingClickSwitchSpy(floatingClickSwitch, &AntSwitch::checkedChanged);
#if defined(Q_OS_WIN)
    if (nativeMouseInputAvailableForExtensionTest())
    {
        const QPoint floatingSwitchGlobal = floatingClickSwitch->mapToGlobal(floatingClickSwitch->rect().center());
        QVERIFY(nativeMouseClickForExtensionTest(manager, floatingSwitchGlobal));
    }
    else
    {
        QTest::mousePress(floatingClickSwitch,
                          Qt::LeftButton,
                          Qt::NoModifier,
                          floatingClickSwitch->rect().center());
        QTest::mouseRelease(floatingClickSwitch,
                            Qt::LeftButton,
                            Qt::NoModifier,
                            floatingClickSwitch->rect().center());
    }
#else
    QTest::mousePress(floatingClickSwitch,
                      Qt::LeftButton,
                      Qt::NoModifier,
                      floatingClickSwitch->rect().center());
    QTest::mouseRelease(floatingClickSwitch,
                        Qt::LeftButton,
                        Qt::NoModifier,
                        floatingClickSwitch->rect().center());
#endif
    QTRY_COMPARE(floatingClickSwitchSpy.count(), 1);
    QVERIFY(floatingClickSwitch->isChecked());
#if defined(Q_OS_WIN)
    if (explorer->property("antDockLegacySoftwareShadowEnabled").toBool())
    {
        QWidget* shadow = nullptr;
        QTRY_VERIFY((shadow = explorer->findChild<QWidget*>(QStringLiteral("AntDockLegacySoftwareShadow"))) != nullptr);
        QVERIFY(shadow->testAttribute(Qt::WA_TransparentForMouseEvents));
        QTRY_COMPARE(shadow->property("antDockLegacySoftwareShadowClickThrough").toBool(), true);
        QTRY_COMPARE(shadow->property("antDockLegacySoftwareShadowRingRegion").toBool(), true);
    }
    if (!explorer->property("antDockUsesNativeCaptionFrame").toBool())
    {
        const HWND explorerHwnd = reinterpret_cast<HWND>(explorer->winId());
        ::SendMessageW(explorerHwnd, WM_ENTERSIZEMOVE, 0, 0);
        QTRY_COMPARE(explorer->property("antDockLegacyLiveResize").toBool(), true);
        QCOMPARE(explorer->property("antDockLegacySoftwareShadowEnabled").toBool(), false);
        ::SendMessageW(explorerHwnd, WM_EXITSIZEMOVE, 0, 0);
        QTRY_COMPARE(explorer->property("antDockLegacyLiveResize").toBool(), false);
        QTRY_VERIFY(explorer->property("antDockFloatingRefreshCount").toInt() > 0);

        HDC eraseDc = ::GetDC(explorerHwnd);
        QVERIFY(eraseDc != nullptr);
        const LRESULT eraseResult =
            ::SendMessageW(explorerHwnd, WM_ERASEBKGND, reinterpret_cast<WPARAM>(eraseDc), 0);
        ::ReleaseDC(explorerHwnd, eraseDc);
        QCOMPARE(eraseResult, static_cast<LRESULT>(1));
        QCOMPARE(explorer->property("antDockNativeEraseBackgroundFilled").toBool(), true);

        MSG outsideHitTestMessage{};
        outsideHitTestMessage.hwnd = explorerHwnd;
        outsideHitTestMessage.message = WM_NCHITTEST;
        const QPoint outsideFrameGlobal = explorer->mapToGlobal(QPoint(-4, explorer->height() / 2));
        outsideHitTestMessage.lParam = MAKELPARAM(outsideFrameGlobal.x(), outsideFrameGlobal.y());
        const qintptr outsideHitTestResult =
            ::SendMessageW(explorerHwnd, outsideHitTestMessage.message, 0, outsideHitTestMessage.lParam);
        QCOMPARE(outsideHitTestResult, static_cast<qintptr>(HTTRANSPARENT));
        QCOMPARE(explorer->property("antDockLegacyOutsideClientHitTestTransparent").toBool(), true);
    }
#endif
    QVERIFY(manager->savePerspective(QStringLiteral("floating")));
    const QRect savedFloatingGeometry = explorer->geometry();
#if defined(Q_OS_WIN)
    if (nativeMouseInputAvailableForExtensionTest())
    {
        dragFloatingDockBackWithNativeMouse(explorer, inspector);
    }
    else
    {
        dragFloatingDockBack(explorer, inspector);
    }
#else
    dragFloatingDockBack(explorer, inspector);
#endif
    QTRY_VERIFY(!explorer->isFloating());
    QVERIFY(manager->restorePerspective(QStringLiteral("floating")));
    QTRY_VERIFY(explorer->isFloating());
    QCOMPARE(explorer->geometry().size(), savedFloatingGeometry.size());
    QVERIFY((explorer->geometry().topLeft() - savedFloatingGeometry.topLeft()).manhattanLength() <= 4);
    QVERIFY(manager->removePerspective(QStringLiteral("floating")));
    QVERIFY(floatedSpy.count() >= 1);

    manager->setDockWidgetFloating(explorer, false);
    QTRY_VERIFY(!manager->isDockWidgetFloating(explorer));
    QVERIFY(dockAreaForExtensionTest(explorer) != nullptr);
    QCOMPARE(QWidget::mouseGrabber(), nullptr);
    if (QWidget* dragOverlay = topLevelWidgetForExtensionTest(QStringLiteral("AntDockDragPreviewWindow")))
    {
        QVERIFY(!dragOverlay->isVisible());
    }
    if (QWidget* dropOverlay = topLevelWidgetForExtensionTest(QStringLiteral("AntDockDropPreviewWindow")))
    {
        QVERIFY(!dropOverlay->isVisible());
    }
    QVERIFY(!explorer->isWindow());
    QVERIFY(!explorer->windowFlags().testFlag(Qt::Window));
    QVERIFY(!explorer->testAttribute(Qt::WA_TransparentForMouseEvents));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    QVERIFY(!explorer->windowFlags().testFlag(Qt::WindowTransparentForInput));
#endif
#if defined(Q_OS_WIN)
    QCOMPARE(explorer->property("antDockNativeMouseCaptureCleared").toBool(), true);
    QCOMPARE(explorer->property("antDockNativeFloatingHwndDestroyed").toBool(), true);
    QCOMPARE(explorer->property("antDockNativeEmbeddedChildFrame").toBool(), true);
    if (const WId explorerId = explorer->internalWinId())
    {
        HWND capturedHwnd = ::GetCapture();
        HWND explorerHwnd = reinterpret_cast<HWND>(explorerId);
        QVERIFY(!capturedHwnd || (capturedHwnd != explorerHwnd && !::IsChild(explorerHwnd, capturedHwnd)));
        const LONG_PTR nativeStyle = ::GetWindowLongPtrW(explorerHwnd, GWL_STYLE);
        const LONG_PTR nativeExStyle = ::GetWindowLongPtrW(explorerHwnd, GWL_EXSTYLE);
        QVERIFY((nativeStyle & WS_CHILD) != 0);
        QVERIFY((nativeStyle & WS_POPUP) == 0);
        QVERIFY((nativeExStyle & WS_EX_TOPMOST) == 0);
        QVERIFY((nativeExStyle & WS_EX_TRANSPARENT) == 0);
        QVERIFY((nativeExStyle & WS_EX_NOACTIVATE) == 0);
    }
#endif
    QVERIFY(dockedSpy.count() >= 1);
    manager->setDockWidgetFloating(explorer, true, savedFloatingGeometry);
    QTRY_VERIFY(manager->isDockWidgetFloating(explorer));
    QCOMPARE(explorer->geometry().size(), savedFloatingGeometry.size());

    QWidget* floatingTitleBar = explorer->titleBarWidget();
    QVERIFY(floatingTitleBar != nullptr);
    const QPoint floatingTitlePoint = floatingTitleBar->rect().center();
    const QPoint floatingTitleGlobal = floatingTitleBar->mapToGlobal(floatingTitlePoint);
    const QRect floatingGeometryBeforeClick = explorer->geometry();
    QWidget* floatingContent = explorer->widget();
    QVERIFY(floatingContent != nullptr);
    const QSize floatingContentSizeBeforeMaximize = floatingContent->size();
    QMouseEvent floatingClickPress(QEvent::MouseButtonPress,
                                   QPointF(floatingTitlePoint),
                                   QPointF(floatingTitleGlobal),
                                   Qt::LeftButton,
                                   Qt::LeftButton,
                                   Qt::NoModifier);
    QCoreApplication::sendEvent(floatingTitleBar, &floatingClickPress);
    QMouseEvent floatingClickRelease(QEvent::MouseButtonRelease,
                                     QPointF(floatingTitlePoint),
                                     QPointF(floatingTitleGlobal),
                                     Qt::LeftButton,
                                     Qt::NoButton,
                                     Qt::NoModifier);
    QCoreApplication::sendEvent(floatingTitleBar, &floatingClickRelease);
    QTRY_VERIFY(!manager->isDropPreviewVisible());
    QCOMPARE(manager->property("antDockDragActivated").toBool(), false);
    QVERIFY(explorer->graphicsEffect() == nullptr);
    QCOMPARE(explorer->geometry(), floatingGeometryBeforeClick);

    QMouseEvent floatingDoubleClick(QEvent::MouseButtonDblClick,
                                    QPointF(floatingTitlePoint),
                                    QPointF(floatingTitleGlobal),
                                    Qt::LeftButton,
                                    Qt::LeftButton,
                                    Qt::NoModifier);
    QCoreApplication::sendEvent(floatingTitleBar, &floatingDoubleClick);
    QMouseEvent floatingDoubleRelease(QEvent::MouseButtonRelease,
                                      QPointF(floatingTitlePoint),
                                      QPointF(floatingTitleGlobal),
                                      Qt::LeftButton,
                                      Qt::NoButton,
                                      Qt::NoModifier);
    QCoreApplication::sendEvent(floatingTitleBar, &floatingDoubleRelease);
    QTRY_VERIFY(!manager->isDropPreviewVisible());
    QCOMPARE(manager->property("antDockDragActivated").toBool(), false);
    QVERIFY(explorer->graphicsEffect() == nullptr);
    QTRY_VERIFY(explorer->isFloating());
    if (explorer->isMaximized())
    {
        QTRY_VERIFY(floatingContent->width() > floatingContentSizeBeforeMaximize.width() ||
                    floatingContent->height() > floatingContentSizeBeforeMaximize.height());
#if defined(Q_OS_WIN)
        if (!explorer->property("antDockUsesNativeCaptionFrame").toBool())
        {
            QCOMPARE(explorer->property("antDockLegacyRoundedMaskApplied").toBool(), false);
            QTRY_VERIFY(explorer->property("antDockFloatingRefreshCount").toInt() > 0);
            QVERIFY(explorer->property("antDockFloatingRefreshReason").toString().startsWith(QStringLiteral("legacy-")));
        }
#endif
        explorer->showNormal();
        QTRY_VERIFY(!explorer->isMaximized());
    }

    dragFloatingDockBack(explorer, inspector);
    QTRY_VERIFY(!explorer->isFloating());
    QTRY_VERIFY(dockAreaForExtensionTest(explorer) != nullptr);
    QCOMPARE(QWidget::mouseGrabber(), nullptr);
    if (QWidget* dragOverlay = topLevelWidgetForExtensionTest(QStringLiteral("AntDockDragPreviewWindow")))
    {
        QVERIFY(!dragOverlay->isVisible());
    }
    if (QWidget* dropOverlay = topLevelWidgetForExtensionTest(QStringLiteral("AntDockDropPreviewWindow")))
    {
        QVERIFY(!dropOverlay->isVisible());
    }
    QTRY_VERIFY(!explorer->titleBarWidget()->isVisible());
    QCOMPARE(explorer->property("antDockFloatingFrame").toBool(), false);
    QCOMPARE(explorer->property("antDockFloatingOwnedByManager").toBool(), false);
    QVERIFY(!explorer->isWindow());
    QVERIFY(!explorer->windowFlags().testFlag(Qt::Window));
    QVERIFY(!explorer->testAttribute(Qt::WA_TransparentForMouseEvents));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    QVERIFY(!explorer->windowFlags().testFlag(Qt::WindowTransparentForInput));
#endif
#if defined(Q_OS_WIN)
    QCOMPARE(explorer->property("antDockNativeMouseCaptureCleared").toBool(), true);
    QCOMPARE(explorer->property("antDockNativeFloatingHwndDestroyed").toBool(), true);
    QCOMPARE(explorer->property("antDockNativeEmbeddedChildFrame").toBool(), true);
    if (const WId explorerId = explorer->internalWinId())
    {
        HWND capturedHwnd = ::GetCapture();
        HWND explorerHwnd = reinterpret_cast<HWND>(explorerId);
        QVERIFY(!capturedHwnd || (capturedHwnd != explorerHwnd && !::IsChild(explorerHwnd, capturedHwnd)));
        const LONG_PTR nativeStyle = ::GetWindowLongPtrW(explorerHwnd, GWL_STYLE);
        const LONG_PTR nativeExStyle = ::GetWindowLongPtrW(explorerHwnd, GWL_EXSTYLE);
        QVERIFY((nativeStyle & WS_CHILD) != 0);
        QVERIFY((nativeStyle & WS_POPUP) == 0);
        QVERIFY((nativeExStyle & WS_EX_TOPMOST) == 0);
        QVERIFY((nativeExStyle & WS_EX_TRANSPARENT) == 0);
        QVERIFY((nativeExStyle & WS_EX_NOACTIVATE) == 0);
    }
#endif

    auto* embeddedClickSwitch = new AntSwitch(manager);
    embeddedClickSwitch->setObjectName(QStringLiteral("dockEmbeddedClickSwitch"));
    embeddedClickSwitch->setGeometry(124, 16, 92, 28);
    embeddedClickSwitch->show();
    embeddedClickSwitch->raise();
    QTRY_VERIFY(embeddedClickSwitch->isVisible());
    QWidget* embeddedHitWidget = QApplication::widgetAt(embeddedClickSwitch->mapToGlobal(embeddedClickSwitch->rect().center()));
    QVERIFY(embeddedHitWidget == embeddedClickSwitch || embeddedClickSwitch->isAncestorOf(embeddedHitWidget));
#if defined(Q_OS_WIN)
    if (nativeMouseInputAvailableForExtensionTest())
    {
        QVERIFY(bringWidgetToFrontForExtensionTest(manager));
        HWND managerRootHwnd = rootHwndForExtensionTest(manager);
        QVERIFY(managerRootHwnd != nullptr);
        const QPoint embeddedSwitchGlobal = embeddedClickSwitch->mapToGlobal(embeddedClickSwitch->rect().center());
        const QPoint embeddedSwitchNative = nativePointForExtensionTest(manager, embeddedSwitchGlobal);
        POINT nativePoint{embeddedSwitchNative.x(), embeddedSwitchNative.y()};
        HWND nativeAtPoint = ::WindowFromPoint(nativePoint);
        QVERIFY(nativeAtPoint != nullptr);
        HWND pointRootHwnd = ::GetAncestor(nativeAtPoint, GA_ROOT);
        DWORD pointProcessId = 0;
        ::GetWindowThreadProcessId(pointRootHwnd, &pointProcessId);
        QCOMPARE(pointProcessId, ::GetCurrentProcessId());
        QCOMPARE(pointRootHwnd, managerRootHwnd);
        if (const WId explorerId = explorer->internalWinId())
        {
            HWND explorerHwnd = reinterpret_cast<HWND>(explorerId);
            QVERIFY(nativeAtPoint != explorerHwnd);
            QVERIFY(!::IsChild(explorerHwnd, nativeAtPoint));
        }
        releaseTopMostForExtensionTest(manager);
    }
#endif
    QSignalSpy embeddedClickSwitchSpy(embeddedClickSwitch, &AntSwitch::checkedChanged);
#if defined(Q_OS_WIN)
    if (nativeMouseInputAvailableForExtensionTest())
    {
        const QPoint embeddedSwitchGlobal = embeddedClickSwitch->mapToGlobal(embeddedClickSwitch->rect().center());
        QVERIFY(nativeMouseClickForExtensionTest(manager, embeddedSwitchGlobal));
    }
    else
    {
        QTest::mousePress(embeddedClickSwitch,
                          Qt::LeftButton,
                          Qt::NoModifier,
                          embeddedClickSwitch->rect().center());
        QTest::mouseRelease(embeddedClickSwitch,
                            Qt::LeftButton,
                            Qt::NoModifier,
                            embeddedClickSwitch->rect().center());
    }
#else
    QTest::mousePress(embeddedClickSwitch,
                      Qt::LeftButton,
                      Qt::NoModifier,
                      embeddedClickSwitch->rect().center());
    QTest::mouseRelease(embeddedClickSwitch,
                        Qt::LeftButton,
                        Qt::NoModifier,
                        embeddedClickSwitch->rect().center());
#endif
    QTRY_COMPARE(embeddedClickSwitchSpy.count(), 1);
    QVERIFY(embeddedClickSwitch->isChecked());

    QTabBar* doubleClickTabBar = dockTabBarForExtensionTest(explorer);
    QVERIFY(doubleClickTabBar != nullptr);
    const QPoint doubleClickPoint = dockTabCenterForExtensionTest(explorer);
    QVERIFY(!doubleClickPoint.isNull());
    QMouseEvent doubleClickEvent(QEvent::MouseButtonDblClick,
                                 QPointF(doubleClickPoint),
                                 QPointF(doubleClickTabBar->mapToGlobal(doubleClickPoint)),
                                 Qt::LeftButton,
                                 Qt::LeftButton,
                                 Qt::NoModifier);
    QCoreApplication::sendEvent(doubleClickTabBar, &doubleClickEvent);
    QTRY_VERIFY(explorer->isFloating());
    QTRY_VERIFY(explorer->titleBarWidget()->isVisible());
    QCOMPARE(explorer->property("antDockFloatingFrame").toBool(), true);
    QCOMPARE(explorer->property("antDockFloatingTitleBarHeight").toInt(), 40);
    QCOMPARE(explorer->titleBarWidget()->property("antDockTitleBarHeight").toInt(), 40);
    QCOMPARE(explorer->windowType(), Qt::Window);
    QVERIFY(explorer->windowFlags().testFlag(Qt::FramelessWindowHint));
    QVERIFY(!explorer->testAttribute(Qt::WA_TranslucentBackground));
    QVERIFY(explorer->windowHandle() != nullptr);
    QVERIFY(manager->windowHandle() != nullptr);
    QCOMPARE(explorer->windowHandle()->transientParent(), manager->windowHandle());
    QCOMPARE(explorer->property("antDockFloatingOwnedByManager").toBool(), true);
#if defined(Q_OS_WIN)
    QTRY_VERIFY(explorer->property("antDockNativeWindowFrameEnabled").toBool());
    QVERIFY(explorer->property("antDockDwmFrameApplyCount").toInt() > 0);
    QCOMPARE(explorer->property("antDockFloatingNativeOwnedByManager").toBool(), true);
#endif

    dragFloatingDockBack(explorer, inspector);
    QTRY_VERIFY(!explorer->isFloating());
    QTRY_VERIFY(dockAreaForExtensionTest(explorer) != nullptr);
    QTRY_VERIFY(!explorer->titleBarWidget()->isVisible());
    QCOMPARE(explorer->property("antDockFloatingFrame").toBool(), false);
    QCOMPARE(explorer->property("antDockFloatingOwnedByManager").toBool(), false);

    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCOMPARE(manager->palette().color(QPalette::Window), antTheme->tokens().colorBgLayout);
    QCOMPARE(explorer->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);

    QSignalSpy removedSpy(manager, &AntDockManager::dockWidgetRemoved);
    manager->removeDockWidget(preview);
    QCOMPARE(removedSpy.count(), 1);
    QVERIFY(!manager->dockWidgets().contains(preview));
    delete preview;
    delete manager;

    {
        auto* hiddenManager = new AntDockManager;
        hiddenManager->setGeometry(60, 60, 520, 360);
        hiddenManager->show();
        QVERIFY(QTest::qWaitForWindowExposed(hiddenManager));

        auto* hiddenExplorer = new AntDockWidget(QStringLiteral("Hidden Explorer"));
        hiddenExplorer->setWidget(new QWidget);
        auto* hiddenInspector = new AntDockWidget(QStringLiteral("Hidden Inspector"));
        hiddenInspector->setWidget(new QWidget);
        hiddenManager->addDockWidget(Qt::LeftDockWidgetArea, hiddenExplorer);
        hiddenManager->splitDockWidget(hiddenExplorer, hiddenInspector, Qt::Horizontal);
        hiddenManager->setDockWidgetFloating(
            hiddenExplorer,
            true,
            QRect(hiddenManager->mapToGlobal(QPoint(96, 96)), QSize(260, 180)));
        QTRY_VERIFY(hiddenExplorer->isFloating());
        QTRY_VERIFY(hiddenExplorer->titleBarWidget()->isVisible());

        const QPoint floatingPosBeforeHiddenDrag = hiddenExplorer->pos();
        hiddenManager->hide();
        QCoreApplication::processEvents(QEventLoop::AllEvents, 80);
        hiddenExplorer->show();
        hiddenExplorer->raise();
        QTRY_VERIFY(hiddenExplorer->isVisible());

        QWidget* hiddenTitleBar = hiddenExplorer->titleBarWidget();
        QVERIFY(hiddenTitleBar != nullptr);
        const QPoint pressLocal = hiddenTitleBar->rect().center();
        const QPoint pressGlobal = hiddenTitleBar->mapToGlobal(pressLocal);
        const QPoint releaseGlobal = pressGlobal + QPoint(96, 52);
        const QPoint releaseLocal = hiddenTitleBar->mapFromGlobal(releaseGlobal);
        QMouseEvent hiddenPress(QEvent::MouseButtonPress,
                                QPointF(pressLocal),
                                QPointF(pressGlobal),
                                Qt::LeftButton,
                                Qt::LeftButton,
                                Qt::NoModifier);
        QCoreApplication::sendEvent(hiddenTitleBar, &hiddenPress);
        QMouseEvent hiddenMove(QEvent::MouseMove,
                               QPointF(releaseLocal),
                               QPointF(releaseGlobal),
                               Qt::NoButton,
                               Qt::LeftButton,
                               Qt::NoModifier);
        QCoreApplication::sendEvent(hiddenTitleBar, &hiddenMove);
        QMouseEvent hiddenRelease(QEvent::MouseButtonRelease,
                                  QPointF(releaseLocal),
                                  QPointF(releaseGlobal),
                                  Qt::LeftButton,
                                  Qt::NoButton,
                                  Qt::NoModifier);
        QCoreApplication::sendEvent(hiddenTitleBar, &hiddenRelease);

        const QPoint expectedFloatingPos = floatingPosBeforeHiddenDrag + (releaseGlobal - pressGlobal);
        QTRY_VERIFY((hiddenExplorer->pos() - expectedFloatingPos).manhattanLength() <= 4);
        QCOMPARE(hiddenManager->property("antDockDragActivated").toBool(), false);
        QVERIFY(!hiddenManager->isDropPreviewVisible());
        QCOMPARE(hiddenManager->activeDropGuide(), AntDockManager::DockPlacement::None);

        QSignalSpy hiddenRemovedSpy(hiddenManager, &AntDockManager::dockWidgetRemoved);
        hiddenManager->removeDockWidget(hiddenExplorer);
        QCOMPARE(hiddenRemovedSpy.count(), 1);
        QVERIFY(!hiddenManager->dockWidgets().contains(hiddenExplorer));
        QVERIFY(!hiddenManager->floatingDockWidgets().contains(hiddenExplorer));
        QCOMPARE(hiddenExplorer->property("antDockFloatingOwnedByManager").toBool(), false);
        QTRY_VERIFY(!hiddenExplorer->isVisible());
        hiddenManager->removeDockWidget(hiddenInspector);
        delete hiddenExplorer;
        delete hiddenInspector;
        delete hiddenManager;
    }

#if defined(Q_OS_WIN)
    if (nativeMouseInputAvailableForExtensionTest())
    {
        AntWindow host;
        host.setGeometry(80, 80, 900, 620);
        auto* page = new QWidget;
        auto* pageLayout = new QVBoxLayout(page);
        pageLayout->setContentsMargins(24, 24, 24, 24);
        pageLayout->setSpacing(12);

        auto* pageSwitch = new AntSwitch(page);
        pageSwitch->setObjectName(QStringLiteral("dockAntWindowPageSwitch"));
        pageSwitch->setFixedSize(92, 28);
        pageLayout->addWidget(pageSwitch);

        auto* windowManager = new AntDockManager(page);
        windowManager->setMinimumHeight(360);
        pageLayout->addWidget(windowManager, 1);

        host.setCentralWidget(page);
        host.show();
        page->show();
        pageSwitch->show();
        windowManager->show();
        QVERIFY(QTest::qWaitForWindowExposed(&host));
        QTRY_VERIFY(page->isVisible());
        QTRY_VERIFY(pageSwitch->isVisible());
        QTRY_VERIFY(windowManager->isVisible());

        auto* windowExplorer = new AntDockWidget(QStringLiteral("Explorer"));
        windowExplorer->setWidget(new QWidget);
        auto* windowInspector = new AntDockWidget(QStringLiteral("Inspector"));
        windowInspector->setWidget(new QWidget);
        windowManager->addDockWidget(Qt::LeftDockWidgetArea, windowExplorer);
        windowManager->splitDockWidget(windowExplorer, windowInspector, Qt::Horizontal);
        QTRY_VERIFY(dockAreaForExtensionTest(windowExplorer) != nullptr);
        QTRY_VERIFY(dockAreaForExtensionTest(windowInspector) != nullptr);

        QSignalSpy switchBeforeSpy(pageSwitch, &AntSwitch::checkedChanged);
        QVERIFY(nativeMouseClickForExtensionTest(&host, pageSwitch->mapToGlobal(pageSwitch->rect().center())));
        QTRY_COMPARE(switchBeforeSpy.count(), 1);
        QVERIFY(pageSwitch->isChecked());

        windowManager->setDockWidgetFloating(
            windowExplorer,
            true,
            QRect(windowManager->mapToGlobal(QPoint(96, 96)), QSize(280, 190)));
        QTRY_VERIFY(windowExplorer->isFloating());

        dragFloatingDockBack(windowExplorer, windowInspector);
        QTRY_VERIFY(!windowExplorer->isFloating());
        QTRY_VERIFY(dockAreaForExtensionTest(windowExplorer) != nullptr);

        QSignalSpy switchAfterSpy(pageSwitch, &AntSwitch::checkedChanged);
        QVERIFY(bringWidgetToFrontForExtensionTest(&host));
        {
            const QPoint switchGlobal = pageSwitch->mapToGlobal(pageSwitch->rect().center());
            const QPoint switchNative = nativePointForExtensionTest(&host, switchGlobal);
            POINT nativePoint{switchNative.x(), switchNative.y()};
            HWND nativeAtPoint = ::WindowFromPoint(nativePoint);
            QVERIFY(nativeAtPoint != nullptr);
            QWidget* nativeWidget = QWidget::find(reinterpret_cast<WId>(nativeAtPoint));
            QVERIFY(!nativeWidget || nativeWidget->objectName() != QStringLiteral("AntWindowCornerSmoother"));
            QVERIFY(qobject_cast<AntWave*>(nativeWidget) == nullptr);
            QWidget* smoother = host.findChild<QWidget*>(QStringLiteral("AntWindowCornerSmoother"));
            QVERIFY(smoother != nullptr);
            QCOMPARE(smoother->property("antWindowCornerSmootherClickThrough").toBool(), true);
        }
        releaseTopMostForExtensionTest(&host);
        QVERIFY(nativeMouseClickForExtensionTest(&host, pageSwitch->mapToGlobal(pageSwitch->rect().center())));
        QTRY_COMPARE(switchAfterSpy.count(), 1);
        QVERIFY(!pageSwitch->isChecked());
    }
#endif

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
    QCOMPARE(w->cornerRadius(), 8);
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

    QSignalSpy cornerSpy(w, &AntWindow::cornerRadiusChanged);
    w->setCornerRadius(12);
    QCOMPARE(w->cornerRadius(), 12);
    QCOMPARE(cornerSpy.count(), 1);
    w->setCornerRadius(-1);
    QCOMPARE(w->cornerRadius(), 0);
    QCOMPARE(cornerSpy.count(), 2);

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

void TestAntQtExtensions::windowAlwaysOnTopDoesNotRecreateVisibleWindow()
{
#ifndef Q_OS_WIN
    QSKIP("Windows topmost behavior is verified through native SetWindowPos.");
#else
    class VisibleStateWindow : public AntWindow
    {
    public:
        int showEvents = 0;
        int hideEvents = 0;

    protected:
        void showEvent(QShowEvent* event) override
        {
            ++showEvents;
            AntWindow::showEvent(event);
        }

        void hideEvent(QHideEvent* event) override
        {
            ++hideEvents;
            QMainWindow::hideEvent(event);
        }
    };

    VisibleStateWindow window;
    window.resize(640, 420);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const HWND hwnd = reinterpret_cast<HWND>(window.winId());
    QVERIFY(hwnd != nullptr);
    window.showEvents = 0;
    window.hideEvents = 0;

    QSignalSpy alwaysOnTopSpy(&window, &AntWindow::alwaysOnTopChanged);
    window.setAlwaysOnTop(true);
    QCoreApplication::processEvents();
    QCOMPARE(window.isAlwaysOnTop(), true);
    QCOMPARE(alwaysOnTopSpy.count(), 1);
    QCOMPARE(window.showEvents, 0);
    QCOMPARE(window.hideEvents, 0);
    QVERIFY(window.isVisible());
    QVERIFY((::GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0);

    window.setAlwaysOnTop(false);
    QCoreApplication::processEvents();
    QCOMPARE(window.isAlwaysOnTop(), false);
    QCOMPARE(alwaysOnTopSpy.count(), 2);
    QCOMPARE(window.showEvents, 0);
    QCOMPARE(window.hideEvents, 0);
    QVERIFY(window.isVisible());
    QVERIFY((::GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == 0);
#endif
}

void TestAntQtExtensions::windowTitleBarHoverStateClearsOnLeave()
{
    AntWindow window;
    window.resize(640, 420);
    window.setCentralWidget(new QWidget);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QPoint closePoint = window.titleBarButtonRect(AntWindow::TitleBarButton::Close).center();
    QHoverEvent closeHover(QEvent::HoverMove, QPointF(closePoint), QPointF(closePoint - QPoint(1, 0)));
    QCoreApplication::sendEvent(&window, &closeHover);
    QCOMPARE(window.hoveredTitleBarButton(), AntWindow::TitleBarButton::Close);

    QEvent windowLeave(QEvent::Leave);
    QCoreApplication::sendEvent(&window, &windowLeave);
    QCOMPARE(window.hoveredTitleBarButton(), AntWindow::TitleBarButton::None);

    QWidget* titleBarEventTarget = window.centralWidget();
    QVERIFY(titleBarEventTarget != nullptr);
    QVERIFY(titleBarEventTarget->testAttribute(Qt::WA_Hover));

    const QPoint themePoint = window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center();
    const QPoint contentThemePoint = titleBarEventTarget->mapFrom(&window, themePoint);
    QHoverEvent themeHover(QEvent::HoverMove, QPointF(contentThemePoint), QPointF(contentThemePoint - QPoint(1, 0)));
    QCoreApplication::sendEvent(titleBarEventTarget, &themeHover);
    QCOMPARE(window.hoveredTitleBarButton(), AntWindow::TitleBarButton::Theme);

    QEvent contentLeave(QEvent::Leave);
    QCoreApplication::sendEvent(titleBarEventTarget, &contentLeave);
    QCOMPARE(window.hoveredTitleBarButton(), AntWindow::TitleBarButton::None);
}

void TestAntQtExtensions::windowThemeButtonShowsTransitionOverlay()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    AntWindow window;
    window.resize(640, 420);
    window.setCentralWidget(new QWidget);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QTest::mouseClick(&window,
                      Qt::LeftButton,
                      Qt::NoModifier,
                      window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center());
    QCOMPARE(antTheme->themeMode(), Ant::ThemeMode::Dark);

    auto* overlay = window.findChild<QWidget*>(QStringLiteral("AntWindowThemeTransitionOverlay"));
    QVERIFY(overlay != nullptr);
    QVERIFY(overlay->isVisible());
    QVERIFY(overlay->testAttribute(Qt::WA_TransparentForMouseEvents));
    QCOMPARE(overlay->geometry(), window.rect());
    QCOMPARE(overlay->property("transitionFrameIntervalMs").toInt(), 8);
    QCOMPARE(overlay->property("transitionDurationMs").toInt(), 320);
    QCOMPARE(overlay->property("transitionMotionCurve").toString(), QStringLiteral("smootherstep"));
    QCOMPARE(overlay->property("transitionEdgeFeather").toInt(), 24);
    QCOMPARE(overlay->property("transitionDrawsCapturedNewFrame").toBool(), true);

    QTRY_VERIFY(window.findChild<QWidget*>(QStringLiteral("AntWindowThemeTransitionOverlay")) == nullptr);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::windowThemeTransitionOverlayKeepsOldFrameScale()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    class SplitPaintWidget : public QWidget
    {
    public:
        explicit SplitPaintWidget(QWidget* parent = nullptr)
            : QWidget(parent)
        {
            setAutoFillBackground(false);
        }

    protected:
        void paintEvent(QPaintEvent*) override
        {
            QPainter painter(this);
            painter.fillRect(rect(), QColor(220, 30, 30));
            painter.fillRect(QRect(400, 0, qMax(0, width() - 400), height()), QColor(20, 90, 230));
        }
    };

    AntWindow window;
    window.resize(640, 420);
    window.setCentralWidget(new SplitPaintWidget);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    if (window.devicePixelRatioF() <= 1.01)
    {
        QSKIP("High-DPI frame scale guard only applies when devicePixelRatioF > 1.");
    }

    QTest::mouseClick(&window,
                      Qt::LeftButton,
                      Qt::NoModifier,
                      window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center());
    auto* overlay = window.findChild<QWidget*>(QStringLiteral("AntWindowThemeTransitionOverlay"));
    QVERIFY(overlay != nullptr);

    QImage rendered(overlay->size(), QImage::Format_ARGB32_Premultiplied);
    rendered.fill(Qt::transparent);
    QPainter painter(&rendered);
    overlay->render(&painter);
    painter.end();

    const QColor rightSide = rendered.pixelColor(520, 220);
    QVERIFY2(rightSide.blue() > 170 && rightSide.red() < 120,
             qPrintable(QStringLiteral("Overlay old frame was scaled incorrectly: sampled rgba(%1,%2,%3,%4)")
                            .arg(rightSide.red())
                            .arg(rightSide.green())
                            .arg(rightSide.blue())
                            .arg(rightSide.alpha())));

    QTRY_VERIFY(window.findChild<QWidget*>(QStringLiteral("AntWindowThemeTransitionOverlay")) == nullptr);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::windowThemeTransitionRevealsNewFrameWithoutBlackHole()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    class ThemePaintWidget : public QWidget
    {
    public:
        explicit ThemePaintWidget(QWidget* parent = nullptr)
            : QWidget(parent)
        {
            setAutoFillBackground(false);
            connect(antTheme, &AntTheme::themeChanged, this, [this]() { update(); });
        }

    protected:
        void paintEvent(QPaintEvent*) override
        {
            QPainter painter(this);
            painter.fillRect(rect(),
                             antTheme->themeMode() == Ant::ThemeMode::Dark
                                 ? QColor(20, 100, 235)
                                 : QColor(230, 45, 45));
        }
    };

    AntWindow window;
    window.resize(640, 420);
    window.setCentralWidget(new ThemePaintWidget);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QPoint samplePoint = window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center() + QPoint(0, 80);
    QTest::mouseClick(&window,
                      Qt::LeftButton,
                      Qt::NoModifier,
                      window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center());
    auto* overlay = window.findChild<QWidget*>(QStringLiteral("AntWindowThemeTransitionOverlay"));
    QVERIFY(overlay != nullptr);
    QTest::qWait(140);

    QImage rendered(overlay->size(), QImage::Format_ARGB32_Premultiplied);
    rendered.fill(Qt::black);
    QPainter painter(&rendered);
    overlay->render(&painter);
    painter.end();

    const QColor revealed = rendered.pixelColor(samplePoint);
    QVERIFY2(revealed.blue() > 160 && revealed.red() < 120,
             qPrintable(QStringLiteral("Transition reveal exposed a black/transparent hole instead of the new frame: rgba(%1,%2,%3,%4)")
                            .arg(revealed.red())
                            .arg(revealed.green())
                            .arg(revealed.blue())
                            .arg(revealed.alpha())));

    QTRY_VERIFY(window.findChild<QWidget*>(QStringLiteral("AntWindowThemeTransitionOverlay")) == nullptr);
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
    const LONG_PTR nativeStyle = ::GetWindowLongPtrW(hwnd, GWL_STYLE);
    QVERIFY((nativeStyle & WS_THICKFRAME) != 0);
    if (supportsNativeCaptionSnapLayoutsForTest())
    {
        QVERIFY((nativeStyle & WS_CAPTION) != 0);
    }
    else
    {
        QVERIFY((nativeStyle & WS_CAPTION) == 0);
    }
    QVERIFY((nativeStyle & WS_MAXIMIZEBOX) != 0);
    QVERIFY((nativeStyle & WS_MINIMIZEBOX) != 0);

    auto nativeGlobalPoint = [&](const QPoint& localPos) {
        POINT nativePoint{qRound(localPos.x() * window.devicePixelRatioF()),
                          qRound(localPos.y() * window.devicePixelRatioF())};
        ::ClientToScreen(hwnd, &nativePoint);
        return QPoint(nativePoint.x, nativePoint.y);
    };

    auto hitTest = [&](const QPoint& localPos) -> qintptr {
        MSG msg{};
        msg.hwnd = hwnd;
        msg.message = WM_NCHITTEST;
        const QPoint globalPos = nativeGlobalPoint(localPos);
        msg.lParam = MAKELPARAM(globalPos.x(), globalPos.y());
        qintptr result = 0;
        if (!window.nativeEvent("windows_generic_MSG", &msg, &result))
        {
            return -1;
        }
        return result;
    };
    auto systemHitTest = [&](const QPoint& localPos) -> qintptr {
        const QPoint globalPos = nativeGlobalPoint(localPos);
        return ::SendMessageW(hwnd,
                              WM_NCHITTEST,
                              0,
                              MAKELPARAM(globalPos.x(), globalPos.y()));
    };

    QCOMPARE(hitTest(QPoint(2, 2)), static_cast<qintptr>(HTTOPLEFT));
    QCOMPARE(hitTest(QPoint(window.width() - 3, 2)), static_cast<qintptr>(HTTOPRIGHT));
    QCOMPARE(hitTest(QPoint(2, window.height() - 3)), static_cast<qintptr>(HTBOTTOMLEFT));
    QCOMPARE(hitTest(QPoint(window.width() - 3, window.height() - 3)), static_cast<qintptr>(HTBOTTOMRIGHT));
    QCOMPARE(hitTest(QPoint(2, AntWindow::TitleBarHeight + 30)), static_cast<qintptr>(HTLEFT));
    QCOMPARE(hitTest(QPoint(window.width() - 3, AntWindow::TitleBarHeight + 30)), static_cast<qintptr>(HTRIGHT));
    QCOMPARE(hitTest(QPoint(80, 2)), static_cast<qintptr>(HTTOP));
    QCOMPARE(hitTest(QPoint(80, window.height() - 3)), static_cast<qintptr>(HTBOTTOM));
    QCOMPARE(hitTest(QPoint(-2, AntWindow::TitleBarHeight + 30)), static_cast<qintptr>(HTLEFT));
    QCOMPARE(hitTest(QPoint(window.width() + 2, AntWindow::TitleBarHeight + 30)), static_cast<qintptr>(HTRIGHT));
    QCOMPARE(hitTest(QPoint(80, -2)), static_cast<qintptr>(HTTOP));
    QCOMPARE(hitTest(QPoint(80, window.height() + 2)), static_cast<qintptr>(HTBOTTOM));
    QCOMPARE(hitTest(QPoint(-2, -2)), static_cast<qintptr>(HTTOPLEFT));
    QCOMPARE(hitTest(QPoint(window.width() + 2, -2)), static_cast<qintptr>(HTTOPRIGHT));
    QCOMPARE(hitTest(QPoint(-2, window.height() + 2)), static_cast<qintptr>(HTBOTTOMLEFT));
    QCOMPARE(hitTest(QPoint(window.width() + 2, window.height() + 2)), static_cast<qintptr>(HTBOTTOMRIGHT));
    QCOMPARE(systemHitTest(QPoint(-2, AntWindow::TitleBarHeight + 30)), static_cast<qintptr>(HTLEFT));
    QCOMPARE(systemHitTest(QPoint(window.width() + 2, AntWindow::TitleBarHeight + 30)),
             static_cast<qintptr>(HTRIGHT));
    QCOMPARE(systemHitTest(QPoint(80, -2)), static_cast<qintptr>(HTTOP));
    QCOMPARE(systemHitTest(QPoint(80, window.height() + 2)), static_cast<qintptr>(HTBOTTOM));
    QCOMPARE(hitTest(QPoint(80, AntWindow::TitleBarHeight / 2)), static_cast<qintptr>(HTCAPTION));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Pin).center()), static_cast<qintptr>(HTCLIENT));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center()), static_cast<qintptr>(HTCLIENT));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Minimize).center()),
             static_cast<qintptr>(HTREDUCE));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Maximize).center()),
             static_cast<qintptr>(HTZOOM));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Close).center()),
             static_cast<qintptr>(HTCLOSE));

    auto sendNonClientButtonMessage = [&](UINT message, WPARAM hitTestCode, const QPoint& localPos) -> qintptr {
        MSG msg{};
        msg.hwnd = hwnd;
        msg.message = message;
        msg.wParam = hitTestCode;
        const QPoint globalPos = nativeGlobalPoint(localPos);
        msg.lParam = MAKELPARAM(globalPos.x(), globalPos.y());
        qintptr result = 0;
        if (!window.nativeEvent("windows_generic_MSG", &msg, &result))
        {
            return -1;
        }
        return result;
    };

    const QPoint maximizePoint = window.titleBarButtonRect(AntWindow::TitleBarButton::Maximize).center();
    QCOMPARE(hitTest(maximizePoint), static_cast<qintptr>(HTZOOM));
    QVERIFY(sendNonClientButtonMessage(WM_NCMOUSEMOVE, HTCLIENT, maximizePoint) != -1);
    QCOMPARE(window.hoveredTitleBarButton(), AntWindow::TitleBarButton::Maximize);
    sendNonClientButtonMessage(WM_NCMOUSELEAVE, HTZOOM, maximizePoint);
    QCOMPARE(window.hoveredTitleBarButton(), AntWindow::TitleBarButton::None);
    QVERIFY(sendNonClientButtonMessage(WM_NCMOUSEMOVE, HTCLIENT, maximizePoint) != -1);
    QVERIFY(sendNonClientButtonMessage(WM_NCMOUSEHOVER, HTCLIENT, maximizePoint) != -1);
    QVERIFY(sendNonClientButtonMessage(WM_NCLBUTTONDOWN, HTZOOM, maximizePoint) != -1);
    QVERIFY(sendNonClientButtonMessage(WM_NCLBUTTONUP, HTZOOM, maximizePoint) != -1);
    QTRY_VERIFY(window.isMaximized());

    window.showMaximized();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QCoreApplication::processEvents();

    QCOMPARE(hitTest(QPoint(80, AntWindow::TitleBarHeight / 2)), static_cast<qintptr>(HTCAPTION));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Maximize).center()),
             static_cast<qintptr>(HTZOOM));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Close).center()),
             static_cast<qintptr>(HTCLOSE));
#endif
}

void TestAntQtExtensions::windowDwmFrameMarginsPreserveShadow()
{
#ifndef Q_OS_WIN
    QSKIP("Windows DWM frame margins are only available on Windows.");
#else
    AntWindow window;
    window.resize(640, 420);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QVariant usesNativeCaption = window.property("antWindowUsesNativeCaptionFrame");
    QVERIFY2(usesNativeCaption.isValid(), "AntWindow should expose the active native caption frame policy for diagnostics");
    QCOMPARE(usesNativeCaption.toBool(), supportsNativeCaptionSnapLayoutsForTest());

    const QVariant frameMarginsProperty = window.property("antWindowDwmFrameMargins");
    QVERIFY2(frameMarginsProperty.isValid(), "AntWindow should expose the DWM frame margins applied to the native window");
    const QMargins margins = frameMarginsProperty.value<QMargins>();
    QCOMPARE(margins,
             supportsNativeCaptionSnapLayoutsForTest() ? QMargins(1, 1, 1, 1) : QMargins(0, 0, 0, 0));

    const HWND hwnd = reinterpret_cast<HWND>(window.winId());
    QVERIFY(hwnd != nullptr);
    const LONG_PTR nativeStyle = ::GetWindowLongPtrW(hwnd, GWL_STYLE);
    if (!supportsNativeCaptionSnapLayoutsForTest())
    {
        QVERIFY2((nativeStyle & WS_CAPTION) == 0, "Windows 10 must keep the no-caption path to avoid native title buttons");
    }
#endif
}

void TestAntQtExtensions::windowLegacyFramePolicyRestoresShadowAfterResize()
{
#ifndef Q_OS_WIN
    QSKIP("Windows legacy frame policy is only available on Windows.");
#else
    AntWindow window;
    window.setProperty("antWindowForceLegacyFramePolicy", true);
    window.resize(640, 420);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QCOMPARE(window.property("antWindowUsesNativeCaptionFrame").toBool(), false);
    // AntWindow no longer uses setMask to clip its rounded corners — it relies
    // on WA_TranslucentBackground + alpha-painted corners (style rounded clip
    // path + AntWindowCornerSmoother). The mask was a source of shrink-resize
    // flicker because its update is asynchronous w.r.t. the WM_SIZE backing
    // store paint. The legacy-frame-policy property contract still exposes
    // the historical frame-inset value for diagnostics.
    QCOMPARE(window.property("antWindowLegacyRoundedMaskApplied").toBool(), false);
    QCOMPARE(window.property("antWindowLegacyRoundedMaskFrameInset").toInt(), 1);
    QCOMPARE(window.property("antWindowDwmFrameMargins").value<QMargins>(), QMargins(0, 0, 0, 0));
    QCOMPARE(window.property("antWindowLegacyClassDropShadowEnabled").toBool(), false);
    QTRY_COMPARE(window.property("antWindowLegacySoftwareShadowEnabled").toBool(), true);
    QCOMPARE(window.property("antWindowLegacySoftwareShadowMargin").toInt(), 14);
    QVERIFY(window.property("antWindowLegacySoftwareShadowInnerClearance").isValid());
    QCOMPARE(window.property("antWindowLegacySoftwareShadowInnerClearance").toInt(), 0);
    const QRect initialShadowGeometry = window.property("antWindowLegacySoftwareShadowGeometry").toRect();
    QVERIFY(initialShadowGeometry.contains(window.geometry()));
    QVERIFY(initialShadowGeometry.left() < window.geometry().left());
    QVERIFY(initialShadowGeometry.top() < window.geometry().top());
    QVERIFY(initialShadowGeometry.right() > window.geometry().right());
    QVERIFY(initialShadowGeometry.bottom() > window.geometry().bottom());
    auto* shadowWidget = window.findChild<QWidget*>(QStringLiteral("AntWindowLegacySoftwareShadow"));
    QVERIFY(shadowWidget);
    QTRY_VERIFY(shadowWidget->isVisible());
    QVERIFY(shadowWidget->testAttribute(Qt::WA_TransparentForMouseEvents));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    QVERIFY(shadowWidget->windowFlags().testFlag(Qt::WindowTransparentForInput));
#endif
    QTRY_COMPARE(shadowWidget->property("antWindowLegacySoftwareShadowClickThrough").toBool(), true);

    auto maxAlphaIn = [](const QImage& image, const QRect& sampleRect) {
        int maxAlpha = 0;
        const QRect clipped = sampleRect.intersected(image.rect());
        for (int y = clipped.top(); y <= clipped.bottom(); ++y)
        {
            for (int x = clipped.left(); x <= clipped.right(); ++x)
            {
                maxAlpha = qMax(maxAlpha, qAlpha(image.pixel(x, y)));
            }
        }
        return maxAlpha;
    };
    const int shadowMargin = window.property("antWindowLegacySoftwareShadowMargin").toInt();
    const QImage shadowImage = shadowWidget->grab().toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QVERIFY(maxAlphaIn(shadowImage, QRect(0, shadowMargin, shadowMargin, shadowImage.height() - shadowMargin * 2)) > 0);
    QVERIFY(maxAlphaIn(shadowImage, QRect(shadowMargin, 0, shadowImage.width() - shadowMargin * 2, shadowMargin)) > 0);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowImage.width() - shadowMargin,
                             shadowMargin,
                             shadowMargin,
                             shadowImage.height() - shadowMargin * 2)) > 0);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowMargin,
                             shadowImage.height() - shadowMargin,
                             shadowImage.width() - shadowMargin * 2,
                             shadowMargin)) > 0);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowMargin - 2,
                             shadowMargin,
                             2,
                             shadowImage.height() - shadowMargin * 2)) > 4);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowMargin,
                             shadowMargin - 2,
                             shadowImage.width() - shadowMargin * 2,
                             2)) > 4);
    QVERIFY(maxAlphaIn(shadowImage, QRect(0, 0, shadowMargin, shadowMargin)) <= 32);
    QVERIFY(maxAlphaIn(shadowImage, QRect(shadowImage.width() - shadowMargin, 0, shadowMargin, shadowMargin)) <= 32);
    QVERIFY(maxAlphaIn(shadowImage, QRect(0, shadowImage.height() - shadowMargin, shadowMargin, shadowMargin)) <= 32);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowImage.width() - shadowMargin,
                             shadowImage.height() - shadowMargin,
                             shadowMargin,
                             shadowMargin)) <= 32);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowMargin - 2,
                             shadowMargin,
                             2,
                             shadowImage.height() - shadowMargin * 2)) <= 16);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowMargin,
                             shadowMargin - 2,
                             shadowImage.width() - shadowMargin * 2,
                             2)) <= 16);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowMargin - 2,
                             shadowMargin,
                             2,
                             shadowImage.height() - shadowMargin * 2)) <= 10);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowMargin,
                             shadowMargin - 2,
                             shadowImage.width() - shadowMargin * 2,
                             2)) <= 10);
    QVERIFY(maxAlphaIn(shadowImage, QRect(0, 0, shadowMargin, shadowMargin)) <= 20);
    QVERIFY(maxAlphaIn(shadowImage, QRect(shadowImage.width() - shadowMargin, 0, shadowMargin, shadowMargin)) <= 20);
    QVERIFY(maxAlphaIn(shadowImage, QRect(0, shadowImage.height() - shadowMargin, shadowMargin, shadowMargin)) <= 20);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowImage.width() - shadowMargin,
                             shadowImage.height() - shadowMargin,
                             shadowMargin,
                             shadowMargin)) <= 20);

    const int frameApplyCount = window.property("antWindowDwmFrameApplyCount").toInt();
    QVERIFY2(frameApplyCount > 0, "legacy frame policy should apply a shadow-preserving DWM frame");

    window.resize(700, 460);

    QVERIFY2(window.property("antWindowDwmFrameApplyCount").toInt() > frameApplyCount,
             "resizing a Win10 legacy-frame AntWindow should reapply the DWM frame after updating the mask");
    QTRY_VERIFY2(window.property("antWindowDwmFrameApplyCount").toInt() >= frameApplyCount + 2,
                 "resizing a Win10 legacy-frame AntWindow should queue a second DWM frame refresh after native resize settles");
    QCOMPARE(window.property("antWindowDwmFrameLastReason").toString(), QStringLiteral("resize-deferred"));

    const QRect resizedShadowGeometry = window.property("antWindowLegacySoftwareShadowGeometry").toRect();
    QVERIFY(resizedShadowGeometry.contains(window.geometry()));
    QCOMPARE(resizedShadowGeometry.marginsRemoved(QMargins(window.property("antWindowLegacySoftwareShadowMargin").toInt(),
                                                          window.property("antWindowLegacySoftwareShadowMargin").toInt(),
                                                          window.property("antWindowLegacySoftwareShadowMargin").toInt(),
                                                          window.property("antWindowLegacySoftwareShadowMargin").toInt())),
             window.geometry());
#endif
}

void TestAntQtExtensions::windowMaximizedNcCalcKeepsFullWorkArea()
{
#ifndef Q_OS_WIN
    QSKIP("Windows native frame sizing is only available on Windows.");
#else
    class NativeFrameWindow : public AntWindow
    {
    public:
        using AntWindow::nativeEvent;
    };

    NativeFrameWindow window;
    window.resize(640, 420);
    window.setWindowState(window.windowState() | Qt::WindowMaximized);
    QVERIFY(window.isMaximized());

    const HWND hwnd = reinterpret_cast<HWND>(window.winId());
    QVERIFY(hwnd != nullptr);

    NCCALCSIZE_PARAMS params{};
    params.rgrc[0] = RECT{100, 120, 900, 760};
    const RECT before = params.rgrc[0];

    MSG msg{};
    msg.hwnd = hwnd;
    msg.message = WM_NCCALCSIZE;
    msg.wParam = TRUE;
    msg.lParam = reinterpret_cast<LPARAM>(&params);

    qintptr result = -1;
    QVERIFY(window.nativeEvent("windows_generic_MSG", &msg, &result));
    QCOMPARE(result, static_cast<qintptr>(0));
    QCOMPARE(params.rgrc[0].left, before.left);
    QCOMPARE(params.rgrc[0].top, before.top);
    QCOMPARE(params.rgrc[0].right, before.right);
    QCOMPARE(params.rgrc[0].bottom, before.bottom);
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
