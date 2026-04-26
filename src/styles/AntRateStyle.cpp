#include "AntRateStyle.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include <algorithm>
#include <cmath>

#include "styles/AntPalette.h"
#include "widgets/AntRate.h"

namespace
{
struct RateMetrics
{
    int starSize = 20;
    int margin = 8;
    int totalWidth = 0;
};

RateMetrics metricsFor(const AntRate* rate)
{
    const auto& token = antTheme->tokens();
    RateMetrics m;
    switch (rate ? rate->rateSize() : Ant::Size::Middle)
    {
        case Ant::Size::Small:
            m.starSize = static_cast<int>(std::round(token.controlHeightSM * 0.625));
            break;
        case Ant::Size::Large:
            m.starSize = static_cast<int>(std::round(token.controlHeightLG * 0.625));
            break;
        default:
            m.starSize = static_cast<int>(std::round(token.controlHeight * 0.625));
            break;
    }
    m.margin = token.marginXS;
    const int count = rate ? rate->count() : 5;
    m.totalWidth = count * m.starSize + (count - 1) * m.margin;
    return m;
}

QRectF starRectFor(const AntRate* rate, int index, const QRect& rect)
{
    const RateMetrics m = metricsFor(rate);
    const qreal x = rect.left() + index * (m.starSize + m.margin);
    const qreal y = rect.top() + (rect.height() - m.starSize) / 2.0;
    return QRectF(x, y, m.starSize, m.starSize);
}

QPainterPath starPath(const QRectF& rect)
{
    QPainterPath path;
    const qreal cx = rect.center().x();
    const qreal cy = rect.center().y();
    const qreal outerR = std::min(rect.width(), rect.height()) / 2.0;
    const qreal innerR = outerR * 0.4;

    for (int i = 0; i < 10; ++i)
    {
        const qreal r = (i % 2 == 0) ? outerR : innerR;
        const qreal angle = M_PI / 2.0 - i * 2.0 * M_PI / 10.0;
        const qreal x = cx + r * std::cos(angle);
        const qreal y = cy - r * std::sin(angle);
        if (i == 0)
        {
            path.moveTo(x, y);
        }
        else
        {
            path.lineTo(x, y);
        }
    }
    path.closeSubpath();
    return path;
}

void drawStar(QPainter* painter, const QRectF& rect, const QColor& color)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPath(starPath(rect));
    painter->restore();
}

int hoveredStarIndex(const AntRate* rate)
{
    const double hv = rate->hoverValue();
    if (hv < 0.0)
    {
        return -1;
    }
    return static_cast<int>(std::ceil(hv)) - 1;
}
}

AntRateStyle::AntRateStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntRate>();
}

void AntRateStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntRate*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

void AntRateStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntRate*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntRateStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntRate*>(widget))
    {
        drawRate(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

bool AntRateStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* rate = qobject_cast<AntRate*>(watched);
    if (rate && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(rate);
        option.rect = rate->rect();
        if (rate->isEnabled())
        {
            option.state |= QStyle::State_Enabled;
        }
        if (rate->hasFocus())
        {
            option.state |= QStyle::State_HasFocus;
        }

        QPainter painter(rate);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, rate);
        return false;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntRateStyle::drawRate(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* rate = qobject_cast<const AntRate*>(widget);
    if (!rate || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const RateMetrics m = metricsFor(rate);
    const int count = rate->count();
    const double effectiveValue = rate->hoverValue() >= 0.0 ? rate->hoverValue() : rate->value();
    const bool enabled = rate->isEnabled() && !rate->isDisabled();
    const bool focused = option->state.testFlag(QStyle::State_HasFocus);
    const int hoverIndex = hoveredStarIndex(rate);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < count; ++i)
    {
        QRectF rect = starRectFor(rate, i, option->rect);
        const double starFill = std::clamp(effectiveValue - i, 0.0, 1.0);

        // Hover scale effect
        const bool isHoveredStar = (i == hoverIndex) && enabled;
        if (isHoveredStar)
        {
            painter->save();
            painter->translate(rect.center());
            painter->scale(1.1, 1.1);
            painter->translate(-rect.center());
        }

        // Draw background star (unselected)
        QColor bgColor = token.colorFillTertiary;
        if (!enabled)
        {
            bgColor = AntPalette::disabledColor(bgColor, token.colorBgContainer);
        }
        drawStar(painter, rect, bgColor);

        // Draw foreground star (selected portion)
        if (starFill > 0.0)
        {
            QColor fgColor = token.colorRateStar;
            if (!enabled)
            {
                fgColor = AntPalette::disabledColor(fgColor, token.colorBgContainer);
            }

            painter->save();
            if (starFill < 1.0)
            {
                QRectF clipRect(rect.left(), rect.top(), rect.width() * starFill, rect.height());
                painter->setClipRect(clipRect);
            }
            drawStar(painter, rect, fgColor);
            painter->restore();
        }

        if (isHoveredStar)
        {
            painter->restore();
        }
    }

    // Focus outline around the whole rate area
    if (focused && enabled)
    {
        const QColor focusColor = AntPalette::alpha(token.colorPrimary, 0.22);
        QRectF focusRect(option->rect.left(), option->rect.top() + (option->rect.height() - m.starSize) / 2.0 - 2,
                         m.totalWidth, m.starSize + 4);
        AntStyleBase::drawCrispRoundedRect(painter, focusRect.toRect(),
            QPen(focusColor, token.controlOutlineWidth, Qt::DashLine), Qt::NoBrush,
            token.borderRadiusXS, token.borderRadiusXS);
    }

    painter->restore();
}
