#include "AntDockWidget.h"

#include <QEvent>
#include <QHideEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMargins>
#include <QMouseEvent>
#include <QMoveEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmap>
#include <QPointer>
#include <QRegion>
#include <QResizeEvent>
#include <QScreen>
#include <QShowEvent>
#include <QTimer>
#include <QWindow>
#include <QtMath>

#include "core/AntTheme.h"
#include "styles/AntIconSvgRenderer.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#include <windowsx.h>
#endif

namespace
{
constexpr int kFloatingCornerRadius = 8;
constexpr int kFloatingBorderWidth = 1;
constexpr int kFloatingTitleBarHeight = 40;
constexpr int kEmbeddedTitleBarHeight = 32;
constexpr int kFloatingTitleButtonWidth = 46;
constexpr auto kDockLegacyLiveResizeProperty = "antDockLegacyLiveResize";

#if defined(Q_OS_WIN)
constexpr int kNativeFrameShadowMargin = 14;
constexpr int kLegacyRoundedMaskFrameInset = 1;
constexpr auto kDockNativeFrameEnabledProperty = "antDockNativeWindowFrameEnabled";
constexpr auto kDockUsesNativeCaptionFrameProperty = "antDockUsesNativeCaptionFrame";
constexpr auto kDockDwmFrameMarginsProperty = "antDockDwmFrameMargins";
constexpr auto kDockDwmFrameApplyCountProperty = "antDockDwmFrameApplyCount";
constexpr auto kDockDwmFrameLastReasonProperty = "antDockDwmFrameLastReason";
constexpr auto kDockLegacyRoundedMaskAppliedProperty = "antDockLegacyRoundedMaskApplied";
constexpr auto kDockLegacyShadowEnabledProperty = "antDockLegacySoftwareShadowEnabled";
constexpr auto kDockLegacyShadowClickThroughProperty = "antDockLegacySoftwareShadowClickThrough";
constexpr auto kDockLegacyShadowObjectName = "AntDockLegacySoftwareShadow";
constexpr auto kDockLegacyShadowRingRegionProperty = "antDockLegacySoftwareShadowRingRegion";
constexpr auto kDockLegacyShadowGeometryProperty = "antDockLegacySoftwareShadowGeometry";
constexpr auto kDockLegacyShadowGeometryModeProperty = "antDockLegacySoftwareShadowGeometryMode";
constexpr auto kDockLegacyShadowDprProperty = "antDockLegacySoftwareShadowDevicePixelRatio";
constexpr auto kDockDwmFrameRefreshQueuedProperty = "antDockDwmFrameRefreshQueued";
constexpr auto kDockForceLegacyFramePolicyProperty = "antDockForceLegacyFramePolicy";
#endif

QRectF centeredIconRect(const QRect& buttonRect, qreal iconSize = 14.0)
{
    if (buttonRect.isNull())
    {
        return {};
    }

    return QRectF(buttonRect.center().x() - iconSize / 2.0,
                  buttonRect.center().y() - iconSize / 2.0,
                  iconSize,
                  iconSize);
}

QPixmap renderAntdIconPixmap(const QString& iconName, qreal iconSize, const QColor& color, qreal devicePixelRatio)
{
    return AntIconSvgRenderer::renderIconPixmap(iconName, iconSize, color, devicePixelRatio);
}

#if defined(Q_OS_WIN)
void makeWindowClickThrough(QWidget* widget)
{
    if (!widget)
    {
        return;
    }

    const HWND hwnd = reinterpret_cast<HWND>(widget->winId());
    if (!hwnd)
    {
        return;
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
                       SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE |
                           SWP_FRAMECHANGED);
    }

    widget->setProperty(kDockLegacyShadowClickThroughProperty, true);
}

void applyShadowWindowRingRegion(QWidget* widget, int margin, int cornerRadius)
{
    if (!widget)
    {
        return;
    }

    const HWND hwnd = reinterpret_cast<HWND>(widget->winId());
    if (!hwnd)
    {
        return;
    }

    RECT nativeRect{};
    if (!::GetWindowRect(hwnd, &nativeRect))
    {
        return;
    }

    const int nativeWidth = qMax(0L, nativeRect.right - nativeRect.left);
    const int nativeHeight = qMax(0L, nativeRect.bottom - nativeRect.top);
    if (nativeWidth <= 0 || nativeHeight <= 0)
    {
        return;
    }

    const qreal xScale = widget->width() > 0 ? static_cast<qreal>(nativeWidth) / widget->width() : 1.0;
    const qreal yScale = widget->height() > 0 ? static_cast<qreal>(nativeHeight) / widget->height() : 1.0;
    const int nativeMarginX = qBound(1, qRound(margin * xScale), nativeWidth / 2);
    const int nativeMarginY = qBound(1, qRound(margin * yScale), nativeHeight / 2);
    const int nativeRadiusX = qMax(0, qRound(cornerRadius * 2.0 * xScale));
    const int nativeRadiusY = qMax(0, qRound(cornerRadius * 2.0 * yScale));

    HRGN outerRegion = ::CreateRectRgn(0, 0, nativeWidth, nativeHeight);
    HRGN innerRegion = ::CreateRoundRectRgn(nativeMarginX,
                                            nativeMarginY,
                                            qMax(nativeMarginX + 1, nativeWidth - nativeMarginX + 1),
                                            qMax(nativeMarginY + 1, nativeHeight - nativeMarginY + 1),
                                            nativeRadiusX,
                                            nativeRadiusY);
    if (!outerRegion || !innerRegion)
    {
        if (outerRegion) ::DeleteObject(outerRegion);
        if (innerRegion) ::DeleteObject(innerRegion);
        return;
    }

    ::CombineRgn(outerRegion, outerRegion, innerRegion, RGN_DIFF);
    ::DeleteObject(innerRegion);
    if (::SetWindowRgn(hwnd, outerRegion, TRUE) == 0)
    {
        ::DeleteObject(outerRegion);
        widget->setProperty(kDockLegacyShadowRingRegionProperty, false);
        return;
    }

    widget->setProperty(kDockLegacyShadowRingRegionProperty, true);
}

bool fillNativeDockEraseBackground(QWidget* widget, WPARAM paintDevice)
{
    if (!widget || !paintDevice)
    {
        return false;
    }

    const HWND hwnd = reinterpret_cast<HWND>(widget->winId());
    if (!hwnd)
    {
        return false;
    }

    RECT clientRect{};
    if (!::GetClientRect(hwnd, &clientRect))
    {
        return false;
    }

    const auto& token = antTheme->tokens();
    QColor fill = token.colorBgElevated;
    HBRUSH brush = ::CreateSolidBrush(RGB(fill.red(), fill.green(), fill.blue()));
    if (!brush)
    {
        return false;
    }

    ::FillRect(reinterpret_cast<HDC>(paintDevice), &clientRect, brush);
    ::DeleteObject(brush);
    widget->setProperty("antDockNativeEraseBackgroundFilled", true);
    return true;
}

