#include "AntDropdownStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntDropdown.h"

AntDropdownStyle::AntDropdownStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntDropdown>();
}

void AntDropdownStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntDropdown*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntDropdownStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntDropdown*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntDropdownStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntDropdown*>(widget))
    {
        drawDropdown(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntDropdownStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntDropdownStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* dropdown = qobject_cast<AntDropdown*>(watched);
    if (dropdown && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(dropdown);
        option.rect = dropdown->rect();
        QPainter painter(dropdown);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, dropdown);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntDropdownStyle::drawDropdown(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* dropdown = qobject_cast<const AntDropdown*>(widget);
    if (!dropdown || !painter || !option)
    {
        return;
    }

    // AntDropdown is a trigger widget; its popup is rendered by an internal
    // PopupFrame. The trigger widget itself has no custom paintEvent — it
    // relies on child widgets (e.g. a button) to provide the visual.
    Q_UNUSED(dropdown)
    Q_UNUSED(option)
    Q_UNUSED(painter)
}
