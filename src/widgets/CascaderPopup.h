#pragma once

#include <QApplication>
#include <QFrame>
#include <QHideEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

#include <algorithm>

#include "AntCascader.h"
#include "core/AntTheme.h"
#include "styles/AntIconPainter.h"
#include "styles/AntPalette.h"

namespace
{
constexpr int kColumnWidth = 112;
constexpr int kMaxVisibleItems = 8;
constexpr int kPopupHPadding = 4;
constexpr int kPopupVPadding = 4;
constexpr int kPopupShadowMargin = 8;
constexpr int kMinPopupPanelHeight = 180;
constexpr int kOptionHPadding = 12;
constexpr int kArrowSize = 16;

struct ColumnState
{
    const QVector<AntCascaderOption>* options = nullptr;
    int highlightedIndex = -1;
    int selectedIndex = -1;
};
} // namespace

class CascaderPopup : public QFrame
{
public:
    explicit CascaderPopup(AntCascader* owner)
        : QFrame(owner, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
          m_owner(owner)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setObjectName(QStringLiteral("CascaderPopup"));
        setMouseTracking(true);
    }

    void rebuildColumns()
    {
        m_columns.clear();
        m_hoveredColumn = -1;
        m_hoveredRow = -1;

        const QVector<AntCascaderOption>* current = &m_owner->m_options;
        const QStringList& valPath = m_owner->m_value;

        for (int depth = 0; depth <= valPath.size(); ++depth)
        {
            if (!current || current->isEmpty())
            {
                break;
            }

            ColumnState col;
            col.options = current;
            col.highlightedIndex = -1;
            col.selectedIndex = -1;

            if (depth < valPath.size())
            {
                for (int i = 0; i < current->size(); ++i)
                {
                    if (current->at(i).value.toString() == valPath.at(depth))
                    {
                        col.selectedIndex = i;
                        break;
                    }
                }
            }

            m_columns.push_back(col);

            if (depth < valPath.size() && col.selectedIndex >= 0)
            {
                current = &current->at(col.selectedIndex).children;
            }
            else
            {
                current = nullptr;
            }
        }
    }

    void updateSizeAndPosition()
    {
        int columnCount = std::max(1, static_cast<int>(m_columns.size()));
        int maxItemsInAnyCol = 0;
        for (const auto& col : m_columns)
        {
            if (col.options)
            {
                maxItemsInAnyCol = std::max(maxItemsInAnyCol, static_cast<int>(col.options->size()));
            }
        }
        maxItemsInAnyCol = std::min(std::max(1, maxItemsInAnyCol), kMaxVisibleItems);

        const auto& token = antTheme->tokens();
        int optionHeight = token.controlHeight;
        if (m_owner->cascaderSize() == Ant::Size::Large)
        {
            optionHeight = token.controlHeightLG;
        }
        else if (m_owner->cascaderSize() == Ant::Size::Small)
        {
            optionHeight = token.controlHeightSM;
        }

        int popupWidth = columnCount * kColumnWidth + kPopupHPadding * 2 + kPopupShadowMargin * 2;
        int popupHeight = std::max(kMinPopupPanelHeight, maxItemsInAnyCol * optionHeight + kPopupVPadding * 2)
            + kPopupShadowMargin * 2;

        QPoint globalPos = m_owner->mapToGlobal(QPoint(0, m_owner->height() + 4));

        QScreen* screen = QApplication::screenAt(globalPos);
        if (!screen)
        {
            screen = QApplication::primaryScreen();
        }
        QRect screenGeom = screen->availableGeometry();

        if (globalPos.x() + popupWidth > screenGeom.right())
        {
            globalPos.setX(screenGeom.right() - popupWidth - 4);
        }
        if (globalPos.x() < screenGeom.left())
        {
            globalPos.setX(screenGeom.left() + 4);
        }
        if (globalPos.y() + popupHeight > screenGeom.bottom())
        {
            globalPos.setY(m_owner->mapToGlobal(QPoint(0, -popupHeight - 4)).y());
        }
        if (globalPos.y() < screenGeom.top())
        {
            globalPos.setY(screenGeom.top() + 4);
        }

        setGeometry(globalPos.x(), globalPos.y(), popupWidth, popupHeight);
    }

    void setHoveredCell(int col, int row)
    {
        if (m_hoveredColumn == col && m_hoveredRow == row)
        {
            return;
        }

        m_hoveredColumn = col;
        m_hoveredRow = row;

        if (col >= 0 && col < m_columns.size() && row >= 0 && m_columns[col].options && row < m_columns[col].options->size())
        {
            m_columns[col].highlightedIndex = row;
        }

        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)

        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

