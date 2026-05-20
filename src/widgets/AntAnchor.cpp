#include "AntAnchor.h"

#include <QEasingCurve>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QVariantAnimation>
#include <QVBoxLayout>

#include "core/AntTheme.h"

AntAnchor::AntAnchor(QWidget* parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_indicatorAnimation = new QVariantAnimation(this);
    m_indicatorAnimation->setDuration(180);
    m_indicatorAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_indicatorAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        const QRectF nextRect = value.toRectF();
        const QRect dirtyRect = indicatorDirtyRect(m_indicatorRect, nextRect);
        m_indicatorRect = nextRect;
        dirtyRect.isValid() ? update(dirtyRect) : update();
    });

    m_scrollResolveTimer = new QTimer(this);
    m_scrollResolveTimer->setSingleShot(true);
    m_scrollResolveTimer->setInterval(0);
    connect(m_scrollResolveTimer, &QTimer::timeout, this, &AntAnchor::resolvePendingScrollPosition);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        applyLabelVisualState();
        invalidateLinkRectCache();
        refreshActiveIndicatorGeometry();
        update();
    });
    syncAnchorPerfCounters();
}

void AntAnchor::setScrollArea(QScrollArea* area)
{
    if (m_scrollArea)
        m_scrollArea->verticalScrollBar()->disconnect(this);

    m_scrollArea = area;

    if (m_scrollArea)
    {
        connect(m_scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
            scheduleScrollResolve(value);
        });
        scheduleScrollResolve(m_scrollArea->verticalScrollBar()->value());
    }
}

int AntAnchor::activeIndex() const { return m_activeIndex; }

void AntAnchor::addLink(const QString& title, int targetY)
{
    m_links.append({title, targetY});

    auto* label = new QLabel(title, this);
    label->setCursor(Qt::PointingHandCursor);
    label->setContentsMargins(16, 4, 8, 4);
    label->setProperty("anchorIndex", m_links.size() - 1);
    label->installEventFilter(this);
    m_linkLabels.append(label);

    m_layout->addWidget(label);
    invalidateLinkRectCache();
    applyLabelVisualState();
}

