#include "AntMenu.h"

#include <QKeyEvent>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include <algorithm>

#include "../styles/AntMenuStyle.h"
#include "AntIcon.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"

namespace
{
QColor withAlpha(QColor color, int alpha)
{
    color.setAlpha(alpha);
    return color;
}

bool hasIcon(const AntMenuItem& item)
{
    return item.iconType != Ant::IconType::None || !item.iconText.isEmpty();
}

void drawMenuIcon(QPainter& painter, const AntMenuItem& item, const QRectF& rect, const QColor& color)
{
    if (item.iconType == Ant::IconType::None)
    {
        painter.drawText(rect.toRect(), Qt::AlignCenter, item.iconText.left(2));
        return;
    }

    const AntIcon::IconPaths paths = AntIcon::builtinPaths(item.iconType, Ant::IconTheme::Outlined);
    const QPainterPath primaryPath = AntIcon::transformPath(paths.primary, rect);
    const QPainterPath secondaryPath = paths.secondary.isEmpty() ? QPainterPath() : AntIcon::transformPath(paths.secondary, rect);

    painter.save();
    if (paths.useStroke)
    {
        painter.setPen(QPen(color, qMax<qreal>(1.4, rect.width() * 0.09), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(primaryPath);
        if (!secondaryPath.isEmpty())
        {
            painter.drawPath(secondaryPath);
        }
    }
    else
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        if (!secondaryPath.isEmpty())
        {
            painter.drawPath(secondaryPath);
        }
        painter.drawPath(primaryPath);
    }
    painter.restore();
}
} // namespace

AntMenu::AntMenu(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntMenuStyle(style()));
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

}

Ant::MenuMode AntMenu::mode() const { return m_mode; }

void AntMenu::setMode(Ant::MenuMode mode)
{
    if (m_mode == mode)
    {
        return;
    }
    m_mode = mode;
    m_hoveredIndex = -1;
    updateMenuGeometry();
    update();
    Q_EMIT modeChanged(m_mode);
}

Ant::MenuTheme AntMenu::menuTheme() const { return m_menuTheme; }

void AntMenu::setMenuTheme(Ant::MenuTheme theme)
{
    if (m_menuTheme == theme)
    {
        return;
    }
    m_menuTheme = theme;
    update();
    Q_EMIT menuThemeChanged(m_menuTheme);
}

bool AntMenu::isInlineCollapsed() const { return m_inlineCollapsed; }

void AntMenu::setInlineCollapsed(bool collapsed)
{
    if (m_inlineCollapsed == collapsed)
    {
        return;
    }
    m_inlineCollapsed = collapsed;
    updateMenuGeometry();
    update();
    Q_EMIT inlineCollapsedChanged(m_inlineCollapsed);
}

bool AntMenu::isSelectable() const { return m_selectable; }

void AntMenu::setSelectable(bool selectable)
{
    if (m_selectable == selectable)
    {
        return;
    }
    m_selectable = selectable;
    update();
    Q_EMIT selectableChanged(m_selectable);
}

bool AntMenu::isCompact() const { return m_compact; }

void AntMenu::setCompact(bool compact)
{
    if (m_compact == compact)
    {
        return;
    }
    m_compact = compact;
    updateMenuGeometry();
    update();
}

QString AntMenu::selectedKey() const { return m_selectedKey; }

void AntMenu::setSelectedKey(const QString& key)
{
    if (m_selectedKey == key)
    {
        return;
    }
    m_selectedKey = key;
    update();
    Q_EMIT selectedKeyChanged(m_selectedKey);
}

QStringList AntMenu::openKeys() const { return m_openKeys; }

void AntMenu::setOpenKeys(const QStringList& keys)
{
    if (m_openKeys == keys)
    {
        return;
    }
    m_openKeys = keys;
    updateMenuGeometry();
    update();
    Q_EMIT openKeysChanged(m_openKeys);
}

int AntMenu::inlineIndent() const { return m_inlineIndent; }

void AntMenu::setInlineIndent(int indent)
{
    indent = std::max(8, indent);
    if (m_inlineIndent == indent)
    {
        return;
    }
    m_inlineIndent = indent;
    updateMenuGeometry();
    update();
    Q_EMIT inlineIndentChanged(m_inlineIndent);
}

