#include "AntWindowStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntWindowChrome.h"
#include "widgets/AntWindow.h"

static_assert(AntWindow::TitleBarHeight == AntWindowChrome::TitleBarHeight,
              "AntWindow chrome metrics must stay shared.");
static_assert(AntWindow::TitleBarButtonWidth == AntWindowChrome::TitleBarButtonWidth,
              "AntWindow button metrics must stay shared.");

namespace
{
constexpr auto kForceLegacyFramePolicyProperty = "antWindowForceLegacyFramePolicy";

bool shouldDrawLegacyOutline(const AntWindow* window, bool maximized)
{
    if (!window || maximized || window->isFullScreen())
    {
        return false;
    }
    return window->usesLegacyOpaquePath() || window->property(kForceLegacyFramePolicyProperty).toBool();
}
}

AntWindowStyle::AntWindowStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntWindow>();
}

void AntWindowStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    if (qobject_cast<AntWindow*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntWindowStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntWindow*>(widget))
    {
        widget->removeEventFilter(this);
    }
    AntStyleBase::unpolish(widget);
}

void AntWindowStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntWindow*>(widget))
    {
        drawWindow(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntWindowStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntWindowStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* window = qobject_cast<AntWindow*>(watched);
    if (window && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(window);
        option.rect = window->rect();
        QPainter painter(window);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, window);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntWindowStyle::drawWindow(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* window = qobject_cast<const AntWindow*>(widget);
    if (!window || !painter || !option)
    {
        return;
    }

    const int titleBarH = AntWindow::TitleBarHeight;
    const int w = option->rect.width();
    const bool maximized = window->isMaximized();
    // During an interactive Win10 drag-resize, the backing store cannot keep
    // pace with WM_SIZE — newly exposed pixels (on grow) or pixels DWM still
    // composites (on shrink) read alpha=0 and show through to the desktop,
    // producing visible flicker. While the drag is active we paint with
    // square corners so the entire client rect is fully opaque; the rounded
    // outline is restored on WM_EXITSIZEMOVE.
    const bool liveResize = window->property("antWindowLegacyLiveResize").toBool();
    // The Win10 opaque path has no alpha channel in the backing store, no
    // corner smoother, and an unreliable live-resize state machine. We
    // accept square corners on that path (with the legacy software shadow
    // providing the visible window outline) instead of trying to clip a
    // rounded path against opaque pixels - the latter leaves the four
    // outside-the-rounded-path corners showing the palette fill color and
    // also occasionally gets stuck square after an interrupted resize loop.
    const bool forceSquareCorners = window->usesLegacyOpaquePath();
    const int cornerRadius = (maximized || liveResize || forceSquareCorners) ? 0 : window->cornerRadius();

    AntWindowChrome::PaintOptions chrome;
    chrome.surfaceRect = option->rect;
    chrome.titleBarRect = QRect(0, 0, w, titleBarH);
    chrome.widget = window;
    chrome.title = window->windowTitle();
    chrome.icon = window->windowIcon();
    chrome.cornerRadius = cornerRadius;
    chrome.maximized = maximized;
    chrome.drawLegacyOutline = shouldDrawLegacyOutline(window, maximized);

    const AntWindow::TitleBarButton hoveredButton = window->hoveredTitleBarButton();
    auto appendButton = [&](AntWindow::TitleBarButton source,
                            AntWindowChrome::TitleBarButton target,
                            bool destructive = false,
                            bool active = false) {
        const QRect rect = window->titleBarButtonRect(source);
        if (!rect.isValid())
        {
            return;
        }
        AntWindowChrome::TitleBarButtonState state;
        state.button = target;
        state.rect = rect;
        state.hovered = hoveredButton == source;
        state.active = active;
        state.destructive = destructive;
        chrome.buttons.append(state);
    };

    appendButton(AntWindow::TitleBarButton::Pin,
                 AntWindowChrome::TitleBarButton::Pin,
                 false,
                 window->isAlwaysOnTop());
    appendButton(AntWindow::TitleBarButton::Theme, AntWindowChrome::TitleBarButton::Theme);
    appendButton(AntWindow::TitleBarButton::Minimize, AntWindowChrome::TitleBarButton::Minimize);
    appendButton(AntWindow::TitleBarButton::Maximize, AntWindowChrome::TitleBarButton::Maximize);
    appendButton(AntWindow::TitleBarButton::Close,
                 AntWindowChrome::TitleBarButton::Close,
                 true);

    AntWindowChrome::drawChrome(painter, chrome);
}
