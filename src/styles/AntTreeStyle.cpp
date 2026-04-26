#include "AntTreeStyle.h"

#include <algorithm>

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntTree.h"

namespace
{
constexpr int RowHeight = 32;
constexpr int IndentWidth = 24;
constexpr int ArrowSize = 8;
constexpr int CheckboxSize = 16;
constexpr int IconSize = 16;
constexpr int ArrowZoneWidth = 24;
constexpr int CheckboxZoneWidth = 24;
constexpr int LineDashPattern = 2;
} // namespace

AntTreeStyle::AntTreeStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntTree>();
}

void AntTreeStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntTree*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntTreeStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntTree*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntTreeStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntTree*>(widget))
    {
        drawTree(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

bool AntTreeStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* tree = qobject_cast<AntTree*>(watched);
    if (tree && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(tree);
        option.rect = tree->rect();
        QPainter painter(tree);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, tree);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntTreeStyle::drawTree(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* tree = qobject_cast<const AntTree*>(widget);
    if (!tree || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // Background
    painter->fillRect(option->rect, token.colorBgContainer);

    const QVector<FlatNode> flat = tree->flattenVisible();
    const int scrollY = tree->m_scrollY;
    const int hoveredRow = tree->m_hoveredRow;
    const QStringList selectedKeys = tree->m_selectedKeys;
    const int viewWidth = option->rect.width();

    // Determine visible row range
    const int firstVisibleRow = scrollY / RowHeight;
    const int lastVisibleRow = std::min(firstVisibleRow + (option->rect.height() / RowHeight) + 1,
                                        static_cast<int>(flat.size()) - 1);

    for (int i = firstVisibleRow; i <= lastVisibleRow && i < flat.size(); ++i)
    {
        const FlatNode& fn = flat.at(i);
        const AntTreeNode* node = fn.node;
        if (!node)
        {
            continue;
        }

        const int y = i * RowHeight - scrollY;
        const int indent = fn.depth * IndentWidth;
        const bool isHovered = (i == hoveredRow);
        const bool isSelected = selectedKeys.contains(node->key);

        // Row background
        if (isSelected)
        {
            painter->fillRect(QRect(0, y, viewWidth, RowHeight), token.colorPrimaryBg);
            // Left accent bar
            painter->setPen(Qt::NoPen);
            painter->setBrush(token.colorPrimary);
            painter->drawRoundedRect(QRectF(0, y + 4, 3, RowHeight - 8), 1.5, 1.5);
        }
        else if (isHovered && !node->disabled)
        {
            painter->fillRect(QRect(0, y, viewWidth, RowHeight), token.colorFillTertiary);
        }

        // Connecting lines (showLine mode)
        if (tree->isShowLine() && fn.depth > 0)
        {
            painter->setPen(QPen(token.colorBorderSecondary, 1, Qt::DashLine));
            const int lineX = indent - IndentWidth / 2;
            // Vertical line from parent level
            if (fn.isLastChild)
            {
                painter->drawLine(lineX, y, lineX, y + RowHeight / 2);
            }
            else
            {
                painter->drawLine(lineX, y, lineX, y + RowHeight);
            }
            // Horizontal connector
            painter->drawLine(lineX, y + RowHeight / 2, indent, y + RowHeight / 2);
        }

        int x = indent;

        // Expand arrow
        if (!node->isLeaf && !node->children.isEmpty())
        {
            const QRectF arrowRect(x + (ArrowZoneWidth - ArrowSize) / 2.0,
                                   y + (RowHeight - ArrowSize) / 2.0,
                                   ArrowSize, ArrowSize);
            painter->save();
            painter->translate(arrowRect.center());
            if (node->expanded)
            {
                painter->rotate(90);
            }
            painter->translate(-arrowRect.center());

            painter->setPen(QPen(token.colorTextSecondary, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->setBrush(Qt::NoBrush);
            QPainterPath arrow;
            arrow.moveTo(arrowRect.left() + 2, arrowRect.top());
            arrow.lineTo(arrowRect.right() - 2, arrowRect.center().y());
            arrow.lineTo(arrowRect.left() + 2, arrowRect.bottom());
            painter->drawPath(arrow);
            painter->restore();
        }
        x += ArrowZoneWidth;

        // Checkbox
        if (tree->isCheckable() && node->checkable)
        {
            const QRectF cbRect(x + (CheckboxZoneWidth - CheckboxSize) / 2.0 + 0.5,
                                y + (RowHeight - CheckboxSize) / 2.0 + 0.5,
                                CheckboxSize - 1, CheckboxSize - 1);

            if (node->checked && !node->halfChecked)
            {
                // Checked: filled primary with white checkmark
                painter->setPen(Qt::NoPen);
                painter->setBrush(token.colorPrimary);
                painter->drawRoundedRect(cbRect, token.borderRadiusSM, token.borderRadiusSM);

                QPainterPath check;
                check.moveTo(cbRect.left() + cbRect.width() * 0.28, cbRect.top() + cbRect.height() * 0.52);
                check.lineTo(cbRect.left() + cbRect.width() * 0.43, cbRect.top() + cbRect.height() * 0.68);
                check.lineTo(cbRect.left() + cbRect.width() * 0.74, cbRect.top() + cbRect.height() * 0.32);
                painter->setPen(QPen(token.colorTextLightSolid, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter->setBrush(Qt::NoBrush);
                painter->drawPath(check);
            }
            else if (node->halfChecked)
            {
                // Half-checked: border with horizontal line
                painter->setPen(QPen(token.colorPrimary, token.lineWidth));
                painter->setBrush(token.colorBgContainer);
                painter->drawRoundedRect(cbRect, token.borderRadiusSM, token.borderRadiusSM);

                painter->setPen(Qt::NoPen);
                painter->setBrush(token.colorPrimary);
                const QRectF mark(cbRect.left() + 4, cbRect.center().y() - 1.5, cbRect.width() - 8, 3);
                painter->drawRoundedRect(mark, 1.5, 1.5);
            }
            else
            {
                // Unchecked: border only
                painter->setPen(QPen(token.colorBorder, token.lineWidth));
                painter->setBrush(token.colorBgContainer);
                painter->drawRoundedRect(cbRect, token.borderRadiusSM, token.borderRadiusSM);
            }
            x += CheckboxZoneWidth;
        }

        // Icon
        if (tree->isShowIcon())
        {
            const QRect iconRect(x + 2, y + (RowHeight - IconSize) / 2, IconSize, IconSize);
            if (!node->icon.isNull())
            {
                const QPixmap pixmap = node->icon.pixmap(IconSize, IconSize);
                painter->drawPixmap(iconRect, pixmap);
            }
            x += IconSize + 4;
        }
        else
        {
            x += 4;
        }

        // Title
        QFont textFont = painter->font();
        textFont.setPixelSize(token.fontSize);
        painter->setFont(textFont);
        painter->setPen(node->disabled ? token.colorTextDisabled : token.colorText);
        const QRect textRect(x, y, viewWidth - x, RowHeight);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, node->title);
    }

    painter->restore();
}
