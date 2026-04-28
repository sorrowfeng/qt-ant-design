#include "AntDatePickerStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntDatePicker.h"

AntDatePickerStyle::AntDatePickerStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntDatePicker>();
}

void AntDatePickerStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntDatePicker*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntDatePickerStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntDatePicker*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntDatePickerStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntDatePicker*>(widget))
    {
        drawDatePicker(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntDatePickerStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntDatePickerStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* picker = qobject_cast<AntDatePicker*>(watched);
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
struct PickerMetrics
{
    int height = 32;
    int fontSize = 14;
    int radius = 6;
    int paddingX = 11;
    int iconWidth = 30;
};

PickerMetrics datePickerMetrics(const AntDatePicker* picker)
{
    const auto& token = antTheme->tokens();
    PickerMetrics m;
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

QRectF datePickerControlRect(const AntDatePicker* picker)
{
    const PickerMetrics m = datePickerMetrics(picker);
    return QRectF(1, (picker->height() - m.height) / 2.0, picker->width() - 2, m.height);
}

QRectF datePickerIconRect(const AntDatePicker* picker)
{
    const PickerMetrics m = datePickerMetrics(picker);
    const QRectF control = datePickerControlRect(picker);
    return QRectF(control.right() - m.iconWidth, control.top(), m.iconWidth, control.height());
}

QColor datePickerBorderColor(const AntDatePicker* picker)
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

QColor datePickerBackgroundColor(const AntDatePicker* picker)
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

bool datePickerCanClear(const AntDatePicker* picker)
{
    return picker->isEnabled() && picker->allowClear() && picker->isHoveredState() && picker->hasSelectedDate();
}
} // namespace

void AntDatePickerStyle::drawDatePicker(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* picker = qobject_cast<const AntDatePicker*>(widget);
    if (!picker || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const PickerMetrics m = datePickerMetrics(picker);
    const QRectF control = datePickerControlRect(picker);
    const bool focused = picker->hasFocus() || picker->isOpen();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // Focus ring
    if (focused && picker->isEnabled() &&
        picker->variant() != Ant::Variant::Borderless &&
        picker->variant() != Ant::Variant::Underlined)
    {
        AntStyleBase::drawCrispRoundedRect(painter, control.adjusted(-1, -1, 1, 1).toRect(),
            QPen(AntPalette::alpha(datePickerBorderColor(picker), 0.16), token.controlOutlineWidth),
            Qt::NoBrush, m.radius + 1, m.radius + 1);
    }

    // Border and background
    if (picker->variant() != Ant::Variant::Borderless &&
        picker->variant() != Ant::Variant::Underlined)
    {
        const bool filled = picker->variant() == Ant::Variant::Filled;
        const QPen framePen = filled && !focused
            ? Qt::NoPen
            : QPen(datePickerBorderColor(picker), token.lineWidth);
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(),
            framePen,
            datePickerBackgroundColor(picker), m.radius, m.radius);
    }
    else
    {
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(), Qt::NoPen,
            datePickerBackgroundColor(picker), m.radius, m.radius);
        if (picker->variant() == Ant::Variant::Underlined)
        {
            painter->setPen(QPen(datePickerBorderColor(picker), focused ? 2 : token.lineWidth));
            painter->drawLine(QPointF(control.left(), control.bottom() - 0.5),
                             QPointF(control.right(), control.bottom() - 0.5));
        }
    }

    // Text
    QFont f = painter->font();
    f.setPixelSize(m.fontSize);
    painter->setFont(f);

    const QRectF textArea = control.adjusted(m.paddingX, 0, -(m.iconWidth + m.paddingX), 0);
    if (picker->isRangeMode())
    {
        const QColor placeholderColor = picker->isEnabled() ? token.colorTextPlaceholder : token.colorTextDisabled;
        const QColor valueColor = picker->isEnabled() ? token.colorText : token.colorTextDisabled;
        const qreal arrowWidth = 24;
        const qreal fieldWidth = (textArea.width() - arrowWidth) / 2.0;
        const QRectF startRect(textArea.left(), textArea.top(), fieldWidth, textArea.height());
        const QRectF arrowRect(startRect.right(), textArea.top(), arrowWidth, textArea.height());
        const QRectF endRect(arrowRect.right(), textArea.top(), fieldWidth, textArea.height());
        const QString startText = picker->startDate().isValid()
            ? picker->startDate().toString(picker->displayFormat())
            : QStringLiteral("Start date");
        const QString endText = picker->endDate().isValid()
            ? picker->endDate().toString(picker->displayFormat())
            : QStringLiteral("End date");

        painter->setPen(picker->startDate().isValid() ? valueColor : placeholderColor);
        painter->drawText(startRect, Qt::AlignVCenter | Qt::AlignLeft, startText);
        painter->setPen(picker->endDate().isValid() ? valueColor : placeholderColor);
        painter->drawText(endRect, Qt::AlignVCenter | Qt::AlignLeft, endText);
        painter->setPen(token.colorTextDisabled);
        const QPointF arrowCenter = arrowRect.center();
        painter->drawLine(arrowCenter + QPointF(-5, 0), arrowCenter + QPointF(5, 0));
        painter->drawLine(arrowCenter + QPointF(2, -3), arrowCenter + QPointF(5, 0));
        painter->drawLine(arrowCenter + QPointF(2, 3), arrowCenter + QPointF(5, 0));
    }
    else
    {
        const QString text = picker->hasSelectedDate() ? picker->dateString() : picker->placeholderText();
        QColor textColor = picker->hasSelectedDate() ? token.colorText : token.colorTextPlaceholder;
        if (!picker->isEnabled())
        {
            textColor = token.colorTextDisabled;
        }
        painter->setPen(textColor);
        painter->drawText(textArea, Qt::AlignVCenter | Qt::AlignLeft, text);
    }

    // Icon area
    const QRectF icon = datePickerIconRect(picker);
    if (datePickerCanClear(picker))
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
        const qreal iconSize = picker->pickerSize() == Ant::Size::Small ? 12.0 : 14.0;
        const QRectF cal(icon.center().x() - iconSize / 2.0,
                         icon.center().y() - iconSize / 2.0,
                         iconSize,
                         iconSize);
        QColor iconColor = token.colorTextDisabled;
        if (picker->isEnabled() && picker->status() == Ant::Status::Error)
        {
            iconColor = token.colorError;
        }
        else if (picker->isEnabled() && picker->status() == Ant::Status::Warning)
        {
            iconColor = token.colorWarning;
        }
        painter->setPen(QPen(iconColor, 1.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(cal, 1.5, 1.5);
        painter->drawLine(QPointF(cal.left(), cal.top() + 5), QPointF(cal.right(), cal.top() + 5));
        painter->drawLine(QPointF(cal.left() + 4, cal.top() - 1.5), QPointF(cal.left() + 4, cal.top() + 3));
        painter->drawLine(QPointF(cal.right() - 4, cal.top() - 1.5), QPointF(cal.right() - 4, cal.top() + 3));
    }

    painter->restore();
}
