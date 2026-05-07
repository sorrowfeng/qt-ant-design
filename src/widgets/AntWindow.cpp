#include "AntWindow.h"

#include <QCursor>
#include <QEvent>
#include <QGuiApplication>
#include <QMetaType>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QScreen>
#include <QShowEvent>
#include <QStyleHints>
#include <QVBoxLayout>
#include <QWindow>

#include "../styles/AntWindowStyle.h"
#include "core/AntTheme.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#endif

namespace
{
constexpr AntWindow::TitleBarButton kTitleBarButtonsRightToLeft[] = {
    AntWindow::TitleBarButton::Close,
    AntWindow::TitleBarButton::Maximize,
    AntWindow::TitleBarButton::Minimize,
    AntWindow::TitleBarButton::Theme,
    AntWindow::TitleBarButton::Pin,
};

#ifdef Q_OS_WIN
using GetDpiForWindowFn = UINT(WINAPI*)(HWND);
using GetSystemMetricsForDpiFn = int(WINAPI*)(int, UINT);
using DwmSetWindowAttributeFn = HRESULT(WINAPI*)(HWND, DWORD, LPCVOID, DWORD);

struct DwmMargins
{
    int cxLeftWidth;
    int cxRightWidth;
    int cyTopHeight;
    int cyBottomHeight;
};

using DwmExtendFrameIntoClientAreaFn = HRESULT(WINAPI*)(HWND, const DwmMargins*);

constexpr DWORD kDwmUseImmersiveDarkMode = 20;
constexpr DWORD kDwmWindowCornerPreference = 33;
constexpr DWORD kDwmBorderColor = 34;
constexpr int kDwmCornerDoNotRound = 1;
constexpr int kDwmCornerRound = 2;
constexpr int kDwmCornerRoundSmall = 3;

UINT dpiForWindow(HWND hwnd)
{
    if (HMODULE user32 = ::GetModuleHandleW(L"user32.dll"))
    {
        auto* getDpiForWindow = reinterpret_cast<GetDpiForWindowFn>(::GetProcAddress(user32, "GetDpiForWindow"));
        if (getDpiForWindow)
        {
            return getDpiForWindow(hwnd);
        }
    }

    return 96;
}

int systemMetricForDpi(int metric, UINT dpi)
{
    if (HMODULE user32 = ::GetModuleHandleW(L"user32.dll"))
    {
        auto* getSystemMetricsForDpi =
            reinterpret_cast<GetSystemMetricsForDpiFn>(::GetProcAddress(user32, "GetSystemMetricsForDpi"));
        if (getSystemMetricsForDpi)
        {
            return getSystemMetricsForDpi(metric, dpi);
        }
    }

    Q_UNUSED(dpi)
    return ::GetSystemMetrics(metric);
}

int resizeBorderMetric(HWND hwnd, int frameMetric, int paddedMetric)
{
    const UINT dpi = dpiForWindow(hwnd);
    const int frame = systemMetricForDpi(frameMetric, dpi);
    const int padded = systemMetricForDpi(paddedMetric, dpi);
    return qMax(8, frame + padded);
}

qintptr nativeHitTestForTitleBarButton(AntWindow::TitleBarButton button)
{
    switch (button)
    {
    case AntWindow::TitleBarButton::Minimize:
        return HTREDUCE;
    case AntWindow::TitleBarButton::Maximize:
        return HTZOOM;
    case AntWindow::TitleBarButton::Close:
        return HTCLOSE;
    default:
        return HTCLIENT;
    }
}

AntWindow::TitleBarButton titleBarButtonForNativeHitTest(WPARAM hitTestCode)
{
    switch (hitTestCode)
    {
    case HTREDUCE:
        return AntWindow::TitleBarButton::Minimize;
    case HTZOOM:
        return AntWindow::TitleBarButton::Maximize;
    case HTCLOSE:
        return AntWindow::TitleBarButton::Close;
    default:
        return AntWindow::TitleBarButton::None;
    }
}

bool resolveDwmApis(DwmSetWindowAttributeFn* setWindowAttribute, DwmExtendFrameIntoClientAreaFn* extendFrame)
{
    HMODULE dwmapi = ::LoadLibraryW(L"dwmapi.dll");
    if (!dwmapi)
    {
        return false;
    }

    if (setWindowAttribute)
    {
        *setWindowAttribute =
            reinterpret_cast<DwmSetWindowAttributeFn>(::GetProcAddress(dwmapi, "DwmSetWindowAttribute"));
    }
    if (extendFrame)
    {
        *extendFrame =
            reinterpret_cast<DwmExtendFrameIntoClientAreaFn>(::GetProcAddress(dwmapi, "DwmExtendFrameIntoClientArea"));
    }

    return (setWindowAttribute && *setWindowAttribute) || (extendFrame && *extendFrame);
}

void triggerFrameChange(HWND hwnd)
{
    ::SetWindowPos(hwnd,
                   nullptr,
                   0,
                   0,
                   0,
                   0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

void ensureNativeSnapWindowStyle(HWND hwnd)
{
    LONG_PTR style = ::GetWindowLongPtrW(hwnd, GWL_STYLE);
    const LONG_PTR snapStyle = WS_THICKFRAME | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU;
    const LONG_PTR newStyle = style | snapStyle;
    if (newStyle == style)
    {
        return;
    }

    ::SetWindowLongPtrW(hwnd, GWL_STYLE, newStyle);
    triggerFrameChange(hwnd);
}
#endif
}

AntWindow::AntWindow(QWidget* parent)
    : QMainWindow(parent)
{
    qRegisterMetaType<AntWindow::TitleBarButton>("AntWindow::TitleBarButton");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground, false);
    setAttribute(Qt::WA_Hover, true);
    installAntStyle<AntWindowStyle>(this);
    setMinimumSize(400, 300);
    syncTheme();

    m_contentWidget = new QWidget(this);
    m_contentWidget->installEventFilter(this);
    auto* contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(0, TitleBarHeight, 0, 0);
    contentLayout->setSpacing(0);
    QMainWindow::setCentralWidget(m_contentWidget);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        syncTheme();
        update();
    });
}

