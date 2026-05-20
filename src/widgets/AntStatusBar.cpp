#include "AntStatusBar.h"

#include <QEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QTimer>

#include "../styles/AntStatusBarStyle.h"
#include "core/AntTheme.h"

namespace
{

int statusBarItemWidth(const AntStatusBarItem& item, const QFontMetrics& metrics)
{
    const auto& token = antTheme->tokens();
    int textWidth = metrics.horizontalAdvance(item.text);
    if (!item.icon.isEmpty())
    {
        textWidth += 16 + token.paddingXXS;
    }
    return textWidth + token.paddingXS * 2;
}

} // namespace

AntStatusBar::AntStatusBar(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntStatusBarStyle>(this);
    setMouseTracking(true);
    setFixedHeight(28);
    m_messageTimer = new QTimer(this);
    m_messageTimer->setSingleShot(true);
    connect(m_messageTimer, &QTimer::timeout, this, &AntStatusBar::clearMessage);
}

QString AntStatusBar::message() const { return m_message; }

void AntStatusBar::setMessage(const QString& message)
{
    if (m_message == message)
    {
        return;
    }
    m_message = message;
    updateStatusRegion(messageAreaRect());
    ++m_messageRegionUpdateCount;
    syncStatusBarPerfCounters();
    Q_EMIT messageChanged(m_message);
}

QString AntStatusBar::currentMessage() const { return m_message; }

void AntStatusBar::showMessage(const QString& message, int timeout)
{
    setMessage(message);
    if (timeout > 0)
    {
        m_messageTimer->start(timeout);
    }
    else
    {
        m_messageTimer->stop();
    }
}

void AntStatusBar::clearMessage()
{
    m_messageTimer->stop();
    setMessage(QString());
}

bool AntStatusBar::hasSizeGrip() const { return m_sizeGrip; }

void AntStatusBar::setSizeGrip(bool enabled)
{
    if (m_sizeGrip == enabled)
    {
        return;
    }
    m_sizeGrip = enabled;
    invalidateLayoutCache();
    update();
    Q_EMIT sizeGripChanged(m_sizeGrip);
}

int AntStatusBar::addItem(const QString& text,
                          const QString& icon,
                          const QString& tooltip,
                          int stretch)
{
    AntStatusBarItem item;
    item.text = text;
    item.icon = icon;
    item.tooltip = tooltip;
    item.stretch = stretch;
    m_items.append(item);
    invalidateLayoutCache();
    update();
    return static_cast<int>(m_items.size()) - 1;
}

int AntStatusBar::addPermanentItem(const QString& text,
                                   const QString& icon,
                                   const QString& tooltip,
                                   int stretch)
{
    AntStatusBarItem item;
    item.text = text;
    item.icon = icon;
    item.tooltip = tooltip;
    item.stretch = stretch;
    m_permanentItems.append(item);
    invalidateLayoutCache();
    update();
    return static_cast<int>(m_permanentItems.size()) - 1;
}

void AntStatusBar::removeItem(int index)
{
    if (index >= 0 && index < m_items.size())
    {
        m_items.remove(index);
        if (m_hoveredRegularIndex >= m_items.size())
        {
            m_hoveredRegularIndex = -1;
        }
        invalidateLayoutCache();
        update();
    }
}

int AntStatusBar::itemCount() const { return static_cast<int>(m_items.size()); }

int AntStatusBar::permanentItemCount() const { return static_cast<int>(m_permanentItems.size()); }

AntStatusBarItem AntStatusBar::itemAt(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }
    return m_items.at(index);
}

AntStatusBarItem AntStatusBar::permanentItemAt(int index) const
{
    if (index < 0 || index >= m_permanentItems.size())
    {
        return {};
    }
    return m_permanentItems.at(index);
}

int AntStatusBar::hoveredRegularIndex() const { return m_hoveredRegularIndex; }

int AntStatusBar::hoveredPermanentIndex() const { return m_hoveredPermanentIndex; }

QSize AntStatusBar::sizeHint() const
{
    return QSize(200, 28);
}

QSize AntStatusBar::minimumSizeHint() const
{
    return QSize(100, 28);
}

void AntStatusBar::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::FontChange || event->type() == QEvent::EnabledChange)
    {
        invalidateLayoutCache();
        update();
    }
    QWidget::changeEvent(event);
}

