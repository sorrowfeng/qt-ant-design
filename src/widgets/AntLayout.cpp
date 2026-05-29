#include "AntLayout.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QRegion>

#include "../styles/AntLayoutStyle.h"
#include "core/AntTheme.h"
#include "core/AntThemeRefresh_p.h"

// ─── AntLayoutHeader ───

AntLayoutHeader::AntLayoutHeader(QWidget* parent)
    : QWidget(parent)
{
    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
        update();
    });
}

QSize AntLayoutHeader::sizeHint() const
{
    return QSize(0, 64);
}

QSize AntLayoutHeader::minimumSizeHint() const
{
    return QSize(0, 64);
}

void AntLayoutHeader::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.fillRect(rect(), QColor(QStringLiteral("#001529")));
}

// ─── AntLayoutFooter ───

AntLayoutFooter::AntLayoutFooter(QWidget* parent)
    : QWidget(parent)
{
    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
        update();
    });
}

QSize AntLayoutFooter::sizeHint() const
{
    return QSize(0, 70);
}

QSize AntLayoutFooter::minimumSizeHint() const
{
    return QSize(0, 70);
}

void AntLayoutFooter::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.fillRect(rect(), antTheme->tokens().colorBgLayout);
}

// ─── AntLayoutContent ───

AntLayoutContent::AntLayoutContent(QWidget* parent)
    : QWidget(parent)
{
    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
        update();
    });
}

QSize AntLayoutContent::sizeHint() const
{
    return QSize(0, 0);
}

QSize AntLayoutContent::minimumSizeHint() const
{
    return QSize(0, 0);
}

void AntLayoutContent::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.fillRect(rect(), antTheme->tokens().colorBgContainer);
}

// ─── AntLayoutSider ───

AntLayoutSider::AntLayoutSider(QWidget* parent)
    : QWidget(parent)
{
    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
        update();
    });
}

bool AntLayoutSider::isCollapsed() const { return m_collapsed; }

void AntLayoutSider::setCollapsed(bool collapsed)
{
    if (m_collapsed == collapsed)
    {
        return;
    }
    m_collapsed = collapsed;
    updateGeometry();
    update();
    Q_EMIT collapsedChanged(m_collapsed);
    Q_EMIT collapseTriggered(m_collapsed);
}

bool AntLayoutSider::isCollapsible() const { return m_collapsible; }

void AntLayoutSider::setCollapsible(bool collapsible)
{
    if (m_collapsible == collapsible)
    {
        return;
    }
    m_collapsible = collapsible;
    update();
    Q_EMIT collapsibleChanged(m_collapsible);
}

int AntLayoutSider::siderWidth() const { return m_width; }

void AntLayoutSider::setWidth(int width)
{
    if (m_width == width)
    {
        return;
    }
    m_width = width;
    updateGeometry();
    update();
    Q_EMIT widthChanged(m_width);
}

int AntLayoutSider::collapsedWidth() const { return m_collapsedWidth; }

void AntLayoutSider::setCollapsedWidth(int width)
{
    if (m_collapsedWidth == width)
    {
        return;
    }
    m_collapsedWidth = width;
    updateGeometry();
    update();
    Q_EMIT collapsedWidthChanged(m_collapsedWidth);
}

Ant::LayoutSiderTheme AntLayoutSider::siderTheme() const { return m_siderTheme; }

void AntLayoutSider::setSiderTheme(Ant::LayoutSiderTheme theme)
{
    if (m_siderTheme == theme)
    {
        return;
    }
    m_siderTheme = theme;
    update();
    Q_EMIT siderThemeChanged(m_siderTheme);
}

bool AntLayoutSider::isReverseArrow() const { return m_reverseArrow; }

void AntLayoutSider::setReverseArrow(bool reverse)
{
    if (m_reverseArrow == reverse)
    {
        return;
    }
    m_reverseArrow = reverse;
    update();
    Q_EMIT reverseArrowChanged(m_reverseArrow);
}

void AntLayoutSider::setTriggerWidget(QWidget* widget)
{
    if (m_triggerWidget == widget)
    {
        return;
    }
    m_triggerWidget = widget;
    if (m_triggerWidget)
    {
        m_triggerWidget->setParent(this);
        m_triggerWidget->show();
    }
    syncTriggerGeometry();
    update();
}

QWidget* AntLayoutSider::triggerWidget() const { return m_triggerWidget; }

QSize AntLayoutSider::sizeHint() const
{
    return QSize(m_collapsed ? m_collapsedWidth : m_width, 0);
}

QSize AntLayoutSider::minimumSizeHint() const
{
    return QSize(m_collapsed ? m_collapsedWidth : 0, 0);
}