void AntMenu::addItem(const QString& key,
                      const QString& label,
                      const QString& iconText,
                      const QString& extra,
                      bool disabled,
                      bool danger)
{
    AntMenuItem item;
    item.key = key;
    item.label = label;
    item.iconText = iconText;
    item.extra = extra;
    item.disabled = disabled;
    item.danger = danger;
    m_items.append(item);
    updateMenuGeometry();
    update();
}

void AntMenu::addItem(const QString& key,
                      const QString& label,
                      Ant::IconType iconType,
                      const QString& extra,
                      bool disabled,
                      bool danger)
{
    AntMenuItem item;
    item.key = key;
    item.label = label;
    item.iconType = iconType;
    item.extra = extra;
    item.disabled = disabled;
    item.danger = danger;
    m_items.append(item);
    updateMenuGeometry();
    update();
}

void AntMenu::addSubMenu(const QString& key, const QString& label, const QString& iconText, bool disabled)
{
    AntMenuItem item;
    item.key = key;
    item.label = label;
    item.iconText = iconText;
    item.disabled = disabled;
    item.subMenu = true;
    m_items.append(item);
    if (!m_openKeys.contains(key))
    {
        m_openKeys.append(key);
        Q_EMIT openKeysChanged(m_openKeys);
    }
    updateMenuGeometry();
    update();
}

void AntMenu::addSubMenu(const QString& key, const QString& label, Ant::IconType iconType, bool disabled)
{
    AntMenuItem item;
    item.key = key;
    item.label = label;
    item.iconType = iconType;
    item.disabled = disabled;
    item.subMenu = true;
    m_items.append(item);
    if (!m_openKeys.contains(key))
    {
        m_openKeys.append(key);
        Q_EMIT openKeysChanged(m_openKeys);
    }
    updateMenuGeometry();
    update();
}

void AntMenu::addSubItem(const QString& parentKey,
                         const QString& key,
                         const QString& label,
                         const QString& iconText,
                         const QString& extra,
                         bool disabled,
                         bool danger)
{
    AntMenuItem item;
    item.key = key;
    item.label = label;
    item.iconText = iconText;
    item.extra = extra;
    item.parentKey = parentKey;
    item.level = 1;
    item.disabled = disabled;
    item.danger = danger;
    m_items.append(item);
    updateMenuGeometry();
    update();
}

void AntMenu::addSubItem(const QString& parentKey,
                         const QString& key,
                         const QString& label,
                         Ant::IconType iconType,
                         const QString& extra,
                         bool disabled,
                         bool danger)
{
    AntMenuItem item;
    item.key = key;
    item.label = label;
    item.iconType = iconType;
    item.extra = extra;
    item.parentKey = parentKey;
    item.level = 1;
    item.disabled = disabled;
    item.danger = danger;
    m_items.append(item);
    updateMenuGeometry();
    update();
}

void AntMenu::addDivider()
{
    AntMenuItem item;
    item.divider = true;
    m_items.append(item);
    updateMenuGeometry();
    update();
}

void AntMenu::clearItems()
{
    m_items.clear();
    m_selectedKey.clear();
    m_openKeys.clear();
    m_hoveredIndex = -1;
    updateMenuGeometry();
    update();
}

QSize AntMenu::sizeHint() const
{
    const auto& token = antTheme->tokens();
    if (m_mode == Ant::MenuMode::Horizontal)
    {
        int width = token.paddingSM;
        for (const auto& item : m_items)
        {
            if (item.parentKey.isEmpty() && !item.divider)
            {
                width += horizontalItemWidth(item);
            }
        }
        return QSize(std::max(width, 240), 48);
    }

    const int width = m_inlineCollapsed ? 80 : (m_compact ? 120 : 240);
    const int verticalPadding = m_compact ? token.paddingXXS : token.paddingXS;
    const auto visible = visibleItems();
    const int rows = visible.count();
    int dividerCount = 0;
    for (const auto& item : visible)
    {
        if (m_items.at(item.index).divider)
        {
            ++dividerCount;
        }
    }
    const int dividerAdjustment = dividerCount * ((m_compact ? 8 : 12) - itemHeight());
    return QSize(width, std::max(itemHeight(), rows * itemHeight() + dividerAdjustment + verticalPadding * 2));
}

QSize AntMenu::minimumSizeHint() const
{
    return QSize(m_inlineCollapsed ? 64 : 160, itemHeight());
}

