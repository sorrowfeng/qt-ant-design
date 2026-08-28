#include "AntStyleBase.h"

#include <QEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QStyleFactory>
#include <QStringList>
#include <QtMath>

#include "styles/AntPalette.h"

namespace
{
QStyle* createDetachedBaseStyle(QStyle* sourceStyle)
{
    auto createByName = [](const QString& name) -> QStyle* {
        return name.isEmpty() ? nullptr : QStyleFactory::create(name);
    };

    if (sourceStyle)
    {
        if (auto* style = createByName(sourceStyle->objectName()))
            return style;

        if (auto* sourceProxy = qobject_cast<QProxyStyle*>(sourceStyle))
        {
            if (QStyle* baseStyle = sourceProxy->baseStyle())
            {
                if (auto* style = createByName(baseStyle->objectName()))
                    return style;
            }
        }
    }

    if (QApplication::instance())
    {
        if (QStyle* appStyle = QApplication::style())
        {
            if (auto* style = createByName(appStyle->objectName()))
                return style;
        }
    }

    if (auto* style = QStyleFactory::create(QStringLiteral("Fusion")))
        return style;

    const QStringList keys = QStyleFactory::keys();
    return keys.isEmpty() ? nullptr : QStyleFactory::create(keys.constFirst());
}
}

AntStyleBase::AntStyleBase(QStyle* style)
    // QProxyStyle owns its base style, so never pass a shared QApplication/widget style directly.
    : QProxyStyle(createDetachedBaseStyle(style))
{
}

QFont AntStyleBase::withPixelSize(const QFont& base, int pixelSize)
{
    QFont font = base;
    font.setPixelSize(pixelSize);
    return font;
}

QFont AntStyleBase::withPixelSize(const QFont& base, int pixelSize, QFont::Weight weight)
{
    QFont font = base;
    font.setPixelSize(pixelSize);
    font.setWeight(weight);
    return font;
}

void AntStyleBase::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    registerThemeWidget(widget);
}

void AntStyleBase::unpolish(QWidget* widget)
{
    unregisterThemeWidget(widget);
    QProxyStyle::unpolish(widget);
}

void AntStyleBase::drawCrispRoundedRect(QPainter* painter, const QRect& rect,
    const QPen& pen, const QBrush& brush, qreal rx, qreal ry)
{
    painter->setPen(pen);
    painter->setBrush(brush);
    if (pen.style() != Qt::NoPen && pen.widthF() > 0)
        painter->drawRoundedRect(QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5), rx, ry);
    else
        painter->drawRoundedRect(QRectF(rect), rx, ry);
}

void AntStyleBase::drawInputFocusGlow(QPainter* painter, const QRectF& frameRect,
    qreal radius, const QColor& glowColor, qreal outlineWidth)
{
    if (!painter || outlineWidth <= 0 || frameRect.isEmpty())
    {
        return;
    }
    drawCrispRoundedRect(painter, frameRect.adjusted(-1, -1, 1, 1).toRect(),
        QPen(glowColor, outlineWidth), Qt::NoBrush, radius + 1, radius + 1);
}

int AntStyleBase::focusPaddingFor()
{
    return antTheme->tokens().lineWidthFocus + 1;
}

