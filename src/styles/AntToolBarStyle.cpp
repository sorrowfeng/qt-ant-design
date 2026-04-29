#include "AntToolBarStyle.h"

#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>
#include <QStyleOptionToolButton>
#include <QToolButton>

#include "widgets/AntToolBar.h"

namespace
{
struct ToolBarButtonMetrics
{
    int height = 32;
    int paddingX = 10;
    int iconSize = 16;
    int radius = 4;
};

bool isToolBarButton(const QWidget* widget)
{
    const auto* button = qobject_cast<const QToolButton*>(widget);
    return button && button->property("antToolBarButton").toBool();
}

QRect centeredButtonRect(const QRect& rect, const ToolBarButtonMetrics& metrics)
{
    if (rect.height() <= metrics.height)
    {
        return rect;
    }
    const int top = rect.top() + (rect.height() - metrics.height) / 2;
    return QRect(rect.left(), top, rect.width(), metrics.height);
}
} // namespace

AntToolBarStyle::AntToolBarStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntToolBar>();
}

void AntToolBarStyle::onThemeUpdate(QWidget* w)
{
    w->update();
}

void AntToolBarStyle::drawControl(ControlElement element, const QStyleOption* option,
                                   QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::CE_ToolBar && qobject_cast<const AntToolBar*>(widget))
    {
        const auto* tb = qobject_cast<const AntToolBar*>(widget);
        if (!tb) return;

        painter->save();

        if (tb->isFloating())
        {
            const auto& token = antTheme->tokens();
            const int shadowBorder = 6;
            const QRectF bg = option->rect.adjusted(shadowBorder, shadowBorder, -shadowBorder, -shadowBorder);

            antTheme->drawEffectShadow(painter, option->rect, shadowBorder, 6, 0.15);

            painter->setRenderHint(QPainter::Antialiasing);
            AntStyleBase::drawCrispRoundedRect(painter, bg.toRect(),
                QPen(token.colorBorderSecondary, token.lineWidth),
                token.colorBgElevated, 6, 6);
        }

        painter->restore();
        return;
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

void AntToolBarStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                                         QPainter* painter, const QWidget* widget) const
{
    if (control == QStyle::CC_ToolButton && isToolBarButton(widget))
    {
        drawToolBarButton(option, painter, widget);
        return;
    }
    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

QSize AntToolBarStyle::sizeFromContents(ContentsType type, const QStyleOption* option,
                                        const QSize& size, const QWidget* widget) const
{
    if (type == QStyle::CT_ToolButton && isToolBarButton(widget))
    {
        const auto& token = antTheme->tokens();
        const auto* button = qobject_cast<const QToolButton*>(widget);
        ToolBarButtonMetrics metrics;

        QFont font = button ? button->font() : QFont();
        font.setPixelSize(token.fontSize);
        int width = button ? QFontMetrics(font).horizontalAdvance(button->text()) : size.width();
        if (button && !button->icon().isNull())
        {
            width += metrics.iconSize + 6;
        }
        width += metrics.paddingX * 2;
        return QSize(qMax(width, metrics.height), metrics.height);
    }
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

void AntToolBarStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                                     QPainter* painter, const QWidget* widget) const
{
    if (!qobject_cast<const AntToolBar*>(widget))
    {
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        return;
    }

    if (element == QStyle::PE_IndicatorToolBarHandle)
    {
        const auto& token = antTheme->tokens();
        painter->save();
        painter->setPen(QPen(token.colorTextTertiary, 2));
        const QRectF r = option->rect;
        const bool horizontal = r.width() < r.height();
        const qreal cx = r.center().x();
        const qreal cy = r.center().y();
        if (horizontal)
        {
            for (int i = -4; i <= 4; i += 4)
                painter->drawPoint(QPointF(cx, cy + i));
        }
        else
        {
            for (int i = -4; i <= 4; i += 4)
                painter->drawPoint(QPointF(cx + i, cy));
        }
        painter->restore();
        return;
    }

    if (element == QStyle::PE_IndicatorToolBarSeparator)
    {
        const auto& token = antTheme->tokens();
        painter->save();
        painter->setPen(QPen(token.colorSplit, 1));
        const QRectF r = option->rect;
        const bool horizontal = r.width() < r.height();
        if (horizontal)
            painter->drawLine(QPointF(r.center().x(), r.top() + 4), QPointF(r.center().x(), r.bottom() - 4));
        else
            painter->drawLine(QPointF(r.left() + 4, r.center().y()), QPointF(r.right() - 4, r.center().y()));
        painter->restore();
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

int AntToolBarStyle::pixelMetric(PixelMetric metric, const QStyleOption* option,
                                  const QWidget* widget) const
{
    if (qobject_cast<const AntToolBar*>(widget))
    {
        if (metric == QStyle::PM_ToolBarIconSize) return 20;
        if (metric == QStyle::PM_ToolBarHandleExtent) return 8;
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

void AntToolBarStyle::drawToolBarButton(const QStyleOptionComplex* option, QPainter* painter,
                                        const QWidget* widget) const
{
    const auto* button = qobject_cast<const QToolButton*>(widget);
    const auto* toolOption = qstyleoption_cast<const QStyleOptionToolButton*>(option);
    if (!button || !toolOption || !painter)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const ToolBarButtonMetrics metrics;
    const QRect outer = centeredButtonRect(toolOption->rect, metrics);
    const bool enabled = toolOption->state.testFlag(QStyle::State_Enabled);
    const bool hovered = enabled && toolOption->state.testFlag(QStyle::State_MouseOver);
    const bool pressed = enabled && (toolOption->state.testFlag(QStyle::State_Sunken) || button->isDown());

    QColor bg = QColor(Qt::transparent);
    QColor text = enabled ? token.colorText : token.colorTextDisabled;
    if (hovered)
    {
        bg = token.colorFillTertiary;
    }
    if (pressed)
    {
        bg = token.colorFillSecondary;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    AntStyleBase::drawCrispRoundedRect(painter, outer, Qt::NoPen, bg, metrics.radius, metrics.radius);

    QFont font = button->font();
    font.setPixelSize(token.fontSize);
    painter->setFont(font);
    painter->setPen(text);

    const QString label = button->text();
    const QFontMetrics fm(font);
    const int textWidth = fm.horizontalAdvance(label);
    const bool hasIcon = !button->icon().isNull();
    const int contentWidth = textWidth + (hasIcon ? metrics.iconSize + 6 : 0);
    int cursorX = outer.left() + qMax(metrics.paddingX, (outer.width() - contentWidth) / 2);

    if (hasIcon)
    {
        const QRect iconRect(cursorX, outer.center().y() - metrics.iconSize / 2, metrics.iconSize, metrics.iconSize);
        button->icon().paint(painter, iconRect, Qt::AlignCenter, enabled ? QIcon::Normal : QIcon::Disabled);
        cursorX += metrics.iconSize + 6;
    }

    const QRect textRect(cursorX, outer.top(), qMax(0, outer.right() - cursorX - metrics.paddingX + 1), outer.height());
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, label);

    painter->restore();
}
