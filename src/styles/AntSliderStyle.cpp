#include "AntSliderStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include <algorithm>

#include "styles/AntPalette.h"
#include "widgets/AntSlider.h"

namespace
{
}

AntSliderStyle::AntSliderStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntSliderStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntSlider*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

void AntSliderStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntSlider*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntSliderStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntSlider*>(widget))
    {
        drawSlider(option, painter, widget);
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

bool AntSliderStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* slider = qobject_cast<AntSlider*>(watched);
    if (slider && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(slider);
        option.rect = slider->rect();
        if (slider->isHoveredState())
        {
            option.state |= QStyle::State_MouseOver;
        }
        if (slider->isPressedState())
        {
            option.state |= QStyle::State_Sunken;
        }

        QPainter painter(slider);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, slider);
        return false;
    }

    return QProxyStyle::eventFilter(watched, event);
}

void AntSliderStyle::drawSlider(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* slider = qobject_cast<const AntSlider*>(widget);
    if (!slider || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const auto& cache = slider->layoutCache();
    const auto& metrics = cache.metrics;
    const QRectF groove = cache.grooveRect;
    const QRectF track = cache.trackRect;
    const QRectF handle = cache.valueHandleRect;
    const QRectF activeHandle = slider->isRangeMode()
        ? slider->cachedHandleRectForValue(cache, slider->activeDisplayValue())
        : handle;
    const bool interactive = option->state.testFlag(QStyle::State_Enabled);
    const bool active = interactive
        && (option->state.testFlag(QStyle::State_MouseOver)
            || option->state.testFlag(QStyle::State_Sunken)
            || option->state.testFlag(QStyle::State_HasFocus));

    QColor railColor = active ? token.colorFillTertiary : token.colorFillQuaternary;
    QColor trackColor = active ? token.colorPrimaryHover : token.colorPrimaryBorder;
    QColor handleBorder = active ? token.colorPrimary : token.colorPrimaryBorder;

    if (!interactive)
    {
        railColor = token.colorFillQuaternary;
        trackColor = token.colorBgContainerDisabled;
        handleBorder = token.colorTextDisabled;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    AntStyleBase::drawCrispRoundedRect(painter, groove.toRect(),
        Qt::NoPen, railColor, metrics.railSize / 2.0, metrics.railSize / 2.0);

    if (slider->included())
    {
        AntStyleBase::drawCrispRoundedRect(painter, track.toRect(),
            Qt::NoPen, trackColor, metrics.railSize / 2.0, metrics.railSize / 2.0);
    }

    if (slider->dots())
    {
        const int count = cache.dotCenters.size() - 1;
        if (count > 0)
        {
            const QColor dotBorder = slider->isEnabled() ? token.colorBorderSecondary : token.colorBorderDisabled;
            const QColor activeBorder = slider->isEnabled() ? token.colorPrimaryBorder : token.colorTextDisabled;
            const qreal ratio = slider->valueRatio();
            painter->save();
            painter->setBrush(token.colorBgElevated);
            for (int index = 0; index < cache.dotCenters.size(); ++index)
            {
                const qreal stepRatio = index / static_cast<qreal>(count);
                const bool dotActive = slider->isReverse() ? stepRatio >= ratio : stepRatio <= ratio;
                painter->setPen(QPen(dotActive ? activeBorder : dotBorder, 1.5));
                painter->drawEllipse(cache.dotCenters.at(index), metrics.dotSize / 2.0, metrics.dotSize / 2.0);
            }
            painter->restore();
        }
    }
    if (!cache.markLayouts.isEmpty())
    {
        QFont markFont = painter->font();
        markFont.setPixelSize(token.fontSizeSM);
        painter->setFont(markFont);
        for (const auto& mark : cache.markLayouts)
        {
            const bool markActive = slider->isRangeMode()
                ? mark.value >= slider->rangeStart() && mark.value <= slider->rangeEnd()
                : (slider->isReverse() ? mark.value >= slider->value() : mark.value <= slider->value());
            painter->setPen(QPen(markActive ? token.colorPrimaryBorder : token.colorBorderSecondary, 1.5));
            painter->setBrush(token.colorBgElevated);
            painter->drawEllipse(mark.center, metrics.dotSize / 2.0, metrics.dotSize / 2.0);
            painter->setPen(token.colorTextTertiary);
            painter->drawText(mark.textRect, Qt::AlignCenter, mark.text);
        }
    }

    if (slider->focusProgress() > 0.0 && interactive)
    {
        const QColor outline = AntPalette::alpha(token.colorPrimary, 0.20 * slider->focusProgress());
        painter->setPen(QPen(outline, 6 * slider->focusProgress(), Qt::SolidLine, Qt::RoundCap));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(activeHandle);
    }

    auto drawHandle = [&](const QRectF& handleRect) {
        painter->setPen(QPen(handleBorder, active ? 2.5 : 2.0));
        painter->setBrush(token.colorBgElevated);
        painter->drawEllipse(handleRect);
    };

    if (slider->isRangeMode())
    {
        drawHandle(cache.rangeStartHandleRect);
        drawHandle(cache.rangeEndHandleRect);
    }
    else
    {
        drawHandle(handle);
    }

    if (slider->isPressedState())
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(AntPalette::alpha(token.colorPrimary, 0.14));
        painter->drawEllipse(activeHandle.adjusted(-5, -5, 5, 5));
    }

    painter->restore();
}
