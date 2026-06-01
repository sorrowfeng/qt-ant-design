#include "AntNav.h"

#include <algorithm>

#include <QFrame>
#include <QPalette>
#include <QVBoxLayout>
#include <QWidget>

#include "core/AntTheme.h"
#include "widgets/AntNavItem.h"
#include "widgets/AntScrollArea.h"
#include "widgets/AntTypography.h"

namespace
{

void applyNavSurfaceColors(QWidget* widget, const QColor& bg, const QColor& text)
{
    if (!widget)
    {
        return;
    }

    QPalette palette = widget->palette();
    palette.setColor(QPalette::Window, bg);
    palette.setColor(QPalette::Base, bg);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::WindowText, text);
    widget->setPalette(palette);
}

bool sameIndices(const QVector<int>& left, const QVector<int>& right)
{
    if (left.size() != right.size())
    {
        return false;
    }
    for (int i = 0; i < left.size(); ++i)
    {
        if (left.at(i) != right.at(i))
        {
            return false;
        }
    }
    return true;
}

} // namespace

AntNav::AntNav(QWidget* parent)
    : AntWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_scrollArea = new AntScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_scrollContent = new QWidget();
    m_navLayout = new QVBoxLayout(m_scrollContent);
    m_navLayout->setContentsMargins(0, 8, 0, 0);
    m_navLayout->setSpacing(0);
    m_navLayout->addStretch();

    m_scrollArea->setWidget(m_scrollContent);
    root->addWidget(m_scrollArea, 1);

    updateTheme();
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateTheme();
        update();
        if (m_scrollArea)
        {
            m_scrollArea->update();
            if (m_scrollArea->viewport())
            {
                m_scrollArea->viewport()->update();
            }
        }
        if (m_scrollContent)
        {
            m_scrollContent->update();
        }
    });

    syncNavPerfCounters();
}

int AntNav::addCategory(const QString& title)
{
    auto* header = new AntTypography(title.toUpper(), m_scrollContent);
    header->setTitle(true);
    header->setTitleLevel(Ant::TypographyTitleLevel::H5);
    header->setContentsMargins(20, 12, 20, 4);

    const int layoutIndex = qMax(0, m_navLayout->count() - 1);
    m_navLayout->insertWidget(layoutIndex, header);
    return layoutIndex;
}

int AntNav::addItem(const QString& text, const QVariant& data)
{
    return insertItem(m_entries.size(), text, data);
}

int AntNav::insertItem(int index, const QString& text, const QVariant& data)
{
    index = normalizedInsertIndex(index);
    auto* navItem = new AntNavItem(text, m_scrollContent);
    const int layoutIndex = layoutIndexForItemInsert(index);
    m_navLayout->insertWidget(layoutIndex, navItem);
    m_entries.insert(index, NavEntry{navItem, data});

    connect(navItem, &AntNavItem::clicked, this, [this, navItem]() {
        const int itemIndex = indexOfItem(navItem);
        if (itemIndex < 0)
        {
            return;
        }
        setCurrentIndex(itemIndex);
        Q_EMIT itemClicked(itemIndex);
    });
    connect(navItem, &AntNavItem::activeChanged, this, [this, navItem](bool active) {
        if (m_syncingItemStates)
        {
            return;
        }
        const int itemIndex = indexOfItem(navItem);
        if (itemIndex < 0)
        {
            return;
        }
        setItemSelected(itemIndex, active);
    });

    if (m_currentIndex >= index)
    {
        ++m_currentIndex;
    }
    for (int& selectedIndex : m_selectedIndices)
    {
        if (selectedIndex >= index)
        {
            ++selectedIndex;
        }
    }

    Q_EMIT countChanged(m_entries.size());
    if (m_currentIndex < 0)
    {
        setCurrentIndex(0);
    }
    else
    {
        syncActiveItemStates();
    }
    syncNavPerfCounters();
    return index;
}

