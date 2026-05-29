#include "AntStyleBase.h"

#include <QEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>
#include <QStyleFactory>
#include <QStringList>

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
