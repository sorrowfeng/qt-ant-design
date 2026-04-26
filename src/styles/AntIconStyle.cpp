#include "AntIconStyle.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPathStroker>
#include <QStyleOption>
#include <QTransform>

#include "styles/AntPalette.h"
#include "widgets/AntIcon.h"

namespace
{
QPainterPath createCirclePath(qreal x, qreal y, qreal w, qreal h)
{
    QPainterPath path;
    path.addEllipse(QRectF(x, y, w, h));
    return path;
}

QPainterPath createRoundedRectPath(qreal x, qreal y, qreal w, qreal h, qreal radius)
{
    QPainterPath path;
    path.addRoundedRect(QRectF(x, y, w, h), radius, radius);
    return path;
}

QPainterPath createLinePath(const QPointF& start, const QPointF& end)
{
    QPainterPath path;
    path.moveTo(start);
    path.lineTo(end);
    return path;
}

QPainterPath createPolylinePath(const QVector<QPointF>& points)
{
    QPainterPath path;
    if (points.isEmpty())
    {
        return path;
    }
    path.moveTo(points.first());
    for (int i = 1; i < points.size(); ++i)
    {
        path.lineTo(points.at(i));
    }
    return path;
}

QPainterPath createSearchLensPath()
{
    QPainterPath path;
    path.addEllipse(QRectF(6, 6, 16, 16));
    path.moveTo(20, 20);
    path.lineTo(27, 27);
    return path;
}

QPainterPath createCrossPath()
{
    QPainterPath path;
    path.moveTo(9, 9);
    path.lineTo(23, 23);
    path.moveTo(23, 9);
    path.lineTo(9, 23);
    return path;
}

QPainterPath createCheckPath()
{
    return createPolylinePath({QPointF(7, 17), QPointF(13, 23), QPointF(25, 10)});
}

QPainterPath createChevronPath(Ant::IconType type)
{
    switch (type)
    {
    case Ant::IconType::Down:
        return createPolylinePath({QPointF(8, 12), QPointF(16, 20), QPointF(24, 12)});
    case Ant::IconType::Up:
        return createPolylinePath({QPointF(8, 20), QPointF(16, 12), QPointF(24, 20)});
    case Ant::IconType::Left:
        return createPolylinePath({QPointF(20, 8), QPointF(12, 16), QPointF(20, 24)});
    case Ant::IconType::Right:
        return createPolylinePath({QPointF(12, 8), QPointF(20, 16), QPointF(12, 24)});
    default:
        return QPainterPath();
    }
}

QPainterPath createStarPath()
{
    QPainterPath path;
    path.moveTo(16, 4);
    path.lineTo(19.53, 11.15);
    path.lineTo(27.42, 12.29);
    path.lineTo(21.71, 17.85);
    path.lineTo(23.06, 25.71);
    path.lineTo(16, 22);
    path.lineTo(8.94, 25.71);
    path.lineTo(10.29, 17.85);
    path.lineTo(4.58, 12.29);
    path.lineTo(12.47, 11.15);
    path.closeSubpath();
    return path;
}

QPainterPath createHomePath()
{
    QPainterPath path;
    path.moveTo(6, 15);
    path.lineTo(16, 6);
    path.lineTo(26, 15);
    path.lineTo(26, 26);
    path.lineTo(19, 26);
    path.lineTo(19, 19);
    path.lineTo(13, 19);
    path.lineTo(13, 26);
    path.lineTo(6, 26);
    path.closeSubpath();
    return path;
}

QPainterPath createUserPath()
{
    QPainterPath path;
    path.addEllipse(QRectF(10, 5, 12, 12));
    path.moveTo(6, 27);
    path.cubicTo(7.5, 22.5, 11, 20, 16, 20);
    path.cubicTo(21, 20, 24.5, 22.5, 26, 27);
    return path;
}

QPainterPath createCalendarOutlinePath()
{
    QPainterPath path;
    path.addRoundedRect(QRectF(5, 7, 22, 20), 2, 2);
    path.moveTo(10, 4);
    path.lineTo(10, 10);
    path.moveTo(22, 4);
    path.lineTo(22, 10);
    path.moveTo(5, 13);
    path.lineTo(27, 13);
    path.moveTo(10, 17);
    path.lineTo(10, 17.1);
    path.moveTo(16, 17);
    path.lineTo(16, 17.1);
    path.moveTo(22, 17);
    path.lineTo(22, 17.1);
    path.moveTo(10, 22);
    path.lineTo(10, 22.1);
    path.moveTo(16, 22);
    path.lineTo(16, 22.1);
    return path;
}

QPainterPath createClockOutlinePath()
{
    QPainterPath path;
    path.addEllipse(QRectF(5, 5, 22, 22));
    path.moveTo(16, 10);
    path.lineTo(16, 16);
    path.lineTo(20.5, 18.5);
    return path;
}

QPainterPath createInfoCirclePath(bool warning)
{
    QPainterPath path;
    path.addEllipse(QRectF(4.5, 4.5, 23, 23));
    if (warning)
    {
        path.moveTo(16, 10);
        path.lineTo(16, 18);
        path.moveTo(16, 22);
        path.lineTo(16, 22.1);
    }
    else
    {
        path.moveTo(16, 13);
        path.lineTo(16, 21);
        path.moveTo(16, 9.5);
        path.lineTo(16, 9.6);
    }
    return path;
}

QPainterPath createMinusPath()
{
    return createLinePath(QPointF(8, 16), QPointF(24, 16));
}

QPainterPath createPlusPath()
{
    QPainterPath path = createMinusPath();
    path.moveTo(16, 8);
    path.lineTo(16, 24);
    return path;
}

QPainterPath createLoadingPath()
{
    QPainterPath path;
    path.arcMoveTo(QRectF(5, 5, 22, 22), 35);
    path.arcTo(QRectF(5, 5, 22, 22), 35, 290);
    return path;
}

struct IconPaths
{
    QPainterPath primary;
    QPainterPath secondary;
    bool useStroke = false;
};

IconPaths builtinPaths(Ant::IconType type, Ant::IconTheme theme)
{
    IconPaths result;
    result.useStroke = theme == Ant::IconTheme::Outlined;
    QPainterPathStroker stroker;
    stroker.setWidth(2.8);
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);

