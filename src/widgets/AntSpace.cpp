#include "AntSpace.h"

#include <QBoxLayout>
#include <QEvent>
#include <QResizeEvent>
#include <QSizePolicy>

#include "../styles/AntSpaceStyle.h"
#include "core/AntTheme.h"

AntSpace::AntSpace(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntSpaceStyle>(this);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(spacingValue());
    syncPerfCounters();
}

Ant::Orientation AntSpace::orientation() const { return m_orientation; }

void AntSpace::setOrientation(Ant::Orientation orientation)
{
    if (m_orientation == orientation)
    {
        return;
    }
    m_orientation = orientation;
    rebuildLayout();
    updateGeometry();
    update();
    Q_EMIT orientationChanged(m_orientation);
}

Ant::Size AntSpace::size() const { return m_size; }

void AntSpace::setSize(Ant::Size size)
{
    if (m_size == size)
    {
        return;
    }
    m_size = size;
    if (m_layout)
    {
        m_layout->setSpacing(spacingValue());
        ++m_spacingUpdateCount;
    }
    invalidateSizeCache();
    updateGeometry();
    update();
    syncPerfCounters();
    Q_EMIT sizeChanged(m_size);
}

int AntSpace::customSpacing() const { return m_customSpacing; }

void AntSpace::setCustomSpacing(int spacing)
{
    if (m_customSpacing == spacing)
    {
        return;
    }
    m_customSpacing = spacing;
    if (m_layout)
    {
        m_layout->setSpacing(spacingValue());
        ++m_spacingUpdateCount;
    }
    invalidateSizeCache();
    updateGeometry();
    update();
    syncPerfCounters();
}

bool AntSpace::isWrap() const { return m_wrap; }

void AntSpace::setWrap(bool wrap)
{
    if (m_wrap == wrap)
    {
        return;
    }
    m_wrap = wrap;
    rebuildLayout();
    update();
    Q_EMIT wrapChanged(m_wrap);
}

Qt::Alignment AntSpace::alignment() const { return m_alignment; }

void AntSpace::setAlignment(Qt::Alignment alignment)
{
    if (m_alignment == alignment)
    {
        return;
    }
    m_alignment = alignment;
    if (m_layout)
    {
        m_layout->setAlignment(alignment);
    }
    rebuildLayout();
}

void AntSpace::addItem(QWidget* widget)
{
    if (!widget || m_items.contains(widget))
    {
        return;
    }
    m_items.append(QPointer<QWidget>(widget));
    widget->setParent(this);
    widget->installEventFilter(this);
    widget->show();

    if (!m_layout)
    {
        rebuildLayout();
        return;
    }

    appendWidgetToLayout(widget, m_items.size() - 1);
    ++m_incrementalAddCount;
    invalidateSizeCache();
    updateGeometry();
    syncPerfCounters();
}

void AntSpace::insertItem(int index, QWidget* widget)
{
    if (!widget || m_items.contains(widget))
    {
        return;
    }
    index = qBound(0, index, m_items.size());
    if (index == m_items.size())
    {
        addItem(widget);
        return;
    }
    m_items.insert(index, QPointer<QWidget>(widget));
    widget->setParent(this);
    widget->installEventFilter(this);
    widget->show();
    rebuildLayout();
}

void AntSpace::removeItem(QWidget* widget)
{
    if (!widget)
    {
        return;
    }
    if (!m_items.contains(widget))
    {
        return;
    }
    widget->removeEventFilter(this);
    m_items.removeAll(QPointer<QWidget>(widget));
    rebuildLayout();
}

int AntSpace::itemCount() const { return m_items.size(); }

QWidget* AntSpace::itemAt(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return nullptr;
    }
    return m_items.at(index).data();
}

void AntSpace::clearItems()
{
    for (const auto& item : m_items)
    {
        if (item)
        {
            item->removeEventFilter(this);
        }
    }
    m_items.clear();
    rebuildLayout();
}

void AntSpace::setSeparator(QWidget* widget)
{
    if (m_separator == widget)
    {
        return;
    }
    m_separator = QPointer<QWidget>(widget);
    rebuildLayout();
}

QWidget* AntSpace::separator() const { return m_separator.data(); }

QSize AntSpace::sizeHint() const
{
    if (!m_layout)
    {
        return QWidget::sizeHint();
    }
    if (m_sizeHintDirty)
    {
        m_cachedSizeHint = m_layout->totalSizeHint();
        m_sizeHintDirty = false;
        ++m_sizeHintComputeCount;
        syncPerfCounters();
    }
    return m_cachedSizeHint;
}