void refreshFloatingDockWindow(AntDockWidget* dockWidget, const char* reason)
{
    if (!dockWidget)
    {
        return;
    }

    dockWidget->setProperty("antDockFloatingRefreshReason", QString::fromLatin1(reason ? reason : ""));
    dockWidget->setProperty("antDockFloatingRefreshCount",
                            dockWidget->property("antDockFloatingRefreshCount").toInt() + 1);

    if (QWidget* titleBar = dockWidget->titleBarWidget())
    {
        titleBar->updateGeometry();
        titleBar->update();
    }
    if (QWidget* content = dockWidget->widget())
    {
        content->updateGeometry();
        content->update();
    }
    dockWidget->updateGeometry();
    dockWidget->update();

    if (const WId windowId = dockWidget->internalWinId())
    {
        ::RedrawWindow(reinterpret_cast<HWND>(windowId),
                       nullptr,
                       nullptr,
                       RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_NOERASE);
    }
}

class AntDockLegacySoftwareShadow : public QWidget
{
public:
    explicit AntDockLegacySoftwareShadow(QWidget* owner)
        : QWidget(owner, Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowDoesNotAcceptFocus)
    {
        setObjectName(QString::fromLatin1(kDockLegacyShadowObjectName));
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setAttribute(Qt::WA_ShowWithoutActivating, true);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        setWindowFlag(Qt::WindowTransparentForInput, true);
#endif
        setFocusPolicy(Qt::NoFocus);
        setProperty("shadowMargin", kNativeFrameShadowMargin);
        setProperty(kDockLegacyShadowClickThroughProperty, false);
        setProperty(kDockLegacyShadowRingRegionProperty, false);
    }

    void setCornerRadius(int radius)
    {
        m_cornerRadius = qMax(0, radius);
        update();
    }

protected:
    void showEvent(QShowEvent* event) override
    {
        QWidget::showEvent(event);
        makeWindowClickThrough(this);
    }

    bool nativeEvent(const QByteArray& eventType, void* message, AntNativeEventResult* result) override
    {
        if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
        {
            auto* msg = static_cast<MSG*>(message);
            if (msg && msg->message == WM_NCHITTEST)
            {
                *result = HTTRANSPARENT;
                setProperty(kDockLegacyShadowClickThroughProperty, true);
                return true;
            }
        }
        return QWidget::nativeEvent(eventType, message, result);
    }

    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(rect(), Qt::transparent);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QRectF panelRect = QRectF(rect()).adjusted(kNativeFrameShadowMargin,
                                                        kNativeFrameShadowMargin,
                                                        -kNativeFrameShadowMargin,
                                                        -kNativeFrameShadowMargin);
        if (panelRect.isEmpty())
        {
            return;
        }

        QColor shadowBase = antTheme->tokens().colorShadow;
        const qreal maxOpacity = antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.046 : 0.032;
        for (int distance = kNativeFrameShadowMargin; distance > 0; --distance)
        {
            const qreal t = qBound<qreal>(
                0.0,
                1.0 - static_cast<qreal>(distance - 1) / static_cast<qreal>(kNativeFrameShadowMargin),
                1.0);
            const qreal eased = t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
            const qreal opacity = qBound<qreal>(0.0, maxOpacity * eased, 0.22);
            if (opacity <= 0.0)
            {
                continue;
            }

            QColor shadow = shadowBase;
            shadow.setAlphaF(opacity);

            QPainterPath outerPath;
            const qreal radiusGrowth = static_cast<qreal>(distance) * 0.55;
            const QRectF outer = panelRect.adjusted(-distance, -distance, distance, distance);
            outerPath.addRoundedRect(outer, m_cornerRadius + radiusGrowth, m_cornerRadius + radiusGrowth);

            QPainterPath innerPath;
            const int innerDistance = qMax(0, distance - 1);
            const qreal innerRadiusGrowth = static_cast<qreal>(innerDistance) * 0.55;
            const QRectF inner = panelRect.adjusted(-innerDistance, -innerDistance, innerDistance, innerDistance);
            innerPath.addRoundedRect(inner, m_cornerRadius + innerRadiusGrowth, m_cornerRadius + innerRadiusGrowth);

            painter.fillPath(outerPath.subtracted(innerPath), shadow);
        }
    }

private:
    int m_cornerRadius = kFloatingCornerRadius;
};

using GetDpiForWindowFn = UINT(WINAPI*)(HWND);
using GetSystemMetricsForDpiFn = int(WINAPI*)(int, UINT);
using DwmSetWindowAttributeFn = HRESULT(WINAPI*)(HWND, DWORD, LPCVOID, DWORD);
using RtlGetVersionFn = LONG(WINAPI*)(OSVERSIONINFOW*);

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
    return qMax(8, systemMetricForDpi(frameMetric, dpi) + systemMetricForDpi(paddedMetric, dpi));
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

bool supportsNativeCaptionFrame(const QWidget* widget = nullptr)
{
    constexpr int kWindows11Build = 22000;
    const bool forceLegacyFrame = widget && widget->property(kDockForceLegacyFramePolicyProperty).toBool();
    return windowsBuildNumber() >= kWindows11Build && !forceLegacyFrame;
}

DwmMargins shadowPreservingDwmMargins(bool useNativeCaption)
{
    return useNativeCaption ? DwmMargins{1, 1, 1, 1} : DwmMargins{0, 0, 0, 0};
}

QPoint nativeMessageLocalPoint(HWND hwnd, LPARAM messagePos, qreal devicePixelRatio)
{
    POINT nativePoint{GET_X_LPARAM(messagePos), GET_Y_LPARAM(messagePos)};
    ::ScreenToClient(hwnd, &nativePoint);

    const qreal dpr = qMax<qreal>(1.0, devicePixelRatio);
    return QPoint(qRound(static_cast<qreal>(nativePoint.x) / dpr),
                  qRound(static_cast<qreal>(nativePoint.y) / dpr));
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
    if (!widget || !hwnd)
    {
        return;
    }

    const LONG_PTR classStyle = ::GetClassLongPtrW(hwnd, GCL_STYLE);
    const LONG_PTR newClassStyle = useNativeCaption ? classStyle : (classStyle & ~static_cast<LONG_PTR>(CS_DROPSHADOW));
    if (newClassStyle != classStyle)
    {
        ::SetClassLongPtrW(hwnd, GCL_STYLE, newClassStyle);
    }
}

