#include "AntSteps.h"

#include <QMouseEvent>
#include <QPainter>

#include "../styles/AntStepsStyle.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"

AntSteps::AntSteps(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntStepsStyle>(this);
    setMouseTracking(true);
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateStepsGeometry();
        update();
    });
    syncStepsPerfCounters();
}

int AntSteps::currentIndex() const { return m_currentIndex; }

void AntSteps::setCurrentIndex(int index)
{
    index = qBound(0, index, qMax(0, m_steps.size() - 1));
    if (m_currentIndex == index)
    {
        return;
    }
    const int previous = m_currentIndex;
    m_currentIndex = index;
    updateStepRangeRegion(previous, m_currentIndex);
    Q_EMIT currentIndexChanged(m_currentIndex);
}

Ant::Orientation AntSteps::direction() const { return m_direction; }

void AntSteps::setDirection(Ant::Orientation direction)
{
    if (m_direction == direction)
    {
        return;
    }
    m_direction = direction;
    updateStepsGeometry();
    update();
    Q_EMIT directionChanged(m_direction);
}

bool AntSteps::isClickable() const { return m_clickable; }

void AntSteps::setClickable(bool clickable)
{
    if (m_clickable == clickable)
    {
        return;
    }
    m_clickable = clickable;
    update();
    Q_EMIT clickableChanged(m_clickable);
}

int AntSteps::count() const { return m_steps.size(); }

AntStepItem AntSteps::stepAt(int index) const
{
    if (index < 0 || index >= m_steps.size())
    {
        return {};
    }
    return m_steps.at(index);
}

void AntSteps::addStep(const AntStepItem& step)
{
    m_steps.push_back(step);
    updateStepsGeometry();
    update();
}

void AntSteps::addStep(const QString& title,
                       const QString& description,
                       const QString& subTitle,
                       Ant::StepStatus status,
                       bool disabled)
{
    addStep({title, description, subTitle, status, disabled});
}

void AntSteps::setStepStatus(int index, Ant::StepStatus status)
{
    if (index < 0 || index >= m_steps.size())
    {
        return;
    }
    if (m_steps[index].status == status)
    {
        return;
    }
    m_steps[index].status = status;
    updateStepStateRegion(index, index);
}

void AntSteps::clearSteps()
{
    m_steps.clear();
    m_currentIndex = 0;
    m_hoveredIndex = -1;
    updateStepsGeometry();
    update();
}

QSize AntSteps::sizeHint() const
{
    const Metrics m = metrics();
    if (m_direction == Ant::Orientation::Vertical)
    {
        return QSize(420, qMax(120, m_steps.size() * 92));
    }
    return QSize(qMax(320, m_steps.size() * 220), 120);
}

QSize AntSteps::minimumSizeHint() const
{
    return QSize(220, 96);
}

void AntSteps::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntSteps::mouseMoveEvent(QMouseEvent* event)
{
    const int index = stepAtPos(event->pos());
    if (m_hoveredIndex != index)
    {
        const int previous = m_hoveredIndex;
        m_hoveredIndex = index;
        updateStepStateRegion(previous, m_hoveredIndex);
    }
    QWidget::mouseMoveEvent(event);
}

