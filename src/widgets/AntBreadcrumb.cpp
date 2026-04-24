#include "AntBreadcrumb.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>

#include <algorithm>

#include "../styles/AntBreadcrumbStyle.h"
#include "core/AntTheme.h"

AntBreadcrumb::AntBreadcrumb(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntBreadcrumbStyle(style()));
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

QString AntBreadcrumb::separator() const { return m_separator; }

void AntBreadcrumb::setSeparator(const QString& separator)
{
    const QString next = separator.isEmpty() ? QStringLiteral("/") : separator;
    if (m_separator == next)
    {
        return;
    }
    m_separator = next;
    updateBreadcrumbGeometry();
    update();
    Q_EMIT separatorChanged(m_separator);
}

void AntBreadcrumb::addItem(const QString& title, const QString& href, const QString& iconText, bool disabled)
{
    AntBreadcrumbItem item;
    item.title = title;
    item.href = href;
    item.iconText = iconText;
    item.disabled = disabled;
    m_items.append(item);
    updateBreadcrumbGeometry();
    update();
}

void AntBreadcrumb::addSeparator(const QString& separator)
{
    AntBreadcrumbItem item;
    item.separatorOnly = true;
    item.separator = separator.isEmpty() ? m_separator : separator;
    m_items.append(item);
    updateBreadcrumbGeometry();
    update();
}

void AntBreadcrumb::clearItems()
{
    m_items.clear();
    m_hoveredIndex = -1;
    updateBreadcrumbGeometry();
    update();
}

int AntBreadcrumb::count() const { return static_cast<int>(m_items.size()); }

AntBreadcrumbItem AntBreadcrumb::itemAt(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }
    return m_items.at(index);
}

QSize AntBreadcrumb::sizeHint() const
{
    const auto& token = antTheme->tokens();
    int width = token.paddingXS;
    for (int i = 0; i < m_items.size(); ++i)
    {
        width += itemWidth(m_items.at(i));
        if (i < m_items.size() - 1 && !m_items.at(i).separatorOnly)
        {
            QFont f = font();
            f.setPixelSize(token.fontSize);
            width += QFontMetrics(f).horizontalAdvance(m_separator) + token.marginXS * 2;
        }
    }
    return QSize(std::max(width, 160), token.controlHeight);
}

QSize AntBreadcrumb::minimumSizeHint() const
{
    return QSize(120, antTheme->tokens().controlHeightSM);
}

void AntBreadcrumb::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntBreadcrumb::mouseMoveEvent(QMouseEvent* event)
{
    const int index = itemIndexAt(event->pos());
    if (m_hoveredIndex != index)
    {
        m_hoveredIndex = index;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void AntBreadcrumb::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const int index = itemIndexAt(event->pos());
        if (index >= 0)
        {
            const AntBreadcrumbItem& item = m_items.at(index);
            if (!item.disabled && !isLastRouteItem(index))
            {
                Q_EMIT itemClicked(index, item.title, item.href);
                event->accept();
                return;
            }
        }
    }
    QWidget::mousePressEvent(event);
}

void AntBreadcrumb::leaveEvent(QEvent* event)
{
    if (m_hoveredIndex != -1)
    {
        m_hoveredIndex = -1;
        update();
    }
    QWidget::leaveEvent(event);
}

QVector<QRect> AntBreadcrumb::itemRects() const
{
    const auto& token = antTheme->tokens();
    QVector<QRect> rects;
    int x = token.paddingXS;
    const int h = height() > 0 ? height() : sizeHint().height();
    QFont f = font();
    f.setPixelSize(token.fontSize);
    const int separatorWidth = QFontMetrics(f).horizontalAdvance(m_separator) + token.marginXS * 2;

    for (int i = 0; i < m_items.size(); ++i)
    {
        const int w = itemWidth(m_items.at(i));
        rects.append(QRect(x, 0, w, h));
        x += w;
        if (i < m_items.size() - 1 && !m_items.at(i).separatorOnly)
        {
            x += separatorWidth;
        }
    }
    return rects;
}

int AntBreadcrumb::itemIndexAt(const QPoint& pos) const
{
    const auto rects = itemRects();
    for (int i = 0; i < rects.size(); ++i)
    {
        if (rects.at(i).contains(pos))
        {
            const AntBreadcrumbItem& item = m_items.at(i);
            if (item.separatorOnly || item.disabled || isLastRouteItem(i))
            {
                return -1;
            }
            return i;
        }
    }
    return -1;
}

bool AntBreadcrumb::isLastRouteItem(int index) const
{
    for (int i = m_items.size() - 1; i >= 0; --i)
    {
        if (!m_items.at(i).separatorOnly)
        {
            return i == index;
        }
    }
    return false;
}

int AntBreadcrumb::itemWidth(const AntBreadcrumbItem& item) const
{
    const auto& token = antTheme->tokens();
    QFont f = font();
    f.setPixelSize(token.fontSize);
    if (item.separatorOnly)
    {
        return QFontMetrics(f).horizontalAdvance(item.separator.isEmpty() ? m_separator : item.separator) + token.marginXS * 2;
    }
    return QFontMetrics(f).horizontalAdvance(item.title) + (item.iconText.isEmpty() ? 0 : 22);
}

QColor AntBreadcrumb::itemColor(const AntBreadcrumbItem& item, int index, bool hovered) const
{
    const auto& token = antTheme->tokens();
    if (item.disabled)
    {
        return token.colorTextDisabled;
    }
    if (isLastRouteItem(index))
    {
        return token.colorText;
    }
    return hovered ? token.colorText : token.colorTextSecondary;
}

void AntBreadcrumb::updateBreadcrumbGeometry()
{
    updateGeometry();
}
