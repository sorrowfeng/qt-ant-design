#include "AntGrid.h"

#include <QGridLayout>
#include <QVBoxLayout>

// ---- AntCol ----

AntCol::AntCol(int span, QWidget* parent)
    : QWidget(parent), m_span(span)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

int AntCol::span() const { return m_span; }
void AntCol::setSpan(int span)
{
    span = qBound(1, span, 24);
    if (m_span == span) return;
    m_span = span;
    Q_EMIT spanChanged(m_span);
}

int AntCol::offset() const { return m_offset; }
void AntCol::setOffset(int offset)
{
    offset = qBound(0, offset, 23);
    if (m_offset == offset) return;
    m_offset = offset;
    Q_EMIT offsetChanged(m_offset);
}

// ---- AntRow ----

AntRow::AntRow(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_grid = new QGridLayout(this);
    m_grid->setContentsMargins(0, 0, 0, 0);
    m_grid->setSpacing(0);
    ensureColumnStretch();
    syncPerfCounters();
}

int AntRow::gutter() const { return m_gutter; }
void AntRow::setGutter(int px)
{
    if (m_gutter == px) return;
    m_gutter = px;
    m_grid->setSpacing(px);
    updateGeometry();
    Q_EMIT gutterChanged(m_gutter);
}

void AntRow::addWidget(QWidget* widget, int span, int offset)
{
    span = qBound(1, span, 24);
    offset = qBound(0, offset, 23);

    auto* col = new AntCol(span, this);
    auto* colLayout = new QVBoxLayout(col);
    colLayout->setContentsMargins(0, 0, 0, 0);
    colLayout->addWidget(widget);

    ensureColumnStretch();
    const QPoint placement = appendPlacement(span, offset);
    m_cols.append({col, span, offset, placement.y(), placement.x()});
    m_grid->addWidget(col, placement.y(), placement.x(), 1, span);
    col->show();
    updateGeometry();
    syncPerfCounters();
}

void AntRow::relayout()
{
    QLayoutItem* item;
    while ((item = m_grid->takeAt(0)) != nullptr)
    {
        if (item->widget()) item->widget()->hide();
        delete item;
    }

    m_currentRow = 0;
    m_currentCol = 0;
    for (auto& ci : m_cols)
    {
        const QPoint placement = appendPlacement(ci.span, ci.offset);
        ci.row = placement.y();
        ci.column = placement.x();
        m_grid->addWidget(ci.widget, ci.row, ci.column, 1, ci.span);
        ci.widget->show();
    }

    ++m_relayoutCount;
    updateGeometry();
    syncPerfCounters();
}

void AntRow::ensureColumnStretch()
{
    if (m_columnStretchInitialized) return;
    for (int i = 0; i < 24; ++i)
        m_grid->setColumnStretch(i, 1);
    m_columnStretchInitialized = true;
}

QPoint AntRow::appendPlacement(int span, int offset)
{
    int start = m_currentCol + offset;
    if (start + span > 24)
    {
        ++m_currentRow;
        m_currentCol = 0;
        start = offset;
    }

    const QPoint placement(start, m_currentRow);
    m_currentCol = start + span;
    if (m_currentCol >= 24)
    {
        ++m_currentRow;
        m_currentCol = 0;
    }

    ++m_placementBuildCount;
    return placement;
}

void AntRow::syncPerfCounters()
{
    setProperty("antGridPlacementBuildCount", m_placementBuildCount);
    setProperty("antGridRelayoutCount", m_relayoutCount);
    setProperty("antGridColumnStretchInitialized", m_columnStretchInitialized);
}