void AntSteps::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_clickable)
    {
        const int index = stepAtPos(event->pos());
        if (index >= 0 && index < m_steps.size() && !m_steps.at(index).disabled)
        {
            setCurrentIndex(index);
            Q_EMIT stepClicked(index);
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void AntSteps::leaveEvent(QEvent* event)
{
    if (m_hoveredIndex != -1)
    {
        const int previous = m_hoveredIndex;
        m_hoveredIndex = -1;
        updateStepStateRegion(previous, m_hoveredIndex);
    }
    QWidget::leaveEvent(event);
}

void AntSteps::changeEvent(QEvent* event)
{
    if (event && (event->type() == QEvent::FontChange ||
                  event->type() == QEvent::ApplicationFontChange ||
                  event->type() == QEvent::StyleChange ||
                  event->type() == QEvent::LayoutDirectionChange))
    {
        updateStepsGeometry();
    }
    QWidget::changeEvent(event);
}

AntSteps::Metrics AntSteps::metrics() const
{
    Metrics m;
    const auto& token = antTheme->tokens();
    m.iconSize = token.controlHeight;
    m.titleGap = token.marginXS;
    m.tailThickness = 2;
    m.itemGap = token.marginLG;
    m.titleFontSize = token.fontSizeLG;
    m.descFontSize = token.fontSize;
    return m;
}

QVector<AntSteps::LayoutItem> AntSteps::layoutItems() const
{
    const QSize cacheSize = size();
    if (m_layoutCacheValid &&
        m_layoutCacheRevision == m_layoutRevision &&
        m_layoutCacheSize == cacheSize)
    {
        ++m_layoutCacheHitCount;
        syncStepsPerfCounters();
        return m_cachedLayoutItems;
    }

    QVector<LayoutItem> items;
    if (m_steps.isEmpty())
    {
        m_cachedLayoutItems = items;
        m_layoutCacheSize = cacheSize;
        m_layoutCacheRevision = m_layoutRevision;
        m_layoutCacheValid = true;
        ++m_layoutBuildCount;
        syncStepsPerfCounters();
        return m_cachedLayoutItems;
    }

    if (m_direction == Ant::Orientation::Horizontal)
    {
        const int itemWidth = qMax(160, width() / m_steps.size());
        for (int i = 0; i < m_steps.size(); ++i)
        {
            LayoutItem item;
            item.itemRect = QRect(i * itemWidth, 0, itemWidth, height());
            item.iconRect = iconRect(item.itemRect);
            item.textRect = textRect(item.itemRect);
            items.push_back(item);
        }
    }
    else
    {
        const int itemHeight = qMax(72, height() / m_steps.size());
        for (int i = 0; i < m_steps.size(); ++i)
        {
            LayoutItem item;
            item.itemRect = QRect(0, i * itemHeight, width(), itemHeight);
            item.iconRect = iconRect(item.itemRect);
            item.textRect = textRect(item.itemRect);
            items.push_back(item);
        }
    }

    m_cachedLayoutItems = items;
    m_layoutCacheSize = cacheSize;
    m_layoutCacheRevision = m_layoutRevision;
    m_layoutCacheValid = true;
    ++m_layoutBuildCount;
    syncStepsPerfCounters();
    return m_cachedLayoutItems;
}

QRect AntSteps::iconRect(const QRect& itemRect) const
{
    const Metrics m = metrics();
    const int leftInset = m.tailThickness + 1;
    if (m_direction == Ant::Orientation::Horizontal)
    {
        return QRect(itemRect.left() + leftInset, 8, m.iconSize, m.iconSize);
    }
    return QRect(itemRect.left() + leftInset, itemRect.top() + 8, m.iconSize, m.iconSize);
}

QRect AntSteps::textRect(const QRect& itemRect) const
{
    const Metrics m = metrics();
    if (m_direction == Ant::Orientation::Horizontal)
    {
        const QRect icon = iconRect(itemRect);
        return QRect(icon.right() + 1 + m.titleGap,
                     icon.top(),
                     itemRect.width() - m.iconSize - m.titleGap,
                     itemRect.height() - icon.top());
    }
    return QRect(itemRect.left() + m.iconSize + 16, itemRect.top() + 4, itemRect.width() - m.iconSize - 20, itemRect.height() - 8);
}

int AntSteps::stepAtPos(const QPoint& pos) const
{
    const QVector<LayoutItem> items = layoutItems();
    for (int i = 0; i < items.size(); ++i)
    {
        if (items.at(i).itemRect.contains(pos))
        {
            return i;
        }
    }
    return -1;
}

Ant::StepStatus AntSteps::effectiveStatus(int index) const
{
    if (index < 0 || index >= m_steps.size())
    {
        return Ant::StepStatus::Wait;
    }
    const AntStepItem& step = m_steps.at(index);
    if (step.status == Ant::StepStatus::Error)
    {
        return Ant::StepStatus::Error;
    }
    if (step.status == Ant::StepStatus::Finish)
    {
        return Ant::StepStatus::Finish;
    }
    if (index < m_currentIndex)
    {
        return Ant::StepStatus::Finish;
    }
    if (index == m_currentIndex)
    {
        return Ant::StepStatus::Process;
    }
    return Ant::StepStatus::Wait;
}

QColor AntSteps::statusColor(Ant::StepStatus status) const
{
    const auto& token = antTheme->tokens();
    switch (status)
    {
    case Ant::StepStatus::Finish:
    case Ant::StepStatus::Process:
        return token.colorPrimary;
    case Ant::StepStatus::Error:
        return token.colorError;
    case Ant::StepStatus::Wait:
    default:
        return token.colorBorder;
    }
}

QString AntSteps::iconText(Ant::StepStatus status, int index) const
{
    Q_UNUSED(status)
    return QString::number(index + 1);
}

void AntSteps::invalidateLayoutCache()
{
    ++m_layoutRevision;
    if (m_layoutRevision == 0)
    {
        m_layoutRevision = 1;
    }
    m_layoutCacheValid = false;
}

QRect AntSteps::stepDirtyRect(int index) const
{
    const QVector<LayoutItem> items = layoutItems();
    if (index < 0 || index >= items.size())
    {
        return {};
    }
    return items.at(index).itemRect.adjusted(-4, -4, 4, 4).intersected(rect());
}

void AntSteps::updateStepStateRegion(int oldIndex, int newIndex)
{
    QRect dirty;
    const QRect oldRect = stepDirtyRect(oldIndex);
    const QRect newRect = stepDirtyRect(newIndex);
    if (!oldRect.isNull())
    {
        dirty = oldRect;
    }
    if (!newRect.isNull())
    {
        dirty = dirty.isNull() ? newRect : dirty.united(newRect);
    }
    if (dirty.isNull())
    {
        return;
    }
    ++m_scopedStepUpdateCount;
    syncStepsPerfCounters();
    update(dirty);
}

void AntSteps::updateStepRangeRegion(int oldIndex, int newIndex)
{
    if (m_steps.isEmpty())
    {
        update();
        return;
    }
    int first = qBound(0, qMin(oldIndex, newIndex), m_steps.size() - 1);
    int last = qBound(0, qMax(oldIndex, newIndex), m_steps.size() - 1);
    QRect dirty;
    for (int i = first; i <= last; ++i)
    {
        const QRect r = stepDirtyRect(i);
        if (!r.isNull())
        {
            dirty = dirty.isNull() ? r : dirty.united(r);
        }
    }
    if (dirty.isNull())
    {
        update();
        return;
    }
    ++m_scopedStepUpdateCount;
    syncStepsPerfCounters();
    update(dirty);
}

void AntSteps::updateStepsGeometry()
{
    invalidateLayoutCache();
    updateGeometry();
}

void AntSteps::syncStepsPerfCounters() const
{
    auto* self = const_cast<AntSteps*>(this);
    self->setProperty("antStepsLayoutCacheHitCount", m_layoutCacheHitCount);
    self->setProperty("antStepsLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antStepsScopedStepUpdateCount", m_scopedStepUpdateCount);
}
