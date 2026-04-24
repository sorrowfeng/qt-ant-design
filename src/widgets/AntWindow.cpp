#include "AntWindow.h"

#include <QCursor>
#include <QEvent>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QScreen>
#include <QShowEvent>
#include <QVBoxLayout>

#include "../styles/AntWindowStyle.h"
#include "core/AntTheme.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#endif

AntWindow::AntWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground, false);
    setAttribute(Qt::WA_Hover, true);
    setStyle(new AntWindowStyle(style()));
    setMinimumSize(400, 300);
    syncTheme();

    m_contentWidget = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(0, TitleBarHeight, 0, 0);
    contentLayout->setSpacing(0);
    QMainWindow::setCentralWidget(m_contentWidget);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        syncTheme();
        update();
    });
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
        layout->addWidget(widget);
    }
}

bool AntWindow::isMaximized() const
{
    return m_windowMaximized;
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

void AntWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        QMainWindow::mousePressEvent(event);
        return;
    }

    const QPoint pos = event->pos();

    if (isButtonArea(pos))
    {
        const TitleBarButton button = buttonAtPosition(pos);
        handleButtonClicked(button);
        event->accept();
        return;
    }

    if (isTitleBarArea(pos))
    {
        m_dragging = true;
        m_dragStartPosition = event->globalPosition().toPoint();
        m_dragStartWindowPos = this->pos();
        event->accept();
        return;
    }

    QMainWindow::mousePressEvent(event);
}

void AntWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton))
    {
        const QPoint delta = event->globalPosition().toPoint() - m_dragStartPosition;

        if (m_windowMaximized)
        {
            // If maximized, restore on drag
            const qreal widthRatio = static_cast<qreal>(m_dragStartWindowPos.x()) / width();
            m_windowMaximized = false;
            showNormal();

            const QSize restoredSize = size();
            m_dragStartWindowPos.setX(static_cast<int>(widthRatio * restoredSize.width()));
            m_dragStartWindowPos.setY(TitleBarHeight / 2);

            move(event->globalPosition().toPoint() - m_dragStartWindowPos);
        }
        else
        {
            move(m_dragStartWindowPos + delta);
        }

        event->accept();
        return;
    }

    QMainWindow::mouseMoveEvent(event);
}

void AntWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_dragging && event->button() == Qt::LeftButton)
    {
        m_dragging = false;
        event->accept();
        return;
    }

    QMainWindow::mouseReleaseEvent(event);
}

void AntWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isTitleBarArea(event->pos()) && !isButtonArea(event->pos()))
    {
        if (m_windowMaximized)
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
        event->accept();
        return;
    }

    QMainWindow::mouseDoubleClickEvent(event);
}

void AntWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
}

void AntWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
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
        case WM_NCHITTEST:
        {
            if (isMaximized())
            {
                *result = HTCLIENT;
                return true;
            }

            const POINT nativeLocalPos{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            POINT localPos = nativeLocalPos;
            ::ScreenToClient(hwnd, &localPos);

            RECT clientRect{0, 0, 0, 0};
            ::GetClientRect(hwnd, &clientRect);
            const int clientWidth = clientRect.right - clientRect.left;
            const int clientHeight = clientRect.bottom - clientRect.top;

            const int borderWidth = 8;
            const bool left = localPos.x < borderWidth;
            const bool right = localPos.x > clientWidth - borderWidth;
            const bool top = localPos.y < borderWidth;
            const bool bottom = localPos.y > clientHeight - borderWidth;

            *result = 0;

            if (left && top)
            {
                *result = HTTOPLEFT;
            }
            else if (left && bottom)
            {
                *result = HTBOTTOMLEFT;
            }
            else if (right && top)
            {
                *result = HTTOPRIGHT;
            }
            else if (right && bottom)
            {
                *result = HTBOTTOMRIGHT;
            }
            else if (left)
            {
                *result = HTLEFT;
            }
            else if (right)
            {
                *result = HTRIGHT;
            }
            else if (top)
            {
                *result = HTTOP;
            }
            else if (bottom)
            {
                *result = HTBOTTOM;
            }

            if (*result != 0)
            {
                return true;
            }

            // Check if in title bar area (but not on buttons)
            if (localPos.y >= 0 && localPos.y < TitleBarHeight)
            {
                const int btnAreaWidth = TitleBarButtonWidth * 3;
                if (localPos.x >= clientWidth - btnAreaWidth)
                {
                    *result = HTCLIENT;
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
        case WM_GETMINMAXINFO:
        {
            auto* minmaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);
            RECT workArea;
            SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
            minmaxInfo->ptMaxPosition.x = workArea.left;
            minmaxInfo->ptMaxPosition.y = workArea.top;
            minmaxInfo->ptMaxSize.x = workArea.right - workArea.left;
            minmaxInfo->ptMaxSize.y = workArea.bottom - workArea.top;
            minmaxInfo->ptMinTrackSize.x = minimumWidth() * devicePixelRatioF();
            minmaxInfo->ptMinTrackSize.y = minimumHeight() * devicePixelRatioF();
            return true;
        }
        case WM_NCLBUTTONDBLCLK:
        {
            // Handle double-click on title bar to maximize/restore.
            // The system won't deliver mouseDoubleClickEvent for HTCAPTION areas.
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

    const int btnW = TitleBarButtonWidth;
    const int w = width();

    const QRect closeRect(w - btnW, 0, btnW, TitleBarHeight);
    if (closeRect.contains(pos))
    {
        return TitleBarButton::Close;
    }

    const QRect maximizeRect(w - btnW * 2, 0, btnW, TitleBarHeight);
    if (maximizeRect.contains(pos))
    {
        return TitleBarButton::Maximize;
    }

    const QRect minimizeRect(w - btnW * 3, 0, btnW, TitleBarHeight);
    if (minimizeRect.contains(pos))
    {
        return TitleBarButton::Minimize;
    }

    return TitleBarButton::None;
}

QRect AntWindow::titleBarRect() const
{
    return QRect(0, 0, width(), TitleBarHeight);
}

QRect AntWindow::minimizeButtonRect() const
{
    return QRect(width() - TitleBarButtonWidth * 3, 0, TitleBarButtonWidth, TitleBarHeight);
}

QRect AntWindow::maximizeButtonRect() const
{
    return QRect(width() - TitleBarButtonWidth * 2, 0, TitleBarButtonWidth, TitleBarHeight);
}

QRect AntWindow::closeButtonRect() const
{
    return QRect(width() - TitleBarButtonWidth, 0, TitleBarButtonWidth, TitleBarHeight);
}

void AntWindow::handleButtonClicked(TitleBarButton button)
{
    switch (button)
    {
    case TitleBarButton::Minimize:
        showMinimized();
        Q_EMIT minimizeRequested();
        break;
    case TitleBarButton::Maximize:
        if (m_windowMaximized)
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

void AntWindow::syncTheme()
{
    const auto& token = antTheme->tokens();
    QPalette pal = palette();
    pal.setColor(QPalette::Window, token.colorBgContainer);
    setPalette(pal);
}
