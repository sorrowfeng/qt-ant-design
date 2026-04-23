#include "AntSpinStyle.h"

#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include <algorithm>
#include <cmath>

#include "core/AntTheme.h"
#include "widgets/AntSpin.h"

namespace
{
struct SpinMetrics
{
    int indicatorSize = 20;
    int dotSize = 6;
    int fontSize = 14;
    int spacing = 8;
};

SpinMetrics metricsFor(const AntSpin* spin)
{
    const auto& token = antTheme->tokens();
    SpinMetrics metrics;
    metrics.fontSize = token.fontSize;

    if (!spin)
    {
        return metrics;
    }

    if (spin->spinSize() == Ant::SpinSize::Small)
    {
        metrics.indicatorSize = 14;
        metrics.dotSize = 4;
        metrics.fontSize = token.fontSizeSM;
        metrics.spacing = 6;
    }
    else if (spin->spinSize() == Ant::SpinSize::Large)
    {
        metrics.indicatorSize = 32;
        metrics.dotSize = 8;
        metrics.fontSize = token.fontSizeLG;
        metrics.spacing = 10;
    }

    return metrics;
}

void drawIndeterminate(QPainter& painter, const AntSpin* spin, const QRectF& rect)
{
    const auto& token = antTheme->tokens();
    const SpinMetrics metrics = metricsFor(spin);
    const QPointF center = rect.center();
    const qreal radius = rect.width() / 2.0 - metrics.dotSize / 2.0;
    constexpr qreal Pi = 3.14159265358979323846;

    for (int index = 0; index < 4; ++index)
    {
        const qreal radians = (spin->angle() + index * 90) * Pi / 180.0;
        QColor color = token.colorPrimary;
        color.setAlphaF(0.35 + 0.16 * index);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        const QPointF dot(center.x() + std::cos(radians) * radius,
                          center.y() + std::sin(radians) * radius);
        painter.drawEllipse(dot, metrics.dotSize / 2.0, metrics.dotSize / 2.0);
    }
}

void drawPercent(QPainter& painter, const AntSpin* spin, const QRectF& rect)
{
    const auto& token = antTheme->tokens();
    const SpinMetrics metrics = metricsFor(spin);
    const QRectF arcRect = rect.adjusted(2, 2, -2, -2);
    const int lineWidth = std::max(2, metrics.indicatorSize / 10);

    painter.setPen(QPen(token.colorFillTertiary, lineWidth, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(arcRect, 0, 360 * 16);

    painter.setPen(QPen(token.colorPrimary, lineWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(arcRect, 90 * 16, -spin->percent() * 360 * 16 / 100);

    if (spin->spinSize() != Ant::SpinSize::Small)
    {
        QFont font = painter.font();
        font.setPixelSize(spin->spinSize() == Ant::SpinSize::Large ? 10 : 8);
        font.setWeight(QFont::DemiBold);
        painter.setFont(font);
        painter.setPen(token.colorTextSecondary);
        painter.drawText(rect, Qt::AlignCenter, QStringLiteral("%1").arg(spin->percent()));
    }
}
}

AntSpinStyle::AntSpinStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* widget : widgets)
        {
            if (qobject_cast<AntSpin*>(widget) && widget->style() == this)
            {
                unpolish(widget);
                polish(widget);
                widget->updateGeometry();
                widget->update();
            }
        }
    });
}

void AntSpinStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
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
    QProxyStyle::unpolish(widget);
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
        return false;
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

    const auto& token = antTheme->tokens();
    const SpinMetrics metrics = metricsFor(spin);
    const int totalHeight = metrics.indicatorSize
        + (spin->description().isEmpty() ? 0 : metrics.spacing + metrics.fontSize);
    const QRectF indicator((option->rect.width() - metrics.indicatorSize) / 2.0,
                           (option->rect.height() - totalHeight) / 2.0,
                           metrics.indicatorSize,
                           metrics.indicatorSize);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (spin->percent() >= 0)
    {
        drawPercent(*painter, spin, indicator);
    }
    else
    {
        drawIndeterminate(*painter, spin, indicator);
    }

    if (!spin->description().isEmpty())
    {
        QFont font = painter->font();
        font.setPixelSize(metrics.fontSize);
        painter->setFont(font);
        painter->setPen(token.colorTextSecondary);
        const QRectF textRect(0, indicator.bottom() + metrics.spacing, option->rect.width(), metrics.fontSize + 4);
        painter->drawText(textRect, Qt::AlignHCenter | Qt::AlignTop, spin->description());
    }

    painter->restore();
}