void AntWindow::setWindowTitle(const QString& title)
{
    if (QMainWindow::windowTitle() == title)
    {
        return;
    }

    QMainWindow::setWindowTitle(title);
    update(titleBarRect());
    Q_EMIT windowTitleChanged(title);
}

void AntWindow::setCentralWidget(QWidget* widget)
{
    if (!m_contentWidget)
    {
        return;
    }

    QLayout* layout = m_contentWidget->layout();
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (QWidget* w = item->widget())
        {
            w->setParent(nullptr);
        }
        delete item;
    }

    if (widget)
    {
        applyContentPalette(widget);
        layout->addWidget(widget);
    }
}

bool AntWindow::isMaximized() const
{
    return QMainWindow::isMaximized();
}

bool AntWindow::isAlwaysOnTop() const
{
    return m_alwaysOnTop;
}

void AntWindow::setAlwaysOnTop(bool on)
{
    if (m_alwaysOnTop == on)
    {
        return;
    }

    m_alwaysOnTop = on;
    const bool wasVisible = isVisible();
    const QRect oldGeometry = geometry();
    const Qt::WindowStates oldState = windowState();

    setWindowFlag(Qt::WindowStaysOnTopHint, on);

    if (wasVisible)
    {
        setGeometry(oldGeometry);
        if (oldState.testFlag(Qt::WindowMaximized))
        {
            showMaximized();
        }
        else if (oldState.testFlag(Qt::WindowMinimized))
        {
            showMinimized();
        }
        else
        {
            show();
        }
    }

    update(titleBarRect());
    Q_EMIT alwaysOnTopChanged(m_alwaysOnTop);
}

void AntWindow::toggleAlwaysOnTop()
{
    setAlwaysOnTop(!m_alwaysOnTop);
}

int AntWindow::cornerRadius() const
{
    return m_cornerRadius;
}

void AntWindow::setCornerRadius(int radius)
{
    const int normalizedRadius = qMax(0, radius);
    if (m_cornerRadius == normalizedRadius)
    {
        return;
    }

    m_cornerRadius = normalizedRadius;
    applyNativeWindowFrame();
    update();
    Q_EMIT cornerRadiusChanged(m_cornerRadius);
}

