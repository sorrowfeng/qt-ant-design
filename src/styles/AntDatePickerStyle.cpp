#include "AntDatePickerStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntIconPainter.h"
#include "styles/AntPalette.h"
#include "widgets/AntDatePicker.h"

AntDatePickerStyle::AntDatePickerStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntDatePicker>();
}

void AntDatePickerStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
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
    AntStyleBase::unpolish(widget);
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

void AntDatePickerStyle::drawDatePicker(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* picker = qobject_cast<const AntDatePicker*>(widget);
    if (!picker || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const auto m = picker->metrics();
    const QRectF control = picker->controlRect();
    const bool focused = picker->hasFocus() || picker->isOpen();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // Focus ring
    if (focused && picker->isEnabled() &&
        picker->variant() != Ant::Variant::Borderless &&
        picker->variant() != Ant::Variant::Underlined)
    {
        AntStyleBase::drawCrispRoundedRect(painter, control.adjusted(-1, -1, 1, 1).toRect(),
            QPen(AntPalette::alpha(picker->borderColor(), 0.16), token.controlOutlineWidth),
            Qt::NoBrush, m.radius + 1, m.radius + 1);
    }

    // Border and background
    if (picker->variant() != Ant::Variant::Borderless &&
        picker->variant() != Ant::Variant::Underlined)
    {
        const bool filled = picker->variant() == Ant::Variant::Filled;
        const QPen framePen = filled && !focused
            ? Qt::NoPen
            : QPen(picker->borderColor(), token.lineWidth);
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(),
            framePen,
            picker->backgroundColor(), m.radius, m.radius);
    }
    else
    {
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(), Qt::NoPen,
            picker->backgroundColor(), m.radius, m.radius);
        if (picker->variant() == Ant::Variant::Underlined)
        {
            painter->setPen(QPen(picker->borderColor(), focused ? 2 : token.lineWidth));
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
        AntIconPainter::drawIcon(*painter,
                                 Ant::IconType::Right,
                                 arrowRect.adjusted(4, 4, -4, -4),
                                 token.colorTextDisabled);
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
    const QRectF icon = picker->iconRect(m);
    if (picker->canClear())
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
