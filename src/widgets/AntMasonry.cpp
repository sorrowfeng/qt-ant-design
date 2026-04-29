#include "AntMasonry.h"

#include <QPalette>
#include <QResizeEvent>

#include <algorithm>

#include "core/AntTheme.h"

AntMasonry::AntMasonry(QWidget* parent)
    : QWidget(parent)
{
    updateTheme();
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateTheme();
        update();
    });
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
    if (!widget)
    {
        return;
    }
    widget->setParent(this);
    m_items.append(widget);
    widget->show();
    relayout();
}

void AntMasonry::clear()
{
    for (auto* w : m_items) w->deleteLater();
    m_items.clear();
    setMinimumHeight(0);
    updateGeometry();
}

void AntMasonry::resizeEvent(QResizeEvent*) { relayout(); }

void AntMasonry::relayout()
{
    if (m_items.isEmpty())
    {
        setMinimumHeight(0);
        updateGeometry();
        return;
    }

    const int totalSpacing = (m_columns - 1) * m_spacing;
    const int colW = qMax(0, (width() - totalSpacing) / m_columns);
    if (colW <= 0)
    {
        return;
    }

    QVector<int> colHeights(m_columns, 0);

    for (auto* w : m_items)
    {
        // Find shortest column
        int shortestCol = 0;
        for (int i = 1; i < m_columns; ++i)
            if (colHeights[i] < colHeights[shortestCol]) shortestCol = i;

        int x = shortestCol * (colW + m_spacing);
        int y = colHeights[shortestCol];

        int itemHeight = w->hasHeightForWidth() ? w->heightForWidth(colW) : w->sizeHint().height();
        itemHeight = qMax(itemHeight, w->minimumSizeHint().height());
        itemHeight = qMax(itemHeight, w->minimumHeight());
        if (w->maximumHeight() < QWIDGETSIZE_MAX)
        {
            itemHeight = qMin(itemHeight, w->maximumHeight());
        }
        itemHeight = qMax(0, itemHeight);

        w->setGeometry(x, y, colW, itemHeight);
        colHeights[shortestCol] += itemHeight + m_spacing;
    }

    int maxH = 0;
    for (int h : colHeights) maxH = std::max(maxH, h);
    if (maxH > 0)
    {
        maxH -= m_spacing;
    }
    setMinimumHeight(maxH);
    updateGeometry();
}

void AntMasonry::updateTheme()
{
    const auto& token = antTheme->tokens();
    QPalette pal = palette();
    pal.setColor(QPalette::Window, token.colorBgContainer);
    pal.setColor(QPalette::WindowText, token.colorText);
    setPalette(pal);
    setAutoFillBackground(true);
}