void AntNav::removeItem(int index)
{
    if (index < 0 || index >= m_entries.size())
    {
        return;
    }

    const int previousIndex = m_currentIndex;
    const QString previousText = currentText();
    const QVariant previousData = currentData();
    const QVector<int> previousSelectedIndices = m_selectedIndices;

    NavEntry entry = m_entries.takeAt(index);
    if (entry.item)
    {
        m_navLayout->removeWidget(entry.item);
        entry.item->deleteLater();
    }

    if (m_entries.isEmpty())
    {
        m_currentIndex = -1;
        m_selectedIndices.clear();
    }
    else if (previousIndex == index)
    {
        m_currentIndex = qMin(index, m_entries.size() - 1);
    }
    else if (previousIndex > index)
    {
        m_currentIndex = previousIndex - 1;
    }

    QVector<int> nextSelectedIndices;
    for (int selectedIndex : previousSelectedIndices)
    {
        if (selectedIndex == index)
        {
            continue;
        }
        nextSelectedIndices.append(selectedIndex > index ? selectedIndex - 1 : selectedIndex);
    }
    if (m_currentIndex >= 0 && !nextSelectedIndices.contains(m_currentIndex))
    {
        if (m_multiple)
        {
            nextSelectedIndices.append(m_currentIndex);
        }
        else
        {
            nextSelectedIndices = QVector<int>{m_currentIndex};
        }
    }
    m_selectedIndices = normalizedSelectedIndices(nextSelectedIndices);
    syncActiveItemStates();
    Q_EMIT countChanged(m_entries.size());
    emitCurrentChanged(previousText, previousData, previousIndex);
    if (!sameIndices(previousSelectedIndices, m_selectedIndices))
    {
        Q_EMIT selectionChanged(m_selectedIndices);
    }
    syncNavPerfCounters();
}

void AntNav::clear()
{
    const int previousIndex = m_currentIndex;
    const QString previousText = currentText();
    const QVariant previousData = currentData();

    while (QLayoutItem* item = m_navLayout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
        {
            widget->deleteLater();
        }
        delete item;
    }
    m_navLayout->addStretch();

    m_entries.clear();
    m_currentIndex = -1;
    m_selectedIndices.clear();

    Q_EMIT countChanged(0);
    emitCurrentChanged(previousText, previousData, previousIndex);
    Q_EMIT selectionChanged(m_selectedIndices);
    syncNavPerfCounters();
}

int AntNav::count() const
{
    return m_entries.size();
}

AntNavItem* AntNav::item(int index) const
{
    if (index < 0 || index >= m_entries.size())
    {
        return nullptr;
    }
    return m_entries.at(index).item;
}

QString AntNav::itemText(int index) const
{
    AntNavItem* navItem = item(index);
    return navItem ? navItem->text() : QString();
}

void AntNav::setItemText(int index, const QString& text)
{
    AntNavItem* navItem = item(index);
    if (!navItem || navItem->text() == text)
    {
        return;
    }
    const QString previousText = currentText();
    const QVariant previousData = currentData();
    const int previousIndex = m_currentIndex;
    navItem->setText(text);
    emitCurrentChanged(previousText, previousData, previousIndex);
}

QVariant AntNav::itemData(int index) const
{
    if (index < 0 || index >= m_entries.size())
    {
        return {};
    }
    return m_entries.at(index).data;
}

void AntNav::setItemData(int index, const QVariant& data)
{
    if (index < 0 || index >= m_entries.size() || m_entries.at(index).data == data)
    {
        return;
    }
    const QString previousText = currentText();
    const QVariant previousData = currentData();
    const int previousIndex = m_currentIndex;
    m_entries[index].data = data;
    emitCurrentChanged(previousText, previousData, previousIndex);
}

QIcon AntNav::itemIcon(int index) const
{
    AntNavItem* navItem = item(index);
    return navItem ? navItem->icon() : QIcon();
}

