#include "AntSegmentedStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

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

QVector<QRectF> segmentItemRects(const AntSegmented* seg, const QRect& widgetRect)
{
    QVector<QRectF> rects;
    const int n = seg->options().size();
    if (n == 0) return rects;

    const int pad = 2;
    const QRect track = widgetRect.adjusted(pad, pad, -pad, -pad);

    if (seg->isVertical())
    {
        const qreal segH = static_cast<qreal>(track.height()) / n;
        for (int i = 0; i < n; ++i)
        {
            rects.append(QRectF(track.x(), track.y() + i * segH, track.width(), segH));
        }
    }
    else
    {
        const qreal segW = static_cast<qreal>(track.width()) / n;
        for (int i = 0; i < n; ++i)
        {
            rects.append(QRectF(track.x() + i * segW, track.y(), segW, track.height()));
        }
    }
    return rects;
}

QRectF segmentedThumbRect(const AntSegmented* seg, const QRect& widgetRect)
{
    const auto rects = segmentItemRects(seg, widgetRect);
    const int idx = static_cast<int>(seg->thumbPosition());
    if (idx < 0 || idx >= rects.size()) return QRectF();

    const int r = segmentedRadius(seg);
    QRectF thumb = rects[idx];
    return thumb.adjusted(1, 1, -1, -1);
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
    const bool enabled = option->state & QStyle::State_Enabled;
    const int selIdx = static_cast<int>(seg->thumbPosition());
    const auto itemRects = segmentItemRects(seg, option->rect);

    // Track background
    const QRectF track = option->rect.adjusted(2, 2, -2, -2);
    AntStyleBase::drawCrispRoundedRect(painter, track.toRect(),
        Qt::NoPen, token.colorFillQuaternary, r, r);

    // Thumb
    if (selIdx >= 0 && selIdx < itemRects.size())
    {
        QRectF thumb = itemRects[selIdx].adjusted(1, 1, -1, -1);
        antTheme->drawEffectShadow(painter, thumb.toRect(), 6, 4, 0.08);
        AntStyleBase::drawCrispRoundedRect(painter, thumb.toRect(),
            Qt::NoPen, token.colorBgContainer, r - 1, r - 1);
    }

    // Separators
    painter->setPen(QPen(token.colorSplit, token.lineWidth));
    for (int i = 1; i < itemRects.size(); ++i)
    {
        if (i == selIdx || i - 1 == selIdx) continue;
        const QRectF& r0 = itemRects[i];
        if (seg->isVertical())
        {
            const qreal y = r0.top();
            painter->drawLine(QPointF(r0.left() + 12, y), QPointF(r0.right() - 12, y));
        }
        else
        {
            const qreal x = r0.left();
            painter->drawLine(QPointF(x, r0.top() + 6), QPointF(x, r0.bottom() - 6));
        }
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
            textCol = token.colorPrimary;
        }
        else
        {
            textCol = token.colorTextSecondary;
        }

        // Hover overlay
        if (isHovered && !isSel && !isDisabled && enabled)
        {
            QColor hoverBg = token.colorPrimary;
            hoverBg.setAlphaF(0.06);
            AntStyleBase::drawCrispRoundedRect(painter, r0.toRect(),
                Qt::NoPen, hoverBg, r - 1, r - 1);
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