void applyLegacyRoundedMask(QWidget* widget, int radius, bool useNativeCaption)
{
    if (!widget)
    {
        return;
    }

    if (useNativeCaption || widget->isMaximized() || widget->isFullScreen() || radius <= 0)
    {
        widget->setProperty(kDockLegacyRoundedMaskAppliedProperty, false);
        widget->clearMask();
        return;
    }

    const QRect bounds = widget->rect();
    if (bounds.isEmpty())
    {
        widget->setProperty(kDockLegacyRoundedMaskAppliedProperty, false);
        widget->clearMask();
        return;
    }

    const QRectF maskRect =
        QRectF(bounds).adjusted(0.0, 0.0, -kLegacyRoundedMaskFrameInset, -kLegacyRoundedMaskFrameInset);
    if (maskRect.isEmpty())
    {
        widget->setProperty(kDockLegacyRoundedMaskAppliedProperty, false);
        widget->clearMask();
        return;
    }

    QPainterPath path;
    path.addRoundedRect(maskRect, radius + kLegacyRoundedMaskFrameInset, radius + kLegacyRoundedMaskFrameInset);
    widget->setMask(QRegion(path.toFillPolygon().toPolygon()));
    widget->setProperty(kDockLegacyRoundedMaskAppliedProperty, true);
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
    widget->setProperty(kDockDwmFrameMarginsProperty,
                        QVariant::fromValue(QMargins(margins.cxLeftWidth,
                                                     margins.cyTopHeight,
                                                     margins.cxRightWidth,
                                                     margins.cyBottomHeight)));
    if (FAILED(extendFrame(hwnd, &margins)))
    {
        return false;
    }

    widget->setProperty(kDockDwmFrameApplyCountProperty, widget->property(kDockDwmFrameApplyCountProperty).toInt() + 1);
    widget->setProperty(kDockDwmFrameLastReasonProperty, QString::fromLatin1(reason ? reason : ""));
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

void queueDwmFrameRefresh(QWidget* widget, bool useNativeCaption, const char* reason)
{
    if (!widget || widget->property(kDockDwmFrameRefreshQueuedProperty).toBool())
    {
        return;
    }

    widget->setProperty(kDockDwmFrameRefreshQueuedProperty, true);
    QPointer<QWidget> guard(widget);
    QTimer::singleShot(0, widget, [guard, useNativeCaption, reason]() {
        if (!guard)
        {
            return;
        }
        guard->setProperty(kDockDwmFrameRefreshQueuedProperty, false);
        if (!guard->isVisible() || guard->isMaximized() || guard->isFullScreen())
        {
            return;
        }
        // The dock may have been embedded back into a manager between the
        // queue and the fire. In that case it is no longer a top-level
        // window and must not be re-extended into DWM frame tracking; in
        // particular, calling winId() on an embedded dock with
        // WA_NativeWindow=false would force-create a brand-new native
        // child HWND under the manager and re-enroll it in DWM, which
        // disturbs the host AntWindow's compositor cadence on Win10. Only
        // touch DWM state while we still own a top-level HWND that came
        // from the floating path. Use internalWinId() rather than winId()
        // so we never implicitly promote an embedded child.
        if (!guard->isWindow())
        {
            return;
        }
        if (auto* dock = qobject_cast<AntDockWidget*>(guard.data()))
        {
            if (!dock->isFloating())
            {
                return;
            }
        }
        const WId id = guard->internalWinId();
        if (!id)
        {
            return;
        }
        reapplyDwmFrameMargins(guard.data(), reinterpret_cast<HWND>(id), useNativeCaption, reason);
    });
}
#endif

class DockTitleButton : public QWidget
{
public:
    enum class Role
    {
        Minimize,
        Maximize,
        Close,
    };

    DockTitleButton(Role role, AntDockWidget* dock, QWidget* parent = nullptr)
        : QWidget(parent), m_role(role), m_dock(dock)
    {
        switch (m_role)
        {
        case Role::Minimize:
            setObjectName(QStringLiteral("AntDockTitleMinimizeButton"));
            break;
        case Role::Maximize:
            setObjectName(QStringLiteral("AntDockTitleMaximizeButton"));
            break;
        case Role::Close:
            setObjectName(QStringLiteral("AntDockTitleCloseButton"));
            break;
        }
        setFixedSize(kFloatingTitleButtonWidth, kFloatingTitleBarHeight);
        setCursor(Qt::ArrowCursor);
        setAttribute(Qt::WA_Hover, true);
        setMouseTracking(true);
    }

protected:
    bool event(QEvent* event) override
    {
        if (event->type() == QEvent::Enter)
        {
            m_hovered = true;
            update();
        }
        else if (event->type() == QEvent::Leave)
        {
            m_hovered = false;
            m_pressed = false;
            update();
        }
        return QWidget::event(event);
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            m_pressed = true;
            update();
            event->accept();
            return;
        }
        QWidget::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        const bool trigger = m_pressed && event->button() == Qt::LeftButton && rect().contains(event->pos());
        m_pressed = false;
        update();
        if (trigger && m_dock)
        {
            switch (m_role)
            {
            case Role::Minimize:
                m_dock->showMinimized();
                break;
            case Role::Maximize:
                m_dock->isMaximized() ? m_dock->showNormal() : m_dock->showMaximized();
                break;
            case Role::Close:
                if (m_dock->features().testFlag(QDockWidget::DockWidgetClosable))
                {
                    m_dock->close();
                }
                break;
            }
            event->accept();
            return;
        }
        QWidget::mouseReleaseEvent(event);
    }

    QPixmap cachedIconPixmap(const QString& iconName, const QColor& iconColor, qreal iconSize)
    {
        const qreal dpr = qMax<qreal>(1.0, devicePixelRatioF());
        const QString cacheKey = QStringLiteral("%1|%2|%3|%4")
                                     .arg(iconName,
                                          iconColor.name(QColor::HexArgb),
                                          QString::number(iconSize, 'f', 2),
                                          QString::number(dpr, 'f', 2));
        if (!m_iconPixmap.isNull() && m_iconCacheKey == cacheKey)
        {
            setProperty("antDockTitleButtonIconCacheHit", true);
            return m_iconPixmap;
        }

        m_iconPixmap = renderAntdIconPixmap(iconName, iconSize, iconColor, dpr);
        m_iconCacheKey = cacheKey;
        ++m_iconRenderCount;
        setProperty("antDockTitleButtonIconCacheHit", false);
        setProperty("antDockTitleButtonIconCacheKey", cacheKey);
        setProperty("antDockTitleButtonIconRenderCount", m_iconRenderCount);
        return m_iconPixmap;
    }

    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

        const bool destructive = m_role == Role::Close;
        if (m_hovered || m_pressed)
        {
            painter.setPen(Qt::NoPen);
            if (destructive && m_hovered)
            {
                painter.setBrush(token.colorError);
            }
            else
            {
                painter.setBrush(m_pressed ? token.colorFillSecondary : token.colorFillTertiary);
            }
            painter.drawRect(rect());
        }

        QString iconName;
        switch (m_role)
        {
        case Role::Minimize:
            iconName = QStringLiteral("MinusOutlined");
            break;
        case Role::Maximize:
            iconName = m_dock && m_dock->isMaximized()
                ? QStringLiteral("FullscreenExitOutlined")
                : QStringLiteral("FullscreenOutlined");
            break;
        case Role::Close:
            iconName = QStringLiteral("CloseOutlined");
            break;
        }

        const QColor iconColor = destructive && m_hovered ? QColor(Qt::white)
                                                          : (m_hovered ? token.colorTextSecondary : token.colorText);
        constexpr qreal iconSize = 14.0;
        const QRectF iconRect = centeredIconRect(rect(), iconSize);
        const QPixmap icon = cachedIconPixmap(iconName, iconColor, iconSize);
        if (!icon.isNull())
        {
            painter.drawPixmap(iconRect.topLeft(), icon);
        }
    }

