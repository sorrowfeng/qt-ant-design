#include "AntEmptyStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include "core/AntStyleBase.h"
#include "styles/AntPalette.h"
#include "widgets/AntEmpty.h"

namespace
{

QSize emptyEffectiveImageSize(const AntEmpty* empty)
{
    const QSize imageSize = empty->imageSize();
    if (imageSize.isValid() && !imageSize.isEmpty())
    {
        return imageSize;
    }
    return empty->isSimple() ? QSize(96, 56) : QSize(121, 100);
}

QRect emptyImageRect(const AntEmpty* empty, const QRect& widgetRect)
{
    const auto& token = antTheme->tokens();
    const QSize image = emptyEffectiveImageSize(empty);
    return QRect((widgetRect.width() - image.width()) / 2, token.padding, image.width(), image.height());
}

QRect emptyDescriptionRect(const AntEmpty* empty, const QRect& widgetRect)
{
    const auto& token = antTheme->tokens();
    const int top = empty->imageVisible() ? emptyImageRect(empty, widgetRect).bottom() + token.marginXS : token.padding;
    QWidget* extra = empty->extraWidget();
    const int bottom = extra ? widgetRect.height() - token.padding - extra->sizeHint().height() - token.margin
                             : widgetRect.height() - token.padding;
    return QRect(token.paddingSM, top, qMax(40, widgetRect.width() - token.paddingSM * 2),
                 qMax(token.fontSize + 4, bottom - top));
}

} // namespace

AntEmptyStyle::AntEmptyStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntEmpty>();
}

void AntEmptyStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntEmpty*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntEmptyStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntEmpty*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntEmptyStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntEmpty*>(widget))
    {
        drawEmpty(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntEmptyStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntEmptyStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* empty = qobject_cast<AntEmpty*>(watched);
    if (empty && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(empty);
        option.rect = empty->rect();
        QPainter painter(empty);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, empty);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntEmptyStyle::drawEmpty(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* empty = qobject_cast<const AntEmpty*>(widget);
    if (!empty || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (empty->imageVisible())
    {
        const QRect r = emptyImageRect(empty, option->rect);
        const bool isDark = antTheme->themeMode() == Ant::ThemeMode::Dark;
        const bool simple = empty->isSimple();
        const QColor primary = AntPalette::alpha(token.colorTextTertiary, isDark ? 0.5 : 0.32);
        const QColor fill = AntPalette::alpha(token.colorFillQuaternary, isDark ? 0.78 : 1.0);
        const QColor line = AntPalette::alpha(token.colorTextTertiary, isDark ? 0.68 : 0.45);

        painter->save();
        painter->translate(r.topLeft());

        if (simple)
        {
            painter->scale(r.width() / 128.0, r.height() / 80.0);

            painter->setPen(Qt::NoPen);
            painter->setBrush(AntPalette::alpha(token.colorPrimary, 0.12));
            painter->drawEllipse(QRectF(16, 58, 96, 14));

            AntStyleBase::drawCrispRoundedRect(painter, QRect(34, 10, 60, 46), Qt::NoPen, fill, 10, 10);
            AntStyleBase::drawCrispRoundedRect(painter, QRect(42, 18, 44, 30), Qt::NoPen,
                AntPalette::alpha(token.colorBgContainer, 0.88), 6, 6);

            painter->setPen(QPen(primary, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawArc(QRectF(10, 20, 26, 26), 35 * 16, 260 * 16);
            painter->drawArc(QRectF(92, 18, 22, 22), 220 * 16, 220 * 16);

            painter->setPen(QPen(line, 2.2, Qt::SolidLine, Qt::RoundCap));
            painter->drawLine(QPointF(49, 27), QPointF(79, 27));
            painter->drawLine(QPointF(49, 34), QPointF(73, 34));
        }
        else
        {
            painter->scale(r.width() / 184.0, r.height() / 152.0);

            const qreal themeOpacity = isDark ? 0.65 : 1.0;
            const auto color = [themeOpacity](int red, int green, int blue, qreal alpha = 1.0) {
                QColor c(red, green, blue);
                c.setAlphaF(qBound<qreal>(0.0, alpha * themeOpacity, 1.0));
                return c;
            };
            const QColor shadow = color(245, 245, 247, 0.8);
            const QColor trayBack = color(174, 184, 194);
            const QColor paper = color(245, 245, 247);
            const QColor ink = color(220, 224, 230);
            const QColor white = color(255, 255, 255);

            painter->setPen(Qt::NoPen);

            painter->setBrush(shadow);
            painter->drawEllipse(QRectF(24.0, 125.9, 135.6, 25.4));

            QPainterPath backPath;
            backPath.moveTo(146.0, 101.4);
            backPath.lineTo(122.1, 71.9);
            backPath.cubicTo(120.9, 70.5, 119.2, 69.7, 117.5, 69.7);
            backPath.lineTo(66.1, 69.7);
            backPath.cubicTo(64.4, 69.7, 62.7, 70.5, 61.5, 71.9);
            backPath.lineTo(37.5, 101.4);
            backPath.lineTo(37.5, 116.7);
            backPath.lineTo(146.0, 116.7);
            backPath.closeSubpath();
            painter->setBrush(trayBack);
            painter->drawPath(backPath);

            QPainterPath paperPath;
            paperPath.addRoundedRect(QRectF(57.8, 31.7, 76.0, 101.3), 4.0, 4.0);
            painter->setBrush(paper);
            painter->drawPath(paperPath);

            painter->setBrush(ink);
            painter->drawRoundedRect(QRectF(66.7, 41.7, 50.2, 29.0), 2.0, 2.0);
            painter->drawRoundedRect(QRectF(66.9, 81.5, 49.8, 4.5), 2.25, 2.25);
            painter->drawRoundedRect(QRectF(66.9, 93.2, 49.8, 4.6), 2.3, 2.3);

            QPainterPath frontPath;
            frontPath.moveTo(37.6, 101.4);
            frontPath.lineTo(63.9, 101.4);
            frontPath.cubicTo(66.8, 101.4, 69.1, 103.8, 69.1, 106.8);
            frontPath.cubicTo(69.1, 109.8, 71.5, 112.2, 74.4, 112.2);
            frontPath.lineTo(109.2, 112.2);
            frontPath.cubicTo(112.1, 112.2, 114.5, 109.8, 114.5, 106.8);
            frontPath.cubicTo(114.5, 103.8, 116.8, 101.4, 119.7, 101.4);
            frontPath.lineTo(146.0, 101.4);
            frontPath.lineTo(146.0, 134.9);
            frontPath.cubicTo(146.0, 135.5, 145.9, 136.1, 145.8, 136.7);
            frontPath.cubicTo(145.1, 139.9, 142.3, 142.1, 139.1, 142.1);
            frontPath.lineTo(44.5, 142.1);
            frontPath.cubicTo(41.3, 142.1, 38.5, 139.9, 37.8, 136.7);
            frontPath.cubicTo(37.7, 136.1, 37.6, 135.5, 37.6, 134.9);
            frontPath.closeSubpath();
            painter->drawPath(frontPath);

            QPainterPath bubblePath;
            bubblePath.moveTo(149.1, 33.3);
            bubblePath.lineTo(142.3, 35.9);
            bubblePath.cubicTo(141.5, 36.2, 140.8, 35.5, 141.0, 34.7);
            bubblePath.lineTo(143.0, 28.5);
            bubblePath.cubicTo(140.3, 25.5, 138.9, 22.0, 138.8, 18.1);
            bubblePath.cubicTo(138.8, 8.1, 148.9, 0.0, 161.4, 0.0);
            bubblePath.cubicTo(173.9, 0.0, 184.0, 8.1, 184.0, 18.1);
            bubblePath.cubicTo(184.0, 28.1, 173.9, 36.1, 161.4, 36.1);
            bubblePath.cubicTo(156.9, 36.1, 152.8, 35.2, 149.1, 33.3);
            bubblePath.closeSubpath();
            painter->setBrush(ink);
            painter->drawPath(bubblePath);

            painter->setBrush(white);
            painter->drawEllipse(QRectF(167.6, 15.8, 5.6, 5.6));

            QPainterPath triangle;
            triangle.moveTo(149.7, 21.0);
            triangle.lineTo(155.4, 21.0);
            triangle.lineTo(152.6, 16.1);
            triangle.closeSubpath();
            painter->drawPath(triangle);
            painter->drawRect(QRectF(159.0, 16.1, 5.0, 5.0));
        }
        painter->restore();
    }

    QFont descFont = painter->font();
    descFont.setPixelSize(token.fontSize);
    descFont.setWeight(QFont::Normal);
    painter->setFont(descFont);
    painter->setPen(token.colorTextTertiary);
    painter->drawText(emptyDescriptionRect(empty, option->rect), Qt::AlignCenter | Qt::TextWordWrap, empty->description());

    painter->restore();
}
