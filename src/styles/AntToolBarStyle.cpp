#include "AntToolBarStyle.h"

#include <QApplication>
#include <QPainter>
#include <QStyleOption>

#include "core/AntTheme.h"
#include "widgets/AntToolBar.h"

AntToolBarStyle::AntToolBarStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntToolBar*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->update();
            }
        }
    });
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
            painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
            painter->setBrush(token.colorBgElevated);
            painter->drawRoundedRect(bg, 6, 6);
        }

        painter->restore();
        return;
    }
    QProxyStyle::drawControl(element, option, painter, widget);
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
