#include "AntTabs.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QStackedWidget>

#include <algorithm>

#include "core/AntTheme.h"

AntTabs::AntTabs(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_stack = new QStackedWidget(this);
    m_stack->setObjectName(QStringLiteral("AntTabsStack"));
    m_stack->setAutoFillBackground(false);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateStackGeometry();
        update();
    });
}

QString AntTabs::activeKey() const { return m_activeKey; }

void AntTabs::setActiveKey(const QString& key)
{
    setActiveIndex(indexOfKey(key));
}

Ant::TabsType AntTabs::tabsType() const { return m_tabsType; }

void AntTabs::setTabsType(Ant::TabsType type)
{
    if (m_tabsType == type)
    {
        return;
    }
    m_tabsType = type;
    updateStackGeometry();
    update();
    Q_EMIT tabsTypeChanged(m_tabsType);
}

Ant::TabsSize AntTabs::tabsSize() const { return m_tabsSize; }

void AntTabs::setTabsSize(Ant::TabsSize size)
{
    if (m_tabsSize == size)
    {
        return;
    }
    m_tabsSize = size;
    updateStackGeometry();
    update();
    Q_EMIT tabsSizeChanged(m_tabsSize);
}

Ant::TabsPlacement AntTabs::tabPlacement() const { return m_tabPlacement; }

void AntTabs::setTabPlacement(Ant::TabsPlacement placement)
{
    if (m_tabPlacement == placement)
    {
        return;
    }
    m_tabPlacement = placement;
    updateStackGeometry();
    update();
    Q_EMIT tabPlacementChanged(m_tabPlacement);
}

bool AntTabs::isCentered() const { return m_centered; }

void AntTabs::setCentered(bool centered)
{
    if (m_centered == centered)
    {
        return;
    }
    m_centered = centered;
    update();
    Q_EMIT centeredChanged(m_centered);
}

bool AntTabs::isAnimated() const { return m_animated; }

void AntTabs::setAnimated(bool animated)
{
    if (m_animated == animated)
    {
        return;
    }
    m_animated = animated;
    Q_EMIT animatedChanged(m_animated);
}

bool AntTabs::isHideAdd() const { return m_hideAdd; }

void AntTabs::setHideAdd(bool hide)
{
    if (m_hideAdd == hide)
    {
        return;
    }
    m_hideAdd = hide;
    update();
    Q_EMIT hideAddChanged(m_hideAdd);
}

int AntTabs::tabBarGutter() const { return m_tabBarGutter; }

void AntTabs::setTabBarGutter(int gutter)
{
    gutter = std::max(0, gutter);
    if (m_tabBarGutter == gutter)
    {
        return;
    }
    m_tabBarGutter = gutter;
    updateStackGeometry();
    update();
    Q_EMIT tabBarGutterChanged(m_tabBarGutter);
}

int AntTabs::addTab(QWidget* page,
                    const QString& key,
                    const QString& label,
                    const QString& iconText,
                    bool disabled,
                    bool closable)
{
    if (!page || key.isEmpty() || indexOfKey(key) >= 0)
    {
        return -1;
    }

    AntTabItem item;
    item.key = key;
    item.label = label;
    item.iconText = iconText;
    item.page = page;
    item.disabled = disabled;
    item.closable = closable;
    m_tabs.append(item);
    m_stack->addWidget(page);

    if (m_activeKey.isEmpty() && !disabled)
    {
        setActiveIndex(m_tabs.size() - 1);
    }
    updateStackGeometry();
    update();
    return m_tabs.size() - 1;
}

void AntTabs::removeTab(const QString& key)
{
    const int index = indexOfKey(key);
    if (index < 0)
    {
        return;
    }

    QWidget* page = m_tabs.at(index).page;
    m_stack->removeWidget(page);
    if (page)
    {
        page->deleteLater();
    }
    m_tabs.removeAt(index);

    if (m_activeKey == key)
    {
        m_activeKey.clear();
        const int tabCount = static_cast<int>(m_tabs.size());
        for (int i = std::min(index, tabCount - 1); i >= 0 && i < tabCount; --i)
        {
            if (!m_tabs.at(i).disabled)
            {
                setActiveIndex(i);
                break;
            }
        }
        if (m_activeKey.isEmpty())
        {
            for (int i = 0; i < tabCount; ++i)
            {
                if (!m_tabs.at(i).disabled)
                {
                    setActiveIndex(i);
                    break;
                }
            }
        }
    }
    updateStackGeometry();
    update();
}

