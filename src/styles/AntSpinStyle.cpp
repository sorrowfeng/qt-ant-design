#include "AntSpinStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include <cmath>

#include "widgets/AntSpin.h"

namespace
{
constexpr qreal Pi = 3.14159265358979323846;
}

AntSpinStyle::AntSpinStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntSpinStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    if (qobject_cast<AntSpin*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntSpinStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntSpin*>(widget))
    {
        widget->removeEventFilter(this);
    }
    AntStyleBase::unpolish(widget);
}

void AntSpinStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntSpin*>(widget))
    {
        drawSpin(option, painter, widget);
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

bool AntSpinStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* spin = qobject_cast<AntSpin*>(watched);
    if (spin && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(spin);
        option.rect = spin->rect();
        QPainter painter(spin);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, spin);
        return true;
    }

    return QProxyStyle::eventFilter(watched, event);
}

void AntSpinStyle::drawSpin(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* spin = qobject_cast<const AntSpin*>(widget);
    if (!spin || !painter || !option || !spin->isEffectiveSpinning())
    {
        return;
    }

    Q_UNUSED(option)

    const auto& layout = spin->spinLayout();
    if (!layout.effectiveSpinning)
    {
        return;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (spin->percent() >= 0)
    {
        painter->setPen(QPen(layout.trackColor, layout.percentLineWidth, Qt::SolidLine, Qt::RoundCap));
        painter->setBrush(Qt::NoBrush);
        painter->drawArc(layout.arcRect, 0, 360 * 16);

        painter->setPen(QPen(layout.primaryColor, layout.percentLineWidth, Qt::SolidLine, Qt::RoundCap));
        painter->drawArc(layout.arcRect, 90 * 16, -spin->percent() * 360 * 16 / 100);

        if (spin->spinSize() != Ant::Size::Small)
        {
            QFont font = painter->font();
            font.setPixelSize(layout.percentFontSize);
            font.setWeight(QFont::DemiBold);
            painter->setFont(font);
            painter->setPen(layout.textColor);
            painter->drawText(layout.indicatorRect, Qt::AlignCenter, layout.percentText);
        }
    }
    else
    {
        for (int index = 0; index < 4; ++index)
        {
            const qreal radians = (spin->angle() + index * 90) * Pi / 180.0;
            QColor color = layout.primaryColor;
            color.setAlphaF(0.35 + 0.16 * index);
            painter->setPen(Qt::NoPen);
            painter->setBrush(color);
            const QPointF dot(layout.indicatorCenter.x() + std::cos(radians) * layout.dotTravelRadius,
                              layout.indicatorCenter.y() + std::sin(radians) * layout.dotTravelRadius);
            painter->drawEllipse(dot, layout.metrics.dotSize / 2.0, layout.metrics.dotSize / 2.0);
        }
    }

    if (!spin->description().isEmpty())
    {
        QFont font = painter->font();
        font.setPixelSize(layout.metrics.fontSize);
        painter->setFont(font);
        painter->setPen(layout.textColor);
        painter->drawText(layout.textRect, Qt::AlignHCenter | Qt::AlignTop, layout.description);
    }

    painter->restore();
}
