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
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntTabsStyle::unpolish(QWidget* widget)
{
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

    painter->fillRect(option->rect, token.colorBgContainer);

    painter->restore();
}
