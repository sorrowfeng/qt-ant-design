#include "AntCascaderStyle.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include <algorithm>

#include "styles/AntPalette.h"
#include "widgets/AntCascader.h"

namespace
{
struct CascaderMetrics
{
    int height = 32;
    int fontSize = 14;
    int radius = 6;
    int paddingX = 11;
    int arrowWidth = 28;
};

CascaderMetrics metricsFor(const AntCascader* cascader)
{
    const auto& token = antTheme->tokens();
    CascaderMetrics metrics;
    metrics.height = token.controlHeight;
    metrics.fontSize = token.fontSize;
    metrics.radius = token.borderRadius;
    metrics.paddingX = token.paddingSM - token.lineWidth;
    metrics.arrowWidth = token.fontSize + token.paddingXS * 2;

    if (!cascader)
    {
        return metrics;
    }

    if (cascader->cascaderSize() == Ant::Size::Large)
    {
        metrics.height = token.controlHeightLG;
        metrics.fontSize = token.fontSizeLG;
    }
    else if (cascader->cascaderSize() == Ant::Size::Small)
    {
        metrics.height = token.controlHeightSM;
        metrics.fontSize = token.fontSizeSM;
        metrics.radius = token.borderRadiusSM;
        metrics.paddingX = token.paddingXS;
    }

    return metrics;
}

QRectF controlRectFor(const AntCascader* cascader, const QRect& rect)
{
    const CascaderMetrics metrics = metricsFor(cascader);
    return QRectF(1, (rect.height() - metrics.height) / 2.0, rect.width() - 2, metrics.height);
}

QRectF clearButtonRectFor(const AntCascader* cascader, const QRect& rect)
{
    const CascaderMetrics metrics = metricsFor(cascader);
    const QRectF control = controlRectFor(cascader, rect);
    const qreal size = std::min<qreal>(18, control.height() - 8);
    return QRectF(control.right() - metrics.paddingX - size,
                  control.center().y() - size / 2.0,
                  size,
                  size);
}

QColor borderColorFor(const AntCascader* cascader)
{
    const auto& token = antTheme->tokens();
    if (!cascader || !cascader->isEnabled())
    {
        return token.colorBorderDisabled;
    }
    if (cascader->status() == Ant::Status::Error)
    {
        return (cascader->isHoveredState() || cascader->hasFocus() || cascader->isOpen())
            ? token.colorErrorHover
            : token.colorError;
    }
    if (cascader->status() == Ant::Status::Warning)
    {
        return (cascader->isHoveredState() || cascader->hasFocus() || cascader->isOpen())
            ? token.colorWarningHover
            : token.colorWarning;
    }
    if (cascader->isHoveredState() || cascader->hasFocus() || cascader->isOpen())
    {
        return token.colorPrimaryHover;
    }
    return token.colorBorder;
}

QColor backgroundColorFor(const AntCascader* cascader)
{
    const auto& token = antTheme->tokens();
    if (!cascader || !cascader->isEnabled())
    {
        return token.colorBgContainerDisabled;
    }
    if (cascader->variant() == Ant::Variant::Filled)
    {
        return cascader->isHoveredState() ? token.colorFillTertiary : token.colorFillQuaternary;
    }
    if (cascader->variant() == Ant::Variant::Borderless
        || cascader->variant() == Ant::Variant::Underlined)
    {
        return QColor(0, 0, 0, 0);
    }
    return token.colorBgContainer;
}

bool canClear(const AntCascader* cascader)
{
    return cascader
        && cascader->isEnabled()
        && cascader->allowClear()
        && cascader->isHoveredState()
        && !cascader->value().isEmpty();
}
} // namespace

AntCascaderStyle::AntCascaderStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntCascader>();
}

void AntCascaderStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntCascader*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

void AntCascaderStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntCascader*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntCascaderStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntCascader*>(widget))
    {
        drawCascader(option, painter, widget);
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

bool AntCascaderStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* cascader = qobject_cast<AntCascader*>(watched);
    if (cascader && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(cascader);
        option.rect = cascader->rect();
        if (cascader->isHoveredState())
        {
            option.state |= QStyle::State_MouseOver;
        }
        if (cascader->isPressedState())
        {
            option.state |= QStyle::State_Sunken;
        }
        if (cascader->isOpen())
        {
            option.state |= QStyle::State_On;
        }

        QPainter painter(cascader);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, cascader);
        return true;
    }

    return QProxyStyle::eventFilter(watched, event);
}

void AntCascaderStyle::drawCascader(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* cascader = qobject_cast<const AntCascader*>(widget);
    if (!cascader || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const CascaderMetrics metrics = metricsFor(cascader);
    const QRectF control = controlRectFor(cascader, option->rect);
    const QRectF clearRect = clearButtonRectFor(cascader, option->rect);
    const bool disabled = !option->state.testFlag(QStyle::State_Enabled);
    const bool focused = option->state.testFlag(QStyle::State_HasFocus) || cascader->isOpen();
    const QColor bColor = borderColorFor(cascader);
    const QColor bgColor = backgroundColorFor(cascader);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (focused
        && !disabled
        && cascader->variant() != Ant::Variant::Borderless
        && cascader->variant() != Ant::Variant::Underlined)
    {
        const QColor outline = AntPalette::alpha(bColor, 0.16);
        AntStyleBase::drawCrispRoundedRect(painter, control.adjusted(-1, -1, 1, 1).toRect(),
            QPen(outline, token.controlOutlineWidth), Qt::NoBrush, metrics.radius + 1, metrics.radius + 1);
    }

    if (cascader->variant() != Ant::Variant::Borderless
        && cascader->variant() != Ant::Variant::Underlined)
    {
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(), QPen(bColor, token.lineWidth),
            bgColor, metrics.radius, metrics.radius);
    }
    else
    {
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(), Qt::NoPen,
            bgColor, metrics.radius, metrics.radius);
        if (cascader->variant() == Ant::Variant::Underlined)
        {
            painter->setPen(QPen(bColor, focused ? 2 : token.lineWidth));
            painter->drawLine(QPointF(control.left(), control.bottom() - 0.5),
                              QPointF(control.right(), control.bottom() - 0.5));
        }
    }

    const bool hasValue = !cascader->value().isEmpty();
    const QString text = hasValue ? cascader->displayText() : cascader->placeholder();
    QColor textColor = hasValue ? token.colorText : token.colorTextPlaceholder;
    if (disabled)
    {
        textColor = token.colorTextDisabled;
    }

    QFont font = painter->font();
    font.setPixelSize(metrics.fontSize);
    painter->setFont(font);
    painter->setPen(textColor);
    const QRectF textRect = control.adjusted(metrics.paddingX, 0, -(metrics.arrowWidth + metrics.paddingX), 0);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, text);

    if (canClear(cascader))
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorBgBase);
        painter->drawEllipse(clearRect.adjusted(1, 1, -1, -1));
        painter->setPen(QPen(token.colorTextTertiary, 1.5, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(clearRect.center() + QPointF(-4, -4), clearRect.center() + QPointF(4, 4));
        painter->drawLine(clearRect.center() + QPointF(4, -4), clearRect.center() + QPointF(-4, 4));
    }
    else
    {
        painter->save();
        painter->translate(clearRect.center());
        painter->rotate(cascader->arrowRotation());
        painter->translate(-clearRect.center());
        painter->setPen(QPen(disabled ? token.colorTextDisabled : token.colorTextTertiary,
                             1.7,
                             Qt::SolidLine,
                             Qt::RoundCap,
                             Qt::RoundJoin));
        QPainterPath arrow;
        arrow.moveTo(clearRect.center().x() - 5, clearRect.center().y() - 2);
        arrow.lineTo(clearRect.center().x(), clearRect.center().y() + 3);
        arrow.lineTo(clearRect.center().x() + 5, clearRect.center().y() - 2);
        painter->drawPath(arrow);
        painter->restore();
    }

    painter->restore();
}