bool AntAnchor::eventFilter(QObject* watched, QEvent* event)
{
    auto* label = qobject_cast<QLabel*>(watched);
    if (label && (event->type() == QEvent::Resize || event->type() == QEvent::Move))
    {
        invalidateLinkRectCache();
        if (label->property("anchorIndex").toInt() == m_activeIndex)
        {
            refreshActiveIndicatorGeometry();
        }
    }

    if (event->type() == QEvent::MouseButtonPress)
    {
        if (label)
        {
            int idx = label->property("anchorIndex").toInt();
            if (idx >= 0 && idx < m_links.size())
            {
                setActiveIndexInternal(idx, true);
                Q_EMIT linkClicked(idx, m_links[idx].targetY);
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntAnchor::paintEvent(QPaintEvent*)
{
    const auto& token = antTheme->tokens();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(QRect(0, 0, 2, height()), token.colorSplit);

    if (m_activeIndex >= 0)
    {
        if (m_indicatorRect.isNull())
        {
            m_indicatorRect = indicatorRectForIndex(m_activeIndex);
        }
        p.fillRect(m_indicatorRect.toRect(), token.colorPrimary);
    }
}

void AntAnchor::resizeEvent(QResizeEvent* event)
{
    invalidateLinkRectCache();
    refreshActiveIndicatorGeometry();
    QWidget::resizeEvent(event);
}

void AntAnchor::setActiveIndexInternal(int index, bool animate)
{
    if (index < -1 || index >= m_links.size() || m_activeIndex == index)
    {
        return;
    }

    const QRectF startRect = m_indicatorRect.isNull() ? indicatorRectForIndex(m_activeIndex) : m_indicatorRect;
    const QRectF endRect = indicatorRectForIndex(index);
    const QRect dirtyBeforeAnimation = indicatorDirtyRect(startRect, endRect);
    m_activeIndex = index;
    applyLabelVisualState();

    if (animate && !startRect.isNull() && !endRect.isNull())
    {
        m_indicatorAnimation->stop();
        m_indicatorAnimation->setStartValue(startRect);
        m_indicatorAnimation->setEndValue(endRect);
        m_indicatorAnimation->start();
    }
    else
    {
        m_indicatorAnimation->stop();
        m_indicatorRect = endRect;
        dirtyBeforeAnimation.isValid() ? update(dirtyBeforeAnimation) : update();
    }

    Q_EMIT activeIndexChanged(m_activeIndex);
}

QRectF AntAnchor::indicatorRectForIndex(int index) const
{
    if (index < 0 || index >= m_layout->count())
    {
        return {};
    }
    ensureLinkRectCache();
    if (index >= m_linkIndicatorRects.size())
    {
        return {};
    }
    return m_linkIndicatorRects[index];
}

void AntAnchor::scheduleScrollResolve(int value)
{
    m_pendingScrollValue = value;
    if (!m_scrollResolveTimer->isActive())
    {
        m_scrollResolveTimer->start();
    }
}

void AntAnchor::resolvePendingScrollPosition()
{
    int best = -1;
    for (int i = 0; i < m_links.size(); ++i)
    {
        if (m_pendingScrollValue >= m_links[i].targetY - 16)
        {
            best = i;
        }
    }
    ++m_scrollResolveCount;
    syncAnchorPerfCounters();
    if (best != m_activeIndex)
    {
        setActiveIndexInternal(best, true);
    }
}

void AntAnchor::applyLabelVisualState()
{
    const auto& token = antTheme->tokens();
    for (int i = 0; i < m_linkLabels.size(); ++i)
    {
        QLabel* label = m_linkLabels[i];
        if (!label)
        {
            continue;
        }

        bool changed = false;
        QFont font = label->font();
        if (font.pixelSize() != token.fontSize)
        {
            font.setPixelSize(token.fontSize);
            label->setFont(font);
            changed = true;
        }

        QPalette palette = label->palette();
        const QColor targetColor = (i == m_activeIndex) ? token.colorPrimary : token.colorText;
        if (palette.color(QPalette::WindowText) != targetColor)
        {
            palette.setColor(QPalette::WindowText, targetColor);
            label->setPalette(palette);
            changed = true;
        }

        if (changed)
        {
            ++m_labelVisualApplyCount;
        }
    }
    syncAnchorPerfCounters();
}

void AntAnchor::invalidateLinkRectCache() const
{
    m_linkRectCacheValid = false;
}

void AntAnchor::ensureLinkRectCache() const
{
    if (m_linkRectCacheValid)
    {
        ++m_linkRectCacheHitCount;
        syncAnchorPerfCounters();
        return;
    }

    m_linkIndicatorRects.clear();
    m_linkIndicatorRects.reserve(m_layout->count());
    for (int i = 0; i < m_layout->count(); ++i)
    {
        auto* label = qobject_cast<QLabel*>(m_layout->itemAt(i)->widget());
        m_linkIndicatorRects.append(label ? QRectF(label->x(), label->y(), 2, label->height()) : QRectF());
    }
    m_linkRectCacheValid = true;
    ++m_linkRectCacheMissCount;
    syncAnchorPerfCounters();
}

void AntAnchor::refreshActiveIndicatorGeometry()
{
    if (m_activeIndex < 0 || m_indicatorAnimation->state() == QAbstractAnimation::Running)
    {
        return;
    }

    const QRectF previous = m_indicatorRect;
    const QRectF next = indicatorRectForIndex(m_activeIndex);
    if (previous == next)
    {
        return;
    }

    m_indicatorRect = next;
    const QRect dirtyRect = indicatorDirtyRect(previous, next);
    dirtyRect.isValid() ? update(dirtyRect) : update();
}

QRect AntAnchor::indicatorDirtyRect(const QRectF& previous, const QRectF& current) const
{
    QRect dirty;
    if (!previous.isNull())
    {
        dirty = dirty.united(previous.toAlignedRect());
    }
    if (!current.isNull())
    {
        dirty = dirty.united(current.toAlignedRect());
    }
    return dirty.adjusted(-2, -2, 2, 2).intersected(rect());
}

void AntAnchor::syncAnchorPerfCounters() const
{
    auto* self = const_cast<AntAnchor*>(this);
    self->setProperty("antAnchorLinkRectCacheHitCount", m_linkRectCacheHitCount);
    self->setProperty("antAnchorLinkRectCacheMissCount", m_linkRectCacheMissCount);
    self->setProperty("antAnchorLabelVisualApplyCount", m_labelVisualApplyCount);
    self->setProperty("antAnchorScrollResolveCount", m_scrollResolveCount);
}
