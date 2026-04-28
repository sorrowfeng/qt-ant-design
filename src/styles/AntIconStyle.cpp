#include "AntIconStyle.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPathStroker>
#include <QStyleOption>
#include <QSvgRenderer>
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

struct OfficialSvgIcon
{
    const char* viewBox = "64 64 896 896";
    const char* paths[2] = {nullptr, nullptr};
};

const OfficialSvgIcon* officialOutlinedIcon(Ant::IconType type)
{
    static const OfficialSvgIcon home = {
        "64 64 896 896",
        {R"(M946.5 505L560.1 118.8l-25.9-25.9a31.5 31.5 0 00-44.4 0L77.5 505a63.9 63.9 0 00-18.8 46c.4 35.2 29.7 63.3 64.9 63.3h42.5V940h691.8V614.3h43.4c17.1 0 33.2-6.7 45.3-18.8a63.6 63.6 0 0018.7-45.3c0-17-6.7-33.1-18.8-45.2zM568 868H456V664h112v204zm217.9-325.7V868H632V640c0-22.1-17.9-40-40-40H432c-22.1 0-40 17.9-40 40v228H238.1V542.3h-96l370-369.7 23.1 23.1L882 542.3h-96.1z)", nullptr}
    };
    static const OfficialSvgIcon user = {
        "64 64 896 896",
        {R"(M858.5 763.6a374 374 0 00-80.6-119.5 375.63 375.63 0 00-119.5-80.6c-.4-.2-.8-.3-1.2-.5C719.5 518 760 444.7 760 362c0-137-111-248-248-248S264 225 264 362c0 82.7 40.5 156 102.8 201.1-.4.2-.8.3-1.2.5-44.8 18.9-85 46-119.5 80.6a375.63 375.63 0 00-80.6 119.5A371.7 371.7 0 00136 901.8a8 8 0 008 8.2h60c4.4 0 7.9-3.5 8-7.8 2-77.2 33-149.5 87.8-204.3 56.7-56.7 132-87.9 212.2-87.9s155.5 31.2 212.2 87.9C779 752.7 810 825 812 902.2c.1 4.4 3.6 7.8 8 7.8h60a8 8 0 008-8.2c-1-47.8-10.9-94.3-29.5-138.2zM512 534c-45.9 0-89.1-17.9-121.6-50.4S340 407.9 340 362c0-45.9 17.9-89.1 50.4-121.6S466.1 190 512 190s89.1 17.9 121.6 50.4S684 316.1 684 362c0 45.9-17.9 89.1-50.4 121.6S557.9 534 512 534z)", nullptr}
    };
    static const OfficialSvgIcon search = {
        "64 64 896 896",
        {R"(M909.6 854.5L649.9 594.8C690.2 542.7 712 479 712 412c0-80.2-31.3-155.4-87.9-212.1-56.6-56.7-132-87.9-212.1-87.9s-155.5 31.3-212.1 87.9C143.2 256.5 112 331.8 112 412c0 80.1 31.3 155.5 87.9 212.1C256.5 680.8 331.8 712 412 712c67 0 130.6-21.8 182.7-62l259.7 259.6a8.2 8.2 0 0011.6 0l43.6-43.5a8.2 8.2 0 000-11.6zM570.4 570.4C528 612.7 471.8 636 412 636s-116-23.3-158.4-65.6C211.3 528 188 471.8 188 412s23.3-116.1 65.6-158.4C296 211.3 352.2 188 412 188s116.1 23.2 158.4 65.6S636 352.2 636 412s-23.3 116.1-65.6 158.4z)", nullptr}
    };
    static const OfficialSvgIcon setting = {
        "64 64 896 896",
        {R"(M924.8 625.7l-65.5-56c3.1-19 4.7-38.4 4.7-57.8s-1.6-38.8-4.7-57.8l65.5-56a32.03 32.03 0 009.3-35.2l-.9-2.6a443.74 443.74 0 00-79.7-137.9l-1.8-2.1a32.12 32.12 0 00-35.1-9.5l-81.3 28.9c-30-24.6-63.5-44-99.7-57.6l-15.7-85a32.05 32.05 0 00-25.8-25.7l-2.7-.5c-52.1-9.4-106.9-9.4-159 0l-2.7.5a32.05 32.05 0 00-25.8 25.7l-15.8 85.4a351.86 351.86 0 00-99 57.4l-81.9-29.1a32 32 0 00-35.1 9.5l-1.8 2.1a446.02 446.02 0 00-79.7 137.9l-.9 2.6c-4.5 12.5-.8 26.5 9.3 35.2l66.3 56.6c-3.1 18.8-4.6 38-4.6 57.1 0 19.2 1.5 38.4 4.6 57.1L99 625.5a32.03 32.03 0 00-9.3 35.2l.9 2.6c18.1 50.4 44.9 96.9 79.7 137.9l1.8 2.1a32.12 32.12 0 0035.1 9.5l81.9-29.1c29.8 24.5 63.1 43.9 99 57.4l15.8 85.4a32.05 32.05 0 0025.8 25.7l2.7.5a449.4 449.4 0 00159 0l2.7-.5a32.05 32.05 0 0025.8-25.7l15.7-85a350 350 0 0099.7-57.6l81.3 28.9a32 32 0 0035.1-9.5l1.8-2.1c34.8-41.1 61.6-87.5 79.7-137.9l.9-2.6c4.5-12.3.8-26.3-9.3-35zM788.3 465.9c2.5 15.1 3.8 30.6 3.8 46.1s-1.3 31-3.8 46.1l-6.6 40.1 74.7 63.9a370.03 370.03 0 01-42.6 73.6L721 702.8l-31.4 25.8c-23.9 19.6-50.5 35-79.3 45.8l-38.1 14.3-17.9 97a377.5 377.5 0 01-85 0l-17.9-97.2-37.8-14.5c-28.5-10.8-55-26.2-78.7-45.7l-31.4-25.9-93.4 33.2c-17-22.9-31.2-47.6-42.6-73.6l75.5-64.5-6.5-40c-2.4-14.9-3.7-30.3-3.7-45.5 0-15.3 1.2-30.6 3.7-45.5l6.5-40-75.5-64.5c11.3-26.1 25.6-50.7 42.6-73.6l93.4 33.2 31.4-25.9c23.7-19.5 50.2-34.9 78.7-45.7l37.9-14.3 17.9-97.2c28.1-3.2 56.8-3.2 85 0l17.9 97 38.1 14.3c28.7 10.8 55.4 26.2 79.3 45.8l31.4 25.8 92.8-32.9c17 22.9 31.2 47.6 42.6 73.6L781.8 426l6.5 39.9zM512 326c-97.2 0-176 78.8-176 176s78.8 176 176 176 176-78.8 176-176-78.8-176-176-176zm79.2 255.2A111.6 111.6 0 01512 614c-29.9 0-58-11.7-79.2-32.8A111.6 111.6 0 01400 502c0-29.9 11.7-58 32.8-79.2C454 401.6 482.1 390 512 390c29.9 0 58 11.6 79.2 32.8A111.6 111.6 0 01624 502c0 29.9-11.7 58-32.8 79.2z)", nullptr}
    };
    static const OfficialSvgIcon star = {
        "64 64 896 896",
        {R"(M908.1 353.1l-253.9-36.9L540.7 86.1c-3.1-6.3-8.2-11.4-14.5-14.5-15.8-7.8-35-1.3-42.9 14.5L369.8 316.2l-253.9 36.9c-7 1-13.4 4.3-18.3 9.3a32.05 32.05 0 00.6 45.3l183.7 179.1-43.4 252.9a31.95 31.95 0 0046.4 33.7L512 754l227.1 119.4c6.2 3.3 13.4 4.4 20.3 3.2 17.4-3 29.1-19.5 26.1-36.9l-43.4-252.9 183.7-179.1c5-4.9 8.3-11.3 9.3-18.3 2.7-17.5-9.5-33.7-27-36.3zM664.8 561.6l36.1 210.3L512 672.7 323.1 772l36.1-210.3-152.8-149L417.6 382 512 190.7 606.4 382l211.2 30.7-152.8 148.9z)", nullptr}
    };
    static const OfficialSvgIcon heart = {
        "64 64 896 896",
        {R"(M923 283.6a260.04 260.04 0 00-56.9-82.8 264.4 264.4 0 00-84-55.5A265.34 265.34 0 00679.7 125c-49.3 0-97.4 13.5-139.2 39-10 6.1-19.5 12.8-28.5 20.1-9-7.3-18.5-14-28.5-20.1-41.8-25.5-89.9-39-139.2-39-35.5 0-69.9 6.8-102.4 20.3-31.4 13-59.7 31.7-84 55.5a258.44 258.44 0 00-56.9 82.8c-13.9 32.3-21 66.6-21 101.9 0 33.3 6.8 68 20.3 103.3 11.3 29.5 27.5 60.1 48.2 91 32.8 48.9 77.9 99.9 133.9 151.6 92.8 85.7 184.7 144.9 188.6 147.3l23.7 15.2c10.5 6.7 24 6.7 34.5 0l23.7-15.2c3.9-2.5 95.7-61.6 188.6-147.3 56-51.7 101.1-102.7 133.9-151.6 20.7-30.9 37-61.5 48.2-91 13.5-35.3 20.3-70 20.3-103.3.1-35.3-7-69.6-20.9-101.9zM512 814.8S156 586.7 156 385.5C156 283.6 240.3 201 344.3 201c73.1 0 136.5 40.8 167.7 100.4C543.2 241.8 606.6 201 679.7 201c104 0 188.3 82.6 188.3 184.5 0 201.2-356 429.3-356 429.3z)", nullptr}
    };
    static const OfficialSvgIcon bell = {
        "64 64 896 896",
        {R"(M816 768h-24V428c0-141.1-104.3-257.7-240-277.1V112c0-22.1-17.9-40-40-40s-40 17.9-40 40v38.9c-135.7 19.4-240 136-240 277.1v340h-24c-17.7 0-32 14.3-32 32v32c0 4.4 3.6 8 8 8h216c0 61.8 50.2 112 112 112s112-50.2 112-112h216c4.4 0 8-3.6 8-8v-32c0-17.7-14.3-32-32-32zM512 888c-26.5 0-48-21.5-48-48h96c0 26.5-21.5 48-48 48zM304 768V428c0-55.6 21.6-107.8 60.9-147.1S456.4 220 512 220c55.6 0 107.8 21.6 147.1 60.9S720 372.4 720 428v340H304z)", nullptr}
    };
    static const OfficialSvgIcon mail = {
        "64 64 896 896",
        {R"(M928 160H96c-17.7 0-32 14.3-32 32v640c0 17.7 14.3 32 32 32h832c17.7 0 32-14.3 32-32V192c0-17.7-14.3-32-32-32zm-40 110.8V792H136V270.8l-27.6-21.5 39.3-50.5 42.8 33.3h643.1l42.8-33.3 39.3 50.5-27.7 21.5zM833.6 232L512 482 190.4 232l-42.8-33.3-39.3 50.5 27.6 21.5 341.6 265.6a55.99 55.99 0 0068.7 0L888 270.8l27.6-21.5-39.3-50.5-42.7 33.2z)", nullptr}
    };
    static const OfficialSvgIcon calendar = {
        "64 64 896 896",
        {R"(M880 184H712v-64c0-4.4-3.6-8-8-8h-56c-4.4 0-8 3.6-8 8v64H384v-64c0-4.4-3.6-8-8-8h-56c-4.4 0-8 3.6-8 8v64H144c-17.7 0-32 14.3-32 32v664c0 17.7 14.3 32 32 32h736c17.7 0 32-14.3 32-32V216c0-17.7-14.3-32-32-32zm-40 656H184V460h656v380zM184 392V256h128v48c0 4.4 3.6 8 8 8h56c4.4 0 8-3.6 8-8v-48h256v48c0 4.4 3.6 8 8 8h56c4.4 0 8-3.6 8-8v-48h128v136H184z)", nullptr}
    };
    static const OfficialSvgIcon clock = {
        "64 64 896 896",
        {R"(M512 64C264.6 64 64 264.6 64 512s200.6 448 448 448 448-200.6 448-448S759.4 64 512 64zm0 820c-205.4 0-372-166.6-372-372s166.6-372 372-372 372 166.6 372 372-166.6 372-372 372z)",
         R"(M686.7 638.6L544.1 535.5V288c0-4.4-3.6-8-8-8H488c-4.4 0-8 3.6-8 8v275.4c0 2.6 1.2 5 3.3 6.5l165.4 120.6c3.6 2.6 8.6 1.8 11.2-1.7l28.6-39c2.6-3.7 1.8-8.7-1.8-11.2z)"}
    };
    static const OfficialSvgIcon check = {
        "64 64 896 896",
        {R"(M912 190h-69.9c-9.8 0-19.1 4.5-25.1 12.2L404.7 724.5 207 474a32 32 0 00-25.1-12.2H112c-6.7 0-10.4 7.7-6.3 12.9l273.9 347c12.8 16.2 37.4 16.2 50.3 0l488.4-618.9c4.1-5.1.4-12.8-6.3-12.8z)", nullptr}
    };
    static const OfficialSvgIcon close = {
        "64 64 896 896",
        {R"(M799.86 166.31c.02 0 .04.02.08.06l57.69 57.7c.04.03.05.05.06.08a.12.12 0 010 .06c0 .03-.02.05-.06.09L569.93 512l287.7 287.7c.04.04.05.06.06.09a.12.12 0 010 .07c0 .02-.02.04-.06.08l-57.7 57.69c-.03.04-.05.05-.07.06a.12.12 0 01-.07 0c-.03 0-.05-.02-.09-.06L512 569.93l-287.7 287.7c-.04.04-.06.05-.09.06a.12.12 0 01-.07 0c-.02 0-.04-.02-.08-.06l-57.69-57.7c-.04-.03-.05-.05-.06-.07a.12.12 0 010-.07c0-.03.02-.05.06-.09L454.07 512l-287.7-287.7c-.04-.04-.05-.06-.06-.09a.12.12 0 010-.07c0-.02.02-.04.06-.08l57.7-57.69c.03-.04.05-.05.07-.06a.12.12 0 01.07 0c.03 0 .05.02.09.06L512 454.07l287.7-287.7c.04-.04.06-.05.09-.06a.12.12 0 01.07 0z)", nullptr}
    };
    static const OfficialSvgIcon plus = {
        "64 64 896 896",
        {R"(M482 152h60q8 0 8 8v704q0 8-8 8h-60q-8 0-8-8V160q0-8 8-8z)",
         R"(M192 474h672q8 0 8 8v60q0 8-8 8H160q-8 0-8-8v-60q0-8 8-8z)"}
    };
    static const OfficialSvgIcon edit = {
        "64 64 896 896",
        {R"(M257.7 752c2 0 4-.2 6-.5L431.9 722c2-.4 3.9-1.3 5.3-2.8l423.9-423.9a9.96 9.96 0 000-14.1L694.9 114.9c-1.9-1.9-4.4-2.9-7.1-2.9s-5.2 1-7.1 2.9L256.8 538.8c-1.5 1.5-2.4 3.3-2.8 5.3l-29.5 168.2a33.5 33.5 0 009.4 29.8c6.6 6.4 14.9 9.9 23.8 9.9zm67.4-174.4L687.8 215l73.3 73.3-362.7 362.6-88.9 15.7 15.6-89zM880 836H144c-17.7 0-32 14.3-32 32v36c0 4.4 3.6 8 8 8h784c4.4 0 8-3.6 8-8v-36c0-17.7-14.3-32-32-32z)", nullptr}
    };
    static const OfficialSvgIcon remove = {
        "64 64 896 896",
        {R"(M360 184h-8c4.4 0 8-3.6 8-8v8h304v-8c0 4.4 3.6 8 8 8h-8v72h72v-80c0-35.3-28.7-64-64-64H352c-35.3 0-64 28.7-64 64v80h72v-72zm504 72H160c-17.7 0-32 14.3-32 32v32c0 4.4 3.6 8 8 8h60.4l24.7 523c1.6 34.1 29.8 61 63.9 61h454c34.2 0 62.3-26.8 63.9-61l24.7-523H888c4.4 0 8-3.6 8-8v-32c0-17.7-14.3-32-32-32zM731.3 840H292.7l-24.2-512h487l-24.2 512z)", nullptr}
    };
    static const OfficialSvgIcon upload = {
        "64 64 896 896",
        {R"(M518.3 459a8 8 0 00-12.6 0l-112 141.7a7.98 7.98 0 006.3 12.9h73.9V856c0 4.4 3.6 8 8 8h60c4.4 0 8-3.6 8-8V613.7H624c6.7 0 10.4-7.7 6.3-12.9L518.3 459z)",
         R"(M811.4 366.7C765.6 245.9 648.9 160 512.2 160S258.8 245.8 213 366.6C127.3 389.1 64 467.2 64 560c0 110.5 89.5 200 199.9 200H304c4.4 0 8-3.6 8-8v-60c0-4.4-3.6-8-8-8h-40.1c-33.7 0-65.4-13.4-89-37.7-23.5-24.2-36-56.8-34.9-90.6.9-26.4 9.9-51.2 26.2-72.1 16.7-21.3 40.1-36.8 66.1-43.7l37.9-9.9 13.9-36.6c8.6-22.8 20.6-44.1 35.7-63.4a245.6 245.6 0 0152.4-49.9c41.1-28.9 89.5-44.2 140-44.2s98.9 15.3 140 44.2c19.9 14 37.5 30.8 52.4 49.9 15.1 19.3 27.1 40.7 35.7 63.4l13.8 36.5 37.8 10C846.1 454.5 884 503.8 884 560c0 33.1-12.9 64.3-36.3 87.7a123.07 123.07 0 01-87.6 36.3H720c-4.4 0-8 3.6-8 8v60c0 4.4 3.6 8 8 8h40.1C870.5 760 960 670.5 960 560c0-92.7-63.1-170.7-148.6-193.3z)"}
    };

    switch (type)
    {
    case Ant::IconType::Home: return &home;
    case Ant::IconType::User: return &user;
    case Ant::IconType::Search: return &search;
    case Ant::IconType::Setting: return &setting;
    case Ant::IconType::Star: return &star;
    case Ant::IconType::Heart: return &heart;
    case Ant::IconType::Bell: return &bell;
    case Ant::IconType::Mail: return &mail;
    case Ant::IconType::Calendar: return &calendar;
    case Ant::IconType::ClockCircle: return &clock;
    case Ant::IconType::Check: return &check;
    case Ant::IconType::Close: return &close;
    case Ant::IconType::Plus: return &plus;
    case Ant::IconType::Edit: return &edit;
    case Ant::IconType::Delete: return &remove;
    case Ant::IconType::CloudUpload: return &upload;
    default: return nullptr;
    }
}