        QRect panelRect = rect().adjusted(kPopupShadowMargin, kPopupShadowMargin, -kPopupShadowMargin, -kPopupShadowMargin);
        antTheme->drawEffectShadow(&painter, panelRect, 10, token.borderRadiusLG, 0.55);

        painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter.setBrush(token.colorBgElevated);
        painter.drawRoundedRect(panelRect, token.borderRadiusLG, token.borderRadiusLG);

        QRect contentRect = panelRect.adjusted(kPopupHPadding, kPopupVPadding, -kPopupHPadding, -kPopupVPadding);

        int optionHeight = token.controlHeight;
        int fontSize = token.fontSize;
        if (m_owner->cascaderSize() == Ant::Size::Large)
        {
            optionHeight = token.controlHeightLG;
            fontSize = token.fontSizeLG;
        }
        else if (m_owner->cascaderSize() == Ant::Size::Small)
        {
            optionHeight = token.controlHeightSM;
            fontSize = token.fontSizeSM;
        }

        int colX = contentRect.left();
        for (int colIdx = 0; colIdx < m_columns.size(); ++colIdx)
        {
            const ColumnState& col = m_columns.at(colIdx);
            if (!col.options || col.options->isEmpty())
            {
                break;
            }

            int colRight = colX + kColumnWidth;

            if (colIdx > 0)
            {
                painter.setPen(QPen(token.colorSplit, token.lineWidth));
                painter.drawLine(QPointF(colX - 0.5, contentRect.top()),
                                 QPointF(colX - 0.5, contentRect.bottom()));
            }

            for (int rowIdx = 0; rowIdx < col.options->size(); ++rowIdx)
            {
                const AntCascaderOption& opt = col.options->at(rowIdx);
                QRect itemRect(colX, contentRect.top() + rowIdx * optionHeight, kColumnWidth, optionHeight);

                bool isSelected = col.selectedIndex == rowIdx;
                bool isHighlighted = col.highlightedIndex == rowIdx;
                bool isDisabled = opt.disabled;

                QRectF bgRect = QRectF(itemRect).adjusted(4, 2, -4, -2);

                if (isSelected)
                {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(AntPalette::alpha(token.colorPrimary,
                                                       antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.24 : 0.12));
                    painter.drawRoundedRect(bgRect, token.borderRadiusSM, token.borderRadiusSM);
                }
                else if (isHighlighted && !isDisabled)
                {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(token.colorFillQuaternary);
                    painter.drawRoundedRect(bgRect, token.borderRadiusSM, token.borderRadiusSM);
                }

                QFont f = painter.font();
                f.setPixelSize(fontSize);
                f.setWeight(isSelected ? QFont::DemiBold : QFont::Normal);
                painter.setFont(f);

                QColor textColor = isDisabled ? token.colorTextDisabled : token.colorText;
                painter.setPen(textColor);

                bool hasChildren = !opt.isLeaf && !opt.children.isEmpty();
                int rightPad = kOptionHPadding;
                if (hasChildren)
                {
                    rightPad = kArrowSize + kOptionHPadding;
                }

                QRect textRect = itemRect.adjusted(kOptionHPadding, 0, -rightPad, 0);
                painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, opt.label);

                if (hasChildren)
                {
                    const QRectF arrowRect(itemRect.right() - kOptionHPadding - kArrowSize,
                                           itemRect.center().y() - kArrowSize / 2.0,
                                           kArrowSize,
                                           kArrowSize);
                    AntIconPainter::drawIcon(painter,
                                             Ant::IconType::Right,
                                             arrowRect,
                                             isDisabled ? token.colorTextDisabled : token.colorTextTertiary);
                }
            }