void AntStatusBar::mouseMoveEvent(QMouseEvent* event)
{
    const int regIdx = regularItemIndexAt(event->pos());
    const int permIdx = permanentItemIndexAt(event->pos());
    const QVector<QRect>& regularRects = regularItemRects();
    const QVector<QRect>& permanentRects = permanentItemRects();
    QRect dirty;
    if (m_hoveredRegularIndex >= 0 && m_hoveredRegularIndex < regularRects.size())
    {
        dirty = dirty.united(regularRects.at(m_hoveredRegularIndex));
    }
    if (m_hoveredPermanentIndex >= 0 && m_hoveredPermanentIndex < permanentRects.size())
    {
        dirty = dirty.united(permanentRects.at(m_hoveredPermanentIndex));
    }

    bool changed = false;
    if (m_hoveredRegularIndex != regIdx)
    {
        m_hoveredRegularIndex = regIdx;
        changed = true;
    }
    if (m_hoveredPermanentIndex != permIdx)
    {
        m_hoveredPermanentIndex = permIdx;
        changed = true;
    }

    if (changed)
    {
        if (m_hoveredRegularIndex >= 0 && m_hoveredRegularIndex < regularRects.size())
        {
            dirty = dirty.united(regularRects.at(m_hoveredRegularIndex));
        }
        if (m_hoveredPermanentIndex >= 0 && m_hoveredPermanentIndex < permanentRects.size())
        {
            dirty = dirty.united(permanentRects.at(m_hoveredPermanentIndex));
        }
        updateStatusRegion(dirty);
        const int hoverIdx = (m_hoveredRegularIndex >= 0) ? m_hoveredRegularIndex : m_hoveredPermanentIndex;
        if (hoverIdx >= 0)
        {
            const AntStatusBarItem& item = (m_hoveredRegularIndex >= 0) ? m_items.at(hoverIdx) : m_permanentItems.at(hoverIdx);
            if (!item.tooltip.isEmpty())
            {
                setToolTip(item.tooltip);
            }
            else
            {
                setToolTip(QString());
            }
        }
        else
        {
            setToolTip(QString());
        }
    }

    QWidget::mouseMoveEvent(event);
}

void AntStatusBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const int regIdx = regularItemIndexAt(event->pos());
        if (regIdx >= 0)
        {
            Q_EMIT itemClicked(regIdx);
            event->accept();
            return;
        }

        const int permIdx = permanentItemIndexAt(event->pos());
        if (permIdx >= 0)
        {
            Q_EMIT itemClicked(permIdx);
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void AntStatusBar::leaveEvent(QEvent* event)
{
    const QVector<QRect>& regularRects = regularItemRects();
    const QVector<QRect>& permanentRects = permanentItemRects();
    QRect dirty;
    if (m_hoveredRegularIndex >= 0 && m_hoveredRegularIndex < regularRects.size())
    {
        dirty = dirty.united(regularRects.at(m_hoveredRegularIndex));
    }
    if (m_hoveredPermanentIndex >= 0 && m_hoveredPermanentIndex < permanentRects.size())
    {
        dirty = dirty.united(permanentRects.at(m_hoveredPermanentIndex));
    }

    bool changed = false;
    if (m_hoveredRegularIndex != -1)
    {
        m_hoveredRegularIndex = -1;
        changed = true;
    }
    if (m_hoveredPermanentIndex != -1)
    {
        m_hoveredPermanentIndex = -1;
        changed = true;
    }
    if (changed)
    {
        setToolTip(QString());
        updateStatusRegion(dirty);
    }
    QWidget::leaveEvent(event);
}

void AntStatusBar::resizeEvent(QResizeEvent* event)
{
    invalidateLayoutCache();
    QWidget::resizeEvent(event);
}

int AntStatusBar::regularItemIndexAt(const QPoint& pos) const
{
    const QVector<QRect>& rects = regularItemRects();
    for (int i = 0; i < rects.size(); ++i)
    {
        if (rects.at(i).contains(pos))
        {
            return i;
        }
    }
    return -1;
}

int AntStatusBar::permanentItemIndexAt(const QPoint& pos) const
{
    const QVector<QRect>& rects = permanentItemRects();
    for (int i = 0; i < rects.size(); ++i)
    {
        if (rects.at(i).contains(pos))
        {
            return i;
        }
    }
    return -1;
}

void AntStatusBar::invalidateLayoutCache() const
{
    m_layoutCacheDirty = true;
}

void AntStatusBar::ensureLayoutCache() const
{
    const auto& token = antTheme->tokens();
    QFont itemFont = font();
    itemFont.setPixelSize(token.fontSizeSM);

    if (!m_layoutCacheDirty &&
        m_layoutCacheWidth == width() &&
        m_layoutCacheHeight == height() &&
        m_layoutCacheSizeGrip == m_sizeGrip &&
        m_layoutCacheFont == itemFont &&
        m_layoutCacheItems == m_items &&
        m_layoutCachePermanentItems == m_permanentItems)
    {
        return;
    }

    QFontMetrics fm(itemFont);
    m_regularItemRects.clear();
    m_permanentItemRects.clear();
    m_regularDividerXs.clear();
    m_permanentDividerXs.clear();

    const int h = height();
    const int dividerWidth = token.lineWidth + token.paddingXXS * 2;

    int x = token.paddingXS;
    for (int i = 0; i < m_items.size(); ++i)
    {
        const int itemWidth = statusBarItemWidth(m_items.at(i), fm);
        m_regularItemRects.append(QRect(x, 0, itemWidth, h));
        x += itemWidth;
        if (i < m_items.size() - 1)
        {
            m_regularDividerXs.append(x + token.paddingXXS);
            x += dividerWidth;
        }
    }

    const int rightX = m_sizeGrip ? width() - 20 : width();
    int rx = rightX - token.paddingXS;
    m_permanentItemRects.resize(m_permanentItems.size());
    for (int i = static_cast<int>(m_permanentItems.size()) - 1; i >= 0; --i)
    {
        if (i < m_permanentItems.size() - 1)
        {
            m_permanentDividerXs.prepend(rx - dividerWidth / 2);
            rx -= dividerWidth;
        }

        const int itemWidth = statusBarItemWidth(m_permanentItems.at(i), fm);
        rx -= itemWidth;
        m_permanentItemRects[i] = QRect(rx, 0, itemWidth, h);
    }

    const int msgLeft = x + token.paddingXS;
    const int msgRight = (!m_permanentItems.isEmpty()) ? rx : rightX;
    m_messageRect = msgRight > msgLeft ? QRect(msgLeft, 0, msgRight - msgLeft, h) : QRect();

    m_layoutCacheDirty = false;
    m_layoutCacheWidth = width();
    m_layoutCacheHeight = height();
    m_layoutCacheSizeGrip = m_sizeGrip;
    m_layoutCacheFont = itemFont;
    m_layoutCacheItems = m_items;
    m_layoutCachePermanentItems = m_permanentItems;
    ++m_layoutBuildCount;
    syncStatusBarPerfCounters();
}

void AntStatusBar::updateStatusRegion(const QRect& rect)
{
    if (!rect.isValid())
    {
        return;
    }

    const QRect updateRect = rect.adjusted(-2, -2, 2, 2).intersected(this->rect());
    if (!updateRect.isValid())
    {
        return;
    }

    update(updateRect);
    ++m_regionUpdateCount;
    syncStatusBarPerfCounters();
}

void AntStatusBar::syncStatusBarPerfCounters() const
{
    auto* self = const_cast<AntStatusBar*>(this);
    self->setProperty("antStatusBarLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antStatusBarRegionUpdateCount", m_regionUpdateCount);
    self->setProperty("antStatusBarMessageRegionUpdateCount", m_messageRegionUpdateCount);
}

const QVector<QRect>& AntStatusBar::regularItemRects() const
{
    ensureLayoutCache();
    return m_regularItemRects;
}

const QVector<QRect>& AntStatusBar::permanentItemRects() const
{
    ensureLayoutCache();
    return m_permanentItemRects;
}

const QVector<int>& AntStatusBar::regularDividerXs() const
{
    ensureLayoutCache();
    return m_regularDividerXs;
}

const QVector<int>& AntStatusBar::permanentDividerXs() const
{
    ensureLayoutCache();
    return m_permanentDividerXs;
}

QRect AntStatusBar::messageAreaRect() const
{
    ensureLayoutCache();
    return m_messageRect;
}