bool drawOfficialOutlinedIcon(Ant::IconType type, const QRectF& iconRect, const QColor& color, QPainter* painter)
{
    const auto* data = officialOutlinedIcon(type);
    if (!data || !painter)
    {
        return false;
    }

    QString svg = QStringLiteral("<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"%1\">").arg(QString::fromLatin1(data->viewBox));
    const QString fill = color.name(QColor::HexRgb);
    for (const char* pathData : data->paths)
    {
        if (!pathData)
        {
            continue;
        }
        svg += QStringLiteral("<path fill=\"");
        svg += fill;
        svg += QStringLiteral("\" d=\"");
        svg += QString::fromLatin1(pathData);
        svg += QStringLiteral("\"/>");
    }
    svg += QStringLiteral("</svg>");

    QSvgRenderer renderer(svg.toUtf8());
    if (!renderer.isValid())
    {
        return false;
    }
    renderer.render(painter, iconRect);
    return true;
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

    AntIcon::IconPaths paths;
    if (icon->hasCustomPath())
    {
        paths.primary = icon->customPrimaryPath();
        paths.secondary = icon->customSecondaryPath();
        paths.useStroke = iconTheme == Ant::IconTheme::Outlined;
    }
    else
    {
        paths = AntIcon::builtinPaths(iconType, iconTheme);
    }

    if (paths.primary.isEmpty() && paths.secondary.isEmpty())
    {
        painter->restore();
        return;
    }

    painter->translate(widget->rect().center());
    painter->rotate(icon->rotate() + (icon->isSpin() ? icon->spinAngle() : 0));
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

    if (!icon->hasCustomPath() && iconTheme == Ant::IconTheme::Outlined &&
        drawOfficialOutlinedIcon(iconType, iconRect, primaryColor, painter))
    {
        painter->restore();
        return;
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

    const QPainterPath primaryPath = AntIcon::transformPath(paths.primary, iconRect);
    const QPainterPath secondaryPath = AntIcon::transformPath(paths.secondary, iconRect);

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
        painter->setBrush(primaryColor);
        painter->drawPath(primaryPath);
        if (!secondaryPath.isEmpty())
        {
            painter->setBrush(secondaryColor);
            painter->drawPath(secondaryPath);
        }
    }

    painter->restore();
}
