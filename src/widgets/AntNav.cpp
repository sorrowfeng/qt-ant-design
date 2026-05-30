#include "AntNav.h"

#include <QFrame>
#include <QPalette>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "core/AntTheme.h"
#include "widgets/AntNavItem.h"
#include "widgets/AntScrollBar.h"
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

} // namespace

AntNav::AntNav(QWidget* parent)
    : AntWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBar(new AntScrollBar(Qt::Vertical, m_scrollArea));

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

    if (m_currentIndex >= index)
    {
        ++m_currentIndex;
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

    NavEntry entry = m_entries.takeAt(index);
    if (entry.item)
    {
        m_navLayout->removeWidget(entry.item);
        entry.item->deleteLater();
    }

    if (m_entries.isEmpty())
    {
        m_currentIndex = -1;
    }
    else if (previousIndex == index)
    {
        m_currentIndex = qMin(index, m_entries.size() - 1);
    }
    else if (previousIndex > index)
    {
        m_currentIndex = previousIndex - 1;
    }

    syncActiveItemStates();
    Q_EMIT countChanged(m_entries.size());
    emitCurrentChanged(previousText, previousData, previousIndex);
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

    Q_EMIT countChanged(0);
    emitCurrentChanged(previousText, previousData, previousIndex);
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
    if (index < -1 || index >= m_entries.size() || m_currentIndex == index)
    {
        return;
    }

    const int previousIndex = m_currentIndex;
    const QString previousText = currentText();
    const QVariant previousData = currentData();

    m_currentIndex = index;
    syncActiveItemStates();
    scrollToIndex(index);
    emitCurrentChanged(previousText, previousData, previousIndex);
    syncNavPerfCounters();
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
    for (int i = 0; i < m_entries.size(); ++i)
    {
        if (m_entries.at(i).item)
        {
            m_entries.at(i).item->setActive(i == m_currentIndex);
        }
    }
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
}