QSize AntSpace::minimumSizeHint() const
{
    if (!m_layout)
    {
        return QWidget::minimumSizeHint();
    }
    if (m_minimumSizeHintDirty)
    {
        m_cachedMinimumSizeHint = m_layout->totalMinimumSize();
        m_minimumSizeHintDirty = false;
        ++m_minimumSizeHintComputeCount;
        syncPerfCounters();
    }
    return m_cachedMinimumSizeHint;
}

bool AntSpace::eventFilter(QObject* watched, QEvent* event)
{
    auto* widget = qobject_cast<QWidget*>(watched);
    if (widget && m_items.contains(widget))
    {
        switch (event->type())
        {
        case QEvent::Destroy:
            m_items.removeAll(QPointer<QWidget>(widget));
            invalidateSizeCache();
            syncPerfCounters();
            break;
        case QEvent::FontChange:
        case QEvent::Hide:
        case QEvent::LayoutRequest:
        case QEvent::Resize:
        case QEvent::Show:
        case QEvent::StyleChange:
            invalidateSizeCache();
            updateGeometry();
            syncPerfCounters();
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntSpace::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntSpace::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

int AntSpace::spacingValue() const
{
    if (m_customSpacing >= 0)
    {
        return m_customSpacing;
    }

    switch (m_size)
    {
    case Ant::Size::Small:
        return 8;
    case Ant::Size::Middle:
        return 16;
    case Ant::Size::Large:
        return 24;
    }
    return 8;
}

Qt::Alignment AntSpace::childAlignment() const
{
    return m_alignment
        ? m_alignment
        : (m_orientation == Ant::Orientation::Horizontal ? Qt::AlignLeft | Qt::AlignVCenter : Qt::AlignLeft);
}

QBoxLayout* AntSpace::createLayoutForOrientation()
{
    QBoxLayout* layout = nullptr;
    if (m_orientation == Ant::Orientation::Horizontal)
    {
        layout = new QHBoxLayout(this);
    }
    else
    {
        layout = new QVBoxLayout(this);
    }
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(spacingValue());
    if (m_alignment)
    {
        layout->setAlignment(m_alignment);
    }
    return layout;
}

void AntSpace::clearSeparatorCopies()
{
    for (const auto& copy : m_separatorCopies)
    {
        if (copy)
        {
            delete copy.data();
        }
    }
    m_separatorCopies.clear();
}

QWidget* AntSpace::createSeparatorCopy()
{
    if (!m_separator)
    {
        return nullptr;
    }

    auto* sepCopy = new QWidget(this);
    sepCopy->setFixedSize(m_separator->sizeHint());
    m_separatorCopies.append(QPointer<QWidget>(sepCopy));
    return sepCopy;
}

void AntSpace::appendWidgetToLayout(QWidget* widget, int itemIndex)
{
    if (!m_layout || !widget)
    {
        return;
    }

    const Qt::Alignment alignment = childAlignment();
    if (itemIndex > 0 && m_separator && !m_separator.isNull())
    {
        if (auto* sepCopy = createSeparatorCopy())
        {
            m_layout->addWidget(sepCopy, 0, alignment);
        }
    }

    m_layout->addWidget(widget, 0, alignment);
}

void AntSpace::invalidateSizeCache()
{
    m_sizeHintDirty = true;
    m_minimumSizeHintDirty = true;
}

void AntSpace::rebuildLayout()
{
    clearSeparatorCopies();

    if (m_layout)
    {
        delete m_layout;
        m_layout = nullptr;
    }

    m_layout = createLayoutForOrientation();
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    for (int i = 0; i < m_items.size(); ++i)
    {
        QWidget* item = m_items.at(i).data();
        if (!item)
        {
            continue;
        }

        item->setParent(this);
        item->show();
        appendWidgetToLayout(item, i);
    }

    ++m_rebuildCount;
    invalidateSizeCache();
    updateGeometry();
    syncPerfCounters();
}

void AntSpace::syncPerfCounters() const
{
    const_cast<AntSpace*>(this)->setProperty("antSpaceRebuildCount", m_rebuildCount);
    const_cast<AntSpace*>(this)->setProperty("antSpaceIncrementalAddCount", m_incrementalAddCount);
    const_cast<AntSpace*>(this)->setProperty("antSpaceSpacingUpdateCount", m_spacingUpdateCount);
    const_cast<AntSpace*>(this)->setProperty("antSpaceSizeHintComputeCount", m_sizeHintComputeCount);
    const_cast<AntSpace*>(this)->setProperty("antSpaceMinimumSizeHintComputeCount", m_minimumSizeHintComputeCount);
    const_cast<AntSpace*>(this)->setProperty("antSpaceSeparatorCopyCount", m_separatorCopies.size());
    const_cast<AntSpace*>(this)->setProperty("antSpaceSizeHintDirty", m_sizeHintDirty || m_minimumSizeHintDirty);
}