void AntNav::setItemIcon(int index, const QIcon& icon)
{
    if (AntNavItem* navItem = item(index))
    {
        navItem->setIcon(icon);
    }
}

Ant::IconType AntNav::itemIconType(int index) const
{
    AntNavItem* navItem = item(index);
    return navItem ? navItem->iconType() : Ant::IconType::None;
}

void AntNav::setItemIcon(int index, Ant::IconType iconType, Ant::IconTheme theme)
{
    if (AntNavItem* navItem = item(index))
    {
        navItem->setIcon(iconType, theme);
    }
}

QString AntNav::itemIconName(int index) const
{
    AntNavItem* navItem = item(index);
    return navItem ? navItem->iconName() : QString();
}

void AntNav::setItemIconName(int index, const QString& iconName, Ant::IconTheme theme)
{
    if (AntNavItem* navItem = item(index))
    {
        navItem->setIconName(iconName, theme);
    }
}

Ant::IconTheme AntNav::itemIconTheme(int index) const
{
    AntNavItem* navItem = item(index);
    return navItem ? navItem->iconTheme() : Ant::IconTheme::Outlined;
}

void AntNav::setItemIconTheme(int index, Ant::IconTheme theme)
{
    if (AntNavItem* navItem = item(index))
    {
        navItem->setIconTheme(theme);
    }
}

QColor AntNav::itemIconColor(int index) const
{
    AntNavItem* navItem = item(index);
    return navItem ? navItem->iconColor() : QColor();
}

void AntNav::setItemIconColor(int index, const QColor& color)
{
    if (AntNavItem* navItem = item(index))
    {
        navItem->setIconColor(color);
    }
}

QColor AntNav::itemIconTwoToneColor(int index) const
{
    AntNavItem* navItem = item(index);
    return navItem ? navItem->iconTwoToneColor() : QColor();
}

void AntNav::setItemIconTwoToneColor(int index, const QColor& color)
{
    if (AntNavItem* navItem = item(index))
    {
        navItem->setIconTwoToneColor(color);
    }
}

QPixmap AntNav::itemIconPixmap(int index) const
{
    AntNavItem* navItem = item(index);
    return navItem ? navItem->iconPixmap() : QPixmap();
}

void AntNav::setItemIconPixmap(int index, const QPixmap& pixmap)
{
    if (AntNavItem* navItem = item(index))
    {
        navItem->setIconPixmap(pixmap);
    }
}

QImage AntNav::itemIconImage(int index) const
{
    AntNavItem* navItem = item(index);
    return navItem ? navItem->iconImage() : QImage();
}

void AntNav::setItemIconImage(int index, const QImage& image)
{
    if (AntNavItem* navItem = item(index))
    {
        navItem->setIconImage(image);
    }
}

QSize AntNav::itemIconSize(int index) const
{
    AntNavItem* navItem = item(index);
    return navItem ? navItem->iconSize() : QSize();
}

void AntNav::setItemIconSize(int index, const QSize& size)
{
    if (AntNavItem* navItem = item(index))
    {
        navItem->setIconSize(size);
    }
}

bool AntNav::itemHasIcon(int index) const
{
    AntNavItem* navItem = item(index);
    return navItem ? navItem->hasIcon() : false;
}

void AntNav::clearItemIcon(int index)
{
    if (AntNavItem* navItem = item(index))
    {
        navItem->clearIcon();
    }
}

int AntNav::currentIndex() const
{
    return m_currentIndex;
}

QString AntNav::currentText() const
{
    return itemText(m_currentIndex);
}

QVariant AntNav::currentData() const
{
    return itemData(m_currentIndex);
}

bool AntNav::isMultiple() const
{
    return m_multiple;
}

QVector<int> AntNav::selectedIndices() const
{
    return m_selectedIndices;
}

bool AntNav::isItemSelected(int index) const
{
    return m_selectedIndices.contains(index);
}

QSize AntNav::sizeHint() const
{
    return QSize(220, 320);
}

