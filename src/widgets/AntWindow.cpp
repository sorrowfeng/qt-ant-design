#include "AntWindow.h"

#include <QCloseEvent>
#include <QCoreApplication>
#include <QCursor>
#include <QElapsedTimer>
#include <QEvent>
#include <QGuiApplication>
#include <QHideEvent>
#include <QHoverEvent>
#include <QLayout>
#include <QLineF>
#include <QMargins>
#include <QMetaType>
#include <QMouseEvent>
#include <QMoveEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPointer>
#include <QRegion>
#include <QResizeEvent>
#include <QScreen>
#include <QShowEvent>
#include <QStyleHints>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>

#include <functional>
#include <utility>

#include "../styles/AntWindowStyle.h"
#include "AntModal.h"
#include "AntRibbon.h"
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

constexpr auto kThemeTransitionOverlayName = "AntWindowThemeTransitionOverlay";
constexpr int kThemeTransitionFrameIntervalMs = 16;
constexpr int kThemeTransitionDurationMs = 220;
constexpr int kThemeTransitionEdgeFeather = 16;
constexpr auto kLegacySoftwareShadowObjectName = "AntWindowLegacySoftwareShadow";
constexpr int kLegacySoftwareShadowMargin = 14;
constexpr int kLegacySoftwareShadowInnerClearance = 0;

int titleBarButtonIndex(AntWindow::TitleBarButton button)
{
    return static_cast<int>(button);
}

QPixmap captureAntWindowFrame(QWidget* widget)
{
    if (!widget || widget->width() <= 0 || widget->height() <= 0)
    {
        return {};
    }

    const qreal dpr = qMax<qreal>(1.0, widget->devicePixelRatioF());
    const QSize imageSize(qMax(1, qRound(widget->width() * dpr)),
                          qMax(1, qRound(widget->height() * dpr)));
    QPixmap frame(imageSize);
    frame.setDevicePixelRatio(dpr);
    frame.fill(Qt::transparent);

    QPainter painter(&frame);
    widget->render(&painter, QPoint(), QRegion(), QWidget::DrawWindowBackground | QWidget::DrawChildren);
    return frame;
}

void activateAntWindowLayoutTree(QWidget* widget)
{
    if (!widget)
    {
        return;
    }

    widget->ensurePolished();
    const auto childWidgets = widget->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (auto* child : childWidgets)
    {
        activateAntWindowLayoutTree(child);
    }

    if (auto* layout = widget->layout())
    {
        layout->invalidate();
        layout->activate();
    }
}

#if defined(Q_OS_WIN)
bool makeAntWindowNativeInputTransparent(QWidget* widget,
                                         const char* propertyName,
                                         bool forceNativeHandle = true,
                                         bool transparentWhenNoNativeHandle = false)
{
    if (!widget)
    {
        return false;
    }

    const WId nativeId = forceNativeHandle ? widget->winId() : widget->internalWinId();
    const HWND hwnd = reinterpret_cast<HWND>(nativeId);
    if (!hwnd)
    {
        if (propertyName)
        {
            widget->setProperty(propertyName, transparentWhenNoNativeHandle);
        }
        return false;
    }

    const LONG_PTR currentStyle = ::GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    const LONG_PTR clickThroughStyle = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE;
    const LONG_PTR nextStyle = currentStyle | clickThroughStyle;
    if (nextStyle != currentStyle)
    {
        ::SetWindowLongPtrW(hwnd, GWL_EXSTYLE, nextStyle);
        ::SetWindowPos(hwnd,
                       nullptr,
                       0,
                       0,
                       0,
                       0,
                       SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER |
                           SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }

    if (propertyName)
    {
        widget->setProperty(propertyName, true);
    }
    return true;
}

void makeAntWindowShadowClickThrough(QWidget* widget)
{
    makeAntWindowNativeInputTransparent(widget, "antWindowLegacySoftwareShadowClickThrough");
}
#endif

class AntWindowThemeTransitionOverlay : public QWidget
{
public:
    enum class Mode
    {
        CircularReveal,
        CrossFade,
    };

    AntWindowThemeTransitionOverlay(QWidget* parent, const QPixmap& oldFrame, const QPoint& origin, Mode mode)
        : QWidget(parent)
        , m_oldFrame(oldFrame)
        , m_origin(origin)
        , m_mode(mode)
    {
        setObjectName(QString::fromLatin1(kThemeTransitionOverlayName));
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_TranslucentBackground, m_mode == Mode::CircularReveal);
        setAttribute(Qt::WA_OpaquePaintEvent, m_mode == Mode::CrossFade);
        setAutoFillBackground(false);
        setFocusPolicy(Qt::NoFocus);
        m_timer.setInterval(kThemeTransitionFrameIntervalMs);
        m_timer.setTimerType(Qt::PreciseTimer);
        setProperty("transitionFrameIntervalMs", m_timer.interval());
        setProperty("transitionDurationMs", kThemeTransitionDurationMs);
        setProperty("transitionMotionCurve", QStringLiteral("smootherstep"));
        setProperty("transitionEdgeFeather", kThemeTransitionEdgeFeather);
        setProperty("transitionDrawsCapturedNewFrame", true);
        setProperty("transitionCaptureMethod", QStringLiteral("render"));
        setProperty("transitionUsesEventLoopCapture", false);
        setProperty("transitionMode",
                    m_mode == Mode::CrossFade ? QStringLiteral("crossfade") : QStringLiteral("circular-reveal"));
        connect(&m_timer, &QTimer::timeout, this, [this]() {
            advanceAnimation();
        });
    }

    void setNewFrame(const QPixmap& newFrame)
    {
        m_newFrame = newFrame;
        update();
    }

    void startTransition(int durationMs, std::function<void()> finished)
    {
        m_durationMs = qMax(1, durationMs);
        m_finished = std::move(finished);
        m_progress = 0.0;
        m_elapsed.restart();
        update();
        m_timer.start();
    }

    qreal progress() const
    {
        return m_progress;
    }

private:
    void setProgress(qreal progress)
    {
        const qreal normalized = qBound<qreal>(0.0, progress, 1.0);
        if (qFuzzyCompare(m_progress, normalized))
        {
            return;
        }
        m_progress = normalized;
        update();
    }

    void advanceAnimation()
    {
        const qreal linearProgress =
            qBound<qreal>(0.0, static_cast<qreal>(m_elapsed.elapsed()) / static_cast<qreal>(m_durationMs), 1.0);
        setProgress(smootherStep(linearProgress));
        if (linearProgress >= 1.0)
        {
            m_timer.stop();
            if (m_finished)
            {
                auto finished = std::move(m_finished);
                finished();
            }
        }
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        if (m_oldFrame.isNull())
        {
            return;
        }

        const QRectF bounds(rect());
        if (bounds.isEmpty())
        {
            return;
        }

        const qreal radius = transitionRadius();
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        drawFrame(&painter, m_oldFrame, 1.0);

        if (m_newFrame.isNull() || radius <= 0.0)
        {
            return;
        }

        if (m_mode == Mode::CrossFade)
        {
            drawFrame(&painter, m_newFrame, m_progress);
            return;
        }

        drawRevealedNewFrame(&painter, radius);
    }

    qreal transitionRadius() const
    {
        const QRectF bounds(rect());
        const QPointF origin(m_origin);
        qreal radius = 0.0;
        for (const QPointF corner : {bounds.topLeft(), bounds.topRight(), bounds.bottomLeft(), bounds.bottomRight()})
        {
            radius = qMax(radius, QLineF(origin, corner).length());
        }
        return (radius + kThemeTransitionEdgeFeather) * m_progress;
    }

    static qreal smootherStep(qreal value)
    {
        const qreal t = qBound<qreal>(0.0, value, 1.0);
        return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    }

    void drawFrame(QPainter* painter, const QPixmap& frame, qreal opacity)
    {
        painter->save();
        painter->setOpacity(opacity);
        painter->drawPixmap(QRectF(rect()), frame, QRectF(frame.rect()));
        painter->restore();
    }

    void drawRevealedNewFrame(QPainter* painter, qreal radius)
    {
        const qreal feather = qMin<qreal>(kThemeTransitionEdgeFeather, qMax<qreal>(1.0, radius));
        const qreal solidRadius = qMax<qreal>(0.0, radius - feather);
        const int featherSteps = qMax(1, qRound(feather));

        painter->save();
        QPainterPath solidPath;
        solidPath.addEllipse(QPointF(m_origin), solidRadius, solidRadius);
        painter->setClipPath(solidPath);
        drawFrame(painter, m_newFrame, 1.0);
        painter->restore();

        for (int step = featherSteps; step >= 1; --step)
        {
            const qreal outerRadius = solidRadius + step;
            const qreal innerRadius = qMax<qreal>(0.0, outerRadius - 1.0);
            QPainterPath ring;
            ring.addEllipse(QPointF(m_origin), outerRadius, outerRadius);
            QPainterPath inner;
            inner.addEllipse(QPointF(m_origin), innerRadius, innerRadius);
            ring = ring.subtracted(inner);

            const qreal alpha = qBound<qreal>(
                0.0,
                static_cast<qreal>(featherSteps - step + 1) / static_cast<qreal>(featherSteps + 1),
                1.0);
            painter->save();
            painter->setClipPath(ring);
            drawFrame(painter, m_newFrame, alpha * 0.9);
            painter->restore();
        }
    }

    QPixmap m_oldFrame;
    QPixmap m_newFrame;
    QPoint m_origin;
    Mode m_mode = Mode::CircularReveal;
    QTimer m_timer;
    QElapsedTimer m_elapsed;
    std::function<void()> m_finished;
    int m_durationMs = kThemeTransitionDurationMs;
    qreal m_progress = 0.0;
};