void AntLayoutSider::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    QColor bg = (m_siderTheme == Ant::LayoutSiderTheme::Dark)
                    ? QColor(QStringLiteral("#001529"))
                    : antTheme->tokens().colorBgContainer;
    painter.fillRect(rect(), bg);
}

void AntLayoutSider::resizeEvent(QResizeEvent* event)
{
    syncTriggerGeometry();
    QWidget::resizeEvent(event);
}

void AntLayoutSider::mousePressEvent(QMouseEvent* event)
{
    if (m_collapsible && triggerRect().contains(event->pos()))
    {
        setCollapsed(!m_collapsed);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntLayoutSider::mouseMoveEvent(QMouseEvent* event)
{
    if (m_collapsible)
    {
        const bool hover = triggerRect().contains(event->pos());
        if (hover != m_triggerHovered)
        {
            m_triggerHovered = hover;
            update();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void AntLayoutSider::leaveEvent(QEvent* event)
{
    if (m_triggerHovered)
    {
        m_triggerHovered = false;
        update();
    }
    QWidget::leaveEvent(event);
}

QRect AntLayoutSider::triggerRect() const
{
    if (!m_collapsible)
    {
        return {};
    }
    const int triggerH = 48;
    return QRect(0, height() - triggerH, width(), triggerH);
}

void AntLayoutSider::syncTriggerGeometry()
{
    if (m_triggerWidget)
    {
        const QRect tr = triggerRect();
        m_triggerWidget->setGeometry(tr);
        m_triggerWidget->show();
    }
}

// ─── AntLayout ───

AntLayout::AntLayout(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntLayoutStyle>(this);
    syncPerfCounters();
}

bool AntLayout::hasSider() const { return m_hasSider; }

int AntLayout::borderRadius() const { return m_borderRadius; }

void AntLayout::setBorderRadius(int radius)
{
    radius = qMax(0, radius);
    if (m_borderRadius == radius)
    {
        return;
    }
    m_borderRadius = radius;
    m_maskDirty = true;
    syncMask();
    update();
    Q_EMIT borderRadiusChanged(m_borderRadius);
}

void AntLayout::setHeader(AntLayoutHeader* header)
{
    if (m_header == header)
    {
        return;
    }
    if (m_header)
    {
        m_header->setParent(nullptr);
    }
    m_header = header;
    if (m_header)
    {
        m_header->setParent(this);
        m_header->show();
    }
    m_layoutDirty = true;
    syncLayout();
    updateGeometry();
    update();
}

AntLayoutHeader* AntLayout::header() const
{
    return qobject_cast<AntLayoutHeader*>(m_header);
}

void AntLayout::setFooter(AntLayoutFooter* footer)
{
    if (m_footer == footer)
    {
        return;
    }
    if (m_footer)
    {
        m_footer->setParent(nullptr);
    }
    m_footer = footer;
    if (m_footer)
    {
        m_footer->setParent(this);
        m_footer->show();
    }
    m_layoutDirty = true;
    syncLayout();
    updateGeometry();
    update();
}

AntLayoutFooter* AntLayout::footer() const
{
    return qobject_cast<AntLayoutFooter*>(m_footer);
}

void AntLayout::setContent(AntLayoutContent* content)
{
    if (m_content == content)
    {
        return;
    }
    if (m_content)
    {
        m_content->setParent(nullptr);
    }
    m_content = content;
    if (m_content)
    {
        m_content->setParent(this);
        m_content->show();
    }
    m_layoutDirty = true;
    syncLayout();
    updateGeometry();
    update();
}

AntLayoutContent* AntLayout::content() const
{
    return qobject_cast<AntLayoutContent*>(m_content);
}

void AntLayout::addSider(AntLayoutSider* sider)
{
    if (!sider)
    {
        return;
    }
    if (m_siders.contains(sider))
    {
        return;
    }
    sider->setParent(this);
    m_siders.append(sider);
    auto syncSiderLayout = [this, sider]() {
        if (!m_siders.contains(sider))
        {
            return;
        }
        m_layoutDirty = true;
        syncLayout();
        updateGeometry();
    };
    m_siderConnections.insert(sider, {
        connect(sider, &AntLayoutSider::collapsedChanged, this, syncSiderLayout),
        connect(sider, &AntLayoutSider::widthChanged, this, syncSiderLayout),
        connect(sider, &AntLayoutSider::collapsedWidthChanged, this, syncSiderLayout),
    });
    updateHasSider();
    m_layoutDirty = true;
    syncLayout();
    updateGeometry();
    update();
}

void AntLayout::removeSider(AntLayoutSider* sider)
{
    if (!sider)
    {
        return;
    }
    if (!m_siders.removeOne(sider))
    {
        return;
    }
    const auto connections = m_siderConnections.take(sider);
    for (const auto& connection : connections)
    {
        disconnect(connection);
    }
    sider->setParent(nullptr);
    updateHasSider();
    m_layoutDirty = true;
    syncLayout();
    updateGeometry();
    update();
}

int AntLayout::siderCount() const { return m_siders.size(); }

AntLayoutSider* AntLayout::siderAt(int index) const
{
    if (index < 0 || index >= m_siders.size())
    {
        return nullptr;
    }
    return qobject_cast<AntLayoutSider*>(m_siders.at(index));
}

QSize AntLayout::sizeHint() const
{
    int h = 0;
    if (m_header)
    {
        h += m_header->sizeHint().height();
    }
    if (m_content)
    {
        h += m_content->sizeHint().height();
    }
    if (m_footer)
    {
        h += m_footer->sizeHint().height();
    }
    int w = 0;
    for (auto* s : m_siders)
    {
        if (s)
        {
            w += s->sizeHint().width();
        }
    }
    return QSize(qMax(400, w + 400), qMax(300, h));
}

QSize AntLayout::minimumSizeHint() const
{
    return QSize(200, 100);
}

void AntLayout::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntLayout::resizeEvent(QResizeEvent* event)
{
    if (event->size() != event->oldSize())
    {
        m_layoutDirty = true;
        m_maskDirty = true;
    }
    syncLayout();
    syncMask();
    QWidget::resizeEvent(event);
}

int AntLayout::siderVisualWidth(QWidget* sider) const
{
    if (!sider)
    {
        return 0;
    }
    auto* layoutSider = qobject_cast<AntLayoutSider*>(sider);
    return layoutSider ? (layoutSider->isCollapsed() ? layoutSider->collapsedWidth() : layoutSider->siderWidth())
                       : sider->sizeHint().width();
}

bool AntLayout::applyRegionGeometry(QWidget* widget, const QRect& geometry)
{
    if (!widget)
    {
        return false;
    }

    bool changed = false;
    if (widget->geometry() != geometry)
    {
        widget->setGeometry(geometry);
        ++m_geometryApplyCount;
        changed = true;
    }
    if (!widget->isVisible())
    {
        widget->show();
    }
    return changed;
}

void AntLayout::syncLayout()
{
    const QSize currentSize = size();
    if (!m_layoutDirty && m_lastLayoutSize == currentSize)
    {
        syncPerfCounters();
        return;
    }

    m_layoutDirty = false;
    m_lastLayoutSize = currentSize;
    ++m_layoutSyncCount;

    const int w = width();
    const int h = height();

    int y = 0;
    if (m_header)
    {
        const int hh = m_header->sizeHint().height();
        applyRegionGeometry(m_header, QRect(0, 0, w, hh));
        y += hh;
    }

    int contentBottom = h;
    if (m_footer)
    {
        const int fh = m_footer->sizeHint().height();
        contentBottom = h - fh;
        applyRegionGeometry(m_footer, QRect(0, contentBottom, w, fh));
    }

    int x = 0;
    const int contentHeight = qMax(0, contentBottom - y);
    for (auto* s : m_siders)
    {
        if (s)
        {
            const int sw = qMax(0, siderVisualWidth(s));
            applyRegionGeometry(s, QRect(x, y, sw, contentHeight));
            x += sw;
        }
    }

    if (m_content)
    {
        applyRegionGeometry(m_content, QRect(x, y, qMax(0, w - x), contentHeight));
    }
    syncPerfCounters();
}

void AntLayout::syncMask()
{
    const QRect currentRect = rect();
    if (!m_maskDirty && m_lastMaskRect == currentRect && m_lastMaskRadius == m_borderRadius)
    {
        syncPerfCounters();
        return;
    }

    m_maskDirty = false;
    m_lastMaskRect = currentRect;
    m_lastMaskRadius = m_borderRadius;
    ++m_maskSyncCount;

    if (m_borderRadius <= 0 || rect().isEmpty())
    {
        clearMask();
        syncPerfCounters();
        return;
    }

    QPainterPath path;
    path.addRoundedRect(QRectF(rect()), m_borderRadius, m_borderRadius);
    setMask(QRegion(path.toFillPolygon().toPolygon()));
    syncPerfCounters();
}

void AntLayout::updateHasSider()
{
    const bool has = !m_siders.isEmpty();
    if (has != m_hasSider)
    {
        m_hasSider = has;
        Q_EMIT hasSiderChanged(m_hasSider);
    }
}

void AntLayout::syncPerfCounters()
{
    setProperty("antLayoutSyncCount", m_layoutSyncCount);
    setProperty("antLayoutGeometryApplyCount", m_geometryApplyCount);
    setProperty("antLayoutMaskSyncCount", m_maskSyncCount);
}