private:
    Role m_role;
    AntDockWidget* m_dock = nullptr;
    bool m_hovered = false;
    bool m_pressed = false;
    QString m_iconCacheKey;
    QPixmap m_iconPixmap;
    int m_iconRenderCount = 0;
};

class DockTitleBar : public QWidget
{
public:
    explicit DockTitleBar(AntDockWidget* dock)
        : QWidget(dock), m_dock(dock)
    {
        setMouseTracking(true);
        setAttribute(Qt::WA_Hover, true);

        auto* layout = new QHBoxLayout(this);
        m_layout = layout;
        layout->setContentsMargins(12, 0, 0, 0);
        layout->setSpacing(8);

        m_iconLabel = new QLabel(this);
        m_iconLabel->setFixedSize(16, 16);
        m_iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        layout->addWidget(m_iconLabel);

        m_titleLabel = new QLabel(this);
        m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        QFont f = m_titleLabel->font();
        f.setBold(true);
        m_titleLabel->setFont(f);
        m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        layout->addWidget(m_titleLabel, 1);

        m_minimizeButton = new DockTitleButton(DockTitleButton::Role::Minimize, dock, this);
        m_maximizeButton = new DockTitleButton(DockTitleButton::Role::Maximize, dock, this);
        m_closeButton = new DockTitleButton(DockTitleButton::Role::Close, dock, this);
        layout->addWidget(m_minimizeButton);
        layout->addWidget(m_maximizeButton);
        layout->addWidget(m_closeButton);

        connect(antTheme, &AntTheme::themeChanged, this, [this]() {
            updateTheme();
            update();
        });

        updateFromDock();
        updateTheme();
    }

    void updateFromDock()
    {
        m_titleLabel->setText(m_dock->windowTitle());
        m_iconLabel->setPixmap(m_dock->windowIcon().pixmap(16, 16));
        updateChrome();
    }

    void updateTheme()
    {
        const auto& token = antTheme->tokens();
        const bool floating = m_dock->isFloating();
        const int titleFontSize = floating ? token.fontSizeLG : token.fontSize;
        const int leftMargin = floating ? 12 : 8;
        const int spacing = floating ? 8 : 4;
        if (m_themeInitialized &&
            m_cachedTextColor == token.colorText &&
            m_cachedTitleFontSize == titleFontSize &&
            m_cachedLeftMargin == leftMargin &&
            m_cachedSpacing == spacing)
        {
            return;
        }

        QPalette pal = palette();
        pal.setColor(QPalette::WindowText, token.colorText);
        pal.setColor(QPalette::ButtonText, token.colorText);
        pal.setColor(QPalette::Text, token.colorText);
        setPalette(pal);

        QPalette titlePalette = m_titleLabel->palette();
        titlePalette.setColor(QPalette::WindowText, token.colorText);
        titlePalette.setColor(QPalette::Text, token.colorText);
        m_titleLabel->setPalette(titlePalette);
        m_iconLabel->setPalette(titlePalette);

        QFont f = m_titleLabel->font();
        f.setPixelSize(titleFontSize);
        f.setWeight(QFont::DemiBold);
        m_titleLabel->setFont(f);
        if (m_layout)
        {
            m_layout->setContentsMargins(leftMargin, 0, 0, 0);
            m_layout->setSpacing(spacing);
        }
        m_themeInitialized = true;
        m_cachedTextColor = token.colorText;
        m_cachedTitleFontSize = titleFontSize;
        m_cachedLeftMargin = leftMargin;
        m_cachedSpacing = spacing;
        ++m_themeApplyCount;
        setProperty("antDockTitleBarThemeApplyCount", m_themeApplyCount);
        update();
    }

    void updateChrome()
    {
        const bool floating = m_dock->isFloating();
        const bool closable = m_dock->features().testFlag(QDockWidget::DockWidgetClosable);
        const QSize closeButtonSize = floating ? QSize(kFloatingTitleButtonWidth, kFloatingTitleBarHeight)
                                               : QSize(26, kEmbeddedTitleBarHeight);
        const int titleBarHeight = floating ? kFloatingTitleBarHeight : kEmbeddedTitleBarHeight;
        if (m_chromeInitialized &&
            m_cachedChromeFloating == floating &&
            m_cachedChromeClosable == closable &&
            m_cachedCloseButtonSize == closeButtonSize &&
            m_cachedTitleBarHeight == titleBarHeight)
        {
            return;
        }

        m_minimizeButton->setVisible(floating);
        m_maximizeButton->setVisible(floating);
        m_closeButton->setVisible(closable);
        m_closeButton->setEnabled(closable);
        m_closeButton->setFixedSize(closeButtonSize);
        setProperty("antDockTitleBarHeight", titleBarHeight);
        m_chromeInitialized = true;
        m_cachedChromeFloating = floating;
        m_cachedChromeClosable = closable;
        m_cachedCloseButtonSize = closeButtonSize;
        m_cachedTitleBarHeight = titleBarHeight;
        ++m_chromeApplyCount;
        setProperty("antDockTitleBarChromeApplyCount", m_chromeApplyCount);
        updateTheme();
        updateGeometry();
        update();
    }