bool AntWindow::isTitleBarButtonVisible(TitleBarButton button) const
{
    switch (button)
    {
    case TitleBarButton::Pin:
        return m_pinButtonVisible;
    case TitleBarButton::Theme:
        return m_themeButtonVisible;
    case TitleBarButton::Minimize:
        return m_minimizeButtonVisible;
    case TitleBarButton::Maximize:
        return m_maximizeButtonVisible;
    case TitleBarButton::Close:
        return m_closeButtonVisible;
    default:
        return false;
    }
}

void AntWindow::setTitleBarButtonVisible(TitleBarButton button, bool visible)
{
    bool* target = nullptr;
    switch (button)
    {
    case TitleBarButton::Pin:
        target = &m_pinButtonVisible;
        break;
    case TitleBarButton::Theme:
        target = &m_themeButtonVisible;
        break;
    case TitleBarButton::Minimize:
        target = &m_minimizeButtonVisible;
        break;
    case TitleBarButton::Maximize:
        target = &m_maximizeButtonVisible;
        break;
    case TitleBarButton::Close:
        target = &m_closeButtonVisible;
        break;
    default:
        break;
    }

    if (!target || *target == visible)
    {
        return;
    }

    *target = visible;
    if (!visible && m_hoveredButton == button)
    {
        m_hoveredButton = TitleBarButton::None;
    }

    update(titleBarRect());
    emitTitleBarButtonVisibleChanged(button, visible);
}

QRect AntWindow::titleBarButtonRect(TitleBarButton button) const
{
    if (!isTitleBarButtonVisible(button))
    {
        return {};
    }

    int right = width();
    for (TitleBarButton current : kTitleBarButtonsRightToLeft)
    {
        if (!isTitleBarButtonVisible(current))
        {
            continue;
        }

        right -= TitleBarButtonWidth;
        const QRect rect(right, 0, TitleBarButtonWidth, TitleBarHeight);
        if (current == button)
        {
            return rect;
        }
    }

    return {};
}

bool AntWindow::isPinButtonVisible() const
{
    return m_pinButtonVisible;
}

bool AntWindow::isThemeButtonVisible() const
{
    return m_themeButtonVisible;
}

bool AntWindow::isMinimizeButtonVisible() const
{
    return m_minimizeButtonVisible;
}

bool AntWindow::isMaximizeButtonVisible() const
{
    return m_maximizeButtonVisible;
}

bool AntWindow::isCloseButtonVisible() const
{
    return m_closeButtonVisible;
}

void AntWindow::setPinButtonVisible(bool visible)
{
    setTitleBarButtonVisible(TitleBarButton::Pin, visible);
}

void AntWindow::setThemeButtonVisible(bool visible)
{
    setTitleBarButtonVisible(TitleBarButton::Theme, visible);
}

void AntWindow::setMinimizeButtonVisible(bool visible)
{
    setTitleBarButtonVisible(TitleBarButton::Minimize, visible);
}

void AntWindow::setMaximizeButtonVisible(bool visible)
{
    setTitleBarButtonVisible(TitleBarButton::Maximize, visible);
}

void AntWindow::setCloseButtonVisible(bool visible)
{
    setTitleBarButtonVisible(TitleBarButton::Close, visible);
}

void AntWindow::moveToCenter()
{
    if (isMaximized() || isFullScreen())
    {
        return;
    }

    QRect available;
    if (QScreen* currentScreen = screen())
    {
        available = currentScreen->availableGeometry();
    }
    else if (QScreen* primary = QGuiApplication::primaryScreen())
    {
        available = primary->availableGeometry();
    }
    else
    {
        return;
    }

    move(available.center() - rect().center());
}

bool AntWindow::event(QEvent* event)
{
    if (event->type() == QEvent::HoverMove)
    {
        auto* hoverEvent = static_cast<QHoverEvent*>(event);
        const TitleBarButton oldHovered = m_hoveredButton;
        m_hoveredButton = buttonAtPosition(hoverEvent->position().toPoint());
        if (m_hoveredButton != oldHovered)
        {
            update();
        }

        if (m_hoveredButton != TitleBarButton::None)
        {
            setCursor(Qt::ArrowCursor);
        }
        else if (isTitleBarArea(hoverEvent->position().toPoint()))
        {
            setCursor(Qt::ArrowCursor);
        }
    }
    return QMainWindow::event(event);
}

