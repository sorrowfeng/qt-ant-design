#include "AntLayout.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>

#include "../styles/AntLayoutStyle.h"
#include "core/AntTheme.h"

// ─── AntLayoutHeader ───

AntLayoutHeader::AntLayoutHeader(QWidget* parent)
    : QWidget(parent)
{
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
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
    painter.fillRect(rect(), antTheme->tokens().colorPrimary);
}

// ─── AntLayoutFooter ───

AntLayoutFooter::AntLayoutFooter(QWidget* parent)
    : QWidget(parent)
{
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
        update();
    });
}

QSize AntLayoutFooter::sizeHint() const
{
    return QSize(0, 48);
}

QSize AntLayoutFooter::minimumSizeHint() const
{
    return QSize(0, 48);
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
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
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
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
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
                    ? antTheme->tokens().colorBgLayout
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
    setStyle(new AntLayoutStyle(style()));
}

bool AntLayout::hasSider() const { return m_hasSider; }

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
    sider->setParent(this);
    m_siders.append(sider);
    updateHasSider();
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
    m_siders.removeAll(sider);
    sider->setParent(nullptr);
    updateHasSider();
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
    syncLayout();
    QWidget::resizeEvent(event);
}

void AntLayout::syncLayout()
{
    const int w = width();
    const int h = height();

    // Calculate sider widths
    int totalSiderWidth = 0;
    for (auto* s : m_siders)
    {
        if (s)
        {
            auto* sider = qobject_cast<AntLayoutSider*>(s);
            const int sw = sider ? (sider->isCollapsed() ? sider->collapsedWidth() : sider->siderWidth()) : s->sizeHint().width();
            totalSiderWidth += sw;
        }
    }

    // Header
    int y = 0;
    if (m_header)
    {
        const int hh = m_header->sizeHint().height();
        m_header->setGeometry(0, 0, w, hh);
        m_header->show();
        y += hh;
    }

    // Footer
    int contentBottom = h;
    if (m_footer)
    {
        const int fh = m_footer->sizeHint().height();
        contentBottom = h - fh;
        m_footer->setGeometry(0, contentBottom, w, fh);
        m_footer->show();
    }

    // Siders + Content
    int x = 0;
    for (auto* s : m_siders)
    {
        if (s)
        {
            auto* sider = qobject_cast<AntLayoutSider*>(s);
            const int sw = sider ? (sider->isCollapsed() ? sider->collapsedWidth() : sider->siderWidth()) : s->sizeHint().width();
            s->setGeometry(x, y, sw, contentBottom - y);
            s->show();
            x += sw;
        }
    }

    if (m_content)
    {
        m_content->setGeometry(x, y, w - x - totalSiderWidth + x - (totalSiderWidth > 0 ? 0 : 0), contentBottom - y);
        m_content->setGeometry(x, y, w - x, contentBottom - y);
        m_content->show();
    }
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
