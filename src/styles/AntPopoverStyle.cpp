#include "AntPopoverStyle.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntIcon.h"
#include "widgets/AntPopover.h"

namespace
{
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

QPainterPath popoverSurfacePath(const QRect& bubble, const QPolygonF& arrow, int radius)
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(QRectF(bubble), radius, radius);
    if (!arrow.isEmpty())
    {
        QPainterPath arrowPath;
        arrowPath.addPolygon(arrow);
        arrowPath.closeSubpath();
        path = path.united(arrowPath);
    }
    return path;
}

void drawPopoverShadow(QPainter* painter, const QPainterPath& surface)
{
    if (!painter)
    {
        return;
    }

    const QColor shadowBase = antTheme->tokens().colorShadow;
    painter->setPen(Qt::NoPen);
    for (int i = 12; i >= 1; --i)
    {
        const qreal progress = static_cast<qreal>(i) / 12.0;
        QColor shadow = shadowBase;
        shadow.setAlphaF(0.018 * (1.0 - progress) + 0.004);
        painter->setBrush(shadow);
        painter->drawPath(surface.translated(0, i * 0.55));
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
} // namespace

AntPopoverStyle::AntPopoverStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntPopoverStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
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
    AntStyleBase::unpolish(widget);
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
    const auto& layout = popover->popoverLayout();
    const QRect bubble = layout.bubbleRect;
    const QPolygonF& arrow = layout.arrowPolygon;
    const QPainterPath surface = popoverSurfacePath(bubble, arrow, token.borderRadiusLG);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Shadow and bubble
    drawPopoverShadow(painter, surface);
    painter->setPen(Qt::NoPen);
    painter->setBrush(token.colorBgElevated);
    painter->drawPath(surface);

    QColor edge = token.colorShadow;
    edge.setAlphaF(0.08);
    painter->setPen(QPen(edge, token.lineWidth));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(surface);

    // Header
    const bool hasTitleIcon = !popover->title().isEmpty() && popover->titleIconType() != Ant::IconType::None;
    const QRect header = layout.headerRect;
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
    bodyFont.setPixelSize(token.fontSize);
    bodyFont.setWeight(QFont::Normal);
    painter->setFont(bodyFont);
    painter->setPen(token.colorText);
    painter->drawText(layout.bodyRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, popover->content());

    painter->restore();
}
