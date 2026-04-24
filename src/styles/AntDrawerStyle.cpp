#include "AntDrawerStyle.h"

#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "widgets/AntDrawer.h"

AntDrawerStyle::AntDrawerStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntDrawer*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->updateGeometry();
                w->update();
            }
        }
    });
}

void AntDrawerStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntDrawer*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntDrawerStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntDrawer*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntDrawerStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntDrawer*>(widget))
    {
        drawDrawer(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntDrawerStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntDrawerStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* drawer = qobject_cast<AntDrawer*>(watched);
    if (drawer && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(drawer);
        option.rect = drawer->rect();
        QPainter painter(drawer);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, drawer);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntDrawerStyle::drawDrawer(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* drawer = qobject_cast<const AntDrawer*>(widget);
    if (!drawer || !painter || !option)
    {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    const qreal opacity = antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.58 : 0.45;
    painter->fillRect(option->rect, AntPalette::alpha(Qt::black, opacity));
    painter->restore();
}
