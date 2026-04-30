#pragma once

#include <QPainter>
#include <QPainterPath>
#include <QRectF>
#include <QString>

#include "core/AntTypes.h"
#include "widgets/AntIcon.h"

namespace AntIconPainter
{
inline bool drawIcon(QPainter& painter,
                     Ant::IconType iconType,
                     const QRectF& iconRect,
                     const QColor& primaryColor,
                     Ant::IconTheme iconTheme = Ant::IconTheme::Outlined,
                     const QColor& secondaryColor = QColor())
{
    if (iconType == Ant::IconType::None || iconRect.isEmpty())
    {
        return false;
    }

    const AntIcon::IconPaths paths = AntIcon::builtinPaths(iconType, iconTheme);
    if (paths.primary.isEmpty() && paths.secondary.isEmpty())
    {
        return false;
    }

    const QPainterPath primaryPath = AntIcon::transformPath(paths.primary, iconRect);
    const QPainterPath secondaryPath = paths.secondary.isEmpty()
        ? QPainterPath()
        : AntIcon::transformPath(paths.secondary, iconRect);
    const QColor resolvedSecondary = secondaryColor.isValid() ? secondaryColor : primaryColor;

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    if (paths.useStroke)
    {
        const qreal width = qMax<qreal>(1.4, iconRect.width() * 0.1);
        painter.setPen(QPen(primaryColor, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(primaryPath);
        if (!secondaryPath.isEmpty())
        {
            painter.setPen(QPen(resolvedSecondary, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawPath(secondaryPath);
        }
    }
    else
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(primaryColor);
        painter.drawPath(primaryPath);
        if (!secondaryPath.isEmpty())
        {
            painter.setBrush(resolvedSecondary);
            painter.drawPath(secondaryPath);
        }
    }
    painter.restore();
    return true;
}

inline Ant::IconType iconTypeForKey(const QString& icon)
{
    const QString key = icon.trimmed().toLower();
    if (key == QStringLiteral("home")) return Ant::IconType::Home;
    if (key == QStringLiteral("search")) return Ant::IconType::Search;
    if (key == QStringLiteral("close")) return Ant::IconType::Close;
    if (key == QStringLiteral("plus")) return Ant::IconType::Plus;
    if (key == QStringLiteral("minus")) return Ant::IconType::Minus;
    if (key == QStringLiteral("check")) return Ant::IconType::Check;
    if (key == QStringLiteral("mail")) return Ant::IconType::Mail;
    if (key == QStringLiteral("bell")) return Ant::IconType::Bell;
    if (key == QStringLiteral("setting")) return Ant::IconType::Setting;
    if (key == QStringLiteral("user")) return Ant::IconType::User;
    if (key == QStringLiteral("left")) return Ant::IconType::Left;
    if (key == QStringLiteral("right")) return Ant::IconType::Right;
    if (key == QStringLiteral("up")) return Ant::IconType::Up;
    if (key == QStringLiteral("down")) return Ant::IconType::Down;
    if (key == QStringLiteral("arrow-left")) return Ant::IconType::Left;
    if (key == QStringLiteral("arrow-right")) return Ant::IconType::Right;
    if (key == QStringLiteral("arrow-up")) return Ant::IconType::Up;
    if (key == QStringLiteral("arrow-down")) return Ant::IconType::Down;
    return Ant::IconType::None;
}

inline bool drawIconForKey(QPainter& painter, const QString& icon, const QRectF& iconRect, const QColor& color)
{
    return drawIcon(painter, iconTypeForKey(icon), iconRect, color);
}

inline void drawEllipsis(QPainter& painter, const QRectF& iconRect, const QColor& color)
{
    const qreal dot = qMax<qreal>(2.0, iconRect.width() * 0.14);
    const qreal gap = iconRect.width() * 0.24;
    const QPointF center = iconRect.center();

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    for (int i = -1; i <= 1; ++i)
    {
        const QPointF dotCenter(center.x() + i * gap, center.y());
        painter.drawEllipse(QRectF(dotCenter.x() - dot / 2.0,
                                   dotCenter.y() - dot / 2.0,
                                   dot,
                                   dot));
    }
    painter.restore();
}
} // namespace AntIconPainter
