#include "AntStatusBarStyle.h"

#include <QApplication>
#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include "core/AntTheme.h"
#include "widgets/AntStatusBar.h"

AntStatusBarStyle::AntStatusBarStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntStatusBar*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->updateGeometry();
                w->update();
            }
        }
    });
}

void AntStatusBarStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntStatusBar*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntStatusBarStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntStatusBar*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntStatusBarStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntStatusBar*>(widget))
    {
        drawStatusBar(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntStatusBarStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntStatusBarStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* statusBar = qobject_cast<AntStatusBar*>(watched);
    if (statusBar && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(statusBar);
        option.rect = statusBar->rect();
        QPainter painter(statusBar);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, statusBar);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

namespace
{
constexpr int kSizeGripWidth = 20;
constexpr int kSizeGripDotRadius = 1;
constexpr int kSizeGripDotSpacing = 4;
constexpr int kSizeGripMargin = 4;
} // namespace

void AntStatusBarStyle::drawStatusBar(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* statusBar = qobject_cast<const AntStatusBar*>(widget);
    if (!statusBar || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const int w = option->rect.width();
    const int h = option->rect.height();

    // Background
    painter->fillRect(option->rect, token.colorBgContainer);

    // Top border
    painter->setPen(QPen(token.colorBorderSecondary, 1.0));
    painter->drawLine(0, 0, w, 0);

    // Font for items
    QFont itemFont = painter->font();
    itemFont.setPixelSize(token.fontSizeSM);
    painter->setFont(itemFont);
    QFontMetrics fm(itemFont);

    const int iconSize = 16;
    const int dividerHeight = h * 2 / 3;
    const int dividerY = (h - dividerHeight) / 2;
    const int permCount = statusBar->permanentItemCount();
    const int regCount = statusBar->itemCount();
    const int rightX = statusBar->hasSizeGrip() ? w - kSizeGripWidth : w;
    const int dividerWidth = token.lineWidth + token.paddingXXS * 2;

    // --- Draw permanent items (right-aligned, iterating right to left) ---
    int rx = rightX - token.paddingXS;
    for (int i = permCount - 1; i >= 0; --i)
    {
        const AntStatusBarItem item = statusBar->permanentItemAt(i);

        // Draw divider to the right of this item (between items, not at edge)
        if (i < permCount - 1)
        {
            const int dx = rx - dividerWidth / 2;
            painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
            painter->drawLine(dx, dividerY, dx, dividerY + dividerHeight);
            rx -= dividerWidth;
        }

        // Compute item width
        int textWidth = fm.horizontalAdvance(item.text);
        if (!item.icon.isEmpty())
        {
            textWidth += iconSize + token.paddingXXS;
        }
        const int itemWidth = textWidth + token.paddingXS * 2;
        rx -= itemWidth;

        const QRect itemRect(rx, 0, itemWidth, h);

        // Hover background
        if (i == statusBar->hoveredPermanentIndex())
        {
            painter->fillRect(itemRect, token.colorFillTertiary);
        }

        // Draw item content
        painter->setPen(token.colorTextSecondary);
        int tx = itemRect.left() + token.paddingXS;
        if (!item.icon.isEmpty())
        {
            painter->drawText(QRect(tx, itemRect.top(), iconSize, itemRect.height()),
                              Qt::AlignCenter, item.icon.left(2));
            tx += iconSize + token.paddingXXS;
        }
        painter->drawText(QRect(tx, itemRect.top(), itemRect.right() - tx, itemRect.height()),
                          Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
                          item.text);
    }

    // --- Draw regular items (left-aligned) ---
    int x = token.paddingXS;
    for (int i = 0; i < regCount; ++i)
    {
        const AntStatusBarItem item = statusBar->itemAt(i);

        // Compute item width
        int textWidth = fm.horizontalAdvance(item.text);
        if (!item.icon.isEmpty())
        {
            textWidth += iconSize + token.paddingXXS;
        }
        const int itemWidth = textWidth + token.paddingXS * 2;
        const QRect itemRect(x, 0, itemWidth, h);

        // Hover background
        if (i == statusBar->hoveredRegularIndex())
        {
            painter->fillRect(itemRect, token.colorFillTertiary);
        }

        // Draw item content
        painter->setPen(token.colorTextSecondary);
        int tx = itemRect.left() + token.paddingXS;
        if (!item.icon.isEmpty())
        {
            painter->drawText(QRect(tx, itemRect.top(), iconSize, itemRect.height()),
                              Qt::AlignCenter, item.icon.left(2));
            tx += iconSize + token.paddingXXS;
        }
        painter->drawText(QRect(tx, itemRect.top(), itemRect.right() - tx, itemRect.height()),
                          Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
                          item.text);

        x += itemWidth;

        // Divider between regular items
        if (i < regCount - 1)
        {
            const int dx = x + token.paddingXXS;
            painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
            painter->drawLine(dx, dividerY, dx, dividerY + dividerHeight);
            x += dividerWidth;
        }
    }

    // --- Draw message in stretch area ---
    if (!statusBar->message().isEmpty())
    {
        const int msgLeft = x + token.paddingXS;
        const int msgRight = (permCount > 0) ? rx : rightX;
        if (msgRight > msgLeft)
        {
            painter->setPen(token.colorTextSecondary);
            const QRect msgRect(msgLeft, 0, msgRight - msgLeft, h);
            const QString elidedMsg = fm.elidedText(statusBar->message(), Qt::ElideRight, msgRect.width());
            painter->drawText(msgRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, elidedMsg);
        }
    }

    // --- Draw size grip ---
    if (statusBar->hasSizeGrip())
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorTextTertiary);

        const int gripX = w - kSizeGripMargin;
        const int gripY = h - kSizeGripMargin;

        // 3 diagonal dots: bottom-right, center, top-left
        for (int i = 0; i < 3; ++i)
        {
            const int dotX = gripX - i * kSizeGripDotSpacing;
            const int dotY = gripY - i * kSizeGripDotSpacing;
            painter->drawEllipse(dotX, dotY, kSizeGripDotRadius, kSizeGripDotRadius);
        }
    }

    painter->restore();
}
