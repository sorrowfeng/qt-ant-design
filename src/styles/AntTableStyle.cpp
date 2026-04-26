#include "AntTableStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntTable.h"

AntTableStyle::AntTableStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntTable>();
}

void AntTableStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntTable*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntTableStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntTable*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntTableStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntTable*>(widget))
    {
        drawTable(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntTableStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntTableStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* table = qobject_cast<AntTable*>(watched);
    if (table && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(table);
        option.rect = table->rect();
        QPainter painter(table);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, table);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

namespace
{
struct TableMetrics
{
    int headerHeight = 40;
    int rowHeight = 48;
    int selectionColWidth = 48;
    int cellHPadding = 12;
    int cellVPadding = 12;
    int fontSize = 14;
    int fontSizeSM = 12;
};

TableMetrics tableMetrics(Ant::Size size)
{
    TableMetrics m;
    const auto& token = antTheme->tokens();
    switch (size)
    {
    case Ant::Size::Large:
        m.headerHeight = 48;
        m.rowHeight = 54;
        m.cellVPadding = 16;
        m.fontSize = token.fontSizeLG;
        m.fontSizeSM = token.fontSize;
        break;
    case Ant::Size::Small:
        m.headerHeight = 32;
        m.rowHeight = 40;
        m.cellVPadding = 8;
        m.fontSize = token.fontSize;
        m.fontSizeSM = token.fontSizeSM;
        break;
    default: // Middle
        m.headerHeight = 40;
        m.rowHeight = 48;
        m.cellVPadding = 12;
        m.fontSize = token.fontSize;
        m.fontSizeSM = token.fontSizeSM;
        break;
    }
    return m;
}

int totalColWidth(const QVector<AntTableColumn>& columns)
{
    int w = 0;
    for (const auto& col : columns)
    {
        if (!col.hidden)
        {
            w += qMax(col.minWidth, col.width);
        }
    }
    return w;
}

int calcBodyHeight(const AntTable* table, const TableMetrics& m)
{
    const bool hasPagination = table->rowCount() > table->pageSize();
    const int paginationH = hasPagination ? 48 : 0;
    return table->height() - m.headerHeight - paginationH;
}

int pageStart(const AntTable* table)
{
    return (table->currentPage() - 1) * table->pageSize();
}

int pageEnd(const AntTable* table)
{
    return qMin(table->currentPage() * table->pageSize(), table->rowCount());
}

bool isAllPageSelected(const AntTable* table)
{
    const int start = pageStart(table);
    const int end = pageEnd(table);
    for (int i = start; i < end; ++i)
    {
        const AntTableRow row = table->rowAt(i);
        if (!row.disabled && !row.selected)
        {
            return false;
        }
    }
    return start < end;
}

bool hasAnyPageSelected(const AntTable* table)
{
    const int start = pageStart(table);
    const int end = pageEnd(table);
    for (int i = start; i < end; ++i)
    {
        if (table->rowAt(i).selected)
        {
            return true;
        }
    }
    return false;
}

void drawCheckbox(QPainter* painter, const QRect& rect, bool checked, bool partial,
                  const QColor& borderColor, const QColor& fillColor, const QColor& checkColor)
{
    painter->save();
    const int r = 2;

    if (checked || partial)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(fillColor);
        painter->drawRoundedRect(rect, r, r);
    }
    else
    {
        painter->setPen(QPen(borderColor, 1.5));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(rect.adjusted(0, 0, -1, -1), r, r);
    }

    if (checked)
    {
        painter->setPen(QPen(checkColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        const int cx = rect.center().x();
        const int cy = rect.center().y();
        painter->drawLine(QPoint(cx - 4, cy), QPoint(cx - 1, cy + 3));
        painter->drawLine(QPoint(cx - 1, cy + 3), QPoint(cx + 4, cy - 3));
    }
    else if (partial)
    {
        painter->setPen(QPen(checkColor, 2, Qt::SolidLine, Qt::RoundCap));
        const int cy = rect.center().y();
        painter->drawLine(QPoint(rect.left() + 3, cy), QPoint(rect.right() - 3, cy));
    }

    painter->restore();
}

void drawRadio(QPainter* painter, const QRect& rect, bool checked,
               const QColor& borderColor, const QColor& fillColor, const QColor& dotColor)
{
    painter->save();
    const QRect circleRect(rect.left(), rect.top(), 16, 16);

    if (checked)
    {
        painter->setPen(QPen(fillColor, 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(circleRect);
        const QRect inner = circleRect.adjusted(4, 4, -4, -4);
        painter->setPen(Qt::NoPen);
        painter->setBrush(dotColor);
        painter->drawEllipse(inner);
    }
    else
    {
        painter->setPen(QPen(borderColor, 1.5));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(circleRect.adjusted(0, 0, -1, -1));
    }

    painter->restore();
}

Qt::AlignmentFlag alignFlag(Ant::TableColumnAlign align)
{
    switch (align)
    {
    case Ant::TableColumnAlign::Center:
        return Qt::AlignHCenter;
    case Ant::TableColumnAlign::Right:
        return Qt::AlignRight;
    default:
        return Qt::AlignLeft;
    }
}

} // namespace

void AntTableStyle::drawTable(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* table = qobject_cast<const AntTable*>(widget);
    if (!table || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const TableMetrics m = tableMetrics(table->tableSize());
    const QVector<AntTableColumn> columns = table->columns();
    const bool hasSelection = table->rowSelection() != Ant::TableSelectionMode::None;
    const int selW = hasSelection ? m.selectionColWidth : 0;
    const int bHeight = calcBodyHeight(table, m);
    const bool hasPagination = table->rowCount() > table->pageSize();
    const int totalH = m.headerHeight + bHeight + (hasPagination ? 48 : 0);
    const int scrollOffset = table->scrollY();
    const int hoveredRow = table->hoveredRow();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // ─── Background ───
    painter->setPen(Qt::NoPen);
    painter->setBrush(token.colorBgContainer);
    painter->drawRect(option->rect);

    // ─── Header ───
    const QRect headerRect(0, 0, option->rect.width(), m.headerHeight);

    // Header background
    painter->setPen(Qt::NoPen);
    painter->setBrush(token.colorFillQuaternary);
    painter->drawRect(headerRect);

    // Header bottom border
    painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
    painter->drawLine(headerRect.bottomLeft(), headerRect.bottomRight());

    // Selection column header
    if (hasSelection)
    {
        const QRect selHeaderRect(0, 0, selW, m.headerHeight);
        if (table->rowSelection() == Ant::TableSelectionMode::Checkbox)
        {
            const QRect cbRect(selHeaderRect.center().x() - 8, selHeaderRect.center().y() - 8, 16, 16);
            const bool allSelected = isAllPageSelected(table);
            const bool anySelected = hasAnyPageSelected(table);
            const bool partial = anySelected && !allSelected;
            drawCheckbox(painter, cbRect, allSelected, partial,
                         token.colorBorder, token.colorPrimary, token.colorTextLightSolid);
        }

        // Separator after selection column
        painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter->drawLine(QPoint(selW, 4), QPoint(selW, m.headerHeight - 4));
    }

    // Column headers
    QFont headerFont = painter->font();
    headerFont.setPixelSize(m.fontSize);
    headerFont.setWeight(QFont::DemiBold);
    painter->setFont(headerFont);
    const QFontMetrics headerFm(headerFont);

    int colX = selW;
    for (const auto& col : columns)
    {
        if (col.hidden)
        {
            continue;
        }
        const int colW = qMax(col.minWidth, col.width);
        const Qt::AlignmentFlag hAlign = alignFlag(col.align);

        if (col.sorter)
        {
            const bool isActive = (table->currentSortColumn() == col.key
                                   && table->sortOrder() != Ant::TableSortOrder::None);
            const Ant::TableSortOrder activeOrder = isActive ? table->sortOrder() : Ant::TableSortOrder::None;

            const int arrowW = 10;
            const int arrowGap = 4;
            const int titleTextW = headerFm.horizontalAdvance(col.title);
            int titleStartX;
            int arrowX;

            if (hAlign == Qt::AlignRight)
            {
                arrowX = colX + colW - m.cellHPadding - arrowW;
                titleStartX = arrowX - arrowGap - titleTextW;
            }
            else if (hAlign == Qt::AlignHCenter)
            {
                const int totalW = titleTextW + arrowGap + arrowW;
                titleStartX = colX + (colW - totalW) / 2;
                arrowX = titleStartX + titleTextW + arrowGap;
            }
            else
            {
                titleStartX = colX + m.cellHPadding;
                arrowX = titleStartX + titleTextW + arrowGap;
            }

            // Title text
            painter->setPen(token.colorText);
            painter->drawText(QRect(titleStartX, 0, titleTextW, m.headerHeight),
                              Qt::AlignVCenter | Qt::AlignLeft, col.title);

            // Sort arrows
            const int arrowH = 8;
            const int arrowTop = (m.headerHeight - arrowH) / 2;

            // Up arrow
            {
                const QRect upRect(arrowX, arrowTop, arrowW, arrowH / 2);
                const bool upActive = (activeOrder == Ant::TableSortOrder::Ascending);
                painter->setPen(upActive ? token.colorPrimary : token.colorTextTertiary);
                const int cx = upRect.center().x();
                painter->drawLine(cx, upRect.top(), cx - 3, upRect.bottom());
                painter->drawLine(cx, upRect.top(), cx + 3, upRect.bottom());
            }
            // Down arrow
            {
                const QRect downRect(arrowX, arrowTop + arrowH / 2, arrowW, arrowH / 2);
                const bool downActive = (activeOrder == Ant::TableSortOrder::Descending);
                painter->setPen(downActive ? token.colorPrimary : token.colorTextTertiary);
                const int cx = downRect.center().x();
                painter->drawLine(cx, downRect.bottom(), cx - 3, downRect.top());
                painter->drawLine(cx, downRect.bottom(), cx + 3, downRect.top());
            }
        }
        else
        {
            // Plain title
            const int textLeft = colX + m.cellHPadding;
            const int textMaxW = colW - m.cellHPadding * 2;
            painter->setPen(token.colorText);
            painter->drawText(QRect(textLeft, 0, textMaxW, m.headerHeight),
                              Qt::AlignVCenter | hAlign, col.title);
        }

        // Column separator
        painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter->drawLine(QPoint(colX + colW, 4), QPoint(colX + colW, m.headerHeight - 4));

        colX += colW;
    }

    // ─── Body ───
    const QRect bodyRect(0, m.headerHeight, option->rect.width(), bHeight);
    painter->setClipRect(bodyRect);

    const int start = pageStart(table);
    const int end = pageEnd(table);

    for (int i = start; i < end; ++i)
    {
        const int localIdx = i - start;
        const int rowY = m.headerHeight + localIdx * m.rowHeight - scrollOffset;

        // Skip rows outside visible area
        if (rowY + m.rowHeight <= m.headerHeight || rowY >= m.headerHeight + bHeight)
        {
            continue;
        }

        const QRect rowRect(0, rowY, option->rect.width(), m.rowHeight);
        const AntTableRow row = table->rowAt(i);

        // Alternating row background (bordered mode)
        if (table->isBordered() && (localIdx % 2 == 1))
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(token.colorFillQuaternary);
            painter->drawRect(rowRect);
        }

        // Hover background
        if (i == hoveredRow && !row.disabled)
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(token.colorFillTertiary);
            painter->drawRect(rowRect);
        }

        // Selected row background
        if (row.selected)
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(token.colorPrimaryBg);
            painter->drawRect(rowRect);
        }

        // Disabled row text color
        const QColor textColor = row.disabled ? token.colorTextDisabled : token.colorText;

        // Selection column cell
        if (hasSelection)
        {
            const QRect selCellRect(0, rowY, selW, m.rowHeight);
            const QRect cbRect(selCellRect.center().x() - 8, selCellRect.center().y() - 8, 16, 16);

            if (table->rowSelection() == Ant::TableSelectionMode::Checkbox)
            {
                drawCheckbox(painter, cbRect, row.selected, false,
                             token.colorBorder, token.colorPrimary, token.colorTextLightSolid);
            }
            else if (table->rowSelection() == Ant::TableSelectionMode::Radio)
            {
                drawRadio(painter, cbRect, row.selected,
                          token.colorBorder, token.colorPrimary, token.colorPrimary);
            }
        }

        // Data cells
        int cellX = selW;
        QFont cellFont = painter->font();
        cellFont.setPixelSize(m.fontSize);
        cellFont.setWeight(QFont::Normal);
        painter->setFont(cellFont);

        for (const auto& col : columns)
        {
            if (col.hidden)
            {
                continue;
            }
            const int colW = qMax(col.minWidth, col.width);
            const QRect cellRect(cellX + m.cellHPadding, rowY, colW - m.cellHPadding * 2, m.rowHeight);
            const Qt::AlignmentFlag hAlign = alignFlag(col.align);

            painter->setPen(textColor);
            const QString cellText = row.data.value(col.dataIndex).toString();
            painter->drawText(cellRect, Qt::AlignVCenter | hAlign, cellText);

            cellX += colW;
        }

        // Row bottom border
        painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter->drawLine(QPoint(0, rowY + m.rowHeight), QPoint(option->rect.width(), rowY + m.rowHeight));
    }

    // Empty state
    if (table->rowCount() == 0 && !table->isLoading())
    {
        const bool isDark = antTheme->themeMode() == Ant::ThemeMode::Dark;
        const QColor primary = AntPalette::alpha(token.colorTextTertiary, isDark ? 0.5 : 0.32);
        const QColor fill = AntPalette::alpha(token.colorFillQuaternary, isDark ? 0.78 : 1.0);
        const QColor line = AntPalette::alpha(token.colorTextTertiary, isDark ? 0.68 : 0.45);

        const int imgW = 96;
        const int imgH = 60;
        const int centerX = bodyRect.center().x();
        const int centerY = bodyRect.center().y() - 14;
        const QRectF imgRect(centerX - imgW / 2.0, centerY - imgH / 2.0, imgW, imgH);

        painter->save();
        painter->translate(imgRect.topLeft());
        painter->scale(imgW / 128.0, imgH / 80.0);

        painter->setPen(Qt::NoPen);
        painter->setBrush(AntPalette::alpha(token.colorPrimary, 0.08));
        painter->drawEllipse(QRectF(16, 58, 96, 14));

        painter->setBrush(fill);
        painter->drawRoundedRect(QRectF(34, 10, 60, 46), 10, 10);
        painter->setBrush(AntPalette::alpha(token.colorBgContainer, 0.88));
        painter->drawRoundedRect(QRectF(42, 18, 44, 30), 6, 6);

        painter->setPen(QPen(primary, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawArc(QRectF(10, 20, 26, 26), 35 * 16, 260 * 16);
        painter->drawArc(QRectF(92, 18, 22, 22), 220 * 16, 220 * 16);

        painter->setPen(QPen(line, 2.2, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(49, 27), QPointF(79, 27));
        painter->drawLine(QPointF(49, 34), QPointF(73, 34));
        painter->drawLine(QPointF(49, 41), QPointF(67, 41));
        painter->restore();

        QFont descFont = painter->font();
        descFont.setPixelSize(token.fontSize);
        painter->setFont(descFont);
        painter->setPen(token.colorTextSecondary);
        const QRect descRect(0, centerY + imgH / 2 + 8, option->rect.width(), token.fontSize + 8);
        painter->drawText(descRect, Qt::AlignHCenter | Qt::AlignTop, QStringLiteral("No Data"));
    }

    painter->setClipping(false);

    // Vertical column separators in body
    if (table->isBordered())
    {
        painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        int sepX = selW;
        for (const auto& col : columns)
        {
            if (col.hidden)
            {
                continue;
            }
            const int colW = qMax(col.minWidth, col.width);
            painter->drawLine(QPoint(sepX + colW, m.headerHeight),
                              QPoint(sepX + colW, m.headerHeight + bHeight));
            sepX += colW;
        }
    }

    // ─── Pagination ───
    if (hasPagination)
    {
        const int paginationTop = m.headerHeight + bHeight;
        const QRect paginationRect(0, paginationTop, option->rect.width(), 48);

        // Pagination background
        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorBgContainer);
        painter->drawRect(paginationRect);

        // Top border
        painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter->drawLine(paginationRect.topLeft(), paginationRect.topRight());

        // "Showing X-Y of Z" text
        QFont pageFont = painter->font();
        pageFont.setPixelSize(m.fontSizeSM);
        pageFont.setWeight(QFont::Normal);
        painter->setFont(pageFont);

        const int startRow = start + 1;
        const int endRow = qMin(end, table->rowCount());
        const int totalRows = table->rowCount();
        const QString pageText = QString::fromLatin1("Showing %1-%2 of %3")
                                     .arg(startRow)
                                     .arg(endRow)
                                     .arg(totalRows);

        painter->setPen(token.colorTextSecondary);
        const QRect textRect(16, paginationTop, 200, 48);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, pageText);

        // Page number buttons
        const int totalPages = table->totalPages();
        const int currentPage = table->currentPage();
        const int btnSize = 32;
        const int btnGap = 4;
        const int btnsWidth = totalPages * (btnSize + btnGap) - btnGap;
        int btnX = option->rect.width() - 16 - btnsWidth;

        for (int p = 1; p <= totalPages; ++p)
        {
            const QRect btnRect(btnX, paginationTop + (48 - btnSize) / 2, btnSize, btnSize);
            const bool isCurrent = (p == currentPage);

            if (isCurrent)
            {
                painter->setPen(Qt::NoPen);
                painter->setBrush(token.colorPrimary);
                painter->drawRoundedRect(btnRect, 4, 4);
            }
            else
            {
                painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
                painter->setBrush(Qt::NoBrush);
                painter->drawRoundedRect(btnRect, 4, 4);
            }

            QFont btnFont = painter->font();
            btnFont.setPixelSize(m.fontSizeSM);
            btnFont.setWeight(isCurrent ? QFont::DemiBold : QFont::Normal);
            painter->setFont(btnFont);
            painter->setPen(isCurrent ? token.colorTextLightSolid : token.colorText);
            painter->drawText(btnRect, Qt::AlignCenter, QString::number(p));

            btnX += btnSize + btnGap;
        }
    }

    // ─── Loading overlay ───
    if (table->isLoading())
    {
        const QRect overlayRect(0, 0, option->rect.width(), m.headerHeight + bHeight);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 255, 255, 160));
        painter->drawRect(overlayRect);

        // Spinner arc
        const int spinnerSize = 32;
        const QRect spinnerRect(
            overlayRect.center().x() - spinnerSize / 2,
            overlayRect.center().y() - spinnerSize / 2,
            spinnerSize,
            spinnerSize);

        painter->setPen(QPen(token.colorPrimary, 3, Qt::SolidLine, Qt::RoundCap));
        painter->setBrush(Qt::NoBrush);
        painter->drawArc(spinnerRect, 30 * 16, 300 * 16);
    }

    // ─── Outer border ───
    if (table->isBordered())
    {
        painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(0, 0, option->rect.width() - 1, totalH - 1);
    }

    painter->restore();
}
