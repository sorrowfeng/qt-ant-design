#include "AntSliderStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include <algorithm>

#include "styles/AntPalette.h"
#include "widgets/AntSlider.h"

namespace
{
struct SliderMetrics
{
    int controlSize = 10;
    int railSize = 4;
    int handleSize = 10;
    int handleSizeHover = 12;
    int dotSize = 8;
};

SliderMetrics metricsFor(const AntSlider* slider)
{
    const auto& token = antTheme->tokens();
    SliderMetrics metrics;
    metrics.controlSize = token.controlHeightLG / 4;
    metrics.railSize = 4;
    metrics.handleSize = metrics.controlSize;
    metrics.handleSizeHover = token.controlHeightSM / 2;
    metrics.dotSize = 8;
    Q_UNUSED(slider)
    return metrics;
}

QRectF grooveRectFor(const AntSlider* slider, const QRect& rect)
{
    const SliderMetrics metrics = metricsFor(slider);
    const qreal halfHandle = metrics.handleSizeHover / 2.0 + 2.0;
    if (slider && slider->orientation() == Qt::Vertical)
    {
        const qreal height = std::max<qreal>(0.0, rect.height() - halfHandle * 2.0);
        return QRectF((rect.width() - metrics.railSize) / 2.0, halfHandle, metrics.railSize, height);
    }

    const qreal width = std::max<qreal>(0.0, rect.width() - halfHandle * 2.0);
    return QRectF(halfHandle, (rect.height() - metrics.railSize) / 2.0, width, metrics.railSize);
}

qreal valueRatioFor(const AntSlider* slider)
{
    if (!slider || slider->maximum() == slider->minimum())
    {
        return 0.0;
    }

    qreal ratio = (slider->value() - slider->minimum())
        / static_cast<qreal>(slider->maximum() - slider->minimum());
    if (slider->isReverse())
    {
        ratio = 1.0 - ratio;
    }
    return std::clamp(ratio, 0.0, 1.0);
}

QRectF trackRectFor(const AntSlider* slider, const QRect& rect)
{
    const SliderMetrics metrics = metricsFor(slider);
    const QRectF groove = grooveRectFor(slider, rect);
    const qreal ratio = valueRatioFor(slider);

    if (slider && slider->orientation() == Qt::Vertical)
    {
        const qreal handleY = groove.bottom() - groove.height() * ratio;
        if (slider->isReverse())
        {
            return QRectF(groove.left(), groove.top(), groove.width(), handleY - groove.top());
        }
        return QRectF(groove.left(), handleY, groove.width(), groove.bottom() - handleY);
    }

    const qreal handleX = groove.left() + groove.width() * ratio;
    if (slider && slider->isReverse())
    {
        return QRectF(handleX, groove.top(), groove.right() - handleX, groove.height());
    }
    return QRectF(groove.left(), groove.top(), handleX - groove.left(), groove.height());
}

QRectF handleRectFor(const AntSlider* slider, const QRect& rect)
{
    const SliderMetrics metrics = metricsFor(slider);
    const QRectF groove = grooveRectFor(slider, rect);
    const qreal size = metrics.handleSize * (slider ? slider->handleScale() : 1.0);
    QPointF center;

    if (slider && slider->orientation() == Qt::Vertical)
    {
        center = QPointF(groove.center().x(), groove.bottom() - groove.height() * valueRatioFor(slider));
    }
    else
    {
        center = QPointF(groove.left() + groove.width() * valueRatioFor(slider), groove.center().y());
    }

    return QRectF(center.x() - size / 2.0, center.y() - size / 2.0, size, size);
}

void drawDots(QPainter& painter, const AntSlider* slider, const SliderMetrics& metrics, const QRectF& groove)
{
    if (!slider)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const int count = std::max(1, (slider->maximum() - slider->minimum()) / std::max(1, slider->singleStep()));
    if (count > 80)
    {
        return;
    }

    const QColor dotBorder = slider->isEnabled() ? token.colorBorderSecondary : token.colorBorderDisabled;
    const QColor activeBorder = slider->isEnabled() ? token.colorPrimaryBorder : token.colorTextDisabled;
    const qreal ratio = slider->maximum() == slider->minimum()
        ? 0.0
        : (slider->value() - slider->minimum()) / static_cast<qreal>(slider->maximum() - slider->minimum());

    painter.save();
    painter.setBrush(token.colorBgElevated);
    for (int index = 0; index <= count; ++index)
    {
        const qreal stepRatio = index / static_cast<qreal>(count);
        const bool active = slider->isReverse() ? stepRatio >= ratio : stepRatio <= ratio;
        painter.setPen(QPen(active ? activeBorder : dotBorder, 1.5));

        QPointF center;
        if (slider->orientation() == Qt::Vertical)
        {
            center = QPointF(groove.center().x(), groove.bottom() - groove.height() * stepRatio);
        }
        else
        {
            center = QPointF(groove.left() + groove.width() * stepRatio, groove.center().y());
        }

        painter.drawEllipse(center, metrics.dotSize / 2.0, metrics.dotSize / 2.0);
    }
    painter.restore();
}
}

AntSliderStyle::AntSliderStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntSlider>();
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
    const SliderMetrics metrics = metricsFor(slider);
    const QRectF groove = grooveRectFor(slider, option->rect);
    const QRectF track = trackRectFor(slider, option->rect);
    const QRectF handle = handleRectFor(slider, option->rect);
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
        drawDots(*painter, slider, metrics, groove);
    }

    if (slider->focusProgress() > 0.0 && interactive)
    {
        const QColor outline = AntPalette::alpha(token.colorPrimary, 0.20 * slider->focusProgress());
        painter->setPen(QPen(outline, 6 * slider->focusProgress(), Qt::SolidLine, Qt::RoundCap));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(handle);
    }

    painter->setPen(QPen(handleBorder, active ? 2.5 : 2.0));
    painter->setBrush(token.colorBgElevated);
    painter->drawEllipse(handle);

    if (slider->isPressedState())
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(AntPalette::alpha(token.colorPrimary, 0.14));
        painter->drawEllipse(handle.adjusted(-5, -5, 5, 5));
    }

    painter->restore();
}
