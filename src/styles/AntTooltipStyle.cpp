#include "AntTooltipStyle.h"

#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "widgets/AntTooltip.h"

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
    int paddingX = 12;
    int paddingY = 8;
    int radius = 6;
    int arrowSize = 8;
    int gap = 10;
    int maxWidth = 280;
};

QRect computeBubbleRect(const QRect& rect, const Metrics& m, Ant::TooltipPlacement placement, bool arrowVisible)
{
    if (!arrowVisible)
    {
        return rect;
    }
    if (isTopPlacement(placement))
    {
        return rect.adjusted(0, 0, 0, -m.arrowSize);
    }
    if (isBottomPlacement(placement))
    {
        return rect.adjusted(0, m.arrowSize, 0, 0);
    }
    if (placement == Ant::TooltipPlacement::Left)
    {
        return rect.adjusted(0, 0, -m.arrowSize, 0);
    }
    return rect.adjusted(m.arrowSize, 0, 0, 0);
}

QPolygonF computeArrowPolygon(const QRect& rect, const QRect& bubble, const Metrics& m,
                               Ant::TooltipPlacement placement, bool arrowVisible)
{
    if (!arrowVisible)
    {
        return {};
    }

    switch (placement)
    {
    case Ant::TooltipPlacement::Top:
        return QPolygonF({QPointF(rect.width() / 2.0 - m.arrowSize, bubble.bottom()),
                          QPointF(rect.width() / 2.0 + m.arrowSize, bubble.bottom()),
                          QPointF(rect.width() / 2.0, rect.bottom())});
    case Ant::TooltipPlacement::TopLeft:
        return QPolygonF({QPointF(bubble.left() + 18 - m.arrowSize, bubble.bottom()),
                          QPointF(bubble.left() + 18 + m.arrowSize, bubble.bottom()),
                          QPointF(bubble.left() + 18, rect.bottom())});
    case Ant::TooltipPlacement::TopRight:
        return QPolygonF({QPointF(bubble.right() - 18 - m.arrowSize, bubble.bottom()),
                          QPointF(bubble.right() - 18 + m.arrowSize, bubble.bottom()),
                          QPointF(bubble.right() - 18, rect.bottom())});
    case Ant::TooltipPlacement::Bottom:
        return QPolygonF({QPointF(rect.width() / 2.0 - m.arrowSize, bubble.top()),
                          QPointF(rect.width() / 2.0 + m.arrowSize, bubble.top()),
                          QPointF(rect.width() / 2.0, rect.top())});
    case Ant::TooltipPlacement::BottomLeft:
        return QPolygonF({QPointF(bubble.left() + 18 - m.arrowSize, bubble.top()),
                          QPointF(bubble.left() + 18 + m.arrowSize, bubble.top()),
                          QPointF(bubble.left() + 18, rect.top())});
    case Ant::TooltipPlacement::BottomRight:
        return QPolygonF({QPointF(bubble.right() - 18 - m.arrowSize, bubble.top()),
                          QPointF(bubble.right() - 18 + m.arrowSize, bubble.top()),
                          QPointF(bubble.right() - 18, rect.top())});
    case Ant::TooltipPlacement::Left:
        return QPolygonF({QPointF(bubble.right(), rect.height() / 2.0 - m.arrowSize),
                          QPointF(bubble.right(), rect.height() / 2.0 + m.arrowSize),
                          QPointF(rect.right(), rect.height() / 2.0)});
    case Ant::TooltipPlacement::Right:
    default:
        return QPolygonF({QPointF(bubble.left(), rect.height() / 2.0 - m.arrowSize),
                          QPointF(bubble.left(), rect.height() / 2.0 + m.arrowSize),
                          QPointF(rect.left(), rect.height() / 2.0)});
    }
}
} // namespace

AntTooltipStyle::AntTooltipStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntTooltip*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->updateGeometry();
                w->update();
            }
        }
    });
}

void AntTooltipStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntTooltip*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntTooltipStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntTooltip*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntTooltipStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntTooltip*>(widget))
    {
        drawTooltip(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntTooltipStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntTooltipStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* tooltip = qobject_cast<AntTooltip*>(watched);
    if (tooltip && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(tooltip);
        option.rect = tooltip->rect();
        QPainter painter(tooltip);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, tooltip);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntTooltipStyle::drawTooltip(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* tooltip = qobject_cast<const AntTooltip*>(widget);
    if (!tooltip || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const Metrics m;
    const Ant::TooltipPlacement placement = tooltip->placement();
    const bool arrowVisible = tooltip->arrowVisible();

    const QRect bubble = computeBubbleRect(option->rect, m, placement, arrowVisible);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Bubble color
    const QColor customColor = tooltip->color();
    const QColor bgColor = customColor.isValid()
        ? customColor
        : AntPalette::alpha(token.colorText, antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.92 : 0.88);
    const QColor txtColor = customColor.isValid()
        ? (customColor.lightnessF() > 0.6 ? QColor(Qt::black) : QColor(Qt::white))
        : QColor(Qt::white);

    painter->setPen(Qt::NoPen);
    painter->setBrush(bgColor);
    painter->drawRoundedRect(bubble, token.borderRadiusSM, token.borderRadiusSM);

    if (arrowVisible)
    {
        painter->drawPolygon(computeArrowPolygon(option->rect, bubble, m, placement, arrowVisible));
    }

    painter->setPen(txtColor);
    QFont textFont = painter->font();
    textFont.setPixelSize(token.fontSizeSM);
    painter->setFont(textFont);
    painter->drawText(bubble.adjusted(m.paddingX, m.paddingY, -m.paddingX, -m.paddingY),
                      Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap,
                      tooltip->title());

    painter->restore();
}
