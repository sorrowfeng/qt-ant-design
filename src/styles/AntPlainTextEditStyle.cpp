#include "AntPlainTextEditStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntPlainTextEdit.h"

namespace
{

struct Metrics
{
    int radius = 6;
    int lineWidth = 1;
    int focusWidth = 2;
};

Metrics frameMetrics()
{
    const auto& token = antTheme->tokens();
    Metrics m;
    m.radius = token.borderRadius;
    m.lineWidth = token.lineWidth;
    m.focusWidth = token.controlOutlineWidth;
    return m;
}

} // namespace

AntPlainTextEditStyle::AntPlainTextEditStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntPlainTextEdit>();
}

void AntPlainTextEditStyle::onThemeUpdate(QWidget* w)
{
    w->update();
}

void AntPlainTextEditStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntPlainTextEdit*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntPlainTextEditStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntPlainTextEdit*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntPlainTextEditStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                                          QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntPlainTextEdit*>(widget))
    {
        drawFrame(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

bool AntPlainTextEditStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* te = qobject_cast<AntPlainTextEdit*>(watched);
    if (te && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(te);
        option.rect = te->rect();
        QPainter painter(te);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, te);
        return false;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntPlainTextEditStyle::drawFrame(const QStyleOption* option, QPainter* painter,
                                      const QWidget* widget) const
{
    const auto* te = qobject_cast<const AntPlainTextEdit*>(widget);
    if (!te || !painter || !option) return;

    const auto& token = antTheme->tokens();
    const Metrics m = frameMetrics();
    const QRectF r = option->rect;
    const bool focused = te->hasFocus();
    const bool enabled = option->state.testFlag(QStyle::State_Enabled);
    const Ant::Variant variant = te->variant();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    if (variant == Ant::Variant::Filled)
    {
        QColor bg = token.colorFillTertiary;
        if (focused) bg = token.colorBgContainer;
        AntStyleBase::drawCrispRoundedRect(painter, r.toRect(),
            Qt::NoPen, enabled ? bg : token.colorBgContainerDisabled, m.radius, m.radius);
        if (focused)
        {
            AntStyleBase::drawCrispRoundedRect(painter, r.toRect(),
                QPen(token.colorBorder, m.lineWidth), Qt::NoBrush, m.radius, m.radius);
        }
    }
    else if (variant == Ant::Variant::Outlined)
    {
        QColor border = token.colorBorder;
        if (focused) border = token.colorPrimary;
        if (!enabled) border = token.colorBorderDisabled;
        AntStyleBase::drawCrispRoundedRect(painter, r.toRect(),
            QPen(border, m.lineWidth), token.colorBgContainer, m.radius, m.radius);
    }
    // Borderless: no frame

    // Focus glow
    if (focused && enabled && variant != Ant::Variant::Borderless)
    {
        const QColor focus = AntPalette::alpha(token.colorPrimary, 0.18);
        AntStyleBase::drawCrispRoundedRect(painter, r.toRect(),
            QPen(focus, m.focusWidth), Qt::NoBrush, m.radius, m.radius);
    }

    // Placeholder
    if (te->document()->isEmpty() && !focused && !te->placeholderText().isEmpty())
    {
        QFont f = te->font();
        painter->setFont(f);
        painter->setPen(token.colorTextPlaceholder);
        const QRectF textRect = r.adjusted(token.paddingSM, 0, -token.paddingSM, 0);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, te->placeholderText());
    }

    painter->restore();
}
