#include "AntWindowFrame.h"

#include <QByteArray>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPointer>
#include <QScreen>
#include <QShowEvent>
#include <QTimer>
#include <QVariant>
#include <QWindow>

#include "core/AntTheme.h"

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace
{
constexpr int kLegacyRoundedMaskFrameInset = 1;

#ifdef Q_OS_WIN
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
constexpr COLORREF kDwmBorderColorNone = 0xFFFFFFFEu;
constexpr int kDwmCornerDoNotRound = 1;
constexpr int kDwmCornerRound = 2;
constexpr int kDwmCornerRoundSmall = 3;

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

DwmMargins shadowPreservingDwmMargins(bool useNativeCaption)
{
    return useNativeCaption ? DwmMargins{1, 1, 1, 1} : DwmMargins{0, 0, 0, 0};
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

void ensureNativeWindowStyle(HWND hwnd, const AntWindowFrame::NativeFrameOptions& options, bool useNativeCaption)
{
    LONG_PTR style = ::GetWindowLongPtrW(hwnd, GWL_STYLE);
    LONG_PTR baseStyle = WS_THICKFRAME | WS_SYSMENU;
    if (options.enableMaximizeBox)
    {
        baseStyle |= WS_MAXIMIZEBOX;
    }
    if (options.enableMinimizeBox)
    {
        baseStyle |= WS_MINIMIZEBOX;
    }

    LONG_PTR newStyle = style | baseStyle;
    if (!options.enableMaximizeBox)
    {
        newStyle &= ~WS_MAXIMIZEBOX;
    }
    if (!options.enableMinimizeBox)
    {
        newStyle &= ~WS_MINIMIZEBOX;
    }
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

void applyLegacyClassDropShadow(QWidget* widget,
                                HWND hwnd,
                                bool useNativeCaption,
                                const AntWindowFrame::NativeFrameOptions& options)
{
    if (!widget)
    {
        return;
    }

    if (!hwnd)
    {
        if (options.legacyClassDropShadowEnabledProperty)
        {
            widget->setProperty(options.legacyClassDropShadowEnabledProperty, false);
        }
        return;
    }

    const LONG_PTR classStyle = ::GetClassLongPtrW(hwnd, GCL_STYLE);
    const LONG_PTR newClassStyle = useNativeCaption ? classStyle : (classStyle & ~static_cast<LONG_PTR>(CS_DROPSHADOW));
    if (newClassStyle != classStyle)
    {
        ::SetClassLongPtrW(hwnd, GCL_STYLE, newClassStyle);
    }
    if (options.legacyClassDropShadowEnabledProperty)
    {
        widget->setProperty(options.legacyClassDropShadowEnabledProperty,
                            (::GetClassLongPtrW(hwnd, GCL_STYLE) & CS_DROPSHADOW) == CS_DROPSHADOW);
    }
}

void applyLegacyRoundedMask(QWidget* widget, int radius, const AntWindowFrame::NativeFrameOptions& options)
{
    Q_UNUSED(radius)
    if (!widget)
    {
        return;
    }
    if (options.legacyRoundedMaskFrameInsetProperty)
    {
        widget->setProperty(options.legacyRoundedMaskFrameInsetProperty, kLegacyRoundedMaskFrameInset);
    }
    if (options.legacyRoundedMaskAppliedProperty)
    {
        widget->setProperty(options.legacyRoundedMaskAppliedProperty, false);
    }
    widget->clearMask();
}

bool applyDwmFrameMargins(QWidget* widget,
                          HWND hwnd,
                          bool useNativeCaption,
                          DwmExtendFrameIntoClientAreaFn extendFrame,
                          const AntWindowFrame::NativeFrameOptions& options,
                          const char* reason)
{
    if (!widget || !hwnd || !extendFrame)
    {
        return false;
    }

    const DwmMargins margins = shadowPreservingDwmMargins(useNativeCaption);
    if (options.dwmFrameMarginsProperty)
    {
        widget->setProperty(options.dwmFrameMarginsProperty,
                            QVariant::fromValue(QMargins(margins.cxLeftWidth,
                                                         margins.cyTopHeight,
                                                         margins.cxRightWidth,
                                                         margins.cyBottomHeight)));
    }
    if (FAILED(extendFrame(hwnd, &margins)))
    {
        return false;
    }

    if (options.dwmFrameApplyCountProperty)
    {
        widget->setProperty(options.dwmFrameApplyCountProperty,
                            widget->property(options.dwmFrameApplyCountProperty).toInt() + 1);
    }
    if (options.dwmFrameLastReasonProperty)
    {
        widget->setProperty(options.dwmFrameLastReasonProperty, QString::fromLatin1(reason ? reason : ""));
    }
    return true;
}

bool makeNativeInputTransparent(QWidget* widget, const char* propertyName)
{
    if (!widget)
    {
        return false;
    }

    const HWND hwnd = reinterpret_cast<HWND>(widget->winId());
    if (!hwnd)
    {
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
                       SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE |
                           SWP_FRAMECHANGED);
    }

    if (propertyName)
    {
        widget->setProperty(propertyName, true);
    }
    return true;
}

class LegacySoftwareShadow : public QWidget
{
public:
    LegacySoftwareShadow(QWidget* owner, const QString& objectName, const char* clickThroughProperty)
        : QWidget(owner, Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowDoesNotAcceptFocus),
          m_clickThroughProperty(clickThroughProperty)
    {
        setObjectName(objectName);
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setAttribute(Qt::WA_ShowWithoutActivating, true);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        setWindowFlag(Qt::WindowTransparentForInput, true);
#endif
        setFocusPolicy(Qt::NoFocus);
        setProperty("shadowMargin", AntWindowFrame::LegacySoftwareShadowMargin);
        setProperty("shadowInnerClearance", AntWindowFrame::LegacySoftwareShadowInnerClearance);
        if (m_clickThroughProperty)
        {
            setProperty(m_clickThroughProperty, false);
        }
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
        makeNativeInputTransparent(this, m_clickThroughProperty);
    }

    bool nativeEvent(const QByteArray& eventType, void* message, AntNativeEventResult* result) override
    {
        if ((eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") && message)
        {
            auto* msg = static_cast<MSG*>(message);
            if (msg->message == WM_NCHITTEST)
            {
                *result = HTTRANSPARENT;
                if (m_clickThroughProperty)
                {
                    setProperty(m_clickThroughProperty, true);
                }
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

        const int shadowWidth = AntWindowFrame::LegacySoftwareShadowMargin;
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
        const qreal effectiveSpread =
            qMax<qreal>(1.0, shadowWidth - AntWindowFrame::LegacySoftwareShadowInnerClearance);
        for (int distance = shadowWidth; distance > AntWindowFrame::LegacySoftwareShadowInnerClearance; --distance)
        {
            const qreal t = qBound<qreal>(
                0.0,
                1.0 - static_cast<qreal>(distance - AntWindowFrame::LegacySoftwareShadowInnerClearance - 1) /
                          effectiveSpread,
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
            const int innerDistance = qMax(AntWindowFrame::LegacySoftwareShadowInnerClearance, distance - 1);
            const qreal innerRadiusGrowth = static_cast<qreal>(innerDistance) * 0.55;
            const QRectF inner = panelRect.adjusted(-innerDistance, -innerDistance, innerDistance, innerDistance);
            innerPath.addRoundedRect(inner, m_cornerRadius + innerRadiusGrowth, m_cornerRadius + innerRadiusGrowth);

            painter.fillPath(outerPath.subtracted(innerPath), shadow);
        }
    }

private:
    int m_cornerRadius = 0;
    const char* m_clickThroughProperty = nullptr;
};
#endif
} // namespace

namespace AntWindowFrame
{
bool legacyFramePolicyEnabled(const QWidget* widget, const char* forcePropertyName)
{
    if (widget && forcePropertyName && widget->property(forcePropertyName).toBool())
    {
        return true;
    }

    const QByteArray policy = qgetenv("QT_ANT_DESIGN_FORCE_LEGACY_FRAME").trimmed().toLower();
    return policy == "1" || policy == "true" || policy == "yes" || policy == "on";
}

bool supportsNativeCaptionFrame()
{
#ifdef Q_OS_WIN
    constexpr int kWindows11Build = 22000;
    return windowsBuildNumber() >= kWindows11Build;
#else
    return true;
#endif
}

bool usesNativeCaptionFrameForWidget(const QWidget* widget, const char* forcePropertyName)
{
#ifdef Q_OS_WIN
    return supportsNativeCaptionFrame() && !legacyFramePolicyEnabled(widget, forcePropertyName);
#else
    return !legacyFramePolicyEnabled(widget, forcePropertyName);
#endif
}

void applyNativeFrame(QWidget* widget, const NativeFrameOptions& options)
{
#ifdef Q_OS_WIN
    if (!widget || !widget->windowHandle())
    {
        return;
    }

    const HWND hwnd = reinterpret_cast<HWND>(widget->winId());
    if (!hwnd)
    {
        return;
    }

    const bool useNativeCaption = options.enableNativeCaption &&
        usesNativeCaptionFrameForWidget(widget, options.forceLegacyFramePolicyProperty);
    ensureNativeWindowStyle(hwnd, options, useNativeCaption);
    applyLegacyClassDropShadow(widget, hwnd, useNativeCaption, options);
    applyLegacyRoundedMask(widget, options.cornerRadius, options);
    if (options.usesNativeCaptionFrameProperty)
    {
        widget->setProperty(options.usesNativeCaptionFrameProperty, useNativeCaption);
    }

    DwmSetWindowAttributeFn setWindowAttribute = nullptr;
    DwmExtendFrameIntoClientAreaFn extendFrame = nullptr;
    if (!resolveDwmApis(&setWindowAttribute, &extendFrame))
    {
        return;
    }

    if (extendFrame)
    {
        applyDwmFrameMargins(widget, hwnd, useNativeCaption, extendFrame, options, "frame");
    }

    if (!setWindowAttribute)
    {
        return;
    }

    if (useNativeCaption)
    {
        const int cornerPreference = [&options]() {
            if (options.maximized || options.fullScreen || options.cornerRadius <= 0)
            {
                return kDwmCornerDoNotRound;
            }
            if (options.cornerRadius <= 6)
            {
                return kDwmCornerRoundSmall;
            }
            return kDwmCornerRound;
        }();
        setWindowAttribute(hwnd, kDwmWindowCornerPreference, &cornerPreference, sizeof(cornerPreference));
    }

    const auto& token = antTheme->tokens();
    if (options.translucentBackground)
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
    if (widget)
    {
        widget->update();
    }
    Q_UNUSED(options)
#endif
}

void updateLegacySoftwareShadow(QWidget* owner,
                                QWidget*& shadowWidget,
                                const QString& objectName,
                                const char* enabledProperty,
                                const char* marginProperty,
                                const char* innerClearanceProperty,
                                const char* geometryProperty,
                                const char* geometryModeProperty,
                                const char* dprProperty,
                                const char* clickThroughProperty,
                                bool enabled,
                                int cornerRadius)
{
#ifdef Q_OS_WIN
    if (!owner)
    {
        return;
    }

    if (enabledProperty)
    {
        owner->setProperty(enabledProperty, enabled);
    }
    if (marginProperty)
    {
        owner->setProperty(marginProperty, LegacySoftwareShadowMargin);
    }
    if (innerClearanceProperty)
    {
        owner->setProperty(innerClearanceProperty, LegacySoftwareShadowInnerClearance);
    }

    if (!enabled)
    {
        hideLegacySoftwareShadow(owner, shadowWidget, enabledProperty, clickThroughProperty);
        return;
    }

    if (!shadowWidget)
    {
        auto* shadow = new LegacySoftwareShadow(owner, objectName, clickThroughProperty);
        shadowWidget = shadow;
        QObject::connect(antTheme, &AntTheme::themeChanged, shadow, qOverload<>(&QWidget::update));
    }

    if (auto* shadow = static_cast<LegacySoftwareShadow*>(shadowWidget))
    {
        shadow->setCornerRadius(cornerRadius);
    }

    if (QWindow* shadowWindow = shadowWidget->windowHandle())
    {
        if (QScreen* hostScreen = owner->windowHandle() ? owner->windowHandle()->screen() : nullptr)
        {
            if (shadowWindow->screen() != hostScreen)
            {
                shadowWindow->setScreen(hostScreen);
            }
        }
    }

    const QRect shadowGeometry = owner->geometry().adjusted(-LegacySoftwareShadowMargin,
                                                           -LegacySoftwareShadowMargin,
                                                           LegacySoftwareShadowMargin,
                                                           LegacySoftwareShadowMargin);
    shadowWidget->setGeometry(shadowGeometry);
    if (geometryProperty)
    {
        owner->setProperty(geometryProperty, shadowGeometry);
        shadowWidget->setProperty(geometryProperty, shadowGeometry);
    }
    if (geometryModeProperty)
    {
        owner->setProperty(geometryModeProperty, QStringLiteral("qt-logical"));
        shadowWidget->setProperty(geometryModeProperty, QStringLiteral("qt-logical"));
    }
    if (!shadowWidget->isVisible())
    {
        shadowWidget->show();
    }
    if (dprProperty)
    {
        shadowWidget->setProperty(dprProperty, shadowWidget->devicePixelRatioF());
    }

    const HWND hwnd = reinterpret_cast<HWND>(owner->winId());
    const HWND shadowHwnd = reinterpret_cast<HWND>(shadowWidget->winId());
    if (shadowHwnd && hwnd)
    {
        makeNativeInputTransparent(shadowWidget, clickThroughProperty);
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
        shadowWidget->show();
    }
#else
    Q_UNUSED(owner)
    Q_UNUSED(shadowWidget)
    Q_UNUSED(objectName)
    Q_UNUSED(enabledProperty)
    Q_UNUSED(marginProperty)
    Q_UNUSED(innerClearanceProperty)
    Q_UNUSED(geometryProperty)
    Q_UNUSED(geometryModeProperty)
    Q_UNUSED(dprProperty)
    Q_UNUSED(clickThroughProperty)
    Q_UNUSED(enabled)
    Q_UNUSED(cornerRadius)
#endif
}

void hideLegacySoftwareShadow(QWidget* owner,
                              QWidget* shadowWidget,
                              const char* enabledProperty,
                              const char* clickThroughProperty)
{
    if (owner && enabledProperty)
    {
        owner->setProperty(enabledProperty, false);
    }
    if (!shadowWidget)
    {
        return;
    }

#ifdef Q_OS_WIN
    if (const WId shadowId = shadowWidget->internalWinId())
    {
        HWND shadowHwnd = reinterpret_cast<HWND>(shadowId);
        makeNativeInputTransparent(shadowWidget, clickThroughProperty);
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
#else
    Q_UNUSED(clickThroughProperty)
#endif
    shadowWidget->hide();
}
} // namespace AntWindowFrame