            colX = colRight;
        }
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        const auto& token = antTheme->tokens();
        int optionHeight = token.controlHeight;
        if (m_owner->cascaderSize() == Ant::Size::Large)
        {
            optionHeight = token.controlHeightLG;
        }
        else if (m_owner->cascaderSize() == Ant::Size::Small)
        {
            optionHeight = token.controlHeightSM;
        }

        QRect panelRect = rect().adjusted(kPopupShadowMargin, kPopupShadowMargin, -kPopupShadowMargin, -kPopupShadowMargin);
        QRect contentRect = panelRect.adjusted(kPopupHPadding, kPopupVPadding, -kPopupHPadding, -kPopupVPadding);

        QPoint pos = event->pos();

        if (!contentRect.contains(pos))
        {
            setHoveredCell(-1, -1);
            return;
        }

        int relX = pos.x() - contentRect.left();
        int relY = pos.y() - contentRect.top();
        int colIdx = relX / kColumnWidth;
        int rowIdx = relY / optionHeight;

        if (colIdx < 0 || colIdx >= m_columns.size())
        {
            setHoveredCell(-1, -1);
            return;
        }

        const ColumnState& col = m_columns.at(colIdx);
        if (!col.options || rowIdx < 0 || rowIdx >= col.options->size())
        {
            setHoveredCell(-1, -1);
            return;
        }

        setHoveredCell(colIdx, rowIdx);

        if (m_owner->expandTrigger() == Ant::Trigger::Hover)
        {
            handleColumnExpand(colIdx, rowIdx);
        }

        QFrame::mouseMoveEvent(event);
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() != Qt::LeftButton)
        {
            QFrame::mousePressEvent(event);
            return;
        }

        const auto& token = antTheme->tokens();
        int optionHeight = token.controlHeight;
        if (m_owner->cascaderSize() == Ant::Size::Large)
        {
            optionHeight = token.controlHeightLG;
        }
        else if (m_owner->cascaderSize() == Ant::Size::Small)
        {
            optionHeight = token.controlHeightSM;
        }

        QRect panelRect = rect().adjusted(kPopupShadowMargin, kPopupShadowMargin, -kPopupShadowMargin, -kPopupShadowMargin);
        QRect contentRect = panelRect.adjusted(kPopupHPadding, kPopupVPadding, -kPopupHPadding, -kPopupVPadding);

        QPoint pos = event->pos();
        if (!contentRect.contains(pos))
        {
            return;
        }

        int relX = pos.x() - contentRect.left();
        int relY = pos.y() - contentRect.top();
        int colIdx = relX / kColumnWidth;
        int rowIdx = relY / optionHeight;

        if (colIdx < 0 || colIdx >= m_columns.size())
        {
            return;
        }

        const ColumnState& col = m_columns.at(colIdx);
        if (!col.options || rowIdx < 0 || rowIdx >= col.options->size())
        {
            return;
        }

        const AntCascaderOption& opt = col.options->at(rowIdx);
        if (opt.disabled)
        {
            return;
        }

        handleColumnClick(colIdx, rowIdx);
        event->accept();
    }

    void hideEvent(QHideEvent* event) override
    {
        if (m_owner && m_owner->isOpen())
        {
            m_owner->setOpen(false);
        }
        QFrame::hideEvent(event);
    }

private:
    void handleColumnClick(int colIdx, int rowIdx)
    {
        if (colIdx < 0 || colIdx >= m_columns.size())
        {
            return;
        }

        const ColumnState& col = m_columns.at(colIdx);
        if (!col.options || rowIdx < 0 || rowIdx >= col.options->size())
        {
            return;
        }

        const AntCascaderOption& opt = col.options->at(rowIdx);

        QStringList newPath;
        for (int i = 0; i < colIdx; ++i)
        {
            const ColumnState& prevCol = m_columns.at(i);
            if (prevCol.selectedIndex >= 0 && prevCol.options)
            {
                newPath.push_back(prevCol.options->at(prevCol.selectedIndex).value.toString());
            }
        }
        newPath.push_back(opt.value.toString());

        if (opt.isLeaf || opt.children.isEmpty())
        {
            m_owner->setValue(newPath);
            m_owner->setOpen(false);
        }
        else
        {
            m_columns.resize(colIdx + 1);
            m_columns[colIdx].selectedIndex = rowIdx;
            m_columns[colIdx].highlightedIndex = -1;

            ColumnState newCol;
            newCol.options = &opt.children;
            newCol.highlightedIndex = -1;
            newCol.selectedIndex = -1;
            m_columns.push_back(newCol);

            m_owner->m_value = newPath;
            emit m_owner->valueChanged(m_owner->m_value);

            updateSizeAndPosition();
            update();
        }
    }

    void handleColumnExpand(int colIdx, int rowIdx)
    {
        if (colIdx < 0 || colIdx >= m_columns.size())
        {
            return;
        }

        const ColumnState& col = m_columns.at(colIdx);
        if (!col.options || rowIdx < 0 || rowIdx >= col.options->size())
        {
            return;
        }

        if (col.selectedIndex == rowIdx)
        {
            return;
        }

        const AntCascaderOption& opt = col.options->at(rowIdx);
        if (opt.disabled || opt.isLeaf || opt.children.isEmpty())
        {
            return;
        }

        handleColumnClick(colIdx, rowIdx);
    }

    AntCascader* m_owner = nullptr;
    QVector<ColumnState> m_columns;
    int m_hoveredColumn = -1;
    int m_hoveredRow = -1;
};