bool AntWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_contentWidget)
    {
        auto mapMousePosition = [this](QMouseEvent* mouseEvent) {
            return m_contentWidget->mapTo(this, mouseEvent->position().toPoint());
        };

        switch (event->type())
        {
        case QEvent::MouseButtonPress:
        {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (handleTitleBarMousePress(mapMousePosition(mouseEvent),
                                         mouseEvent->globalPosition().toPoint(),
                                         mouseEvent->button()))
            {
                return true;
            }
            break;
        }
        case QEvent::MouseMove:
        {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (handleTitleBarMouseMove(mapMousePosition(mouseEvent),
                                        mouseEvent->globalPosition().toPoint(),
                                        mouseEvent->buttons()))
            {
                return true;
            }
            break;
        }
        case QEvent::MouseButtonRelease:
        {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (handleTitleBarMouseRelease(mapMousePosition(mouseEvent),
                                           mouseEvent->globalPosition().toPoint(),
                                           mouseEvent->button()))
            {
                return true;
            }
            break;
        }
        case QEvent::MouseButtonDblClick:
        {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (handleTitleBarMouseDoubleClick(mapMousePosition(mouseEvent), mouseEvent->button()))
            {
                return true;
            }
            break;
        }
        default:
            break;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void AntWindow::mousePressEvent(QMouseEvent* event)
{
    if (handleTitleBarMousePress(event->pos(), event->globalPosition().toPoint(), event->button()))
    {
        event->accept();
        return;
    }

    QMainWindow::mousePressEvent(event);
}

void AntWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (handleTitleBarMouseMove(event->pos(), event->globalPosition().toPoint(), event->buttons()))
    {
        event->accept();
        return;
    }

    QMainWindow::mouseMoveEvent(event);
}

void AntWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (handleTitleBarMouseRelease(event->pos(), event->globalPosition().toPoint(), event->button()))
    {
        event->accept();
        return;
    }

    QMainWindow::mouseReleaseEvent(event);
}

void AntWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (handleTitleBarMouseDoubleClick(event->pos(), event->button()))
    {
        event->accept();
        return;
    }

    QMainWindow::mouseDoubleClickEvent(event);
}

void AntWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        m_windowMaximized = QMainWindow::isMaximized();
        applyNativeWindowFrame();
        update();
    }
    QMainWindow::changeEvent(event);
}

void AntWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
}

void AntWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    m_windowMaximized = QMainWindow::isMaximized();
    applyNativeWindowFrame();
}

void AntWindow::paintEvent(QPaintEvent* event)
{
    QMainWindow::paintEvent(event);
}