QSize AntNav::minimumSizeHint() const
{
    return QSize(120, 80);
}

void AntNav::setCurrentIndex(int index)
{
    if (index < -1 || index >= m_entries.size())
    {
        return;
    }

    if (index < 0)
    {
        applySelection({}, -1);
        return;
    }

    QVector<int> nextSelectedIndices = m_multiple ? m_selectedIndices : QVector<int>();
    if (!nextSelectedIndices.contains(index))
    {
        nextSelectedIndices.append(index);
    }
    applySelection(nextSelectedIndices, index);
}

void AntNav::setMultiple(bool multiple)
{
    if (m_multiple == multiple)
    {
        return;
    }

    m_multiple = multiple;
    if (!m_multiple && m_selectedIndices.size() > 1)
    {
        const int keptIndex = m_selectedIndices.contains(m_currentIndex) ? m_currentIndex : m_selectedIndices.first();
        applySelection(QVector<int>{keptIndex}, keptIndex, false);
    }
    syncNavPerfCounters();
    Q_EMIT multipleChanged(m_multiple);
}

void AntNav::setSelectedIndices(const QVector<int>& indices)
{
    QVector<int> nextSelectedIndices = normalizedSelectedIndices(indices);
    int nextCurrentIndex = -1;
    if (!nextSelectedIndices.isEmpty())
    {
        nextCurrentIndex = nextSelectedIndices.contains(m_currentIndex) ? m_currentIndex : nextSelectedIndices.first();
    }
    applySelection(nextSelectedIndices, nextCurrentIndex);
}

void AntNav::setItemSelected(int index, bool selected)
{
    if (index < 0 || index >= m_entries.size())
    {
        return;
    }

    QVector<int> nextSelectedIndices = m_multiple ? m_selectedIndices : QVector<int>();
    if (selected)
    {
        if (!nextSelectedIndices.contains(index))
        {
            nextSelectedIndices.append(index);
        }
        applySelection(nextSelectedIndices, index);
        return;
    }

    nextSelectedIndices.removeAll(index);
    const int nextCurrentIndex = m_currentIndex == index
        ? (nextSelectedIndices.isEmpty() ? -1 : nextSelectedIndices.first())
        : m_currentIndex;
    applySelection(nextSelectedIndices, nextCurrentIndex);
}

void AntNav::clearSelection()
{
    applySelection({}, -1);
}

void AntNav::scrollToIndex(int index)
{
    AntNavItem* navItem = item(index);
    if (m_scrollArea && navItem)
    {
        m_scrollArea->ensureWidgetVisible(navItem, 0, 8);
    }
}

int AntNav::normalizedInsertIndex(int index) const
{
    return qBound(0, index, m_entries.size());
}

int AntNav::layoutIndexForItemInsert(int index) const
{
    if (index >= 0 && index < m_entries.size())
    {
        const int layoutIndex = m_navLayout->indexOf(m_entries.at(index).item);
        if (layoutIndex >= 0)
        {
            return layoutIndex;
        }
    }
    return qMax(0, m_navLayout->count() - 1);
}

int AntNav::indexOfItem(AntNavItem* item) const
{
    for (int i = 0; i < m_entries.size(); ++i)
    {
        if (m_entries.at(i).item == item)
        {
            return i;
        }
    }
    return -1;
}

void AntNav::updateTheme()
{
    const auto& token = antTheme->tokens();
    const QColor bg = token.colorBgContainer;
    const QColor text = token.colorText;

    setAutoFillBackground(false);
    applyNavSurfaceColors(this, bg, text);

    if (m_scrollArea)
    {
        m_scrollArea->setAutoFillBackground(false);
        applyNavSurfaceColors(m_scrollArea, bg, text);

        if (QWidget* viewport = m_scrollArea->viewport())
        {
            viewport->setAutoFillBackground(true);
            applyNavSurfaceColors(viewport, bg, text);
        }
    }

    if (m_scrollContent)
    {
        m_scrollContent->setAutoFillBackground(true);
        applyNavSurfaceColors(m_scrollContent, bg, text);
    }
}

