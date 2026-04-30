#include "AntSegmentedStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include <algorithm>
#include <cmath>

#include "widgets/AntSegmented.h"

namespace
{

int segmentedHeight(Ant::Size size)
{
    const auto& token = antTheme->tokens();
    switch (size)
    {
    case Ant::Size::Small:  return token.controlHeightSM;
    case Ant::Size::Large:  return token.controlHeightLG;
    default:                         return token.controlHeight;
    }
}

int segmentedFontSize(Ant::Size size)
{
    const auto& token = antTheme->tokens();
    switch (size)
    {
    case Ant::Size::Small:  return token.fontSizeSM;
    case Ant::Size::Large:  return token.fontSizeLG;
    default:                         return token.fontSize;
    }
}

int segmentedRadius(const AntSegmented* seg)
{
    const auto& token = antTheme->tokens();
    const int h = segmentedHeight(seg->segmentedSize());
    if (seg->shape() == Ant::SegmentedShape::Round)
    {
        return h / 2;
    }
    switch (seg->segmentedSize())
    {
    case Ant::Size::Small:  return token.borderRadiusSM;
    case Ant::Size::Large:  return token.borderRadiusLG;
    default:                         return token.borderRadius;
    }
}

int segmentedItemRadius(const AntSegmented* seg)
{
    const auto& token = antTheme->tokens();
    const int h = segmentedHeight(seg->segmentedSize());
    if (seg->shape() == Ant::SegmentedShape::Round)
    {
        return h / 2;
    }
    switch (seg->segmentedSize())
    {
    case Ant::Size::Small: return token.borderRadiusXS;
    case Ant::Size::Large: return token.borderRadius;
    default: return token.borderRadiusSM;
    }
}

QRectF segmentedThumbRect(const AntSegmented* seg)
{
    const auto rects = seg->segmentRects();
    if (rects.isEmpty()) return QRectF();

    const qreal pos = std::clamp(seg->thumbPosition(), 0.0, static_cast<qreal>(rects.size() - 1));
    const int leftIdx = static_cast<int>(std::floor(pos));
    const int rightIdx = std::min(leftIdx + 1, static_cast<int>(rects.size()) - 1);
    const qreal t = pos - leftIdx;

    const QRectF a = rects[leftIdx];
    const QRectF b = rects[rightIdx];
    return QRectF(
        a.left() + (b.left() - a.left()) * t,
        a.top() + (b.top() - a.top()) * t,
        a.width() + (b.width() - a.width()) * t,
        a.height() + (b.height() - a.height()) * t);
}

void drawSegmentedThumbShadow(QPainter* painter, const QRectF& rect, qreal radius)
{
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 8));
    painter->drawRoundedRect(rect.translated(0, 1), radius, radius);
    painter->setBrush(QColor(0, 0, 0, 5));
    painter->drawRoundedRect(rect.adjusted(-1, 0, 1, 2), radius, radius);
    painter->setBrush(QColor(0, 0, 0, 5));
    painter->drawRoundedRect(rect.translated(0, 2), radius, radius);
    painter->restore();
}

QColor itemOverlayColor(const AntThemeTokens& token, bool pressed)
{
    const bool dark = token.colorBgBase.lightness() < 32;
    QColor color = dark ? QColor(255, 255, 255) : QColor(0, 0, 0);
    color.setAlphaF(pressed ? (dark ? 0.18 : 0.15) : (dark ? 0.12 : 0.06));
    return color;
}

void drawSegmentedIcon(QPainter* painter, const QString& icon, const QRectF& rect, const QColor& color)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(color, 1.3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);

    if (icon == QStringLiteral("appstore"))
    {
        const qreal size = 4.0;
        const qreal gap = 2.0;
        const QPointF start(rect.left() + 1.0, rect.top() + 1.0);
        for (int row = 0; row < 2; ++row)
        {
            for (int col = 0; col < 2; ++col)
            {
                const QRectF square(start.x() + col * (size + gap), start.y() + row * (size + gap), size, size);
                painter->drawRoundedRect(square, 0.8, 0.8);
            }
        }
    }
    else if (icon == QStringLiteral("setting"))
    {
        const QPointF c = rect.center();
        const qreal inner = 2.4;
        const qreal outer = 5.4;
        painter->drawEllipse(c, inner, inner);
        for (int i = 0; i < 8; ++i)
        {
            const qreal angle = (45.0 * i) * 3.14159265358979323846 / 180.0;
            const QPointF a(c.x() + std::cos(angle) * (outer - 1.4), c.y() + std::sin(angle) * (outer - 1.4));
            const QPointF b(c.x() + std::cos(angle) * outer, c.y() + std::sin(angle) * outer);
            painter->drawLine(a, b);
        }
    }

    painter->restore();
}

} // namespace