bool AntWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
    {
        auto* msg = static_cast<MSG*>(message);
        const HWND hwnd = msg->hwnd;
        const UINT uMsg = msg->message;
        const LPARAM lParam = msg->lParam;

        switch (uMsg)
        {
        case WM_NCCALCSIZE:
        {
            if (msg->wParam == TRUE && lParam)
            {
                auto* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
                if (isMaximized() && !isFullScreen())
                {
                    const int borderWidth = resizeBorderMetric(hwnd, SM_CXSIZEFRAME, SM_CXPADDEDBORDER);
                    const int borderHeight = resizeBorderMetric(hwnd, SM_CYSIZEFRAME, SM_CXPADDEDBORDER);
                    params->rgrc[0].left += borderWidth;
                    params->rgrc[0].right -= borderWidth;
                    params->rgrc[0].top += borderHeight;
                    params->rgrc[0].bottom -= borderHeight;
                }
            }
            *result = 0;
            return true;
        }
        case WM_NCHITTEST:
        {
            const QPoint globalPos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            const QPoint widgetPos = mapFromGlobal(globalPos);
            const int clientWidth = width();
            const int clientHeight = height();

            const bool canResize = !isMaximized() && !isFullScreen() && clientWidth > 0 && clientHeight > 0;
            if (canResize)
            {
                const qreal dpr = qMax<qreal>(1.0, devicePixelRatioF());
                const int borderWidth =
                    qMax(8, qRound(static_cast<qreal>(resizeBorderMetric(hwnd, SM_CXSIZEFRAME, SM_CXPADDEDBORDER)) / dpr));
                const int borderHeight =
                    qMax(8, qRound(static_cast<qreal>(resizeBorderMetric(hwnd, SM_CYSIZEFRAME, SM_CXPADDEDBORDER)) / dpr));
                const bool left = widgetPos.x() >= 0 && widgetPos.x() < borderWidth;
                const bool right = widgetPos.x() < clientWidth && widgetPos.x() >= clientWidth - borderWidth;
                const bool top = widgetPos.y() >= 0 && widgetPos.y() < borderHeight;
                const bool bottom = widgetPos.y() < clientHeight && widgetPos.y() >= clientHeight - borderHeight;

                if (left && top)
                {
                    *result = HTTOPLEFT;
                    return true;
                }
                if (left && bottom)
                {
                    *result = HTBOTTOMLEFT;
                    return true;
                }
                if (right && top)
                {
                    *result = HTTOPRIGHT;
                    return true;
                }
                if (right && bottom)
                {
                    *result = HTBOTTOMRIGHT;
                    return true;
                }
                if (left)
                {
                    *result = HTLEFT;
                    return true;
                }
                if (right)
                {
                    *result = HTRIGHT;
                    return true;
                }
                if (top)
                {
                    *result = HTTOP;
                    return true;
                }
                if (bottom)
                {
                    *result = HTBOTTOM;
                    return true;
                }
            }

            if (isTitleBarArea(widgetPos))
            {
                const TitleBarButton button = buttonAtPosition(widgetPos);
                if (button != TitleBarButton::None)
                {
                    *result = nativeHitTestForTitleBarButton(button);
                }
                else
                {
                    *result = HTCAPTION;
                }
                return true;
            }

            *result = HTCLIENT;
            return true;
        }
        case WM_NCMOUSEMOVE:
        {
            const TitleBarButton button = titleBarButtonForNativeHitTest(msg->wParam);
            if (button != TitleBarButton::None)
            {
                m_hoveredButton = button;
                update(titleBarButtonRect(button));
                *result = ::DefWindowProcW(hwnd, uMsg, msg->wParam, lParam);
                return true;
            }
            break;
        }
        case WM_NCMOUSELEAVE:
        {
            if (m_hoveredButton == TitleBarButton::Minimize ||
                m_hoveredButton == TitleBarButton::Maximize ||
                m_hoveredButton == TitleBarButton::Close)
            {
                const TitleBarButton oldHovered = m_hoveredButton;
                m_hoveredButton = TitleBarButton::None;
                update(titleBarButtonRect(oldHovered));
            }
            break;
        }
        case WM_NCLBUTTONDOWN:
        {
            const TitleBarButton button = titleBarButtonForNativeHitTest(msg->wParam);
            if (button != TitleBarButton::None)
            {
                m_pressedButton = button;
                update(titleBarButtonRect(button));
                *result = 0;
                return true;
            }
            break;
        }
        case WM_NCLBUTTONUP:
        {
            const TitleBarButton button = titleBarButtonForNativeHitTest(msg->wParam);
            if (button != TitleBarButton::None || m_pressedButton != TitleBarButton::None)
            {
                const QPoint globalPos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                const QPoint widgetPos = mapFromGlobal(globalPos);
                const TitleBarButton pressedButton = m_pressedButton;
                m_pressedButton = TitleBarButton::None;
                if (pressedButton != TitleBarButton::None && buttonAtPosition(widgetPos) == pressedButton)
                {
                    handleButtonClicked(pressedButton);
                }
                update(titleBarRect());
                *result = 0;
                return true;
            }
            break;
        }
        case WM_GETMINMAXINFO:
        {
            auto* minmaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);
            bool hasMonitorInfo = false;
            if (HMONITOR monitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST))
            {
                MONITORINFO monitorInfo{};
                monitorInfo.cbSize = sizeof(MONITORINFO);
                if (::GetMonitorInfoW(monitor, &monitorInfo))
                {
                    const RECT& monitorArea = monitorInfo.rcMonitor;
                    const RECT& workArea = monitorInfo.rcWork;
                    minmaxInfo->ptMaxPosition.x = workArea.left - monitorArea.left;
                    minmaxInfo->ptMaxPosition.y = workArea.top - monitorArea.top;
                    minmaxInfo->ptMaxSize.x = workArea.right - workArea.left;
                    minmaxInfo->ptMaxSize.y = workArea.bottom - workArea.top;
                    hasMonitorInfo = true;
                }
            }
            if (!hasMonitorInfo)
            {
                RECT workArea{};
                ::SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
                minmaxInfo->ptMaxPosition.x = workArea.left;
                minmaxInfo->ptMaxPosition.y = workArea.top;
                minmaxInfo->ptMaxSize.x = workArea.right - workArea.left;
                minmaxInfo->ptMaxSize.y = workArea.bottom - workArea.top;
            }
            minmaxInfo->ptMinTrackSize.x = qRound(minimumWidth() * devicePixelRatioF());
            minmaxInfo->ptMinTrackSize.y = qRound(minimumHeight() * devicePixelRatioF());
            return true;
        }
        case WM_NCLBUTTONDBLCLK:
        {
            // Handle double-click on title bar to maximize/restore.
            // The system won't deliver mouseDoubleClickEvent for HTCAPTION areas.
            if (msg->wParam != HTCAPTION)
            {
                break;
            }
            if (!isMaximized())
            {
                showMaximized();
                m_windowMaximized = true;
                Q_EMIT maximizeRequested();
            }
            else
            {
                showNormal();
                m_windowMaximized = false;
                Q_EMIT restoreRequested();
            }
            return true;
        }
        default:
            break;
        }
    }
