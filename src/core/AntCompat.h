#pragma once

#include <QEvent>
#include <QHoverEvent>
#include <QMargins>
#include <QMetaType>
#include <QMouseEvent>
#include <QPoint>
#include <QPointF>
#include <QWheelEvent>
#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
Q_DECLARE_METATYPE(QMargins)
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using AntEnterEvent = QEnterEvent;
using AntNativeEventResult = qintptr;
#else
using AntEnterEvent = QEvent;
using AntNativeEventResult = long;
#endif

inline QPointF antEventPosition(const QMouseEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event ? event->position() : QPointF();
#else
    return event ? QPointF(event->pos()) : QPointF();
#endif
}

inline QPointF antEventPosition(const QHoverEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event ? event->position() : QPointF();
#else
    return event ? QPointF(event->pos()) : QPointF();
#endif
}

inline QPointF antEventPosition(const QWheelEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event ? event->position() : QPointF();
#else
    return event ? QPointF(event->pos()) : QPointF();
#endif
}

inline QPoint antEventPositionPoint(const QMouseEvent* event)
{
    return antEventPosition(event).toPoint();
}

inline QPoint antEventPositionPoint(const QHoverEvent* event)
{
    return antEventPosition(event).toPoint();
}

inline QPoint antEventPositionPoint(const QWheelEvent* event)
{
    return antEventPosition(event).toPoint();
}

inline QPoint antEventGlobalPosition(const QMouseEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event ? event->globalPosition().toPoint() : QPoint();
#else
    return event ? event->globalPos() : QPoint();
#endif
}

inline bool antIsDevicePixelRatioChangeEvent(QEvent::Type type)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return type == QEvent::DevicePixelRatioChange;
#else
    Q_UNUSED(type);
    return false;
#endif
}
