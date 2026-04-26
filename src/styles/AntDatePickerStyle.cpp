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
    if (!picker->isEnabled())
    {
        return token.colorBorderDisabled;
    }
    const bool focused = picker->hasFocus() || picker->isOpen();
    if (picker->status() == Ant::Status::Error)
    {
        return focused ? token.colorErrorHover : token.colorError;
    }
    if (picker->status() == Ant::Status::Warning)
    {
        return focused ? token.colorWarningHover : token.colorWarning;
    }
    if (focused)
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
        return token.colorFillQuaternary;
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
    return picker->isEnabled() && picker->allowClear() && picker->hasSelectedDate();
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
        painter->setPen(QPen(AntPalette::alpha(datePickerBorderColor(picker), 0.16), token.controlOutlineWidth));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(control.adjusted(-1, -1, 1, 1), m.radius + 1, m.radius + 1);
    }

    // Border and background
    if (picker->variant() != Ant::Variant::Borderless &&
        picker->variant() != Ant::Variant::Underlined)
    {
        painter->setPen(QPen(datePickerBorderColor(picker), token.lineWidth));
        painter->setBrush(datePickerBackgroundColor(picker));
        painter->drawRoundedRect(control.adjusted(0.5, 0.5, -0.5, -0.5), m.radius, m.radius);
    }
    else
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(datePickerBackgroundColor(picker));
        painter->drawRoundedRect(control, m.radius, m.radius);
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

    const QString text = picker->hasSelectedDate() ? picker->dateString() : picker->placeholderText();
    QColor textColor = picker->hasSelectedDate() ? token.colorText : token.colorTextPlaceholder;
    if (!picker->isEnabled())
    {
        textColor = token.colorTextDisabled;
    }
    painter->setPen(textColor);
    painter->drawText(control.adjusted(m.paddingX, 0, -(m.iconWidth + m.paddingX), 0),
                     Qt::AlignVCenter | Qt::AlignLeft, text);

    // Icon area
    const QRectF icon = datePickerIconRect(picker);
    if (datePickerCanClear(picker))
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorBgBase);
        painter->drawEllipse(icon.adjusted(5, 5, -5, -5));
        painter->setPen(QPen(token.colorTextTertiary, 1.5, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(icon.center() + QPointF(-4, -4), icon.center() + QPointF(4, 4));
        painter->drawLine(icon.center() + QPointF(4, -4), icon.center() + QPointF(-4, 4));
    }
    else
    {
        painter->setPen(QPen(picker->isEnabled() ? token.colorTextTertiary : token.colorTextDisabled,
                            1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        QRectF cal = icon.adjusted(7, 6, -7, -6);
        painter->drawRoundedRect(cal, 2, 2);
        painter->drawLine(QPointF(cal.left(), cal.top() + 5), QPointF(cal.right(), cal.top() + 5));
        painter->drawLine(QPointF(cal.left() + 4, cal.top() - 2), QPointF(cal.left() + 4, cal.top() + 3));
        painter->drawLine(QPointF(cal.right() - 4, cal.top() - 2), QPointF(cal.right() - 4, cal.top() + 3));
    }

    painter->restore();
}