#endif

    return QMainWindow::nativeEvent(eventType, message, result);
}

bool AntWindow::isTitleBarArea(const QPoint& pos) const
{
    return pos.y() >= 0 && pos.y() < TitleBarHeight && pos.x() >= 0 && pos.x() < width();
}

bool AntWindow::isButtonArea(const QPoint& pos) const
{
    return buttonAtPosition(pos) != TitleBarButton::None;
}

AntWindow::TitleBarButton AntWindow::buttonAtPosition(const QPoint& pos) const
{
    if (!isTitleBarArea(pos))
    {
        return TitleBarButton::None;
    }

    for (TitleBarButton button : kTitleBarButtonsRightToLeft)
    {
        if (titleBarButtonRect(button).contains(pos))
        {
            return button;
        }
    }

    return TitleBarButton::None;
}

QRect AntWindow::titleBarRect() const
{
    return QRect(0, 0, width(), TitleBarHeight);
}

bool AntWindow::handleTitleBarMousePress(const QPoint& pos, const QPoint& globalPos, Qt::MouseButton button)
{
    if (button != Qt::LeftButton)
    {
        return false;
    }

    if (isButtonArea(pos))
    {
        m_pressedButton = buttonAtPosition(pos);
        update(titleBarRect());
        return true;
    }

    if (isTitleBarArea(pos))
    {
        m_dragging = true;
        m_pressedButton = TitleBarButton::None;
        m_dragStartPosition = globalPos;
        m_dragStartWindowPos = this->pos();
        m_dragStartTitleXRatio = width() > 0 ? qBound<qreal>(0.0, static_cast<qreal>(pos.x()) / width(), 1.0) : 0.5;
        m_dragStartTitleY = qBound(0, pos.y(), TitleBarHeight - 1);
        return true;
    }

    return false;
}

bool AntWindow::handleTitleBarMouseMove(const QPoint& pos, const QPoint& globalPos, Qt::MouseButtons buttons)
{
    Q_UNUSED(pos)
    if (m_dragging && (buttons & Qt::LeftButton))
    {
#ifdef Q_OS_WIN
        if (QWindow* nativeWindow = windowHandle())
        {
            if (nativeWindow->startSystemMove())
            {
                m_dragging = false;
                return true;
            }
        }
#endif

        const QPoint delta = globalPos - m_dragStartPosition;

        if (isMaximized())
        {
            showNormal();
            m_windowMaximized = false;

            const QSize restoredSize = size();
            const QPoint restoredOffset(qRound(m_dragStartTitleXRatio * restoredSize.width()), m_dragStartTitleY);
            m_dragStartWindowPos = globalPos - restoredOffset;
            m_dragStartPosition = globalPos;

            move(m_dragStartWindowPos);
        }
        else
        {
            move(m_dragStartWindowPos + delta);
        }

        return true;
    }

    return false;
}