    switch (type)
    {
    case Ant::IconType::Search:
        result.primary = createSearchLensPath();
        break;
    case Ant::IconType::Close:
        result.primary = createCrossPath();
        break;
    case Ant::IconType::Plus:
        result.primary = createPlusPath();
        break;
    case Ant::IconType::Minus:
        result.primary = createMinusPath();
        break;
    case Ant::IconType::Check:
        result.primary = createCheckPath();
        break;
    case Ant::IconType::Loading:
        result.primary = createLoadingPath();
        result.useStroke = true;
        break;
    case Ant::IconType::Down:
    case Ant::IconType::Up:
    case Ant::IconType::Left:
    case Ant::IconType::Right:
        result.primary = createChevronPath(type);
        break;
    case Ant::IconType::Home:
        result.primary = createHomePath();
        break;
    case Ant::IconType::User:
        result.primary = createUserPath();
        break;
    case Ant::IconType::Star:
        result.primary = createStarPath();
        result.useStroke = theme == Ant::IconTheme::Outlined;
        break;
    case Ant::IconType::Calendar:
        if (theme == Ant::IconTheme::Outlined)
        {
            result.primary = createCalendarOutlinePath();
            result.useStroke = true;
        }
        else
        {
            result.primary = createRoundedRectPath(5, 7, 22, 20, 2);
            result.secondary = createRoundedRectPath(8, 15, 16, 9, 1);
        }
        break;
    case Ant::IconType::ClockCircle:
        if (theme == Ant::IconTheme::Outlined)
        {
            result.primary = createClockOutlinePath();
            result.useStroke = true;
        }
        else
        {
            result.primary = createCirclePath(4.5, 4.5, 23, 23);
            QPainterPath hand;
            hand.moveTo(16, 10);
            hand.lineTo(16, 16);
            hand.lineTo(20.5, 18.5);
            result.secondary = stroker.createStroke(hand);
            result.useStroke = false;
        }
        break;
    case Ant::IconType::InfoCircle:
    case Ant::IconType::ExclamationCircle:
    case Ant::IconType::CloseCircle:
    case Ant::IconType::CheckCircle:
        if (theme == Ant::IconTheme::Outlined)
        {
            if (type == Ant::IconType::InfoCircle)
            {
                result.primary = createInfoCirclePath(false);
            }
            else if (type == Ant::IconType::ExclamationCircle)
            {
                result.primary = createInfoCirclePath(true);
            }
            else
            {
                result.primary = createCirclePath(4.5, 4.5, 23, 23);
                result.secondary = type == Ant::IconType::CloseCircle ? createCrossPath() : createCheckPath();
            }
            result.useStroke = true;
        }
        else
        {
            result.primary = createCirclePath(4.5, 4.5, 23, 23);
            if (type == Ant::IconType::InfoCircle)
            {
                QPainterPath symbol;
                symbol.addRect(QRectF(15, 13, 2, 8));
                symbol.addEllipse(QRectF(15, 9.2, 2, 2));
                result.secondary = symbol;
            }
            else if (type == Ant::IconType::ExclamationCircle)
            {
                QPainterPath symbol;
                symbol.addRect(QRectF(15, 10, 2, 8));
                symbol.addEllipse(QRectF(15, 21, 2, 2));
                result.secondary = symbol;
            }
            else if (type == Ant::IconType::CloseCircle)
            {
                QPainterPath symbol;
                symbol.addPolygon(QPolygonF({QPointF(11, 10), QPointF(16, 15), QPointF(21, 10),
                                             QPointF(22, 11), QPointF(17, 16), QPointF(22, 21),
                                             QPointF(21, 22), QPointF(16, 17), QPointF(11, 22),
                                             QPointF(10, 21), QPointF(15, 16), QPointF(10, 11)}));
                result.secondary = symbol;
            }
            else
            {
                QPainterPath symbol;
                symbol.moveTo(10, 16);
                symbol.lineTo(14, 20);
                symbol.lineTo(22, 12);
                result.secondary = stroker.createStroke(symbol);
            }
        }
        break;
    case Ant::IconType::None:
    default:
        break;
    }