// In-process child overlay that softens the visible window outline by applying
// an anti-aliased rounded-shape composition pass over the parent's backing
// store after every other child has finished painting. Combined with
// `WA_TranslucentBackground` on the host window, this lets us keep the
// existing 1-bit `setMask` (required by tests + for clipping unaware child
// widgets) while presenting smooth corner pixels on Win10 where DWM does not
// expose a rounded-corner API.
class AntWindowCornerSmoother : public QWidget
{
public:
    explicit AntWindowCornerSmoother(QWidget* parent)
        : QWidget(parent)
    {
        setObjectName(QStringLiteral("AntWindowCornerSmoother"));
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_TranslucentBackground, true);
        setFocusPolicy(Qt::NoFocus);
#if defined(Q_OS_WIN)
        setProperty("antWindowCornerSmootherClickThrough", false);
        setProperty("antWindowCornerSmootherNativeHwnd", false);
#endif
    }

    void setCornerRadius(int radius)
    {
        const int normalized = qMax(0, radius);
        if (m_radius == normalized)
        {
            return;
        }
        m_radius = normalized;
        update();
    }

    int cornerRadius() const
    {
        return m_radius;
    }

protected:
#if defined(Q_OS_WIN)
    void showEvent(QShowEvent* event) override
    {
        QWidget::showEvent(event);
        const bool hasNativeHwnd =
            makeAntWindowNativeInputTransparent(this,
                                                "antWindowCornerSmootherClickThrough",
                                                false,
                                                true);
        setProperty("antWindowCornerSmootherNativeHwnd", hasNativeHwnd);
    }

    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override
    {
        if ((eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") && message)
        {
            auto* msg = static_cast<MSG*>(message);
            if (msg->message == WM_NCHITTEST)
            {
                *result = HTTRANSPARENT;
                setProperty("antWindowCornerSmootherClickThrough", true);
                return true;
            }
        }
        return QWidget::nativeEvent(eventType, message, result);
    }
#endif

    void paintEvent(QPaintEvent*) override
    {
        if (m_radius <= 0 || rect().isEmpty())
        {
            return;
        }

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.drawRoundedRect(QRectF(rect()), m_radius, m_radius);
    }

private:
    int m_radius = 0;
};

class AntWindowLegacySoftwareShadow : public QWidget
{
public:
    explicit AntWindowLegacySoftwareShadow(QWidget* owner)
        : QWidget(owner, Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowDoesNotAcceptFocus)
    {
        setObjectName(QString::fromLatin1(kLegacySoftwareShadowObjectName));
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setAttribute(Qt::WA_ShowWithoutActivating, true);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        setWindowFlag(Qt::WindowTransparentForInput, true);
#endif
        setFocusPolicy(Qt::NoFocus);
        setProperty("shadowMargin", kLegacySoftwareShadowMargin);
        setProperty("shadowInnerClearance", kLegacySoftwareShadowInnerClearance);
#if defined(Q_OS_WIN)
        setProperty("antWindowLegacySoftwareShadowClickThrough", false);
#endif
    }

    void setCornerRadius(int radius)
    {
        m_cornerRadius = qMax(0, radius);
        update();
    }

protected:
#if defined(Q_OS_WIN)
    void showEvent(QShowEvent* event) override
    {
        QWidget::showEvent(event);
        makeAntWindowShadowClickThrough(this);
    }

    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override
    {
        if ((eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") && message)
        {
            auto* msg = static_cast<MSG*>(message);
            if (msg->message == WM_NCHITTEST)
            {
                *result = HTTRANSPARENT;
                setProperty("antWindowLegacySoftwareShadowClickThrough", true);
                return true;
            }
        }
        return QWidget::nativeEvent(eventType, message, result);
    }
#endif

    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(rect(), Qt::transparent);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const int shadowWidth = kLegacySoftwareShadowMargin;
        const QRectF panelRect = QRectF(rect()).adjusted(shadowWidth,
                                                        shadowWidth,
                                                        -shadowWidth,
                                                        -shadowWidth);
        if (panelRect.isEmpty())
        {
            return;
        }

        QColor shadowBase = antTheme->tokens().colorShadow;
        const qreal maxOpacity = antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.046 : 0.032;
        const qreal effectiveSpread = qMax<qreal>(1.0, shadowWidth - kLegacySoftwareShadowInnerClearance);
        for (int distance = shadowWidth; distance > kLegacySoftwareShadowInnerClearance; --distance)
        {
            const qreal t = qBound<qreal>(0.0,
                                          1.0 - static_cast<qreal>(distance - kLegacySoftwareShadowInnerClearance - 1) / effectiveSpread,
                                          1.0);
            const qreal eased = t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
            const qreal opacity = qBound<qreal>(0.0, maxOpacity * eased, 0.22);
            if (opacity <= 0.0)
            {
                continue;
            }

            QColor shadow = shadowBase;
            shadow.setAlphaF(opacity);

            const qreal radiusGrowth = static_cast<qreal>(distance) * 0.55;
            QPainterPath outerPath;
            const QRectF outer = panelRect.adjusted(-distance, -distance, distance, distance);
            outerPath.addRoundedRect(outer, m_cornerRadius + radiusGrowth, m_cornerRadius + radiusGrowth);

            QPainterPath innerPath;
            const int innerDistance = qMax(kLegacySoftwareShadowInnerClearance, distance - 1);
            const qreal innerRadiusGrowth = static_cast<qreal>(innerDistance) * 0.55;
            const QRectF inner = panelRect.adjusted(-innerDistance, -innerDistance, innerDistance, innerDistance);
            innerPath.addRoundedRect(inner, m_cornerRadius + innerRadiusGrowth, m_cornerRadius + innerRadiusGrowth);

            painter.fillPath(outerPath.subtracted(innerPath), shadow);
        }
    }

private:
    int m_cornerRadius = 8;
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
using RtlGetVersionFn = LONG(WINAPI*)(OSVERSIONINFOW*);

constexpr DWORD kDwmUseImmersiveDarkMode = 20;
constexpr DWORD kDwmWindowCornerPreference = 33;
constexpr DWORD kDwmBorderColor = 34;
// DWM border-color sentinel meaning "no border color" (DWMWA_COLOR_NONE).
// Used to suppress the gray Win10 native resize-frame outline that DWM
// draws around `WS_THICKFRAME` windows when the AntWindow takes the
// no-translucent-background opaque path.
constexpr COLORREF kDwmBorderColorNone = 0xFFFFFFFEu;
constexpr int kDwmCornerDoNotRound = 1;
constexpr int kDwmCornerRound = 2;
constexpr int kDwmCornerRoundSmall = 3;
constexpr int kLegacyRoundedMaskFrameInset = 1;
constexpr auto kForceLegacyFramePolicyProperty = "antWindowForceLegacyFramePolicy";
constexpr auto kUsesNativeCaptionFrameProperty = "antWindowUsesNativeCaptionFrame";
constexpr auto kDwmFrameMarginsProperty = "antWindowDwmFrameMargins";
constexpr auto kDwmFrameApplyCountProperty = "antWindowDwmFrameApplyCount";
constexpr auto kDwmFrameLastReasonProperty = "antWindowDwmFrameLastReason";
constexpr auto kLegacyRoundedMaskAppliedProperty = "antWindowLegacyRoundedMaskApplied";
constexpr auto kLegacyRoundedMaskFrameInsetProperty = "antWindowLegacyRoundedMaskFrameInset";
constexpr auto kLegacyClassDropShadowEnabledProperty = "antWindowLegacyClassDropShadowEnabled";
constexpr auto kDwmFrameRefreshQueuedProperty = "antWindowDwmFrameRefreshQueued";

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

int windowsBuildNumber()
{
    static const int buildNumber = []() {
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
    }();
    return buildNumber;
}

bool supportsNativeCaptionSnapLayouts()
{
    constexpr int kWindows11Build = 22000;
    return windowsBuildNumber() >= kWindows11Build;
}

bool usesNativeCaptionFrameForWidget(const QWidget* widget)
{
    const bool forceLegacyFrame = widget && widget->property(kForceLegacyFramePolicyProperty).toBool();
    return supportsNativeCaptionSnapLayouts() && !forceLegacyFrame;
}

DwmMargins shadowPreservingDwmMargins(bool useNativeCaption)
{
    return useNativeCaption ? DwmMargins{1, 1, 1, 1} : DwmMargins{0, 0, 0, 0};
}

void setNativeTopMost(HWND hwnd, bool topMost)
{
    if (!hwnd)
    {
        return;
    }

    ::SetWindowPos(hwnd,
                   topMost ? HWND_TOPMOST : HWND_NOTOPMOST,
                   0,
                   0,
                   0,
                   0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}

QPoint nativeMessageLocalPoint(HWND hwnd, LPARAM messagePos, qreal devicePixelRatio)
{
    POINT nativePoint{GET_X_LPARAM(messagePos), GET_Y_LPARAM(messagePos)};
    ::ScreenToClient(hwnd, &nativePoint);

    const qreal dpr = qMax<qreal>(1.0, devicePixelRatio);
    return QPoint(qRound(static_cast<qreal>(nativePoint.x) / dpr),
                  qRound(static_cast<qreal>(nativePoint.y) / dpr));
}

void requestNonClientHoverTracking(HWND hwnd)
{
    TRACKMOUSEEVENT trackEvent{};
    trackEvent.cbSize = sizeof(trackEvent);
    trackEvent.dwFlags = TME_HOVER | TME_LEAVE | TME_NONCLIENT;
    trackEvent.hwndTrack = hwnd;
    trackEvent.dwHoverTime = HOVER_DEFAULT;
    ::TrackMouseEvent(&trackEvent);
}

qintptr nativeHitTestForTitleBarButton(AntWindow::TitleBarButton button)
{
    switch (button)
    {
    case AntWindow::TitleBarButton::Minimize:
        return HTMINBUTTON;
    case AntWindow::TitleBarButton::Maximize:
        return HTMAXBUTTON;
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
    case HTMINBUTTON:
        return AntWindow::TitleBarButton::Minimize;
    case HTMAXBUTTON:
        return AntWindow::TitleBarButton::Maximize;
    case HTCLOSE:
        return AntWindow::TitleBarButton::Close;
    default:
        return AntWindow::TitleBarButton::None;
    }
}

bool isNativeResizeHitTest(WPARAM hitTestCode)
{
    switch (hitTestCode)
    {
    case HTLEFT:
    case HTRIGHT:
    case HTTOP:
    case HTBOTTOM:
    case HTTOPLEFT:
    case HTTOPRIGHT:
    case HTBOTTOMLEFT:
    case HTBOTTOMRIGHT:
        return true;
    default:
        return false;
    }
}

bool resolveDwmApis(DwmSetWindowAttributeFn* setWindowAttribute, DwmExtendFrameIntoClientAreaFn* extendFrame)
{
    HMODULE dwmapi = ::GetModuleHandleW(L"dwmapi.dll");
    if (!dwmapi)
    {
        dwmapi = ::LoadLibraryW(L"dwmapi.dll");
    }
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

bool applyDwmFrameMargins(QWidget* widget,
                          HWND hwnd,
                          bool useNativeCaption,
                          DwmExtendFrameIntoClientAreaFn extendFrame,
                          const char* reason)
{
    if (!widget || !hwnd || !extendFrame)
    {
        return false;
    }

    const DwmMargins margins = shadowPreservingDwmMargins(useNativeCaption);
    widget->setProperty(kDwmFrameMarginsProperty,
                        QVariant::fromValue(QMargins(margins.cxLeftWidth,
                                                     margins.cyTopHeight,
                                                     margins.cxRightWidth,
                                                     margins.cyBottomHeight)));
    if (FAILED(extendFrame(hwnd, &margins)))
    {
        return false;
    }

    widget->setProperty(kDwmFrameApplyCountProperty, widget->property(kDwmFrameApplyCountProperty).toInt() + 1);
    widget->setProperty(kDwmFrameLastReasonProperty, QString::fromLatin1(reason ? reason : ""));
    return true;
}

bool reapplyDwmFrameMargins(QWidget* widget, HWND hwnd, bool useNativeCaption, const char* reason)
{
    DwmExtendFrameIntoClientAreaFn extendFrame = nullptr;
    if (!resolveDwmApis(nullptr, &extendFrame) || !extendFrame)
    {
        return false;
    }

    return applyDwmFrameMargins(widget, hwnd, useNativeCaption, extendFrame, reason);
}

void queueLegacyDwmFrameRefresh(QWidget* widget, const char* reason)
{
    if (!widget || widget->property(kDwmFrameRefreshQueuedProperty).toBool())
    {
        return;
    }

    widget->setProperty(kDwmFrameRefreshQueuedProperty, true);
    QPointer<QWidget> guard(widget);
    QTimer::singleShot(0, widget, [guard, reason]() {
        if (!guard)
        {
            return;
        }

        guard->setProperty(kDwmFrameRefreshQueuedProperty, false);
        if (!guard->isVisible() || usesNativeCaptionFrameForWidget(guard.data()) || guard->isMaximized() || guard->isFullScreen())
        {
            return;
        }

        const HWND hwnd = reinterpret_cast<HWND>(guard->winId());
        reapplyDwmFrameMargins(guard.data(), hwnd, false, reason);
    });
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

void ensureNativeWindowStyle(HWND hwnd, bool useNativeCaption)
{
    LONG_PTR style = ::GetWindowLongPtrW(hwnd, GWL_STYLE);
    const LONG_PTR baseStyle = WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU;
    LONG_PTR newStyle = style | baseStyle;
    if (useNativeCaption)
    {
        newStyle |= WS_CAPTION;
    }
    else
    {
        newStyle &= ~WS_CAPTION;
    }
    if (newStyle == style)
    {
        return;
    }

    ::SetWindowLongPtrW(hwnd, GWL_STYLE, newStyle);
    triggerFrameChange(hwnd);
}

void applyLegacyClassDropShadow(QWidget* widget, HWND hwnd, bool useNativeCaption)
{
    if (!widget)
    {
        return;
    }

    if (!hwnd)
    {
        widget->setProperty(kLegacyClassDropShadowEnabledProperty, false);
        return;
    }

    const LONG_PTR classStyle = ::GetClassLongPtrW(hwnd, GCL_STYLE);
    const LONG_PTR newClassStyle = useNativeCaption ? classStyle : (classStyle & ~static_cast<LONG_PTR>(CS_DROPSHADOW));
    if (newClassStyle != classStyle)
    {
        ::SetClassLongPtrW(hwnd, GCL_STYLE, newClassStyle);
    }
    widget->setProperty(kLegacyClassDropShadowEnabledProperty,
                        (::GetClassLongPtrW(hwnd, GCL_STYLE) & CS_DROPSHADOW) == CS_DROPSHADOW);
}

void applyLegacyRoundedMask(QWidget* widget, int radius)
{
    if (!widget)
    {
        return;
    }

    // AntWindow now relies on WA_TranslucentBackground + alpha-painted corners
    // (from AntWindowStyle::drawWindow's rounded clip + AntWindowCornerSmoother)
    // for the rounded outline. Applying setMask on top is redundant AND causes
    // visible flicker on shrink: the mask update is asynchronous w.r.t. the
    // WM_SIZE-triggered backing store paint, so the compositor briefly shows
    // stale pixels along the edge that the new (smaller) mask should have
    // clipped. Skipping setMask entirely keeps the corner pixels coming from
    // the alpha-aware backing store, which Qt resizes synchronously with the
    // paint event.
    Q_UNUSED(radius)
    widget->setProperty(kLegacyRoundedMaskFrameInsetProperty, kLegacyRoundedMaskFrameInset);
    widget->setProperty(kLegacyRoundedMaskAppliedProperty, false);
    widget->clearMask();
}

bool shouldForwardChildHitTestToAntWindow(AntWindow* owner, LPARAM messagePos)
{
    if (!owner || !owner->windowHandle())
    {
        return false;
    }

    const HWND ownerHwnd = reinterpret_cast<HWND>(owner->winId());
    if (!ownerHwnd)
    {
        return false;
    }

    const QPoint ownerPos = nativeMessageLocalPoint(ownerHwnd, messagePos, owner->devicePixelRatioF());
    if (ownerPos.y() >= 0 && ownerPos.y() < AntWindow::TitleBarHeight &&
        ownerPos.x() >= 0 && ownerPos.x() < owner->width())
    {
        return true;
    }

    const int clientWidth = owner->width();
    const int clientHeight = owner->height();
    if (owner->isMaximized() || owner->isFullScreen() || clientWidth <= 0 || clientHeight <= 0)
    {
        return false;
    }

    const qreal dpr = qMax<qreal>(1.0, owner->devicePixelRatioF());
    const int borderWidth =
        qMax(8, qRound(static_cast<qreal>(resizeBorderMetric(ownerHwnd, SM_CXSIZEFRAME, SM_CXPADDEDBORDER)) / dpr));
    const int borderHeight =
        qMax(8, qRound(static_cast<qreal>(resizeBorderMetric(ownerHwnd, SM_CYSIZEFRAME, SM_CXPADDEDBORDER)) / dpr));
    const bool inHorizontalResizeBand =
        ownerPos.x() >= -borderWidth && ownerPos.x() < clientWidth + borderWidth;
    const bool inVerticalResizeBand =
        ownerPos.y() >= -borderHeight && ownerPos.y() < clientHeight + borderHeight;
    const bool left = inVerticalResizeBand &&
                      ownerPos.x() >= -borderWidth && ownerPos.x() < borderWidth;
    const bool right = inVerticalResizeBand &&
                       ownerPos.x() < clientWidth + borderWidth &&
                       ownerPos.x() >= clientWidth - borderWidth;
    const bool top = inHorizontalResizeBand &&
                     ownerPos.y() >= -borderHeight && ownerPos.y() < borderHeight;
    const bool bottom = inHorizontalResizeBand &&
                        ownerPos.y() < clientHeight + borderHeight &&
                        ownerPos.y() >= clientHeight - borderHeight;
    return left || right || top || bottom;
}

class AntWindowContentWidget : public QWidget
{
public:
    explicit AntWindowContentWidget(AntWindow* owner)
        : QWidget(owner)
        , m_owner(owner)
    {
        setObjectName(QStringLiteral("AntWindowContentWidget"));
    }

protected:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override
    {
        if ((eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") && message)
        {
            auto* msg = static_cast<MSG*>(message);
            if (msg->message == WM_NCHITTEST && shouldForwardChildHitTestToAntWindow(m_owner, msg->lParam))
            {
                if (m_owner)
                {
                    m_owner->setProperty("antWindowChildHitTestForwarded",
                                         m_owner->property("antWindowChildHitTestForwarded").toInt() + 1);
                }
                *result = HTTRANSPARENT;
                return true;
            }
        }
        return QWidget::nativeEvent(eventType, message, result);
    }

private:
    AntWindow* m_owner = nullptr;
};
#endif
}

AntWindow::AntWindow(QWidget* parent)
    : QMainWindow(parent)
{
    qRegisterMetaType<AntWindow::TitleBarButton>("AntWindow::TitleBarButton");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground, false);
#ifdef Q_OS_WIN
    // Required so the AA fragments produced by the corner-smoother overlay
    // survive in the backing store and reach the compositor as smooth corner
    // alpha. Without this, the 1-bit `setMask` would clip the painted
    // anti-aliased fragments and the corners would render jagged on Win10.
    //
    // EXPERIMENT (issue 40 follow-up): on the Win10 legacy frame path the
    // combination of `WA_TranslucentBackground=true`, a `CompositionMode_
    // DestinationIn` corner-smoother child, and repeated DWM frame margin
    // re-extension across resize cycles drives DWM compositor state into
    // failure modes that surface as "stutter after dock float/embed" and
    // "black screen after repeated resizes". Disable the translucent +
    // smoother path on Win10 and accept square corners as a visual
    // compromise in exchange for stable compositor behavior. Win11 keeps
    // the AA-corner code path because it does not rely on this same DWM
    // glass interaction (Snap Layout caption + native rounded corners).
    m_useTranslucentBackground = supportsNativeCaptionSnapLayouts();
    if (m_useTranslucentBackground)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
    }
#else
    setAttribute(Qt::WA_TranslucentBackground, true);
    m_useTranslucentBackground = true;
#endif
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    installAntStyle<AntWindowStyle>(this);
    setMinimumSize(400, 300);
    syncTheme();
    syncTitleBarPerfCounters();

#ifdef Q_OS_WIN
    m_contentWidget = new AntWindowContentWidget(this);
#else
    m_contentWidget = new QWidget(this);
#endif
    m_contentWidget->setAttribute(Qt::WA_Hover, true);
    m_contentWidget->setMouseTracking(true);
    m_contentWidget->installEventFilter(this);
    auto* contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(0, TitleBarHeight, 0, 0);
    contentLayout->setSpacing(0);
    QMainWindow::setCentralWidget(m_contentWidget);

#ifdef Q_OS_WIN
    if (QCoreApplication* app = QCoreApplication::instance())
    {
        app->installNativeEventFilter(this);
    }
#endif

    if (m_useTranslucentBackground)
    {
        auto* smoother = new AntWindowCornerSmoother(this);
        smoother->setCornerRadius(m_cornerRadius);
        smoother->raise();
        m_cornerSmoother = smoother;
    }

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        syncTheme();
        update();
    });
}

AntWindow::~AntWindow()
{
#ifdef Q_OS_WIN
    if (QCoreApplication* app = QCoreApplication::instance())
    {
        app->removeNativeEventFilter(this);
    }
#endif
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

    auto* layout = qobject_cast<QVBoxLayout*>(m_contentWidget->layout());
    if (!layout)
    {
        return;
    }
    if (m_centralContentWidget)
    {
        layout->removeWidget(m_centralContentWidget);
        m_centralContentWidget->setParent(nullptr);
        m_centralContentWidget = nullptr;
    }

    if (widget)
    {
        applyContentPalette(widget);
        m_centralContentWidget = widget;
        layout->addWidget(widget, 1);
    }

    if (m_cornerSmoother)
    {
        m_cornerSmoother->raise();
    }
}

void AntWindow::setRibbon(AntRibbon* ribbon)
{
    if (!m_contentWidget || m_ribbon == ribbon)
    {
        return;
    }

    auto* layout = qobject_cast<QVBoxLayout*>(m_contentWidget->layout());
    if (!layout)
    {
        return;
    }
    if (m_ribbon)
    {
        layout->removeWidget(m_ribbon);
        m_ribbon->setParent(nullptr);
    }

    m_ribbon = ribbon;
    if (m_ribbon)
    {
        applyContentPalette(m_ribbon);
        m_ribbon->setParent(m_contentWidget);
        layout->insertWidget(0, m_ribbon);
        m_ribbon->setVisible(m_ribbonVisible);
    }
    if (m_cornerSmoother)
    {
        m_cornerSmoother->raise();
    }
    updateGeometry();
    update();
}

AntRibbon* AntWindow::ribbon() const
{
    return m_ribbon;
}

void AntWindow::setRibbonVisible(bool visible)
{
    if (m_ribbonVisible == visible)
    {
        return;
    }

    m_ribbonVisible = visible;
    if (m_ribbon)
    {
        m_ribbon->setVisible(m_ribbonVisible);
    }
    updateGeometry();
    update();
}

bool AntWindow::isRibbonVisible() const
{
    return m_ribbonVisible;
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
#ifdef Q_OS_WIN
    if (isVisible() && windowHandle())
    {
        Qt::WindowFlags flags = windowFlags();
        if (on)
        {
            flags |= Qt::WindowStaysOnTopHint;
        }
        else
        {
            flags &= ~Qt::WindowStaysOnTopHint;
        }
        overrideWindowFlags(flags);
        setNativeTopMost(reinterpret_cast<HWND>(winId()), on);
        updateLegacySoftwareShadow();
        update(titleBarRect());
        Q_EMIT alwaysOnTopChanged(m_alwaysOnTop);
        return;
    }
#endif

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

bool AntWindow::usesLegacyOpaquePath() const
{
    // m_useTranslucentBackground is decided at construction time based on
    // whether the build supports Win11 caption / Snap Layout. Anything else
    // (Win10, forced legacy) takes the opaque path.
    return !m_useTranslucentBackground;
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
    updateCornerSmoother();
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

    const QRect oldButtonStrip = titleBarButtonStripRect();
    *target = visible;
    if (!visible && m_hoveredButton == button)
    {
        m_hoveredButton = TitleBarButton::None;
    }
    if (!visible && m_pressedButton == button)
    {
        m_pressedButton = TitleBarButton::None;
    }

    invalidateTitleBarButtonRectCache();
    updateTitleBarRegion(oldButtonStrip.united(titleBarButtonStripRect()));
    emitTitleBarButtonVisibleChanged(button, visible);
}

QRect AntWindow::titleBarButtonRect(TitleBarButton button) const
{
    if (button == TitleBarButton::None || !isTitleBarButtonVisible(button))
    {
        return {};
    }

    ensureTitleBarButtonRectCache();
    return m_titleBarButtonRectCache[titleBarButtonIndex(button)];
}

AntWindow::TitleBarButton AntWindow::hoveredTitleBarButton() const
{
    return m_hoveredButton;
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

bool AntWindow::isCloseConfirmationEnabled() const
{
    return m_closeConfirmationEnabled;
}

QString AntWindow::closeConfirmationTitle() const
{
    return m_closeConfirmationTitle;
}

QString AntWindow::closeConfirmationContent() const
{
    return m_closeConfirmationContent;
}

QString AntWindow::closeConfirmationOkText() const
{
    return m_closeConfirmationOkText;
}

QString AntWindow::closeConfirmationCancelText() const
{
    return m_closeConfirmationCancelText;
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

void AntWindow::setCloseConfirmationEnabled(bool enabled)
{
    if (m_closeConfirmationEnabled == enabled)
    {
        return;
    }

    m_closeConfirmationEnabled = enabled;
    if (!m_closeConfirmationEnabled && m_closeConfirmationModal)
    {
        m_closeConfirmationModal->setOpen(false);
    }
    Q_EMIT closeConfirmationEnabledChanged(m_closeConfirmationEnabled);
}

void AntWindow::setCloseConfirmationTitle(const QString& title)
{
    if (m_closeConfirmationTitle == title)
    {
        return;
    }

    m_closeConfirmationTitle = title;
    syncCloseConfirmationModal();
    Q_EMIT closeConfirmationTitleChanged(m_closeConfirmationTitle);
}

void AntWindow::setCloseConfirmationContent(const QString& content)
{
    if (m_closeConfirmationContent == content)
    {
        return;
    }

    m_closeConfirmationContent = content;
    syncCloseConfirmationModal();
    Q_EMIT closeConfirmationContentChanged(m_closeConfirmationContent);
}

void AntWindow::setCloseConfirmationOkText(const QString& text)
{
    if (m_closeConfirmationOkText == text)
    {
        return;
    }

    m_closeConfirmationOkText = text;
    syncCloseConfirmationModal();
    Q_EMIT closeConfirmationOkTextChanged(m_closeConfirmationOkText);
}

void AntWindow::setCloseConfirmationCancelText(const QString& text)
{
    if (m_closeConfirmationCancelText == text)
    {
        return;
    }

    m_closeConfirmationCancelText = text;
    syncCloseConfirmationModal();
    Q_EMIT closeConfirmationCancelTextChanged(m_closeConfirmationCancelText);
}

void AntWindow::forceClose()
{
    m_closingWithoutConfirmation = true;
    if (m_closeConfirmationModal)
    {
        m_closeConfirmationModal->setOpen(false);
    }
    close();
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
    switch (event->type())
    {
    case QEvent::HoverMove:
    {
        auto* hoverEvent = static_cast<QHoverEvent*>(event);
        updateTitleBarHover(hoverEvent->position().toPoint());
        break;
    }
    case QEvent::HoverLeave:
    case QEvent::Leave:
        clearTitleBarHover();
        break;
    case QEvent::ScreenChangeInternal:
    case QEvent::DevicePixelRatioChange:
        // Moving across monitors with different scaling makes the shadow HWND
        // — which is an independent top-level window — keep its previous DPR
        // until its own QScreen is updated. Re-parent the shadow to the new
        // screen and reapply the native frame so the rasterised shadow grid
        // and the painted shadow geometry stay aligned with the host window.
        if (QWindow* shadowWindow = m_legacySoftwareShadow ? m_legacySoftwareShadow->windowHandle() : nullptr)
        {
            if (QScreen* newScreen = windowHandle() ? windowHandle()->screen() : nullptr)
            {
                if (shadowWindow->screen() != newScreen)
                {
                    shadowWindow->setScreen(newScreen);
                }
            }
        }
        applyNativeWindowFrame();
        updateLegacySoftwareShadow();
        updateCornerSmoother();
        update();
        break;
    default:
        break;
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
        auto mapHoverPosition = [this](QHoverEvent* hoverEvent) {
            return m_contentWidget->mapTo(this, hoverEvent->position().toPoint());
        };

        switch (event->type())
        {
        case QEvent::HoverMove:
        {
            auto* hoverEvent = static_cast<QHoverEvent*>(event);
            updateTitleBarHover(mapHoverPosition(hoverEvent));
            break;
        }
        case QEvent::HoverLeave:
        case QEvent::Leave:
            clearTitleBarHover();
            break;
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

void AntWindow::closeEvent(QCloseEvent* event)
{
    if (!m_closeConfirmationEnabled || m_closingWithoutConfirmation)
    {
        m_closingWithoutConfirmation = false;
        if (m_closeConfirmationModal)
        {
            m_closeConfirmationModal->setOpen(false);
        }
        QMainWindow::closeEvent(event);
        return;
    }

    event->ignore();
    showCloseConfirmationModal();
}

void AntWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        m_windowMaximized = QMainWindow::isMaximized();
        applyNativeWindowFrame();
        updateLegacySoftwareShadow();
        updateCornerSmoother();
        update();
    }
    QMainWindow::changeEvent(event);
}

void AntWindow::moveEvent(QMoveEvent* event)
{
    QMainWindow::moveEvent(event);
    if (m_legacyLiveResize)
    {
        return;
    }
    updateLegacySoftwareShadow();
}

void AntWindow::hideEvent(QHideEvent* event)
{
    hideLegacySoftwareShadow();
    QMainWindow::hideEvent(event);
}

void AntWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    invalidateTitleBarButtonRectCache();
#ifdef Q_OS_WIN
    // Always reapply the rounded mask — it controls which pixels of the
    // backing store the compositor shows, so if we leave it at the previous
    // (smaller) rect, growing the window during a live drag leaves stale
    // areas clipped until the user releases. Mask updates are cheap.
    applyLegacyRoundedMask(this, m_cornerRadius);
    // The expensive parts (DWM frame reapply + queued refresh) cause visible
    // flicker because they force the compositor to rebuild the non-client
    // surface. Defer them until the drag finishes (WM_EXITSIZEMOVE handler).
    if (!m_legacyLiveResize && isVisible() && !usesNativeCaptionFrameForWidget(this) &&
        !isMaximized() && !isFullScreen())
    {
        const HWND hwnd = reinterpret_cast<HWND>(winId());
        reapplyDwmFrameMargins(this, hwnd, false, "resize");
        queueLegacyDwmFrameRefresh(this, "resize-deferred");
    }
#endif
    if (!m_legacyLiveResize &&
        (m_legacySoftwareShadow || (windowHandle() && windowHandle()->isExposed())))
    {
        updateLegacySoftwareShadow();
    }
    updateCornerSmoother();
    if (m_themeTransitionOverlay)
    {
        m_themeTransitionOverlay->setGeometry(rect());
        m_themeTransitionOverlay->raise();
    }
}

void AntWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    m_windowMaximized = QMainWindow::isMaximized();
    applyNativeWindowFrame();
    updateCornerSmoother();
    QTimer::singleShot(16, this, [this]() {
        updateLegacySoftwareShadow();
    });
#ifdef Q_OS_WIN
    if (!usesNativeCaptionFrameForWidget(this))
    {
        queueLegacyDwmFrameRefresh(this, "show-deferred");
    }
#endif
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
        const auto titleBarButtonForNativeMouseMessage = [this, hwnd](WPARAM hitTestCode, LPARAM messagePos) {
            const TitleBarButton buttonAtCursor =
                buttonAtPosition(nativeMessageLocalPoint(hwnd, messagePos, devicePixelRatioF()));
            if (titleBarButtonForNativeHitTest(nativeHitTestForTitleBarButton(buttonAtCursor)) != TitleBarButton::None)
            {
                return buttonAtCursor;
            }
            return titleBarButtonForNativeHitTest(hitTestCode);
        };
        const auto beginLegacyLiveResize = [this, hwnd]() {
            // On the Win10 opaque path the window already paints square
            // corners + an opaque backing store all the time. There is no
            // need to switch corner shape or shadow visibility during a
            // resize - doing so just creates extra repaints (which is
            // exactly what produced the "圆角变直角卡住" symptom). Only
            // execute the live-resize state machine when this AntWindow is
            // on the alpha-corner path that actually needs it.
            if (!m_useTranslucentBackground)
            {
                return;
            }
            if (!usesNativeCaptionFrameForWidget(this) && !m_legacyLiveResize &&
                !isMaximized() && !isFullScreen())
            {
                m_legacyLiveResize = true;
                setProperty("antWindowLegacyLiveResize", true);
                hideLegacySoftwareShadow();
                // Extend the DWM frame across the whole client area for the
                // duration of an edge resize. DWM provides its own backdrop in
                // the extended region, which masks the single-frame gap where
                // Qt's backing store has not yet caught up with WM_SIZE.
                //
                // Only meaningful when the window has WA_TranslucentBackground
                // and a real alpha channel - on the Win10 opaque path (no
                // translucent background) glass extension is unnecessary
                // (the backing store is already opaque to the edges) and
                // contributes to compositor state drift across repeated
                // resize cycles, eventually producing the "black screen"
                // symptom users see in the example.
                if (m_useTranslucentBackground)
                {
                    const HWND targetHwnd = hwnd ? hwnd : reinterpret_cast<HWND>(winId());
                    DwmExtendFrameIntoClientAreaFn extendFrame = nullptr;
                    if (targetHwnd && resolveDwmApis(nullptr, &extendFrame) && extendFrame)
                    {
                        const DwmMargins fullFrameMargins{-1, -1, -1, -1};
                        extendFrame(targetHwnd, &fullFrameMargins);
                    }
                }
                // Force one repaint with square corners + smoother off before
                // the first WM_SIZE lands, so the first resize frame is opaque
                // to the edges. Pure window moves keep the normal rounded
                // shadow visible and update it in moveEvent().
                updateCornerSmoother();
                update();
            }
        };

        switch (uMsg)
        {
        case WM_DPICHANGED:
        {
            // Win32 fires WM_DPICHANGED with the new suggested window rect when
            // the window crosses to a monitor with a different DPI. Defer to Qt
            // for the main window's own geometry update, then refresh the
            // shadow HWND (a separate top-level window whose DPR doesn't track
            // automatically) so the painted shadow lines up at the new scale.
            QPointer<AntWindow> guard(this);
            QTimer::singleShot(0, this, [guard]() {
                if (!guard)
                {
                    return;
                }
                if (QWindow* shadowWindow = guard->m_legacySoftwareShadow
                                                ? guard->m_legacySoftwareShadow->windowHandle()
                                                : nullptr)
                {
                    if (QScreen* newScreen = guard->windowHandle() ? guard->windowHandle()->screen() : nullptr)
                    {
                        if (shadowWindow->screen() != newScreen)
                        {
                            shadowWindow->setScreen(newScreen);
                        }
                    }
                }
                guard->applyNativeWindowFrame();
                guard->updateLegacySoftwareShadow();
                guard->updateCornerSmoother();
                guard->update();
            });
            break;
        }
        case WM_SIZING:
            beginLegacyLiveResize();
            break;
        case WM_EXITSIZEMOVE:
        case WM_CAPTURECHANGED:
        case WM_CANCELMODE:
            // WM_EXITSIZEMOVE is the normal end of the modal resize loop. But
            // Windows can also cancel the loop without firing WM_EXITSIZEMOVE
            // - WM_CAPTURECHANGED fires when mouse capture is taken away (e.g.
            // a system event steals focus mid-drag) and WM_CANCELMODE fires
            // when modal state is explicitly cancelled. If we miss the
            // termination signal, m_legacyLiveResize stays true forever and
            // the next paint keeps drawing square corners (the symptom users
            // see as "圆角变直角"). Handle all three the same way.
            if (m_legacyLiveResize)
            {
                m_legacyLiveResize = false;
                setProperty("antWindowLegacyLiveResize", false);
                // Synchronously schedule a repaint with the rounded path
                // visible. The deferred 16ms timer below handles the heavier
                // frame/shadow/smoother refresh, but the corner shape itself
                // should flip back immediately on the next paint - no need
                // to wait a frame.
                update();
                QPointer<AntWindow> guard(this);
                QTimer::singleShot(16, this, [guard]() {
                    if (!guard)
                    {
                        return;
                    }
                    guard->applyNativeWindowFrame();
                    guard->updateLegacySoftwareShadow();
                    guard->updateCornerSmoother();
                    guard->update();
                });
            }
            else if (uMsg == WM_EXITSIZEMOVE && !usesNativeCaptionFrameForWidget(this))
            {
                updateLegacySoftwareShadow();
            }
            break;
        case WM_NCCALCSIZE:
        {
            *result = 0;
            return true;
        }
        case WM_NCACTIVATE:
        {
            // On the Win10 opaque path the window still carries WS_THICKFRAME
            // (needed for native edge resize hit-testing). When focus leaves
            // and re-enters the window, DefWindowProc repaints the sizing
            // border, producing a thin gray "focus" rectangle around the
            // four edges. Suppress that visual update by passing lParam=-1
            // to DefWindowProc - it still tracks the activation state but
            // skips the non-client repaint. Win11 caption windows keep the
            // default behavior so the title bar continues to dim/undim with
            // focus.
            if (!usesNativeCaptionFrameForWidget(this))
            {
                *result = ::DefWindowProcW(hwnd, WM_NCACTIVATE, msg->wParam, -1);
                return true;
            }
            return false;
        }
        case WM_NCPAINT:
        {
            // Same rationale as WM_NCACTIVATE: any non-client paint on the
            // opaque Win10 frameless window draws the WS_THICKFRAME sizing
            // border that the user reads as a focus rectangle. Consume the
            // message so no NC pixels are drawn. The custom Ant title bar
            // and content are painted by Qt as normal client widgets.
            if (!usesNativeCaptionFrameForWidget(this))
            {
                *result = 0;
                return true;
            }
            return false;
        }
        case WM_NCHITTEST:
        {
            const QPoint widgetPos = nativeMessageLocalPoint(hwnd, lParam, devicePixelRatioF());
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
                const bool inHorizontalResizeBand =
                    widgetPos.x() >= -borderWidth && widgetPos.x() < clientWidth + borderWidth;
                const bool inVerticalResizeBand =
                    widgetPos.y() >= -borderHeight && widgetPos.y() < clientHeight + borderHeight;
                const bool left = inVerticalResizeBand &&
                                  widgetPos.x() >= -borderWidth && widgetPos.x() < borderWidth;
                const bool right = inVerticalResizeBand &&
                                   widgetPos.x() < clientWidth + borderWidth &&
                                   widgetPos.x() >= clientWidth - borderWidth;
                const bool top = inHorizontalResizeBand &&
                                 widgetPos.y() >= -borderHeight && widgetPos.y() < borderHeight;
                const bool bottom = inHorizontalResizeBand &&
                                    widgetPos.y() < clientHeight + borderHeight &&
                                    widgetPos.y() >= clientHeight - borderHeight;

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
            const TitleBarButton button = titleBarButtonForNativeMouseMessage(msg->wParam, lParam);
            if (button != TitleBarButton::None)
            {
                const WPARAM nativeHitTest = static_cast<WPARAM>(nativeHitTestForTitleBarButton(button));
                setHoveredTitleBarButton(button);
                requestNonClientHoverTracking(hwnd);
                *result = ::DefWindowProcW(hwnd, uMsg, nativeHitTest, lParam);
                return true;
            }
            break;
        }
        case WM_NCMOUSEHOVER:
        {
            const TitleBarButton button = titleBarButtonForNativeMouseMessage(msg->wParam, lParam);
            if (button != TitleBarButton::None)
            {
                const WPARAM nativeHitTest = static_cast<WPARAM>(nativeHitTestForTitleBarButton(button));
                *result = ::DefWindowProcW(hwnd, uMsg, nativeHitTest, lParam);
                return true;
            }
            break;
        }
        case WM_NCMOUSELEAVE:
        {
            clearTitleBarHover();
            break;
        }
        case WM_NCLBUTTONDOWN:
        {
            const TitleBarButton button = titleBarButtonForNativeMouseMessage(msg->wParam, lParam);
            if (button != TitleBarButton::None)
            {
                m_pressedButton = button;
                updateTitleBarRegion(titleBarButtonRect(button));
                *result = 0;
                return true;
            }
            if (isNativeResizeHitTest(msg->wParam) || msg->wParam == HTCAPTION)
            {
                if (isNativeResizeHitTest(msg->wParam))
                {
                    beginLegacyLiveResize();
                }
                m_pressedButton = TitleBarButton::None;
                *result = ::DefWindowProcW(hwnd, uMsg, msg->wParam, lParam);
                return true;
            }
            break;
        }
        case WM_NCLBUTTONUP:
        {
            const TitleBarButton button = titleBarButtonForNativeMouseMessage(msg->wParam, lParam);
            if (button != TitleBarButton::None || m_pressedButton != TitleBarButton::None)
            {
                const QPoint widgetPos = nativeMessageLocalPoint(hwnd, lParam, devicePixelRatioF());
                const TitleBarButton pressedButton = m_pressedButton;
                m_pressedButton = TitleBarButton::None;
                if (pressedButton != TitleBarButton::None && buttonAtPosition(widgetPos) == pressedButton)
                {
                    handleButtonClicked(pressedButton);
                }
                updateTitleBarRegion(titleBarButtonRect(pressedButton));
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

bool AntWindow::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result)
{
#ifdef Q_OS_WIN
    if ((eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG") || !message)
    {
        return false;
    }

    auto* msg = static_cast<MSG*>(message);
    if (msg->message != WM_NCHITTEST)
    {
        return false;
    }

    if (!windowHandle())
    {
        return false;
    }

    const HWND ownerHwnd = reinterpret_cast<HWND>(winId());
    const HWND messageHwnd = msg->hwnd;
    if (!ownerHwnd || !messageHwnd || messageHwnd == ownerHwnd)
    {
        return false;
    }

    if (::GetAncestor(messageHwnd, GA_ROOT) != ownerHwnd && !::IsChild(ownerHwnd, messageHwnd))
    {
        return false;
    }

    if (!shouldForwardChildHitTestToAntWindow(this, msg->lParam))
    {
        return false;
    }

    *result = HTTRANSPARENT;
    setProperty("antWindowChildHitTestForwarded", property("antWindowChildHitTestForwarded").toInt() + 1);
    return true;
#else
    Q_UNUSED(eventType)
    Q_UNUSED(message)
    Q_UNUSED(result)
    return false;
#endif
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

    ensureTitleBarButtonRectCache();
    for (TitleBarButton button : kTitleBarButtonsRightToLeft)
    {
        if (m_titleBarButtonRectCache[titleBarButtonIndex(button)].contains(pos))
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

void AntWindow::updateTitleBarHover(const QPoint& pos)
{
    setHoveredTitleBarButton(buttonAtPosition(pos));
    if (isTitleBarArea(pos))
    {
        setCursor(Qt::ArrowCursor);
    }
}

void AntWindow::setHoveredTitleBarButton(TitleBarButton button)
{
    if (button != TitleBarButton::None && !isTitleBarButtonVisible(button))
    {
        button = TitleBarButton::None;
    }
    if (m_hoveredButton == button)
    {
        return;
    }

    const TitleBarButton oldHovered = m_hoveredButton;
    m_hoveredButton = button;
    if (oldHovered != TitleBarButton::None)
    {
        updateTitleBarRegion(titleBarButtonRect(oldHovered));
    }
    if (m_hoveredButton != TitleBarButton::None)
    {
        updateTitleBarRegion(titleBarButtonRect(m_hoveredButton));
    }
}

void AntWindow::clearTitleBarHover()
{
    setHoveredTitleBarButton(TitleBarButton::None);
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
        updateTitleBarRegion(titleBarButtonRect(m_pressedButton));
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
        updateTitleBarRegion(titleBarButtonRect(pressedButton));
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
        startThemeModeTransition();
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

void AntWindow::ensureTitleBarButtonRectCache() const
{
    const int mask = titleBarButtonVisibilityMask();
    if (m_titleBarButtonCacheWidth == width() && m_titleBarButtonCacheMask == mask)
    {
        return;
    }

    for (QRect& rect : m_titleBarButtonRectCache)
    {
        rect = QRect();
    }

    int right = width();
    for (TitleBarButton button : kTitleBarButtonsRightToLeft)
    {
        if (!isTitleBarButtonVisible(button))
        {
            continue;
        }

        right -= TitleBarButtonWidth;
        m_titleBarButtonRectCache[titleBarButtonIndex(button)] = QRect(right,
                                                                       0,
                                                                       TitleBarButtonWidth,
                                                                       TitleBarHeight);
    }

    m_titleBarButtonCacheWidth = width();
    m_titleBarButtonCacheMask = mask;
    ++m_titleBarButtonRectCacheRebuildCount;
    syncTitleBarPerfCounters();
}

void AntWindow::invalidateTitleBarButtonRectCache() const
{
    m_titleBarButtonCacheWidth = -1;
    m_titleBarButtonCacheMask = -1;
    syncTitleBarPerfCounters();
}

int AntWindow::titleBarButtonVisibilityMask() const
{
    int mask = 0;
    for (TitleBarButton button : kTitleBarButtonsRightToLeft)
    {
        if (isTitleBarButtonVisible(button))
        {
            mask |= (1 << titleBarButtonIndex(button));
        }
    }
    return mask;
}

QRect AntWindow::titleBarButtonStripRect() const
{
    ensureTitleBarButtonRectCache();
    QRect strip;
    for (TitleBarButton button : kTitleBarButtonsRightToLeft)
    {
        strip = strip.united(m_titleBarButtonRectCache[titleBarButtonIndex(button)]);
    }
    return strip;
}

void AntWindow::updateTitleBarRegion(const QRect& rect)
{
    const QRect dirty = rect.intersected(titleBarRect());
    if (dirty.isEmpty())
    {
        return;
    }

    update(dirty);
    ++m_titleBarDirtyUpdateCount;
    syncTitleBarPerfCounters();
}

void AntWindow::syncTitleBarPerfCounters() const
{
    auto* self = const_cast<AntWindow*>(this);
    self->setProperty("antWindowTitleBarButtonRectCacheRebuildCount", m_titleBarButtonRectCacheRebuildCount);
    self->setProperty("antWindowTitleBarDirtyUpdateCount", m_titleBarDirtyUpdateCount);
    self->setProperty("antWindowTitleBarButtonCacheWidth", m_titleBarButtonCacheWidth);
    self->setProperty("antWindowTitleBarButtonCacheMask", m_titleBarButtonCacheMask);
}

void AntWindow::showCloseConfirmationModal()
{
    if (!m_closeConfirmationModal)
    {
        auto* modal = new AntModal(this);
        modal->setObjectName(QStringLiteral("AntWindowCloseConfirmationModal"));
        modal->setShowCancel(true);
        modal->setClosable(false);
        modal->setMaskClosable(false);
        modal->setCentered(false);
        modal->setDialogWidth(416);
        modal->setCommandIconType(Ant::IconType::ExclamationCircle);
        connect(modal, &AntModal::confirmed, this, [this]() {
            m_closingWithoutConfirmation = true;
            close();
        });
        connect(modal, &AntModal::canceled, this, [this]() {
            m_closingWithoutConfirmation = false;
        });
        m_closeConfirmationModal = modal;
    }

    syncCloseConfirmationModal();
    m_closeConfirmationModal->setOpen(true);
    m_closeConfirmationModal->raise();
}

void AntWindow::syncCloseConfirmationModal()
{
    if (!m_closeConfirmationModal)
    {
        return;
    }

    m_closeConfirmationModal->setTitle(m_closeConfirmationTitle);
    m_closeConfirmationModal->setContent(m_closeConfirmationContent);
    m_closeConfirmationModal->setOkText(m_closeConfirmationOkText);
    m_closeConfirmationModal->setCancelText(m_closeConfirmationCancelText);
}

void AntWindow::startThemeModeTransition()
{
    if (m_themeTransitionOverlay)
    {
        m_themeTransitionOverlay->hide();
        m_themeTransitionOverlay->deleteLater();
        m_themeTransitionOverlay = nullptr;
    }

    if (!isVisible() || width() <= 0 || height() <= 0)
    {
        antTheme->toggleThemeMode();
        return;
    }

    const QPixmap oldFrame = captureAntWindowFrame(this);
    if (oldFrame.isNull())
    {
        antTheme->toggleThemeMode();
        return;
    }

    const auto transitionMode = usesLegacyOpaquePath()
        ? AntWindowThemeTransitionOverlay::Mode::CrossFade
        : AntWindowThemeTransitionOverlay::Mode::CircularReveal;
    auto* overlay = new AntWindowThemeTransitionOverlay(this,
                                                        oldFrame,
                                                        titleBarButtonRect(TitleBarButton::Theme).center(),
                                                        transitionMode);
    m_themeTransitionOverlay = overlay;
    overlay->setGeometry(rect());
    overlay->raise();
    overlay->show();
    overlay->update();

    antTheme->toggleThemeMode();

    overlay->hide();
    activateAntWindowLayoutTree(this);
    const QPixmap newFrame = captureAntWindowFrame(this);
    overlay->setNewFrame(newFrame);
    overlay->show();
    overlay->raise();

    overlay->startTransition(kThemeTransitionDurationMs, [this, overlay]() {
        if (m_themeTransitionOverlay == overlay)
        {
            m_themeTransitionOverlay = nullptr;
        }
        overlay->deleteLater();
        if (m_cornerSmoother)
        {
            m_cornerSmoother->raise();
        }
    });
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

    const bool useNativeCaption = usesNativeCaptionFrameForWidget(this);
    ensureNativeWindowStyle(hwnd, useNativeCaption);
    applyLegacyClassDropShadow(this, hwnd, useNativeCaption);
    applyLegacyRoundedMask(this, m_cornerRadius);
    setProperty(kUsesNativeCaptionFrameProperty, useNativeCaption);

    DwmSetWindowAttributeFn setWindowAttribute = nullptr;
    DwmExtendFrameIntoClientAreaFn extendFrame = nullptr;
    if (!resolveDwmApis(&setWindowAttribute, &extendFrame))
    {
        return;
    }

    if (extendFrame)
    {
        applyDwmFrameMargins(this, hwnd, useNativeCaption, extendFrame, "frame");
    }

    if (!setWindowAttribute)
    {
        return;
    }

    if (useNativeCaption)
    {
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
    }

    const auto& token = antTheme->tokens();
    // On the Win10 opaque path the window keeps WS_THICKFRAME without
    // WS_CAPTION. DWM draws its own border around that frame, and a non-
    // sentinel color produces a visible gray outline at the four edges
    // (Windows' default "you can resize me here" cue). We don't want that
    // chrome around a frameless Ant-styled window, so explicitly pass
    // DWMWA_COLOR_NONE to suppress the DWM-drawn border. On the Win11
    // caption path the DWM border is intentional - it forms the rounded
    // outline that matches the native frame radius - so keep using the
    // token border color there.
    if (m_useTranslucentBackground)
    {
        const COLORREF borderColor = RGB(token.colorBorder.red(), token.colorBorder.green(), token.colorBorder.blue());
        setWindowAttribute(hwnd, kDwmBorderColor, &borderColor, sizeof(borderColor));
    }
    else
    {
        const COLORREF noBorder = kDwmBorderColorNone;
        setWindowAttribute(hwnd, kDwmBorderColor, &noBorder, sizeof(noBorder));
    }

    const BOOL darkMode = antTheme->themeMode() == Ant::ThemeMode::Dark;
    setWindowAttribute(hwnd, kDwmUseImmersiveDarkMode, &darkMode, sizeof(darkMode));
#else
    update();
#endif
}

void AntWindow::updateLegacySoftwareShadow()
{
#ifdef Q_OS_WIN
    const bool enabled = isVisible()
        && windowHandle()
        && !usesNativeCaptionFrameForWidget(this)
        && !isMaximized()
        && !isFullScreen()
        && !isMinimized();

    setProperty("antWindowLegacySoftwareShadowEnabled", enabled);
    setProperty("antWindowLegacySoftwareShadowMargin", kLegacySoftwareShadowMargin);
    setProperty("antWindowLegacySoftwareShadowInnerClearance", kLegacySoftwareShadowInnerClearance);

    if (!enabled)
    {
        hideLegacySoftwareShadow();
        return;
    }

    if (!m_legacySoftwareShadow)
    {
        auto* shadow = new AntWindowLegacySoftwareShadow(this);
        m_legacySoftwareShadow = shadow;
        connect(antTheme, &AntTheme::themeChanged, shadow, qOverload<>(&QWidget::update));
    }

    auto* shadow = static_cast<AntWindowLegacySoftwareShadow*>(m_legacySoftwareShadow);
    if (shadow)
    {
        // On the Win10 opaque path the window itself is drawn with square
        // corners (see AntWindowStyle::drawWindow), so the shadow must wrap
        // a square panel too - otherwise the shadow's rounded inner cutout
        // would leave four visible quarter-circle bites at the corners.
        shadow->setCornerRadius(m_useTranslucentBackground ? m_cornerRadius : 0);
    }

    // Pin the shadow HWND to the same QScreen as the host window so its
    // backing-store DPR tracks the host across monitors with different
    // scaling. Without this the shadow keeps the DPR of whichever screen it
    // was created on, which makes the rasterised shadow drift away from the
    // window outline when crossing scale boundaries.
    if (QWindow* shadowWindow = m_legacySoftwareShadow->windowHandle())
    {
        if (QScreen* hostScreen = windowHandle() ? windowHandle()->screen() : nullptr)
        {
            if (shadowWindow->screen() != hostScreen)
            {
                shadowWindow->setScreen(hostScreen);
            }
        }
    }

    const QRect shadowGeometry = geometry().adjusted(-kLegacySoftwareShadowMargin,
                                                     -kLegacySoftwareShadowMargin,
                                                     kLegacySoftwareShadowMargin,
                                                     kLegacySoftwareShadowMargin);
    m_legacySoftwareShadow->setGeometry(shadowGeometry);
    setProperty("antWindowLegacySoftwareShadowGeometry", shadowGeometry);
    setProperty("antWindowLegacySoftwareShadowGeometryMode", QStringLiteral("qt-logical"));
    m_legacySoftwareShadow->setProperty("antWindowLegacySoftwareShadowGeometry", shadowGeometry);
    m_legacySoftwareShadow->setProperty("antWindowLegacySoftwareShadowGeometryMode", QStringLiteral("qt-logical"));
    if (!m_legacySoftwareShadow->isVisible())
    {
        m_legacySoftwareShadow->show();
    }
    m_legacySoftwareShadow->setProperty("antWindowLegacySoftwareShadowDevicePixelRatio",
                                        m_legacySoftwareShadow->devicePixelRatioF());

    const HWND hwnd = reinterpret_cast<HWND>(winId());
    const HWND shadowHwnd = reinterpret_cast<HWND>(m_legacySoftwareShadow->winId());
    if (shadowHwnd && hwnd)
    {
        makeAntWindowShadowClickThrough(m_legacySoftwareShadow);
        // QWidget::geometry() is expressed in Qt logical pixels while
        // SetWindowPos() consumes native physical pixels on per-monitor-DPI
        // Windows. Let Qt own the move/size conversion above, and only use the
        // native call to keep the shadow directly behind the owner without
        // reintroducing a scale-dependent offset at 125%/150% DPI.
        ::SetWindowPos(shadowHwnd,
                       hwnd,
                       0,
                       0,
                       0,
                       0,
                       SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
    }
    else
    {
        m_legacySoftwareShadow->show();
    }
#else
    hideLegacySoftwareShadow();
#endif
}

void AntWindow::hideLegacySoftwareShadow()
{
    setProperty("antWindowLegacySoftwareShadowEnabled", false);
    if (m_legacySoftwareShadow)
    {
#ifdef Q_OS_WIN
        if (const WId shadowId = m_legacySoftwareShadow->internalWinId())
        {
            HWND shadowHwnd = reinterpret_cast<HWND>(shadowId);
            makeAntWindowShadowClickThrough(m_legacySoftwareShadow);
            ::ShowWindow(shadowHwnd, SW_HIDE);
            ::SetWindowPos(shadowHwnd,
                           nullptr,
                           0,
                           0,
                           0,
                           0,
                           SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                               SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
        }
#endif
        m_legacySoftwareShadow->hide();
    }
}

void AntWindow::updateCornerSmoother()
{
    if (!m_cornerSmoother)
    {
        return;
    }

    auto* smoother = static_cast<AntWindowCornerSmoother*>(m_cornerSmoother);
    // During an interactive drag-resize the rounded clip is also disabled in
    // AntWindowStyle::drawWindow so the entire client rect stays opaque and
    // does not flash through to the desktop on grow/shrink. Match that here:
    // running the alpha-erase smoother on top of the square painted client
    // would still produce alpha=0 corner pixels and re-introduce the flicker.
    const bool liveResize = m_legacyLiveResize;
    const int effectiveRadius =
        (m_windowMaximized || isFullScreen() || liveResize) ? 0 : m_cornerRadius;
    smoother->setCornerRadius(effectiveRadius);
    if (m_cornerSmoother->geometry() != rect())
    {
        m_cornerSmoother->setGeometry(rect());
    }
#if defined(Q_OS_WIN)
    const bool hasNativeHwnd =
        makeAntWindowNativeInputTransparent(m_cornerSmoother,
                                            "antWindowCornerSmootherClickThrough",
                                            false,
                                            true);
    m_cornerSmoother->setProperty("antWindowCornerSmootherNativeHwnd", hasNativeHwnd);
#endif
    m_cornerSmoother->raise();
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
