#pragma once

#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QRectF>
#include <QString>

namespace AntIconSvgRenderer
{
bool drawIcon(QPainter& painter,
              const QString& iconName,
              const QRectF& iconRect,
              const QColor& primaryColor,
              const QColor& secondaryColor = QColor());

QPixmap renderIconPixmap(const QString& iconName,
                         qreal iconSize,
                         const QColor& primaryColor,
                         qreal devicePixelRatio,
                         const QColor& secondaryColor = QColor());
} // namespace AntIconSvgRenderer
