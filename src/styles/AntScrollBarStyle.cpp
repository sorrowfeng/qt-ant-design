#include "AntScrollBarStyle.h"

#include <QPainter>
#include <QStyleOptionSlider>

#include "styles/AntPalette.h"
#include "widgets/AntScrollBar.h"

namespace
{
struct ScrollBarMetrics
{
    int thickness = 8;
    int handleMinSize = 32;
    int handleRadius = 4;
};

ScrollBarMetrics metricsFor(const AntScrollBar* scrollBar)
{
    Q_UNUSED(scrollBar)
    ScrollBarMetrics metrics;
    metrics.thickness = 8;
    metrics.handleMinSize = 32;
    metrics.handleRadius = 4;
    return metrics;
}

bool isVertical(const QWidget* widget)
{
    const auto* scrollBar = qobject_cast<const QScrollBar*>(widget);
    return scrollBar && scrollBar->orientation() == Qt::Vertical;
}

bool isHovered(const QWidget* widget)
{
    const auto* scrollBar = qobject_cast<const AntScrollBar*>(widget);
    return scrollBar && scrollBar->isHoveredState();
}

bool isPressed(const QWidget* widget)
{
    const auto* scrollBar = qobject_cast<const AntScrollBar*>(widget);
    return scrollBar && scrollBar->isPressedState();
}

bool isAutoHide(const QWidget* widget)
{
    const auto* scrollBar = qobject_cast<const AntScrollBar*>(widget);
    return scrollBar && scrollBar->autoHide();
}

QColor handleColorFor(const QWidget* widget, const QStyle::State& state)
{
    const auto& token = antTheme->tokens();
    if (!state.testFlag(QStyle::State_Enabled))
    {
        return token.colorTextDisabled;
    }
    if (state.testFlag(QStyle::State_Sunken) || isPressed(widget))
    {
        return token.colorTextSecondary;
    }
    if (state.testFlag(QStyle::State_MouseOver) || isHovered(widget))
    {
        return token.colorTextTertiary;
    }
    return token.colorBorder;
}

qreal handleAlphaFor(const QWidget* widget, const QStyle::State& state)
{
    if (!state.testFlag(QStyle::State_Enabled))
    {
        return isAutoHide(widget) ? 0.0 : 0.45;
    }
    if (!isAutoHide(widget))
    {
        return 1.0;
    }
    if (state.testFlag(QStyle::State_Sunken) || isPressed(widget))
    {
        return 1.0;
    }
    if (state.testFlag(QStyle::State_MouseOver) || isHovered(widget))
    {
        return 0.65;
    }
    return 0.0;
}
}

AntScrollBarStyle::AntScrollBarStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntScrollBar>();
}

void AntScrollBarStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntScrollBar*>(widget))
    {
        widget->setAttribute(Qt::WA_Hover, true);
        widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
    }
}

void AntScrollBarStyle::unpolish(QWidget* widget)
{
    QProxyStyle::unpolish(widget);
}

