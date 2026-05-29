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
void drawStar(QPainter* painter, const QPainterPath& path, const QColor& color)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPath(path);
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
}

void AntRateStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
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
    AntStyleBase::unpolish(widget);
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
        return true;
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
    const auto& cache = rate->layoutCache();
    const int count = cache.starRects.size();
    const double effectiveValue = rate->hoverValue() >= 0.0 ? rate->hoverValue() : rate->value();
    const bool enabled = rate->isEnabled() && !rate->isDisabled();
    const bool focused = option->state.testFlag(QStyle::State_HasFocus);
    const int hoverIndex = hoveredStarIndex(rate);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < count; ++i)
    {
        const QRectF rect = cache.starRects.at(i);
        const QPainterPath& path = cache.starPaths.at(i);
        const double starFill = std::clamp(effectiveValue - i, 0.0, 1.0);

        const bool isHoveredStar = (i == hoverIndex) && enabled;
        const bool isSelectionAnimatedStar = (i == rate->m_selectionAnimationIndex) && enabled;
        const qreal scale = std::max(isHoveredStar ? 1.1 : 1.0,
                                     isSelectionAnimatedStar ? rate->m_selectionScale : 1.0);
        const bool isScaledStar = !qFuzzyCompare(scale, 1.0);
        if (isScaledStar)
        {
            painter->save();
            painter->translate(rect.center());
            painter->scale(scale, scale);
            painter->translate(-rect.center());
        }

        // Draw background star (unselected)
        QColor bgColor = token.colorFillTertiary;
        if (!enabled)
        {
            bgColor = AntPalette::disabledColor(bgColor, token.colorBgContainer);
        }
        drawStar(painter, path, bgColor);

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
            drawStar(painter, path, fgColor);
            painter->restore();
        }

        if (isScaledStar)
        {
            painter->restore();
        }
    }

    // Focus outline around the whole rate area
    if (focused && enabled)
    {
        const QColor focusColor = AntPalette::alpha(token.colorPrimary, 0.22);
        const qreal startX = option->rect.left() + (option->rect.width() - cache.totalWidth) / 2.0;
        QRectF focusRect(startX, option->rect.top() + (option->rect.height() - cache.starSize) / 2.0 - 2,
                         cache.totalWidth, cache.starSize + 4);
        AntStyleBase::drawCrispRoundedRect(painter, focusRect.toRect(),
            QPen(focusColor, token.controlOutlineWidth, Qt::DashLine), Qt::NoBrush,
            token.borderRadiusXS, token.borderRadiusXS);
    }

    painter->restore();
}
