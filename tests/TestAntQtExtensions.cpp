#include <QPalette>
#include <QApplication>
#include <QPainter>
#include <QCoreApplication>
#include <QContextMenuEvent>
#include <QElapsedTimer>
#include <QEnterEvent>
#include <QEventLoop>
#include <QGraphicsOpacityEffect>
#include <QGuiApplication>
#include <QImage>
#include <QLabel>
#include <QFrame>
#include <QMargins>
#include <QMainWindow>
#include <QMetaType>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPixmap>
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

#include <atomic>
#include <algorithm>
#include <thread>
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
#include "widgets/AntButton.h"
#include "widgets/AntModal.h"
#include "widgets/AntToolButton.h"
#include "widgets/AntToolBar.h"
#include "widgets/AntMenu.h"
#include "widgets/AntMenuBar.h"
#include "widgets/AntNavItem.h"
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

int countNearColorForExtensionTest(const QImage& image, const QColor& expected, int tolerance = 18)
{
    if (image.isNull()) return 0;

    int count = 0;
    for (int y = 0; y < image.height(); ++y)
    {
        for (int x = 0; x < image.width(); ++x)
        {
            if (colorNearForExtensionTest(image.pixelColor(x, y), expected, tolerance))
            {
                ++count;
            }
        }
    }
    return count;
}

class PaintProbeWidget : public QWidget
{
public:
    explicit PaintProbeWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setMinimumSize(160, 120);
    }

    int paintCount() const
    {
        return m_paintCount;
    }

    void setFillColor(const QColor& color)
    {
        m_fillColor = color;
        update();
    }

    void resetIntervalSamples()
    {
        m_intervalsMs.clear();
        m_intervalTimer.invalidate();
    }

    QList<qint64> paintIntervalsMs() const
    {
        return m_intervalsMs;
    }

    qint64 maxPaintIntervalMs() const
    {
        if (m_intervalsMs.isEmpty()) return 0;
        return *std::max_element(m_intervalsMs.cbegin(), m_intervalsMs.cend());
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        if (m_intervalTimer.isValid())
        {
            m_intervalsMs.append(m_intervalTimer.elapsed());
        }
        m_intervalTimer.restart();
        ++m_paintCount;
        QPainter painter(this);
        painter.fillRect(rect(), m_fillColor);
    }

private:
    QColor m_fillColor = QColor(32, 112, 240);
    int m_paintCount = 0;
    QElapsedTimer m_intervalTimer;
    QList<qint64> m_intervalsMs;
};

class ThemeSizedWidget : public QWidget
{
public:
    explicit ThemeSizedWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        connect(antTheme, &AntTheme::themeChanged, this, [this]() {
            updateGeometry();
            update();
        });
    }

    QSize sizeHint() const override
    {
        return QSize(180, antTheme->themeMode() == Ant::ThemeMode::Dark ? 96 : 32);
    }

    QSize minimumSizeHint() const override
    {
        return sizeHint();
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), antTheme->themeMode() == Ant::ThemeMode::Dark ? QColor(30, 120, 210)
                                                                                : QColor(210, 80, 60));
    }
};

class ThemeSizedAntWidget : public AntWidget
{
public:
    explicit ThemeSizedAntWidget(QWidget* parent = nullptr)
        : AntWidget(parent)
    {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        setMinimumSize(80, 24);
    }

    QSize sizeHint() const override
    {
        return QSize(180, currentTheme() == Ant::ThemeMode::Dark ? 96 : 32);
    }

    QSize minimumSizeHint() const override
    {
        return sizeHint();
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), tokens().colorBgContainer);
    }

    void onThemeChanged(Ant::ThemeMode mode) override
    {
        Q_UNUSED(mode)
        ++m_onThemeChangedCount;
        setProperty("antWidgetOnThemeChangedCount", m_onThemeChangedCount);
    }

private:
    int m_onThemeChangedCount = 0;
};

class ThemeModeRestorerForExtensionTest
{
public:
    ThemeModeRestorerForExtensionTest()
        : m_originalMode(antTheme->themeMode())
    {
    }