void AntTabs::clearTabs()
{
    while (!m_tabs.isEmpty())
    {
        QWidget* page = m_tabs.takeLast().page;
        m_stack->removeWidget(page);
        if (page)
        {
            page->deleteLater();
        }
    }
    m_activeKey.clear();
    updateStackGeometry();
    update();
    Q_EMIT activeKeyChanged(m_activeKey);
}

void AntTabs::setTabText(const QString& key, const QString& label)
{
    const int index = indexOfKey(key);
    if (index < 0 || m_tabs[index].label == label)
    {
        return;
    }
    m_tabs[index].label = label;
    update();
}

void AntTabs::setTabEnabled(const QString& key, bool enabled)
{
    const int index = indexOfKey(key);
    if (index < 0)
    {
        return;
    }
    m_tabs[index].disabled = !enabled;
    update();
}

QSize AntTabs::sizeHint() const
{
    return QSize(520, 260);
}

QSize AntTabs::minimumSizeHint() const
{
    return QSize(240, tabBarExtent() + 80);
}

void AntTabs::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    const auto& token = antTheme->tokens();
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter.fillRect(rect(), token.colorBgContainer);

    const QRect bar = tabBarRect();
    painter.setPen(QPen(token.colorSplit, token.lineWidth));
    if (isHorizontal())
    {
        const int y = m_tabPlacement == Ant::TabsPlacement::Top ? bar.bottom() : bar.top();
        painter.drawLine(bar.left(), y, bar.right(), y);
    }
    else
    {
        const int x = m_tabPlacement == Ant::TabsPlacement::Left ? bar.right() : bar.left();
        painter.drawLine(x, bar.top(), x, bar.bottom());
    }

    const auto rects = tabRects();
    for (int i = 0; i < rects.size(); ++i)
    {
        const bool active = m_tabs.at(i).key == m_activeKey;
        const bool hovered = i == m_hoveredIndex;
        drawTab(painter, m_tabs.at(i), rects.at(i), active, hovered);
    }

    const QRect addRect = addButtonRect();
    if (!addRect.isNull())
    {
        drawAddButton(painter, addRect);
    }
}

void AntTabs::resizeEvent(QResizeEvent* event)
{
    updateStackGeometry();
    QWidget::resizeEvent(event);
}

