#include "AntWindowStyle.h"

#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QIcon>
#include <QPainter>
#include <QStyleOption>

#include "core/AntTheme.h"
#include "widgets/AntWindow.h"

AntWindowStyle::AntWindowStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntWindow*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->updateGeometry();
                w->update();
            }
        }
    });
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
    const int btnW = AntWindow::TitleBarButtonWidth;
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

        const int iconOffset = windowIcon.isNull() ? 12 : 12 + 16 + 8;
        const int textMaxWidth = w - iconOffset - btnW * 3 - 12;
        const QRect textRect(iconOffset, 0, textMaxWidth, titleBarH);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, titleText);
    }

    // ─── Draw title bar buttons ───
    const QPoint cursorPos = window->mapFromGlobal(QCursor::pos());

    // Determine hovered button
    auto hoveredButton = [w, titleBarH, btnW, &cursorPos]() {
        const QRect closeRect(w - btnW, 0, btnW, titleBarH);
        if (closeRect.contains(cursorPos))
            return 3;
        const QRect maxRect(w - btnW * 2, 0, btnW, titleBarH);
        if (maxRect.contains(cursorPos))
            return 2;
        const QRect minRect(w - btnW * 3, 0, btnW, titleBarH);
        if (minRect.contains(cursorPos))
            return 1;
        return 0;
    }();

    // Minimize button
    {
        const QRect btnRect(w - btnW * 3, 0, btnW, titleBarH);
        const bool isBtnHovered = hoveredButton == 1;

        if (isBtnHovered)
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(token.colorFillTertiary);
            painter->drawRect(btnRect);
        }

        // Draw minimize icon (horizontal line)
        painter->setPen(QPen(token.colorText, 1.4, Qt::SolidLine, Qt::RoundCap));
        const int cx = btnRect.center().x();
        const int cy = btnRect.center().y();
        painter->drawLine(QPointF(cx - 5, cy), QPointF(cx + 5, cy));
    }

    // Maximize/Restore button
    {
        const QRect btnRect(w - btnW * 2, 0, btnW, titleBarH);
        const bool isBtnHovered = hoveredButton == 2;

        if (isBtnHovered)
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(token.colorFillTertiary);
            painter->drawRect(btnRect);
        }

        // Draw maximize/restore icon
        const QColor iconColor = isBtnHovered ? token.colorTextSecondary : token.colorText;
        painter->setPen(QPen(iconColor, 1.4, Qt::SolidLine, Qt::SquareCap));
        painter->setBrush(Qt::NoBrush);

        if (maximized)
        {
            // Restore icon: two overlapping rectangles
            const int cx = btnRect.center().x();
            const int cy = btnRect.center().y();
            const int rw = 8;
            const int rh = 7;

            // Back rectangle (offset top-right)
            painter->drawRect(QRectF(cx - 1, cy - rh + 1, rw, rh));
            // Front rectangle (offset bottom-left)
            painter->drawRect(QRectF(cx - rw + 1, cy, rw, rh));
        }
        else
        {
            // Maximize icon: single rectangle
            const int cx = btnRect.center().x();
            const int cy = btnRect.center().y();
            const int rw = 8;
            const int rh = 8;
            painter->drawRect(QRectF(cx - rw / 2.0, cy - rh / 2.0, rw, rh));
        }
    }

    // Close button
    {
        const QRect btnRect(w - btnW, 0, btnW, titleBarH);
        const bool isBtnHovered = hoveredButton == 3;

        if (isBtnHovered)
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(token.colorError);
            painter->drawRect(btnRect);
        }

        // Draw close icon (X)
        const QColor closeColor = isBtnHovered ? QColor(Qt::white) : token.colorText;
        painter->setPen(QPen(closeColor, 1.4, Qt::SolidLine, Qt::RoundCap));
        const int cx = btnRect.center().x();
        const int cy = btnRect.center().y();
        painter->drawLine(QPointF(cx - 4.5, cy - 4.5), QPointF(cx + 4.5, cy + 4.5));
        painter->drawLine(QPointF(cx + 4.5, cy - 4.5), QPointF(cx - 4.5, cy + 4.5));
    }

    painter->restore();
}
