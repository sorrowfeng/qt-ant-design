#include "AntMasonry.h"

#include <QResizeEvent>

AntMasonry::AntMasonry(QWidget* parent)
    : QWidget(parent)
{
}

int AntMasonry::columns() const { return m_columns; }
void AntMasonry::setColumns(int cols)
{
    cols = std::max(1, cols);
    if (m_columns == cols) return;
    m_columns = cols;
    relayout();
    Q_EMIT columnsChanged(m_columns);
}

int AntMasonry::spacing() const { return m_spacing; }
void AntMasonry::setSpacing(int px)
{
    px = std::max(0, px);
    if (m_spacing == px) return;
    m_spacing = px;
    relayout();
    Q_EMIT spacingChanged(m_spacing);
}

void AntMasonry::addWidget(QWidget* widget)
{
    widget->setParent(this);
    m_items.append(widget);
    widget->show();
    relayout();
}

void AntMasonry::clear()
{
    for (auto* w : m_items) w->deleteLater();
    m_items.clear();
}

void AntMasonry::resizeEvent(QResizeEvent*) { relayout(); }

void AntMasonry::relayout()
{
    if (m_items.isEmpty()) return;

    const int totalSpacing = (m_columns - 1) * m_spacing;
    const int colW = (width() - totalSpacing) / m_columns;

    QVector<int> colHeights(m_columns, 0);

    for (auto* w : m_items)
    {
        // Find shortest column
        int shortestCol = 0;
        for (int i = 1; i < m_columns; ++i)
            if (colHeights[i] < colHeights[shortestCol]) shortestCol = i;

        int x = shortestCol * (colW + m_spacing);
        int y = colHeights[shortestCol];

        w->setGeometry(x, y, colW, w->sizeHint().height());
        colHeights[shortestCol] += w->height() + m_spacing;
    }

    int maxH = 0;
    for (int h : colHeights) maxH = std::max(maxH, h);
    setMinimumHeight(maxH);
}