void AntScrollBarStyle::drawComplexControl(ComplexControl control,
                                           const QStyleOptionComplex* option,
                                           QPainter* painter,
                                           const QWidget* widget) const
{
    if (control == QStyle::CC_ScrollBar && qobject_cast<const AntScrollBar*>(widget))
    {
        drawScrollBar(option, painter, widget);
        return;
    }

    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

QRect AntScrollBarStyle::subControlRect(ComplexControl control,
                                        const QStyleOptionComplex* option,
                                        SubControl subControl,
                                        const QWidget* widget) const
{
    if (control == QStyle::CC_ScrollBar)
    {
        const auto* scrollBar = qobject_cast<const AntScrollBar*>(widget);
        const ScrollBarMetrics m = metricsFor(scrollBar);
        const QRect rect = option ? option->rect : QRect();

        const bool vertical = isVertical(widget);

        switch (subControl)
        {
        case SC_ScrollBarGroove:
            return rect;

        case SC_ScrollBarSlider:
        {
            const auto* scrollOption = qstyleoption_cast<const QStyleOptionSlider*>(option);
            if (!scrollOption)
            {
                return QRect();
            }

            if (vertical)
            {
                const int grooveHeight = rect.height();
                const int range = scrollOption->maximum - scrollOption->minimum;
                int sliderHeight = grooveHeight;
                if (range > 0)
                {
                    const int pageStep = qMax(scrollOption->pageStep, 1);
                    const int totalSteps = range + pageStep;
                    sliderHeight = qMax(m.handleMinSize, static_cast<int>(
                        static_cast<qreal>(pageStep) / totalSteps * grooveHeight));
                    sliderHeight = qMin(sliderHeight, grooveHeight);
                }

                const int sliderRange = grooveHeight - sliderHeight;
                const int value = scrollOption->sliderValue - scrollOption->minimum;
                const int sliderPos = (range > 0)
                    ? static_cast<int>(static_cast<qreal>(value) / range * sliderRange)
                    : 0;

                return QRect(rect.x(), rect.y() + sliderPos, rect.width(), sliderHeight);
            }
            else
            {
                const int grooveWidth = rect.width();
                const int range = scrollOption->maximum - scrollOption->minimum;
                int sliderWidth = grooveWidth;
                if (range > 0)
                {
                    const int pageStep = qMax(scrollOption->pageStep, 1);
                    const int totalSteps = range + pageStep;
                    sliderWidth = qMax(m.handleMinSize, static_cast<int>(
                        static_cast<qreal>(pageStep) / totalSteps * grooveWidth));
                    sliderWidth = qMin(sliderWidth, grooveWidth);
                }

                const int sliderRange = grooveWidth - sliderWidth;
                const int value = scrollOption->sliderValue - scrollOption->minimum;
                const int sliderPos = (range > 0)
                    ? static_cast<int>(static_cast<qreal>(value) / range * sliderRange)
                    : 0;

                return QRect(rect.x() + sliderPos, rect.y(), sliderWidth, rect.height());
            }
        }

        case SC_ScrollBarAddLine:
        case SC_ScrollBarSubLine:
        case SC_ScrollBarAddPage:
        case SC_ScrollBarSubPage:
            return QRect();

        default:
            break;
        }

        return QRect();
    }

    return QProxyStyle::subControlRect(control, option, subControl, widget);
}

int AntScrollBarStyle::pixelMetric(PixelMetric metric,
                                   const QStyleOption* option,
                                   const QWidget* widget) const
{
    const auto* scrollBar = qobject_cast<const AntScrollBar*>(widget);
    const ScrollBarMetrics m = metricsFor(scrollBar);

    switch (metric)
    {
    case PM_ScrollBarExtent:
        return m.thickness;

    case PM_ScrollBarSliderMin:
        return m.handleMinSize;

    default:
        break;
    }

    return QProxyStyle::pixelMetric(metric, option, widget);
}

void AntScrollBarStyle::drawScrollBar(const QStyleOptionComplex* option,
                                      QPainter* painter,
                                      const QWidget* widget) const
{
    if (!option || !painter || !widget)
    {
        return;
    }

    const auto* scrollOption = qstyleoption_cast<const QStyleOptionSlider*>(option);
    if (!scrollOption)
    {
        return;
    }

    const ScrollBarMetrics m = metricsFor(qobject_cast<const AntScrollBar*>(widget));
    const QRect grooveRect = subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget);
    const QRect sliderRect = subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget);

    if (grooveRect.isEmpty())
    {
        return;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    // Groove: transparent background
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::transparent);
    painter->drawRect(grooveRect);

    // Handle (slider)
    if (!sliderRect.isEmpty())
    {
        const QColor color = handleColorFor(widget, option->state);
        const qreal alpha = handleAlphaFor(widget, option->state);

        if (alpha > 0.0)
        {
            QColor handleColor = color;
            handleColor.setAlphaF(alpha);

            const bool vertical = isVertical(widget);
            const int radius = m.handleRadius;

            QRect handleRect = sliderRect;
            if (vertical)
            {
                const int inset = (handleRect.width() - m.thickness) / 2;
                if (inset > 0)
                {
                    handleRect.adjust(inset, 0, -inset, 0);
                }
            }
            else
            {
                const int inset = (handleRect.height() - m.thickness) / 2;
                if (inset > 0)
                {
                    handleRect.adjust(0, inset, 0, -inset);
                }
            }

            AntStyleBase::drawCrispRoundedRect(painter, handleRect,
                Qt::NoPen, handleColor, radius, radius);
        }
    }

    painter->restore();
}