    QSize sizeHint() const override
    {
        return QSize(240, m_dock->isFloating() ? kFloatingTitleBarHeight : kEmbeddedTitleBarHeight);
    }

    QSize minimumSizeHint() const override
    {
        return QSize(96, m_dock->isFloating() ? kFloatingTitleBarHeight : kEmbeddedTitleBarHeight);
    }

protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            event->accept();
            return;
        }
        QWidget::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (event->buttons() & Qt::LeftButton)
        {
            event->accept();
            return;
        }
        QWidget::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            event->accept();
            return;
        }
        QWidget::mouseReleaseEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            if (m_dock->isFloating())
            {
                m_dock->isMaximized() ? m_dock->showNormal() : m_dock->showMaximized();
            }
            else
            {
                if (m_dock->features().testFlag(QDockWidget::DockWidgetFloatable))
                {
                    m_dock->setFloating(true);
                }
            }
            event->accept();
            return;
        }
        QWidget::mouseDoubleClickEvent(event);
    }

    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        const bool floating = m_dock->isFloating();
        const bool maximized = m_dock->isMaximized();
        const QColor bg = floating ? token.colorBgElevated : token.colorBgContainer;

        const bool legacyLiveResize = m_dock->property(kDockLegacyLiveResizeProperty).toBool();
        if (floating && !maximized && !legacyLiveResize)
        {
            QPainterPath titlePath;
            titlePath.addRoundedRect(QRectF(rect()), kFloatingCornerRadius, kFloatingCornerRadius);
            titlePath.addRect(QRectF(0, kFloatingCornerRadius, width(), qMax(0, height() - kFloatingCornerRadius)));
            p.fillPath(titlePath, bg);
        }
        else
        {
            p.fillRect(rect(), bg);
        }

        p.setPen(QPen(token.colorSplit, 1));
        p.drawLine(rect().bottomLeft(), rect().bottomRight());
    }

private:
    AntDockWidget* m_dock;
    QHBoxLayout* m_layout = nullptr;
    QLabel* m_iconLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    DockTitleButton* m_minimizeButton = nullptr;
    DockTitleButton* m_maximizeButton = nullptr;
    DockTitleButton* m_closeButton = nullptr;
    bool m_themeInitialized = false;
    QColor m_cachedTextColor;
    int m_cachedTitleFontSize = -1;
    int m_cachedLeftMargin = -1;
    int m_cachedSpacing = -1;
    int m_themeApplyCount = 0;
    bool m_chromeInitialized = false;
    bool m_cachedChromeFloating = false;
    bool m_cachedChromeClosable = false;
    QSize m_cachedCloseButtonSize;
    int m_cachedTitleBarHeight = -1;
    int m_chromeApplyCount = 0;
};

} // namespace

AntDockWidget::AntDockWidget(QWidget* parent, Qt::WindowFlags flags)
    : QDockWidget(parent, flags)
{
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable |
                QDockWidget::DockWidgetFloatable);
    setupTitleBar();
    updateTheme();

    connect(this, &QDockWidget::topLevelChanged, this, [this](bool) {
        updateFloatingFrame();
#if defined(Q_OS_WIN)
        applyNativeWindowFrame();
        updateLegacySoftwareShadow();
#endif
        update();
    });

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateTheme();
#if defined(Q_OS_WIN)
        if (isFloating())
        {
            applyNativeWindowFrame();
            updateLegacySoftwareShadow();
        }
#endif
        update();
    });
}

AntDockWidget::AntDockWidget(const QString& title, QWidget* parent, Qt::WindowFlags flags)
    : AntDockWidget(parent, flags)
{
    setWindowTitle(title);
}

void AntDockWidget::setupTitleBar()
{
    auto* bar = new DockTitleBar(this);
    setTitleBarWidget(bar);

    connect(this, &QDockWidget::windowTitleChanged, bar, &DockTitleBar::updateFromDock);
    connect(this, &QDockWidget::windowIconChanged, bar, [bar](const QIcon&) { bar->updateFromDock(); });
    connect(this, &QDockWidget::topLevelChanged, bar, [bar](bool) { bar->updateChrome(); });
    connect(this, &QDockWidget::featuresChanged, bar, [bar](QDockWidget::DockWidgetFeatures) {
        bar->updateChrome();
    });
}

void AntDockWidget::setWidget(QWidget* widget)
{
    QDockWidget::setWidget(widget);
    updateTheme();
}

