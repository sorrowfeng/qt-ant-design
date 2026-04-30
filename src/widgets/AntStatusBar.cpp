#include "AntStatusBar.h"

#include <QEvent>
#include <QFontMetrics>
#include <QMouseEvent>

#include "../styles/AntStatusBarStyle.h"
#include "core/AntTheme.h"

AntStatusBar::AntStatusBar(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntStatusBarStyle>(this);
    setMouseTracking(true);
    setFixedHeight(28);
}

QString AntStatusBar::message() const { return m_message; }

void AntStatusBar::setMessage(const QString& message)
{
    if (m_message == message)
    {
        return;
    }
    m_message = message;
    update();
    Q_EMIT messageChanged(m_message);
}

bool AntStatusBar::hasSizeGrip() const { return m_sizeGrip; }

void AntStatusBar::setSizeGrip(bool enabled)
{
    if (m_sizeGrip == enabled)
    {
        return;
    }
    m_sizeGrip = enabled;
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

void AntStatusBar::mouseMoveEvent(QMouseEvent* event)
{
    const int regIdx = regularItemIndexAt(event->pos());
    const int permIdx = permanentItemIndexAt(event->pos());

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
        update();
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
        update();
    }
    QWidget::leaveEvent(event);
}

int AntStatusBar::regularItemIndexAt(const QPoint& pos) const
{
    if (m_items.isEmpty())
    {
        return -1;
    }

    const auto& token = antTheme->tokens();
    QFont f = font();
    f.setPixelSize(token.fontSizeSM);
    QFontMetrics fm(f);
    int x = token.paddingXS;
    const int h = height();

    for (int i = 0; i < m_items.size(); ++i)
    {
        const AntStatusBarItem& item = m_items.at(i);
        int textWidth = fm.horizontalAdvance(item.text);
        if (!item.icon.isEmpty())
        {
            textWidth += 16 + token.paddingXXS;
        }
        const int itemWidth = textWidth + token.paddingXS * 2;
        const QRect rect(x, 0, itemWidth, h);
        if (rect.contains(pos))
        {
            return i;
        }
        x += itemWidth + token.paddingXS;
    }
    return -1;
}

int AntStatusBar::permanentItemIndexAt(const QPoint& pos) const
{
    if (m_permanentItems.isEmpty())
    {
        return -1;
    }

    const auto& token = antTheme->tokens();
    QFont f = font();
    f.setPixelSize(token.fontSizeSM);
    QFontMetrics fm(f);

    const int rightX = m_sizeGrip ? width() - 20 : width();
    const int dividerWidth = token.lineWidth + token.paddingXXS * 2;
    int x = rightX - token.paddingXS;

    for (int i = static_cast<int>(m_permanentItems.size()) - 1; i >= 0; --i)
    {
        if (i < m_permanentItems.size() - 1)
        {
            x -= dividerWidth;
        }

        const AntStatusBarItem& item = m_permanentItems.at(i);
        int textWidth = fm.horizontalAdvance(item.text);
        if (!item.icon.isEmpty())
        {
            textWidth += 16 + token.paddingXXS;
        }
        const int itemWidth = textWidth + token.paddingXS * 2;
        x -= itemWidth;
        const QRect rect(x, 0, itemWidth, height());
        if (rect.contains(pos))
        {
            return i;
        }
    }
    return -1;
}
