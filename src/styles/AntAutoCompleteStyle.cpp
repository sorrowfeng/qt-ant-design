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
    AntStyleBase::polish(widget);
    if (qobject_cast<AntAutoComplete*>(widget))
        installPaintFilter<AntAutoComplete>(widget);
}

void AntAutoCompleteStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntAutoComplete*>(widget))
        removePaintFilter<AntAutoComplete>(widget);
    AntStyleBase::unpolish(widget);
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

bool AntAutoCompleteStyle::drawWidget(QWidget* widget, QPaintEvent* event)
{
    auto* ac = qobject_cast<AntAutoComplete*>(widget);
    if (!ac)
    {
        return false;
    }

    QStyleOption option;
    option.initFrom(ac);
    option.rect = ac->rect();
    QPainter painter(ac);
    drawPrimitive(QStyle::PE_Widget, &option, &painter, ac);

    return false;
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

    AntStyleBase::drawCrispRoundedRect(painter, r.toRect(), QPen(border, token.lineWidth),
        enabled ? QBrush(token.colorBgContainer) : QBrush(token.colorBgContainerDisabled), token.borderRadius, token.borderRadius);

    if (focused && enabled)
    {
        AntStyleBase::drawInputFocusGlow(painter, r, token.borderRadius,
            AntPalette::alpha(border, 0.16), token.controlOutlineWidth);
    }

    painter->restore();
}
