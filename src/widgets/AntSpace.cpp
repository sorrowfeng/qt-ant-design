#include "AntSpace.h"

#include <QBoxLayout>
#include <QResizeEvent>

#include "../styles/AntSpaceStyle.h"
#include "core/AntTheme.h"

AntSpace::AntSpace(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntSpaceStyle(style()));

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(spacingValue());
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
    }
    updateGeometry();
    update();
    Q_EMIT sizeChanged(m_size);
}

int AntSpace::customSpacing() const { return m_customSpacing; }

void AntSpace::setCustomSpacing(int spacing)
{
    m_customSpacing = spacing;
    if (m_layout)
    {
        m_layout->setSpacing(spacingValue());
    }
    updateGeometry();
    update();
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
    Q_EMIT wrapChanged(m_wrap);
}

Qt::Alignment AntSpace::alignment() const { return m_alignment; }

void AntSpace::setAlignment(Qt::Alignment alignment)
{
    m_alignment = alignment;
    if (m_layout)
    {
        m_layout->setAlignment(alignment);
    }
}

void AntSpace::addItem(QWidget* widget)
{
    if (!widget || m_items.contains(widget))
    {
        return;
    }
    m_items.append(QPointer<QWidget>(widget));
    rebuildLayout();
}

void AntSpace::insertItem(int index, QWidget* widget)
{
    if (!widget || m_items.contains(widget))
    {
        return;
    }
    index = qBound(0, index, m_items.size());
    m_items.insert(index, QPointer<QWidget>(widget));
    rebuildLayout();
}

void AntSpace::removeItem(QWidget* widget)
{
    if (!widget)
    {
        return;
    }
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
    m_items.clear();
    rebuildLayout();
}

void AntSpace::setSeparator(QWidget* widget)
{
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
    return m_layout->totalSizeHint();
}

QSize AntSpace::minimumSizeHint() const
{
    if (!m_layout)
    {
        return QWidget::minimumSizeHint();
    }
    return m_layout->totalMinimumSize();
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

void AntSpace::rebuildLayout()
{
    if (!m_layout)
    {
        return;
    }

    delete m_layout;
    m_layout = nullptr;

    if (m_orientation == Ant::Orientation::Horizontal)
    {
        auto* hLayout = new QHBoxLayout(this);
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->setSpacing(spacingValue());
        if (m_alignment)
        {
            hLayout->setAlignment(m_alignment);
        }
        m_layout = hLayout;
    }
    else
    {
        auto* vLayout = new QVBoxLayout(this);
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->setSpacing(spacingValue());
        if (m_alignment)
        {
            vLayout->setAlignment(m_alignment);
        }
        m_layout = vLayout;
    }

    for (int i = 0; i < m_items.size(); ++i)
    {
        QWidget* item = m_items.at(i).data();
        if (!item)
        {
            continue;
        }

        if (i > 0 && m_separator && !m_separator.isNull())
        {
            auto* sepCopy = new QWidget(this);
            sepCopy->setFixedSize(m_separator->sizeHint());
            m_layout->addWidget(sepCopy);
        }

        m_layout->addWidget(item);
    }

    updateGeometry();
}