#if defined(Q_OS_WIN)
void AntDockWidget::resetNativeFloatingWindowForEmbedding()
{
    if (const WId dockId = internalWinId())
    {
        HWND dockHwnd = reinterpret_cast<HWND>(dockId);
        ::ShowWindow(dockHwnd, SW_HIDE);
        ::SetWindowPos(dockHwnd,
                       nullptr,
                       0,
                       0,
                       0,
                       0,
                       SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                           SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
    }
    hideLegacySoftwareShadow();
    // The Win10 legacy frame path is the only one that ever spawns a software
    // shadow (Win11 keeps `useNativeCaption=true` and skips creation). Hiding
    // the shadow is not enough on Win10: the widget keeps a top-level layered
    // window enrolled in DWM tracking (`WS_EX_LAYERED | WS_EX_TRANSPARENT |
    // WS_EX_NOACTIVATE`). Leaving that hidden layered HWND alive after the
    // dock re-embeds disturbs the host AntWindow's compositor cadence, so
    // user-visible animations / hover / click repaints stutter. Fully delete
    // the shadow widget here; the next float will lazily recreate a fresh one.
    if (m_legacySoftwareShadow)
    {
        delete m_legacySoftwareShadow;
        m_legacySoftwareShadow = nullptr;
        setProperty(kDockLegacyShadowEnabledProperty, false);
    }
    // Clear live-resize and DWM frame bookkeeping immediately. Without this the
    // dock re-enters the embedded tab area while properties still claim it
    // owns DWM-extended frame margins; any subsequent code path that consults
    // `kDockNativeFrameEnabledProperty` (e.g. queued frame refresh fired from
    // before the float-back) would happily re-enroll the freshly embedded
    // child in DWM, recreating exactly the residual layered HWND the embed
    // path is trying to evict.
    m_legacyLiveResize = false;
    setProperty(kDockLegacyLiveResizeProperty, false);
    setProperty(kDockNativeFrameEnabledProperty, false);
    setProperty(kDockUsesNativeCaptionFrameProperty, false);
    setProperty(kDockDwmFrameRefreshQueuedProperty, false);
    // Destroy child native handles as well. A floating dock may contain child
    // HWNDs created while it was top-level; keeping them alive across reparent
    // can leave stale native children above the embedded layout.
    destroy(true, true);
    setProperty("antDockNativeFloatingHwndDestroyed", true);
    setProperty("antDockLegacyShadowDestroyedOnEmbed", true);
}
#endif

void AntDockWidget::updateTheme()
{
    const auto& token = antTheme->tokens();
    QPalette pal = palette();
    pal.setColor(QPalette::Window, token.colorBgContainer);
    pal.setColor(QPalette::Base, token.colorBgContainer);
    pal.setColor(QPalette::AlternateBase, token.colorFillQuaternary);
    pal.setColor(QPalette::Button, token.colorBgContainer);
    pal.setColor(QPalette::WindowText, token.colorText);
    pal.setColor(QPalette::Text, token.colorText);
    pal.setColor(QPalette::ButtonText, token.colorText);
    pal.setColor(QPalette::PlaceholderText, token.colorTextTertiary);
    setPalette(pal);

    if (QWidget* content = widget())
    {
        QPalette contentPalette = content->palette();
        contentPalette.setColor(QPalette::Window, token.colorBgContainer);
        contentPalette.setColor(QPalette::Base, token.colorBgContainer);
        contentPalette.setColor(QPalette::AlternateBase, token.colorFillQuaternary);
        contentPalette.setColor(QPalette::Button, token.colorBgContainer);
        contentPalette.setColor(QPalette::WindowText, token.colorText);
        contentPalette.setColor(QPalette::Text, token.colorText);
        contentPalette.setColor(QPalette::ButtonText, token.colorText);
        contentPalette.setColor(QPalette::PlaceholderText, token.colorTextTertiary);
        content->setPalette(contentPalette);
        content->setAutoFillBackground(true);
    }
    setProperty("antDockDarkSurfaceApplied", antTheme->themeMode() == Ant::ThemeMode::Dark);
}

void AntDockWidget::updateFloatingFrame()
{
    const bool floating = isFloating() && !property("antDockEmbeddedByManager").toBool();
    const int shadowMargin = 0;
    const int cornerRadius = floating ? floatingCornerRadius() : 0;
    setProperty("antDockFloatingFrame", floating);
    setProperty("antDockFloatingShadowMargin", shadowMargin);
    setProperty("antDockFloatingCornerRadius", cornerRadius);
    setProperty("antDockFloatingTitleBarHeight", floating ? kFloatingTitleBarHeight : 0);

    if (floating)
    {
        setAttribute(Qt::WA_TranslucentBackground, false);
        setAttribute(Qt::WA_NoSystemBackground, false);
        setAttribute(Qt::WA_OpaquePaintEvent, true);
        setAutoFillBackground(false);
        setContentsMargins(0, 0, 0, 0);

        Qt::WindowFlags wantedFlags = windowFlags();
        wantedFlags &= ~Qt::WindowType_Mask;
        wantedFlags |= Qt::Window;
        wantedFlags |= Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint;
        wantedFlags &= ~Qt::WindowTitleHint;
        if (windowFlags() != wantedFlags)
        {
            const bool wasVisible = isVisible();
            const QRect oldGeometry = geometry();
            setWindowFlags(wantedFlags);
            setGeometry(oldGeometry);
            if (wasVisible)
            {
                show();
            }
        }
    }
    else
    {
        setAttribute(Qt::WA_TranslucentBackground, false);
        setAttribute(Qt::WA_NoSystemBackground, false);
        setAttribute(Qt::WA_OpaquePaintEvent, false);
        setContentsMargins(0, 0, 0, 0);
        setProperty(kDockLegacyLiveResizeProperty, false);
#if defined(Q_OS_WIN)
        m_legacyLiveResize = false;
        hideLegacySoftwareShadow();
        setProperty(kDockNativeFrameEnabledProperty, false);
        setProperty(kDockUsesNativeCaptionFrameProperty, false);
        setProperty(kDockLegacyRoundedMaskAppliedProperty, false);
        clearMask();
#endif
    }

    if (m_floatingFrameActive != floating)
    {
        m_floatingFrameActive = floating;
        updateGeometry();
    }
    if (auto* bar = dynamic_cast<DockTitleBar*>(titleBarWidget()))
    {
        bar->updateChrome();
    }
    update();
}

QRect AntDockWidget::floatingPanelRect() const
{
    const int margin = floatingShadowMargin();
    return rect().adjusted(margin, margin, -margin, -margin);
}

int AntDockWidget::floatingShadowMargin() const
{
    return 0;
}

int AntDockWidget::floatingCornerRadius() const
{
    return isMaximized() ? 0 : kFloatingCornerRadius;
}

bool AntDockWidget::event(QEvent* event)
{
#if defined(Q_OS_WIN)
    if (isFloating() &&
        (event->type() == QEvent::ScreenChangeInternal || antIsDevicePixelRatioChangeEvent(event->type())))
    {
        if (QWindow* shadowWindow = m_legacySoftwareShadow ? m_legacySoftwareShadow->windowHandle() : nullptr)
        {
            if (QScreen* hostScreen = windowHandle() ? windowHandle()->screen() : nullptr)
            {
                if (shadowWindow->screen() != hostScreen)
                {
                    shadowWindow->setScreen(hostScreen);
                }
            }
        }
        applyNativeWindowFrame();
        updateLegacySoftwareShadow();
        update();
    }
#endif
    return QDockWidget::event(event);
}

void AntDockWidget::paintEvent(QPaintEvent* event)
{
    if (!isFloating())
    {
        if (property("antDockEmbeddedByManager").toBool())
        {
            Q_UNUSED(event)
            QPainter painter(this);
            painter.fillRect(rect(), antTheme->tokens().colorBgContainer);
            return;
        }
        QDockWidget::paintEvent(event);
        return;
    }

    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const QRect panel = floatingPanelRect();
    if (panel.isEmpty())
    {
        return;
    }

    const int cornerRadius = floatingCornerRadius();
    const auto& token = antTheme->tokens();
    const bool dark = antTheme->themeMode() == Ant::ThemeMode::Dark;
    QColor fill = token.colorBgElevated;
    QColor border = dark ? token.colorBorder : token.colorBorderSecondary;
    border.setAlphaF(dark ? 0.82 : 0.72);

    const bool liveResize = property(kDockLegacyLiveResizeProperty).toBool();
    if (liveResize)
    {
        // Mid-drag: fill the entire client rect with the panel colour and
        // skip the 1px border. A pen-stroked border at QRectF(panel).adjusted
        // (0.5, 0.5, -0.5, -0.5) lags one frame behind WM_SIZE — the old
        // edge pixels are still in the backing store while the new edge has
        // already moved inward, producing the flickering edge lines. The
        // border is restored on WM_EXITSIZEMOVE.
        painter.fillRect(rect(), fill);
        return;
    }

    const QRectF panelRect = QRectF(panel).adjusted(0.5, 0.5, -0.5, -0.5);
    painter.setPen(QPen(border, kFloatingBorderWidth));
    painter.setBrush(fill);
    if (cornerRadius > 0)
    {
        painter.drawRoundedRect(panelRect, cornerRadius, cornerRadius);
    }
    else
    {
        painter.drawRect(panelRect);
    }
}

void AntDockWidget::moveEvent(QMoveEvent* event)
{
    QDockWidget::moveEvent(event);
#if defined(Q_OS_WIN)
    if (isFloating() && !m_legacyLiveResize)
    {
        updateLegacySoftwareShadow();
    }
#endif
}

void AntDockWidget::hideEvent(QHideEvent* event)
{
#if defined(Q_OS_WIN)
    hideLegacySoftwareShadow();
#endif
    QDockWidget::hideEvent(event);
}

void AntDockWidget::resizeEvent(QResizeEvent* event)
{
    QDockWidget::resizeEvent(event);
    if (isFloating())
    {
#if defined(Q_OS_WIN)
        if (m_legacyLiveResize && !supportsNativeCaptionFrame(this))
        {
            // Drop the rounded mask entirely while the user is dragging an
            // edge — keeping a mask in sync with WM_SIZE is the principal
            // cause of edge-flicker, because SetWindowRgn is asynchronous
            // w.r.t. the WM_SIZE-triggered backing-store paint. The dock
            // already paints a square outline during live resize (see
            // paintEvent + kDockLegacyLiveResizeProperty), so removing the
            // mask is a no-op visually but keeps the entire client area
            // continuously opaque.
            setProperty(kDockLegacyRoundedMaskAppliedProperty, false);
            clearMask();
            hideLegacySoftwareShadow();
            refreshFloatingDockWindow(this, "legacy-live-resize");
        }
        else
        {
            applyNativeWindowFrame();
            updateLegacySoftwareShadow();
            if (!supportsNativeCaptionFrame(this) &&
                (event->size().width() > event->oldSize().width() ||
                 event->size().height() > event->oldSize().height()))
            {
                refreshFloatingDockWindow(this, "legacy-resize-grow");
            }
        }
#endif
        update();
    }
}

void AntDockWidget::showEvent(QShowEvent* event)
{
    updateFloatingFrame();
    QDockWidget::showEvent(event);
#if defined(Q_OS_WIN)
    if (isFloating())
    {
        applyNativeWindowFrame();
        QPointer<AntDockWidget> guard(this);
        QTimer::singleShot(16, this, [guard]() {
            if (guard && guard->isFloating() && !guard->property(kDockLegacyLiveResizeProperty).toBool())
            {
                guard->updateLegacySoftwareShadow();
            }
        });
    }
#endif
}

void AntDockWidget::changeEvent(QEvent* event)
{
    QDockWidget::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange && isFloating())
    {
        updateFloatingFrame();
#if defined(Q_OS_WIN)
        if (!m_legacyLiveResize)
        {
            applyNativeWindowFrame();
            updateLegacySoftwareShadow();
            if (!supportsNativeCaptionFrame(this))
            {
                refreshFloatingDockWindow(this, isMaximized() ? "legacy-maximized" : "legacy-state");
            }
        }
#endif
        if (QWidget* bar = titleBarWidget())
        {
            bar->update();
        }
    }
}

