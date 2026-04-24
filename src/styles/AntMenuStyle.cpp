#include "AntMenuStyle.h"

#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "widgets/AntMenu.h"

AntMenuStyle::AntMenuStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntMenu*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->updateGeometry();
                w->update();
            }
        }
    });
}

void AntMenuStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntMenu*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntMenuStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntMenu*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntMenuStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntMenu*>(widget))
    {
        drawMenu(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntMenuStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntMenuStyle::eventFilter(QObject* watched, QEvent* event)
{
    Q_UNUSED(watched)
    Q_UNUSED(event)
    // AntMenu paints itself (background + items); we don't intercept paint
    // here, otherwise the menu items would vanish. Style still participates
    // via polish()/unpolish() for theme wiring.
    return QProxyStyle::eventFilter(watched, event);
}

void AntMenuStyle::drawMenu(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* menu = qobject_cast<const AntMenu*>(widget);
    if (!menu || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    const Ant::MenuMode mode = menu->mode();
    const Ant::MenuTheme menuTheme = menu->menuTheme();

    // Background
    const QColor bgColor = (menuTheme == Ant::MenuTheme::Dark) ? QColor(0, 21, 41) : token.colorBgContainer;
    painter->fillRect(option->rect, bgColor);

    // Border line
    if (mode != Ant::MenuMode::Horizontal)
    {
        if (menuTheme != Ant::MenuTheme::Dark)
        {
            painter->setPen(QPen(token.colorSplit, token.lineWidth));
            painter->drawLine(option->rect.topRight(), option->rect.bottomRight());
        }
    }
    else
    {
        if (menuTheme != Ant::MenuTheme::Dark)
        {
            painter->setPen(QPen(token.colorSplit, token.lineWidth));
            painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
        }
    }

    // Note: AntMenu's item data (m_items) is private with no public accessor.
    // Individual menu items cannot be painted from the style class without
    // adding a public items API. The background and border are drawn here;
    // item rendering is handled by the widget's internal paintEvent which
    // still runs for item-level drawing via child widgets or overlays.

    painter->restore();
}
