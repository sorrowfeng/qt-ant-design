#include "AntTimeline.h"

#include <QMouseEvent>

#include "../styles/AntTimelineStyle.h"
#include "core/AntTheme.h"

AntTimeline::AntTimeline(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntTimelineStyle(style()));
}

Ant::TimelineMode AntTimeline::mode() const { return m_mode; }

void AntTimeline::setMode(Ant::TimelineMode mode)
{
    if (m_mode == mode)
    {
        return;
    }
    m_mode = mode;
    update();
    Q_EMIT modeChanged(m_mode);
}

Ant::TimelineOrientation AntTimeline::orientation() const { return m_orientation; }

void AntTimeline::setOrientation(Ant::TimelineOrientation orientation)
{
    if (m_orientation == orientation)
    {
        return;
    }
    m_orientation = orientation;
    updateGeometry();
    update();
    Q_EMIT orientationChanged(m_orientation);
}

Ant::TimelineDotVariant AntTimeline::dotVariant() const { return m_dotVariant; }

void AntTimeline::setDotVariant(Ant::TimelineDotVariant variant)
{
    if (m_dotVariant == variant)
    {
        return;
    }
    m_dotVariant = variant;
    update();
    Q_EMIT dotVariantChanged(m_dotVariant);
}

bool AntTimeline::isReverse() const { return m_reverse; }

void AntTimeline::setReverse(bool reverse)
{
    if (m_reverse == reverse)
    {
        return;
    }
    m_reverse = reverse;
    update();
    Q_EMIT reverseChanged(m_reverse);
}

int AntTimeline::count() const { return m_items.size(); }

AntTimelineItem AntTimeline::itemAt(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }
    return m_items.at(index);
}

void AntTimeline::addItem(const AntTimelineItem& item)
{
    m_items.push_back(item);
    updateGeometry();
    update();
}

void AntTimeline::addItem(const QString& title, const QString& content,
                          const QString& color, bool loading)
{
    addItem({title, content, color, loading});
}

void AntTimeline::insertItem(int index, const AntTimelineItem& item)
{
    if (index < 0 || index > m_items.size())
    {
        return;
    }
    m_items.insert(index, item);
    updateGeometry();
    update();
}

void AntTimeline::removeItem(int index)
{
    if (index < 0 || index >= m_items.size())
    {
        return;
    }
    m_items.remove(index);
    updateGeometry();
    update();
}

void AntTimeline::clearItems()
{
    m_items.clear();
    updateGeometry();
    update();
}

QSize AntTimeline::sizeHint() const
{
    if (m_orientation == Ant::TimelineOrientation::Vertical)
    {
        const int itemHeight = 80;
        return QSize(320, qMax(80, m_items.size() * itemHeight));
    }
    const int itemWidth = 120;
    return QSize(qMax(240, m_items.size() * itemWidth), 160);
}

QSize AntTimeline::minimumSizeHint() const
{
    return QSize(200, 80);
}

void AntTimeline::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntTimeline::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && !m_items.isEmpty())
    {
        int index = -1;
        if (m_orientation == Ant::TimelineOrientation::Vertical)
        {
            const int itemHeight = 80;
            const int topMargin = 16;
            const int row = (event->pos().y() - topMargin) / itemHeight;
            if (row >= 0 && row < m_items.size())
            {
                index = m_reverse ? (m_items.size() - 1 - row) : row;
            }
        }
        else
        {
            const int itemWidth = 120;
            const int leftMargin = 16;
            const int col = (event->pos().x() - leftMargin) / itemWidth;
            if (col >= 0 && col < m_items.size())
            {
                index = m_reverse ? (m_items.size() - 1 - col) : col;
            }
        }

        if (index >= 0 && index < m_items.size())
        {
            Q_EMIT itemClicked(index);
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}