bool AntWindow::handleTitleBarMouseRelease(const QPoint& pos, const QPoint& globalPos, Qt::MouseButton button)
{
    if (button != Qt::LeftButton)
    {
        return false;
    }

    if (m_pressedButton != TitleBarButton::None)
    {
        const TitleBarButton pressedButton = m_pressedButton;
        m_pressedButton = TitleBarButton::None;
        if (buttonAtPosition(pos) == pressedButton)
        {
            handleButtonClicked(pressedButton);
        }
        update(titleBarRect());
        return true;
    }

    if (m_dragging)
    {
        m_dragging = false;
        applyManualSnap(globalPos);
        return true;
    }

    return false;
}

bool AntWindow::handleTitleBarMouseDoubleClick(const QPoint& pos, Qt::MouseButton button)
{
    if (button == Qt::LeftButton && isTitleBarArea(pos) && !isButtonArea(pos))
    {
        if (isMaximized())
        {
            showNormal();
            m_windowMaximized = false;
            Q_EMIT restoreRequested();
        }
        else
        {
            showMaximized();
            m_windowMaximized = true;
            Q_EMIT maximizeRequested();
        }
        return true;
    }

    return false;
}

void AntWindow::handleButtonClicked(TitleBarButton button)
{
    switch (button)
    {
    case TitleBarButton::Pin:
        toggleAlwaysOnTop();
        break;
    case TitleBarButton::Theme:
        antTheme->toggleThemeMode();
        break;
    case TitleBarButton::Minimize:
        showMinimized();
        Q_EMIT minimizeRequested();
        break;
    case TitleBarButton::Maximize:
        if (isMaximized())
        {
            showNormal();
            m_windowMaximized = false;
            Q_EMIT restoreRequested();
        }
        else
        {
            showMaximized();
            m_windowMaximized = true;
            Q_EMIT maximizeRequested();
        }
        break;
    case TitleBarButton::Close:
        Q_EMIT closeRequested();
        close();
        break;
    default:
        break;
    }
}

void AntWindow::applyManualSnap(const QPoint& globalPos)
{
    if (isFullScreen())
    {
        return;
    }

    const int dragDistance = (globalPos - m_dragStartPosition).manhattanLength();
    const QStyleHints* hints = QGuiApplication::styleHints();
    const int minimumDragDistance = hints ? hints->startDragDistance() : 10;
    if (dragDistance < minimumDragDistance)
    {
        return;
    }

    QScreen* targetScreen = QGuiApplication::screenAt(globalPos);
    if (!targetScreen)
    {
        targetScreen = screen();
    }
    if (!targetScreen)
    {
        targetScreen = QGuiApplication::primaryScreen();
    }
    if (!targetScreen)
    {
        return;
    }

    const QRect availableGeometry = targetScreen->availableGeometry();
    const int snapThreshold = qMax(16, minimumDragDistance * 2);
    const bool snapTop = globalPos.y() <= availableGeometry.top() + snapThreshold;
    const bool snapLeft = globalPos.x() <= availableGeometry.left() + snapThreshold;
    const bool snapRight = globalPos.x() >= availableGeometry.right() - snapThreshold;

    if (snapTop)
    {
        if (!isMaximized())
        {
            showMaximized();
            Q_EMIT maximizeRequested();
        }
        m_windowMaximized = true;
        return;
    }

    QRect snappedGeometry;
    if (snapLeft)
    {
        snappedGeometry = QRect(availableGeometry.left(),
                                availableGeometry.top(),
                                availableGeometry.width() / 2,
                                availableGeometry.height());
    }
    else if (snapRight)
    {
        const int leftWidth = availableGeometry.width() / 2;
        snappedGeometry = QRect(availableGeometry.left() + leftWidth,
                                availableGeometry.top(),
                                availableGeometry.width() - leftWidth,
                                availableGeometry.height());
    }

    if (!snappedGeometry.isNull())
    {
        const bool wasMaximized = isMaximized();
        if (wasMaximized)
        {
            showNormal();
        }
        m_windowMaximized = false;
        setGeometry(snappedGeometry);
        if (wasMaximized)
        {
            Q_EMIT restoreRequested();
        }
    }
}

