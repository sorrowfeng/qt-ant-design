#include "AntPopoverStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntIcon.h"
#include "widgets/AntPopover.h"

namespace
{
bool isTopPlacement(Ant::TooltipPlacement placement)
{
    return placement == Ant::TooltipPlacement::Top
        || placement == Ant::TooltipPlacement::TopLeft
        || placement == Ant::TooltipPlacement::TopRight;
}

bool isBottomPlacement(Ant::TooltipPlacement placement)
{
    return placement == Ant::TooltipPlacement::Bottom
        || placement == Ant::TooltipPlacement::BottomLeft
        || placement == Ant::TooltipPlacement::BottomRight;
}

struct Metrics
{
    int paddingX = 16;
    int paddingY = 12;
    int radius = 8;
    int arrowSize = 8;
    int gap = 10;
    int maxWidth = 320;
};

QRect computeBubbleRect(const QRect& rect, const Metrics& m, Ant::TooltipPlacement placement, bool arrowVisible)
{
    if (!arrowVisible)
    {
        return rect.adjusted(8, 8, -8, -8);
    }
    if (isTopPlacement(placement))
    {
        return rect.adjusted(8, 8, -8, -(8 + m.arrowSize));
    }
    if (isBottomPlacement(placement))
    {
        return rect.adjusted(8, 8 + m.arrowSize, -8, -8);
    }
    if (placement == Ant::TooltipPlacement::Left)
    {
        return rect.adjusted(8, 8, -(8 + m.arrowSize), -8);
    }
    return rect.adjusted(8 + m.arrowSize, 8, -8, -8);
}

QPolygonF computeArrowPolygon(const QRect& bubble, const Metrics& m, Ant::TooltipPlacement placement, bool arrowVisible)
{
    if (!arrowVisible)
    {
        return {};
    }
    switch (placement)
    {
    case Ant::TooltipPlacement::Top:
        return QPolygonF()
            << QPointF(bubble.center().x() - m.arrowSize, bubble.bottom())
            << QPointF(bubble.center().x() + m.arrowSize, bubble.bottom())
            << QPointF(bubble.center().x(), bubble.bottom() + m.arrowSize);
    case Ant::TooltipPlacement::TopLeft:
        return QPolygonF()
            << QPointF(bubble.left() + 24 - m.arrowSize, bubble.bottom())
            << QPointF(bubble.left() + 24 + m.arrowSize, bubble.bottom())
            << QPointF(bubble.left() + 24, bubble.bottom() + m.arrowSize);
    case Ant::TooltipPlacement::TopRight:
        return QPolygonF()
            << QPointF(bubble.right() - 24 - m.arrowSize, bubble.bottom())
            << QPointF(bubble.right() - 24 + m.arrowSize, bubble.bottom())
            << QPointF(bubble.right() - 24, bubble.bottom() + m.arrowSize);
    case Ant::TooltipPlacement::Bottom:
        return QPolygonF()
            << QPointF(bubble.center().x() - m.arrowSize, bubble.top())
            << QPointF(bubble.center().x() + m.arrowSize, bubble.top())
            << QPointF(bubble.center().x(), bubble.top() - m.arrowSize);
    case Ant::TooltipPlacement::BottomLeft:
        return QPolygonF()
            << QPointF(bubble.left() + 24 - m.arrowSize, bubble.top())
            << QPointF(bubble.left() + 24 + m.arrowSize, bubble.top())
            << QPointF(bubble.left() + 24, bubble.top() - m.arrowSize);
    case Ant::TooltipPlacement::BottomRight:
        return QPolygonF()
            << QPointF(bubble.right() - 24 - m.arrowSize, bubble.top())
            << QPointF(bubble.right() - 24 + m.arrowSize, bubble.top())
            << QPointF(bubble.right() - 24, bubble.top() - m.arrowSize);
    case Ant::TooltipPlacement::Left:
        return QPolygonF()
            << QPointF(bubble.right(), bubble.center().y() - m.arrowSize)
            << QPointF(bubble.right(), bubble.center().y() + m.arrowSize)
            << QPointF(bubble.right() + m.arrowSize, bubble.center().y());
    case Ant::TooltipPlacement::Right:
    default:
        return QPolygonF()
            << QPointF(bubble.left(), bubble.center().y() - m.arrowSize)
            << QPointF(bubble.left(), bubble.center().y() + m.arrowSize)
            << QPointF(bubble.left() - m.arrowSize, bubble.center().y());
    }
}

QRect computeHeaderRect(const QRect& bubble, const Metrics& m, const QString& title, Ant::IconType iconType, const QFont& baseFont)
{
    QRect inner = bubble.adjusted(m.paddingX, m.paddingY, -m.paddingX, -m.paddingY);
    int headerHeight = 0;
    if (!title.isEmpty())
    {
        QFont titleFont = baseFont;
        titleFont.setPixelSize(antTheme->tokens().fontSize);
        titleFont.setWeight(QFont::DemiBold);
        const int iconExtra = iconType != Ant::IconType::None ? 26 : 0;
        headerHeight = QFontMetrics(titleFont).boundingRect(QRect(0, 0, inner.width() - iconExtra, 120), Qt::TextWordWrap, title).height();
        headerHeight = qMax(headerHeight, iconExtra > 0 ? 18 : 0);
    }
    return QRect(inner.left(), inner.top(), inner.width(), headerHeight);
}

QColor titleIconColor(Ant::IconType iconType)
{
    const auto& token = antTheme->tokens();
    switch (iconType)
    {
    case Ant::IconType::CheckCircle:
        return token.colorSuccess;
    case Ant::IconType::ExclamationCircle:
        return token.colorWarning;
    case Ant::IconType::CloseCircle:
        return token.colorError;
    case Ant::IconType::InfoCircle:
        return token.colorPrimary;
    default:
        return token.colorTextTertiary;
    }
}

void drawTitleIcon(QPainter& painter, const QRectF& rect, Ant::IconType iconType)
{
    if (iconType == Ant::IconType::None)
    {
        return;
    }

    const AntIcon::IconPaths paths = AntIcon::builtinPaths(iconType, Ant::IconTheme::Filled);
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(titleIconColor(iconType));
    painter.drawPath(AntIcon::transformPath(paths.primary, rect));
    if (!paths.secondary.isEmpty())
    {
        painter.setBrush(antTheme->tokens().colorTextLightSolid);
        painter.drawPath(AntIcon::transformPath(paths.secondary, rect));
    }
    painter.restore();
}

QRect computeBodyRect(const QRect& bubble, const Metrics& m, const QRect& header, bool hasActionWidget)
{
    QRect inner = bubble.adjusted(m.paddingX, m.paddingY, -m.paddingX, -m.paddingY);
    int top = header.isNull() ? inner.top() : header.bottom() + 8;
    int bottom = hasActionWidget ? inner.bottom() - 38 : inner.bottom();
    return QRect(inner.left(), top, inner.width(), qMax(24, bottom - top));
}
} // namespace

