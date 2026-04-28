#include "AntTagStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntTag.h"

AntTagStyle::AntTagStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntTag>();
}

void AntTagStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntTag*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntTagStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntTag*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntTagStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntTag*>(widget))
    {
        drawTag(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntTagStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntTagStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* tag = qobject_cast<AntTag*>(watched);
    if (tag && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(tag);
        option.rect = tag->rect();
        QPainter painter(tag);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, tag);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntTagStyle::drawTag(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* tag = qobject_cast<const AntTag*>(widget);
    if (!tag || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const QRectF pill = option->rect.adjusted(0.5, 1.5, -0.5, -1.5);

    // Replicate baseColor()
    const QString color = tag->color();
    QColor base;
    if (color.compare(QStringLiteral("success"), Qt::CaseInsensitive) == 0)
    {
        base = token.colorSuccess;
    }
    else if (color.compare(QStringLiteral("warning"), Qt::CaseInsensitive) == 0)
    {
        base = token.colorWarning;
    }
    else if (color.compare(QStringLiteral("error"), Qt::CaseInsensitive) == 0)
    {
        base = token.colorError;
    }
    else if (color.compare(QStringLiteral("processing"), Qt::CaseInsensitive) == 0)
    {
        base = token.colorPrimary;
    }
    else
    {
        const QColor preset = AntPalette::presetColor(color);
        if (preset.isValid())
        {
            base = preset;
        }
        else
        {
            const QColor parsed(color);
            base = parsed.isValid() ? parsed : token.colorText;
        }
    }

    const bool hasCustom = !color.trimmed().isEmpty();
    const bool hovered = option->state & QStyle::State_MouseOver;
    const bool enabled = option->state & QStyle::State_Enabled;
    const bool checkable = tag->isCheckable();
    const bool checked = tag->isChecked();
    const auto variant = tag->variant();

    // Replicate backgroundColor()
    QColor bg;
    if (checkable)
    {
        if (checked)
        {
            bg = hovered ? token.colorPrimaryHover : token.colorPrimary;
        }
        else
        {
            bg = hovered ? token.colorFillTertiary : Qt::transparent;
        }
    }
    else if (!hasCustom)
    {
        bg = token.colorFillQuaternary;
    }
    else if (variant == Ant::TagVariant::Solid)
    {
        bg = base;
    }
    else
    {
        bg = AntPalette::backgroundColor(base, antTheme->themeMode());
    }

    // Replicate borderColor()
    QColor border;
    if (checkable)
    {
        border = Qt::transparent;
    }
    else if (!hasCustom)
    {
        border = token.colorBorder;
    }
    else
    {
        border = variant == Ant::TagVariant::Solid ? base : AntPalette::borderColor(base, antTheme->themeMode());
    }

    // Replicate textColor()
    QColor textCol;
    if (!enabled)
    {
        textCol = token.colorTextDisabled;
    }
    else if (checkable && checked)
    {
        textCol = token.colorTextLightSolid;
    }
    else if (checkable)
    {
        textCol = hovered ? token.colorPrimary : token.colorText;
    }
    else if (hasCustom)
    {
        textCol = variant == Ant::TagVariant::Solid ? token.colorTextLightSolid : base;
    }
    else
    {
        textCol = token.colorText;
    }

    AntStyleBase::drawCrispRoundedRect(painter, pill.toRect(), QPen(border, token.lineWidth), bg,
        token.borderRadiusSM, token.borderRadiusSM);

    QFont f = painter->font();
    f.setPixelSize(token.fontSizeSM);
    painter->setFont(f);
    painter->setPen(textCol);

    int x = token.paddingXS;
    const QString iconText = tag->iconText();
    if (!iconText.isEmpty())
    {
        painter->drawText(QRect(x - 2, 0, 16, option->rect.height()), Qt::AlignCenter, iconText.left(2));
        x += 18;
    }
    const bool closable = tag->isClosable();
    const int rightReserve = closable ? 18 : token.paddingXXS;
    painter->drawText(QRect(x, 0, option->rect.width() - x - rightReserve, option->rect.height()),
                      Qt::AlignLeft | Qt::AlignVCenter, tag->text());

    if (closable)
    {
        const QRect close(option->rect.width() - 19, option->rect.height() / 2 - 8, 16, 16);
        AntStyleBase::drawCrispRoundedRect(painter, close, Qt::NoPen, Qt::transparent,
            token.borderRadiusXS, token.borderRadiusXS);
        painter->setPen(QPen(textCol, 1.3, Qt::SolidLine, Qt::RoundCap));
        const QPoint c = close.center();
        painter->drawLine(QPoint(c.x() - 3, c.y() - 3), QPoint(c.x() + 3, c.y() + 3));
        painter->drawLine(QPoint(c.x() + 3, c.y() - 3), QPoint(c.x() - 3, c.y() + 3));
    }

    painter->restore();
}