    if (theme == Ant::IconTheme::Filled)
    {
        result.useStroke = false;
    }
    if (theme == Ant::IconTheme::TwoTone)
    {
        result.useStroke = false;
    }
    if (!result.useStroke)
    {
        switch (type)
        {
        case Ant::IconType::Search:
        case Ant::IconType::Close:
        case Ant::IconType::Plus:
        case Ant::IconType::Minus:
        case Ant::IconType::Check:
        case Ant::IconType::Loading:
        case Ant::IconType::Down:
        case Ant::IconType::Up:
        case Ant::IconType::Left:
        case Ant::IconType::Right:
            result.primary = stroker.createStroke(result.primary);
            break;
        default:
            break;
        }
    }
    return result;
}

QPainterPath transformPath(const QPainterPath& path, const QRectF& targetRect)
{
    if (path.isEmpty())
    {
        return path;
    }
    QTransform transform;
    transform.translate(targetRect.x(), targetRect.y());
    transform.scale(targetRect.width() / 32.0, targetRect.height() / 32.0);
    return transform.map(path);
}
} // namespace

AntIconStyle::AntIconStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntIcon>();
}

void AntIconStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntIcon*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntIconStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntIcon*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntIconStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntIcon*>(widget))
    {
        drawIcon(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntIconStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntIconStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* icon = qobject_cast<AntIcon*>(watched);
    if (icon && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(icon);
        option.rect = icon->rect();
        QPainter painter(icon);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, icon);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntIconStyle::drawIcon(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* icon = qobject_cast<const AntIcon*>(widget);
    if (!icon || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const QRectF iconRect = widget->rect().adjusted(1.0, 1.0, -1.0, -1.0);
    if (iconRect.width() <= 0 || iconRect.height() <= 0)
    {
        painter->restore();
        return;
    }

    const Ant::IconType iconType = icon->iconType();
    const Ant::IconTheme iconTheme = icon->iconTheme();
    const bool enabled = icon->isEnabled();

    IconPaths paths = builtinPaths(iconType, iconTheme);

    if (paths.primary.isEmpty() && paths.secondary.isEmpty())
    {
        painter->restore();
        return;
    }

    painter->translate(widget->rect().center());
    painter->rotate(icon->rotate());
    painter->translate(-widget->rect().center());

    QColor primaryColor = icon->color().isValid() ? icon->color() : token.colorText;
    if (!enabled)
    {
        primaryColor = token.colorTextDisabled;
    }
    if (iconTheme == Ant::IconTheme::TwoTone)
    {
        primaryColor = icon->twoToneColor().isValid() ? icon->twoToneColor() : token.colorPrimary;
    }

    QColor secondaryColor;
    if (!enabled)
    {
        secondaryColor = token.colorBgContainerDisabled;
    }
    else if (iconTheme == Ant::IconTheme::TwoTone)
    {
        const QColor tone = icon->twoToneColor().isValid() ? icon->twoToneColor() : token.colorPrimary;
        secondaryColor = AntPalette::tint(tone, 0.72);
    }
    else if (iconTheme == Ant::IconTheme::Filled)
    {
        secondaryColor = token.colorTextLightSolid;
    }
    else
    {
        secondaryColor = icon->color().isValid() ? icon->color() : token.colorText;
    }

    const QPainterPath primaryPath = transformPath(paths.primary, iconRect);
    const QPainterPath secondaryPath = transformPath(paths.secondary, iconRect);

    if (paths.useStroke)
    {
        QPen pen(primaryColor, qMax<qreal>(1.6, iconRect.width() * 0.1), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(primaryPath);
        if (!secondaryPath.isEmpty())
        {
            QPen secondaryPen(secondaryColor, pen.widthF(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->setPen(secondaryPen);
            painter->drawPath(secondaryPath);
        }
    }
    else
    {
        painter->setPen(Qt::NoPen);
        if (!secondaryPath.isEmpty())
        {
            painter->setBrush(secondaryColor);
            painter->drawPath(secondaryPath);
        }
        painter->setBrush(primaryColor);
        painter->drawPath(primaryPath);
    }

    painter->restore();
}