AntPopoverStyle::AntPopoverStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntPopover>();
}

void AntPopoverStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntPopover*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntPopoverStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntPopover*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntPopoverStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntPopover*>(widget))
    {
        drawPopover(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntPopoverStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntPopoverStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* popover = qobject_cast<AntPopover*>(watched);
    if (popover && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(popover);
        option.rect = popover->rect();
        QPainter painter(popover);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, popover);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntPopoverStyle::drawPopover(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* popover = qobject_cast<const AntPopover*>(widget);
    if (!popover || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const Metrics m;
    const Ant::TooltipPlacement placement = popover->renderPlacement();
    const bool arrowVisible = popover->arrowVisible();
    const bool hasActionWidget = popover->actionWidget() != nullptr;

    const QRect bubble = computeBubbleRect(option->rect, m, placement, arrowVisible);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Shadow and bubble
    antTheme->drawEffectShadow(painter, bubble, 12, token.borderRadiusLG, 0.55);
    AntStyleBase::drawCrispRoundedRect(painter, bubble,
        QPen(token.colorBorderSecondary, token.lineWidth),
        token.colorBgElevated, token.borderRadiusLG, token.borderRadiusLG);

    // Arrow
    if (arrowVisible)
    {
        painter->drawPolygon(computeArrowPolygon(bubble, m, placement, arrowVisible));
    }

    // Header
    const bool hasTitleIcon = !popover->title().isEmpty() && popover->titleIconType() != Ant::IconType::None;
    const QRect header = computeHeaderRect(bubble, m, popover->title(), popover->titleIconType(), widget->font());
    if (!popover->title().isEmpty())
    {
        QRect titleRect = header;
        if (hasTitleIcon)
        {
            const QRectF iconRect(header.left(), header.top() + qMax(0, (header.height() - 18) / 2), 18, 18);
            drawTitleIcon(*painter, iconRect, popover->titleIconType());
            titleRect.adjust(26, 0, 0, 0);
        }
        QFont titleFont = painter->font();
        titleFont.setPixelSize(token.fontSize);
        titleFont.setWeight(QFont::DemiBold);
        painter->setFont(titleFont);
        painter->setPen(token.colorText);
        painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, popover->title());
    }

    // Body
    QFont bodyFont = painter->font();
    bodyFont.setPixelSize(token.fontSizeSM);
    bodyFont.setWeight(QFont::Normal);
    painter->setFont(bodyFont);
    painter->setPen(token.colorTextSecondary);
    const QRect body = computeBodyRect(bubble, m, header, hasActionWidget);
    painter->drawText(body, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, popover->content());

    painter->restore();
}