void AntMenu::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    const auto& token = antTheme->tokens();
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Background
    painter.fillRect(rect(), menuBackgroundColor());

    // Right/bottom separator line (for vertical/inline or horizontal bar)
    if (m_menuTheme != Ant::MenuTheme::Dark)
    {
        painter.setPen(QPen(token.colorSplit, token.lineWidth));
        if (m_mode == Ant::MenuMode::Horizontal)
        {
            painter.drawLine(rect().bottomLeft(), rect().bottomRight());
        }
    }

    // Items
    const auto visible = visibleItems();
    const int selectedIdx = selectedVisibleIndex();
    for (int i = 0; i < visible.count(); ++i)
    {
        const int itemIdx = visible.at(i).index;
        const AntMenuItem& item = m_items.at(itemIdx);
        const QRect r = visible.at(i).rect;
        if (item.divider)
        {
            painter.setPen(QPen(token.colorSplit, token.lineWidth));
            const int y = r.center().y();
            painter.drawLine(r.left() + token.paddingSM, y, r.right() - token.paddingSM, y);
            continue;
        }
        const bool selected = (i == selectedIdx);
        const bool hovered = (itemIdx == m_hoveredIndex);
        drawItem(painter, item, r, selected, hovered);
    }
}

void AntMenu::mouseMoveEvent(QMouseEvent* event)
{
    const int index = itemAt(event->pos());
    if (m_hoveredIndex != index)
    {
        m_hoveredIndex = index;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void AntMenu::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const int index = itemAt(event->pos());
        if (index >= 0)
        {
            activateItem(index);
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void AntMenu::leaveEvent(QEvent* event)
{
    if (m_hoveredIndex != -1)
    {
        m_hoveredIndex = -1;
        update();
    }
    QWidget::leaveEvent(event);
}

void AntMenu::keyPressEvent(QKeyEvent* event)
{
    const int current = selectedVisibleIndex();
    if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Right)
    {
        const int next = nextSelectableVisibleIndex(current, 1);
        if (next >= 0)
        {
            activateItem(next);
            event->accept();
            return;
        }
    }
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Left)
    {
        const int next = nextSelectableVisibleIndex(current, -1);
        if (next >= 0)
        {
            activateItem(next);
            event->accept();
            return;
        }
    }
    QWidget::keyPressEvent(event);
}

QList<AntMenu::VisibleItem> AntMenu::visibleItems() const
{
    const auto& token = antTheme->tokens();
    QList<VisibleItem> visible;
    if (m_mode == Ant::MenuMode::Horizontal)
    {
        int x = token.paddingXS;
        const int h = height() > 0 ? height() : 48;
        for (int i = 0; i < m_items.size(); ++i)
        {
            const AntMenuItem& item = m_items.at(i);
            if (!item.parentKey.isEmpty() || item.divider)
            {
                continue;
            }
            const int w = horizontalItemWidth(item);
            visible.append({i, QRect(x, 0, w, h)});
            x += w;
        }
        return visible;
    }

    int y = m_compact ? token.paddingXXS : token.paddingXS;
    const int w = width() > 0 ? width() : sizeHint().width();
    for (int i = 0; i < m_items.size(); ++i)
    {
        const AntMenuItem& item = m_items.at(i);
        if (!item.parentKey.isEmpty() && (!isOpen(item.parentKey) || m_inlineCollapsed))
        {
            continue;
        }
        const int h = item.divider ? (m_compact ? 8 : 12) : itemHeight();
        visible.append({i, QRect(0, y, w, h)});
        y += h;
    }
    return visible;
}

int AntMenu::itemAt(const QPoint& pos) const
{
    for (const auto& visible : visibleItems())
    {
        if (visible.rect.contains(pos))
        {
            const AntMenuItem& item = m_items.at(visible.index);
            return item.divider ? -1 : visible.index;
        }
    }
    return -1;
}

int AntMenu::selectedVisibleIndex() const
{
    const auto items = visibleItems();
    for (int i = 0; i < items.count(); ++i)
    {
        if (m_items.at(items.at(i).index).key == m_selectedKey)
        {
            return i;
        }
    }
    return -1;
}

int AntMenu::nextSelectableVisibleIndex(int from, int direction) const
{
    const auto items = visibleItems();
    if (items.isEmpty())
    {
        return -1;
    }
    int index = from < 0 ? (direction > 0 ? -1 : items.count()) : from;
    for (int step = 0; step < items.count(); ++step)
    {
        index = (index + direction + items.count()) % items.count();
        const AntMenuItem& item = m_items.at(items.at(index).index);
        if (!item.disabled && !item.divider && !item.subMenu)
        {
            return items.at(index).index;
        }
    }
    return -1;
}

bool AntMenu::isOpen(const QString& key) const
{
    return m_openKeys.contains(key);
}

void AntMenu::toggleOpen(const QString& key)
{
    QStringList keys = m_openKeys;
    if (keys.contains(key))
    {
        keys.removeAll(key);
    }
    else
    {
        keys.append(key);
    }
    setOpenKeys(keys);
}

void AntMenu::activateItem(int itemIndex)
{
    if (itemIndex < 0 || itemIndex >= m_items.size())
    {
        return;
    }
    const AntMenuItem& item = m_items.at(itemIndex);
    if (item.disabled || item.divider)
    {
        return;
    }

    if (item.subMenu)
    {
        toggleOpen(item.key);
        Q_EMIT itemClicked(item.key);
        return;
    }

    Q_EMIT itemClicked(item.key);
    if (m_selectable)
    {
        setSelectedKey(item.key);
        Q_EMIT itemSelected(item.key);
    }
}

int AntMenu::itemHeight() const
{
    if (m_mode == Ant::MenuMode::Horizontal)
    {
        return 48;
    }
    return m_compact ? 32 : 40;
}

int AntMenu::horizontalItemWidth(const AntMenuItem& item) const
{
    const auto& token = antTheme->tokens();
    QFont f = font();
    f.setPixelSize(token.fontSize);
    const int textWidth = QFontMetrics(f).horizontalAdvance(item.label);
    const int iconWidth = hasIcon(item) ? 22 : 0;
    const int arrowWidth = item.subMenu ? 18 : 0;
    return std::max(72, textWidth + iconWidth + arrowWidth + token.paddingLG * 2);
}

QRect AntMenu::itemContentRect(const QRect& rect, const AntMenuItem& item) const
{
    const auto& token = antTheme->tokens();
    if (m_mode == Ant::MenuMode::Horizontal)
    {
        return rect.adjusted(token.paddingSM, 6, -token.paddingSM, -6);
    }
    const int left = m_inlineCollapsed ? token.paddingSM
        : (m_compact ? token.paddingXXS : token.paddingSM) + item.level * m_inlineIndent;
    const int verticalInset = m_compact ? 3 : 4;
    return rect.adjusted(left, verticalInset, -token.paddingSM, -verticalInset);
}

QColor AntMenu::menuBackgroundColor() const
{
    if (m_menuTheme == Ant::MenuTheme::Dark)
    {
        return QColor(0, 21, 41);
    }
    return antTheme->tokens().colorBgContainer;
}

QColor AntMenu::itemTextColor(const AntMenuItem& item, bool selected, bool hovered) const
{
    const auto& token = antTheme->tokens();
    if (m_menuTheme == Ant::MenuTheme::Dark)
    {
        if (item.disabled)
        {
            return withAlpha(Qt::white, 64);
        }
        if (selected)
        {
            return token.colorTextLightSolid;
        }
        if (item.danger)
        {
            return hovered ? token.colorErrorHover : token.colorError;
        }
        return hovered ? token.colorTextLightSolid : withAlpha(Qt::white, 166);
    }

    if (item.disabled)
    {
        return token.colorTextDisabled;
    }
    if (item.danger)
    {
        return selected ? token.colorError : (hovered ? token.colorErrorHover : token.colorError);
    }
    if (selected || hovered)
    {
        return token.colorPrimary;
    }
    return token.colorText;
}

QColor AntMenu::itemBackgroundColor(const AntMenuItem& item, bool selected, bool hovered) const
{
    const auto& token = antTheme->tokens();
    if (item.disabled)
    {
        return Qt::transparent;
    }
    if (m_mode == Ant::MenuMode::Horizontal)
    {
        return hovered ? token.colorFillTertiary : Qt::transparent;
    }
    if (m_menuTheme == Ant::MenuTheme::Dark)
    {
        if (selected)
        {
            return item.danger ? token.colorError : token.colorPrimary;
        }
        return hovered ? QColor(17, 34, 51) : Qt::transparent;
    }
    if (selected)
    {
        return item.danger ? token.colorErrorBg : token.colorPrimaryBg;
    }
    return hovered ? token.colorFillTertiary : Qt::transparent;
}

void AntMenu::drawItem(QPainter& painter, const AntMenuItem& item, const QRect& rect, bool selected, bool hovered) const
{
    const auto& token = antTheme->tokens();
    const QRect content = itemContentRect(rect, item);
    const QColor bg = itemBackgroundColor(item, selected, hovered);
    painter.setPen(Qt::NoPen);
    painter.setBrush(bg);

    QRectF bgRect = content;
    if (m_mode == Ant::MenuMode::Horizontal)
    {
        bgRect.adjust(4, 7, -4, -7);
    }
    painter.drawRoundedRect(bgRect, token.borderRadiusLG, token.borderRadiusLG);

    if (selected && m_mode == Ant::MenuMode::Inline && m_menuTheme == Ant::MenuTheme::Light)
    {
        painter.setBrush(item.danger ? token.colorError : token.colorPrimary);
        painter.drawRoundedRect(QRectF(rect.right() - 3, content.top(), 3, content.height()), 1.5, 1.5);
    }
    if (selected && m_mode == Ant::MenuMode::Horizontal)
    {
        painter.setBrush(item.danger ? token.colorError : token.colorPrimary);
        painter.drawRoundedRect(QRectF(content.left() + token.paddingSM, rect.bottom() - 3, content.width() - token.paddingSM * 2, 3), 1.5, 1.5);
    }

    QFont textFont = painter.font();
    textFont.setPixelSize(token.fontSize);
    textFont.setWeight(selected ? QFont::DemiBold : QFont::Normal);
    painter.setFont(textFont);
    painter.setPen(itemTextColor(item, selected, hovered));

    int x = content.left() + token.paddingXS;
    const int iconSize = 20;
    if (hasIcon(item))
    {
        if (m_inlineCollapsed)
        {
            const QRectF centeredIcon(content.center().x() - 8, content.center().y() - 8, 16, 16);
            drawMenuIcon(painter, item, centeredIcon, itemTextColor(item, selected, hovered));
            return;
        }
        const QRectF iconRect(x + 2, content.center().y() - 8, 16, 16);
        drawMenuIcon(painter, item, iconRect, itemTextColor(item, selected, hovered));
        x += iconSize + token.marginXS;
    }
    else if (m_inlineCollapsed)
    {
        painter.drawText(content, Qt::AlignCenter, item.label.left(1).toUpper());
        return;
    }

    const int rightReserve = (item.subMenu ? 20 : 0) + (!item.extra.isEmpty() ? 48 : 0);
    painter.drawText(QRect(x, content.top(), content.right() - x - rightReserve, content.height()),
                     Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
                     item.label);

    if (!item.extra.isEmpty() && !m_inlineCollapsed)
    {
        painter.setPen(m_menuTheme == Ant::MenuTheme::Dark ? withAlpha(Qt::white, 105) : token.colorTextTertiary);
        painter.drawText(QRect(content.right() - 54, content.top(), 38, content.height()),
                         Qt::AlignRight | Qt::AlignVCenter | Qt::TextSingleLine,
                         item.extra);
    }

    if (item.subMenu && !m_inlineCollapsed)
    {
        painter.save();
        painter.setPen(QPen(itemTextColor(item, selected, hovered), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        const QPointF c(content.right() - 10, content.center().y());
        QPainterPath arrow;
        if (isOpen(item.key) && m_mode != Ant::MenuMode::Horizontal)
        {
            arrow.moveTo(c.x() - 5, c.y() - 2);
            arrow.lineTo(c.x(), c.y() + 3);
            arrow.lineTo(c.x() + 5, c.y() - 2);
        }
        else
        {
            arrow.moveTo(c.x() - 2, c.y() - 5);
            arrow.lineTo(c.x() + 3, c.y());
            arrow.lineTo(c.x() - 2, c.y() + 5);
        }
        painter.drawPath(arrow);
        painter.restore();
    }
}

void AntMenu::updateMenuGeometry()
{
    updateGeometry();
    if (parentWidget() && layout())
    {
        layout()->invalidate();
    }
}
