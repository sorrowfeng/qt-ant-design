#include "AntTabsStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include <algorithm>

#include "styles/AntPalette.h"
#include "widgets/AntTabs.h"

AntTabsStyle::AntTabsStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntTabs>();
}

void AntTabsStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntTabs*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntTabsStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntTabs*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntTabsStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntTabs*>(widget))
    {
        drawTabs(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntTabsStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntTabsStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* tabs = qobject_cast<AntTabs*>(watched);
    if (tabs && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(tabs);
        option.rect = tabs->rect();
        QPainter painter(tabs);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, tabs);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntTabsStyle::drawTabs(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* tabs = qobject_cast<const AntTabs*>(widget);
    if (!tabs || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // Background
    painter->fillRect(option->rect, token.colorBgContainer);

    // Note: AntTabs's tab items (m_tabs) are private with no public accessor.
    // Tab items, tab rects, and individual tab drawing cannot be performed
    // from the style class. The background is drawn here; tab bar rendering
    // requires a public tabs API or friend access.

    painter->restore();
}
