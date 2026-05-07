#include "AntWindowStyle.h"

#include <QCursor>
#include <QEvent>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include <cmath>

#include "widgets/AntWindow.h"

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

    // ─── Draw window background ───
    painter->setPen(Qt::NoPen);
    painter->setBrush(token.colorBgContainer);
    painter->drawRect(option->rect);

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
    const QPoint cursorPos = window->mapFromGlobal(QCursor::pos());

    auto drawButtonBackground = [&](AntWindow::TitleBarButton button, bool destructive = false, bool active = false) {
        const QRect btnRect = window->titleBarButtonRect(button);
        if (btnRect.isNull())
        {
            return btnRect;
        }

        const bool hovered = btnRect.contains(cursorPos);
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
        const bool hovered = btnRect.contains(cursorPos);
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

    auto drawPinIcon = [&](const QRect& btnRect, const QColor& iconColor) {
        if (btnRect.isNull())
        {
            return;
        }

        const QPointF center = btnRect.center();
        painter->setPen(QPen(iconColor, 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        painter->drawLine(center + QPointF(-4.0, -5.5), center + QPointF(4.0, 2.5));
        painter->drawLine(center + QPointF(-5.5, -3.5), center + QPointF(-2.0, -7.0));
        painter->drawLine(center + QPointF(2.0, 0.5), center + QPointF(-4.5, 7.0));
        painter->drawPoint(center + QPointF(-5.5, 8.0));
    };

    auto drawThemeIcon = [&](const QRect& btnRect, const QColor& iconColor) {
        if (btnRect.isNull())
        {
            return;
        }

        const QPointF center = btnRect.center();
        if (antTheme->themeMode() == Ant::ThemeMode::Dark)
        {
            constexpr qreal kPi = 3.14159265358979323846;
            painter->setPen(QPen(iconColor, 1.3, Qt::SolidLine, Qt::RoundCap));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(center, 4.2, 4.2);
            for (int i = 0; i < 8; ++i)
            {
                const qreal angle = i * kPi / 4.0;
                const QPointF start(center.x() + std::cos(angle) * 7.0, center.y() + std::sin(angle) * 7.0);
                const QPointF end(center.x() + std::cos(angle) * 9.0, center.y() + std::sin(angle) * 9.0);
                painter->drawLine(start, end);
            }
        }
        else
        {
            QPainterPath moon;
            moon.addEllipse(QRectF(center.x() - 5.0, center.y() - 6.0, 11.0, 12.0));
            QPainterPath cutout;
            cutout.addEllipse(QRectF(center.x() - 1.0, center.y() - 7.0, 11.0, 12.0));
            painter->fillPath(moon.subtracted(cutout), iconColor);
        }
    };

    // Pin button
    {
        const bool active = window->isAlwaysOnTop();
        const QRect btnRect = drawButtonBackground(AntWindow::TitleBarButton::Pin, false, active);
        drawPinIcon(btnRect, iconColorFor(AntWindow::TitleBarButton::Pin, false, active));
    }

    // Theme toggle button
    {
        const QRect btnRect = drawButtonBackground(AntWindow::TitleBarButton::Theme);
        drawThemeIcon(btnRect, iconColorFor(AntWindow::TitleBarButton::Theme));
    }

    // Minimize button
    {
        const QRect btnRect = drawButtonBackground(AntWindow::TitleBarButton::Minimize);
        if (!btnRect.isNull())
        {
            // Draw minimize icon (horizontal line)
            painter->setPen(QPen(iconColorFor(AntWindow::TitleBarButton::Minimize), 1.4, Qt::SolidLine, Qt::RoundCap));
            const int cx = btnRect.center().x();
            const int cy = btnRect.center().y();
            painter->drawLine(QPointF(cx - 5, cy), QPointF(cx + 5, cy));
        }
    }

    // Maximize/Restore button
    {
        const QRect btnRect = drawButtonBackground(AntWindow::TitleBarButton::Maximize);

        // Draw maximize/restore icon
        if (!btnRect.isNull())
        {
            painter->setPen(QPen(iconColorFor(AntWindow::TitleBarButton::Maximize), 1.4, Qt::SolidLine, Qt::SquareCap));
            painter->setBrush(Qt::NoBrush);
            const int cx = btnRect.center().x();
            const int cy = btnRect.center().y();
            if (maximized)
            {
                // Restore icon: two overlapping rectangles
                const int rw = 8;
                const int rh = 7;
                painter->drawRect(QRectF(cx - 1, cy - rh + 1, rw, rh));
                painter->drawRect(QRectF(cx - rw + 1, cy, rw, rh));
            }
            else
            {
                // Maximize icon: single rectangle
                const int rw = 8;
                const int rh = 8;
                painter->drawRect(QRectF(cx - rw / 2.0, cy - rh / 2.0, rw, rh));
            }
        }
    }

    // Close button
    {
        const QRect btnRect = drawButtonBackground(AntWindow::TitleBarButton::Close, true);

        // Draw close icon (X)
        if (!btnRect.isNull())
        {
            painter->setPen(QPen(iconColorFor(AntWindow::TitleBarButton::Close, true), 1.4, Qt::SolidLine, Qt::RoundCap));
            const int cx = btnRect.center().x();
            const int cy = btnRect.center().y();
            painter->drawLine(QPointF(cx - 4.5, cy - 4.5), QPointF(cx + 4.5, cy + 4.5));
            painter->drawLine(QPointF(cx + 4.5, cy - 4.5), QPointF(cx - 4.5, cy + 4.5));
        }
    }

    // Draw an inner outline so frameless windows remain visible on light desktop backgrounds.
    if (!maximized)
    {
        painter->setPen(QPen(token.colorBorder, token.lineWidth));
        painter->setBrush(Qt::NoBrush);
        const qreal halfLine = token.lineWidth / 2.0;
        painter->drawRect(QRectF(option->rect).adjusted(halfLine, halfLine, -halfLine, -halfLine));
    }

    painter->restore();
}