AntSegmentedStyle::AntSegmentedStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntSegmented>();
}

void AntSegmentedStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntSegmented*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntSegmentedStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntSegmented*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntSegmentedStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntSegmented*>(widget))
    {
        drawSegmented(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntSegmentedStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntSegmentedStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* seg = qobject_cast<AntSegmented*>(watched);
    if (seg && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(seg);
        option.rect = seg->rect();
        QPainter painter(seg);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, seg);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntSegmentedStyle::drawSegmented(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* seg = qobject_cast<const AntSegmented*>(widget);
    if (!seg || !painter || !option) return;

    const auto& token = antTheme->tokens();
    const auto options = seg->options();
    if (options.isEmpty()) return;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const int r = segmentedRadius(seg);
    const int itemR = segmentedItemRadius(seg);
    const bool enabled = option->state & QStyle::State_Enabled;
    const int selIdx = seg->selectedIndex();
    const int pressedIdx = seg->pressedIndex();
    const auto itemRects = seg->segmentRects();

    // Track background
    AntStyleBase::drawCrispRoundedRect(painter, option->rect,
        Qt::NoPen, token.colorBgLayout, r, r);

    for (int i = 0; i < options.size(); ++i)
    {
        if (!enabled || options[i].disabled || i == selIdx)
        {
            continue;
        }

        const bool isPressed = i == pressedIdx;
        const bool isHovered = i == seg->hoveredIndex();
        if (!isPressed && !isHovered)
        {
            continue;
        }

        AntStyleBase::drawCrispRoundedRect(painter, itemRects[i].toRect(),
            Qt::NoPen, itemOverlayColor(token, isPressed), itemR, itemR);
    }

    // Thumb
    const QRectF thumb = segmentedThumbRect(seg);
    if (!thumb.isEmpty())
    {
        drawSegmentedThumbShadow(painter, thumb, itemR);
        AntStyleBase::drawCrispRoundedRect(painter, thumb.toRect(),
            Qt::NoPen, token.colorBgElevated, itemR, itemR);
    }

    // Labels
    const int fontSize = segmentedFontSize(seg->segmentedSize());
    QFont f = painter->font();
    f.setPixelSize(fontSize);
    painter->setFont(f);

    for (int i = 0; i < options.size(); ++i)
    {
        const QRectF& r0 = itemRects[i];
        const bool isSel = (i == selIdx);
        const bool isHovered = (i == seg->hoveredIndex());
        const bool isDisabled = options[i].disabled;

        // Text color
        QColor textCol;
        if (!enabled || isDisabled)
        {
            textCol = token.colorTextDisabled;
        }
        else if (isSel)
        {
            textCol = token.colorText;
        }
        else if (isHovered)
        {
            textCol = token.colorText;
        }
        else
        {
            textCol = token.colorTextSecondary;
        }

        painter->setPen(textCol);
        if (!options[i].icon.isEmpty())
        {
            const QFontMetrics fm(f);
            const int iconSize = 14;
            const int iconGap = 6;
            const int textWidth = fm.horizontalAdvance(options[i].label);
            const int totalWidth = iconSize + iconGap + textWidth;
            const qreal left = r0.center().x() - totalWidth / 2.0;
            const QRectF iconRect(left, r0.center().y() - iconSize / 2.0, iconSize, iconSize);
            drawSegmentedIcon(painter, options[i].icon, iconRect, textCol);
            const QRectF textRect(left + iconSize + iconGap, r0.top(), textWidth + 2, r0.height());
            painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, options[i].label);
        }
        else
        {
            painter->drawText(r0, Qt::AlignCenter, options[i].label);
        }
    }

    painter->restore();
}