void AntStyleBase::drawButtonBottomShadow(QPainter* painter, const QRectF& outer,
    int radius, const QColor& color)
{
    if (!painter || color.alpha() == 0)
    {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawRoundedRect(outer.adjusted(0, 2, 0, 2), radius, radius);
    painter->restore();
}

void AntStyleBase::drawButtonFocusOutline(QPainter* painter, const QRectF& bodyRect,
    int radius)
{
    const auto& token = antTheme->tokens();
    const qreal offset = 1.0;
    const qreal width = token.lineWidthFocus;
    const qreal expand = offset + width / 2.0;
    const QRectF focusRect = bodyRect.adjusted(-expand, -expand, expand, expand);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(token.colorPrimaryBorder, width, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(focusRect, radius + expand, radius + expand);
    painter->restore();
}

void AntStyleBase::drawSpinner(QPainter* painter, const QRectF& rect,
    const QColor& color, int angle, int spanAngle, qreal penWidth)
{
    if (!painter || rect.isEmpty())
    {
        return;
    }
    if (penWidth <= 0)
    {
        penWidth = qMax<qreal>(1.5, rect.width() * 0.12);
    }
    const QRectF arcRect = rect.adjusted(penWidth / 2.0, penWidth / 2.0,
        -penWidth / 2.0, -penWidth / 2.0);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->translate(arcRect.center());
    painter->rotate(angle);
    painter->translate(-arcRect.center());
    painter->setPen(QPen(color, penWidth, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(Qt::NoBrush);
    // Start at 12 o'clock (90deg) when angle == 0, sweep clockwise.
    painter->drawArc(arcRect, 90 * 16, -spanAngle * 16);
    painter->restore();
}

void AntStyleBase::drawEmptyIllustration(QPainter* painter, const QRectF& targetRect,
    bool simple, bool extraLine)
{
    if (!painter || targetRect.isEmpty())
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const bool isDark = antTheme->themeMode() == Ant::ThemeMode::Dark;
    const QColor primary = AntPalette::alpha(token.colorTextTertiary, isDark ? 0.5 : 0.32);
    const QColor fill = AntPalette::alpha(token.colorFillQuaternary, isDark ? 0.78 : 1.0);
    const QColor line = AntPalette::alpha(token.colorTextTertiary, isDark ? 0.68 : 0.45);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter->translate(targetRect.topLeft());

    if (simple)
    {
        painter->scale(targetRect.width() / 128.0, targetRect.height() / 80.0);

        painter->setPen(Qt::NoPen);
        painter->setBrush(AntPalette::alpha(token.colorPrimary, 0.12));
        painter->drawEllipse(QRectF(16, 58, 96, 14));

        AntStyleBase::drawCrispRoundedRect(painter, QRect(34, 10, 60, 46), Qt::NoPen, fill, token.borderRadiusLG + 2, token.borderRadiusLG + 2);
        AntStyleBase::drawCrispRoundedRect(painter, QRect(42, 18, 44, 30), Qt::NoPen,
            AntPalette::alpha(token.colorBgContainer, 0.88), token.borderRadius, token.borderRadius);

        painter->setPen(QPen(primary, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawArc(QRectF(10, 20, 26, 26), 35 * 16, 260 * 16);
        painter->drawArc(QRectF(92, 18, 22, 22), 220 * 16, 220 * 16);

        painter->setPen(QPen(line, 2.2, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(49, 27), QPointF(79, 27));
        painter->drawLine(QPointF(49, 34), QPointF(73, 34));
        if (extraLine)
        {
            painter->drawLine(QPointF(49, 41), QPointF(67, 41));
        }
    }
    else
    {
        painter->scale(targetRect.width() / 184.0, targetRect.height() / 152.0);

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

bool AntStyleBase::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Paint)
    {
        auto* widget = qobject_cast<QWidget*>(watched);
        if (widget && widget->style() == this)
        {
            auto* paintEvent = static_cast<QPaintEvent*>(event);
            if (drawWidget(widget, paintEvent))
            {
                return true;
            }
        }
    }
    return QProxyStyle::eventFilter(watched, event);
}

bool AntStyleBase::drawWidget(QWidget* /*widget*/, QPaintEvent* /*event*/)
{
    return false;
}

void AntStyleBase::updateThemeTarget(QWidget* widget)
{
    if (!widget)
    {
        return;
    }

    widget->setProperty("antStyleThemeSizeHintBefore", widget->property("antStyleThemeCachedSizeHint"));
    widget->setProperty("antStyleThemeMinimumSizeHintBefore",
                        widget->property("antStyleThemeCachedMinimumSizeHint"));
    unpolish(widget);
    polish(widget);
    onThemeUpdate(widget);
}

void AntStyleBase::cacheThemeGeometryHints(QWidget* widget)
{
    if (!widget)
    {
        return;
    }

    widget->setProperty("antStyleThemeCachedSizeHint", widget->sizeHint());
    widget->setProperty("antStyleThemeCachedMinimumSizeHint", widget->minimumSizeHint());
}

void AntStyleBase::pruneThemeWidgets()
{
    for (int i = m_themeWidgets.size() - 1; i >= 0; --i)
    {
        if (m_themeWidgets.at(i).isNull())
        {
            m_themeWidgets.removeAt(i);
        }
    }
}

void AntStyleBase::registerThemeWidget(QWidget* widget)
{
    if (!widget)
    {
        return;
    }

    pruneThemeWidgets();
    for (const QPointer<QWidget>& watched : m_themeWidgets)
    {
        if (watched.data() == widget)
        {
            return;
        }
    }
    m_themeWidgets.append(QPointer<QWidget>(widget));
}

void AntStyleBase::unregisterThemeWidget(QWidget* widget)
{
    for (int i = m_themeWidgets.size() - 1; i >= 0; --i)
    {
        QWidget* watched = m_themeWidgets.at(i).data();
        if (!watched || watched == widget)
        {
            m_themeWidgets.removeAt(i);
        }
    }
}
