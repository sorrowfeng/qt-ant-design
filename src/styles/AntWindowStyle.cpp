#include "AntWindowStyle.h"

#include <QEvent>
#include <QFile>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QSvgRenderer>

#include "widgets/AntWindow.h"

namespace
{
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

bool drawAntdIcon(const QString& iconName, const QRectF& iconRect, const QColor& color, QPainter* painter)
{
    if (iconName.isEmpty() || iconRect.isEmpty() || !painter)
    {
        return false;
    }

    QFile file(QStringLiteral(":/qt-ant-design/icons/antd/%1.svg").arg(iconName));
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QString svg = QString::fromUtf8(file.readAll());
    svg.replace(QStringLiteral("__PRIMARY__"), color.name(QColor::HexRgb));
    svg.replace(QStringLiteral("__SECONDARY__"), color.name(QColor::HexRgb));

    QSvgRenderer renderer(svg.toUtf8());
    if (!renderer.isValid())
    {
        return false;
    }

    renderer.render(painter, iconRect);
    return true;
}
}

AntWindowStyle::AntWindowStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntWindow>();
}

void AntWindowStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
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
    QProxyStyle::unpolish(widget);
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

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

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
    const QRectF windowRect(option->rect);
    QPainterPath windowPath;
    if (cornerRadius > 0)
    {
        windowPath.addRoundedRect(windowRect, cornerRadius, cornerRadius);
        painter->setClipPath(windowPath);
    }
    else
    {
        windowPath.addRect(windowRect);
    }

    // ─── Draw window background ───
    painter->setPen(Qt::NoPen);
    painter->setBrush(token.colorBgContainer);
    painter->drawPath(windowPath);

    // ─── Draw title bar background ───
    const QRect titleBarRect(0, 0, w, titleBarH);
    painter->setPen(Qt::NoPen);
    painter->setBrush(token.colorBgContainer);
    painter->drawRect(titleBarRect);

    // ─── Draw title bar bottom border ───
    painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(QPointF(0, titleBarH - 0.5), QPointF(w, titleBarH - 0.5));

    // ─── Draw window icon ───
    const QIcon windowIcon = window->windowIcon();
    if (!windowIcon.isNull())
    {
        const int iconSize = 16;
        const int iconLeft = 12;
        const int iconTop = (titleBarH - iconSize) / 2;
        const QPixmap pixmap = windowIcon.pixmap(iconSize, iconSize);
        painter->drawPixmap(iconLeft, iconTop, pixmap);
    }

    // ─── Draw title text ───
    const QString titleText = window->windowTitle();
    if (!titleText.isEmpty())
    {
        QFont titleFont = window->font();
        titleFont.setPixelSize(token.fontSizeLG);
        titleFont.setWeight(QFont::DemiBold);
        painter->setFont(titleFont);
        painter->setPen(token.colorText);

        int controlsLeft = w;
        for (AntWindow::TitleBarButton button : {AntWindow::TitleBarButton::Pin,
                                                 AntWindow::TitleBarButton::Theme,
                                                 AntWindow::TitleBarButton::Minimize,
                                                 AntWindow::TitleBarButton::Maximize,
                                                 AntWindow::TitleBarButton::Close})
        {
            const QRect rect = window->titleBarButtonRect(button);
            if (!rect.isNull())
            {
                controlsLeft = qMin(controlsLeft, rect.left());
            }
        }

        const int iconOffset = windowIcon.isNull() ? 12 : 12 + 16 + 8;
        const int textMaxWidth = qMax(0, controlsLeft - iconOffset - 12);
        const QRect textRect(iconOffset, 0, textMaxWidth, titleBarH);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, titleText);
    }

    // ─── Draw title bar buttons ───
    const AntWindow::TitleBarButton hoveredButton = window->hoveredTitleBarButton();

    auto drawButtonBackground = [&](AntWindow::TitleBarButton button, bool destructive = false, bool active = false) {
        const QRect btnRect = window->titleBarButtonRect(button);
        if (btnRect.isNull())
        {
            return btnRect;
        }

        const bool hovered = hoveredButton == button;
        if (hovered || active)
        {
            painter->setPen(Qt::NoPen);
            if (destructive && hovered)
            {
                painter->setBrush(token.colorError);
            }
            else if (active)
            {
                painter->setBrush(token.colorPrimaryBg);
            }
            else
            {
                painter->setBrush(token.colorFillTertiary);
            }
            painter->drawRect(btnRect);
        }

        return btnRect;
    };

    auto iconColorFor = [&](AntWindow::TitleBarButton button, bool destructive = false, bool active = false) {
        const QRect btnRect = window->titleBarButtonRect(button);
        const bool hovered = hoveredButton == button;
        if (destructive && hovered)
        {
            return QColor(Qt::white);
        }
        if (active)
        {
            return token.colorPrimary;
        }
        return hovered ? token.colorTextSecondary : token.colorText;
    };

    // Pin button
    {
        const bool active = window->isAlwaysOnTop();
        const QRect btnRect = drawButtonBackground(AntWindow::TitleBarButton::Pin, false, active);
        const QString iconName = active ? QStringLiteral("PushpinFilled") : QStringLiteral("PushpinOutlined");
        drawAntdIcon(iconName, centeredIconRect(btnRect), iconColorFor(AntWindow::TitleBarButton::Pin, false, active), painter);
    }

    // Theme toggle button
    {
        const QRect btnRect = drawButtonBackground(AntWindow::TitleBarButton::Theme);
        const QString iconName = antTheme->themeMode() == Ant::ThemeMode::Dark
            ? QStringLiteral("SunOutlined")
            : QStringLiteral("MoonOutlined");
        drawAntdIcon(iconName, centeredIconRect(btnRect), iconColorFor(AntWindow::TitleBarButton::Theme), painter);
    }

    // Minimize button
    {
        const QRect btnRect = drawButtonBackground(AntWindow::TitleBarButton::Minimize);
        drawAntdIcon(QStringLiteral("MinusOutlined"),
                     centeredIconRect(btnRect),
                     iconColorFor(AntWindow::TitleBarButton::Minimize),
                     painter);
    }

    // Maximize/Restore button
    {
        const QRect btnRect = drawButtonBackground(AntWindow::TitleBarButton::Maximize);
        drawAntdIcon(maximized ? QStringLiteral("FullscreenExitOutlined") : QStringLiteral("FullscreenOutlined"),
                     centeredIconRect(btnRect),
                     iconColorFor(AntWindow::TitleBarButton::Maximize),
                     painter);
    }

    // Close button
    {
        const QRect btnRect = drawButtonBackground(AntWindow::TitleBarButton::Close, true);
        drawAntdIcon(QStringLiteral("CloseOutlined"),
                     centeredIconRect(btnRect),
                     iconColorFor(AntWindow::TitleBarButton::Close, true),
                     painter);
    }

    painter->restore();
}
