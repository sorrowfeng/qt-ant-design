#include "AntSwitchStyle.h"

#include <QApplication>
#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include <algorithm>
#include <cmath>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "widgets/AntSwitch.h"

namespace
{
struct SwitchMetrics
{
    int trackHeight = 22;
    int trackMinWidth = 44;
    int trackPadding = 2;
    int handleSize = 18;
    int fontSize = 12;
    int innerMinMargin = 9;
};

SwitchMetrics metricsFor(const AntSwitch* sw)
{
    const auto& token = antTheme->tokens();
    SwitchMetrics metrics;
    const qreal defaultHeight = token.fontSize * token.lineHeight;
    if (!sw || sw->switchSize() == Ant::SwitchSize::Middle)
    {
        metrics.trackHeight = static_cast<int>(std::round(defaultHeight));
        metrics.trackPadding = 2;
        metrics.handleSize = metrics.trackHeight - metrics.trackPadding * 2;
        metrics.trackMinWidth = metrics.handleSize * 2 + metrics.trackPadding * 4;
        metrics.fontSize = token.fontSizeSM;
        metrics.innerMinMargin = metrics.handleSize / 2;
    }
    else
    {
        metrics.trackHeight = token.controlHeight / 2;
        metrics.trackPadding = 2;
        metrics.handleSize = metrics.trackHeight - metrics.trackPadding * 2;
        metrics.trackMinWidth = metrics.handleSize * 2 + metrics.trackPadding * 2;
        metrics.fontSize = token.fontSizeSM;
        metrics.innerMinMargin = metrics.handleSize / 2;
    }

    QFont font = sw ? sw->font() : QFont();
    font.setPixelSize(metrics.fontSize);
    const QFontMetrics fontMetrics(font);
    const int labelWidth = sw ? std::max(fontMetrics.horizontalAdvance(sw->checkedText()),
                                         fontMetrics.horizontalAdvance(sw->uncheckedText()))
                              : 0;
    if (labelWidth > 0)
    {
        metrics.trackMinWidth = std::max(metrics.trackMinWidth,
                                         labelWidth + metrics.handleSize + metrics.trackPadding * 8);
    }

    return metrics;
}

QRectF trackRectFor(const AntSwitch* sw, const QRect& rect)
{
    const SwitchMetrics metrics = metricsFor(sw);
    return QRectF((rect.width() - metrics.trackMinWidth) / 2.0,
                  (rect.height() - metrics.trackHeight) / 2.0,
                  metrics.trackMinWidth,
                  metrics.trackHeight);
}

QRectF handleRectFor(const AntSwitch* sw, const QRect& rect)
{
    const SwitchMetrics metrics = metricsFor(sw);
    const QRectF track = trackRectFor(sw, rect);
    const qreal left = track.left() + metrics.trackPadding;
    const qreal right = track.right() - metrics.trackPadding - metrics.handleSize;
    qreal x = left + (right - left) * (sw ? sw->handleProgress() : 0.0);
    qreal width = metrics.handleSize + metrics.trackPadding * 2 * (sw ? sw->handleStretch() : 0.0);

    if (sw && sw->isChecked())
    {
        x -= metrics.trackPadding * 2 * sw->handleStretch();
    }

    return QRectF(x, track.top() + metrics.trackPadding, width, metrics.handleSize);
}

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
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* widget : widgets)
        {
            if (qobject_cast<AntSwitch*>(widget) && widget->style() == this)
            {
                unpolish(widget);
                polish(widget);
                widget->updateGeometry();
                widget->update();
            }
        }
    });
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
    const SwitchMetrics metrics = metricsFor(sw);
    const QRectF track = trackRectFor(sw, option->rect);
    const QRectF handle = handleRectFor(sw, option->rect);
    const bool enabled = option->state.testFlag(QStyle::State_Enabled);
    const bool focused = option->state.testFlag(QStyle::State_HasFocus);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    painter->setPen(Qt::NoPen);
    painter->setBrush(trackColorFor(sw));
    painter->drawRoundedRect(track, track.height() / 2.0, track.height() / 2.0);

    const QString label = sw->isChecked() ? sw->checkedText() : sw->uncheckedText();
    if (!label.isEmpty() && sw->switchSize() == Ant::SwitchSize::Middle)
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

    painter->setPen(Qt::NoPen);
    painter->setBrush(token.colorTextLightSolid);
    painter->drawRoundedRect(handle, handle.height() / 2.0, handle.height() / 2.0);

    painter->setPen(QPen(QColor(0, 35, 11, 50), 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(handle.adjusted(0.5, 0.5, -0.5, -0.5),
                             handle.height() / 2.0,
                             handle.height() / 2.0);

    if (sw->isLoading())
    {
        const QColor spinnerColor = sw->isChecked() ? token.colorPrimary : token.colorTextTertiary;
        drawSpinner(*painter, handle.adjusted(4, 4, -4, -4), spinnerColor, sw->loadingAngle());
    }

    if (focused && enabled && !sw->isLoading())
    {
        const QColor focus = AntPalette::alpha(token.colorPrimary, 0.22);
        painter->setPen(QPen(focus, token.controlOutlineWidth));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(track.adjusted(1, 1, -1, -1), track.height() / 2.0, track.height() / 2.0);
    }

    painter->restore();
}
