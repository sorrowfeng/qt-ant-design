#include "AntMenuBarStyle.h"

#include <QApplication>
#include <QPainter>
#include <QStyleOptionMenuItem>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "widgets/AntMenuBar.h"

AntMenuBarStyle::AntMenuBarStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntMenuBar*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->update();
            }
        }
    });
}

void AntMenuBarStyle::drawControl(ControlElement element, const QStyleOption* option,
                                   QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::CE_MenuBarEmptyArea && qobject_cast<const AntMenuBar*>(widget))
    {
        painter->save();
        painter->fillRect(option->rect, antTheme->tokens().colorBgContainer);
        painter->restore();
        return;
    }

    if (element == QStyle::CE_MenuBarItem && qobject_cast<const AntMenuBar*>(widget))
    {
        const auto* menuItem = qstyleoption_cast<const QStyleOptionMenuItem*>(option);
        if (!menuItem || menuItem->text.isEmpty()) return;

        const auto& token = antTheme->tokens();
        const bool active = menuItem->state.testFlag(QStyle::State_Selected);
        const bool pressed = menuItem->state.testFlag(QStyle::State_Sunken);
        const bool enabled = menuItem->state.testFlag(QStyle::State_Enabled);
        const QRectF r = menuItem->rect;

        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        // Background
        if (active || pressed)
        {
            QColor bg = active ? token.colorFillTertiary : token.colorFillQuaternary;
            painter->setPen(Qt::NoPen);
            painter->setBrush(bg);
            painter->drawRoundedRect(r.adjusted(4, 3, -4, -3), 4, 4);
        }

        // Text
        QColor textColor = token.colorText;
        if (active) textColor = token.colorPrimary;
        if (!enabled) textColor = token.colorTextDisabled;

        QFont f = widget ? widget->font() : painter->font();
        f.setPixelSize(token.fontSize);
        painter->setFont(f);
        painter->setPen(textColor);

        QString displayText = menuItem->text;
        displayText.remove(QRegularExpression(QStringLiteral("&(?=\\S)")));

        // Icon
        qreal offsetX = 8;
        if (!menuItem->icon.isNull())
        {
            const int iconSz = static_cast<int>(r.height() * 0.65);
            const QRectF iconRect(r.left() + offsetX, r.center().y() - iconSz / 2.0, iconSz, iconSz);
            menuItem->icon.paint(painter, iconRect.toRect(), Qt::AlignCenter,
                                 enabled ? QIcon::Normal : QIcon::Disabled);
            offsetX += iconSz + 6;
        }

        painter->drawText(r.adjusted(offsetX, 0, -8, 0), Qt::AlignLeft | Qt::AlignVCenter, displayText);

        painter->restore();
        return;
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}

int AntMenuBarStyle::pixelMetric(PixelMetric metric, const QStyleOption* option,
                                  const QWidget* widget) const
{
    if (metric == QStyle::PM_MenuBarHMargin)
        return 0;
    if (metric == QStyle::PM_MenuBarItemSpacing)
        return 4;
    return QProxyStyle::pixelMetric(metric, option, widget);
}

QSize AntMenuBarStyle::sizeFromContents(ContentsType type, const QStyleOption* option,
                                         const QSize& size, const QWidget* widget) const
{
    if (type == QStyle::CT_MenuBar)
        return QSize(size.width(), widget ? widget->height() : antTheme->tokens().controlHeight);
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}
