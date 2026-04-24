#include "AntAffix.h"

#include <QAbstractScrollArea>
#include <QBoxLayout>
#include <QEvent>
#include <QScrollArea>
#include <QScrollBar>

AntAffix::AntAffix(QObject* parent)
    : QObject(parent)
{
}

AntAffix::~AntAffix()
{
    removeAffixed();
    detachScrollMonitor();
}

int AntAffix::offsetTop() const { return m_offsetTop; }

void AntAffix::setOffsetTop(int offset)
{
    m_offsetTop = offset;
    m_hasOffsetTop = true;
    m_hasOffsetBottom = false;
    if (m_affixedWidget) checkAffixState();
    Q_EMIT offsetTopChanged(m_offsetTop);
}

int AntAffix::offsetBottom() const { return m_offsetBottom; }

void AntAffix::setOffsetBottom(int offset)
{
    m_offsetBottom = offset;
    m_hasOffsetBottom = true;
    m_hasOffsetTop = false;
    if (m_affixedWidget) checkAffixState();
    Q_EMIT offsetBottomChanged(m_offsetBottom);
}

QWidget* AntAffix::affixedWidget() const { return m_affixedWidget; }

void AntAffix::setAffixedWidget(QWidget* widget)
{
    if (m_affixedWidget == widget) return;

    removeAffixed();
    detachScrollMonitor();

    m_affixedWidget = widget;
    if (m_affixedWidget)
    {
        m_originalParent = m_affixedWidget->parentWidget();
        m_originalPos = m_affixedWidget->pos();
        m_originalSize = m_affixedWidget->size();
        findScrollContainer();
        attachScrollMonitor();
    }
}

QWidget* AntAffix::scrollTarget() const { return m_scrollTarget; }

void AntAffix::setScrollTarget(QWidget* target)
{
    if (m_scrollTarget == target) return;
    detachScrollMonitor();
    m_scrollTarget = target;
    findScrollContainer();
    attachScrollMonitor();
}

bool AntAffix::isAffixed() const { return m_isAffixed; }

bool AntAffix::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_scrollViewport)
    {
        if (event->type() == QEvent::Wheel ||
            event->type() == QEvent::Scroll ||
            event->type() == QEvent::Resize)
        {
            checkAffixState();
        }
    }
    return QObject::eventFilter(watched, event);
}

void AntAffix::findScrollContainer()
{
    m_scrollViewport = nullptr;

    if (m_scrollTarget)
    {
        QAbstractScrollArea* area = qobject_cast<QAbstractScrollArea*>(m_scrollTarget);
        if (area)
        {
            m_scrollViewport = area->viewport();
            return;
        }
    }

    if (!m_affixedWidget) return;

    // Walk up parent chain to find QAbstractScrollArea
    QWidget* parent = m_affixedWidget->parentWidget();
    while (parent)
    {
        QAbstractScrollArea* area = qobject_cast<QAbstractScrollArea*>(parent);
        if (area)
        {
            m_scrollViewport = area->viewport();
            m_scrollTarget = parent;
            return;
        }
        parent = parent->parentWidget();
    }
}

void AntAffix::attachScrollMonitor()
{
    if (m_scrollViewport)
    {
        m_scrollViewport->installEventFilter(this);
    }
}

void AntAffix::detachScrollMonitor()
{
    if (m_scrollViewport)
    {
        m_scrollViewport->removeEventFilter(this);
        m_scrollViewport = nullptr;
    }
}

void AntAffix::checkAffixState()
{
    if (!m_affixedWidget || !m_scrollViewport) return;

    // Get widget position relative to viewport
    QPoint widgetPosInViewport = m_affixedWidget->mapTo(m_scrollViewport, QPoint(0, 0));
    int viewportHeight = m_scrollViewport->height();
    int widgetHeight = m_affixedWidget->height();

    bool shouldAffix = false;

    if (m_hasOffsetTop)
    {
        shouldAffix = widgetPosInViewport.y() < m_offsetTop;
    }
    else if (m_hasOffsetBottom)
    {
        shouldAffix = (widgetPosInViewport.y() + widgetHeight) > (viewportHeight - m_offsetBottom);
    }

    if (shouldAffix && !m_isAffixed)
    {
        applyAffixed();
    }
    else if (!shouldAffix && m_isAffixed)
    {
        removeAffixed();
    }
}

void AntAffix::applyAffixed()
{
    if (!m_affixedWidget || !m_scrollViewport || m_isAffixed) return;

    // Save state
    m_originalParent = m_affixedWidget->parentWidget();
    m_originalPos = m_affixedWidget->pos();
    m_originalSize = m_affixedWidget->size();

    // Create placeholder
    m_placeholder = new QWidget(m_originalParent);
    m_placeholder->setFixedSize(m_originalSize);
    m_placeholder->setVisible(false); // invisible spacer

    // Insert placeholder at same position in layout
    QLayout* layout = m_originalParent ? m_originalParent->layout() : nullptr;
    if (layout)
    {
        int idx = layout->indexOf(m_affixedWidget);
        if (idx >= 0)
        {
            QLayoutItem* item = layout->takeAt(idx);
            delete item;
            // QLayout doesn't have insertWidget; use addWidget with index for QBoxLayout
            QBoxLayout* boxLayout = qobject_cast<QBoxLayout*>(layout);
            if (boxLayout)
            {
                boxLayout->insertWidget(idx, m_placeholder);
            }
            else
            {
                layout->addWidget(m_placeholder);
            }
            m_placeholder->show();
        }
    }

    // Reparent widget to viewport (overlay)
    m_affixedWidget->setParent(m_scrollViewport);
    m_affixedWidget->raise();

    // Position at fixed offset
    if (m_hasOffsetTop)
    {
        m_affixedWidget->setGeometry(0, m_offsetTop, m_scrollViewport->width(), m_originalSize.height());
    }
    else if (m_hasOffsetBottom)
    {
        m_affixedWidget->setGeometry(0, m_scrollViewport->height() - m_offsetBottom - m_originalSize.height(),
                                     m_scrollViewport->width(), m_originalSize.height());
    }
    m_affixedWidget->show();

    m_isAffixed = true;
    Q_EMIT affixStateChanged(true);
}

void AntAffix::removeAffixed()
{
    if (!m_affixedWidget || !m_isAffixed) return;

    // Reparent back to original
    m_affixedWidget->setParent(m_originalParent);

    // Remove placeholder and restore widget to layout
    QLayout* layout = m_originalParent ? m_originalParent->layout() : nullptr;
    if (layout && m_placeholder)
    {
        int idx = layout->indexOf(m_placeholder);
        if (idx >= 0)
        {
            QLayoutItem* item = layout->takeAt(idx);
            delete item;
            QBoxLayout* boxLayout = qobject_cast<QBoxLayout*>(layout);
            if (boxLayout)
            {
                boxLayout->insertWidget(idx, m_affixedWidget);
            }
            else
            {
                layout->addWidget(m_affixedWidget);
            }
        }
    }
    else
    {
        m_affixedWidget->setGeometry(QRect(m_originalPos, m_originalSize));
    }

    delete m_placeholder;
    m_placeholder = nullptr;

    m_affixedWidget->show();
    m_isAffixed = false;
    Q_EMIT affixStateChanged(false);
}
