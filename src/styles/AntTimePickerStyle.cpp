#include "AntTimePickerStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntIconPainter.h"
#include "styles/AntPalette.h"
#include "widgets/AntTimePicker.h"

AntTimePickerStyle::AntTimePickerStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntTimePicker>();
}

void AntTimePickerStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntTimePicker*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntTimePickerStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntTimePicker*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntTimePickerStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntTimePicker*>(widget))
    {
        drawTimePicker(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntTimePickerStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntTimePickerStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* picker = qobject_cast<AntTimePicker*>(watched);
    if (picker && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(picker);
        option.rect = picker->rect();
        QPainter painter(picker);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, picker);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

namespace
{
struct TimePickerMetrics
{
    int height = 32;
    int fontSize = 14;
    int radius = 6;
    int paddingX = 11;
    int iconWidth = 30;
};

TimePickerMetrics timePickerMetrics(const AntTimePicker* picker)
{
    const auto& token = antTheme->tokens();
    TimePickerMetrics m;
    m.height = token.controlHeight;
    m.fontSize = token.fontSize;
    m.radius = token.borderRadius;
    m.paddingX = token.paddingSM - token.lineWidth;
    m.iconWidth = token.fontSize + token.paddingXS * 2;
    if (picker->pickerSize() == Ant::Size::Large)
    {
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
    }
    else if (picker->pickerSize() == Ant::Size::Small)
    {
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSizeSM;
        m.radius = token.borderRadiusSM;
        m.paddingX = token.paddingXS;
    }
    return m;
}

QRectF timePickerControlRect(const AntTimePicker* picker)
{
    const TimePickerMetrics m = timePickerMetrics(picker);
    return QRectF(1, (picker->height() - m.height) / 2.0, picker->width() - 2, m.height);
}

QRectF timePickerIconRect(const AntTimePicker* picker)
{
    const TimePickerMetrics m = timePickerMetrics(picker);
    const QRectF control = timePickerControlRect(picker);
    return QRectF(control.right() - m.iconWidth, control.top(), m.iconWidth, control.height());
}

QColor timePickerBorderColor(const AntTimePicker* picker)
{
    const auto& token = antTheme->tokens();
    const bool focused = picker->hasFocus() || picker->isOpen();
    if (!picker->isEnabled())
    {
        return token.colorBorderDisabled;
    }
    if (picker->status() == Ant::Status::Error)
    {
        return focused ? token.colorError : (picker->isHoveredState() ? token.colorErrorHover
                                                                       : token.colorError);
    }
    if (picker->status() == Ant::Status::Warning)
    {
        return focused ? token.colorWarning : (picker->isHoveredState() ? token.colorWarningHover
                                                                        : token.colorWarning);
    }
    if (focused)
    {
        return token.colorPrimary;
    }
    if (picker->isHoveredState())
    {
        return token.colorPrimaryHover;
    }
    return token.colorBorder;
}

QColor timePickerBackgroundColor(const AntTimePicker* picker)
{
    const auto& token = antTheme->tokens();
    if (!picker->isEnabled())
    {
        return token.colorBgContainerDisabled;
    }
    if (picker->variant() == Ant::Variant::Filled)
    {
        return (picker->hasFocus() || picker->isOpen())
            ? token.colorBgContainer
            : (picker->isHoveredState() ? token.colorFillSecondary : token.colorFillTertiary);
    }
    if (picker->variant() == Ant::Variant::Borderless ||
        picker->variant() == Ant::Variant::Underlined)
    {
        return QColor(0, 0, 0, 0);
    }
    return token.colorBgContainer;
}

bool timePickerCanClear(const AntTimePicker* picker)
{
    return picker->isEnabled() && picker->allowClear() && picker->isHoveredState() && picker->hasSelectedTime();
}
} // namespace

void AntTimePickerStyle::drawTimePicker(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* picker = qobject_cast<const AntTimePicker*>(widget);
    if (!picker || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const TimePickerMetrics m = timePickerMetrics(picker);
    const QRectF control = timePickerControlRect(picker);
    const bool focused = picker->hasFocus() || picker->isOpen();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // Focus ring
    if (focused && picker->isEnabled() &&
        picker->variant() != Ant::Variant::Borderless &&
        picker->variant() != Ant::Variant::Underlined)
    {
        AntStyleBase::drawCrispRoundedRect(painter, control.adjusted(-1, -1, 1, 1).toRect(),
            QPen(AntPalette::alpha(timePickerBorderColor(picker), 0.16), token.controlOutlineWidth),
            Qt::NoBrush, m.radius + 1, m.radius + 1);
    }

    // Border and background
    if (picker->variant() != Ant::Variant::Borderless &&
        picker->variant() != Ant::Variant::Underlined)
    {
        const bool filled = picker->variant() == Ant::Variant::Filled;
        const QPen framePen = filled && !focused
            ? Qt::NoPen
            : QPen(timePickerBorderColor(picker), token.lineWidth);
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(),
            framePen,
            timePickerBackgroundColor(picker), m.radius, m.radius);
    }
    else
    {
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(), Qt::NoPen,
            timePickerBackgroundColor(picker), m.radius, m.radius);
        if (picker->variant() == Ant::Variant::Underlined)
        {
            painter->setPen(QPen(timePickerBorderColor(picker), focused ? 2 : token.lineWidth));
            painter->drawLine(QPointF(control.left(), control.bottom() - 0.5),
                             QPointF(control.right(), control.bottom() - 0.5));
        }
    }

    // Text
    QFont f = painter->font();
    f.setPixelSize(m.fontSize);
    painter->setFont(f);

    if (picker->isRangeMode())
    {
        const QRectF textArea = control.adjusted(m.paddingX, 0, -(m.iconWidth + m.paddingX), 0);
        const qreal arrowWidth = 24;
        const qreal fieldWidth = (textArea.width() - arrowWidth) / 2.0;
        const QRectF startRect(textArea.left(), textArea.top(), fieldWidth, textArea.height());
        const QRectF arrowRect(startRect.right(), textArea.top(), arrowWidth, textArea.height());
        const QRectF endRect(arrowRect.right(), textArea.top(), fieldWidth, textArea.height());
        const QString startText = picker->startTime().isValid()
            ? picker->startTime().toString(picker->displayFormat())
            : QStringLiteral("Start time");
        const QString endText = picker->endTime().isValid()
            ? picker->endTime().toString(picker->displayFormat())
            : QStringLiteral("End time");
        QColor startColor = picker->startTime().isValid() ? token.colorText : token.colorTextPlaceholder;
        QColor endColor = picker->endTime().isValid() ? token.colorText : token.colorTextPlaceholder;
        if (!picker->isEnabled())
        {
            startColor = token.colorTextDisabled;
            endColor = token.colorTextDisabled;
        }

        painter->setPen(startColor);
        painter->drawText(startRect, Qt::AlignVCenter | Qt::AlignLeft, startText);

        AntIconPainter::drawIcon(*painter,
                                 Ant::IconType::Right,
                                 arrowRect.adjusted(4, 4, -4, -4),
                                 token.colorTextTertiary);

        painter->setPen(endColor);
        painter->drawText(endRect, Qt::AlignVCenter | Qt::AlignLeft, endText);
    }
    else
    {
        const QString text = picker->hasSelectedTime() ? picker->timeString() : picker->placeholderText();
        QColor textColor = picker->hasSelectedTime() ? token.colorText : token.colorTextPlaceholder;
        if (!picker->isEnabled())
        {
            textColor = token.colorTextDisabled;
        }
        painter->setPen(textColor);
        painter->drawText(control.adjusted(m.paddingX, 0, -(m.iconWidth + m.paddingX), 0),
                         Qt::AlignVCenter | Qt::AlignLeft, text);
    }

    // Icon area
    const QRectF icon = timePickerIconRect(picker);
    if (timePickerCanClear(picker))
    {
        const QPointF center = icon.center();
        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorTextTertiary);
        painter->drawEllipse(center, 5, 5);
        painter->setPen(QPen(token.colorBgContainer, 1.3, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(center + QPointF(-2.2, -2.2), center + QPointF(2.2, 2.2));
        painter->drawLine(center + QPointF(2.2, -2.2), center + QPointF(-2.2, 2.2));
    }
    else
    {
        QColor iconColor = token.colorTextDisabled;
        if (picker->isEnabled() && picker->status() == Ant::Status::Error)
        {
            iconColor = token.colorError;
        }
        else if (picker->isEnabled() && picker->status() == Ant::Status::Warning)
        {
            iconColor = token.colorWarning;
        }
        const qreal iconSize = picker->pickerSize() == Ant::Size::Small ? 12.0 : 14.0;
        painter->setPen(QPen(iconColor, 1.2, Qt::SolidLine, Qt::RoundCap));
        painter->setBrush(Qt::NoBrush);
        const QPointF center = icon.center();
        painter->drawEllipse(center, iconSize / 2.0, iconSize / 2.0);
        painter->drawLine(center, center + QPointF(0, -iconSize * 0.32));
        painter->drawLine(center, center + QPointF(iconSize * 0.32, iconSize * 0.18));
    }

    painter->restore();
}
