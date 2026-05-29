#include "AntStatusBarStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntStatusBar.h"

AntStatusBarStyle::AntStatusBarStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntStatusBar>();
}

void AntStatusBarStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
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
    AntStyleBase::unpolish(widget);
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

void AntStatusBarStyle::onThemeUpdate(QWidget* widget)
{
    auto* statusBar = qobject_cast<AntStatusBar*>(widget);
    if (!statusBar)
    {
        AntStyleBase::onThemeUpdate(widget);
        return;
    }

    statusBar->invalidateLayoutCache();
    AntStyleBase::onThemeUpdate(widget);
}

namespace
{
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
    const QVector<QRect>& permanentRects = statusBar->permanentItemRects();
    const QVector<QRect>& regularRects = statusBar->regularItemRects();

    // --- Draw permanent items (right-aligned) ---
    const QVector<int>& permanentDividers = statusBar->permanentDividerXs();
    painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
    for (int dx : permanentDividers)
    {
        painter->drawLine(dx, dividerY, dx, dividerY + dividerHeight);
    }

    for (int i = 0; i < permCount; ++i)
    {
        const AntStatusBarItem item = statusBar->permanentItemAt(i);
        const QRect itemRect = permanentRects.value(i);
        if (!itemRect.isValid())
        {
            continue;
        }

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
    for (int i = 0; i < regCount; ++i)
    {
        const AntStatusBarItem item = statusBar->itemAt(i);
        const QRect itemRect = regularRects.value(i);
        if (!itemRect.isValid())
        {
            continue;
        }

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
    }

    const QVector<int>& regularDividers = statusBar->regularDividerXs();
    painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
    for (int dx : regularDividers)
    {
        painter->drawLine(dx, dividerY, dx, dividerY + dividerHeight);
    }

    // --- Draw message in stretch area ---
    if (!statusBar->message().isEmpty())
    {
        const QRect msgRect = statusBar->messageAreaRect();
        if (msgRect.isValid())
        {
            painter->setPen(token.colorTextSecondary);
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