    ~ThemeModeRestorerForExtensionTest()
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

int primaryLikePixelCountForExtensionTest(const QImage& image, QRect sampleRect)
{
    if (image.isNull()) return 0;

    sampleRect = sampleRect.intersected(image.rect());
    int count = 0;
    for (int y = sampleRect.top(); y <= sampleRect.bottom(); ++y)
    {
        for (int x = sampleRect.left(); x <= sampleRect.right(); ++x)
        {
            const QColor color = image.pixelColor(x, y);
            const bool primaryStroke =
                color.blue() >= 145 &&
                color.blue() > color.red() + 24 &&
                color.blue() > color.green() + 5;
            const bool primarySoftFill =
                color.blue() >= 230 &&
                color.green() >= 210 &&
                color.red() >= 180 &&
                color.blue() > color.red() + 8 &&
                color.blue() >= color.green();
            if (color.alpha() >= 60 && (primaryStroke || primarySoftFill))
            {
                ++count;
            }
        }
    }
    return count;
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

bool nativeMoveMouseToNativePointForExtensionTest(const QPoint& nativePos)
{
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

bool nativeMouseDragFromWorkerForExtensionTest(QWidget* focusWidget, const QPoint& startGlobal, const QPoint& endGlobal)
{
    if (!bringWidgetToFrontForExtensionTest(focusWidget))
    {
        return false;
    }

    const QPoint nativeStart = nativePointForExtensionTest(focusWidget, startGlobal);
    const QPoint nativeEnd = nativePointForExtensionTest(focusWidget, endGlobal);
    std::atomic_bool done{false};
    std::atomic_bool ok{true};
    std::thread worker([nativeStart, nativeEnd, &done, &ok]() {
        ok = nativeMoveMouseToNativePointForExtensionTest(nativeStart);
        ::Sleep(70);
        ok = ok && sendMouseInputForExtensionTest(MOUSEEVENTF_LEFTDOWN);
        ::Sleep(90);
        constexpr int kSteps = 18;
        for (int step = 1; step <= kSteps; ++step)
        {
            const qreal t = static_cast<qreal>(step) / static_cast<qreal>(kSteps);
            const QPoint point(qRound(nativeStart.x() + (nativeEnd.x() - nativeStart.x()) * t),
                               qRound(nativeStart.y() + (nativeEnd.y() - nativeStart.y()) * t));
            ok = ok && nativeMoveMouseToNativePointForExtensionTest(point);
            ::Sleep(16);
        }
        ok = ok && sendMouseInputForExtensionTest(MOUSEEVENTF_LEFTUP);
        ::Sleep(80);
        done = true;
    });

    QElapsedTimer timer;
    timer.start();
    while (!done && timer.elapsed() < 4000)
    {
        QTest::qWait(20);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
    if (worker.joinable())
    {
        worker.join();
    }
    releaseTopMostForExtensionTest(focusWidget);
    return done && ok;
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
    void navItem();
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
    void colorPickerDragSmoothness();
};

void TestAntQtExtensions::app()
{
    QWidget root;

    {
        AntApp app(&root);
        QCOMPARE(app.rootWidget(), &root);
        QCOMPARE(app.feedbackHost(), &root);
        QCOMPARE(app.property("antAppFeedbackHostCacheHit").toBool(), false);
        QCOMPARE(app.property("antAppFeedbackHostResolveCount").toInt(), 1);
        QCOMPARE(AntApp::instance(), &app);

        app.showMessage(QStringLiteral("Cached"), 10);
        QCOMPARE(app.feedbackHost(), &root);
        QCOMPARE(app.property("antAppFeedbackHostCacheHit").toBool(), true);
        QCOMPARE(app.property("antAppFeedbackHostResolveCount").toInt(), 1);

        QWidget nestedRoot(&root);
        {
            AntApp nestedApp(&nestedRoot);
            QCOMPARE(AntApp::instance(), &nestedApp);
            QCOMPARE(nestedApp.feedbackHost(), &nestedRoot);
            nestedApp.showModal(QStringLiteral("Title"), QStringLiteral("Body"));
            nestedApp.showNotification(QStringLiteral("Title"), QStringLiteral("Body"));
            QCOMPARE(nestedApp.property("antAppFeedbackHostResolveCount").toInt(), 1);
        }

        QCOMPARE(AntApp::instance(), &app);
    }

    QCOMPARE(AntApp::instance(), nullptr);
}

void TestAntQtExtensions::configProvider()
{
    ThemeModeRestorerForExtensionTest themeGuard;
    AntConfigProvider provider;

    QCOMPARE(provider.themeMode(), Ant::ThemeMode::Default);
    QCOMPARE(provider.fontSize(), 14);
    QCOMPARE(provider.borderRadius(), 6);
    QCOMPARE(provider.revision(), 0);

    QSignalSpy configSpy(&provider, &AntConfigProvider::configChanged);
    QSignalSpy themeSpy(&provider, &AntConfigProvider::themeModeChanged);
    provider.setThemeMode(themeGuard.alternateMode());
    QCOMPARE(provider.themeMode(), themeGuard.alternateMode());
    QCOMPARE(themeSpy.count(), 1);

    QSignalSpy colorSpy(&provider, &AntConfigProvider::primaryColorChanged);
    provider.setPrimaryColor(Qt::blue);
    QCOMPARE(provider.primaryColor(), QColor(Qt::blue));
    QCOMPARE(colorSpy.count(), 1);

    QSignalSpy fontSpy(&provider, &AntConfigProvider::fontSizeChanged);
    provider.setFontSize(16);
    QCOMPARE(provider.fontSize(), 16);
    QCOMPARE(fontSpy.count(), 1);

    QSignalSpy radiusSpy(&provider, &AntConfigProvider::borderRadiusChanged);
    provider.setBorderRadius(8);
    QCOMPARE(provider.borderRadius(), 8);
    QCOMPARE(radiusSpy.count(), 1);
    QCOMPARE(configSpy.count(), 0);
    QCOMPARE(provider.revision(), 0);

    QTRY_COMPARE(configSpy.count(), 1);
    QCOMPARE(provider.revision(), 1);

    provider.setFontSize(16);
    provider.setBorderRadius(8);
    QCoreApplication::processEvents();
    QCOMPARE(configSpy.count(), 1);
    QCOMPARE(provider.revision(), 1);

    provider.setFontSize(17);
    provider.setBorderRadius(9);
    QTRY_COMPARE(configSpy.count(), 2);
    QCOMPARE(provider.revision(), 2);

    QSignalSpy globalThemeSpy(antTheme, &AntTheme::themeChanged);
    provider.apply();
    QCOMPARE(antTheme->themeMode(), provider.themeMode());
    QCOMPARE(globalThemeSpy.count(), 1);

    provider.apply();
    QCOMPARE(globalThemeSpy.count(), 1);
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
    QCOMPARE(w->property("antLogUsesDocumentCursor").toBool(), true);
    QCOMPARE(w->property("antLogUndoRedoEnabled").toBool(), false);

    w->setMaxEntries(128);
    QCOMPARE(w->maxEntries(), 128);
    QCOMPARE(maxSpy.count(), 2);

    QElapsedTimer appendTimer;
    appendTimer.start();
    for (int i = 0; i < 700; ++i)
    {
        w->info(QStringLiteral("bulk message %1").arg(i));
    }
    const qint64 appendElapsed = appendTimer.elapsed();
    QVERIFY2(appendElapsed < 3000, qPrintable(QStringLiteral("AntLog bulk append took %1 ms").arg(appendElapsed)));
    QCOMPARE(w->property("antLogEntryCount").toInt(), 128);
    QVERIFY(view->document()->blockCount() <= w->maxEntries());
    QVERIFY(!view->toPlainText().contains(QStringLiteral("bulk message 0")));
    QVERIFY(view->toPlainText().contains(QStringLiteral("bulk message 699")));

    w->setMaxEntries(64);
    QCOMPARE(w->maxEntries(), 64);
    QCOMPARE(maxSpy.count(), 3);
    QCOMPARE(w->property("antLogEntryCount").toInt(), 64);
    QCOMPARE(w->property("antLogLastTrimmedCount").toInt(), 64);
    QVERIFY(view->document()->blockCount() <= w->maxEntries());
    QVERIFY(view->toPlainText().contains(QStringLiteral("bulk message 699")));

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
    QCOMPARE(w->property("antSplitterHandlePaletteRefreshCount").toInt(), 1);

    w->resize(240, 80);
    w->show();
    QVERIFY(QTest::qWaitForWindowExposed(w));
    QCoreApplication::processEvents();

    auto* handle = qobject_cast<AntSplitterHandle*>(w->handle(1));
    QVERIFY(handle);
    QVERIFY(!handle->size().isEmpty());

    QImage firstPaint(handle->size(), QImage::Format_ARGB32_Premultiplied);
    firstPaint.fill(Qt::transparent);
    handle->render(&firstPaint);
    QCOMPARE(handle->property("antSplitterHandleColorResolveCount").toInt(), 1);

    QImage secondPaint(handle->size(), QImage::Format_ARGB32_Premultiplied);
    secondPaint.fill(Qt::transparent);
    handle->render(&secondPaint);
    QCOMPARE(handle->property("antSplitterHandleColorResolveCount").toInt(), 1);
    QVERIFY(handle->property("antSplitterHandlePaintCount").toInt() >= 2);
    QVERIFY2(colorNearForExtensionTest(secondPaint.pixelColor(secondPaint.rect().center()), antTheme->tokens().colorBorder),
             qPrintable(QStringLiteral("splitter handle color should match cached border color, actual %1 expected %2")
                            .arg(colorStringForExtensionTest(secondPaint.pixelColor(secondPaint.rect().center())),
                                 colorStringForExtensionTest(antTheme->tokens().colorBorder))));

    const auto previousTheme = antTheme->themeMode();
    const auto nextTheme = previousTheme == Ant::ThemeMode::Dark ? Ant::ThemeMode::Default : Ant::ThemeMode::Dark;
    const int paletteRefreshes = w->property("antSplitterHandlePaletteRefreshCount").toInt();
    antTheme->setThemeMode(nextTheme);
    QCoreApplication::processEvents();
    QCOMPARE(w->property("antSplitterHandlePaletteRefreshCount").toInt(), paletteRefreshes + 1);

    QImage afterThemePaint(handle->size(), QImage::Format_ARGB32_Premultiplied);
    afterThemePaint.fill(Qt::transparent);
    handle->render(&afterThemePaint);
    QCOMPARE(handle->property("antSplitterHandleColorResolveCount").toInt(), 2);
    QVERIFY2(colorNearForExtensionTest(afterThemePaint.pixelColor(afterThemePaint.rect().center()), antTheme->tokens().colorBorder),
             qPrintable(QStringLiteral("splitter handle color should update after theme change, actual %1 expected %2")
                            .arg(colorStringForExtensionTest(afterThemePaint.pixelColor(afterThemePaint.rect().center())),
                                 colorStringForExtensionTest(antTheme->tokens().colorBorder))));

    antTheme->setThemeMode(previousTheme);
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
    auto* editMenu = w->addMenu("&Edit");
    auto* viewMenu = w->addMenu("View");
    QVERIFY(menu != nullptr);
    QVERIFY(editMenu != nullptr);
    QVERIFY(viewMenu != nullptr);
    QVERIFY(!w->actions().isEmpty());
    w->resize(260, antTheme->tokens().controlHeight);
    w->show();
    QVERIFY(QTest::qWaitForWindowExposed(w));

    auto renderMenuBar = [w]() {
        const qreal dpr = w->devicePixelRatioF();
        QImage image((QSizeF(w->size()) * dpr).toSize(), QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(dpr);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        w->render(&painter);
    };
    renderMenuBar();
    const int textMetricBuildCount = w->property("antMenuBarTextMetricCacheBuildCount").toInt();
    QVERIFY(textMetricBuildCount >= 3);
    const int textMetricHitCount = w->property("antMenuBarTextMetricCacheHitCount").toInt();
    renderMenuBar();
    QCOMPARE(w->property("antMenuBarTextMetricCacheBuildCount").toInt(), textMetricBuildCount);
    QVERIFY(w->property("antMenuBarTextMetricCacheHitCount").toInt() > textMetricHitCount);

    QAction* fileAction = menu->menuAction();
    QAction* editAction = editMenu->menuAction();
    const QPoint filePoint = w->actionGeometry(fileAction).center();
    const QPoint editPoint = w->actionGeometry(editAction).center();
    QMouseEvent moveFileEvent(QEvent::MouseMove,
                              QPointF(filePoint),
                              QPointF(w->mapToGlobal(filePoint)),
                              Qt::NoButton,
                              Qt::NoButton,
                              Qt::NoModifier);
    QCoreApplication::sendEvent(w, &moveFileEvent);
    QCOMPARE(w->property("antMenuBarScopedHoverUpdate").toBool(), true);
    const int actionGeometryBuildsAfterFile =
        w->property("antMenuBarActionGeometryCacheBuildCount").toInt();
    QVERIFY(actionGeometryBuildsAfterFile >= 1);

    QMouseEvent moveEditEvent(QEvent::MouseMove,
                              QPointF(editPoint),
                              QPointF(w->mapToGlobal(editPoint)),
                              Qt::NoButton,
                              Qt::NoButton,
                              Qt::NoModifier);
    QCoreApplication::sendEvent(w, &moveEditEvent);
    QCOMPARE(w->property("antMenuBarScopedHoverUpdate").toBool(), true);
    const int actionGeometryBuildsAfterEdit =
        w->property("antMenuBarActionGeometryCacheBuildCount").toInt();
    QVERIFY(actionGeometryBuildsAfterEdit >= actionGeometryBuildsAfterFile);
    QCoreApplication::sendEvent(w, &moveEditEvent);
    QCOMPARE(w->property("antMenuBarActionGeometryCacheBuildCount").toInt(), actionGeometryBuildsAfterEdit);

    w->resize(300, antTheme->tokens().controlHeight);
    QCoreApplication::processEvents();
    QCOMPARE(w->property("antMenuBarActionGeometryCacheSize").toInt(), 0);
    delete w;
}

void TestAntQtExtensions::navItem()
{
    AntNavItem item(QStringLiteral("Dashboard"));
    QSignalSpy activeSpy(&item, &AntNavItem::activeChanged);
    item.resize(220, 36);
    item.show();
    QVERIFY(QTest::qWaitForWindowExposed(&item));

    auto renderNavItem = [&item]() {
        const qreal dpr = item.devicePixelRatioF();
        QImage image((QSizeF(item.size()) * dpr).toSize(), QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(dpr);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        item.render(&painter);
    };

    renderNavItem();
    const int initialPaintCacheBuilds = item.property("antNavItemPaintCacheBuildCount").toInt();
    QVERIFY(initialPaintCacheBuilds >= 1);
    const int initialVisualApplies = item.property("antNavItemVisualStateApplyCount").toInt();
    QVERIFY(initialVisualApplies >= 1);

    renderNavItem();
    QCOMPARE(item.property("antNavItemPaintCacheBuildCount").toInt(), initialPaintCacheBuilds);
    QCOMPARE(item.property("antNavItemVisualStateApplyCount").toInt(), initialVisualApplies);

    QEnterEvent enterEvent(QPointF(12, 12),
                           QPointF(12, 12),
                           QPointF(item.mapToGlobal(QPoint(12, 12))));
    QCoreApplication::sendEvent(&item, &enterEvent);
    QCOMPARE(item.property("antNavItemPaintCacheValid").toBool(), false);
    renderNavItem();
    const int hoverPaintCacheBuilds = item.property("antNavItemPaintCacheBuildCount").toInt();
    QVERIFY(hoverPaintCacheBuilds > initialPaintCacheBuilds);
    renderNavItem();
    QCOMPARE(item.property("antNavItemPaintCacheBuildCount").toInt(), hoverPaintCacheBuilds);

    item.setActive(true);
    QCOMPARE(activeSpy.count(), 1);
    QVERIFY(item.property("antNavItemVisualStateApplyCount").toInt() > initialVisualApplies);
    const int activeVisualApplies = item.property("antNavItemVisualStateApplyCount").toInt();
    item.setActive(true);
    QCOMPARE(activeSpy.count(), 1);
    QCOMPARE(item.property("antNavItemVisualStateApplyCount").toInt(), activeVisualApplies);

    renderNavItem();
    const int activePaintCacheBuilds = item.property("antNavItemPaintCacheBuildCount").toInt();
    QVERIFY(activePaintCacheBuilds > hoverPaintCacheBuilds);

    QEvent leaveEvent(QEvent::Leave);
    QCoreApplication::sendEvent(&item, &leaveEvent);
    renderNavItem();
    QVERIFY(item.property("antNavItemPaintCacheBuildCount").toInt() > activePaintCacheBuilds);
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
    auto* closeButton = titleBar->findChild<QWidget*>(QStringLiteral("AntDockTitleCloseButton"));
    QVERIFY(closeButton != nullptr);
    QTRY_VERIFY(closeButton->isVisible());
    closeButton->grab();
    const int closeIconRenderCount = closeButton->property("antDockTitleButtonIconRenderCount").toInt();
    QVERIFY(closeIconRenderCount >= 1);
    closeButton->grab();
    QCOMPARE(closeButton->property("antDockTitleButtonIconRenderCount").toInt(), closeIconRenderCount);
    QCOMPARE(closeButton->property("antDockTitleButtonIconCacheHit").toBool(), true);

    const int chromeApplyCount = titleBar->property("antDockTitleBarChromeApplyCount").toInt();
    lockedFloat->setFeatures(lockedFloat->features() & ~QDockWidget::DockWidgetMovable);
    QCoreApplication::processEvents();
    QCOMPARE(titleBar->property("antDockTitleBarChromeApplyCount").toInt(), chromeApplyCount);

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
#if defined(Q_OS_WIN)
    explorer->setProperty("antDockForceLegacyFramePolicy", true);
#endif
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
    const int styleApplyCountBeforeRestore = manager->property("antDockStyleSheetApplyCount").toInt();
    QVERIFY(manager->restorePerspective(QStringLiteral("default")));
    QCOMPARE(restoredSpy.count(), 1);
    QVERIFY(manager->property("antDockLastRestoreElapsedMs").isValid());
    QVERIFY(manager->property("antDockLastRestoreKeptEmbeddedDocks").toInt() >= 3);
    QVERIFY(manager->property("antDockLastRestoreAreaCount").toInt() >= 2);
    QCOMPARE(manager->property("antDockStyleSheetApplyCount").toInt(), styleApplyCountBeforeRestore);
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
    QTRY_VERIFY(manager->property("antDockAreaHitZoneRebuildCount").toInt() >= 1);
    const int hitZoneRebuildsAfterCenterMove =
        manager->property("antDockAreaHitZoneRebuildCount").toInt();
    const int dropTargetComputesAfterCenterMove =
        manager->property("antDockDropTargetComputeCount").toInt();
    QVERIFY(dropTargetComputesAfterCenterMove >= 1);
    QWidget* dragPreviewWindow = nullptr;
    QWidget* dropPreviewWindow = nullptr;
#if defined(Q_OS_WIN)
    QTRY_VERIFY((dragPreviewWindow = topLevelWidgetForExtensionTest(QStringLiteral("AntDockDragPreviewWindow"))) != nullptr);
    QTRY_VERIFY((dropPreviewWindow = manager->findChild<QWidget*>(QStringLiteral("AntDockDropPreviewWindow"))) != nullptr);
    QVERIFY(dragPreviewWindow->testAttribute(Qt::WA_TransparentForMouseEvents));
    QVERIFY(dropPreviewWindow->testAttribute(Qt::WA_TransparentForMouseEvents));
    QTRY_COMPARE(dragPreviewWindow->property("antDockTransparentToolWindowClickThrough").toBool(), true);
    QTRY_COMPARE(dropPreviewWindow->property("antDockTransparentToolWindowClickThrough").toBool(), true);
    QVERIFY(!dropPreviewWindow->isWindow());
    QVERIFY(!dropPreviewWindow->internalWinId());
#endif
    QTRY_COMPARE(manager->activeDropGuide(), AntDockManager::DockPlacement::Center);
    const QRect managerGlobalRect = QRect(manager->mapToGlobal(QPoint(0, 0)), manager->size());
    QWidget* guideOverlay = manager->findChild<QWidget*>(QStringLiteral("AntDockGuideOverlay"));
    QVERIFY(guideOverlay != nullptr);
    QTRY_VERIFY(guideOverlay->isVisible());
    QVERIFY(!guideOverlay->isWindow());
    QVERIFY(guideOverlay->testAttribute(Qt::WA_TransparentForMouseEvents));
    QCOMPARE(guideOverlay->geometry(), manager->rect());
#if defined(Q_OS_WIN)
    QTRY_COMPARE(guideOverlay->property("antDockTransparentToolWindowClickThrough").toBool(), true);
    QVERIFY(!guideOverlay->internalWinId());
#endif
    const QPoint guideLocalCenter = guideOverlay->mapFromGlobal(propertiesCenterGlobal);
    const QImage guideOverlayImage =
        guideOverlay->grab().toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const qreal guideScaleX = guideOverlay->width() > 0
        ? static_cast<qreal>(guideOverlayImage.width()) / static_cast<qreal>(guideOverlay->width())
        : 1.0;
    const qreal guideScaleY = guideOverlay->height() > 0
        ? static_cast<qreal>(guideOverlayImage.height()) / static_cast<qreal>(guideOverlay->height())
        : 1.0;
    const QRect guideSampleRect(qRound((guideLocalCenter.x() - 30) * guideScaleX),
                                qRound((guideLocalCenter.y() - 30) * guideScaleY),
                                qRound(60 * guideScaleX),
                                qRound(60 * guideScaleY));
    const int guidePrimaryPixels = primaryLikePixelCountForExtensionTest(guideOverlayImage, guideSampleRect);
    const QByteArray guidePaintMessage =
        QStringLiteral("Drop guide overlay must visibly paint the active center guide square; counted %1 primary pixels in (%2,%3 %4x%5) within image %6x%7.")
            .arg(guidePrimaryPixels)
            .arg(guideSampleRect.x())
            .arg(guideSampleRect.y())
            .arg(guideSampleRect.width())
            .arg(guideSampleRect.height())
            .arg(guideOverlayImage.width())
            .arg(guideOverlayImage.height())
            .toLocal8Bit();
    QVERIFY2(guidePrimaryPixels > 24,
             guidePaintMessage.constData());
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
    QCOMPARE(manager->property("antDockAreaHitZoneRebuildCount").toInt(), hitZoneRebuildsAfterCenterMove);
    QVERIFY(!manager->dropPreviewRect().isEmpty());
    QVERIFY2(manager->dropPreviewRect().left() >= managerGlobalRect.center().x(),
             "Right edge guide preview must target the right half of the dock container.");
    const int layoutCountBeforeRightDrop = layoutSpy.count();
    const int rightDropComputesBeforeRelease =
        manager->property("antDockDropTargetComputeCount").toInt();
    const int rightDropCacheHitsBeforeRelease =
        manager->property("antDockDropTargetCacheHitCount").toInt();
    QMouseEvent releaseRightEvent(QEvent::MouseButtonRelease,
                                  QPointF(rightGuideMove),
                                  QPointF(rightGuideGlobal),
                                  Qt::LeftButton,
                                  Qt::NoButton,
                                  Qt::NoModifier);
    QCoreApplication::sendEvent(explorerTabBar, &releaseRightEvent);
    QVERIFY2(layoutSpy.count() > layoutCountBeforeRightDrop,
             "Embedded guided dock drops should apply the new layout before the release event returns.");
    QCOMPARE(manager->property("antDockDropTargetComputeCount").toInt(), rightDropComputesBeforeRelease);
    QVERIFY(manager->property("antDockDropTargetCacheHitCount").toInt() > rightDropCacheHitsBeforeRelease);
    QCOMPARE(manager->property("antDockLastGuidedDropSynchronous").toBool(), true);
    QVERIFY(manager->property("antDockLastGuidedDropElapsedMs").isValid());
    QVERIFY(!manager->isDropPreviewVisible());
    const int layoutCountAfterRightDrop = layoutSpy.count();
    QCoreApplication::sendPostedEvents(manager, QEvent::MetaCall);
    QCOMPARE(layoutSpy.count(), layoutCountAfterRightDrop);
    QVERIFY(!manager->isDropPreviewVisible());
    QVERIFY([&]() {
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
    const int layoutCountBeforeCenterDrop = layoutSpy.count();
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
                             QPointF(centerGuideMove),
                             QPointF(centerGuideGlobal),
                             Qt::LeftButton,
                             Qt::NoButton,
                             Qt::NoModifier);
    QCoreApplication::sendEvent(propertiesTabBar, &releaseEvent);
    QVERIFY2(layoutSpy.count() > layoutCountBeforeCenterDrop,
             "Embedded center dock drops should apply the tab layout before the release event returns.");
    QCOMPARE(manager->property("antDockLastGuidedDropSynchronous").toBool(), true);
    QVERIFY(manager->property("antDockLastGuidedDropElapsedMs").isValid());
    QVERIFY(!manager->isDropPreviewVisible());
    const int layoutCountAfterCenterDrop = layoutSpy.count();
    QCoreApplication::sendPostedEvents(manager, QEvent::MetaCall);
    QCOMPARE(layoutSpy.count(), layoutCountAfterCenterDrop);
    QVERIFY(!manager->isDropPreviewVisible());
    QCOMPARE(manager->activeDropGuide(), AntDockManager::DockPlacement::None);
    QVERIFY(properties->graphicsEffect() == nullptr);
    QVERIFY(manager->tabifiedDockWidgets(preview).contains(properties) ||
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
    QCOMPARE(explorer->property("antDockUsesNativeCaptionFrame").toBool(), false);
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
        const int dockShadowMargin = shadow->property("shadowMargin").toInt();
        const QMargins dockShadowMargins(dockShadowMargin,
                                         dockShadowMargin,
                                         dockShadowMargin,
                                         dockShadowMargin);
        const QRect expectedDockShadowGeometry = explorer->geometry().adjusted(-dockShadowMargin,
                                                                               -dockShadowMargin,
                                                                               dockShadowMargin,
                                                                               dockShadowMargin);
        QTRY_COMPARE(explorer->property("antDockLegacySoftwareShadowGeometry").toRect(),
                     expectedDockShadowGeometry);
        QCOMPARE(explorer->property("antDockLegacySoftwareShadowGeometryMode").toString(),
                 QStringLiteral("qt-logical"));
        QCOMPARE(shadow->property("antDockLegacySoftwareShadowGeometryMode").toString(),
                 QStringLiteral("qt-logical"));
        QCOMPARE(shadow->property("antDockLegacySoftwareShadowGeometry").toRect().marginsRemoved(dockShadowMargins),
                 explorer->geometry());
        QTRY_COMPARE(shadow->geometry(), expectedDockShadowGeometry);
        QVERIFY(shadow->property("antDockLegacySoftwareShadowDevicePixelRatio").toReal() >= 1.0);

        const HWND shadowHwnd = reinterpret_cast<HWND>(shadow->winId());
        QVERIFY(shadowHwnd != nullptr);
        RECT nativeShadowRect{};
        QVERIFY(::GetWindowRect(shadowHwnd, &nativeShadowRect));
        const qreal shadowDpr = qMax<qreal>(1.0, shadow->devicePixelRatioF());
        const int nativeShadowWidth = nativeShadowRect.right - nativeShadowRect.left;
        const int nativeShadowHeight = nativeShadowRect.bottom - nativeShadowRect.top;
        QVERIFY(qAbs(nativeShadowWidth - qRound(expectedDockShadowGeometry.width() * shadowDpr)) <= 2);
        QVERIFY(qAbs(nativeShadowHeight - qRound(expectedDockShadowGeometry.height() * shadowDpr)) <= 2);
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
#if defined(Q_OS_WIN)
        QVERIFY(!dragOverlay->internalWinId());
#endif
    }
    if (QWidget* dropOverlay = manager->findChild<QWidget*>(QStringLiteral("AntDockDropPreviewWindow")))
    {
        QVERIFY(!dropOverlay->isVisible());
#if defined(Q_OS_WIN)
        QVERIFY(!dropOverlay->internalWinId());
#endif
    }
#if defined(Q_OS_WIN)
    if (QWidget* guideOverlay = manager->findChild<QWidget*>(QStringLiteral("AntDockGuideOverlay")))
    {
        QVERIFY(!guideOverlay->isVisible());
        QVERIFY(!guideOverlay->internalWinId());
    }
#endif
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
    QCOMPARE(explorer->property("antDockNativeEmbeddedParentApplied").toBool(), false);
    QCOMPARE(explorer->property("antDockNativeEmbeddedUsesQtBackingStore").toBool(), true);
    QVERIFY2(!explorer->internalWinId(),
             "Embedded AntDockWidget must stay non-native after float -> dock so Win10 AntWindow repaint composition remains live.");
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
#if defined(Q_OS_WIN)
        QVERIFY(!dragOverlay->internalWinId());
#endif
    }
    if (QWidget* dropOverlay = manager->findChild<QWidget*>(QStringLiteral("AntDockDropPreviewWindow")))
    {
        QVERIFY(!dropOverlay->isVisible());
#if defined(Q_OS_WIN)
        QVERIFY(!dropOverlay->internalWinId());
#endif
    }
#if defined(Q_OS_WIN)
    if (QWidget* guideOverlay = manager->findChild<QWidget*>(QStringLiteral("AntDockGuideOverlay")))
    {
        QVERIFY(!guideOverlay->isVisible());
        QVERIFY(!guideOverlay->internalWinId());
    }
#endif
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
    QCOMPARE(explorer->property("antDockNativeEmbeddedParentApplied").toBool(), false);
    QCOMPARE(explorer->property("antDockNativeEmbeddedUsesQtBackingStore").toBool(), true);
    QVERIFY2(!explorer->internalWinId(),
             "Embedded AntDockWidget must stay non-native after float -> dock so Win10 AntWindow repaint composition remains live.");
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
    const auto& darkToken = antTheme->tokens();
    QCOMPARE(manager->palette().color(QPalette::Window), darkToken.colorBgLayout);
    QCOMPARE(explorer->palette().color(QPalette::Window), darkToken.colorBgContainer);
    QCOMPARE(explorer->widget()->palette().color(QPalette::Window), darkToken.colorBgContainer);
    QCOMPARE(explorer->widget()->autoFillBackground(), true);
    if (QTabWidget* explorerArea = dockAreaForExtensionTest(explorer))
    {
        QCOMPARE(explorerArea->palette().color(QPalette::Window), darkToken.colorBgContainer);
        QCOMPARE(explorerArea->property("antDockAreaDarkSurfaceApplied").toBool(), true);
        if (QTabBar* tabBar = explorerArea->tabBar())
        {
            QCOMPARE(tabBar->palette().color(QPalette::Button), darkToken.colorBgElevated);
            QVERIFY(!tabBar->styleSheet().contains(QStringLiteral("#ffffff")));
        }
    }
    const QImage darkDockImage = renderForExtensionTest(manager);
    const QImage darkDockBodyImage = darkDockImage.copy(QRect(0,
                                                              qMin(72, darkDockImage.height()),
                                                              darkDockImage.width(),
                                                              qMax(0, darkDockImage.height() - 72)));
    const int darkContainerPixels = countNearColorForExtensionTest(darkDockImage, darkToken.colorBgContainer, 18);
    const int darkElevatedPixels = countNearColorForExtensionTest(darkDockImage, darkToken.colorBgElevated, 18);
    const int staleLightPixels = countNearColorForExtensionTest(darkDockBodyImage, QColor(QStringLiteral("#f5f5f5")), 8);
    QVERIFY2(darkContainerPixels > 12000,
             qPrintable(QStringLiteral("Dark DockWidget should render a visible container pane surface, counted %1 pixels.")
                            .arg(darkContainerPixels)));
    QVERIFY2(darkElevatedPixels > 1200,
             qPrintable(QStringLiteral("Dark DockWidget should render elevated inactive tab surfaces, counted %1 pixels.")
                            .arg(darkElevatedPixels)));
    QVERIFY2(staleLightPixels < 800,
             qPrintable(QStringLiteral("Dark DockWidget should not keep stale light layout background pixels, counted %1 pixels.")
                            .arg(staleLightPixels)));

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

        QSignalSpy hiddenDockedSpy(hiddenManager, &AntDockManager::dockWidgetDocked);
        const QPoint floatingPosBeforeHiddenDrag = hiddenExplorer->pos();
        hiddenManager->hide();
        QCoreApplication::processEvents(QEventLoop::AllEvents, 80);
        hiddenExplorer->show();
        hiddenExplorer->raise();
        QTRY_VERIFY(hiddenExplorer->isVisible());
        QCOMPARE(hiddenManager->isDockingSurfaceAvailable(), false);

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
        QCoreApplication::sendPostedEvents(hiddenManager, QEvent::MetaCall);
        QCOMPARE(hiddenDockedSpy.count(), 0);
        QCOMPARE(hiddenExplorer->isFloating(), true);
        QVERIFY(hiddenManager->floatingDockWidgets().contains(hiddenExplorer));
        QCOMPARE(dockAreaForExtensionTest(hiddenExplorer), nullptr);
        QCOMPARE(hiddenManager->property("antDockLastDropSurfaceAvailable").toBool(), false);
        QCOMPARE(hiddenManager->property("antDockLastHiddenSurfaceDropRejected").toBool(), true);

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
        host.setProperty("antWindowForceLegacyFramePolicy", true);
        host.setGeometry(80, 80, 900, 620);
        auto* page = new QWidget;
        auto* pageLayout = new QVBoxLayout(page);
        pageLayout->setContentsMargins(24, 24, 24, 24);
        pageLayout->setSpacing(12);

        auto* pageSwitch = new AntSwitch(page);
        pageSwitch->setObjectName(QStringLiteral("dockAntWindowPageSwitch"));
        pageSwitch->setFixedSize(92, 28);
        pageLayout->addWidget(pageSwitch);

        auto* pageProbe = new PaintProbeWidget(page);
        pageProbe->setFixedHeight(48);
        pageLayout->addWidget(pageProbe);

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

        // Capture a baseline animation cadence on the host AntWindow before
        // ever floating a dock so we have a fresh-window paint interval to
        // compare against the post-embed cadence. The Win10 re-embed stutter
        // shows up as a per-frame interval inflation, not a missed paint
        // count, so a baseline is what makes the regression check meaningful.
        auto drivePageProbeCadence = [&](int frames, int frameIntervalMs) {
            pageProbe->resetIntervalSamples();
            for (int frame = 0; frame < frames; ++frame)
            {
                const QColor cadenceColor((37 + frame * 11) % 256,
                                          (151 + frame * 7) % 256,
                                          (211 + frame * 5) % 256);
                pageProbe->setFillColor(cadenceColor);
                QElapsedTimer frameTimer;
                frameTimer.start();
                while (frameTimer.elapsed() < frameIntervalMs)
                {
                    QCoreApplication::processEvents(QEventLoop::AllEvents, frameIntervalMs);
                }
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents, 80);
        };
        constexpr int kCadenceFrames = 12;
        constexpr int kCadenceFrameIntervalMs = 16;
        drivePageProbeCadence(kCadenceFrames, kCadenceFrameIntervalMs);
        const qint64 baselineMaxIntervalMs = pageProbe->maxPaintIntervalMs();
        QVERIFY2(!pageProbe->paintIntervalsMs().isEmpty(),
                 "Baseline cadence should produce at least one interval sample.");

        windowManager->setDockWidgetFloating(
            windowExplorer,
            true,
            QRect(windowManager->mapToGlobal(QPoint(96, 96)), QSize(280, 190)));
        QTRY_VERIFY(windowExplorer->isFloating());

        dragFloatingDockBack(windowExplorer, windowInspector);
        QTRY_VERIFY(!windowExplorer->isFloating());
        QTRY_VERIFY(dockAreaForExtensionTest(windowExplorer) != nullptr);
        QCOMPARE(windowExplorer->property("antDockNativeEmbeddedParentApplied").toBool(), false);
        QCOMPARE(windowExplorer->property("antDockNativeEmbeddedUsesQtBackingStore").toBool(), true);
        QVERIFY2(!windowExplorer->internalWinId(),
                 "A dock embedded back into an AntWindow must not keep a native child HWND on the Win10 legacy frame path.");
        // The Win10 legacy software shadow widget must be fully gone after
        // re-embed: even a hidden top-level WS_EX_LAYERED window owned by
        // the dock disturbs the host AntWindow compositor cadence.
        QCOMPARE(windowExplorer->property("antDockLegacyShadowDestroyedOnEmbed").toBool(), true);
        QVERIFY2(windowExplorer->findChild<QWidget*>(QStringLiteral("AntDockLegacySoftwareShadow")) == nullptr,
                 "The dock legacy software shadow widget must be deleted on Win10 re-embed so no hidden layered HWND is left enrolled in DWM tracking.");
        // The DWM frame / native frame flags must be cleared on embed: a
        // queued frame-refresh fire from the float lifecycle could otherwise
        // re-extend DWM frame margins on the embedded child and recreate a
        // residual native HWND on the Win10 legacy frame path.
        QCOMPARE(windowExplorer->property("antDockNativeWindowFrameEnabled").toBool(), false);
        QCOMPARE(windowExplorer->property("antDockUsesNativeCaptionFrame").toBool(), false);
        QCOMPARE(windowExplorer->property("antDockLegacyLiveResize").toBool(), false);
        // No descendant of the dock should keep a native handle either; that
        // would leave a stale child HWND parented under the manager's host.
        const QList<QWidget*> embeddedDockDescendants = windowExplorer->findChildren<QWidget*>();
        for (QWidget* desc : embeddedDockDescendants)
        {
            QVERIFY2(!desc->internalWinId(),
                     qPrintable(QStringLiteral("Embedded dock descendant '%1' must not keep a native HWND after re-embed on Win10.")
                                    .arg(desc->objectName().isEmpty() ? desc->metaObject()->className() : desc->objectName())));
        }
        if (QWidget* dragOverlay = topLevelWidgetForExtensionTest(QStringLiteral("AntDockDragPreviewWindow")))
        {
            QVERIFY(!dragOverlay->isVisible());
            QVERIFY(!dragOverlay->internalWinId());
        }
        if (QWidget* dropOverlay = windowManager->findChild<QWidget*>(QStringLiteral("AntDockDropPreviewWindow")))
        {
            QVERIFY(!dropOverlay->isVisible());
            QVERIFY(!dropOverlay->internalWinId());
        }
        if (QWidget* guideOverlay = windowManager->findChild<QWidget*>(QStringLiteral("AntDockGuideOverlay")))
        {
            QVERIFY(!guideOverlay->isVisible());
            QVERIFY(!guideOverlay->internalWinId());
        }

        QTRY_VERIFY(pageProbe->paintCount() > 0);
        const int repaintBefore = pageProbe->paintCount();
        const QColor repaintColor(42, 176, 120);
        pageProbe->setFillColor(repaintColor);
        QTRY_VERIFY(pageProbe->paintCount() > repaintBefore);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 80);
        const QPixmap hostGrab = host.grab();
        const QImage hostImage = hostGrab.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
        const qreal devicePixelRatio = qMax<qreal>(1.0, hostGrab.devicePixelRatio());
        const QPoint sampleInHost = pageProbe->mapTo(&host, pageProbe->rect().center());
        const QPoint sampleInImage(qRound(sampleInHost.x() * devicePixelRatio),
                                   qRound(sampleInHost.y() * devicePixelRatio));
        QVERIFY(hostImage.rect().contains(sampleInImage));
        const QColor sampledColor = hostImage.pixelColor(sampleInImage);
        QVERIFY2(colorNearForExtensionTest(sampledColor, repaintColor, 24),
                 qPrintable(QStringLiteral("AntWindow child repaint should stay visible after dock float -> embed on the Win10 frame path. expected=%1 actual=%2")
                                .arg(colorStringForExtensionTest(repaintColor),
                                     colorStringForExtensionTest(sampledColor))));

        // Drive an identical animation-style cadence after re-embed and
        // compare against the baseline. The Win10 dock re-embed stutter does
        // not show up as a missing paint - it shows up as an inflated
        // max-interval between consecutive paints. Compare directly against
        // the baseline taken on the fresh host so we catch a real cadence
        // regression even when both runs deliver every frame.
        {
            const int cadenceBefore = pageProbe->paintCount();
            drivePageProbeCadence(kCadenceFrames, kCadenceFrameIntervalMs);
            QTRY_VERIFY2(pageProbe->paintCount() >= cadenceBefore + kCadenceFrames,
                         qPrintable(QStringLiteral("AntWindow child animation cadence stalled after dock float -> embed on Win10. paints before=%1 after=%2 expected>=%3")
                                        .arg(cadenceBefore)
                                        .arg(pageProbe->paintCount())
                                        .arg(cadenceBefore + kCadenceFrames)));
            const qint64 postEmbedMaxIntervalMs = pageProbe->maxPaintIntervalMs();
            // Allow some slack for OS-level scheduling jitter, but a stutter
            // regression (residual hidden layered HWND etc.) inflates the
            // worst frame gap to several times the baseline. 3x the baseline
            // (with a floor) is a comfortable threshold that still catches
            // the issue. Tighten if needed.
            const qint64 thresholdMs = qMax<qint64>(80, baselineMaxIntervalMs * 3);
            QVERIFY2(postEmbedMaxIntervalMs <= thresholdMs,
                     qPrintable(QStringLiteral("AntWindow child animation cadence regressed after dock float -> embed on Win10. baselineMax=%1ms postEmbedMax=%2ms threshold=%3ms")
                                    .arg(baselineMaxIntervalMs)
                                    .arg(postEmbedMaxIntervalMs)
                                    .arg(thresholdMs)));
            QCoreApplication::processEvents(QEventLoop::AllEvents, 80);
            const QPixmap cadenceGrab = host.grab();
            const QImage cadenceImage = cadenceGrab.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
            const qreal cadenceDpr = qMax<qreal>(1.0, cadenceGrab.devicePixelRatio());
            const QPoint cadenceSampleInImage(qRound(sampleInHost.x() * cadenceDpr),
                                              qRound(sampleInHost.y() * cadenceDpr));
            QVERIFY(cadenceImage.rect().contains(cadenceSampleInImage));
        }

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
            // On the forced-legacy AntWindow path the corner smoother is
            // intentionally not created (WA_TranslucentBackground + DWM
            // glass combination causes compositor instability on Win10,
            // so the opaque path skips both). On Win11 / non-legacy
            // AntWindow construction the smoother still exists.
            QWidget* smoother = host.findChild<QWidget*>(QStringLiteral("AntWindowCornerSmoother"));
            if (smoother)
            {
                QCOMPARE(smoother->property("antWindowCornerSmootherClickThrough").toBool(), true);
            }
        }
        releaseTopMostForExtensionTest(&host);
        QVERIFY(nativeMouseClickForExtensionTest(&host, pageSwitch->mapToGlobal(pageSwitch->rect().center())));
        QTRY_COMPARE(switchAfterSpy.count(), 1);
        QVERIFY(!pageSwitch->isChecked());

        auto verifyHostResizeAfterDockEmbed = [&](const QPoint& startLocal,
                                                  const QPoint& delta,
                                                  const char* edgeName) {
            host.setGeometry(80, 80, 900, 620);
            QCoreApplication::processEvents(QEventLoop::AllEvents, 80);
            QVERIFY(bringWidgetToFrontForExtensionTest(&host));

            const QRect before = host.geometry();
            const QPoint startGlobal = host.mapToGlobal(startLocal);
            const QPoint nativeStart = nativePointForExtensionTest(&host, startGlobal);
            POINT nativePoint{nativeStart.x(), nativeStart.y()};
            HWND nativeAtPoint = ::WindowFromPoint(nativePoint);
            QVERIFY2(nativeAtPoint != nullptr,
                     qPrintable(QStringLiteral("WindowFromPoint should find a window at the AntWindow %1 resize edge after a dock is re-embedded.")
                                    .arg(QString::fromLatin1(edgeName))));
            QVERIFY2(::GetAncestor(nativeAtPoint, GA_ROOT) == reinterpret_cast<HWND>(host.winId()),
                     qPrintable(QStringLiteral("The AntWindow %1 resize edge should still belong to the host root after a dock is re-embedded.")
                                    .arg(QString::fromLatin1(edgeName))));

            const int forwardedBefore = host.property("antWindowChildHitTestForwarded").toInt();
            const bool resizeHitStartedOnChildHwnd = nativeAtPoint != reinterpret_cast<HWND>(host.winId());
            releaseTopMostForExtensionTest(&host);
            QVERIFY2(nativeMouseDragFromWorkerForExtensionTest(&host, startGlobal, startGlobal + delta),
                     qPrintable(QStringLiteral("Native mouse drag should complete on the AntWindow %1 resize edge after a dock is re-embedded.")
                                    .arg(QString::fromLatin1(edgeName))));
            if (resizeHitStartedOnChildHwnd)
            {
                QTRY_VERIFY2(host.property("antWindowChildHitTestForwarded").toInt() > forwardedBefore,
                             qPrintable(QStringLiteral("The AntWindow %1 resize edge should forward child-HWND hit tests after a dock is re-embedded.")
                                            .arg(QString::fromLatin1(edgeName))));
            }

            if (delta.x() != 0)
            {
                QVERIFY2(host.width() > before.width() + 40,
                         qPrintable(QStringLiteral("Dragging the AntWindow %1 resize edge should grow width after a dock is re-embedded, before=%2 after=%3.")
                                        .arg(QString::fromLatin1(edgeName))
                                        .arg(before.width())
                                        .arg(host.width())));
            }
            if (delta.y() != 0)
            {
                QVERIFY2(host.height() > before.height() + 40,
                         qPrintable(QStringLiteral("Dragging the AntWindow %1 resize edge should grow height after a dock is re-embedded, before=%2 after=%3.")
                                        .arg(QString::fromLatin1(edgeName))
                                        .arg(before.height())
                                        .arg(host.height())));
            }
        };

        verifyHostResizeAfterDockEmbed(QPoint(host.width() - 2, AntWindow::TitleBarHeight + 140),
                                       QPoint(96, 0),
                                       "right");
        verifyHostResizeAfterDockEmbed(QPoint(2, AntWindow::TitleBarHeight + 140),
                                       QPoint(-96, 0),
                                       "left");
        verifyHostResizeAfterDockEmbed(QPoint(host.width() / 2, host.height() - 2),
                                       QPoint(0, 96),
                                       "bottom");

        // Multi-resize backing-store guard: rapidly grow/shrink the host
        // many times to verify the AntWindow backing store still flushes
        // non-blank frames after a dock float/embed cycle. Repeated WM_SIZE
        // cycles are where the user-reported "黑屏" symptom shows up - if
        // any residual Win10 state from the dock lifecycle starts blocking
        // DWM frame acceptance, the host grab eventually returns the bg
        // color only (all four corners of the content area become identical
        // to the desktop / no content pixels).
        {
            host.setGeometry(80, 80, 880, 600);
            QCoreApplication::processEvents(QEventLoop::AllEvents, 80);
            constexpr int kResizeCycles = 8;
            int nonBlankFrameCount = 0;
            QColor lastSeenContentColor;
            const QPoint contentSampleLocal = pageSwitch->mapTo(&host, pageSwitch->rect().center());
            for (int cycle = 0; cycle < kResizeCycles; ++cycle)
            {
                const int w = 760 + (cycle * 37) % 320;
                const int h = 520 + (cycle * 29) % 220;
                host.setGeometry(host.x(), host.y(), w, h);
                QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
                // Drive a paint on the embedded probe so we know there is
                // a fresh content frame to grab.
                pageProbe->setFillColor(QColor((47 + cycle * 13) % 256,
                                               (157 + cycle * 7) % 256,
                                               (211 + cycle * 5) % 256));
                QCoreApplication::processEvents(QEventLoop::AllEvents, 50);

                const QPixmap cycleGrab = host.grab();
                if (cycleGrab.isNull()) continue;
                const QImage cycleImage = cycleGrab.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
                const qreal cycleDpr = qMax<qreal>(1.0, cycleGrab.devicePixelRatio());
                const QPoint cycleSampleInImage(qRound(contentSampleLocal.x() * cycleDpr),
                                                qRound(contentSampleLocal.y() * cycleDpr));
                if (!cycleImage.rect().contains(cycleSampleInImage)) continue;
                const QColor cycleSampledColor = cycleImage.pixelColor(cycleSampleInImage);
                // Black = catastrophic backing-store failure (rgba 0,0,0,*).
                // Any real pixel from AntSwitch / page background is non-black.
                if (cycleSampledColor.red() > 4 || cycleSampledColor.green() > 4 || cycleSampledColor.blue() > 4)
                {
                    ++nonBlankFrameCount;
                    lastSeenContentColor = cycleSampledColor;
                }
            }
            QVERIFY2(nonBlankFrameCount >= kResizeCycles - 2,
                     qPrintable(QStringLiteral("AntWindow backing store must keep producing non-blank frames across repeated resizes after dock float/embed on Win10. nonBlank=%1/%2 lastSample=%3")
                                    .arg(nonBlankFrameCount)
                                    .arg(kResizeCycles)
                                    .arg(lastSeenContentColor.isValid()
                                             ? colorStringForExtensionTest(lastSeenContentColor)
                                             : QStringLiteral("invalid"))));
        }
    }
#endif

    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::widget()
{
    ThemeModeRestorerForExtensionTest guard;

    AntWidget w;
    QVERIFY(w.currentTheme() == Ant::ThemeMode::Default ||
            w.currentTheme() == Ant::ThemeMode::Dark);
    QCOMPARE(w.property("antWidgetThemeChangeCount").toInt(), 0);
    QCOMPARE(w.property("antWidgetThemeRepolishCount").toInt(), 0);
    QCOMPARE(w.property("antWidgetUpdateGeometryCount").toInt(), 0);
    QCOMPARE(w.property("antWidgetSurfaceUpdateCount").toInt(), 0);

    ThemeSizedAntWidget sized;
    sized.resize(180, 64);
    sized.show();
    QVERIFY(QTest::qWaitForWindowExposed(&sized));

    antTheme->setThemeMode(guard.alternateMode());
    QCoreApplication::processEvents();

    QCOMPARE(w.currentTheme(), guard.alternateMode());
    QCOMPARE(w.property("antWidgetThemeChangeCount").toInt(), 1);
    QCOMPARE(w.property("antWidgetThemeRepolishCount").toInt(), 0);
    QCOMPARE(w.property("antWidgetUpdateGeometryCount").toInt(), 0);
    QCOMPARE(w.property("antWidgetSurfaceUpdateCount").toInt(), 1);

    QCOMPARE(sized.property("antWidgetThemeChangeCount").toInt(), 1);
    QCOMPARE(sized.property("antWidgetThemeRepolishCount").toInt(), 0);
    QCOMPARE(sized.property("antWidgetUpdateGeometryCount").toInt(), 1);
    QCOMPARE(sized.property("antWidgetSurfaceUpdateCount").toInt(), 1);
    QCOMPARE(sized.property("antWidgetOnThemeChangedCount").toInt(), 1);
    QCOMPARE(sized.property("antWidgetCachedSizeHint").toSize(), sized.sizeHint());

    const QImage alternateImage = renderForExtensionTest(&sized);
    QVERIFY2(colorNearForExtensionTest(alternateImage.pixelColor(alternateImage.rect().center()),
                                       antTheme->tokens().colorBgContainer),
             qPrintable(QStringLiteral("AntWidget should repaint with current theme container color, actual %1 expected %2")
                            .arg(colorStringForExtensionTest(alternateImage.pixelColor(alternateImage.rect().center())),
                                 colorStringForExtensionTest(antTheme->tokens().colorBgContainer))));

    antTheme->setThemeMode(guard.alternateMode() == Ant::ThemeMode::Dark ? Ant::ThemeMode::Default
                                                                         : Ant::ThemeMode::Dark);
    QCoreApplication::processEvents();

    QCOMPARE(w.property("antWidgetThemeChangeCount").toInt(), 2);
    QCOMPARE(w.property("antWidgetThemeRepolishCount").toInt(), 0);
    QCOMPARE(w.property("antWidgetUpdateGeometryCount").toInt(), 0);
    QCOMPARE(w.property("antWidgetSurfaceUpdateCount").toInt(), 2);
    QCOMPARE(sized.property("antWidgetThemeChangeCount").toInt(), 2);
    QCOMPARE(sized.property("antWidgetUpdateGeometryCount").toInt(), 2);
    QCOMPARE(sized.property("antWidgetOnThemeChangedCount").toInt(), 2);
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
    QCOMPARE(w->isCloseConfirmationEnabled(), false);
    QCOMPARE(w->closeConfirmationTitle(), QStringLiteral("Exit application?"));
    QCOMPARE(w->closeConfirmationContent(), QStringLiteral("The window will close. Do you want to exit?"));
    QCOMPARE(w->closeConfirmationOkText(), QStringLiteral("Exit"));
    QCOMPARE(w->closeConfirmationCancelText(), QStringLiteral("Cancel"));
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
    QCOMPARE(w->property("antWindowTitleBarButtonRectCacheRebuildCount").toInt(), 0);
    const QRect closeButtonRect = w->titleBarButtonRect(AntWindow::TitleBarButton::Close);
    QVERIFY(!closeButtonRect.isNull());
    QCOMPARE(w->property("antWindowTitleBarButtonRectCacheRebuildCount").toInt(), 1);
    QVERIFY(!w->titleBarButtonRect(AntWindow::TitleBarButton::Pin).isNull());
    QVERIFY(!w->titleBarButtonRect(AntWindow::TitleBarButton::Theme).isNull());
    QVERIFY(!w->titleBarButtonRect(AntWindow::TitleBarButton::Minimize).isNull());
    QVERIFY(!w->titleBarButtonRect(AntWindow::TitleBarButton::Maximize).isNull());
    QCOMPARE(w->titleBarButtonRect(AntWindow::TitleBarButton::Close), closeButtonRect);
    QCOMPARE(w->property("antWindowTitleBarButtonRectCacheRebuildCount").toInt(), 1);

    QSignalSpy visibilitySpy(w, &AntWindow::titleBarButtonVisibilityChanged);
    QSignalSpy pinVisibilitySpy(w, &AntWindow::pinButtonVisibleChanged);
    const int dirtyUpdatesBeforeVisibility = w->property("antWindowTitleBarDirtyUpdateCount").toInt();
    w->setPinButtonVisible(false);
    QCOMPARE(w->isPinButtonVisible(), false);
    QCOMPARE(w->isTitleBarButtonVisible(AntWindow::TitleBarButton::Pin), false);
    QVERIFY(w->titleBarButtonRect(AntWindow::TitleBarButton::Pin).isNull());
    QCOMPARE(w->property("antWindowTitleBarButtonRectCacheRebuildCount").toInt(), 2);
    QCOMPARE(w->property("antWindowTitleBarDirtyUpdateCount").toInt(), dirtyUpdatesBeforeVisibility + 1);
    QCOMPARE(pinVisibilitySpy.count(), 1);
    QCOMPARE(visibilitySpy.count(), 1);
    w->setTitleBarButtonVisible(AntWindow::TitleBarButton::Pin, true);
    QCOMPARE(w->isPinButtonVisible(), true);
    QCOMPARE(w->property("antWindowTitleBarButtonRectCacheRebuildCount").toInt(), 3);
    QCOMPARE(w->property("antWindowTitleBarDirtyUpdateCount").toInt(), dirtyUpdatesBeforeVisibility + 2);
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

    QSignalSpy closeConfirmEnabledSpy(w, &AntWindow::closeConfirmationEnabledChanged);
    QSignalSpy closeConfirmTitleSpy(w, &AntWindow::closeConfirmationTitleChanged);
    QSignalSpy closeConfirmContentSpy(w, &AntWindow::closeConfirmationContentChanged);
    QSignalSpy closeConfirmOkSpy(w, &AntWindow::closeConfirmationOkTextChanged);
    QSignalSpy closeConfirmCancelSpy(w, &AntWindow::closeConfirmationCancelTextChanged);
    w->setCloseConfirmationEnabled(true);
    QCOMPARE(w->isCloseConfirmationEnabled(), true);
    QCOMPARE(closeConfirmEnabledSpy.count(), 1);
    w->setCloseConfirmationEnabled(false);
    QCOMPARE(w->isCloseConfirmationEnabled(), false);
    QCOMPARE(closeConfirmEnabledSpy.count(), 2);
    w->setCloseConfirmationEnabled(true);
    QCOMPARE(w->isCloseConfirmationEnabled(), true);
    QCOMPARE(closeConfirmEnabledSpy.count(), 3);
    w->setCloseConfirmationTitle(QStringLiteral("Leave workspace?"));
    w->setCloseConfirmationContent(QStringLiteral("Unsaved preview state will be closed."));
    w->setCloseConfirmationOkText(QStringLiteral("Exit"));
    w->setCloseConfirmationCancelText(QStringLiteral("Cancel exit"));
    QCOMPARE(w->closeConfirmationTitle(), QStringLiteral("Leave workspace?"));
    QCOMPARE(w->closeConfirmationContent(), QStringLiteral("Unsaved preview state will be closed."));
    QCOMPARE(w->closeConfirmationOkText(), QStringLiteral("Exit"));
    QCOMPARE(w->closeConfirmationCancelText(), QStringLiteral("Cancel exit"));
    QCOMPARE(closeConfirmTitleSpy.count(), 1);
    QCOMPARE(closeConfirmContentSpy.count(), 1);
    QCOMPARE(closeConfirmOkSpy.count(), 0);
    QCOMPARE(closeConfirmCancelSpy.count(), 1);

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

    AntWindow defaultCloseWindow;
    defaultCloseWindow.resize(320, 240);
    defaultCloseWindow.show();
    QVERIFY(QTest::qWaitForWindowExposed(&defaultCloseWindow));
    defaultCloseWindow.close();
    QTRY_VERIFY(!defaultCloseWindow.isVisible());
    QVERIFY(defaultCloseWindow.findChild<AntModal*>(QStringLiteral("AntWindowCloseConfirmationModal")) == nullptr);

    AntWindow confirmWindow;
    confirmWindow.resize(640, 420);
    confirmWindow.setCloseConfirmationEnabled(true);
    confirmWindow.setCloseConfirmationTitle(QStringLiteral("Leave workspace?"));
    confirmWindow.setCloseConfirmationContent(QStringLiteral("Close this AntWindow now?"));
    confirmWindow.setCloseConfirmationOkText(QStringLiteral("Exit"));
    confirmWindow.setCloseConfirmationCancelText(QStringLiteral("Cancel exit"));
    confirmWindow.show();
    QVERIFY(QTest::qWaitForWindowExposed(&confirmWindow));

    confirmWindow.close();
    QVERIFY(confirmWindow.isVisible());
    auto* modal = confirmWindow.findChild<AntModal*>(QStringLiteral("AntWindowCloseConfirmationModal"));
    QVERIFY(modal != nullptr);
    QTRY_VERIFY(modal->isOpen());
    QCOMPARE(modal->title(), QStringLiteral("Leave workspace?"));
    QCOMPARE(modal->content(), QStringLiteral("Close this AntWindow now?"));
    QCOMPARE(modal->okText(), QStringLiteral("Exit"));
    QCOMPARE(modal->cancelText(), QStringLiteral("Cancel exit"));

    auto findModalButton = [](AntModal* target, const QString& text) -> AntButton* {
        const auto buttons = target->findChildren<AntButton*>();
        for (auto* button : buttons)
        {
            if (button->text() == text)
            {
                return button;
            }
        }
        return nullptr;
    };

    auto* cancelButton = findModalButton(modal, QStringLiteral("Cancel exit"));
    QVERIFY(cancelButton != nullptr);
    cancelButton->click();
    QTRY_VERIFY(!modal->isOpen());
    QVERIFY(confirmWindow.isVisible());

    confirmWindow.close();
    QTRY_VERIFY(modal->isOpen());
    auto* okButton = findModalButton(modal, QStringLiteral("Exit"));
    QVERIFY(okButton != nullptr);
    okButton->click();
    QTRY_VERIFY(!confirmWindow.isVisible());

    AntWindow forceCloseWindow;
    forceCloseWindow.resize(320, 240);
    forceCloseWindow.show();
    QVERIFY(QTest::qWaitForWindowExposed(&forceCloseWindow));
    forceCloseWindow.forceClose();
    QTRY_VERIFY(!forceCloseWindow.isVisible());
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
    auto* content = new QWidget;
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* themeSized = new ThemeSizedWidget(content);
    layout->addWidget(themeSized);
    layout->addStretch();
    window.setCentralWidget(content);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QCOMPARE(themeSized->height(), 32);

    QTest::mouseClick(&window,
                      Qt::LeftButton,
                      Qt::NoModifier,
                      window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center());
    QCOMPARE(antTheme->themeMode(), Ant::ThemeMode::Dark);
    QCOMPARE(themeSized->height(), 96);

    auto* overlay = window.findChild<QWidget*>(QStringLiteral("AntWindowThemeTransitionOverlay"));
    QVERIFY(overlay != nullptr);
    QVERIFY(overlay->isVisible());
    QVERIFY(overlay->testAttribute(Qt::WA_TransparentForMouseEvents));
    QCOMPARE(overlay->geometry(), window.rect());
    QCOMPARE(overlay->property("transitionFrameIntervalMs").toInt(), 16);
    QCOMPARE(overlay->property("transitionDurationMs").toInt(), 220);
    QCOMPARE(overlay->property("transitionMotionCurve").toString(), QStringLiteral("smootherstep"));
    QCOMPARE(overlay->property("transitionEdgeFeather").toInt(), 16);
    QCOMPARE(overlay->property("transitionDrawsCapturedNewFrame").toBool(), true);
    QCOMPARE(overlay->property("transitionCaptureMethod").toString(), QStringLiteral("render"));
    QCOMPARE(overlay->property("transitionUsesEventLoopCapture").toBool(), false);
    QCOMPARE(overlay->property("transitionMode").toString(),
             window.usesLegacyOpaquePath() ? QStringLiteral("crossfade") : QStringLiteral("circular-reveal"));

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

        bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override
        {
#ifdef Q_OS_WIN
            if ((eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") && message)
            {
                auto* msg = static_cast<MSG*>(message);
                if (msg->message == WM_NCLBUTTONDOWN)
                {
                    ++nativeNonClientDownCount;
                    lastNativeDownHitTest = static_cast<int>(msg->wParam);
                }
                else if (msg->message == WM_LBUTTONDOWN)
                {
                    ++nativeClientDownCount;
                }
            }
#endif
            return AntWindow::nativeEvent(eventType, message, result);
        }

        int nativeNonClientDownCount = 0;
        int nativeClientDownCount = 0;
        int lastNativeDownHitTest = 0;
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
    if (nativeMouseInputAvailableForExtensionTest())
    {
        window.showNormal();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        window.setGeometry(120, 120, 640, 420);
        QCoreApplication::processEvents();

        const int widthBeforeResizeDrag = window.width();
        const int ncDownBeforeResizeDrag = window.nativeNonClientDownCount;
        const int clientDownBeforeResizeDrag = window.nativeClientDownCount;
        const QPoint resizeStart = window.mapToGlobal(QPoint(window.width() - 2, AntWindow::TitleBarHeight + 80));
        const QPoint resizeEnd = resizeStart + QPoint(96, 0);
        const QPoint nativeResizeStart = nativePointForExtensionTest(&window, resizeStart);
        POINT resizeStartPoint{nativeResizeStart.x(), nativeResizeStart.y()};
        HWND resizeStartHwnd = ::WindowFromPoint(resizeStartPoint);
        QWidget* resizeStartWidget = resizeStartHwnd ? QWidget::find(reinterpret_cast<WId>(resizeStartHwnd)) : nullptr;
        QVERIFY2(resizeStartHwnd != nullptr,
                 "WindowFromPoint should find a native window at the AntWindow resize edge.");
        QVERIFY2(::GetAncestor(resizeStartHwnd, GA_ROOT) == hwnd,
                 qPrintable(QStringLiteral("AntWindow resize drag must start over the AntWindow HWND; hit widget=%1 native=0x%2 root=0x%3 expectedRoot=0x%4")
                                .arg(resizeStartWidget ? resizeStartWidget->objectName() : QStringLiteral("<none>"))
                                .arg(reinterpret_cast<quintptr>(resizeStartHwnd), 0, 16)
                                .arg(reinterpret_cast<quintptr>(::GetAncestor(resizeStartHwnd, GA_ROOT)), 0, 16)
                                .arg(reinterpret_cast<quintptr>(hwnd), 0, 16)));
        QVERIFY2(resizeStartHwnd == hwnd,
                 qPrintable(QStringLiteral("Resize edge should resolve to AntWindow's HWND, but hit child widget=%1 native=0x%2 root=0x%3 expected=0x%4")
                                .arg(resizeStartWidget ? resizeStartWidget->objectName() : QStringLiteral("<none>"))
                                .arg(reinterpret_cast<quintptr>(resizeStartHwnd), 0, 16)
                                .arg(reinterpret_cast<quintptr>(::GetAncestor(resizeStartHwnd, GA_ROOT)), 0, 16)
                                .arg(reinterpret_cast<quintptr>(hwnd), 0, 16)));
        QVERIFY(nativeMouseDragFromWorkerForExtensionTest(&window, resizeStart, resizeEnd));
        QVERIFY2(window.nativeNonClientDownCount > ncDownBeforeResizeDrag,
                 qPrintable(QStringLiteral("Right-edge drag should enter WM_NCLBUTTONDOWN; nc before=%1 after=%2 client before=%3 after=%4 lastHit=%5")
                                .arg(ncDownBeforeResizeDrag)
                                .arg(window.nativeNonClientDownCount)
                                .arg(clientDownBeforeResizeDrag)
                                .arg(window.nativeClientDownCount)
                                .arg(window.lastNativeDownHitTest)));
        QCOMPARE(window.lastNativeDownHitTest, static_cast<int>(HTRIGHT));
        QVERIFY2(window.width() > widthBeforeResizeDrag + 40,
                 qPrintable(QStringLiteral("Dragging the native right resize edge should grow AntWindow width, before=%1 after=%2")
                                .arg(widthBeforeResizeDrag)
                                .arg(window.width())));
    }
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
    auto* probe = new PaintProbeWidget;
    window.setCentralWidget(probe);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QCOMPARE(window.property("antWindowUsesNativeCaptionFrame").toBool(), false);
    // The Win10 legacy AntWindow path now skips both WA_TranslucentBackground
    // and the corner smoother in exchange for compositor stability across
    // repeated resize cycles. The smoother widget therefore does not exist on
    // this path - only check its non-native invariant when it is present
    // (i.e. when the test happens to run on a Win11-class build that took the
    // translucent-background path before the legacy property was applied).
    QWidget* smoother = window.findChild<QWidget*>(QStringLiteral("AntWindowCornerSmoother"));
    if (smoother)
    {
        QTRY_COMPARE(smoother->property("antWindowCornerSmootherClickThrough").toBool(), true);
        QCOMPARE(smoother->property("antWindowCornerSmootherNativeHwnd").toBool(), false);
        QVERIFY2(!smoother->internalWinId(),
                 "Win10 corner smoother must stay as a non-native child; a full-window native child overlay can freeze child repaint composition.");
    }
    QTRY_VERIFY(probe->paintCount() > 0);
    const int paintCountBeforeProbeUpdate = probe->paintCount();
    const QColor probeColor(220, 40, 72);
    probe->setFillColor(probeColor);
    QTRY_VERIFY2(probe->paintCount() > paintCountBeforeProbeUpdate,
                 "Child widgets inside the Win10 AntWindow path must repaint after update().");
    const QPixmap grabbedWindow = window.grab();
    const QImage grabbedImage = grabbedWindow.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const qreal grabDpr = qMax<qreal>(1.0, grabbedWindow.devicePixelRatio());
    const QPoint sampleInWindow = probe->mapTo(&window, probe->rect().center());
    const QPoint sampleInImage(qRound(sampleInWindow.x() * grabDpr),
                               qRound(sampleInWindow.y() * grabDpr));
    QVERIFY(grabbedImage.rect().contains(sampleInImage));
    const QColor sampledProbeColor = grabbedImage.pixelColor(sampleInImage);
    QVERIFY2(colorNearForExtensionTest(sampledProbeColor, probeColor, 24),
             qPrintable(QStringLiteral("Win10 legacy AntWindow should show child repaint pixels, expected %1 got %2")
                            .arg(colorStringForExtensionTest(probeColor),
                                 colorStringForExtensionTest(sampledProbeColor))));
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
    const int shadowMargin = window.property("antWindowLegacySoftwareShadowMargin").toInt();
    const QMargins shadowMargins(shadowMargin, shadowMargin, shadowMargin, shadowMargin);
    const QRect initialShadowGeometry = window.property("antWindowLegacySoftwareShadowGeometry").toRect();
    QCOMPARE(initialShadowGeometry.marginsRemoved(shadowMargins), window.geometry());
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
    QCOMPARE(window.property("antWindowLegacySoftwareShadowGeometryMode").toString(), QStringLiteral("qt-logical"));
    QCOMPARE(shadowWidget->property("antWindowLegacySoftwareShadowGeometryMode").toString(), QStringLiteral("qt-logical"));
    QTRY_COMPARE(shadowWidget->geometry(), initialShadowGeometry);

    const HWND ownerHwnd = reinterpret_cast<HWND>(window.winId());
    const HWND shadowHwnd = reinterpret_cast<HWND>(shadowWidget->winId());
    QVERIFY(ownerHwnd != nullptr);
    QVERIFY(shadowHwnd != nullptr);
    RECT nativeShadowRect{};
    QVERIFY(::GetWindowRect(shadowHwnd, &nativeShadowRect));
    const qreal shadowDpr = qMax<qreal>(1.0, shadowWidget->devicePixelRatioF());
    const int nativeShadowWidth = nativeShadowRect.right - nativeShadowRect.left;
    const int nativeShadowHeight = nativeShadowRect.bottom - nativeShadowRect.top;
    QVERIFY(qAbs(nativeShadowWidth - qRound(initialShadowGeometry.width() * shadowDpr)) <= 2);
    QVERIFY(qAbs(nativeShadowHeight - qRound(initialShadowGeometry.height() * shadowDpr)) <= 2);

    ::SendMessageW(ownerHwnd, WM_ENTERSIZEMOVE, 0, 0);
    QCoreApplication::processEvents();
    QCOMPARE(window.property("antWindowLegacyLiveResize").toBool(), false);
    QTRY_VERIFY(shadowWidget->isVisible());
    window.move(window.pos() + QPoint(18, 12));
    QTRY_COMPARE(window.property("antWindowLegacySoftwareShadowGeometry").toRect().marginsRemoved(shadowMargins),
                 window.geometry());
    QTRY_COMPARE(shadowWidget->geometry(), window.property("antWindowLegacySoftwareShadowGeometry").toRect());
    ::SendMessageW(ownerHwnd, WM_EXITSIZEMOVE, 0, 0);
    QCoreApplication::processEvents();

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
    QCOMPARE(resizedShadowGeometry.marginsRemoved(shadowMargins), window.geometry());
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
    auto* popup = w->findChild<QFrame*>(QStringLiteral("AntColorPickerPopup"));
    QVERIFY(popup != nullptr);
    QTRY_VERIFY(popup->isVisible());
    QCOMPARE(popup->windowType(), Qt::Tool);
    QCOMPARE(popup->frameShape(), QFrame::NoFrame);
    QVERIFY(popup->testAttribute(Qt::WA_TranslucentBackground));
    QVERIFY(popup->testAttribute(Qt::WA_NoSystemBackground));
    QVERIFY(popup->windowFlags().testFlag(Qt::NoDropShadowWindowHint));
    QCOMPARE(popup->property("antColorPickerPopupNoNativeShadow").toBool(), true);
    QCOMPARE(popup->property("antColorPickerPopupShadowMargin").toInt(), 28);
    QCOMPARE(popup->property("antColorPickerPopupShadowWidth").toInt(), 12);
    QVERIFY(popup->property("antColorPickerPopupShadowStrength").toReal() <= 0.30);
    QCOMPARE(popup->property("antColorPickerPopupMotionEnabled").toBool(), true);
    QCOMPARE(popup->property("antColorPickerPopupMotionDistance").toInt(), 10);
    QCOMPARE(popup->property("antColorPickerPopupUsesManualOutsideClose").toBool(), true);
    QCOMPARE(popup->property("antColorPickerPopupMotionPlacement").toString(), QStringLiteral("bottom"));
    QCOMPARE(popup->property("antColorPickerPopupEnterMotionStarted").toBool(), true);
    QCOMPARE(popup->property("antColorPickerCoalescesDragRefresh").toBool(), true);
    QCOMPARE(popup->property("antColorPickerLiveRefreshIntervalMs").toInt(), 16);
    const int popupShadowMargin = popup->property("antColorPickerPopupShadowMargin").toInt();
    const int popupShadowWidth = popup->property("antColorPickerPopupShadowWidth").toInt();
    QVERIFY(popupShadowMargin <= popupShadowWidth * 3);

    QImage popupImage(popup->size(), QImage::Format_ARGB32_Premultiplied);
    popupImage.fill(Qt::transparent);
    {
        QPainter painter(&popupImage);
        popup->render(&painter);
    }
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
    QVERIFY(maxAlphaIn(popupImage, QRect(0, popupImage.height() - 2, popupImage.width(), 2)) <= 1);
    const QRect panelRect = popup->rect().adjusted(popupShadowMargin, popupShadowMargin, -popupShadowMargin, -popupShadowMargin);
    const int nearBottomAlpha = maxAlphaIn(popupImage, QRect(panelRect.left() + 16, panelRect.bottom() + 2, panelRect.width() - 32, 2));
    const int midBottomAlpha = maxAlphaIn(popupImage, QRect(panelRect.left() + 16, panelRect.bottom() + 12, panelRect.width() - 32, 2));
    const int farBottomAlpha = maxAlphaIn(popupImage, QRect(panelRect.left() + 16, panelRect.bottom() + 26, panelRect.width() - 32, 2));
    QVERIFY(nearBottomAlpha >= midBottomAlpha);
    QVERIFY(midBottomAlpha >= farBottomAlpha);
    QVERIFY(farBottomAlpha <= 4);

    auto* hueSatField = popup->findChild<QWidget*>(QStringLiteral("AntColorPickerHueSatField"));
    QVERIFY(hueSatField != nullptr);
    QCOMPARE(hueSatField->property("antColorPickerOpaqueFieldPaint").toBool(), true);
    QVERIFY(hueSatField->testAttribute(Qt::WA_OpaquePaintEvent));
    QVERIFY(hueSatField->testAttribute(Qt::WA_NoSystemBackground));
    QVERIFY(hueSatField->testAttribute(Qt::WA_StaticContents));
    QCOMPARE(hueSatField->property("antColorPickerNativeDragSurface").toBool(), false);
    QVERIFY(!hueSatField->testAttribute(Qt::WA_NativeWindow));
    QCOMPARE(hueSatField->property("antColorPickerUsesCursorOverlay").toBool(), false);
    QCOMPARE(hueSatField->property("antColorPickerDrawsCursorInField").toBool(), true);
    QCOMPARE(hueSatField->property("antColorPickerCursorDirtyRepaint").toBool(), true);
    QVERIFY(popup->findChild<QWidget*>(QStringLiteral("AntColorPickerHueSatCursor")) == nullptr);
    QCOMPARE(hueSatField->property("antColorPickerUsesCachedFieldBackground").toBool(), true);
    QImage fieldImage(hueSatField->size(), QImage::Format_ARGB32_Premultiplied);
    fieldImage.fill(Qt::transparent);
    {
        QPainter painter(&fieldImage);
        hueSatField->render(&painter);
    }
    QCOMPARE(fieldImage.pixelColor(0, 0).alpha(), 255);
    const int cacheBuildCount = hueSatField->property("antColorPickerFieldBackgroundCacheBuilds").toInt();
    const int fieldPaintCount = hueSatField->property("antColorPickerFieldPaintCount").toInt();
    QVERIFY(cacheBuildCount > 0);
    QSignalSpy selectedSpy(w, &AntColorPicker::colorSelected);
    const int refreshCountBeforeDrag = popup->property("antColorPickerLiveRefreshCount").toInt();
    QMouseEvent fieldPress(QEvent::MouseButtonPress,
                           QPointF(24, 24),
                           Qt::LeftButton,
                           Qt::LeftButton,
                           Qt::NoModifier);
    QCoreApplication::sendEvent(hueSatField, &fieldPress);
    for (int i = 0; i < 12; ++i)
    {
        QMouseEvent fieldMove(QEvent::MouseMove,
                              QPointF(30 + i * 8, 36 + i * 3),
                              Qt::NoButton,
                              Qt::LeftButton,
                              Qt::NoModifier);
        QCoreApplication::sendEvent(hueSatField, &fieldMove);
    }
    QCoreApplication::processEvents();
    QTRY_COMPARE(popup->property("antColorPickerLiveRefreshPending").toBool(), false);
    QCOMPARE(popup->property("antColorPickerLiveRefreshCount").toInt(), refreshCountBeforeDrag + 1);
    QCOMPARE(selectedSpy.count(), 1);
    QCOMPARE(colorSpy.count(), 2);
    QCOMPARE(hueSatField->property("antColorPickerCursorPoint").toPoint(), QPoint(118, 69));
    fieldImage.fill(Qt::transparent);
    {
        QPainter painter(&fieldImage);
        hueSatField->render(&painter);
    }
    QCOMPARE(hueSatField->property("antColorPickerFieldBackgroundCacheBuilds").toInt(), cacheBuildCount);
    QVERIFY(hueSatField->property("antColorPickerFieldPaintCount").toInt() <= fieldPaintCount + 3);
    w->setOpen(false);
    QCOMPARE(w->isOpen(), false);
    QCOMPARE(openSpy.count(), 2);
    QVERIFY(popup->isVisible());
    QCOMPARE(popup->property("antColorPickerPopupLeaveMotionStarted").toBool(), true);
    QTRY_VERIFY(!popup->isVisible());
    QCOMPARE(popup->windowOpacity(), 1.0);

    auto* w2 = new AntColorPicker(Qt::blue);
    QCOMPARE(w2->currentColor(), QColor(Qt::blue));
}

void TestAntQtExtensions::colorPickerDragSmoothness()
{
    auto* w = new AntColorPicker;
    QSignalSpy colorSpy(w, &AntColorPicker::currentColorChanged);
    QSignalSpy selectedSpy(w, &AntColorPicker::colorSelected);
    w->setOpen(true);

    auto* popup = w->findChild<QFrame*>(QStringLiteral("AntColorPickerPopup"));
    QVERIFY(popup != nullptr);
    QTRY_VERIFY(popup->isVisible());
    auto* hueSatField = popup->findChild<QWidget*>(QStringLiteral("AntColorPickerHueSatField"));
    QVERIFY(hueSatField != nullptr);
    QVERIFY(popup->findChild<QWidget*>(QStringLiteral("AntColorPickerHueSatCursor")) == nullptr);

    QImage warmup(hueSatField->size(), QImage::Format_ARGB32_Premultiplied);
    warmup.fill(Qt::transparent);
    {
        QPainter painter(&warmup);
        hueSatField->render(&painter);
    }
    const int cacheBuildCount = hueSatField->property("antColorPickerFieldBackgroundCacheBuilds").toInt();
    const int fieldPaintCount = hueSatField->property("antColorPickerFieldPaintCount").toInt();
    const int refreshCountBeforeDrag = popup->property("antColorPickerLiveRefreshCount").toInt();

    QMouseEvent fieldPress(QEvent::MouseButtonPress,
                           QPointF(6, 6),
                           Qt::LeftButton,
                           Qt::LeftButton,
                           Qt::NoModifier);
    QCoreApplication::sendEvent(hueSatField, &fieldPress);

    constexpr int moveCount = 240;
    QPoint finalPoint;
    QElapsedTimer timer;
    timer.start();
    for (int i = 0; i < moveCount; ++i)
    {
        finalPoint = QPoint(2 + (i * 7) % (hueSatField->width() - 4),
                            2 + (i * 5) % (hueSatField->height() - 4));
        QMouseEvent fieldMove(QEvent::MouseMove,
                              QPointF(finalPoint),
                              Qt::NoButton,
                              Qt::LeftButton,
                              Qt::NoModifier);
        QCoreApplication::sendEvent(hueSatField, &fieldMove);
        QCOMPARE(hueSatField->property("antColorPickerCursorPoint").toPoint(), finalPoint);
    }
    const qint64 elapsedMs = timer.elapsed();
    QVERIFY2(elapsedMs < 80,
             qPrintable(QStringLiteral("ColorPicker drag dispatch should stay below 80ms for %1 moves, actual %2ms")
                        .arg(moveCount)
                        .arg(elapsedMs)));

    QTRY_COMPARE(popup->property("antColorPickerLiveRefreshPending").toBool(), false);
    QCOMPARE(popup->property("antColorPickerLiveRefreshCount").toInt(), refreshCountBeforeDrag + 1);
    QCOMPARE(colorSpy.count(), 1);
    QCOMPARE(selectedSpy.count(), 1);
    QCOMPARE(hueSatField->property("antColorPickerFieldBackgroundCacheBuilds").toInt(), cacheBuildCount);
    QVERIFY(hueSatField->property("antColorPickerFieldPaintCount").toInt() <= fieldPaintCount + 3);
    QCOMPARE(hueSatField->property("antColorPickerCursorPoint").toPoint(), finalPoint);

    QImage dragImage(hueSatField->size(), QImage::Format_ARGB32_Premultiplied);
    dragImage.fill(Qt::transparent);
    {
        QPainter painter(&dragImage);
        hueSatField->render(&painter);
    }
    const QRect cursorRect = hueSatField->property("antColorPickerCursorRect").toRect().adjusted(-2, -2, 2, 2);
    int strayWhitePixels = 0;
    for (int y = 8; y < dragImage.height() - 8; ++y)
    {
        for (int x = 120; x < dragImage.width() - 8; ++x)
        {
            if (cursorRect.contains(x, y))
            {
                continue;
            }
            const QColor pixel = dragImage.pixelColor(x, y);
            if (pixel.alpha() > 220 && pixel.red() > 220 && pixel.green() > 220 && pixel.blue() > 220)
            {
                ++strayWhitePixels;
            }
        }
    }
    QCOMPARE(strayWhitePixels, 0);

    w->setOpen(false);
}

QTEST_MAIN(TestAntQtExtensions)
#include "TestAntQtExtensions.moc"