#if defined(Q_OS_WIN)
void AntDockWidget::applyNativeWindowFrame()
{
    if (!isFloating() || !windowHandle())
    {
        return;
    }

    const HWND hwnd = reinterpret_cast<HWND>(winId());
    if (!hwnd)
    {
        return;
    }

    const bool useNativeCaption = supportsNativeCaptionFrame(this);
    ensureNativeWindowStyle(hwnd, useNativeCaption);
    applyLegacyClassDropShadow(this, hwnd, useNativeCaption);
    if (m_legacyLiveResize && !useNativeCaption)
    {
        // Mid-drag: keep the mask cleared so the entire client area stays
        // opaque and edges do not flash. The rounded outline is restored on
        // WM_EXITSIZEMOVE via the normal applyNativeWindowFrame path.
        setProperty(kDockLegacyRoundedMaskAppliedProperty, false);
        clearMask();
        setProperty(kDockUsesNativeCaptionFrameProperty, useNativeCaption);
        setProperty(kDockLegacyLiveResizeProperty, true);
        return;
    }
    applyLegacyRoundedMask(this, floatingCornerRadius(), useNativeCaption);
    setProperty(kDockNativeFrameEnabledProperty, true);
    setProperty(kDockUsesNativeCaptionFrameProperty, useNativeCaption);

    DwmSetWindowAttributeFn setWindowAttribute = nullptr;
    DwmExtendFrameIntoClientAreaFn extendFrame = nullptr;
    if (!resolveDwmApis(&setWindowAttribute, &extendFrame))
    {
        return;
    }

    if (extendFrame)
    {
        applyDwmFrameMargins(this, hwnd, useNativeCaption, extendFrame, "frame");
        queueDwmFrameRefresh(this, useNativeCaption, "frame-deferred");
    }

    if (!setWindowAttribute)
    {
        return;
    }

    const int cornerPreference = [this]() {
        if (isMaximized() || isFullScreen() || floatingCornerRadius() <= 0)
        {
            return kDwmCornerDoNotRound;
        }
        if (floatingCornerRadius() <= 6)
        {
            return kDwmCornerRoundSmall;
        }
        return kDwmCornerRound;
    }();
    setWindowAttribute(hwnd, kDwmWindowCornerPreference, &cornerPreference, sizeof(cornerPreference));

    const auto& token = antTheme->tokens();
    const COLORREF borderColor = RGB(token.colorBorder.red(), token.colorBorder.green(), token.colorBorder.blue());
    setWindowAttribute(hwnd, kDwmBorderColor, &borderColor, sizeof(borderColor));

    const BOOL darkMode = antTheme->themeMode() == Ant::ThemeMode::Dark;
    setWindowAttribute(hwnd, kDwmUseImmersiveDarkMode, &darkMode, sizeof(darkMode));
}