void AntWindow::applyNativeWindowFrame()
{
#ifdef Q_OS_WIN
    if (!windowHandle())
    {
        return;
    }

    const HWND hwnd = reinterpret_cast<HWND>(winId());
    if (!hwnd)
    {
        return;
    }

    ensureNativeSnapWindowStyle(hwnd);

    DwmSetWindowAttributeFn setWindowAttribute = nullptr;
    DwmExtendFrameIntoClientAreaFn extendFrame = nullptr;
    if (!resolveDwmApis(&setWindowAttribute, &extendFrame))
    {
        return;
    }

    if (extendFrame)
    {
        const DwmMargins margins{1, 1, 1, 1};
        extendFrame(hwnd, &margins);
    }

    if (!setWindowAttribute)
    {
        return;
    }

    const int cornerPreference = [this]() {
        if (isMaximized() || isFullScreen())
        {
            return kDwmCornerDoNotRound;
        }
        if (m_cornerRadius <= 0)
        {
            return kDwmCornerDoNotRound;
        }
        if (m_cornerRadius <= 6)
        {
            return kDwmCornerRoundSmall;
        }
        return kDwmCornerRound;
    }();
    setWindowAttribute(hwnd,
                       kDwmWindowCornerPreference,
                       &cornerPreference,
                       sizeof(cornerPreference));

    const auto& token = antTheme->tokens();
    const COLORREF borderColor = RGB(token.colorBorder.red(), token.colorBorder.green(), token.colorBorder.blue());
    setWindowAttribute(hwnd, kDwmBorderColor, &borderColor, sizeof(borderColor));

    const BOOL darkMode = antTheme->themeMode() == Ant::ThemeMode::Dark;
    setWindowAttribute(hwnd, kDwmUseImmersiveDarkMode, &darkMode, sizeof(darkMode));
#else
    update();
#endif
}

void AntWindow::emitTitleBarButtonVisibleChanged(TitleBarButton button, bool visible)
{
    switch (button)
    {
    case TitleBarButton::Pin:
        Q_EMIT pinButtonVisibleChanged(visible);
        break;
    case TitleBarButton::Theme:
        Q_EMIT themeButtonVisibleChanged(visible);
        break;
    case TitleBarButton::Minimize:
        Q_EMIT minimizeButtonVisibleChanged(visible);
        break;
    case TitleBarButton::Maximize:
        Q_EMIT maximizeButtonVisibleChanged(visible);
        break;
    case TitleBarButton::Close:
        Q_EMIT closeButtonVisibleChanged(visible);
        break;
    default:
        return;
    }

    Q_EMIT titleBarButtonVisibilityChanged(button, visible);
}

void AntWindow::syncTheme()
{
    const auto& token = antTheme->tokens();
    QPalette pal = palette();
    pal.setColor(QPalette::Window, token.colorBgContainer);
    pal.setColor(QPalette::Base, token.colorBgContainer);
    pal.setColor(QPalette::WindowText, token.colorText);
    pal.setColor(QPalette::Text, token.colorText);
    setPalette(pal);

    if (m_contentWidget)
    {
        m_contentWidget->setPalette(pal);
        if (QLayout* layout = m_contentWidget->layout())
        {
            for (int i = 0; i < layout->count(); ++i)
            {
                if (QWidget* widget = layout->itemAt(i)->widget())
                {
                    applyContentPalette(widget);
                }
            }
        }
    }

    applyNativeWindowFrame();
}

void AntWindow::applyContentPalette(QWidget* widget)
{
    if (!widget)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    QPalette pal = widget->palette();
    pal.setColor(QPalette::Window, token.colorBgContainer);
    pal.setColor(QPalette::Base, token.colorBgContainer);
    pal.setColor(QPalette::WindowText, token.colorText);
    pal.setColor(QPalette::Text, token.colorText);
    widget->setPalette(pal);
}
