#include "AntToolBarStyle.h"

#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>
#include <QStyleOptionToolButton>
#include <QToolButton>
#include <QVariant>

#include "widgets/AntToolBar.h"

namespace
{
struct ToolBarButtonMetrics
{
    int height = 32;
    int fontSize = 14;
    int paddingX = 15;
    int iconSize = 14;
    int radius = 6;
};

bool isToolBarButton(const QWidget* widget)
{
    const auto* button = qobject_cast<const QToolButton*>(widget);
    return button && button->property("antToolBarButton").toBool();
}

ToolBarButtonMetrics buttonMetrics()
{
    const auto& token = antTheme->tokens();
    ToolBarButtonMetrics metrics;
    metrics.height = token.controlHeight;
    metrics.fontSize = token.fontSize;
    metrics.paddingX = token.paddingSM + token.lineWidth * 3;
    metrics.iconSize = 14;
    metrics.radius = token.borderRadius;
    return metrics;
}

int focusPaddingForToolBarButton()
{
    const auto& token = antTheme->tokens();
    return token.lineWidthFocus + 1;
}

QRect centeredTotalButtonRect(const QRect& rect, const ToolBarButtonMetrics& metrics)
{
    const int focusPadding = focusPaddingForToolBarButton();
    const int totalHeight = metrics.height + focusPadding * 2;
    if (rect.height() <= totalHeight)
    {
        return rect;
    }
    const int top = rect.top() + (rect.height() - totalHeight) / 2;
    return QRect(rect.left(), top, rect.width(), totalHeight);
}

QString buttonTextMetricKey(const QToolButton* button, const QFont& font)
{
    return button->text() + QLatin1Char('|') + font.toString();
}

int cachedButtonTextWidth(const QToolButton* button, const QFont& font)
{
    if (!button)
    {
        return 0;
    }

    const QString key = buttonTextMetricKey(button, font);
    const QVariant cachedWidth = button->property("antToolBarButtonTextMetricWidth");
    auto* mutableButton = const_cast<QToolButton*>(button);
    if (button->property("antToolBarButtonTextMetricKey").toString() == key &&
        cachedWidth.isValid())
    {
        mutableButton->setProperty("antToolBarButtonTextMetricHitCount",
                                   button->property("antToolBarButtonTextMetricHitCount").toInt() + 1);
        return cachedWidth.toInt();
    }

    const int width = QFontMetrics(font).horizontalAdvance(button->text());
    mutableButton->setProperty("antToolBarButtonTextMetricKey", key);
    mutableButton->setProperty("antToolBarButtonTextMetricWidth", width);
    mutableButton->setProperty("antToolBarButtonTextMetricBuildCount",
                               button->property("antToolBarButtonTextMetricBuildCount").toInt() + 1);
    return width;
}

QColor loadingColor(const QColor& color)
{
    QColor result = color;
    result.setAlphaF(result.alphaF() * 0.65);
    return result;
}

QColor shadowColorForToolBarButton()
{
    return antTheme->tokens().colorFillQuaternary;
}

void drawButtonBottomShadow(QPainter& painter, const QRectF& outer, int radius, const QColor& color)
{
    if (color.alpha() == 0)
    {
        return;
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(outer.adjusted(0, 2, 0, 2), radius, radius);
    painter.restore();
}

void drawButtonFocusOutline(QPainter& painter, const QRectF& bodyRect, int radius)
{
    const auto& token = antTheme->tokens();
    const qreal offset = 1.0;
    const qreal width = token.lineWidthFocus;
    const qreal expand = offset + width / 2.0;
    const QRectF focusRect = bodyRect.adjusted(-expand, -expand, expand, expand);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(token.colorPrimaryBorder, width, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(focusRect, radius + expand, radius + expand);
    painter.restore();
}
} // namespace

AntToolBarStyle::AntToolBarStyle(QStyle* style)
    : AntStyleBase(style)
{
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

            antTheme->drawEffectShadow(painter, bg.toAlignedRect(), shadowBorder, 6, 0.15);

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
        Q_UNUSED(option)
        Q_UNUSED(size)
        const auto* button = qobject_cast<const QToolButton*>(widget);
        const ToolBarButtonMetrics metrics = buttonMetrics();
        const int focusPadding = focusPaddingForToolBarButton();

        QFont font = button ? button->font() : QFont();
        font.setPixelSize(metrics.fontSize);
        int width = button ? cachedButtonTextWidth(button, font) : 0;
        if (button && !button->icon().isNull())
        {
            width += metrics.iconSize + 8;
        }
        width += metrics.paddingX * 2;
        return QSize(qMax(width, metrics.height) + focusPadding * 2,
                     metrics.height + focusPadding * 2);
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
    const ToolBarButtonMetrics metrics = buttonMetrics();
    const int focusPadding = focusPaddingForToolBarButton();
    const QRect total = centeredTotalButtonRect(toolOption->rect, metrics);
    const QRectF outer = QRectF(total).adjusted(focusPadding, focusPadding, -focusPadding, -focusPadding);
    const bool enabled = toolOption->state.testFlag(QStyle::State_Enabled);
    const bool hovered = enabled && toolOption->state.testFlag(QStyle::State_MouseOver);
    const bool pressed = enabled && (toolOption->state.testFlag(QStyle::State_Sunken) || button->isDown());
    const bool focused = enabled && toolOption->state.testFlag(QStyle::State_HasFocus);

    QColor bg = token.colorBgContainer;
    QColor border = token.colorBorder;
    QColor text = enabled ? token.colorText : token.colorTextDisabled;
    if (hovered)
    {
        border = text = token.colorPrimaryHover;
    }
    if (pressed)
    {
        border = text = token.colorPrimaryActive;
    }
    if (!enabled)
    {
        bg = token.colorBgContainerDisabled;
        border = token.colorBorderDisabled;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (enabled && !pressed)
    {
        drawButtonBottomShadow(*painter, outer, metrics.radius, shadowColorForToolBarButton());
    }

    AntStyleBase::drawCrispRoundedRect(painter, outer.toRect(),
        QPen(border, token.lineWidth),
        bg, metrics.radius, metrics.radius);

    if (focused)
    {
        drawButtonFocusOutline(*painter, outer, metrics.radius);
    }

    QFont font = button->font();
    font.setPixelSize(metrics.fontSize);
    painter->setFont(font);
    painter->setPen(text);

    QRectF textRect = QRect(outer.toRect()).adjusted(metrics.paddingX, 0, -metrics.paddingX, 0);
    const QString label = button->text();
    cachedButtonTextWidth(button, font);
    const bool hasIcon = !button->icon().isNull();

    if (hasIcon)
    {
        QRectF iconRect;
        if (label.isEmpty())
        {
            iconRect = QRectF(textRect.center().x() - metrics.iconSize / 2.0,
                              textRect.center().y() - metrics.iconSize / 2.0,
                              metrics.iconSize,
                              metrics.iconSize);
        }
        else
        {
            iconRect = QRectF(textRect.left(),
                              textRect.center().y() - metrics.iconSize / 2.0,
                              metrics.iconSize,
                              metrics.iconSize);
            textRect.adjust(metrics.iconSize + 8, 0, 0, 0);
        }
        button->icon().paint(painter,
                             iconRect.toRect(),
                             Qt::AlignCenter,
                             enabled ? QIcon::Normal : QIcon::Disabled,
                             button->isChecked() ? QIcon::On : QIcon::Off);
    }

    if (!label.isEmpty())
    {
        painter->drawText(textRect, Qt::AlignCenter | Qt::TextSingleLine, label);
    }

    painter->restore();
}
