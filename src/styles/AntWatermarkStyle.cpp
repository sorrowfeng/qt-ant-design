#include "AntWatermarkStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include <cmath>

#include "widgets/AntWatermark.h"

namespace
{

QSize watermarkTextSize(const AntWatermark* wm, QPainter* painter)
{
    const QStringList lines = wm->content();
    if (lines.isEmpty()) return QSize(0, 0);

    const AntWatermarkFont& f = wm->watermarkFont();
    QFont font;
    if (!f.fontFamily.isEmpty()) font.setFamily(f.fontFamily);
    font.setPixelSize(f.fontSize);
    font.setWeight(QFont::Weight(f.fontWeight));
    painter->setFont(font);
    QFontMetrics fm(font);

    int maxWidth = 0;
    int totalHeight = 0;
    for (const QString& line : lines)
    {
        maxWidth = qMax(maxWidth, fm.horizontalAdvance(line));
        totalHeight += fm.height() + 2;
    }
    return QSize(maxWidth, totalHeight);
}

void drawWatermarkPattern(QPainter* painter, const QPointF& center, const AntWatermark* wm)
{
    const QStringList lines = wm->content();
    if (lines.isEmpty()) return;

    const AntWatermarkFont& fData = wm->watermarkFont();
    QFont font;
    if (!fData.fontFamily.isEmpty()) font.setFamily(fData.fontFamily);
    font.setPixelSize(fData.fontSize);
    font.setWeight(QFont::Weight(fData.fontWeight));
    painter->setFont(font);

    QFontMetrics fm(font);
    QColor textColor = fData.color;
    if (!textColor.isValid())
    {
        const auto& token = antTheme->tokens();
        textColor = token.colorText;
        textColor.setAlphaF(0.18);
    }
    painter->setPen(textColor);

    const qreal lineHeight = fm.height() + 2;
    qreal y = center.y() - (lines.size() * lineHeight) / 2.0;
    for (const QString& line : lines)
    {
        const int textWidth = fm.horizontalAdvance(line);
        painter->drawText(QRectF(center.x() - textWidth / 2.0, y, textWidth, lineHeight),
                          Qt::AlignCenter, line);
        y += lineHeight;
    }
}

} // namespace

AntWatermarkStyle::AntWatermarkStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntWatermark>();
}

void AntWatermarkStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntWatermark*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntWatermarkStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntWatermark*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntWatermarkStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntWatermark*>(widget))
    {
        drawWatermark(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntWatermarkStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntWatermarkStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* wm = qobject_cast<AntWatermark*>(watched);
    if (wm && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(wm);
        option.rect = wm->rect();
        QPainter painter(wm);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, wm);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntWatermarkStyle::drawWatermark(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* wm = qobject_cast<const AntWatermark*>(widget);
    if (!wm || !painter || !option) return;

    const QStringList lines = wm->content();
    if (lines.isEmpty()) return;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const QSize textSize = watermarkTextSize(wm, painter);
    if (textSize.isEmpty())
    {
        painter->restore();
        return;
    }

    const QSize gap = wm->gap();
    const qreal tileW = textSize.width() + gap.width();
    const qreal tileH = textSize.height() + gap.height();
    const qreal rotation = wm->rotate();
    const QPoint offset = wm->offset();

    // Calculate coverage expanded for rotation
    const qreal radians = qDegreesToRadians(qAbs(rotation));
    const qreal expandedW = tileW * qCos(radians) + tileH * qSin(radians);
    const qreal expandedH = tileW * qSin(radians) + tileH * qCos(radians);

    const QRectF r = option->rect;
    const qreal margin = qMax(expandedW, expandedH);

    const qreal startX = r.left() - margin;
    const qreal startY = r.top() - margin;
    const qreal endX = r.right() + margin;
    const qreal endY = r.bottom() + margin;

    for (qreal ty = startY; ty < endY; ty += tileH)
    {
        for (qreal tx = startX; tx < endX; tx += tileW)
        {
            painter->save();
            const QPointF tileCenter(tx + tileW / 2.0 + offset.x(),
                                     ty + tileH / 2.0 + offset.y());
            painter->translate(tileCenter);
            painter->rotate(rotation);
            drawWatermarkPattern(painter, QPointF(0, 0), wm);
            painter->restore();
        }
    }

    painter->restore();
}
