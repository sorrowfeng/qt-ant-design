#include "AntDockManager.h"

#include <QApplication>
#include <QCloseEvent>
#include <QContextMenuEvent>
#include <QDockWidget>
#include <QEvent>
#include <QFontMetrics>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QGuiApplication>
#include <QHideEvent>
#include <QIcon>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmap>
#include <QPointer>
#include <QResizeEvent>
#include <QScreen>
#include <QShowEvent>
#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>

#include <functional>

#include "AntDockWidget.h"
#include "AntMenu.h"
#include "core/AntTheme.h"
#include "../private/AntDockLayoutSerializer.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

namespace
{
constexpr int kDockContextMenuShadowMargin = 24;
constexpr int kDockContextMenuInnerPadding = 4;
constexpr int kDockContextMenuMinWidth = 176;
constexpr int kDockContextMenuMaxWidth = 280;
constexpr int kDockContextMenuShadowSpread = 14;

QString cssColor(const QColor& color)
{
    return QStringLiteral("rgba(%1,%2,%3,%4)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(color.alpha());
}

QColor translucent(const QColor& color, qreal alpha)
{
    QColor result = color;
    result.setAlphaF(alpha);
    return result;
}

QPoint mouseGlobalPosition(QMouseEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->globalPosition().toPoint();
#else
    return event->globalPos();
#endif
}

#if defined(Q_OS_WIN)
constexpr auto kTransparentToolWindowClickThroughProperty = "antDockTransparentToolWindowClickThrough";

void makeTransparentToolWindowClickThrough(QWidget* widget)
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

    widget->setProperty(kTransparentToolWindowClickThroughProperty, true);
}

bool handleTransparentToolWindowNativeEvent(QWidget* widget,
                                            const QByteArray& eventType,
                                            void* message,
                                            qintptr* result)
{
    if (eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG")
    {
        return false;
    }

    auto* msg = static_cast<MSG*>(message);
    if (!msg || msg->message != WM_NCHITTEST)
    {
        return false;
    }

    *result = HTTRANSPARENT;
    if (widget)
    {
        widget->setProperty(kTransparentToolWindowClickThroughProperty, true);
    }
    return true;
}

void forceHideNativeToolWindow(QWidget* widget)
{
    if (!widget)
    {
        return;
    }

    widget->hide();
    if (const WId id = widget->internalWinId())
    {
        HWND hwnd = reinterpret_cast<HWND>(id);
        const LONG_PTR currentStyle = ::GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
        const LONG_PTR transparentStyle = currentStyle | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE;
        if (transparentStyle != currentStyle)
        {
            ::SetWindowLongPtrW(hwnd, GWL_EXSTYLE, transparentStyle);
        }
        ::ShowWindow(hwnd, SW_HIDE);
        ::SetWindowPos(hwnd,
                       nullptr,
                       0,
                       0,
                       0,
                       0,
                       SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER |
                           SWP_NOACTIVATE | SWP_HIDEWINDOW);
    }
    widget->setProperty("antDockNativeToolWindowHidden", true);
}
#endif

void setEmbeddedDockTitleBarVisible(AntDockWidget* dockWidget, bool visible)
{
    if (!dockWidget) return;

    QWidget* titleBar = dockWidget->titleBarWidget();
    if (!titleBar) return;

    titleBar->setVisible(visible);
    titleBar->setMaximumHeight(visible ? QWIDGETSIZE_MAX : 0);
    titleBar->updateGeometry();
    dockWidget->updateGeometry();
}

void setFloatingDockOwner(AntDockWidget* dockWidget, QWidget* ownerWidget)
{
    if (!dockWidget) return;

    // The Win32 owner of a top-level window must itself be a top-level window —
    // pointing GWLP_HWNDPARENT at a child HWND (e.g. one freshly minted by
    // calling winId() on a non-toplevel AntDockManager) breaks the activation
    // and focus chain for every sibling control in the main window. Always
    // resolve to the actual top-level widget so the floating dock is owned by
    // a real top-level HWND.
    QWidget* ownerTopLevel = ownerWidget ? ownerWidget->window() : nullptr;

    QWindow* ownerWindow = nullptr;
    if (ownerTopLevel)
    {
        if (!ownerTopLevel->windowHandle())
        {
            ownerTopLevel->winId();
        }
        ownerWindow = ownerTopLevel->windowHandle();
    }

    if (!dockWidget->windowHandle() && dockWidget->isWindow())
    {
        dockWidget->winId();
    }
    if (QWindow* dockWindow = dockWidget->windowHandle())
    {
        dockWindow->setTransientParent(ownerWindow);
    }

    dockWidget->setProperty("antDockFloatingOwnedByManager", ownerWindow != nullptr);

#if defined(Q_OS_WIN)
    HWND ownerHwnd = nullptr;
    if (ownerTopLevel)
    {
        ownerHwnd = reinterpret_cast<HWND>(ownerTopLevel->winId());
    }

    HWND dockHwnd = reinterpret_cast<HWND>(dockWidget->winId());
    if (dockHwnd)
    {
        ::SetWindowLongPtrW(dockHwnd, GWLP_HWNDPARENT, reinterpret_cast<LONG_PTR>(ownerHwnd));
        if (ownerHwnd)
        {
            ::SetWindowPos(dockHwnd,
                           HWND_TOP,
                           0,
                           0,
                           0,
                           0,
                           SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        }
    }
    dockWidget->setProperty("antDockFloatingNativeOwnedByManager", ownerHwnd != nullptr);
#endif
}

void clearFloatingDockOwner(AntDockWidget* dockWidget)
{
    if (!dockWidget) return;

    if (QWindow* dockWindow = dockWidget->windowHandle())
    {
        dockWindow->setTransientParent(nullptr);
    }

    dockWidget->setProperty("antDockFloatingOwnedByManager", false);

#if defined(Q_OS_WIN)
    if (const WId dockId = dockWidget->internalWinId())
    {
        ::SetWindowLongPtrW(reinterpret_cast<HWND>(dockId), GWLP_HWNDPARENT, 0);
    }
    dockWidget->setProperty("antDockFloatingNativeOwnedByManager", false);
#endif
}

#if defined(Q_OS_WIN)
void releaseNativeMouseCaptureForDock(AntDockWidget* dockWidget)
{
    HWND capturedHwnd = ::GetCapture();
    if (!capturedHwnd)
    {
        if (dockWidget)
        {
            dockWidget->setProperty("antDockNativeMouseCaptureCleared", true);
        }
        return;
    }

    DWORD captureProcessId = 0;
    ::GetWindowThreadProcessId(capturedHwnd, &captureProcessId);
    const bool captureBelongsToThisProcess = captureProcessId == ::GetCurrentProcessId();

    if (captureBelongsToThisProcess)
    {
        ::ReleaseCapture();
    }

    if (dockWidget)
    {
        const HWND afterRelease = ::GetCapture();
        DWORD afterProcessId = 0;
        if (afterRelease)
        {
            ::GetWindowThreadProcessId(afterRelease, &afterProcessId);
        }
        dockWidget->setProperty("antDockNativeMouseCaptureCleared",
                                !afterRelease || afterProcessId != ::GetCurrentProcessId());
        dockWidget->setProperty("antDockNativeMouseCaptureWasDock", captureBelongsToThisProcess);
    }
}

void normalizeEmbeddedDockNativeWindow(AntDockWidget* dockWidget, QWidget* parentWidget)
{
    if (!dockWidget || !parentWidget)
    {
        return;
    }

    HWND dockHwnd = nullptr;
    if (dockWidget->internalWinId())
    {
        dockHwnd = reinterpret_cast<HWND>(dockWidget->internalWinId());
    }
    if (!dockHwnd)
    {
        dockWidget->setProperty("antDockNativeEmbeddedChildFrame", true);
        return;
    }

    HWND parentHwnd = reinterpret_cast<HWND>(parentWidget->winId());
    if (parentHwnd)
    {
        ::SetParent(dockHwnd, parentHwnd);
        dockWidget->setProperty("antDockNativeEmbeddedParentApplied", true);
    }

    LONG_PTR style = ::GetWindowLongPtrW(dockHwnd, GWL_STYLE);
    style |= WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
    ::SetWindowLongPtrW(dockHwnd, GWL_STYLE, style);

    LONG_PTR exStyle = ::GetWindowLongPtrW(dockHwnd, GWL_EXSTYLE);
    exStyle &= ~(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_APPWINDOW |
                 WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE);
    ::SetWindowLongPtrW(dockHwnd, GWL_EXSTYLE, exStyle);
    ::EnableWindow(dockHwnd, TRUE);
    ::SetWindowPos(dockHwnd,
                   nullptr,
                   0,
                   0,
                   0,
                   0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER |
                       SWP_NOACTIVATE | SWP_FRAMECHANGED);

    dockWidget->setProperty("antDockNativeEmbeddedChildFrame", true);
    dockWidget->setProperty("antDockNativeEmbeddedStyle", static_cast<qulonglong>(style));
    dockWidget->setProperty("antDockNativeEmbeddedExStyle", static_cast<qulonglong>(exStyle));
}
#endif

void prepareDockWidgetForEmbedding(AntDockWidget* dockWidget, QWidget* parentWidget)
{
    if (!dockWidget || !parentWidget)
    {
        return;
    }

    if (QWidget* grabber = QWidget::mouseGrabber())
    {
        if (grabber == dockWidget || dockWidget->isAncestorOf(grabber))
        {
            grabber->releaseMouse();
        }
    }
    dockWidget->releaseMouse();

#if defined(Q_OS_WIN)
    releaseNativeMouseCaptureForDock(dockWidget);
#endif

    const bool wasTopLevel = dockWidget->isWindow() || dockWidget->isFloating() ||
                             dockWidget->property("antDockFloatingOwnedByManager").toBool();
    if (wasTopLevel)
    {
        dockWidget->hide();
    }

    clearFloatingDockOwner(dockWidget);
    dockWidget->setWindowOpacity(1.0);
    dockWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    dockWidget->setWindowFlag(Qt::WindowTransparentForInput, false);
#endif
    dockWidget->setProperty("antDockEmbeddedByManager", true);

#if defined(Q_OS_WIN)
    if (wasTopLevel)
    {
        dockWidget->resetNativeFloatingWindowForEmbedding();
    }
#endif

    // A floating AntDockWidget has been promoted to a native top-level HWND.
    // When it is placed back into the custom tab/splitter tree, force the Qt
    // window type back to a plain child widget so the old native window cannot
    // remain above the main window's client area and eat mouse input.
    dockWidget->setParent(parentWidget, Qt::Widget);
    dockWidget->clearMask();
#if defined(Q_OS_WIN)
    normalizeEmbeddedDockNativeWindow(dockWidget, parentWidget);
#endif
}

QRect availableScreenGeometryForPopup(const QPoint& globalPos, const QWidget* fallback)
{
    if (QScreen* screen = QGuiApplication::screenAt(globalPos))
    {
        return screen->availableGeometry();
    }
    if (fallback)
    {
        if (QScreen* screen = fallback->screen())
        {
            return screen->availableGeometry();
        }
    }
    if (QScreen* screen = QGuiApplication::primaryScreen())
    {
        return screen->availableGeometry();
    }
    return QRect(0, 0, 1280, 720);
}

void drawDockContextMenuShadow(QPainter* painter, const QRect& panel, int radius)
{
    if (!painter || panel.isEmpty())
    {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    const auto& token = antTheme->tokens();
    const QRectF sourceRect(panel);
    const QColor shadowBase = token.colorShadow;
    const qreal maxOpacity = antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.14 : 0.10;

    for (int step = kDockContextMenuShadowSpread; step > 0; --step)
    {
        const qreal outerDistance = static_cast<qreal>(step);
        const qreal innerDistance = static_cast<qreal>(step - 1);
        const qreal proximity = 1.0 - outerDistance / static_cast<qreal>(kDockContextMenuShadowSpread);
        const qreal eased = proximity * proximity * (3.0 - 2.0 * proximity);
        const qreal opacity = maxOpacity * eased;
        if (opacity <= 0.0)
        {
            continue;
        }

        QColor shadow = shadowBase;
        shadow.setAlphaF(opacity);

        QPainterPath outerPath;
        const QRectF outerRect = sourceRect.adjusted(-outerDistance,
                                                     -outerDistance,
                                                     outerDistance,
                                                     outerDistance);
        outerPath.addRoundedRect(outerRect, radius + outerDistance, radius + outerDistance);

        QPainterPath innerPath;
        if (innerDistance <= 0.0)
        {
            innerPath.addRoundedRect(sourceRect, radius, radius);
        }
        else
        {
            const QRectF innerRect = sourceRect.adjusted(-innerDistance,
                                                         -innerDistance,
                                                         innerDistance,
                                                         innerDistance);
            innerPath.addRoundedRect(innerRect, radius + innerDistance, radius + innerDistance);
        }

        painter->fillPath(outerPath.subtracted(innerPath), shadow);
    }

    painter->restore();
}

class AntDockContextMenuPopup : public QFrame
{
public:
    explicit AntDockContextMenuPopup(QWidget* parent)
        : QFrame(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
    {
        setObjectName(QStringLiteral("AntDockContextMenuPopup"));
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_ShowWithoutActivating, true);
        setAutoFillBackground(false);
        setMouseTracking(true);

        auto* layout = new QVBoxLayout(this);
        const int popupMargin = kDockContextMenuShadowMargin + kDockContextMenuInnerPadding;
        layout->setContentsMargins(popupMargin, popupMargin, popupMargin, popupMargin);
        layout->setSpacing(0);

        m_menu = new AntMenu(this);
        m_menu->setObjectName(QStringLiteral("AntDockContextMenu"));
        m_menu->setMode(Ant::MenuMode::Vertical);
        m_menu->setCompact(true);
        m_menu->setSelectable(false);
        syncTheme();
        layout->addWidget(m_menu);

        connect(antTheme, &AntTheme::themeChanged, this, [this]() {
            syncTheme();
            update();
        });
    }

    ~AntDockContextMenuPopup() override
    {
        uninstallGlobalEventFilter();
    }

    AntMenu* menu() const { return m_menu; }

    void popupAt(const QPoint& globalPos)
    {
        rebuildGeometry();
        QRect geometry(globalPos, size());
        const QRect screenRect = availableScreenGeometryForPopup(globalPos, parentWidget());
        geometry.moveLeft(qBound(screenRect.left() + 4, geometry.left(), screenRect.right() - geometry.width() - 4));
        geometry.moveTop(qBound(screenRect.top() + 4, geometry.top(), screenRect.bottom() - geometry.height() - 4));
        setGeometry(geometry);
        installGlobalEventFilter();
        show();
        raise();
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (!isVisible())
        {
            return QFrame::eventFilter(watched, event);
        }

        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
        {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            const QPoint globalPos = mouseGlobalPosition(mouseEvent);
            if (!geometry().contains(globalPos))
            {
                close();
            }
            return false;
        }

        if (event->type() == QEvent::KeyPress)
        {
            auto* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Escape)
            {
                close();
                return true;
            }
        }

        if (event->type() == QEvent::WindowDeactivate)
        {
            close();
        }

        return QFrame::eventFilter(watched, event);
    }

    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        const QRect panel = rect().adjusted(kDockContextMenuShadowMargin,
                                            kDockContextMenuShadowMargin,
                                            -kDockContextMenuShadowMargin,
                                            -kDockContextMenuShadowMargin);
        drawDockContextMenuShadow(&painter, panel, token.borderRadiusLG);
        const QColor border = antTheme->themeMode() == Ant::ThemeMode::Dark
                                  ? translucent(token.colorTextLightSolid, 0.18)
                                  : token.colorBorder;
        painter.setPen(QPen(border, token.lineWidth));
        painter.setBrush(token.colorBgElevated);
        painter.drawRoundedRect(QRectF(panel).adjusted(0.5, 0.5, -0.5, -0.5),
                                token.borderRadiusLG,
                                token.borderRadiusLG);
    }

    void hideEvent(QHideEvent* event) override
    {
        uninstallGlobalEventFilter();
        QFrame::hideEvent(event);
        deleteLater();
    }

private:
    void installGlobalEventFilter()
    {
        if (!qApp || m_eventFilterInstalled)
        {
            return;
        }
        qApp->installEventFilter(this);
        m_eventFilterInstalled = true;
    }

    void uninstallGlobalEventFilter()
    {
        if (!qApp || !m_eventFilterInstalled)
        {
            return;
        }
        qApp->removeEventFilter(this);
        m_eventFilterInstalled = false;
    }

    void syncTheme()
    {
        if (!m_menu)
        {
            return;
        }
        m_menu->setMenuTheme(antTheme->themeMode() == Ant::ThemeMode::Dark
                                 ? Ant::MenuTheme::Dark
                                 : Ant::MenuTheme::Light);
    }

    void rebuildGeometry()
    {
        if (!m_menu)
        {
            return;
        }

        const auto& token = antTheme->tokens();
        QFont textFont = m_menu->font();
        textFont.setPixelSize(token.fontSize);
        const QFontMetrics fm(textFont);

        int maxTextWidth = 96;
        for (int i = 0; i < m_menu->itemCount(); ++i)
        {
            const AntMenuItem item = m_menu->itemAt(i);
            if (item.divider)
            {
                continue;
            }
            maxTextWidth = qMax(maxTextWidth, fm.horizontalAdvance(item.label));
        }

        const int menuWidth = qBound(kDockContextMenuMinWidth,
                                     maxTextWidth + token.paddingLG * 2 + 42,
                                     kDockContextMenuMaxWidth);
        m_menu->setFixedWidth(menuWidth);
        m_menu->adjustSize();

        const int popupMargin = kDockContextMenuShadowMargin + kDockContextMenuInnerPadding;
        setFixedSize(menuWidth + popupMargin * 2, m_menu->sizeHint().height() + popupMargin * 2);
        update();
    }

    AntMenu* m_menu = nullptr;
    bool m_eventFilterInstalled = false;
};
} // namespace

using AntDockInternal::DockLayoutNode;
using AntDockInternal::DockLayoutNodeType;
using AntDockInternal::FloatingDockSnapshot;
using AntDockInternal::captureDockLayoutNode;
using AntDockInternal::collectDockIds;
using AntDockInternal::deserializeDockPerspective;
using AntDockInternal::dockPersistentId;
using AntDockInternal::isLegacyDockPerspective;
using AntDockInternal::serializeDockPerspective;

struct AntDockManager::DropTarget
{
    bool valid = false;
    bool containerTarget = false;
    AntDockWidget* dockWidget = nullptr;
    DockPlacement placement = DockPlacement::None;
    QRect targetGlobalRect;
    QRect previewGlobalRect;
    QString label;
};

class AntDockManager::Workspace : public QWidget
{
public:
    explicit Workspace(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setAutoFillBackground(true);

        m_layout = new QVBoxLayout(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        setMinimumSize(96, 72);
    }

    QWidget* contentWidget() const
    {
        return m_content;
    }

    void setContentWidget(QWidget* widget)
    {
        if (m_content == widget) return;

        if (m_content)
        {
            m_layout->removeWidget(m_content);
            m_content->setParent(nullptr);
        }

        m_content = widget;
        if (m_content)
        {
            m_content->setParent(this);
            m_layout->addWidget(m_content);
        }
        update();
    }

    void setPlaceholderActive(bool active)
    {
        if (m_placeholderActive == active) return;
        m_placeholderActive = active;
        update();
    }

    void updateTheme()
    {
        const auto& token = antTheme->tokens();
        QPalette pal = palette();
        pal.setColor(QPalette::Window, token.colorBgLayout);
        pal.setColor(QPalette::Base, token.colorBgLayout);
        pal.setColor(QPalette::WindowText, token.colorTextSecondary);
        pal.setColor(QPalette::Text, token.colorTextSecondary);
        setPalette(pal);

        if (m_content)
        {
            QPalette contentPalette = m_content->palette();
            contentPalette.setColor(QPalette::Window, token.colorBgLayout);
            contentPalette.setColor(QPalette::Base, token.colorBgLayout);
            contentPalette.setColor(QPalette::WindowText, token.colorText);
            contentPalette.setColor(QPalette::Text, token.colorText);
            m_content->setPalette(contentPalette);
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QWidget::paintEvent(event);
        if (!m_placeholderActive || m_content) return;

        const auto& token = antTheme->tokens();
        const QRectF r = QRectF(rect()).adjusted(16.5, 16.5, -16.5, -16.5);
        if (r.width() < 40 || r.height() < 32) return;

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        QColor fill = translucent(token.colorPrimaryBg, antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.32 : 0.78);
        painter.setBrush(fill);
        painter.setPen(QPen(token.colorPrimaryBorder, 1, Qt::DashLine));
        painter.drawRoundedRect(r, token.borderRadius, token.borderRadius);

        QFont font = painter.font();
        font.setPixelSize(token.fontSize);
        painter.setFont(font);
        painter.setPen(token.colorTextSecondary);
        painter.drawText(r, Qt::AlignCenter, QStringLiteral("Dock workspace"));
    }

private:
    QVBoxLayout* m_layout = nullptr;
    QWidget* m_content = nullptr;
    bool m_placeholderActive = true;
};

class AntDockManager::DockArea : public QTabWidget
{
public:
    explicit DockArea(AntDockManager* manager)
        : QTabWidget(manager), m_manager(manager)
    {
        setDocumentMode(true);
        setMovable(false);
        setTabsClosable(false);
        setUsesScrollButtons(true);
        setElideMode(Qt::ElideRight);
        setMinimumSize(96, 72);
        if (tabBar())
        {
            tabBar()->setDrawBase(false);
            tabBar()->setExpanding(false);
        }
    }

    bool containsDock(AntDockWidget* dockWidget) const
    {
        return dockWidget && indexOf(dockWidget) >= 0;
    }

    QList<AntDockWidget*> dockWidgets() const
    {
        QList<AntDockWidget*> result;
        for (int i = 0; i < count(); ++i)
        {
            if (auto* dock = qobject_cast<AntDockWidget*>(widget(i)))
            {
                result.append(dock);
            }
        }
        return result;
    }

    int dockIndex(AntDockWidget* dockWidget) const
    {
        return dockWidget ? indexOf(dockWidget) : -1;
    }

    bool moveDock(AntDockWidget* dockWidget, int targetIndex)
    {
        const int from = dockIndex(dockWidget);
        if (from < 0 || count() <= 1) return false;

        const int to = qBound(0, targetIndex, count() - 1);
        if (from == to) return false;

        const QString text = tabText(from);
        const QIcon icon = tabIcon(from);
        const QString toolTip = tabToolTip(from);
        const QString whatsThis = tabWhatsThis(from);
        const bool enabled = isTabEnabled(from);

        removeTab(from);
        const int inserted = insertTab(to, dockWidget, icon, text);
        setTabToolTip(inserted, toolTip);
        setTabWhatsThis(inserted, whatsThis);
        setTabEnabled(inserted, enabled);
        setCurrentIndex(inserted);
        return true;
    }

    void addDock(AntDockWidget* dockWidget)
    {
        if (!dockWidget) return;

        const int existing = indexOf(dockWidget);
        if (existing >= 0)
        {
            setCurrentIndex(existing);
            return;
        }

        prepareDockWidgetForEmbedding(dockWidget, this);
        setEmbeddedDockTitleBarVisible(dockWidget, false);
        const int index = addTab(dockWidget, dockWidget->windowIcon(), dockWidget->windowTitle());
        setCurrentIndex(index);
        dockWidget->setVisible(true);
        connect(dockWidget, &QDockWidget::windowTitleChanged, this, [this, dockWidget](const QString& title) {
            const int tab = indexOf(dockWidget);
            if (tab >= 0) setTabText(tab, title);
        });
        connect(dockWidget, &QDockWidget::windowIconChanged, this, [this, dockWidget](const QIcon& icon) {
            const int tab = indexOf(dockWidget);
            if (tab >= 0) setTabIcon(tab, icon);
        });
    }

    void removeDock(AntDockWidget* dockWidget)
    {
        const int index = indexOf(dockWidget);
        if (index < 0) return;
        removeTab(index);
        dockWidget->setProperty("antDockEmbeddedByManager", false);
        setEmbeddedDockTitleBarVisible(dockWidget, true);
        dockWidget->setParent(nullptr);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QTabWidget::paintEvent(event);
        if (count() > 0) return;

        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        QColor border = token.colorBorderSecondary;
        border.setAlphaF(0.56);
        painter.setPen(QPen(border, 1, Qt::DashLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(QRectF(rect()).adjusted(8.5, 8.5, -8.5, -8.5),
                                token.borderRadius, token.borderRadius);
    }

private:
    AntDockManager* m_manager = nullptr;
};

class AntDockManager::DockGuideOverlay : public QWidget
{
private:
    struct GuideZone
    {
        DockPlacement placement;
        QRect rect;
        bool edge = false;
    };

public:
    explicit DockGuideOverlay(AntDockManager* manager)
        : QWidget(manager), m_manager(manager)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setAutoFillBackground(false);
        hide();
    }

    DockPlacement activePlacement() const
    {
        return m_activePlacement;
    }

    bool activePlacementIsEdge() const
    {
        return m_activePlacementIsEdge;
    }

    QRect areaGuideRect() const
    {
        return m_areaGuideRect;
    }

    void clearActivePlacement()
    {
        if (m_activePlacement == DockPlacement::None && !m_activePlacementIsEdge) return;
        m_activePlacement = DockPlacement::None;
        m_activePlacementIsEdge = false;
        update();
        Q_EMIT m_manager->activeDropGuideChanged(m_activePlacement);
    }

    void updateFromGlobalPos(const QPoint& globalPos, const QRect& areaGlobalRect, const QRect& containerGlobalRect)
    {
        const QRect nextAreaRect = globalRectToLocal(areaGlobalRect);
        const QRect localContainerRect = globalRectToLocal(containerGlobalRect);
        const QRect nextContainerRect = localContainerRect.isEmpty() ? rect() : localContainerRect;
        const bool targetChanged = m_areaGuideRect != nextAreaRect || m_containerGuideRect != nextContainerRect;
        m_areaGuideRect = nextAreaRect;
        m_containerGuideRect = nextContainerRect;

        const GuideZone next = zoneAt(mapFromGlobal(globalPos));
        const bool placementChanged = m_activePlacement != next.placement;
        if (!placementChanged && m_activePlacementIsEdge == next.edge && !targetChanged) return;

        m_activePlacement = next.placement;
        m_activePlacementIsEdge = next.edge;
        update();
        if (placementChanged)
        {
            Q_EMIT m_manager->activeDropGuideChanged(m_activePlacement);
        }
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        QColor scrim = token.colorBgLayout;
        scrim.setAlphaF(antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.20 : 0.10);
        painter.fillRect(rect(), scrim);

        const auto zones = guideRects();
        QRect clusterRect;
        for (const auto& zone : zones)
        {
            if (zone.edge) continue;
            clusterRect = clusterRect.isNull() ? zone.rect : clusterRect.united(zone.rect);
        }

        if (!clusterRect.isNull())
        {
            QRect panel = clusterRect.adjusted(-9, -9, 9, 9);
            QColor panelShadow = token.colorShadow;
            panelShadow.setAlphaF(antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.28 : 0.16);
            painter.setPen(Qt::NoPen);
            painter.setBrush(panelShadow);
            painter.drawRoundedRect(QRectF(panel).translated(0, 4),
                                    token.borderRadiusLG + 2, token.borderRadiusLG + 2);

            QColor panelFill = token.colorBgElevated;
            panelFill.setAlphaF(antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.76 : 0.88);
            QColor panelBorder = token.colorBorderSecondary;
            panelBorder.setAlphaF(0.64);
            painter.setPen(QPen(panelBorder, 1));
            painter.setBrush(panelFill);
            painter.drawRoundedRect(QRectF(panel).adjusted(0.5, 0.5, -0.5, -0.5),
                                    token.borderRadiusLG + 2, token.borderRadiusLG + 2);
        }

        for (const auto& zone : zones)
        {
            paintGuide(&painter, zone);
        }
    }

private:
    QList<GuideZone> guideRects() const
    {
        QList<GuideZone> zones;
        const int guideSize = 38;
        const int gap = 8;
        const int step = guideSize + gap;
        const int edgeMargin = 14;

        const auto squareAt = [guideSize](const QPoint& c) {
            return QRect(c.x() - guideSize / 2, c.y() - guideSize / 2, guideSize, guideSize);
        };

        if (!m_areaGuideRect.isEmpty())
        {
            const QPoint areaCenter = m_areaGuideRect.center();
            zones.append({DockPlacement::Top, squareAt(areaCenter + QPoint(0, -step)), false});
            zones.append({DockPlacement::Left, squareAt(areaCenter + QPoint(-step, 0)), false});
            zones.append({DockPlacement::Center, squareAt(areaCenter), false});
            zones.append({DockPlacement::Right, squareAt(areaCenter + QPoint(step, 0)), false});
            zones.append({DockPlacement::Bottom, squareAt(areaCenter + QPoint(0, step)), false});
        }

        const QRect container = m_containerGuideRect.isEmpty() ? rect() : m_containerGuideRect;
        const QPoint containerCenter = container.center();
        zones.append({DockPlacement::Left,
                      squareAt(QPoint(container.left() + edgeMargin + guideSize / 2, containerCenter.y())),
                      true});
        zones.append({DockPlacement::Right,
                      squareAt(QPoint(container.right() - edgeMargin - guideSize / 2, containerCenter.y())),
                      true});
        zones.append({DockPlacement::Top,
                      squareAt(QPoint(containerCenter.x(), container.top() + edgeMargin + guideSize / 2)),
                      true});
        zones.append({DockPlacement::Bottom,
                      squareAt(QPoint(containerCenter.x(), container.bottom() - edgeMargin - guideSize / 2)),
                      true});
        return zones;
    }

    QRect globalRectToLocal(const QRect& globalRect) const
    {
        if (globalRect.isEmpty()) return QRect();
        return QRect(mapFromGlobal(globalRect.topLeft()), globalRect.size()).intersected(rect());
    }

    GuideZone zoneAt(const QPoint& pos) const
    {
        const auto zones = guideRects();
        for (const auto& zone : zones)
        {
            if (zone.rect.contains(pos)) return zone;
        }
        return {DockPlacement::None, QRect(), false};
    }

    void paintGuide(QPainter* painter, const GuideZone& zone)
    {
        if (!painter || zone.rect.isEmpty()) return;

        const auto& token = antTheme->tokens();
        const bool active = zone.placement == m_activePlacement && zone.edge == m_activePlacementIsEdge;
        const QRectF box = QRectF(zone.rect).adjusted(0.5, 0.5, -0.5, -0.5);

        QColor shadow = token.colorShadow;
        shadow.setAlphaF(active ? (antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.32 : 0.18) : 0.12);
        painter->setPen(Qt::NoPen);
        painter->setBrush(shadow);
        painter->drawRoundedRect(box.translated(0, zone.edge ? 3 : 2),
                                 token.borderRadiusSM + 2, token.borderRadiusSM + 2);

        QColor fill = active ? token.colorPrimaryBg : token.colorBgElevated;
        fill.setAlphaF(active ? (antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.92 : 0.98)
                              : (antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.88 : 0.94));

        QColor border = active ? token.colorPrimary : token.colorPrimaryBorder;
        border.setAlphaF(active ? 1.0 : (zone.edge ? 0.82 : 0.68));
        painter->setBrush(fill);
        painter->setPen(QPen(border, active ? 2 : 1));
        painter->drawRoundedRect(box, token.borderRadiusSM + 1, token.borderRadiusSM + 1);

        paintGuideGlyph(painter, zone.placement, zone.rect, active, zone.edge);
    }

    void paintGuideGlyph(QPainter* painter, DockPlacement placement, const QRect& rect, bool active, bool edge)
    {
        if (!painter) return;

        const auto& token = antTheme->tokens();
        QColor glyph = active ? token.colorPrimary : token.colorPrimaryHover;
        glyph.setAlphaF(active ? 1.0 : 0.82);
        QColor glyphFill = glyph;
        glyphFill.setAlphaF(active ? 0.24 : 0.14);

        const QPointF c = QRectF(rect).center();
        const QRectF icon(c.x() - 9.0, c.y() - 8.0, 18.0, 16.0);
        painter->setPen(QPen(glyph, 1.6));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(icon, 2.0, 2.0);

        QRectF dockArea;
        switch (placement)
        {
        case DockPlacement::Left:
            dockArea = QRectF(icon.left() + 2.0, icon.top() + 2.0, 6.0, icon.height() - 4.0);
            break;
        case DockPlacement::Right:
            dockArea = QRectF(icon.right() - 8.0, icon.top() + 2.0, 6.0, icon.height() - 4.0);
            break;
        case DockPlacement::Top:
            dockArea = QRectF(icon.left() + 2.0, icon.top() + 2.0, icon.width() - 4.0, 5.5);
            break;
        case DockPlacement::Bottom:
            dockArea = QRectF(icon.left() + 2.0, icon.bottom() - 7.5, icon.width() - 4.0, 5.5);
            break;
        case DockPlacement::Center:
            dockArea = icon.adjusted(4.0, 3.5, -4.0, -3.5);
            break;
        case DockPlacement::None:
            break;
        }

        if (!dockArea.isEmpty())
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(glyphFill);
            painter->drawRoundedRect(dockArea, 1.5, 1.5);
        }

        painter->setPen(QPen(glyph, 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        switch (placement)
        {
        case DockPlacement::Left:
            painter->drawLine(QPointF(icon.left() + 9.0, icon.top() + 2.0),
                              QPointF(icon.left() + 9.0, icon.bottom() - 2.0));
            break;
        case DockPlacement::Right:
            painter->drawLine(QPointF(icon.right() - 9.0, icon.top() + 2.0),
                              QPointF(icon.right() - 9.0, icon.bottom() - 2.0));
            break;
        case DockPlacement::Top:
            painter->drawLine(QPointF(icon.left() + 2.0, icon.top() + 8.0),
                              QPointF(icon.right() - 2.0, icon.top() + 8.0));
            break;
        case DockPlacement::Bottom:
            painter->drawLine(QPointF(icon.left() + 2.0, icon.bottom() - 8.0),
                              QPointF(icon.right() - 2.0, icon.bottom() - 8.0));
            break;
        case DockPlacement::Center:
            painter->drawLine(QPointF(icon.left() + 5.0, icon.center().y()),
                              QPointF(icon.right() - 5.0, icon.center().y()));
            break;
        case DockPlacement::None:
            break;
        }

        if (edge)
        {
            paintEdgeArrow(painter, rect, placement, glyph);
        }
    }

    void paintEdgeArrow(QPainter* painter, const QRect& rect, DockPlacement placement, const QColor& color)
    {
        if (!painter) return;

        const QRectF r(rect);
        const QPointF c = r.center();
        QPainterPath arrow;
        switch (placement)
        {
        case DockPlacement::Left:
            arrow.moveTo(r.left() + 7.0, c.y());
            arrow.lineTo(r.left() + 12.0, c.y() - 4.5);
            arrow.lineTo(r.left() + 12.0, c.y() + 4.5);
            break;
        case DockPlacement::Right:
            arrow.moveTo(r.right() - 7.0, c.y());
            arrow.lineTo(r.right() - 12.0, c.y() - 4.5);
            arrow.lineTo(r.right() - 12.0, c.y() + 4.5);
            break;
        case DockPlacement::Top:
            arrow.moveTo(c.x(), r.top() + 7.0);
            arrow.lineTo(c.x() - 4.5, r.top() + 12.0);
            arrow.lineTo(c.x() + 4.5, r.top() + 12.0);
            break;
        case DockPlacement::Bottom:
            arrow.moveTo(c.x(), r.bottom() - 7.0);
            arrow.lineTo(c.x() - 4.5, r.bottom() - 12.0);
            arrow.lineTo(c.x() + 4.5, r.bottom() - 12.0);
            break;
        case DockPlacement::Center:
        case DockPlacement::None:
            return;
        }
        arrow.closeSubpath();

        QColor arrowColor = color;
        arrowColor.setAlphaF(0.90);
        painter->setPen(Qt::NoPen);
        painter->setBrush(arrowColor);
        painter->drawPath(arrow);
    }

    AntDockManager* m_manager = nullptr;
    DockPlacement m_activePlacement = DockPlacement::None;
    bool m_activePlacementIsEdge = false;
    QRect m_areaGuideRect;
    QRect m_containerGuideRect;
};

class AntDockManager::DockDragPreviewWindow : public QWidget
{
public:
    explicit DockDragPreviewWindow(QWidget* parent = nullptr)
        : QWidget(parent,
                  Qt::Tool |
                  Qt::FramelessWindowHint |
                  Qt::WindowStaysOnTopHint |
                  Qt::NoDropShadowWindowHint)
    {
        setObjectName(QStringLiteral("AntDockDragPreviewWindow"));
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_ShowWithoutActivating);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        setWindowFlag(Qt::WindowTransparentForInput, true);
#endif
        setFocusPolicy(Qt::NoFocus);
#if defined(Q_OS_WIN)
        setProperty(kTransparentToolWindowClickThroughProperty, false);
#endif
        hide();
    }

    void begin(AntDockWidget* dockWidget, const QPoint& globalPos, const QPoint& offset)
    {
        if (!dockWidget) return;

        m_offset = offset;
        m_title = dockWidget->windowTitle();
        m_pixmap = QPixmap(dockWidget->size());
        m_pixmap.fill(Qt::transparent);
        dockWidget->render(&m_pixmap, QPoint(), QRegion(), QWidget::DrawChildren);

        const QSize minSize(180, 96);
        QSize proxySize = dockWidget->size();
        proxySize.setWidth(qMax(minSize.width(), proxySize.width()));
        proxySize.setHeight(qMax(minSize.height(), proxySize.height()));
        resize(proxySize);
        const QPoint topLeft = globalPos - m_offset;
        if (pos() != topLeft)
        {
            move(topLeft);
        }
        if (!isVisible()) show();
#if defined(Q_OS_WIN)
        makeTransparentToolWindowClickThrough(this);
#endif
        raise();
        update();
    }

    void moveToGlobalPos(const QPoint& globalPos)
    {
        if (!isVisible()) return;
        const QPoint topLeft = globalPos - m_offset;
        if (pos() != topLeft)
        {
            move(topLeft);
        }
    }

    QRect previewGlobalGeometry() const
    {
        return isVisible() ? geometry() : QRect();
    }

    void end()
    {
#if defined(Q_OS_WIN)
        forceHideNativeToolWindow(this);
#else
        hide();
#endif
        m_pixmap = QPixmap();
        m_title.clear();
        m_offset = QPoint();
    }

protected:
#if defined(Q_OS_WIN)
    void showEvent(QShowEvent* event) override
    {
        QWidget::showEvent(event);
        makeTransparentToolWindowClickThrough(this);
    }

    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override
    {
        if (handleTransparentToolWindowNativeEvent(this, eventType, message, result))
        {
            return true;
        }
        return QWidget::nativeEvent(eventType, message, result);
    }
#endif

    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

        QRectF panel = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        QColor shadow = token.colorShadow;
        shadow.setAlphaF(0.22);
        painter.setPen(Qt::NoPen);
        painter.setBrush(shadow);
        painter.drawRoundedRect(panel.translated(0, 6), token.borderRadiusLG, token.borderRadiusLG);

        QColor fill = token.colorBgElevated;
        fill.setAlphaF(0.76);
        QColor border = token.colorPrimary;
        border.setAlphaF(0.78);
        painter.setPen(QPen(border, 1.5));
        painter.setBrush(fill);
        painter.drawRoundedRect(panel, token.borderRadiusLG, token.borderRadiusLG);

        if (!m_pixmap.isNull())
        {
            painter.setOpacity(0.58);
            painter.drawPixmap(rect(), m_pixmap);
            painter.setOpacity(1.0);
        }

        if (!m_title.isEmpty())
        {
            QFont font = painter.font();
            font.setPixelSize(token.fontSize);
            font.setWeight(QFont::DemiBold);
            painter.setFont(font);
            painter.setPen(token.colorText);
            painter.drawText(rect().adjusted(12, 8, -12, -8),
                             Qt::AlignTop | Qt::AlignLeft,
                             m_title);
        }
    }

private:
    QPixmap m_pixmap;
    QString m_title;
    QPoint m_offset;
};

class AntDockManager::DockDropPreviewWindow : public QWidget
{
public:
    explicit DockDropPreviewWindow(AntDockManager* manager)
        : QWidget(nullptr,
                  Qt::Tool |
                  Qt::FramelessWindowHint |
                  Qt::WindowStaysOnTopHint |
                  Qt::NoDropShadowWindowHint),
          m_manager(manager)
    {
        setObjectName(QStringLiteral("AntDockDropPreviewWindow"));
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_ShowWithoutActivating);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        setWindowFlag(Qt::WindowTransparentForInput, true);
#endif
        setFocusPolicy(Qt::NoFocus);
#if defined(Q_OS_WIN)
        setProperty(kTransparentToolWindowClickThroughProperty, false);
#endif
        hide();
    }

    QRect previewGlobalRect() const
    {
        return m_previewGlobalRect;
    }

    void showTarget(const DropTarget& target, const QRect& managerGlobalRect)
    {
        if (!target.valid || target.previewGlobalRect.isEmpty())
        {
            hideTarget();
            return;
        }

        m_targetGlobalRect = target.targetGlobalRect;
        m_previewGlobalRect = target.previewGlobalRect;
        m_label = target.label;
        m_placement = target.placement;

        const QRect windowRect = managerGlobalRect.united(m_targetGlobalRect).united(m_previewGlobalRect).adjusted(-18, -18, 18, 18);
        if (geometry() != windowRect)
        {
            setGeometry(windowRect);
        }

        if (!isVisible())
        {
            show();
#if defined(Q_OS_WIN)
            makeTransparentToolWindowClickThrough(this);
#endif
            raise();
            Q_EMIT m_manager->dropPreviewVisibleChanged(true);
        }
        update();
    }

    void hideTarget()
    {
        const bool wasVisible = isVisible();
        m_targetGlobalRect = QRect();
        m_previewGlobalRect = QRect();
        m_label.clear();
        m_placement = DockPlacement::None;
#if defined(Q_OS_WIN)
        forceHideNativeToolWindow(this);
#else
        hide();
#endif
        if (wasVisible)
        {
            Q_EMIT m_manager->dropPreviewVisibleChanged(false);
        }
    }

protected:
#if defined(Q_OS_WIN)
    void showEvent(QShowEvent* event) override
    {
        QWidget::showEvent(event);
        makeTransparentToolWindowClickThrough(this);
    }

    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override
    {
        if (handleTransparentToolWindowNativeEvent(this, eventType, message, result))
        {
            return true;
        }
        return QWidget::nativeEvent(eventType, message, result);
    }
#endif

    void paintEvent(QPaintEvent*) override
    {
        if (m_previewGlobalRect.isEmpty()) return;

        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        const QRectF target = QRectF(mapFromGlobal(m_targetGlobalRect.topLeft()), m_targetGlobalRect.size());
        const QRectF preview = QRectF(mapFromGlobal(m_previewGlobalRect.topLeft()), m_previewGlobalRect.size());

        QColor targetStroke = token.colorBorder;
        targetStroke.setAlphaF(0.72);
        painter.setPen(QPen(targetStroke, 1, Qt::DashLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(target.adjusted(0.5, 0.5, -0.5, -0.5),
                                token.borderRadiusLG, token.borderRadiusLG);

        QColor fill = token.colorPrimary;
        fill.setAlphaF(antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.26 : 0.16);
        QColor stroke = token.colorPrimary;
        stroke.setAlphaF(0.92);
        painter.setPen(QPen(stroke, 2));
        painter.setBrush(fill);
        painter.drawRoundedRect(preview.adjusted(0.5, 0.5, -0.5, -0.5),
                                token.borderRadiusLG, token.borderRadiusLG);

        QColor innerStroke = token.colorPrimaryBorder;
        innerStroke.setAlphaF(0.70);
        painter.setPen(QPen(innerStroke, 1, Qt::DashLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(preview.adjusted(7.5, 7.5, -7.5, -7.5),
                                qMax(2, token.borderRadius), qMax(2, token.borderRadius));

        paintCallout(&painter, preview);
    }

private:
    void paintCallout(QPainter* painter, const QRectF& preview)
    {
        if (!painter || m_label.isEmpty()) return;

        const auto& token = antTheme->tokens();
        QFont font = painter->font();
        font.setPixelSize(token.fontSizeSM);
        font.setWeight(QFont::DemiBold);
        painter->setFont(font);

        const QFontMetrics fm(font);
        const int labelW = qMin(qMax(96, fm.horizontalAdvance(m_label) + 24), qMax(120, width() - 24));
        const int labelH = 30;

        QPointF anchor = preview.center();
        QPointF labelCenter = anchor;
        switch (m_placement)
        {
        case DockPlacement::Left:
            anchor = QPointF(preview.right(), preview.center().y());
            labelCenter = anchor + QPointF(76, 0);
            break;
        case DockPlacement::Right:
            anchor = QPointF(preview.left(), preview.center().y());
            labelCenter = anchor - QPointF(76, 0);
            break;
        case DockPlacement::Top:
            anchor = QPointF(preview.center().x(), preview.bottom());
            labelCenter = anchor + QPointF(0, 48);
            break;
        case DockPlacement::Bottom:
            anchor = QPointF(preview.center().x(), preview.top());
            labelCenter = anchor - QPointF(0, 48);
            break;
        case DockPlacement::Center:
            anchor = preview.center();
            labelCenter = anchor + QPointF(0, -qMax<qreal>(44, preview.height() * 0.28));
            break;
        case DockPlacement::None:
            break;
        }

        QRectF labelRect(labelCenter.x() - labelW / 2.0,
                         labelCenter.y() - labelH / 2.0,
                         labelW,
                         labelH);
        labelRect.moveLeft(qBound<qreal>(8, labelRect.left(), width() - labelRect.width() - 8));
        labelRect.moveTop(qBound<qreal>(8, labelRect.top(), height() - labelRect.height() - 8));

        const QPointF clampedCenter = labelRect.center();
        QColor line = token.colorPrimary;
        line.setAlphaF(0.88);
        painter->setPen(QPen(line, 2, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(anchor, clampedCenter);

        QColor calloutFill = token.colorBgElevated;
        calloutFill.setAlphaF(0.96);
        QColor calloutStroke = token.colorPrimary;
        painter->setPen(QPen(calloutStroke, 1));
        painter->setBrush(calloutFill);
        painter->drawRoundedRect(labelRect.adjusted(0.5, 0.5, -0.5, -0.5),
                                 token.borderRadius, token.borderRadius);

        painter->setPen(token.colorPrimary);
        painter->drawText(labelRect, Qt::AlignCenter, m_label);
    }

    AntDockManager* m_manager = nullptr;
    QRect m_targetGlobalRect;
    QRect m_previewGlobalRect;
    QString m_label;
    DockPlacement m_placement = DockPlacement::None;
};

AntDockManager::AntDockManager(QWidget* parent)
    : QMainWindow(parent)
{
    setAnimated(false);
    setDockNestingEnabled(false);
    setDockOptions(QMainWindow::AllowNestedDocks |
                   QMainWindow::AllowTabbedDocks);
    setDocumentMode(true);

    m_workspace = new Workspace(this);
    QMainWindow::setCentralWidget(m_workspace);

    m_dropGuideOverlay = new DockGuideOverlay(this);
    m_dropGuideOverlay->setGeometry(rect());
    m_dragPreviewWindow = new DockDragPreviewWindow();
    m_dropPreviewWindow = new DockDropPreviewWindow(this);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateTheme();
    });

    updateTheme();
    updatePlaceholderState();
}

AntDockManager::~AntDockManager()
{
    stopDockDragTracking();
    if (m_dropPreviewWindow)
    {
        m_dropPreviewWindow->hideTarget();
        delete m_dropPreviewWindow;
        m_dropPreviewWindow = nullptr;
    }
    if (m_dragPreviewWindow)
    {
        m_dragPreviewWindow->end();
        delete m_dragPreviewWindow;
        m_dragPreviewWindow = nullptr;
    }

    for (AntDockWidget* dock : dockWidgets())
    {
        if (!dock) continue;
        removeDockEventFilters(dock);
        disconnect(dock, nullptr, this, nullptr);
    }
    m_docks.clear();
}

void AntDockManager::addDockWidget(Qt::DockWidgetArea area, AntDockWidget* dockWidget)
{
    if (!dockWidget) return;

    DockPlacement placement = DockPlacement::Left;
    if (area == Qt::RightDockWidgetArea) placement = DockPlacement::Right;
    else if (area == Qt::TopDockWidgetArea) placement = DockPlacement::Top;
    else if (area == Qt::BottomDockWidgetArea) placement = DockPlacement::Bottom;

    insertDockWidget(dockWidget, firstDockArea(), placement);
}

void AntDockManager::addDockWidget(Qt::DockWidgetArea area, AntDockWidget* dockWidget, Qt::Orientation orientation)
{
    Q_UNUSED(orientation)
    addDockWidget(area, dockWidget);
}

void AntDockManager::splitDockWidget(AntDockWidget* after, AntDockWidget* dockWidget, Qt::Orientation orientation)
{
    if (!dockWidget) return;

    const DockPlacement placement = orientation == Qt::Vertical
        ? DockPlacement::Bottom
        : DockPlacement::Right;
    insertDockWidget(dockWidget, areaForDock(after), placement);
}

void AntDockManager::tabifyDockWidget(AntDockWidget* first, AntDockWidget* second)
{
    if (!first || !second || first == second) return;
    insertDockWidget(second, areaForDock(first), DockPlacement::Center);
}

Qt::DockWidgetArea AntDockManager::dockWidgetArea(AntDockWidget* dockWidget) const
{
    DockArea* area = areaForDock(dockWidget);
    if (!area) return Qt::NoDockWidgetArea;
    if (area == m_rootDockWidget) return Qt::LeftDockWidgetArea;

    auto* splitter = qobject_cast<QSplitter*>(area->parentWidget());
    if (!splitter) return Qt::LeftDockWidgetArea;

    const int index = splitter->indexOf(area);
    if (splitter->orientation() == Qt::Horizontal)
    {
        return index <= splitter->count() / 2 ? Qt::LeftDockWidgetArea : Qt::RightDockWidgetArea;
    }
    return index <= splitter->count() / 2 ? Qt::TopDockWidgetArea : Qt::BottomDockWidgetArea;
}

QList<AntDockWidget*> AntDockManager::tabifiedDockWidgets(AntDockWidget* dockWidget) const
{
    QList<AntDockWidget*> result;
    DockArea* area = areaForDock(dockWidget);
    if (!area) return result;
    for (AntDockWidget* dock : area->dockWidgets())
    {
        if (dock && dock != dockWidget) result.append(dock);
    }
    return result;
}

void AntDockManager::addDockWidget(AntDockWidget* dockWidget, AntDockWidget* relativeTo, DockPlacement placement)
{
    if (!dockWidget) return;

    if (placement == DockPlacement::None)
    {
        insertDockWidget(dockWidget, firstDockArea(), DockPlacement::Left);
        return;
    }
    insertDockWidget(dockWidget,
                     areaForDock(relativeTo),
                     placement,
                     !relativeTo && placement != DockPlacement::Center);
}

void AntDockManager::removeDockWidget(AntDockWidget* dockWidget)
{
    if (!dockWidget) return;

    if (m_draggedDock == dockWidget)
    {
        stopDockDragTracking();
    }

    const bool known = m_docks.contains(dockWidget);
    const bool wasFloating = dockWidget->isFloating() ||
                             dockWidget->property("antDockFloatingOwnedByManager").toBool() ||
                             (!areaForDock(dockWidget) && dockWidget->isVisible());
    m_docks.remove(dockWidget);
    clearFloatingDockOwner(dockWidget);
    removeDockFromArea(dockWidget, true);
    removeDockEventFilters(dockWidget);
    if (wasFloating)
    {
        dockWidget->setWindowOpacity(1.0);
        dockWidget->hide();
    }
    updatePlaceholderState();
    if (known) Q_EMIT dockWidgetRemoved(dockWidget);
    if (known) Q_EMIT dockLayoutChanged();
}

QList<AntDockWidget*> AntDockManager::dockWidgets() const
{
    QList<AntDockWidget*> result;
    for (AntDockWidget* dock : m_docks)
    {
        if (dock) result.append(dock);
    }
    return result;
}

QList<AntDockWidget*> AntDockManager::floatingDockWidgets() const
{
    QList<AntDockWidget*> result;
    for (AntDockWidget* dock : dockWidgets())
    {
        if (isDockWidgetFloating(dock))
        {
            result.append(dock);
        }
    }
    return result;
}

bool AntDockManager::isDockWidgetFloating(AntDockWidget* dockWidget) const
{
    return dockWidget && m_docks.contains(dockWidget) && (!areaForDock(dockWidget) || dockWidget->isFloating());
}

void AntDockManager::setDockWidgetFloating(AntDockWidget* dockWidget, bool floating, const QRect& globalGeometry)
{
    if (!dockWidget) return;

    if (floating)
    {
        floatDockWidget(dockWidget, globalGeometry);
        return;
    }

    if (!m_docks.contains(dockWidget))
    {
        insertDockWidget(dockWidget, firstDockArea(), DockPlacement::Left);
        return;
    }

    if (!isDockWidgetFloating(dockWidget))
    {
        return;
    }

    DockArea* targetArea = firstDockArea();
    if (!targetArea)
    {
        insertDockWidget(dockWidget, nullptr, DockPlacement::Left);
        return;
    }

    insertDockWidget(dockWidget, targetArea, DockPlacement::Center);
}

bool AntDockManager::isDockWidgetClosable(AntDockWidget* dockWidget) const
{
    return dockWidgetFeatureEnabled(dockWidget, QDockWidget::DockWidgetClosable);
}

void AntDockManager::setDockWidgetClosable(AntDockWidget* dockWidget, bool closable)
{
    setDockWidgetFeatureEnabled(dockWidget, QDockWidget::DockWidgetClosable, closable);
}

bool AntDockManager::isDockWidgetFloatable(AntDockWidget* dockWidget) const
{
    return dockWidgetFeatureEnabled(dockWidget, QDockWidget::DockWidgetFloatable);
}

void AntDockManager::setDockWidgetFloatable(AntDockWidget* dockWidget, bool floatable)
{
    setDockWidgetFeatureEnabled(dockWidget, QDockWidget::DockWidgetFloatable, floatable);
}

bool AntDockManager::isDockWidgetMovable(AntDockWidget* dockWidget) const
{
    return dockWidgetFeatureEnabled(dockWidget, QDockWidget::DockWidgetMovable);
}

void AntDockManager::setDockWidgetMovable(AntDockWidget* dockWidget, bool movable)
{
    setDockWidgetFeatureEnabled(dockWidget, QDockWidget::DockWidgetMovable, movable);
}

int AntDockManager::dockWidgetTabIndex(AntDockWidget* dockWidget) const
{
    DockArea* area = areaForDock(dockWidget);
    return area ? area->dockIndex(dockWidget) : -1;
}

bool AntDockManager::moveDockWidgetTab(AntDockWidget* dockWidget, int index)
{
    DockArea* area = areaForDock(dockWidget);
    if (!dockWidget || !area || !dockWidgetFeatureEnabled(dockWidget, QDockWidget::DockWidgetMovable))
    {
        return false;
    }

    const int from = area->dockIndex(dockWidget);
    if (!area->moveDock(dockWidget, index))
    {
        return false;
    }

    const int to = area->dockIndex(dockWidget);
    Q_EMIT dockWidgetTabMoved(dockWidget, from, to);
    Q_EMIT dockLayoutChanged();
    return true;
}

QWidget* AntDockManager::centralContent() const
{
    return m_workspace ? m_workspace->contentWidget() : nullptr;
}

void AntDockManager::setCentralContent(QWidget* widget)
{
    if (!m_workspace) return;
    m_workspace->setContentWidget(widget);
    m_workspace->updateTheme();
    updatePlaceholderState();
}

bool AntDockManager::isPlaceholderVisible() const
{
    return m_placeholderVisible;
}

void AntDockManager::setPlaceholderVisible(bool visible)
{
    if (m_placeholderVisible == visible) return;
    m_placeholderVisible = visible;
    updatePlaceholderState();
    Q_EMIT placeholderVisibleChanged(m_placeholderVisible);
}

bool AntDockManager::isDropGuideVisible() const
{
    return isDropGuideEnabled();
}

bool AntDockManager::isDropGuideEnabled() const
{
    return m_dropGuideVisible;
}

void AntDockManager::setDropGuideVisible(bool visible)
{
    setDropGuideEnabled(visible);
}

void AntDockManager::setDropGuideEnabled(bool enabled)
{
    if (m_dropGuideVisible == enabled) return;
    m_dropGuideVisible = enabled;
    if (!m_dropGuideVisible && m_dropGuideOverlay)
    {
        m_dropGuideOverlay->clearActivePlacement();
        m_dropGuideOverlay->hide();
    }
    Q_EMIT dropGuideEnabledChanged(m_dropGuideVisible);
    Q_EMIT dropGuideVisibleChanged(m_dropGuideVisible);
}

AntDockManager::DockPlacement AntDockManager::activeDropGuide() const
{
    return m_dropGuideOverlay ? m_dropGuideOverlay->activePlacement() : DockPlacement::None;
}

bool AntDockManager::isDropPreviewVisible() const
{
    return m_dropPreviewWindow && m_dropPreviewWindow->isVisible();
}

QRect AntDockManager::dropPreviewRect() const
{
    return m_dropPreviewWindow ? m_dropPreviewWindow->previewGlobalRect() : QRect();
}

bool AntDockManager::savePerspective(const QString& name)
{
    const QString key = name.trimmed();
    if (key.isEmpty()) return false;

    DockLayoutNode rootNode = captureDockLayoutNode(m_rootDockWidget);
    QList<FloatingDockSnapshot> floatingSnapshots;
    for (AntDockWidget* dock : dockWidgets())
    {
        if (!dock) continue;

        const QString id = dockPersistentId(dock);
        if (id.isEmpty()) continue;

        if (!areaForDock(dock) || dock->isFloating())
        {
            FloatingDockSnapshot snapshot;
            snapshot.dockId = id;
            snapshot.geometry = dock->isWindow()
                ? dock->geometry()
                : QRect(dock->mapToGlobal(QPoint(0, 0)), dock->size().expandedTo(QSize(240, 140)));
            snapshot.visible = dock->isVisible();
            floatingSnapshots.append(snapshot);
        }
    }

    const QByteArray state = serializeDockPerspective(rootNode, floatingSnapshots);
    if (state.isEmpty()) return false;

    m_perspectives.insert(key, state);
    Q_EMIT perspectiveSaved(key);
    return true;
}

bool AntDockManager::restorePerspective(const QString& name)
{
    const QString key = name.trimmed();
    const QByteArray state = m_perspectives.value(key);
    if (state.isEmpty()) return false;

    if (isLegacyDockPerspective(state))
    {
        updatePlaceholderState();
        Q_EMIT perspectiveRestored(key);
        return true;
    }

    DockLayoutNode rootNode;
    QList<FloatingDockSnapshot> floatingSnapshots;
    if (!deserializeDockPerspective(state, &rootNode, &floatingSnapshots))
    {
        return false;
    }

    QHash<QString, AntDockWidget*> docksById;
    for (AntDockWidget* dock : dockWidgets())
    {
        const QString id = dockPersistentId(dock);
        if (!id.isEmpty() && !docksById.contains(id))
        {
            docksById.insert(id, dock);
        }
    }

    QSet<QString> stateDockIds;
    collectDockIds(rootNode, &stateDockIds);
    for (const FloatingDockSnapshot& snapshot : floatingSnapshots)
    {
        if (!snapshot.dockId.isEmpty())
        {
            stateDockIds.insert(snapshot.dockId);
        }
    }

    bool hasMatchedDock = stateDockIds.isEmpty();
    for (const QString& id : stateDockIds)
    {
        if (docksById.contains(id))
        {
            hasMatchedDock = true;
            break;
        }
    }
    if (!hasMatchedDock)
    {
        return false;
    }

    stopDockDragTracking();
    hideDropGuide();

    QHash<AntDockWidget*, QRect> fallbackFloatingGeometry;
    QHash<AntDockWidget*, bool> fallbackVisible;
    for (AntDockWidget* dock : dockWidgets())
    {
        if (!dock) continue;

        fallbackVisible.insert(dock, dock->isVisible());
        fallbackFloatingGeometry.insert(
            dock,
            dock->isWindow()
                ? dock->geometry()
                : QRect(dock->mapToGlobal(QPoint(0, 0)), dock->size().expandedTo(QSize(240, 140))));

        if (DockArea* area = areaForDock(dock))
        {
            area->removeDock(dock);
        }
        else
        {
            setEmbeddedDockTitleBarVisible(dock, true);
            dock->setParent(nullptr);
        }
        clearFloatingDockOwner(dock);
    }
    m_dockAreas.clear();

    QWidget* oldRoot = m_rootDockWidget;
    setRootDockWidget(nullptr);
    if (oldRoot)
    {
        oldRoot->deleteLater();
    }

    QSet<QString> placedDockIds;
    std::function<QWidget*(const DockLayoutNode&)> buildNode = [&](const DockLayoutNode& node) -> QWidget* {
        switch (node.type)
        {
        case DockLayoutNodeType::Area:
        {
            DockArea* area = createDockArea();
            for (const QString& id : node.dockIds)
            {
                AntDockWidget* dock = docksById.value(id, nullptr);
                if (!dock || placedDockIds.contains(id)) continue;

                clearFloatingDockOwner(dock);
                dock->setWindowOpacity(1.0);
                area->addDock(dock);
                m_dockAreas.insert(dock, area);
                placedDockIds.insert(id);
            }

            if (area->count() == 0)
            {
                area->deleteLater();
                return nullptr;
            }

            area->setCurrentIndex(qBound(0, node.currentIndex, area->count() - 1));
            return area;
        }
        case DockLayoutNodeType::Splitter:
        {
            auto* splitter = new QSplitter(node.orientation, this);
            splitter->setObjectName(QStringLiteral("AntDockSplitter"));
            splitter->setChildrenCollapsible(false);
            splitter->setHandleWidth(4);

            for (const DockLayoutNode& child : node.children)
            {
                QWidget* childWidget = buildNode(child);
                if (childWidget)
                {
                    splitter->addWidget(childWidget);
                }
            }

            if (splitter->count() == 0)
            {
                splitter->deleteLater();
                return nullptr;
            }
            if (splitter->count() == 1)
            {
                QWidget* onlyChild = splitter->widget(0);
                onlyChild->setParent(nullptr);
                splitter->deleteLater();
                return onlyChild;
            }

            if (node.sizes.size() == splitter->count())
            {
                splitter->setSizes(node.sizes);
            }
            return splitter;
        }
        case DockLayoutNodeType::Empty:
        default:
            return nullptr;
        }
    };

    QWidget* restoredRoot = buildNode(rootNode);
    setRootDockWidget(restoredRoot);

    for (const FloatingDockSnapshot& snapshot : floatingSnapshots)
    {
        AntDockWidget* dock = docksById.value(snapshot.dockId, nullptr);
        if (!dock || placedDockIds.contains(snapshot.dockId)) continue;

        QRect geometry = snapshot.geometry;
        if (geometry.isEmpty())
        {
            geometry = fallbackFloatingGeometry.value(dock, QRect(pos(), dock->size().expandedTo(QSize(240, 140))));
        }
        floatDockWidget(dock, geometry);
        if (!snapshot.visible)
        {
            dock->hide();
        }
        placedDockIds.insert(snapshot.dockId);
    }

    for (AntDockWidget* dock : dockWidgets())
    {
        if (!dock) continue;
        const QString id = dockPersistentId(dock);
        if (id.isEmpty() || placedDockIds.contains(id)) continue;

        QRect geometry = fallbackFloatingGeometry.value(
            dock,
            QRect(mapToGlobal(QPoint(24, 24)), dock->size().expandedTo(QSize(240, 140))));
        floatDockWidget(dock, geometry);
        if (!fallbackVisible.value(dock, true))
        {
            dock->hide();
        }
    }

    updateTheme();
    updatePlaceholderState();
    Q_EMIT dockLayoutChanged();
    Q_EMIT perspectiveRestored(key);
    return true;
}

bool AntDockManager::removePerspective(const QString& name)
{
    const QString key = name.trimmed();
    if (!m_perspectives.contains(key)) return false;

    m_perspectives.remove(key);
    Q_EMIT perspectiveRemoved(key);
    return true;
}

void AntDockManager::clearPerspectives()
{
    const QStringList names = perspectiveNames();
    m_perspectives.clear();
    for (const QString& name : names)
    {
        Q_EMIT perspectiveRemoved(name);
    }
}

QStringList AntDockManager::perspectiveNames() const
{
    QStringList names = m_perspectives.keys();
    names.sort(Qt::CaseInsensitive);
    return names;
}

QByteArray AntDockManager::perspectiveState(const QString& name) const
{
    return m_perspectives.value(name.trimmed());
}

bool AntDockManager::setPerspectiveState(const QString& name, const QByteArray& state)
{
    const QString key = name.trimmed();
    if (key.isEmpty() || state.isEmpty()) return false;

    m_perspectives.insert(key, state);
    Q_EMIT perspectiveSaved(key);
    return true;
}

bool AntDockManager::eventFilter(QObject* watched, QEvent* event)
{
    handleGlobalDockDragEvent(watched, event);

    if (auto* tabBar = qobject_cast<QTabBar*>(watched))
    {
        DockArea* area = nullptr;
        QWidget* parent = tabBar->parentWidget();
        while (parent && !area)
        {
            area = dynamic_cast<DockArea*>(parent);
            parent = parent->parentWidget();
        }

        if (area)
        {
            switch (event->type())
            {
            case QEvent::MouseButtonPress:
            {
                auto* mouse = static_cast<QMouseEvent*>(event);
                if (mouse->button() == Qt::LeftButton)
                {
                    const int tab = tabBar->tabAt(mouse->pos());
                    if (tab >= 0)
                    {
                        area->setCurrentIndex(tab);
                        if (auto* dock = qobject_cast<AntDockWidget*>(area->widget(tab)))
                        {
                            if (dockWidgetFeatureEnabled(dock, QDockWidget::DockWidgetMovable))
                            {
                                startDockDragTracking(dock, mouseGlobalPosition(mouse));
                                m_tabReorderArea = area;
                                m_tabReorderIndex = tab;
                            }
                            return true;
                        }
                    }
                }
                break;
            }
            case QEvent::MouseMove:
            {
                auto* mouse = static_cast<QMouseEvent*>(event);
                if (m_draggingDockTitle && (mouse->buttons() & Qt::LeftButton))
                {
                    const QPoint globalPos = mouseGlobalPosition(mouse);
                    const QRect tabBarGlobalRect(tabBar->mapToGlobal(QPoint(0, 0)), tabBar->size());
                    const bool inTabReorderBand =
                        m_tabReorderArea == area && tabBarGlobalRect.adjusted(-8, -18, 8, 18).contains(globalPos);
                    if (inTabReorderBand && m_draggedDock)
                    {
                        const int targetTab = tabBar->tabAt(mouse->pos());
                        if (targetTab >= 0 && targetTab != m_tabReorderIndex)
                        {
                            if (moveDockWidgetTab(m_draggedDock, targetTab))
                            {
                                m_tabReorderIndex = dockWidgetTabIndex(m_draggedDock);
                            }
                        }
                        return true;
                    }
                    if ((globalPos - m_dragStartGlobal).manhattanLength() >= QApplication::startDragDistance())
                    {
                        showDropGuideAt(globalPos);
                        return true;
                    }
                }
                break;
            }
            case QEvent::MouseButtonDblClick:
            {
                auto* mouse = static_cast<QMouseEvent*>(event);
                if (mouse->button() == Qt::LeftButton)
                {
                    const int tab = tabBar->tabAt(mouse->pos());
                    if (tab >= 0)
                    {
                        area->setCurrentIndex(tab);
                        if (auto* dock = qobject_cast<AntDockWidget*>(area->widget(tab)))
                        {
                            stopDockDragTracking();
                            if (!dockWidgetFeatureEnabled(dock, QDockWidget::DockWidgetFloatable))
                            {
                                return true;
                            }
                            QRect tabGlobalRect(tabBar->mapToGlobal(tabBar->tabRect(tab).topLeft()),
                                                tabBar->tabRect(tab).size());
                            QSize floatSize = dock->size().expandedTo(QSize(240, 140));
                            const QPoint topLeft(tabGlobalRect.center().x() - floatSize.width() / 2,
                                                 tabGlobalRect.center().y() + 6);
                            floatDockWidget(dock, QRect(topLeft, floatSize));
                            return true;
                        }
                    }
                }
                break;
            }
            case QEvent::MouseButtonRelease:
            {
                auto* mouse = static_cast<QMouseEvent*>(event);
                if (mouse->button() == Qt::LeftButton && m_draggingDockTitle)
                {
                    finishDockDragTracking(mouseGlobalPosition(mouse));
                    return true;
                }
                break;
            }
            case QEvent::ContextMenu:
            {
                auto* context = static_cast<QContextMenuEvent*>(event);
                const int tab = tabBar->tabAt(context->pos());
                if (tab >= 0)
                {
                    area->setCurrentIndex(tab);
                    if (auto* dock = qobject_cast<AntDockWidget*>(area->widget(tab)))
                    {
                        showDockContextMenu(dock, context->globalPos());
                        return true;
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }

    AntDockWidget* dock = dockForWatchedObject(watched);
    if (dock)
    {
        if (event->type() == QEvent::Close)
        {
            stopDockDragTracking();
            updatePlaceholderState();

            if (m_docks.contains(dock))
            {
                if (!dockWidgetFeatureEnabled(dock, QDockWidget::DockWidgetClosable))
                {
                    static_cast<QCloseEvent*>(event)->ignore();
                    return true;
                }

                QPointer<AntDockWidget> guard(dock);
                removeDockWidget(dock);
                if (guard)
                {
                    guard->hide();
                }
                return true;
            }
        }

        if (watched == dock->titleBarWidget())
        {
            handleDockTitleMouseEvent(dock, event);
        }

        if (event->type() == QEvent::Show || event->type() == QEvent::Hide ||
            event->type() == QEvent::ParentChange)
        {
            updatePlaceholderState();
            if (event->type() == QEvent::Hide && !m_draggingDockTitle)
            {
                hideDropGuide();
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void AntDockManager::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    if (m_dropGuideOverlay)
    {
        m_dropGuideOverlay->setGeometry(rect());
    }
}

bool AntDockManager::prepareDockWidget(AntDockWidget* dockWidget)
{
    if (!dockWidget) return false;

    const bool added = !m_docks.contains(dockWidget);
    if (added)
    {
        m_docks.insert(dockWidget);
        installDockEventFilters(dockWidget);

        connect(dockWidget, &QObject::destroyed, this, [this, dockWidget]() {
            m_docks.remove(dockWidget);
            m_dockAreas.remove(dockWidget);
            updatePlaceholderState();
        });
        connect(dockWidget, &QDockWidget::visibilityChanged, this, [this]() {
            updatePlaceholderState();
        });
        connect(dockWidget, &QDockWidget::topLevelChanged, this, [this, dockWidget](bool topLevel) {
            updatePlaceholderState();
            // When the dock transitions to floating, Qt's setWindowFlags() inside
            // AntDockWidget::updateFloatingFrame() destroys and recreates the
            // native HWND, which silently drops any GWLP_HWNDPARENT owner we set
            // before. Without an owner pointing at a top-level HWND, Win32 treats
            // the floating dock as an independent top-level window and its
            // activation chain interferes with sibling controls in the main
            // window — clicks on AntSwitch / AntSegmented stop registering.
            // Reapply the owner here, after the HWND has been recreated.
            if (topLevel)
            {
                // Defer to the next event-loop tick so any pending Qt-internal
                // window handle bookkeeping has settled before we touch
                // GWLP_HWNDPARENT.
                QPointer<AntDockManager> manager(this);
                QPointer<AntDockWidget> guard(dockWidget);
                QTimer::singleShot(0, this, [manager, guard]() {
                    if (manager && guard && guard->isFloating())
                    {
                        setFloatingDockOwner(guard.data(), manager.data());
                    }
                });
            }
            else
            {
                clearFloatingDockOwner(dockWidget);
            }
        });
    }
    else
    {
        installDockEventFilters(dockWidget);
    }

    if (dockWidget->objectName().isEmpty())
    {
        dockWidget->setObjectName(QStringLiteral("AntDockWidget_%1").arg(++m_autoObjectNameCounter));
    }

    const auto& token = antTheme->tokens();
    QPalette pal = dockWidget->palette();
    pal.setColor(QPalette::Window, token.colorBgContainer);
    pal.setColor(QPalette::Base, token.colorBgContainer);
    pal.setColor(QPalette::WindowText, token.colorText);
    pal.setColor(QPalette::Text, token.colorText);
    dockWidget->setPalette(pal);

    return added;
}

AntDockManager::DockArea* AntDockManager::createDockArea()
{
    auto* area = new DockArea(this);
    area->setObjectName(QStringLiteral("AntDockArea"));
    if (area->tabBar())
    {
        area->tabBar()->installEventFilter(this);
    }
    return area;
}

AntDockManager::DockArea* AntDockManager::areaForDock(AntDockWidget* dockWidget) const
{
    return dockWidget ? m_dockAreas.value(dockWidget, nullptr) : nullptr;
}

AntDockManager::DockArea* AntDockManager::firstDockArea() const
{
    if (auto* area = dynamic_cast<DockArea*>(m_rootDockWidget))
    {
        return area;
    }

    if (!m_rootDockWidget) return nullptr;
    const auto tabWidgets = m_rootDockWidget->findChildren<QTabWidget*>();
    for (QTabWidget* tabWidget : tabWidgets)
    {
        if (auto* area = dynamic_cast<DockArea*>(tabWidget))
        {
            return area;
        }
    }
    return nullptr;
}

void AntDockManager::setRootDockWidget(QWidget* widget)
{
    if (m_rootDockWidget == widget) return;

    if (m_rootDockWidget && m_rootDockWidget->parentWidget() == m_workspace)
    {
        m_workspace->setContentWidget(nullptr);
    }

    m_rootDockWidget = widget;
    if (m_rootDockWidget)
    {
        m_workspace->setContentWidget(m_rootDockWidget);
        m_rootDockWidget->show();
    }
    else if (m_workspace)
    {
        m_workspace->setContentWidget(nullptr);
    }
    updatePlaceholderState();
}

void AntDockManager::insertDockWidget(AntDockWidget* dockWidget, DockArea* targetArea, DockPlacement placement, bool containerDrop)
{
    if (!dockWidget) return;
    if (placement == DockPlacement::None) placement = DockPlacement::Left;
    clearFloatingDockOwner(dockWidget);

    const bool added = prepareDockWidget(dockWidget);
    DockArea* oldArea = areaForDock(dockWidget);
    if (oldArea)
    {
        oldArea->removeDock(dockWidget);
        m_dockAreas.remove(dockWidget);
        if (oldArea != targetArea && oldArea->count() == 0)
        {
            pruneEmptyArea(oldArea);
        }
    }

    if (!m_rootDockWidget)
    {
        DockArea* area = createDockArea();
        area->addDock(dockWidget);
        m_dockAreas.insert(dockWidget, area);
        setRootDockWidget(area);
        updatePlaceholderState();
        if (added) Q_EMIT dockWidgetAdded(dockWidget);
        Q_EMIT dockWidgetDocked(dockWidget, placement);
        Q_EMIT dockLayoutChanged();
        return;
    }

    if (!targetArea && (!containerDrop || placement == DockPlacement::Center))
    {
        targetArea = firstDockArea();
    }

    if (placement == DockPlacement::Center && targetArea)
    {
        targetArea->addDock(dockWidget);
        m_dockAreas.insert(dockWidget, targetArea);
        updatePlaceholderState();
        if (added) Q_EMIT dockWidgetAdded(dockWidget);
        Q_EMIT dockWidgetDocked(dockWidget, DockPlacement::Center);
        Q_EMIT dockLayoutChanged();
        return;
    }

    DockArea* newArea = createDockArea();
    newArea->addDock(dockWidget);
    m_dockAreas.insert(dockWidget, newArea);

    QWidget* targetWidget = containerDrop ? m_rootDockWidget : (targetArea ? static_cast<QWidget*>(targetArea) : m_rootDockWidget);
    splitAreaWithWidget(targetWidget, newArea, placement);
    updateTheme();
    updatePlaceholderState();
    if (added) Q_EMIT dockWidgetAdded(dockWidget);
    Q_EMIT dockWidgetDocked(dockWidget, placement);
    Q_EMIT dockLayoutChanged();
}

void AntDockManager::splitAreaWithWidget(QWidget* targetWidget, QWidget* newWidget, DockPlacement placement)
{
    if (!newWidget)
    {
        return;
    }

    if (!targetWidget || !m_rootDockWidget)
    {
        setRootDockWidget(newWidget);
        return;
    }

    const Qt::Orientation orientation =
        (placement == DockPlacement::Top || placement == DockPlacement::Bottom)
            ? Qt::Vertical
            : Qt::Horizontal;
    const bool before = placement == DockPlacement::Left || placement == DockPlacement::Top;
    const auto targetSpan = [orientation](QWidget* widget) {
        if (!widget) return 0;
        return orientation == Qt::Horizontal ? widget->width() : widget->height();
    };
    const auto splitSizes = [](int span, int handleWidth) {
        const int available = qMax(2, span - qMax(0, handleWidth));
        const int first = qMax(1, available / 2);
        const int second = qMax(1, available - first);
        return QList<int>{first, second};
    };

    if (auto* parentSplitter = qobject_cast<QSplitter*>(targetWidget->parentWidget()))
    {
        const int index = parentSplitter->indexOf(targetWidget);
        if (parentSplitter->orientation() == orientation && index >= 0)
        {
            QList<int> sizes = parentSplitter->sizes();
            const int span = index < sizes.size() && sizes.at(index) > 0
                ? sizes.at(index)
                : targetSpan(targetWidget);
            const QList<int> halves = splitSizes(span, parentSplitter->handleWidth());
            parentSplitter->insertWidget(before ? index : index + 1, newWidget);
            parentSplitter->setChildrenCollapsible(false);
            if (sizes.size() == parentSplitter->count() - 1 && index < sizes.size())
            {
                sizes[index] = before ? halves.at(1) : halves.at(0);
                sizes.insert(before ? index : index + 1, before ? halves.at(0) : halves.at(1));
                parentSplitter->setSizes(sizes);
            }
            return;
        }
    }

    auto* splitter = new QSplitter(orientation, this);
    splitter->setObjectName(QStringLiteral("AntDockSplitter"));
    splitter->setChildrenCollapsible(false);
    splitter->setHandleWidth(4);

    QWidget* parent = targetWidget->parentWidget();
    QSplitter* parentSplitter = qobject_cast<QSplitter*>(parent);
    const int oldIndex = parentSplitter ? parentSplitter->indexOf(targetWidget) : -1;
    const QList<int> parentSizes = parentSplitter ? parentSplitter->sizes() : QList<int>();
    const int span = targetSpan(targetWidget);

    if (targetWidget == m_rootDockWidget && m_workspace && m_workspace->contentWidget() == targetWidget)
    {
        m_workspace->setContentWidget(nullptr);
    }

    targetWidget->setParent(nullptr);
    if (before)
    {
        splitter->addWidget(newWidget);
        splitter->addWidget(targetWidget);
    }
    else
    {
        splitter->addWidget(targetWidget);
        splitter->addWidget(newWidget);
    }
    splitter->setSizes(splitSizes(span, splitter->handleWidth()));

    if (parentSplitter && oldIndex >= 0)
    {
        parentSplitter->insertWidget(oldIndex, splitter);
        if (parentSizes.size() == parentSplitter->count())
        {
            parentSplitter->setSizes(parentSizes);
        }
    }
    else if (targetWidget == m_rootDockWidget)
    {
        setRootDockWidget(splitter);
    }
    else
    {
        setRootDockWidget(splitter);
    }
}

void AntDockManager::removeDockFromArea(AntDockWidget* dockWidget, bool detach)
{
    DockArea* area = areaForDock(dockWidget);
    if (!area) return;

    area->removeDock(dockWidget);
    m_dockAreas.remove(dockWidget);
    if (detach && dockWidget)
    {
        dockWidget->setParent(nullptr);
    }
    if (area->count() == 0)
    {
        pruneEmptyArea(area);
    }
}

void AntDockManager::pruneEmptyArea(DockArea* area)
{
    if (!area || area->count() > 0) return;

    QWidget* parent = area->parentWidget();
    if (area == m_rootDockWidget)
    {
        setRootDockWidget(nullptr);
        area->deleteLater();
        return;
    }

    area->setParent(nullptr);
    area->deleteLater();
    collapseSplitter(parent);
}

void AntDockManager::collapseSplitter(QWidget* splitterWidget)
{
    auto* splitter = qobject_cast<QSplitter*>(splitterWidget);
    if (!splitter) return;

    if (splitter->count() > 1)
    {
        return;
    }

    QWidget* replacement = splitter->count() == 1 ? splitter->widget(0) : nullptr;
    QWidget* parent = splitter->parentWidget();
    auto* parentSplitter = qobject_cast<QSplitter*>(parent);
    const int index = parentSplitter ? parentSplitter->indexOf(splitter) : -1;

    if (replacement)
    {
        replacement->setParent(nullptr);
    }

    if (splitter == m_rootDockWidget)
    {
        setRootDockWidget(replacement);
    }
    else if (parentSplitter && index >= 0)
    {
        splitter->setParent(nullptr);
        if (replacement)
        {
            parentSplitter->insertWidget(index, replacement);
        }
        collapseSplitter(parentSplitter);
    }
    else if (replacement)
    {
        setRootDockWidget(replacement);
    }

    splitter->deleteLater();
}

AntDockWidget* AntDockManager::dockForWatchedObject(QObject* watched) const
{
    if (auto* dock = qobject_cast<AntDockWidget*>(watched))
    {
        return m_docks.contains(dock) ? dock : nullptr;
    }

    for (AntDockWidget* dock : m_docks)
    {
        if (dock && dock->titleBarWidget() == watched)
        {
            return dock;
        }
    }
    return nullptr;
}

void AntDockManager::installDockEventFilters(AntDockWidget* dockWidget)
{
    if (!dockWidget) return;

    dockWidget->removeEventFilter(this);
    dockWidget->installEventFilter(this);

    if (QWidget* titleBar = dockWidget->titleBarWidget())
    {
        titleBar->removeEventFilter(this);
        titleBar->installEventFilter(this);
    }
}

void AntDockManager::removeDockEventFilters(AntDockWidget* dockWidget)
{
    if (!dockWidget) return;

    dockWidget->removeEventFilter(this);
    if (QWidget* titleBar = dockWidget->titleBarWidget())
    {
        titleBar->removeEventFilter(this);
    }
}

void AntDockManager::handleDockTitleMouseEvent(AntDockWidget* dockWidget, QEvent* event)
{
    if (!dockWidget || !event) return;

    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (mouse->button() == Qt::LeftButton)
        {
            startDockDragTracking(dockWidget, mouseGlobalPosition(mouse));
        }
        break;
    }
    case QEvent::MouseMove:
    {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (m_draggingDockTitle && (mouse->buttons() & Qt::LeftButton))
        {
            const QPoint globalPos = mouseGlobalPosition(mouse);
            if ((globalPos - m_dragStartGlobal).manhattanLength() >= QApplication::startDragDistance())
            {
                showDropGuideAt(globalPos);
            }
        }
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (mouse->button() == Qt::LeftButton)
        {
            finishDockDragTracking(mouseGlobalPosition(mouse));
        }
        break;
    }
    case QEvent::MouseButtonDblClick:
    {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (mouse->button() == Qt::LeftButton)
        {
            stopDockDragTracking();
        }
        break;
    }
    case QEvent::ContextMenu:
    {
        auto* context = static_cast<QContextMenuEvent*>(event);
        showDockContextMenu(dockWidget, context->globalPos());
        break;
    }
    case QEvent::Close:
        stopDockDragTracking();
        break;
    default:
        break;
    }
}

bool AntDockManager::handleGlobalDockDragEvent(QObject* watched, QEvent* event)
{
    Q_UNUSED(watched)
    if (!m_draggingDockTitle || !event) return false;

    switch (event->type())
    {
    case QEvent::MouseMove:
    {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (!(mouse->buttons() & Qt::LeftButton))
        {
            break;
        }

        const QPoint globalPos = mouseGlobalPosition(mouse);
        if (auto* tabBar = qobject_cast<QTabBar*>(watched))
        {
            const QRect tabBarGlobalRect(tabBar->mapToGlobal(QPoint(0, 0)), tabBar->size());
            if (m_tabReorderArea && tabBarGlobalRect.adjusted(-8, -18, 8, 18).contains(globalPos))
            {
                break;
            }
        }
        if ((globalPos - m_dragStartGlobal).manhattanLength() >= QApplication::startDragDistance())
        {
            showDropGuideAt(globalPos);
        }
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (mouse->button() == Qt::LeftButton)
        {
            finishDockDragTracking(mouseGlobalPosition(mouse));
        }
        break;
    }
    case QEvent::MouseButtonDblClick:
        stopDockDragTracking();
        break;
    case QEvent::KeyPress:
        if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape)
        {
            stopDockDragTracking();
        }
        break;
    default:
        break;
    }

    return false;
}

void AntDockManager::startDockDragTracking(AntDockWidget* dockWidget, const QPoint& globalPos)
{
    if (!dockWidgetFeatureEnabled(dockWidget, QDockWidget::DockWidgetMovable))
    {
        return;
    }

    if (m_draggingDockTitle && m_draggedDock && m_draggedDock != dockWidget)
    {
        stopDockDragTracking();
    }

    m_draggingDockTitle = true;
    m_dockDragActivated = false;
    setProperty("antDockDragActivated", false);
    m_draggedDock = dockWidget;
    m_dragStartGlobal = globalPos;
    m_lastDropGuideGlobal = QPoint();
    m_hasLastDropGuideGlobal = false;
    m_dragPreviewOffset = dockWidget ? globalPos - dockWidget->mapToGlobal(QPoint(0, 0)) : QPoint(24, 18);
    m_draggedDockPreviousOpacity = dockWidget ? dockWidget->windowOpacity() : 1.0;
    m_draggedDockOpacityChanged = false;
    clearRememberedDropTarget();
    if (!m_appEventFilterInstalled && qApp)
    {
        qApp->installEventFilter(this);
        m_appEventFilterInstalled = true;
    }
}

void AntDockManager::finishDockDragTracking(const QPoint& globalPos)
{
    if (!m_draggingDockTitle)
    {
        return;
    }

    if (!m_dockDragActivated)
    {
        stopDockDragTracking();
        return;
    }

    QPointer<AntDockManager> manager(this);
    QPointer<AntDockWidget> draggedDock(m_draggedDock);
    DropTarget target = dropTargetAt(globalPos);
    const bool releaseInsideManager = rect().contains(mapFromGlobal(globalPos));
    if (!target.valid && releaseInsideManager)
    {
        target = rememberedDropTarget();
    }
    QPointer<AntDockWidget> targetDock(target.dockWidget);
    const DockPlacement placement = target.placement;
    const bool containerDrop = target.containerTarget;
    const bool hasGuidedTarget = target.valid && placement != DockPlacement::None;
    const QRect floatGeometry = floatingGeometryForDock(m_draggedDock, globalPos);

    stopDockDragTracking();

    if (!draggedDock)
    {
        return;
    }

    if (!hasGuidedTarget)
    {
        const bool alreadyFloating = draggedDock->property("antDockFloatingOwnedByManager").toBool() ||
                                     (draggedDock->isFloating() && !areaForDock(draggedDock));
        if (alreadyFloating)
        {
            return;
        }

        QTimer::singleShot(0, this, [manager, draggedDock, floatGeometry]() {
            if (!manager || !draggedDock)
            {
                return;
            }
            manager->floatDockWidget(draggedDock, floatGeometry);
        });
        return;
    }

    QTimer::singleShot(0, this, [manager, draggedDock, targetDock, placement, containerDrop]() {
        if (!manager || !draggedDock)
        {
            return;
        }
        manager->applyDropTarget(draggedDock, targetDock, placement, containerDrop);
    });
}

void AntDockManager::stopDockDragTracking()
{
    setDraggedDockTranslucent(false);

    if (m_appEventFilterInstalled && qApp)
    {
        qApp->removeEventFilter(this);
        m_appEventFilterInstalled = false;
    }

    m_draggingDockTitle = false;
    m_dockDragActivated = false;
    setProperty("antDockDragActivated", false);
    m_draggedDock = nullptr;
    m_lastDropGuideGlobal = QPoint();
    m_hasLastDropGuideGlobal = false;
    m_tabReorderArea = nullptr;
    m_tabReorderIndex = -1;
    m_dragPreviewOffset = QPoint();
    if (m_dragPreviewWindow)
    {
        m_dragPreviewWindow->end();
    }
    hideDropGuide();
    clearRememberedDropTarget();
}

void AntDockManager::applyDropTarget(AntDockWidget* dockWidget, AntDockWidget* targetDock, DockPlacement placement, bool containerDrop)
{
    if (!dockWidget || placement == DockPlacement::None)
    {
        return;
    }

    if (targetDock == dockWidget)
    {
        return;
    }

    dockWidget->setWindowOpacity(1.0);
    if (targetDock && (!m_docks.contains(targetDock) || targetDock == dockWidget))
    {
        targetDock = nullptr;
    }

    if (!targetDock && placement == DockPlacement::Center)
    {
        for (AntDockWidget* dock : dockWidgets())
        {
            if (dock && dock != dockWidget && dock->isVisible() && !dock->isFloating())
            {
                targetDock = dock;
                break;
            }
        }
    }

    insertDockWidget(dockWidget, areaForDock(targetDock), placement, containerDrop && placement != DockPlacement::Center);
    hideDropGuide();
    if (m_dragPreviewWindow)
    {
        m_dragPreviewWindow->end();
    }
    setDraggedDockTranslucent(false);
    dockWidget->raise();
    updatePlaceholderState();
}

void AntDockManager::floatDockWidget(AntDockWidget* dockWidget, const QRect& globalGeometry)
{
    if (!dockWidget)
    {
        return;
    }
    if (!dockWidgetFeatureEnabled(dockWidget, QDockWidget::DockWidgetFloatable))
    {
        return;
    }
    const bool added = prepareDockWidget(dockWidget);

    if (m_draggingDockTitle && m_draggedDock == dockWidget)
    {
        stopDockDragTracking();
    }

    QRect targetGeometry = globalGeometry;
    if (targetGeometry.isEmpty())
    {
        targetGeometry = QRect(dockWidget->mapToGlobal(QPoint(0, 0)),
                               dockWidget->size().expandedTo(QSize(240, 140)));
    }

    removeDockFromArea(dockWidget, true);
    dockWidget->setProperty("antDockEmbeddedByManager", false);
    setEmbeddedDockTitleBarVisible(dockWidget, true);
    dockWidget->setWindowOpacity(1.0);
    dockWidget->setFloating(true);
    dockWidget->setMinimumSize(QSize(180, 120));
    dockWidget->setGeometry(targetGeometry);
    setFloatingDockOwner(dockWidget, this);
    dockWidget->show();
    setFloatingDockOwner(dockWidget, this);
    dockWidget->raise();
    dockWidget->activateWindow();
    installDockEventFilters(dockWidget);
    updatePlaceholderState();
    if (added) Q_EMIT dockWidgetAdded(dockWidget);
    Q_EMIT dockWidgetFloated(dockWidget);
    Q_EMIT dockLayoutChanged();
}

bool AntDockManager::dockWidgetFeatureEnabled(AntDockWidget* dockWidget, QDockWidget::DockWidgetFeature feature) const
{
    return dockWidget && dockWidget->features().testFlag(feature);
}

void AntDockManager::setDockWidgetFeatureEnabled(AntDockWidget* dockWidget,
                                                 QDockWidget::DockWidgetFeature feature,
                                                 bool enabled)
{
    if (!dockWidget) return;

    QDockWidget::DockWidgetFeatures features = dockWidget->features();
    const bool wasEnabled = features.testFlag(feature);
    if (wasEnabled == enabled) return;

    if (enabled)
    {
        features |= feature;
    }
    else
    {
        features &= ~feature;
    }
    dockWidget->setFeatures(features);
    Q_EMIT dockWidgetFeatureChanged(dockWidget);
}

void AntDockManager::showDockContextMenu(AntDockWidget* dockWidget, const QPoint& globalPos)
{
    if (!dockWidget || !m_docks.contains(dockWidget)) return;

    Q_EMIT dockWidgetContextMenuRequested(dockWidget, globalPos);

    auto* popup = new AntDockContextMenuPopup(this);
    AntMenu* menu = popup->menu();
    if (!menu)
    {
        popup->deleteLater();
        return;
    }

    QPointer<AntDockManager> manager(this);
    QPointer<AntDockWidget> dock(dockWidget);
    const bool floating = isDockWidgetFloating(dockWidget);
    const bool floatable = isDockWidgetFloatable(dockWidget);
    const bool movable = isDockWidgetMovable(dockWidget);
    const bool closable = isDockWidgetClosable(dockWidget);
    DockArea* area = areaForDock(dockWidget);
    const int tabIndex = area ? area->dockIndex(dockWidget) : -1;
    QList<AntDockWidget*> otherDocks;
    if (area)
    {
        otherDocks = area->dockWidgets();
    }
    QList<AntDockWidget*> closableOtherDocks;
    for (AntDockWidget* other : otherDocks)
    {
        if (other && other != dockWidget && isDockWidgetClosable(other))
        {
            closableOtherDocks.append(other);
        }
    }

    menu->addItem(QStringLiteral("float"),
                  floating ? tr("Dock to workspace") : tr("Float"),
                  floating ? Ant::IconType::Home : Ant::IconType::CloudUpload,
                  QString(),
                  !floating && !floatable);

    if (area && area->count() > 1)
    {
        menu->addDivider();

        menu->addItem(QStringLiteral("move-left"),
                      tr("Move tab left"),
                      Ant::IconType::Left,
                      QString(),
                      !(movable && tabIndex > 0));

        menu->addItem(QStringLiteral("move-right"),
                      tr("Move tab right"),
                      Ant::IconType::Right,
                      QString(),
                      !(movable && tabIndex >= 0 && tabIndex < area->count() - 1));

        menu->addDivider();
        const auto addSplitAction = [&](const QString& text, DockPlacement placement) {
            QString key;
            switch (placement)
            {
            case DockPlacement::Left:
                key = QStringLiteral("split-left");
                break;
            case DockPlacement::Right:
                key = QStringLiteral("split-right");
                break;
            case DockPlacement::Top:
                key = QStringLiteral("split-top");
                break;
            case DockPlacement::Bottom:
                key = QStringLiteral("split-bottom");
                break;
            default:
                return;
            }
            menu->addItem(key, text, Ant::IconType::None, QString(), !movable);
        };
        addSplitAction(tr("Split left"), DockPlacement::Left);
        addSplitAction(tr("Split right"), DockPlacement::Right);
        addSplitAction(tr("Split top"), DockPlacement::Top);
        addSplitAction(tr("Split bottom"), DockPlacement::Bottom);

        menu->addDivider();
        menu->addItem(QStringLiteral("close-others"),
                      tr("Close other tabs"),
                      Ant::IconType::CloseCircle,
                      QString(),
                      closableOtherDocks.isEmpty(),
                      true);
    }

    menu->addDivider();
    menu->addItem(QStringLiteral("close"),
                  tr("Close"),
                  Ant::IconType::Close,
                  QString(),
                  !closable,
                  true);

    QPointer<AntDockContextMenuPopup> popupGuard(popup);
    connect(menu,
            &AntMenu::itemClicked,
            popup,
            [manager, dock, popupGuard, floating, tabIndex, closableOtherDocks](const QString& key) {
        if (popupGuard)
        {
            popupGuard->close();
        }
        if (!manager || !dock)
        {
            return;
        }

        if (key == QStringLiteral("float"))
        {
            manager->setDockWidgetFloating(dock, !floating);
            return;
        }
        if (key == QStringLiteral("move-left"))
        {
            manager->moveDockWidgetTab(dock, tabIndex - 1);
            return;
        }
        if (key == QStringLiteral("move-right"))
        {
            manager->moveDockWidgetTab(dock, tabIndex + 1);
            return;
        }

        const auto splitPlacement = [](const QString& actionKey) {
            if (actionKey == QStringLiteral("split-left"))
            {
                return DockPlacement::Left;
            }
            if (actionKey == QStringLiteral("split-right"))
            {
                return DockPlacement::Right;
            }
            if (actionKey == QStringLiteral("split-top"))
            {
                return DockPlacement::Top;
            }
            if (actionKey == QStringLiteral("split-bottom"))
            {
                return DockPlacement::Bottom;
            }
            return DockPlacement::None;
        };
        const DockPlacement placement = splitPlacement(key);
        if (placement != DockPlacement::None)
        {
            manager->addDockWidget(dock, dock, placement);
            return;
        }

        if (key == QStringLiteral("close-others"))
        {
            for (AntDockWidget* other : closableOtherDocks)
            {
                if (!other || other == dock)
                {
                    continue;
                }
                manager->removeDockWidget(other);
                other->hide();
            }
            return;
        }

        if (key == QStringLiteral("close"))
        {
            manager->removeDockWidget(dock);
            dock->hide();
        }
    });

    popup->popupAt(globalPos);
}

QRect AntDockManager::floatingGeometryForDock(AntDockWidget* dockWidget, const QPoint& globalPos) const
{
    if (m_dragPreviewWindow)
    {
        const QRect previewGeometry = m_dragPreviewWindow->previewGlobalGeometry();
        if (!previewGeometry.isEmpty())
        {
            return previewGeometry;
        }
    }

    if (!dockWidget)
    {
        return QRect(globalPos - QPoint(120, 28), QSize(240, 140));
    }

    const QSize size = dockWidget->size().expandedTo(QSize(240, 140));
    QPoint topLeft = globalPos - m_dragPreviewOffset;
    if (m_dragPreviewOffset.isNull())
    {
        topLeft = globalPos - QPoint(qMin(80, size.width() / 3), 22);
    }
    return QRect(topLeft, size);
}

void AntDockManager::setDraggedDockTranslucent(bool translucent)
{
    if (!m_draggedDock)
    {
        m_draggedDockOpacityChanged = false;
        return;
    }

    if (translucent)
    {
        if (m_draggedDockOpacityChanged)
        {
            return;
        }

        m_draggedDock->setWindowOpacity(0.68);
        if (m_draggedDock->isWindow())
        {
            m_draggedDockOpacityChanged = true;
            return;
        }

        if (!m_draggedDockOpacityEffect)
        {
            m_draggedDockOpacityEffect = new QGraphicsOpacityEffect(m_draggedDock);
            m_draggedDock->setGraphicsEffect(m_draggedDockOpacityEffect);
        }
        m_draggedDockOpacityEffect->setOpacity(0.68);
        m_draggedDockOpacityChanged = true;
    }
    else if (m_draggedDockOpacityChanged)
    {
        m_draggedDock->setWindowOpacity(m_draggedDockPreviousOpacity);
        QGraphicsOpacityEffect* effect = m_draggedDockOpacityEffect;
        m_draggedDockOpacityEffect = nullptr;
        if (effect && m_draggedDock->graphicsEffect() == effect)
        {
            m_draggedDock->setGraphicsEffect(nullptr);
        }
        else
        {
            delete effect;
        }
        m_draggedDockOpacityChanged = false;
    }
}

void AntDockManager::showDropGuideAt(const QPoint& globalPos)
{
    const bool draggingFloatingDock = m_draggedDock && m_draggedDock->isFloating();
    const bool dockingSurfaceVisible = m_dropGuideOverlay &&
        isVisible() &&
        (!window() || !window()->isMinimized());

    if (!dockingSurfaceVisible && !draggingFloatingDock)
    {
        if (m_dropGuideOverlay)
        {
            m_dropGuideOverlay->clearActivePlacement();
            m_dropGuideOverlay->hide();
        }
        if (m_dropPreviewWindow)
        {
            m_dropPreviewWindow->hideTarget();
        }
        if (m_dragPreviewWindow)
        {
            m_dragPreviewWindow->end();
        }
        return;
    }

    if (m_hasLastDropGuideGlobal && m_lastDropGuideGlobal == globalPos)
    {
        return;
    }
    m_dockDragActivated = true;
    setProperty("antDockDragActivated", true);
    m_lastDropGuideGlobal = globalPos;
    m_hasLastDropGuideGlobal = true;

    if (m_draggedDock && m_draggedDock->isFloating())
    {
        const QPoint topLeft = globalPos - m_dragPreviewOffset;
        if (m_draggedDock->pos() != topLeft)
        {
            m_draggedDock->move(topLeft);
        }
        if (m_dragPreviewWindow && m_dragPreviewWindow->isVisible())
        {
            m_dragPreviewWindow->end();
        }
    }
    else if (m_dragPreviewWindow && m_draggedDock)
    {
        if (!m_dragPreviewWindow->isVisible())
        {
            m_dragPreviewWindow->begin(m_draggedDock, globalPos, m_dragPreviewOffset);
        }
        else
        {
            m_dragPreviewWindow->moveToGlobalPos(globalPos);
        }
    }
    setDraggedDockTranslucent(true);

    if (!dockingSurfaceVisible)
    {
        if (m_dropGuideOverlay)
        {
            m_dropGuideOverlay->clearActivePlacement();
            m_dropGuideOverlay->hide();
        }
        if (m_dropPreviewWindow)
        {
            m_dropPreviewWindow->hideTarget();
        }
        return;
    }

    QRect areaGuideGlobalRect;
    if (AntDockWidget* guideDock = dockWidgetAt(globalPos))
    {
        if (DockArea* area = areaForDock(guideDock))
        {
            areaGuideGlobalRect = QRect(area->mapToGlobal(QPoint(0, 0)), area->size());
        }
    }

    QRect containerGuideGlobalRect;
    if (m_rootDockWidget)
    {
        containerGuideGlobalRect = QRect(m_rootDockWidget->mapToGlobal(QPoint(0, 0)), m_rootDockWidget->size());
    }
    else if (m_workspace)
    {
        containerGuideGlobalRect = QRect(m_workspace->mapToGlobal(QPoint(0, 0)), m_workspace->size());
        areaGuideGlobalRect = containerGuideGlobalRect;
    }
    else
    {
        containerGuideGlobalRect = QRect(mapToGlobal(rect().topLeft()), rect().size());
    }

    if (m_dropGuideVisible)
    {
        m_dropGuideOverlay->setGeometry(rect());
        m_dropGuideOverlay->raise();
        if (!m_dropGuideOverlay->isVisible())
        {
            m_dropGuideOverlay->show();
        }
        m_dropGuideOverlay->updateFromGlobalPos(globalPos, areaGuideGlobalRect, containerGuideGlobalRect);
    }
    else
    {
        m_dropGuideOverlay->clearActivePlacement();
        m_dropGuideOverlay->hide();
    }

    if (m_dropPreviewWindow)
    {
        const DropTarget target = dropTargetAt(globalPos);
        if (target.valid)
        {
            rememberDropTarget(target);
            m_dropPreviewWindow->showTarget(target, QRect(mapToGlobal(rect().topLeft()), rect().size()));
        }
        else
        {
            m_dropPreviewWindow->hideTarget();
        }
    }
}

void AntDockManager::hideDropGuide()
{
    if (!m_dropGuideOverlay) return;

    m_dropGuideOverlay->clearActivePlacement();
    m_dropGuideOverlay->hide();
    if (m_dropPreviewWindow)
    {
        m_dropPreviewWindow->hideTarget();
    }
}

AntDockManager::DropTarget AntDockManager::dropTargetAt(const QPoint& globalPos) const
{
    DropTarget target;
    if (!rect().contains(mapFromGlobal(globalPos))) return target;

    const DockPlacement guidedPlacement = activeDropGuide();
    const bool guidedContainerDrop = guidedPlacement != DockPlacement::None &&
        m_dropGuideOverlay && m_dropGuideOverlay->activePlacementIsEdge();

    AntDockWidget* targetDock = guidedContainerDrop ? nullptr : dockWidgetAt(globalPos);
    QRect targetRect;
    if (guidedContainerDrop && m_rootDockWidget)
    {
        targetRect = QRect(m_rootDockWidget->mapToGlobal(QPoint(0, 0)), m_rootDockWidget->size());
    }
    else if (targetDock)
    {
        if (DockArea* area = areaForDock(targetDock))
        {
            targetRect = QRect(area->mapToGlobal(QPoint(0, 0)), area->size());
        }
        else
        {
            targetRect = QRect(targetDock->mapToGlobal(QPoint(0, 0)), targetDock->size());
        }
    }
    else if (m_rootDockWidget)
    {
        targetRect = QRect(m_rootDockWidget->mapToGlobal(QPoint(0, 0)), m_rootDockWidget->size());
    }
    else if (m_workspace)
    {
        targetRect = QRect(m_workspace->mapToGlobal(QPoint(0, 0)), m_workspace->size());
    }
    else
    {
        targetRect = QRect(mapToGlobal(rect().topLeft()), rect().size());
    }

    if (targetRect.isEmpty()) return target;

    DockPlacement placement = guidedPlacement;
    if (placement == DockPlacement::None)
    {
        placement = placementForTarget(globalPos, targetRect);
    }

    const QRect previewRect = previewRectForTarget(targetRect, placement);
    if (placement == DockPlacement::None || previewRect.isEmpty()) return target;

    target.valid = true;
    target.containerTarget = guidedContainerDrop || (!targetDock && placement != DockPlacement::Center);
    target.dockWidget = targetDock;
    target.placement = placement;
    target.targetGlobalRect = targetRect;
    target.previewGlobalRect = previewRect;
    target.label = dropTargetLabel(targetDock, placement);
    return target;
}

AntDockManager::DropTarget AntDockManager::rememberedDropTarget() const
{
    DropTarget target;
    if (!m_hasLastDropTarget || m_lastDropPlacement == DockPlacement::None)
    {
        return target;
    }

    if (m_lastDropTargetDock && !m_docks.contains(m_lastDropTargetDock))
    {
        return target;
    }

    target.valid = true;
    target.containerTarget = m_lastDropTargetIsContainer;
    target.dockWidget = m_lastDropTargetDock;
    target.placement = m_lastDropPlacement;
    target.targetGlobalRect = m_lastDropTargetRect;
    target.previewGlobalRect = m_lastDropPreviewRect;
    target.label = m_lastDropLabel;
    return target;
}

void AntDockManager::rememberDropTarget(const DropTarget& target)
{
    if (!target.valid || target.placement == DockPlacement::None)
    {
        return;
    }

    m_hasLastDropTarget = true;
    m_lastDropTargetIsContainer = target.containerTarget;
    m_lastDropTargetDock = target.dockWidget;
    m_lastDropPlacement = target.placement;
    m_lastDropTargetRect = target.targetGlobalRect;
    m_lastDropPreviewRect = target.previewGlobalRect;
    m_lastDropLabel = target.label;
}

void AntDockManager::clearRememberedDropTarget()
{
    m_hasLastDropTarget = false;
    m_lastDropTargetIsContainer = false;
    m_lastDropTargetDock = nullptr;
    m_lastDropPlacement = DockPlacement::None;
    m_lastDropTargetRect = QRect();
    m_lastDropPreviewRect = QRect();
    m_lastDropLabel.clear();
}

AntDockWidget* AntDockManager::dockWidgetAt(const QPoint& globalPos) const
{
    AntDockWidget* best = nullptr;
    int bestArea = 0;
    QSet<DockArea*> visited;
    for (auto it = m_dockAreas.constBegin(); it != m_dockAreas.constEnd(); ++it)
    {
        DockArea* area = it.value();
        if (!area || visited.contains(area)) continue;
        visited.insert(area);

        const QRect globalRect(area->mapToGlobal(QPoint(0, 0)), area->size());
        if (!globalRect.contains(globalPos)) continue;

        const int areaSize = globalRect.width() * globalRect.height();
        if (!best || areaSize < bestArea)
        {
            auto* currentDock = qobject_cast<AntDockWidget*>(it.value()->currentWidget());
            if (!currentDock || currentDock == m_draggedDock)
            {
                const auto docks = it.value()->dockWidgets();
                for (AntDockWidget* dock : docks)
                {
                    if (dock && dock != m_draggedDock)
                    {
                        currentDock = dock;
                        break;
                    }
                }
            }
            if (!currentDock) continue;
            best = currentDock;
            bestArea = areaSize;
        }
    }
    return best;
}

AntDockManager::DockPlacement AntDockManager::placementForTarget(const QPoint& globalPos, const QRect& targetGlobalRect) const
{
    if (!targetGlobalRect.contains(globalPos)) return DockPlacement::None;

    const int localX = globalPos.x() - targetGlobalRect.x();
    const int localY = globalPos.y() - targetGlobalRect.y();
    const int edgeX = qMax(48, targetGlobalRect.width() / 4);
    const int edgeY = qMax(42, targetGlobalRect.height() / 4);

    if (localX < edgeX) return DockPlacement::Left;
    if (localX > targetGlobalRect.width() - edgeX) return DockPlacement::Right;
    if (localY < edgeY) return DockPlacement::Top;
    if (localY > targetGlobalRect.height() - edgeY) return DockPlacement::Bottom;
    return DockPlacement::Center;
}

QRect AntDockManager::previewRectForTarget(const QRect& targetGlobalRect, DockPlacement placement) const
{
    if (targetGlobalRect.isEmpty()) return QRect();

    QRect r = targetGlobalRect.adjusted(6, 6, -6, -6);
    if (r.width() <= 12 || r.height() <= 12) return QRect();

    switch (placement)
    {
    case DockPlacement::Left:
        r.setWidth(qMax(42, r.width() / 2));
        break;
    case DockPlacement::Right:
        r.setLeft(r.left() + r.width() / 2);
        break;
    case DockPlacement::Top:
        r.setHeight(qMax(36, r.height() / 2));
        break;
    case DockPlacement::Bottom:
        r.setTop(r.top() + r.height() / 2);
        break;
    case DockPlacement::Center:
        r = r.adjusted(qMax(8, r.width() / 16),
                       qMax(8, r.height() / 16),
                       -qMax(8, r.width() / 16),
                       -qMax(8, r.height() / 16));
        break;
    case DockPlacement::None:
        return QRect();
    }
    return r.normalized();
}

QString AntDockManager::dropTargetLabel(AntDockWidget* dockWidget, DockPlacement placement) const
{
    QString action;
    switch (placement)
    {
    case DockPlacement::Left:
        action = QStringLiteral("Dock left");
        break;
    case DockPlacement::Right:
        action = QStringLiteral("Dock right");
        break;
    case DockPlacement::Top:
        action = QStringLiteral("Dock top");
        break;
    case DockPlacement::Bottom:
        action = QStringLiteral("Dock bottom");
        break;
    case DockPlacement::Center:
        action = QStringLiteral("Tab here");
        break;
    case DockPlacement::None:
        return QString();
    }

    if (dockWidget && !dockWidget->windowTitle().isEmpty())
    {
        return QStringLiteral("%1: %2").arg(action, dockWidget->windowTitle());
    }
    return action;
}

void AntDockManager::updateTheme()
{
    const auto& token = antTheme->tokens();
    QPalette pal = palette();
    pal.setColor(QPalette::Window, token.colorBgLayout);
    pal.setColor(QPalette::Base, token.colorBgLayout);
    pal.setColor(QPalette::WindowText, token.colorText);
    pal.setColor(QPalette::Text, token.colorText);
    setPalette(pal);

    if (m_workspace)
    {
        m_workspace->updateTheme();
    }

    const QString style = QStringLiteral(
        "QMainWindow { background: %1; }"
        "QMainWindow::separator { background: %2; width: 4px; height: 4px; }"
        "QMainWindow::separator:hover { background: %3; }"
        "AntDockManager QTabWidget::pane { border: 1px solid %2; top: -1px; }"
        "AntDockManager QTabBar::tab {"
        "  background: %4;"
        "  color: %5;"
        "  border: 1px solid %2;"
        "  border-bottom: none;"
        "  padding: 6px 12px;"
        "  min-height: 24px;"
        "}"
        "AntDockManager QTabBar::tab:selected {"
        "  background: %6;"
        "  color: %7;"
        "  border-top: 2px solid %3;"
        "}"
        "AntDockManager QTabBar::tab:!selected:hover { background: %8; }")
        .arg(cssColor(token.colorBgLayout),
             cssColor(token.colorSplit),
             cssColor(token.colorPrimary),
             cssColor(token.colorBgElevated),
             cssColor(token.colorTextSecondary),
             cssColor(token.colorBgContainer),
             cssColor(token.colorText),
             cssColor(token.colorFillQuaternary));
    setStyleSheet(style);

    const auto tabBars = findChildren<QTabBar*>();
    for (QTabBar* tabBar : tabBars)
    {
        if (!tabBar) continue;
        tabBar->setDocumentMode(true);
        tabBar->setDrawBase(false);
        tabBar->setExpanding(false);
    }

    for (AntDockWidget* dock : dockWidgets())
    {
        prepareDockWidget(dock);
        dock->update();
    }

    updatePlaceholderState();
    update();
}

void AntDockManager::updatePlaceholderState()
{
    if (!m_workspace) return;
    m_workspace->setPlaceholderActive(m_placeholderVisible &&
                                      !m_workspace->contentWidget() &&
                                      visibleDockWidgetCount() == 0);
}

int AntDockManager::visibleDockWidgetCount() const
{
    int count = 0;
    for (AntDockWidget* dock : m_docks)
    {
        if (dock && !dock->isHidden())
        {
            ++count;
        }
    }
    return count;
}