void AntDockWidget::updateLegacySoftwareShadow()
{
    const bool useNativeCaption = property(kDockUsesNativeCaptionFrameProperty).toBool();
    const bool enabled = isFloating()
        && isVisible()
        && windowHandle()
        && !m_legacyLiveResize
        && !useNativeCaption
        && !isMaximized()
        && !isFullScreen()
        && !isMinimized();

    setProperty(kDockLegacyShadowEnabledProperty, enabled);
    setProperty(kDockLegacyLiveResizeProperty, m_legacyLiveResize);
    if (!enabled)
    {
        hideLegacySoftwareShadow();
        return;
    }

    if (!m_legacySoftwareShadow)
    {
        auto* shadow = new AntDockLegacySoftwareShadow(this);
        m_legacySoftwareShadow = shadow;
        connect(antTheme, &AntTheme::themeChanged, shadow, qOverload<>(&QWidget::update));
    }

    auto* shadow = static_cast<AntDockLegacySoftwareShadow*>(m_legacySoftwareShadow);
    if (shadow)
    {
        shadow->setCornerRadius(floatingCornerRadius());
    }

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

    const QRect shadowGeometry = geometry().adjusted(-kNativeFrameShadowMargin,
                                                     -kNativeFrameShadowMargin,
                                                     kNativeFrameShadowMargin,
                                                     kNativeFrameShadowMargin);
    m_legacySoftwareShadow->setGeometry(shadowGeometry);
    setProperty(kDockLegacyShadowGeometryProperty, shadowGeometry);
    setProperty(kDockLegacyShadowGeometryModeProperty, QStringLiteral("qt-logical"));
    m_legacySoftwareShadow->setProperty(kDockLegacyShadowGeometryProperty, shadowGeometry);
    m_legacySoftwareShadow->setProperty(kDockLegacyShadowGeometryModeProperty, QStringLiteral("qt-logical"));
    if (!m_legacySoftwareShadow->isVisible())
    {
        m_legacySoftwareShadow->show();
    }
    m_legacySoftwareShadow->setProperty(kDockLegacyShadowDprProperty, m_legacySoftwareShadow->devicePixelRatioF());

    const HWND hwnd = reinterpret_cast<HWND>(winId());
    const HWND shadowHwnd = reinterpret_cast<HWND>(m_legacySoftwareShadow->winId());
    if (shadowHwnd && hwnd)
    {
        makeWindowClickThrough(m_legacySoftwareShadow);
        applyShadowWindowRingRegion(m_legacySoftwareShadow, kNativeFrameShadowMargin, floatingCornerRadius());
        // shadowGeometry is a Qt logical-pixel QRect. SetWindowPos() uses
        // native physical pixels in a per-monitor-DPI process, so Qt owns the
        // move/size conversion above; the native call only keeps z-order and
        // visibility in sync with the floating dock owner.
        ::SetWindowPos(shadowHwnd,
                       hwnd,
                       0,
                       0,
                       0,
                       0,
                       SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
    }
}

void AntDockWidget::hideLegacySoftwareShadow()
{
    setProperty(kDockLegacyShadowEnabledProperty, false);
    if (m_legacySoftwareShadow)
    {
#if defined(Q_OS_WIN)
        if (const WId shadowId = m_legacySoftwareShadow->internalWinId())
        {
            HWND shadowHwnd = reinterpret_cast<HWND>(shadowId);
            makeWindowClickThrough(m_legacySoftwareShadow);
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

bool AntDockWidget::nativeEvent(const QByteArray& eventType, void* message, AntNativeEventResult* result)
{
    if (!isFloating() || (eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG"))
    {
        return QDockWidget::nativeEvent(eventType, message, result);
    }

    auto* msg = static_cast<MSG*>(message);
    const HWND hwnd = msg->hwnd;
    const UINT uMsg = msg->message;

    switch (uMsg)
    {
    case WM_DPICHANGED:
    {
        QPointer<AntDockWidget> guard(this);
        QTimer::singleShot(0, this, [guard]() {
            if (!guard || !guard->isFloating())
            {
                return;
            }
            if (QWindow* shadowWindow = guard->m_legacySoftwareShadow
                                            ? guard->m_legacySoftwareShadow->windowHandle()
                                            : nullptr)
            {
                if (QScreen* hostScreen = guard->windowHandle() ? guard->windowHandle()->screen() : nullptr)
                {
                    if (shadowWindow->screen() != hostScreen)
                    {
                        shadowWindow->setScreen(hostScreen);
                    }
                }
            }
            guard->applyNativeWindowFrame();
            guard->updateLegacySoftwareShadow();
            guard->update();
        });
        break;
    }
    case WM_ENTERSIZEMOVE:
        if (!supportsNativeCaptionFrame(this))
        {
            m_legacyLiveResize = true;
            setProperty(kDockLegacyLiveResizeProperty, true);
            hideLegacySoftwareShadow();
            update();
        }
        break;
    case WM_SIZING:
        if (!supportsNativeCaptionFrame(this) && !m_legacyLiveResize)
        {
            m_legacyLiveResize = true;
            setProperty(kDockLegacyLiveResizeProperty, true);
            hideLegacySoftwareShadow();
            update();
        }
        break;
    case WM_EXITSIZEMOVE:
        if (m_legacyLiveResize)
        {
            m_legacyLiveResize = false;
            setProperty(kDockLegacyLiveResizeProperty, false);
            QPointer<AntDockWidget> guard(this);
            QTimer::singleShot(16, this, [guard]() {
                if (!guard || !guard->isFloating())
                {
                    return;
                }
                guard->applyNativeWindowFrame();
                guard->updateLegacySoftwareShadow();
                if (!supportsNativeCaptionFrame(guard.data()))
                {
                    refreshFloatingDockWindow(guard.data(), "legacy-live-resize-finished");
                }
                guard->update();
            });
        }
        break;
    case WM_ERASEBKGND:
        fillNativeDockEraseBackground(this, msg->wParam);
        *result = 1;
        return true;
    case WM_NCCALCSIZE:
        *result = 0;
        return true;
    case WM_NCHITTEST:
    {
        const QPoint widgetPos = nativeMessageLocalPoint(hwnd, msg->lParam, devicePixelRatioF());
        const int clientWidth = width();
        const int clientHeight = height();
        if (!supportsNativeCaptionFrame(this) &&
            (widgetPos.x() < 0 || widgetPos.y() < 0 || widgetPos.x() >= clientWidth || widgetPos.y() >= clientHeight))
        {
            *result = HTTRANSPARENT;
            setProperty("antDockLegacyOutsideClientHitTestTransparent", true);
            return true;
        }

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

        *result = HTCLIENT;
        return true;
    }
    case WM_GETMINMAXINFO:
    {
        auto* minmaxInfo = reinterpret_cast<MINMAXINFO*>(msg->lParam);
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
    default:
        break;
    }

    return QDockWidget::nativeEvent(eventType, message, result);
}
#endif
