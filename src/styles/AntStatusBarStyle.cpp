#include "AntStatusBarStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include "AntPalette.h"
#include "widgets/AntStatusBar.h"

AntStatusBarStyle::AntStatusBarStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntStatusBar>();
}

void AntStatusBarStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    installPaintFilter<AntStatusBar>(widget);
}

void AntStatusBarStyle::unpolish(QWidget* widget)
{
    removePaintFilter<AntStatusBar>(widget);
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

bool AntStatusBarStyle::drawWidget(QWidget* widget, QPaintEvent* event)
{
    auto* statusBar = qobject_cast<AntStatusBar*>(widget);
    if (!statusBar)
    {
        return false;
    }

    QStyleOption option;
    option.initFrom(statusBar);
    option.rect = statusBar->rect();
    QPainter painter(statusBar);
    drawPrimitive(QStyle::PE_Widget, &option, &painter, statusBar);

    return true;
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

struct StatusBarColors
{
    QColor background;
    QColor text;
    QColor icon;
    QColor hover;
};

Ant::StatusBarStatus effectiveStatus(Ant::StatusBarStatus localStatus,
                                     Ant::StatusBarStatus barStatus)
{
    if (localStatus != Ant::StatusBarStatus::Inherit)
    {
        return localStatus;
    }
    return barStatus == Ant::StatusBarStatus::Inherit
               ? Ant::StatusBarStatus::Default
               : barStatus;
}

StatusBarColors statusColors(Ant::StatusBarStatus status,
                             const AntThemeTokens& token)
{
    QColor background = token.colorBgContainer;
    QColor accent = token.colorTextSecondary;
    bool semantic = true;

    switch (status)
    {
    case Ant::StatusBarStatus::Info:
        background = token.colorPrimaryBg;
        accent = token.colorPrimary;
        break;
    case Ant::StatusBarStatus::Success:
        background = token.colorSuccessBg;
        accent = token.colorSuccess;
        break;
    case Ant::StatusBarStatus::Warning:
        background = token.colorWarningBg;
        accent = token.colorWarning;
        break;
    case Ant::StatusBarStatus::Error:
        background = token.colorErrorBg;
        accent = token.colorError;
        break;
    case Ant::StatusBarStatus::Default:
    case Ant::StatusBarStatus::Inherit:
        semantic = false;
        break;
    }

    if (!semantic)
    {
        return {background,
                token.colorTextSecondary,
                token.colorTextSecondary,
                token.colorFillTertiary};
    }

    const qreal hoverMix = antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.18 : 0.10;
    return {background,
            token.colorText,
            accent,
            AntPalette::mix(background, accent, hoverMix)};
}

QRect segmentFillRect(const QRect& rect)
{
    return rect.adjusted(0, 1, 0, 0);
}
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
    const bool enabled = statusBar->isEnabled();
    const Ant::StatusBarStatus barStatus = effectiveStatus(
        statusBar->status(), Ant::StatusBarStatus::Default);
    const StatusBarColors barColors = statusColors(barStatus, token);

    // Background
    painter->fillRect(option->rect,
                      enabled ? barColors.background : token.colorBgContainerDisabled);

    // Font for items
    const QFont itemFont = AntStyleBase::withPixelSize(painter->font(), token.fontSizeSM);
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
    for (int i = 0; i < permCount; ++i)
    {
        const AntStatusBarItem item = statusBar->permanentItemAt(i);
        const QRect itemRect = permanentRects.value(i);
        if (!itemRect.isValid())
        {
            continue;
        }

        const Ant::StatusBarStatus localStatus = statusBar->permanentItemStatus(i);
        const StatusBarColors colors = statusColors(
            effectiveStatus(localStatus, barStatus), token);
        if (enabled && localStatus != Ant::StatusBarStatus::Inherit)
        {
            painter->fillRect(segmentFillRect(itemRect), colors.background);
        }

        // Hover background
        if (enabled && i == statusBar->hoveredPermanentIndex())
        {
            painter->fillRect(segmentFillRect(itemRect), colors.hover);
        }

        // Draw item content
        int tx = itemRect.left() + token.paddingXS;
        if (!item.icon.isEmpty())
        {
            painter->setPen(enabled ? colors.icon : token.colorTextDisabled);
            painter->drawText(QRect(tx, itemRect.top(), iconSize, itemRect.height()),
                              Qt::AlignCenter, item.icon.left(2));
            tx += iconSize + token.paddingXXS;
        }
        painter->setPen(enabled ? colors.text : token.colorTextDisabled);
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

        const Ant::StatusBarStatus localStatus = statusBar->itemStatus(i);
        const StatusBarColors colors = statusColors(
            effectiveStatus(localStatus, barStatus), token);
        if (enabled && localStatus != Ant::StatusBarStatus::Inherit)
        {
            painter->fillRect(segmentFillRect(itemRect), colors.background);
        }

        // Hover background
        if (enabled && i == statusBar->hoveredRegularIndex())
        {
            painter->fillRect(segmentFillRect(itemRect), colors.hover);
        }

        // Draw item content
        int tx = itemRect.left() + token.paddingXS;
        if (!item.icon.isEmpty())
        {
            painter->setPen(enabled ? colors.icon : token.colorTextDisabled);
            painter->drawText(QRect(tx, itemRect.top(), iconSize, itemRect.height()),
                              Qt::AlignCenter, item.icon.left(2));
            tx += iconSize + token.paddingXXS;
        }
        painter->setPen(enabled ? colors.text : token.colorTextDisabled);
        painter->drawText(QRect(tx, itemRect.top(), itemRect.right() - tx, itemRect.height()),
                          Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
                          item.text);
    }

    // --- Draw message in stretch area ---
    if (!statusBar->message().isEmpty())
    {
        const QRect msgRect = statusBar->messageAreaRect();
        if (msgRect.isValid())
        {
            const Ant::StatusBarStatus localStatus = statusBar->messageStatus();
            const StatusBarColors colors = statusColors(
                effectiveStatus(localStatus, barStatus), token);
            if (enabled && localStatus != Ant::StatusBarStatus::Inherit)
            {
                painter->fillRect(segmentFillRect(msgRect), colors.background);
            }
            painter->setPen(enabled ? colors.text : token.colorTextDisabled);
            const QString elidedMsg = fm.elidedText(statusBar->message(), Qt::ElideRight, msgRect.width());
            painter->drawText(msgRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, elidedMsg);
        }
    }

    // Dividers and top border stay neutral and are drawn after status fills.
    const QColor borderColor = enabled ? token.colorBorderSecondary : token.colorBorderDisabled;
    painter->setPen(QPen(borderColor, token.lineWidth));
    for (int dx : statusBar->permanentDividerXs())
    {
        painter->drawLine(dx, dividerY, dx, dividerY + dividerHeight);
    }
    for (int dx : statusBar->regularDividerXs())
    {
        painter->drawLine(dx, dividerY, dx, dividerY + dividerHeight);
    }
    painter->setPen(QPen(borderColor, 1.0));
    painter->drawLine(0, 0, w, 0);

    // --- Draw size grip ---
    if (statusBar->hasSizeGrip())
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(enabled ? token.colorTextTertiary : token.colorTextDisabled);

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