void AntTabs::mouseMoveEvent(QMouseEvent* event)
{
    const int hovered = tabAt(event->pos());
    const int closeHovered = closeAt(event->pos());
    const bool addHovered = addButtonRect().contains(event->pos());
    if (m_hoveredIndex != hovered || m_hoveredCloseIndex != closeHovered || m_addHovered != addHovered)
    {
        m_hoveredIndex = hovered;
        m_hoveredCloseIndex = closeHovered;
        m_addHovered = addHovered;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void AntTabs::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (addButtonRect().contains(event->pos()))
        {
            Q_EMIT tabAddRequested();
            event->accept();
            return;
        }

        const int closeIndex = closeAt(event->pos());
        if (closeIndex >= 0)
        {
            const QString key = m_tabs.at(closeIndex).key;
            Q_EMIT tabCloseRequested(key);
            removeTab(key);
            event->accept();
            return;
        }

        const int index = tabAt(event->pos());
        if (index >= 0 && !m_tabs.at(index).disabled)
        {
            Q_EMIT tabClicked(m_tabs.at(index).key);
            setActiveIndex(index);
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void AntTabs::leaveEvent(QEvent* event)
{
    m_hoveredIndex = -1;
    m_hoveredCloseIndex = -1;
    m_addHovered = false;
    update();
    QWidget::leaveEvent(event);
}

void AntTabs::keyPressEvent(QKeyEvent* event)
{
    if (m_tabs.isEmpty())
    {
        QWidget::keyPressEvent(event);
        return;
    }

    const bool nextKey = event->key() == Qt::Key_Right || event->key() == Qt::Key_Down;
    const bool prevKey = event->key() == Qt::Key_Left || event->key() == Qt::Key_Up;
    if (nextKey || prevKey)
    {
        int index = activeIndex();
        for (int step = 0; step < m_tabs.size(); ++step)
        {
            index = (index + (nextKey ? 1 : -1) + m_tabs.size()) % m_tabs.size();
            if (!m_tabs.at(index).disabled)
            {
                setActiveIndex(index);
                event->accept();
                return;
            }
        }
    }
    QWidget::keyPressEvent(event);
}

int AntTabs::indexOfKey(const QString& key) const
{
    for (int i = 0; i < m_tabs.size(); ++i)
    {
        if (m_tabs.at(i).key == key)
        {
            return i;
        }
    }
    return -1;
}

int AntTabs::activeIndex() const
{
    return indexOfKey(m_activeKey);
}

QRect AntTabs::tabBarRect() const
{
    const int extent = tabBarExtent();
    switch (m_tabPlacement)
    {
    case Ant::TabsPlacement::Bottom:
        return QRect(0, height() - extent, width(), extent);
    case Ant::TabsPlacement::Left:
        return QRect(0, 0, extent + 70, height());
    case Ant::TabsPlacement::Right:
        return QRect(width() - extent - 70, 0, extent + 70, height());
    case Ant::TabsPlacement::Top:
    default:
        return QRect(0, 0, width(), extent);
    }
}

QRect AntTabs::pageRect() const
{
    const int extent = tabBarExtent();
    switch (m_tabPlacement)
    {
    case Ant::TabsPlacement::Bottom:
        return rect().adjusted(0, 0, 0, -extent);
    case Ant::TabsPlacement::Left:
        return rect().adjusted(extent + 70, 0, 0, 0);
    case Ant::TabsPlacement::Right:
        return rect().adjusted(0, 0, -extent - 70, 0);
    case Ant::TabsPlacement::Top:
    default:
        return rect().adjusted(0, extent, 0, 0);
    }
}

QVector<QRect> AntTabs::tabRects() const
{
    const auto& token = antTheme->tokens();
    QVector<QRect> rects;
    const QRect bar = tabBarRect();
    if (isHorizontal())
    {
        int total = 0;
        QVector<int> widths;
        for (const auto& item : m_tabs)
        {
            const int w = tabLength(item);
            widths.append(w);
            total += w;
        }
        const int tabCount = static_cast<int>(m_tabs.size());
        total += std::max(0, tabCount - 1) * m_tabBarGutter;
        int x = bar.left() + token.padding;
        if (m_centered)
        {
            x = bar.left() + std::max(token.padding, (bar.width() - total) / 2);
        }
        for (int i = 0; i < widths.size(); ++i)
        {
            rects.append(QRect(x, bar.top(), widths.at(i), bar.height()));
            x += widths.at(i) + m_tabBarGutter;
        }
        return rects;
    }

    int y = bar.top() + token.paddingXS;
    const int tabWidth = bar.width();
    for (const auto& item : m_tabs)
    {
        const int h = std::max(tabBarExtent() - token.paddingXS, 36);
        Q_UNUSED(item)
        rects.append(QRect(bar.left(), y, tabWidth, h));
        y += h + token.marginXS;
    }
    return rects;
}

QRect AntTabs::addButtonRect() const
{
    if (m_tabsType != Ant::TabsType::EditableCard || m_hideAdd)
    {
        return QRect();
    }
    const auto& token = antTheme->tokens();
    const QRect bar = tabBarRect();
    const int side = tabBarExtent() - token.paddingXS;
    const auto rects = tabRects();
    if (isHorizontal())
    {
        const int x = rects.isEmpty() ? bar.left() + token.padding : rects.last().right() + token.marginXS;
        return QRect(x, bar.top() + token.paddingXS / 2, side, side);
    }
    const int y = rects.isEmpty() ? bar.top() + token.paddingXS : rects.last().bottom() + token.marginXS;
    return QRect(bar.left() + token.paddingXS, y, bar.width() - token.padding, side);
}

QRect AntTabs::closeRect(const QRect& tabRect) const
{
    const int side = 18;
    return QRect(tabRect.right() - side - 8, tabRect.center().y() - side / 2, side, side);
}

int AntTabs::tabAt(const QPoint& pos) const
{
    const auto rects = tabRects();
    for (int i = 0; i < rects.size(); ++i)
    {
        if (rects.at(i).contains(pos))
        {
            return i;
        }
    }
    return -1;
}

int AntTabs::closeAt(const QPoint& pos) const
{
    if (m_tabsType != Ant::TabsType::EditableCard)
    {
        return -1;
    }
    const auto rects = tabRects();
    for (int i = 0; i < rects.size(); ++i)
    {
        if (m_tabs.at(i).closable && closeRect(rects.at(i)).contains(pos))
        {
            return i;
        }
    }
    return -1;
}

bool AntTabs::isHorizontal() const
{
    return m_tabPlacement == Ant::TabsPlacement::Top || m_tabPlacement == Ant::TabsPlacement::Bottom;
}

int AntTabs::tabBarExtent() const
{
    const auto& token = antTheme->tokens();
    if (m_tabsType == Ant::TabsType::Card || m_tabsType == Ant::TabsType::EditableCard)
    {
        switch (m_tabsSize)
        {
        case Ant::TabsSize::Large:
            return token.controlHeightLG + 8;
        case Ant::TabsSize::Small:
            return token.controlHeight;
        case Ant::TabsSize::Middle:
        default:
            return token.controlHeightLG;
        }
    }
    switch (m_tabsSize)
    {
    case Ant::TabsSize::Large:
        return 56;
    case Ant::TabsSize::Small:
        return 38;
    case Ant::TabsSize::Middle:
    default:
        return 46;
    }
}

int AntTabs::tabPaddingX() const
{
    const auto& token = antTheme->tokens();
    switch (m_tabsSize)
    {
    case Ant::TabsSize::Large:
        return token.paddingLG;
    case Ant::TabsSize::Small:
        return token.paddingSM;
    case Ant::TabsSize::Middle:
    default:
        return token.padding;
    }
}

int AntTabs::tabFontSize() const
{
    const auto& token = antTheme->tokens();
    switch (m_tabsSize)
    {
    case Ant::TabsSize::Large:
        return token.fontSizeLG;
    case Ant::TabsSize::Small:
        return token.fontSize;
    case Ant::TabsSize::Middle:
    default:
        return token.fontSize;
    }
}

int AntTabs::tabLength(const AntTabItem& item) const
{
    QFont f = font();
    f.setPixelSize(tabFontSize());
    const int text = QFontMetrics(f).horizontalAdvance(item.label);
    const int icon = item.iconText.isEmpty() ? 0 : 22;
    const int close = (m_tabsType == Ant::TabsType::EditableCard && item.closable) ? 26 : 0;
    return std::max(72, text + icon + close + tabPaddingX() * 2);
}

QColor AntTabs::tabTextColor(const AntTabItem& item, bool active, bool hovered) const
{
    const auto& token = antTheme->tokens();
    if (item.disabled)
    {
        return token.colorTextDisabled;
    }
    if (active)
    {
        return token.colorPrimary;
    }
    if (hovered)
    {
        return token.colorPrimaryHover;
    }
    return token.colorTextSecondary;
}

QColor AntTabs::tabBackgroundColor(bool active, bool hovered) const
{
    const auto& token = antTheme->tokens();
    if (m_tabsType == Ant::TabsType::Line)
    {
        return hovered ? token.colorFillQuaternary : Qt::transparent;
    }
    if (active)
    {
        return token.colorBgContainer;
    }
    return hovered ? token.colorFillQuaternary : token.colorFillTertiary;
}

void AntTabs::setActiveIndex(int index)
{
    if (index < 0 || index >= m_tabs.size() || m_tabs.at(index).disabled)
    {
        return;
    }
    const QString key = m_tabs.at(index).key;
    if (m_activeKey == key)
    {
        return;
    }
    m_activeKey = key;
    m_stack->setCurrentWidget(m_tabs.at(index).page);
    update();
    Q_EMIT activeKeyChanged(m_activeKey);
    Q_EMIT currentChanged(index);
}

void AntTabs::updateStackGeometry()
{
    if (m_stack)
    {
        m_stack->setGeometry(pageRect().adjusted(0, 8, 0, 0));
    }
    updateGeometry();
}

void AntTabs::drawTab(QPainter& painter, const AntTabItem& item, const QRect& rect, bool active, bool hovered) const
{
    const auto& token = antTheme->tokens();
    QRect tab = rect;
    if (m_tabsType == Ant::TabsType::Line)
    {
        tab.adjust(2, 6, -2, -6);
    }
    else
    {
        tab.adjust(0, 4, 0, 0);
    }

    painter.setPen(m_tabsType == Ant::TabsType::Line ? Qt::NoPen : QPen(token.colorBorderSecondary, token.lineWidth));
    painter.setBrush(tabBackgroundColor(active, hovered));
    const int radius = m_tabsType == Ant::TabsType::Line ? token.borderRadius : token.borderRadiusLG;
    painter.drawRoundedRect(tab, radius, radius);

    if (active && m_tabsType == Ant::TabsType::Line)
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(token.colorPrimary);
        if (isHorizontal())
        {
            const int y = m_tabPlacement == Ant::TabsPlacement::Top ? rect.bottom() - 2 : rect.top();
            painter.drawRoundedRect(QRectF(rect.left() + tabPaddingX(), y, rect.width() - tabPaddingX() * 2, 3), 1.5, 1.5);
        }
        else
        {
            const int x = m_tabPlacement == Ant::TabsPlacement::Left ? rect.right() - 3 : rect.left();
            painter.drawRoundedRect(QRectF(x, rect.top() + 8, 3, rect.height() - 16), 1.5, 1.5);
        }
    }

    QFont textFont = painter.font();
    textFont.setPixelSize(tabFontSize());
    textFont.setWeight(active ? QFont::DemiBold : QFont::Normal);
    painter.setFont(textFont);
    painter.setPen(tabTextColor(item, active, hovered));

    QRect textRect = tab.adjusted(tabPaddingX(), 0, -tabPaddingX(), 0);
    if (!item.iconText.isEmpty())
    {
        painter.drawText(QRect(textRect.left(), textRect.top(), 18, textRect.height()), Qt::AlignCenter, item.iconText.left(2));
        textRect.adjust(24, 0, 0, 0);
    }
    if (m_tabsType == Ant::TabsType::EditableCard && item.closable)
    {
        textRect.adjust(0, 0, -24, 0);
    }
    painter.drawText(textRect, Qt::AlignCenter | Qt::TextSingleLine, item.label);

    if (m_tabsType == Ant::TabsType::EditableCard && item.closable)
    {
        const QRect close = closeRect(rect);
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_hoveredCloseIndex >= 0 && m_tabs.at(m_hoveredCloseIndex).key == item.key ? token.colorFillTertiary : Qt::transparent);
        painter.drawRoundedRect(close, token.borderRadiusSM, token.borderRadiusSM);
        painter.setPen(QPen(item.disabled ? token.colorTextDisabled : token.colorTextTertiary, 1.4, Qt::SolidLine, Qt::RoundCap));
        const QPoint c = close.center();
        painter.drawLine(QPoint(c.x() - 4, c.y() - 4), QPoint(c.x() + 4, c.y() + 4));
        painter.drawLine(QPoint(c.x() + 4, c.y() - 4), QPoint(c.x() - 4, c.y() + 4));
    }
}

void AntTabs::drawAddButton(QPainter& painter, const QRect& rect) const
{
    const auto& token = antTheme->tokens();
    painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
    painter.setBrush(m_addHovered ? token.colorFillQuaternary : Qt::transparent);
    painter.drawRoundedRect(rect.adjusted(2, 4, -2, -4), token.borderRadiusLG, token.borderRadiusLG);
    painter.setPen(QPen(token.colorTextSecondary, 1.8, Qt::SolidLine, Qt::RoundCap));
    const QPoint c = rect.center();
    painter.drawLine(QPoint(c.x() - 5, c.y()), QPoint(c.x() + 5, c.y()));
    painter.drawLine(QPoint(c.x(), c.y() - 5), QPoint(c.x(), c.y() + 5));
}
