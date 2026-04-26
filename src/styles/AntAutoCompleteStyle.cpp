#include "AntAutoCompleteStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntAutoComplete.h"

AntAutoCompleteStyle::AntAutoCompleteStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntAutoComplete>();
}

void AntAutoCompleteStyle::onThemeUpdate(QWidget* w)
{
    w->update();
}

void AntAutoCompleteStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntAutoComplete*>(widget))
        widget->installEventFilter(this);
}

void AntAutoCompleteStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntAutoComplete*>(widget))
        widget->removeEventFilter(this);
    QProxyStyle::unpolish(widget);
}

void AntAutoCompleteStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                                          QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntAutoComplete*>(widget))
    {
        drawFrame(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

bool AntAutoCompleteStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* ac = qobject_cast<AntAutoComplete*>(watched);
    if (ac && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(ac);
        option.rect = ac->rect();
        QPainter painter(ac);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, ac);
        return false;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntAutoCompleteStyle::drawFrame(const QStyleOption* option, QPainter* painter,
                                      const QWidget* widget) const
{
    const auto* ac = qobject_cast<const AntAutoComplete*>(widget);
    if (!ac || !painter || !option) return;

    const auto& token = antTheme->tokens();
    const QRectF r = option->rect;
    const bool enabled = option->state.testFlag(QStyle::State_Enabled);
    const bool focused = ac->hasFocus() || (option->state.testFlag(QStyle::State_HasFocus));

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    QColor border = focused ? token.colorPrimary : token.colorBorder;
    if (!enabled) border = token.colorBorderDisabled;

    painter->setPen(QPen(border, token.lineWidth));
    painter->setBrush(enabled ? token.colorBgContainer : token.colorBgContainerDisabled);
    painter->drawRoundedRect(r, token.borderRadius, token.borderRadius);

    if (focused && enabled)
    {
        const QColor focus = AntPalette::alpha(token.colorPrimary, 0.18);
        painter->setPen(QPen(focus, token.controlOutlineWidth));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(r.adjusted(1, 1, -1, -1), token.borderRadius, token.borderRadius);
    }

    painter->restore();
}