void AntNav::syncActiveItemStates()
{
    ++m_selectionApplyCount;
    m_syncingItemStates = true;
    for (int i = 0; i < m_entries.size(); ++i)
    {
        if (m_entries.at(i).item)
        {
            m_entries.at(i).item->setActive(m_selectedIndices.contains(i));
        }
    }
    m_syncingItemStates = false;
}

QVector<int> AntNav::normalizedSelectedIndices(const QVector<int>& indices) const
{
    QVector<int> normalized;
    normalized.reserve(indices.size());
    for (int index : indices)
    {
        if (index >= 0 && index < m_entries.size() && !normalized.contains(index))
        {
            normalized.append(index);
        }
    }
    std::sort(normalized.begin(), normalized.end());
    if (!m_multiple && normalized.size() > 1)
    {
        normalized = QVector<int>{normalized.first()};
    }
    return normalized;
}

void AntNav::applySelection(const QVector<int>& indices, int currentIndex, bool scrollCurrent)
{
    QVector<int> nextSelectedIndices = normalizedSelectedIndices(indices);
    int nextCurrentIndex = (currentIndex >= 0 && currentIndex < m_entries.size()) ? currentIndex : -1;

    if (nextCurrentIndex >= 0 && !nextSelectedIndices.contains(nextCurrentIndex))
    {
        if (m_multiple)
        {
            nextSelectedIndices.append(nextCurrentIndex);
            std::sort(nextSelectedIndices.begin(), nextSelectedIndices.end());
        }
        else
        {
            nextSelectedIndices = QVector<int>{nextCurrentIndex};
        }
    }

    if (nextCurrentIndex < 0 && !nextSelectedIndices.isEmpty())
    {
        nextCurrentIndex = nextSelectedIndices.first();
    }
    if (nextSelectedIndices.isEmpty())
    {
        nextCurrentIndex = -1;
    }

    if (m_currentIndex == nextCurrentIndex && sameIndices(m_selectedIndices, nextSelectedIndices))
    {
        return;
    }

    const int previousIndex = m_currentIndex;
    const QString previousText = currentText();
    const QVariant previousData = currentData();
    const QVector<int> previousSelectedIndices = m_selectedIndices;

    m_currentIndex = nextCurrentIndex;
    m_selectedIndices = nextSelectedIndices;
    syncActiveItemStates();
    if (scrollCurrent)
    {
        scrollToIndex(m_currentIndex);
    }
    emitCurrentChanged(previousText, previousData, previousIndex);
    if (!sameIndices(previousSelectedIndices, m_selectedIndices))
    {
        Q_EMIT selectionChanged(m_selectedIndices);
    }
    syncNavPerfCounters();
}

void AntNav::emitCurrentChanged(const QString& previousText, const QVariant& previousData, int previousIndex)
{
    if (previousIndex != m_currentIndex)
    {
        Q_EMIT currentIndexChanged(m_currentIndex);
    }
    if (previousText != currentText())
    {
        Q_EMIT currentTextChanged(currentText());
    }
    if (previousData != currentData())
    {
        Q_EMIT currentDataChanged(currentData());
    }
}

void AntNav::syncNavPerfCounters() const
{
    auto* self = const_cast<AntNav*>(this);
    self->setProperty("antNavSelectionApplyCount", m_selectionApplyCount);
    self->setProperty("antNavItemCount", m_entries.size());
    self->setProperty("antNavCurrentIndex", m_currentIndex);
    QVariantList selected;
    selected.reserve(m_selectedIndices.size());
    for (int index : m_selectedIndices)
    {
        selected.append(index);
    }
    self->setProperty("antNavSelectedIndices", selected);
    self->setProperty("antNavMultiple", m_multiple);
}
