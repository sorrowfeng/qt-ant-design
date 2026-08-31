#include "AntBorderBeamStyle.h"

#include "../widgets/AntBorderBeam.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>

#include <cmath>

namespace
{
// 将沿路径的偏移量（像素，可为负或超过周长）规范到 [0, perimeter)。
qreal wrapOffset(qreal offset, qreal perimeter)
{
    qreal value = std::fmod(offset, perimeter);
    if (value < 0.0)
    {
        value += perimeter;
    }
    return value;
}

// 光束头尾透明度系数：头部最亮，向尾部线性衰减。
qreal beamAlpha(qreal distanceFromHead, qreal length)
{
    if (length <= 0.0)
    {
        return 1.0;
    }
    const qreal t = qBound(0.0, distanceFromHead / length, 1.0);
    return 1.0 - t;
}
} // namespace

AntBorderBeamStyle::AntBorderBeamStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntBorderBeam>();
}

void AntBorderBeamStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    installPaintFilter<AntBorderBeam>(widget);
}

void AntBorderBeamStyle::unpolish(QWidget* widget)
{
    removePaintFilter<AntBorderBeam>(widget);
    QProxyStyle::unpolish(widget);
}

bool AntBorderBeamStyle::drawWidget(QWidget* widget, QPaintEvent* /*event*/)
{
    auto* beam = qobject_cast<AntBorderBeam*>(widget);
    if (!beam)
    {
        return false;
    }

    const int lineWidth = beam->lineWidth();
    const qreal radius = beam->resolvedBorderRadius();
    const QColor beamColor = beam->resolvedBeamColor();

    QPainter painter(widget);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 光束沿边框中心线运动，路径内收半个线宽避免裁切。
    const qreal inset = lineWidth / 2.0 + 0.5;
    const QRectF borderRect = QRectF(widget->rect()).adjusted(inset, inset, -inset, -inset);
    if (borderRect.width() <= 1.0 || borderRect.height() <= 1.0)
    {
        return true;
    }

    QPainterPath borderPath;
    borderPath.addRoundedRect(borderRect, radius, radius);
    const qreal perimeter = borderPath.length();
    if (perimeter <= 0.0)
    {
        return true;
    }

    const qreal length = qMin<qreal>(beam->beamLength(), perimeter / 2.0);
    constexpr int segmentCount = 24;
    const qreal segmentLength = length / segmentCount;
    const int beamCount = beam->count();
    const qreal phase = beam->phase();
    const bool dimmed = !widget->isEnabled();

    for (int i = 0; i < beamCount; ++i)
    {
        const qreal headOffset = wrapOffset((phase + static_cast<qreal>(i) / beamCount) * perimeter, perimeter);

        for (int s = 0; s < segmentCount; ++s)
        {
            const qreal segStart = headOffset - (s + 1) * segmentLength;
            const qreal segEnd = headOffset - s * segmentLength;

            QPainterPath segment;
            segment.moveTo(borderPath.pointAtPercent(wrapOffset(segStart, perimeter) / perimeter));
            segment.lineTo(borderPath.pointAtPercent(wrapOffset(segEnd, perimeter) / perimeter));

            const qreal alphaFactor = beamAlpha(s * segmentLength, length);
            QColor segmentColor = beamColor;
            segmentColor.setAlphaF(segmentColor.alphaF() * alphaFactor * (dimmed ? 0.45 : 1.0));

            // 外层辉光 + 核心光束两笔，形成柔和发展效果。
            QPen glowPen(segmentColor, lineWidth * 2.4, Qt::SolidLine, Qt::RoundCap);
            QColor glowColor = segmentColor;
            glowColor.setAlphaF(glowColor.alphaF() * 0.35);
            glowPen.setColor(glowColor);
            painter.strokePath(segment, glowPen);

            painter.strokePath(segment, QPen(segmentColor, lineWidth, Qt::SolidLine, Qt::RoundCap));
        }
    }
    return true;
}
