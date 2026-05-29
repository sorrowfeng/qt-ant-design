#include "AntDockStyle.h"

#include <QMainWindow>
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QSplitter>
#include <QStyleOption>
#include <QStyleOptionComplex>
#include <QStyleOptionTab>
#include <QStyleOptionToolButton>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>

#include "core/AntTheme.h"
#include "widgets/AntDockManager.h"

namespace
{
constexpr int kDockTabRadius = 6;
constexpr int kDockTabHPadding = 14;
constexpr int kDockTabVPadding = 6;
constexpr int kDockTabMinHeight = 36;

QColor dockBorderColor()
{
    const auto& token = antTheme->tokens();
    return antTheme->themeMode() == Ant::ThemeMode::Dark ? token.colorBorderSecondary : token.colorSplit;
}

QColor dockInactiveTabColor()
{
    const auto& token = antTheme->tokens();
    return antTheme->themeMode() == Ant::ThemeMode::Dark ? token.colorBgElevated : token.colorBgContainer;
}

QColor dockHoverColor()
{
    const auto& token = antTheme->tokens();
    return antTheme->themeMode() == Ant::ThemeMode::Dark ? token.colorFillTertiary : token.colorFillQuaternary;
}

QPainterPath topRoundedPath(const QRectF& rect, qreal radius)
{
    QPainterPath path;
    const qreal r = qMin(radius, qMin(rect.width(), rect.height()) / 2.0);
    path.moveTo(rect.left(), rect.bottom());
    path.lineTo(rect.left(), rect.top() + r);
    path.quadTo(rect.left(), rect.top(), rect.left() + r, rect.top());
    path.lineTo(rect.right() - r, rect.top());
    path.quadTo(rect.right(), rect.top(), rect.right(), rect.top() + r);
    path.lineTo(rect.right(), rect.bottom());
    path.closeSubpath();
    return path;
}

QPolygonF arrowPolygon(Qt::ArrowType arrow, const QRectF& rect)
{
    const QPointF center = rect.center();
    const qreal half = qMin(rect.width(), rect.height()) * 0.28;
    switch (arrow)
    {
    case Qt::LeftArrow:
        return {QPointF(center.x() - half, center.y()),
                QPointF(center.x() + half, center.y() - half),
                QPointF(center.x() + half, center.y() + half)};
    case Qt::RightArrow:
        return {QPointF(center.x() + half, center.y()),
                QPointF(center.x() - half, center.y() - half),
                QPointF(center.x() - half, center.y() + half)};
    case Qt::UpArrow:
        return {QPointF(center.x(), center.y() - half),
                QPointF(center.x() - half, center.y() + half),
                QPointF(center.x() + half, center.y() + half)};
    case Qt::DownArrow:
    default:
        return {QPointF(center.x(), center.y() + half),
                QPointF(center.x() - half, center.y() - half),
                QPointF(center.x() + half, center.y() - half)};
    }
}

bool hasDockManagerAncestor(const QWidget* widget)
{
    const QWidget* current = widget;
    while (current)
    {
        if (qobject_cast<const AntDockManager*>(current))
        {
            return true;
        }
        current = current->parentWidget();
    }
    return false;
}
} // namespace

AntDockStyle::AntDockStyle(QStyle* style)
    : QProxyStyle(style)
{
}

void AntDockStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                                 QPainter* painter, const QWidget* widget) const
{
    if (option && painter && isDockManagedWidget(widget))
    {
        switch (element)
        {
        case QStyle::PE_Widget:
            if (qobject_cast<const AntDockManager*>(widget) ||
                qobject_cast<const QTabBar*>(widget) ||
                qobject_cast<const QTabWidget*>(widget))
            {
                painter->fillRect(option->rect, antTheme->tokens().colorBgLayout);
                return;
            }
            break;
        case QStyle::PE_FrameTabWidget:
        case QStyle::PE_FrameTabBarBase:
            drawDockTabPane(option, painter, widget);
            return;
        case QStyle::PE_PanelButtonTool:
            if (qobject_cast<const QToolButton*>(widget))
            {
                QStyleOptionToolButton toolOption;
                toolOption.rect = option->rect;
                toolOption.state = option->state;
                if (const auto* button = qobject_cast<const QToolButton*>(widget))
                {
                    toolOption.arrowType = button->arrowType();
                    toolOption.icon = button->icon();
                    toolOption.text = button->text();
                    toolOption.fontMetrics = button->fontMetrics();
                }
                drawDockToolButton(&toolOption, painter, widget);
                return;
            }
            break;
        default:
            break;
        }
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void AntDockStyle::drawControl(ControlElement element, const QStyleOption* option,
                               QPainter* painter, const QWidget* widget) const
{
    if (option && painter && isDockManagedWidget(widget))
    {
        switch (element)
        {
        case QStyle::CE_TabBarTab:
            drawDockTab(option, painter, widget);
            return;
        case QStyle::CE_TabBarTabShape:
            drawDockTabShape(option, painter, widget);
            return;
        case QStyle::CE_TabBarTabLabel:
            drawDockTabLabel(option, painter, widget);
            return;
        case QStyle::CE_Splitter:
            drawDockSplitter(option, painter, widget);
            return;
        case QStyle::CE_ToolButtonLabel:
            if (const auto* toolOption = qstyleoption_cast<const QStyleOptionToolButton*>(option))
            {
                drawDockToolButton(toolOption, painter, widget);
                return;
            }
            break;
        default:
            break;
        }
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

void AntDockStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                                      QPainter* painter, const QWidget* widget) const
{
    if (control == QStyle::CC_ToolButton && option && painter && isDockManagedWidget(widget) &&
        qobject_cast<const QToolButton*>(widget))
    {
        drawDockToolButton(option, painter, widget);
        return;
    }
    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

int AntDockStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    if (isDockManagedWidget(widget))
    {
        switch (metric)
        {
        case QStyle::PM_SplitterWidth:
            return 4;
        case QStyle::PM_TabBarTabHSpace:
            return kDockTabHPadding * 2;
        case QStyle::PM_TabBarTabVSpace:
            return kDockTabVPadding * 2;
        default:
            break;
        }
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

QSize AntDockStyle::sizeFromContents(ContentsType type, const QStyleOption* option,
                                     const QSize& size, const QWidget* widget) const
{
    QSize result = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == QStyle::CT_TabBarTab && isDockManagedWidget(widget))
    {
        result.rwidth() += kDockTabHPadding * 2;
        result.rheight() += kDockTabVPadding * 2;
        result.setHeight(qMax(result.height(), kDockTabMinHeight));
    }
    return result;
}

bool AntDockStyle::isDockManagedWidget(const QWidget* widget) const
{
    return hasDockManagerAncestor(widget);
}

void AntDockStyle::drawDockTabPane(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    Q_UNUSED(widget)
    const auto& token = antTheme->tokens();
    const QRectF pane = QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(QPen(dockBorderColor(), token.lineWidth));
    painter->setBrush(token.colorBgContainer);
    painter->drawRect(pane);
    painter->restore();
}

void AntDockStyle::drawDockTab(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    drawDockTabShape(option, painter, widget);
    drawDockTabLabel(option, painter, widget);
}

void AntDockStyle::drawDockTabShape(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    Q_UNUSED(widget)
    const auto* tabOption = qstyleoption_cast<const QStyleOptionTab*>(option);
    if (!tabOption)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const bool selected = tabOption->state.testFlag(QStyle::State_Selected);
    const bool hovered = tabOption->state.testFlag(QStyle::State_MouseOver);
    QRectF rect = QRectF(tabOption->rect).adjusted(0.5, 0.5, -2.5, -0.5);
    if (!rect.isValid())
    {
        return;
    }

    QColor bg = selected ? token.colorBgContainer : dockInactiveTabColor();
    if (!selected && hovered)
    {
        bg = dockHoverColor();
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const QPainterPath path = topRoundedPath(rect, kDockTabRadius);
    painter->fillPath(path, bg);
    painter->setPen(QPen(dockBorderColor(), token.lineWidth));
    painter->drawPath(path);

    if (selected)
    {
        painter->setPen(QPen(token.colorPrimary, 2));
        painter->drawLine(QPointF(rect.left() + kDockTabRadius, rect.top() + 0.5),
                          QPointF(rect.right() - kDockTabRadius, rect.top() + 0.5));
        painter->setPen(QPen(token.colorBgContainer, token.lineWidth + 1));
        painter->drawLine(QPointF(rect.left() + 1, rect.bottom()),
                          QPointF(rect.right() - 1, rect.bottom()));
    }

    painter->restore();
}

void AntDockStyle::drawDockTabLabel(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    Q_UNUSED(widget)
    const auto* tabOption = qstyleoption_cast<const QStyleOptionTab*>(option);
    if (!tabOption)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const bool selected = tabOption->state.testFlag(QStyle::State_Selected);
    const bool hovered = tabOption->state.testFlag(QStyle::State_MouseOver);
    const bool enabled = tabOption->state.testFlag(QStyle::State_Enabled);
    QColor textColor = selected || hovered ? token.colorText : token.colorTextSecondary;
    if (!enabled)
    {
        textColor = token.colorTextDisabled;
    }

    QRect textRect = tabOption->rect.adjusted(kDockTabHPadding, kDockTabVPadding, -kDockTabHPadding - 2, -kDockTabVPadding);
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing);
    if (widget)
    {
        painter->setFont(widget->font());
    }
    painter->setPen(textColor);

    if (!tabOption->icon.isNull())
    {
        const int iconSize = qMin(16, textRect.height());
        const QRect iconRect(textRect.left(), textRect.center().y() - iconSize / 2, iconSize, iconSize);
        tabOption->icon.paint(painter, iconRect, Qt::AlignCenter,
                              enabled ? QIcon::Normal : QIcon::Disabled,
                              selected ? QIcon::On : QIcon::Off);
        textRect.setLeft(iconRect.right() + 6);
    }

    const QString text = tabOption->fontMetrics.elidedText(tabOption->text, Qt::ElideRight, textRect.width());
    painter->drawText(textRect, Qt::AlignCenter | Qt::TextSingleLine, text);
    painter->restore();
}

void AntDockStyle::drawDockToolButton(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const
{
    const auto* button = qobject_cast<const QToolButton*>(widget);
    const auto* toolOption = qstyleoption_cast<const QStyleOptionToolButton*>(option);
    if (!button || !toolOption)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const bool hovered = toolOption->state.testFlag(QStyle::State_MouseOver);
    const bool pressed = toolOption->state.testFlag(QStyle::State_Sunken);
    const bool enabled = toolOption->state.testFlag(QStyle::State_Enabled);
    QRectF rect = QRectF(toolOption->rect).adjusted(2.5, 2.5, -2.5, -2.5);
    if (!rect.isValid())
    {
        return;
    }

    QColor bg = pressed ? token.colorFillQuaternary : (hovered ? dockHoverColor() : dockInactiveTabColor());
    QColor text = enabled ? token.colorText : token.colorTextDisabled;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter->setPen(QPen(dockBorderColor(), token.lineWidth));
    painter->setBrush(bg);
    painter->drawRoundedRect(rect, token.borderRadiusSM, token.borderRadiusSM);

    if (toolOption->arrowType != Qt::NoArrow)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(text);
        painter->drawPolygon(arrowPolygon(toolOption->arrowType, rect.adjusted(4, 4, -4, -4)));
    }
    else if (!toolOption->icon.isNull())
    {
        const int iconSize = qMin(16, qMin(toolOption->rect.width(), toolOption->rect.height()) - 6);
        const QRect iconRect(QPoint(0, 0), QSize(iconSize, iconSize));
        toolOption->icon.paint(painter, iconRect.translated(toolOption->rect.center() - iconRect.center()),
                               Qt::AlignCenter, enabled ? QIcon::Normal : QIcon::Disabled);
    }
    else if (!toolOption->text.isEmpty())
    {
        painter->setPen(text);
        painter->drawText(toolOption->rect.adjusted(4, 0, -4, 0),
                          Qt::AlignCenter | Qt::TextSingleLine,
                          toolOption->fontMetrics.elidedText(toolOption->text, Qt::ElideRight, toolOption->rect.width() - 8));
    }

    painter->restore();
}

void AntDockStyle::drawDockSplitter(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    Q_UNUSED(widget)
    const auto& token = antTheme->tokens();
    const bool hovered = option->state.testFlag(QStyle::State_MouseOver);
    painter->fillRect(option->rect, hovered ? dockHoverColor() : token.colorBgLayout);
}
