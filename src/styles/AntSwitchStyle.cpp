#include "AntSwitchStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntSwitch.h"

namespace
{
QColor trackColorFor(const AntSwitch* sw)
{
    const auto& token = antTheme->tokens();
    if (!sw)
    {
        return token.colorTextDisabled;
    }

    QColor color = sw->isChecked() ? token.colorPrimary : token.colorTextDisabled;
    if (sw->isChecked() && sw->isHoveredState() && sw->isEnabled() && !sw->isLoading())
    {
        color = token.colorPrimaryHover;
    }
    else if (!sw->isChecked() && sw->isHoveredState() && sw->isEnabled() && !sw->isLoading())
    {
        color = token.colorTextTertiary;
    }

    if (!sw->isEnabled() || sw->isLoading())
    {
        color.setAlphaF(0.65);
    }

    return color;
}

void drawSpinner(QPainter& painter, const QRectF& rect, const QColor& color, int angle)
{
    painter.save();
    painter.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(rect, angle * 16, 270 * 16);
    painter.restore();
}
}

AntSwitchStyle::AntSwitchStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntSwitchStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntSwitch*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

void AntSwitchStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntSwitch*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntSwitchStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntSwitch*>(widget))
    {
        drawSwitch(option, painter, widget);
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

bool AntSwitchStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* sw = qobject_cast<AntSwitch*>(watched);
    if (sw && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(sw);
        option.rect = sw->rect();
        if (sw->isChecked())
        {
            option.state |= QStyle::State_On;
        }
        else
        {
            option.state |= QStyle::State_Off;
        }
        if (sw->isHoveredState())
        {
            option.state |= QStyle::State_MouseOver;
        }
        if (sw->isPressedState())
        {
            option.state |= QStyle::State_Sunken;
        }

        QPainter painter(sw);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, sw);
        return false;
    }

    return QProxyStyle::eventFilter(watched, event);
}

void AntSwitchStyle::drawSwitch(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* sw = qobject_cast<const AntSwitch*>(widget);
    if (!sw || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const auto& cache = sw->layoutCache();
    const auto& metrics = cache.metrics;
    const QRectF track = cache.trackRect;
    const QRectF handle = cache.handleRect;
    const bool enabled = option->state.testFlag(QStyle::State_Enabled);
    const bool focused = option->state.testFlag(QStyle::State_HasFocus);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    AntStyleBase::drawCrispRoundedRect(painter, track.toRect(),
        Qt::NoPen, trackColorFor(sw), track.height() / 2.0, track.height() / 2.0);

    const QString label = sw->isChecked() ? sw->checkedText() : sw->uncheckedText();
    if (!label.isEmpty() && sw->switchSize() == Ant::Size::Middle)
    {
        QRectF textRect = track.adjusted(metrics.trackPadding + metrics.innerMinMargin,
                                         0,
                                         -(metrics.trackPadding + metrics.innerMinMargin),
                                         0);
        if (sw->isChecked())
        {
            textRect.setRight(handle.left() - 2);
        }
        else
        {
            textRect.setLeft(handle.right() + 2);
        }

        QFont font = sw->font();
        font.setPixelSize(metrics.fontSize);
        painter->setFont(font);
        QColor textColor = token.colorTextLightSolid;
        if (!enabled || sw->isLoading())
        {
            textColor.setAlphaF(0.9);
        }
        painter->setPen(textColor);
        painter->drawText(textRect, Qt::AlignCenter, label);
    }

    AntStyleBase::drawCrispRoundedRect(painter, handle.toRect(),
        Qt::NoPen, token.colorTextLightSolid, handle.height() / 2.0, handle.height() / 2.0);

    AntStyleBase::drawCrispRoundedRect(painter, handle.toRect(),
        QPen(QColor(0, 35, 11, 50), 1), Qt::NoBrush,
        handle.height() / 2.0, handle.height() / 2.0);

    if (sw->isLoading())
    {
        const QColor spinnerColor = sw->isChecked() ? token.colorPrimary : token.colorTextTertiary;
        drawSpinner(*painter, handle.adjusted(4, 4, -4, -4), spinnerColor, sw->loadingAngle());
    }

    if (focused && enabled && !sw->isLoading())
    {
        const QColor focus = AntPalette::alpha(token.colorPrimary, 0.10);
        AntStyleBase::drawCrispRoundedRect(painter, track.toRect(),
            QPen(focus, token.controlOutlineWidth), Qt::NoBrush,
            track.height() / 2.0, track.height() / 2.0);
    }

    painter->restore();
}
