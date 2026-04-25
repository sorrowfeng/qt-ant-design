#include "AntGrid.h"

#include <QGridLayout>

// ---- AntCol ----

AntCol::AntCol(int span, QWidget* parent)
    : QWidget(parent), m_span(span)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
    m_grid = new QGridLayout(this);
    m_grid->setContentsMargins(0, 0, 0, 0);
    m_grid->setSpacing(0);
}

int AntRow::gutter() const { return m_gutter; }
void AntRow::setGutter(int px)
{
    if (m_gutter == px) return;
    m_gutter = px;
    m_grid->setSpacing(px);
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

    m_cols.append({col, span, offset});
    relayout();
}

void AntRow::relayout()
{
    // Clear grid
    QLayoutItem* item;
    while ((item = m_grid->takeAt(0)) != nullptr)
    {
        if (item->widget()) item->widget()->hide();
        delete item;
    }

    int currentCol = 0;
    int row = 0;
    for (const auto& ci : m_cols)
    {
        int start = currentCol + ci.offset;
        if (start + ci.span > 24) { ++row; currentCol = 0; start = ci.offset; }
        m_grid->addWidget(ci.widget, row, start, 1, ci.span);
        currentCol = start + ci.span;
        if (currentCol >= 24) { ++row; currentCol = 0; }
        ci.widget->show();
    }

    // Set column stretch
    for (int i = 0; i < 24; ++i)
        m_grid->setColumnStretch(i, 1);
}
