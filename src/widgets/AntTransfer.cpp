#include "AntTransfer.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QWheelEvent>

#include <algorithm>
#include <utility>

#include "AntButton.h"
#include "core/AntStyleBase.h"
#include "core/AntTheme.h"
#include "styles/AntIconPainter.h"

namespace
{
constexpr int kPanelWidth = 180;
constexpr int kPanelHeight = 200;
constexpr int kButtonColumnWidth = 40;
constexpr int kHeaderHeight = 40;
constexpr int kRowHeight = 32;
} // namespace

AntTransfer::AntTransfer(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Source
    auto* sourceCol = new QVBoxLayout();
    sourceCol->addWidget(new QLabel(QStringLiteral("Source")));
    m_sourceList = new QListWidget();
    m_sourceList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_sourceList->setMinimumSize(180, 260);
    sourceCol->addWidget(m_sourceList);
    layout->addLayout(sourceCol);

    // Buttons
    auto* btnCol = new QVBoxLayout();
    btnCol->addStretch();
    m_toTargetBtn = new AntButton();
    m_toTargetBtn->setButtonType(Ant::ButtonType::Default);
    m_toTargetBtn->setButtonIconType(Ant::IconType::Right);
    m_toTargetBtn->setFixedSize(32, 32);
    btnCol->addWidget(m_toTargetBtn);
    m_toSourceBtn = new AntButton();
    m_toSourceBtn->setButtonType(Ant::ButtonType::Default);
    m_toSourceBtn->setButtonIconType(Ant::IconType::Left);
    m_toSourceBtn->setFixedSize(32, 32);
    btnCol->addWidget(m_toSourceBtn);
    btnCol->addStretch();
    layout->addLayout(btnCol);

    // Target
    auto* targetCol = new QVBoxLayout();
    targetCol->addWidget(new QLabel(QStringLiteral("Target")));
    m_targetList = new QListWidget();
    m_targetList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_targetList->setMinimumSize(180, 260);
    targetCol->addWidget(m_targetList);
    layout->addLayout(targetCol);

    connect(m_toTargetBtn, &QPushButton::clicked, this, [this]() { doTransfer(true); });
    connect(m_toSourceBtn, &QPushButton::clicked, this, [this]() { doTransfer(false); });

    connect(m_sourceList, &QListWidget::itemSelectionChanged, this, &AntTransfer::updateButtons);
    connect(m_targetList, &QListWidget::itemSelectionChanged, this, &AntTransfer::updateButtons);

    updateButtons();
    for (auto* label : findChildren<QLabel*>())
    {
        label->hide();
    }
    m_sourceList->hide();
    m_targetList->hide();
    m_toTargetBtn->hide();
    m_toSourceBtn->hide();
    setMinimumSize(minimumSizeHint());
    syncTransferPerfCounters();

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidatePanelLayouts();
        updateTransferRegion(rect(), QStringLiteral("theme"), false, true);
    });
}

QStringList AntTransfer::sourceItems() const
{
    return m_sourceItemsData;
}

QStringList AntTransfer::targetItems() const
{
    return m_targetItemsData;
}

void AntTransfer::setSourceItems(const QStringList& items)
{
    if (m_sourceItemsData == items)
    {
        return;
    }
    m_sourceItemsData = items;
    syncListWidget(true);
    const QStringList selectedSource = m_selectedSourceItems;
    for (const QString& selected : selectedSource)
    {
        if (!items.contains(selected))
        {
            m_selectedSourceItems.removeAll(selected);
        }
    }
    setScrollOffset(true, m_sourceScrollOffset);
    updateButtons();
    updateTransferRegion(panelRect(true).united(buttonColumnRect()), QStringLiteral("sourceItems"), false, true);
    Q_EMIT itemsChanged();
}

void AntTransfer::setTargetItems(const QStringList& items)
{
    if (m_targetItemsData == items)
    {
        return;
    }
    m_targetItemsData = items;
    syncListWidget(false);
    const QStringList selectedTarget = m_selectedTargetItems;
    for (const QString& selected : selectedTarget)
    {
        if (!items.contains(selected))
        {
            m_selectedTargetItems.removeAll(selected);
        }
    }
    setScrollOffset(false, m_targetScrollOffset);
    updateButtons();
    updateTransferRegion(panelRect(false).united(buttonColumnRect()), QStringLiteral("targetItems"), false, true);
    Q_EMIT itemsChanged();
}

QSize AntTransfer::sizeHint() const
{
    return QSize(412, 200);
}

QSize AntTransfer::minimumSizeHint() const
{
    return QSize(412, 200);
}

void AntTransfer::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    const auto& token = antTheme->tokens();
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    auto drawCheckBox = [&](const QRect& box, bool checked, bool partial = false) {
        AntStyleBase::drawCrispRoundedRect(&painter, box, QPen(token.colorBorder, token.lineWidth),
            token.colorBgContainer, token.borderRadiusSM, token.borderRadiusSM);
        if (checked)
        {
            AntStyleBase::drawCrispRoundedRect(&painter, box, Qt::NoPen,
                token.colorPrimary, token.borderRadiusSM, token.borderRadiusSM);
            painter.setPen(QPen(token.colorTextLightSolid, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            if (partial)
            {
                painter.drawLine(box.left() + 4, box.center().y(), box.right() - 4, box.center().y());
                return;
            }
            AntIconPainter::drawIcon(painter,
                                     Ant::IconType::Check,
                                     QRectF(box).adjusted(3, 3, -3, -3),
                                     token.colorTextLightSolid);
        }
    };

    auto drawPanel = [&](const PanelLayout& layout, const QStringList& items, const QStringList& selectedItems) {
        const QRect rect = layout.panelRect;
        AntStyleBase::drawCrispRoundedRect(&painter, rect.adjusted(0, 0, -1, -1),
            QPen(token.colorBorder, token.lineWidth), token.colorBgContainer,
            token.borderRadius, token.borderRadius);

        painter.setPen(QPen(token.colorSplit, token.lineWidth));
        painter.drawLine(rect.left(), rect.top() + kHeaderHeight, rect.right(), rect.top() + kHeaderHeight);

        const bool hasSelection = !selectedItems.isEmpty();
        const bool allSelected = !items.isEmpty() && selectedItems.size() >= items.size();
        drawCheckBox(QRect(rect.left() + 12, rect.top() + 12, 16, 16), hasSelection,
            hasSelection && !allSelected);
        QFont headerFont = painter.font();
        headerFont.setPixelSize(token.fontSize);
        painter.setFont(headerFont);
        painter.setPen(token.colorText);
        const int arrowX = rect.left() + 36;
        const int arrowY = rect.top() + 17;
        painter.drawLine(QPoint(arrowX, arrowY), QPoint(arrowX + 4, arrowY + 4));
        painter.drawLine(QPoint(arrowX + 4, arrowY + 4), QPoint(arrowX + 8, arrowY));

        const QString countText = hasSelection
            ? QStringLiteral("%1/%2 items").arg(selectedItems.size()).arg(items.size())
            : QStringLiteral("%1 item%2").arg(items.size()).arg(items.size() <= 1 ? QString() : QStringLiteral("s"));
        painter.drawText(QRect(rect.left() + 50, rect.top(), rect.width() - 62, kHeaderHeight),
                         Qt::AlignVCenter | Qt::AlignLeft, countText);

        if (items.isEmpty())
        {
            painter.setPen(token.colorTextTertiary);
            painter.drawText(rect.adjusted(0, kHeaderHeight, 0, 0), Qt::AlignCenter, QStringLiteral("No data"));
            return;
        }

        const int itemCount = static_cast<int>(items.size());
        for (const RowLayout& rowLayout : layout.visibleRows)
        {
            const int itemIndex = rowLayout.itemIndex;
            const QRect row = rowLayout.rowRect;
            const bool selected = selectedItems.contains(items.at(itemIndex));
            if (selected)
            {
                painter.setPen(Qt::NoPen);
                painter.setBrush(token.colorPrimaryBg);
                painter.drawRect(row.adjusted(1, 0, -1, 0));
            }
            drawCheckBox(rowLayout.checkRect, selected);
            painter.setPen(token.colorText);
            painter.drawText(QRect(row.left() + 38, row.top(), row.width() - 46, row.height()),
                             Qt::AlignVCenter | Qt::AlignLeft, items.at(itemIndex));
        }

        if (layout.hasScrollBar)
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(token.colorFillSecondary);
            painter.drawRoundedRect(layout.scrollTrackRect, 2, 2);
            painter.setBrush(token.colorTextDisabled);
            painter.drawRoundedRect(layout.scrollThumbRect, 2, 2);
        }
    };

    drawPanel(panelLayout(true), m_sourceItemsData, m_selectedSourceItems);
    drawPanel(panelLayout(false), m_targetItemsData, m_selectedTargetItems);

    auto drawArrowButton = [&](const QRect& rect, Ant::IconType iconType, bool enabled) {
        AntStyleBase::drawCrispRoundedRect(&painter, rect,
            QPen(enabled ? token.colorPrimary : token.colorBorderDisabled, token.lineWidth),
            enabled ? token.colorPrimary : token.colorBgContainerDisabled,
            token.borderRadiusSM, token.borderRadiusSM);
        const QColor iconColor = enabled ? token.colorTextLightSolid : token.colorTextDisabled;
        AntIconPainter::drawIcon(painter, iconType, QRectF(rect).adjusted(7, 7, -7, -7), iconColor);
    };
    drawArrowButton(buttonRect(true), Ant::IconType::Right, !m_selectedSourceItems.isEmpty());
    drawArrowButton(buttonRect(false), Ant::IconType::Left, !m_selectedTargetItems.isEmpty());
}

void AntTransfer::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        QWidget::mousePressEvent(event);
        return;
    }

    const QPoint pos = event->pos();
    if (headerCheckRect(true).contains(pos))
    {
        togglePanelSelection(true);
        event->accept();
        return;
    }
    if (headerCheckRect(false).contains(pos))
    {
        togglePanelSelection(false);
        event->accept();
        return;
    }

    const int sourceRow = rowAt(pos, true);
    if (sourceRow >= 0)
    {
        const QString item = m_sourceItemsData.at(sourceRow);
        if (m_selectedSourceItems.contains(item))
        {
            m_selectedSourceItems.removeAll(item);
        }
        else
        {
            m_selectedSourceItems.append(item);
        }
        updateButtons();
        updateTransferRegion(headerRect(true).united(rowRectForIndex(true, sourceRow)).united(buttonRect(true)),
                             QStringLiteral("selection"),
                             true);
        event->accept();
        return;
    }

    const int targetRow = rowAt(pos, false);
    if (targetRow >= 0)
    {
        const QString item = m_targetItemsData.at(targetRow);
        if (m_selectedTargetItems.contains(item))
        {
            m_selectedTargetItems.removeAll(item);
        }
        else
        {
            m_selectedTargetItems.append(item);
        }
        updateButtons();
        updateTransferRegion(headerRect(false).united(rowRectForIndex(false, targetRow)).united(buttonRect(false)),
                             QStringLiteral("selection"),
                             true);
        event->accept();
        return;
    }

    if (buttonRect(true).contains(pos) && !m_selectedSourceItems.isEmpty())
    {
        doTransfer(true);
        event->accept();
        return;
    }

    if (buttonRect(false).contains(pos) && !m_selectedTargetItems.isEmpty())
    {
        doTransfer(false);
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}

void AntTransfer::resizeEvent(QResizeEvent* event)
{
    invalidatePanelLayouts();
    QWidget::resizeEvent(event);
}

void AntTransfer::wheelEvent(QWheelEvent* event)
{
    const QPoint pos = event->position().toPoint();
    const bool inSource = panelRect(true).contains(pos);
    const bool inTarget = panelRect(false).contains(pos);
    if (!inSource && !inTarget)
    {
        QWidget::wheelEvent(event);
        return;
    }

    const bool sourcePanel = inSource;
    int steps = -event->angleDelta().y() / 120;
    if (steps == 0 && event->pixelDelta().y() != 0)
    {
        steps = event->pixelDelta().y() > 0 ? -1 : 1;
    }
    if (steps == 0)
    {
        QWidget::wheelEvent(event);
        return;
    }

    const int before = scrollOffset(sourcePanel);
    setScrollOffset(sourcePanel, before + steps);
    if (before != scrollOffset(sourcePanel))
    {
        updateTransferRegion(panelRect(sourcePanel), QStringLiteral("scroll"), false, true);
        event->accept();
        return;
    }
    QWidget::wheelEvent(event);
}

void AntTransfer::doTransfer(bool toTarget)
{
    QStringList fromItems = toTarget ? m_sourceItemsData : m_targetItemsData;
    QStringList toItems = toTarget ? m_targetItemsData : m_sourceItemsData;
    QStringList& selectedItems = toTarget ? m_selectedSourceItems : m_selectedTargetItems;

    for (const QString& item : std::as_const(selectedItems))
    {
        if (fromItems.removeOne(item))
        {
            toItems.append(item);
        }
    }
    selectedItems.clear();
    if (toTarget)
    {
        m_sourceItemsData = fromItems;
        m_targetItemsData = toItems;
        syncListWidget(true);
        syncListWidget(false);
    }
    else
    {
        m_targetItemsData = fromItems;
        m_sourceItemsData = toItems;
        syncListWidget(false);
        syncListWidget(true);
    }
    setScrollOffset(true, m_sourceScrollOffset);
    setScrollOffset(false, m_targetScrollOffset);
    updateButtons();
    updateTransferRegion(panelRect(true).united(panelRect(false)).united(buttonColumnRect()),
                         QStringLiteral("transfer"),
                         false,
                         true);
    Q_EMIT itemsChanged();
}

void AntTransfer::updateButtons()
{
    m_toTargetBtn->setEnabled(!m_selectedSourceItems.isEmpty());
    m_toSourceBtn->setEnabled(!m_selectedTargetItems.isEmpty());
}

int AntTransfer::rowAt(const QPoint& pos, bool sourcePanel) const
{
    const PanelLayout& layout = panelLayout(sourcePanel);
    if (!layout.bodyRect.contains(pos))
    {
        return -1;
    }
    for (const RowLayout& row : layout.visibleRows)
    {
        if (row.rowRect.contains(pos))
        {
            return row.itemIndex;
        }
    }
    return -1;
}

QRect AntTransfer::panelRect(bool sourcePanel) const
{
    return panelLayout(sourcePanel).panelRect;
}

QRect AntTransfer::headerRect(bool sourcePanel) const
{
    return panelLayout(sourcePanel).headerRect;
}

QRect AntTransfer::headerCheckRect(bool sourcePanel) const
{
    return panelLayout(sourcePanel).headerCheckRect;
}

QRect AntTransfer::rowRectForIndex(bool sourcePanel, int itemIndex) const
{
    const PanelLayout& layout = panelLayout(sourcePanel);
    for (const RowLayout& row : layout.visibleRows)
    {
        if (row.itemIndex == itemIndex)
        {
            return row.rowRect.adjusted(0, -1, 0, 1).intersected(layout.panelRect);
        }
    }
    return QRect();
}

QRect AntTransfer::buttonRect(bool toTarget) const
{
    return QRect(kPanelWidth + 8, toTarget ? 74 : 106, 24, 24);
}

QRect AntTransfer::buttonColumnRect() const
{
    return QRect(kPanelWidth, 0, kButtonColumnWidth, kPanelHeight);
}

int AntTransfer::visibleRowCount() const
{
    return (kPanelHeight - kHeaderHeight) / kRowHeight;
}

int AntTransfer::maxScrollOffset(bool sourcePanel) const
{
    const int count = panelItems(sourcePanel).size();
    return std::max(0, count - visibleRowCount());
}

int AntTransfer::scrollOffset(bool sourcePanel) const
{
    return sourcePanel ? m_sourceScrollOffset : m_targetScrollOffset;
}

void AntTransfer::setScrollOffset(bool sourcePanel, int offset)
{
    offset = std::clamp(offset, 0, maxScrollOffset(sourcePanel));
    if (sourcePanel)
    {
        if (m_sourceScrollOffset == offset)
        {
            return;
        }
        m_sourceScrollOffset = offset;
    }
    else
    {
        if (m_targetScrollOffset == offset)
        {
            return;
        }
        m_targetScrollOffset = offset;
    }
    invalidatePanelLayout(sourcePanel);
}

void AntTransfer::togglePanelSelection(bool sourcePanel)
{
    const QStringList items = panelItems(sourcePanel);
    QStringList& selectedItems = sourcePanel ? m_selectedSourceItems : m_selectedTargetItems;
    selectedItems = selectedItems.size() >= items.size() ? QStringList() : items;
    updateButtons();
    updateTransferRegion(panelRect(sourcePanel).united(buttonRect(sourcePanel)),
                         QStringLiteral("panelSelection"),
                         false,
                         true);
}

const QStringList& AntTransfer::panelItems(bool sourcePanel) const
{
    return sourcePanel ? m_sourceItemsData : m_targetItemsData;
}

void AntTransfer::syncListWidget(bool sourcePanel)
{
    QListWidget* list = sourcePanel ? m_sourceList : m_targetList;
    if (!list)
    {
        return;
    }
    list->clear();
    list->addItems(panelItems(sourcePanel));
    invalidatePanelLayout(sourcePanel);
}

const AntTransfer::PanelLayout& AntTransfer::panelLayout(bool sourcePanel) const
{
    PanelLayout& layout = sourcePanel ? m_sourceLayout : m_targetLayout;
    const QStringList& items = panelItems(sourcePanel);
    const int offset = scrollOffset(sourcePanel);
    if (layout.valid
        && layout.sourcePanel == sourcePanel
        && layout.widgetSize == size()
        && layout.itemCount == items.size()
        && layout.scrollOffset == offset)
    {
        ++m_layoutCacheHitCount;
        syncTransferPerfCounters();
        return layout;
    }

    layout.sourcePanel = sourcePanel;
    layout.widgetSize = size();
    layout.itemCount = items.size();
    layout.scrollOffset = offset;
    layout.panelRect = sourcePanel
        ? QRect(0, 0, kPanelWidth, kPanelHeight)
        : QRect(kPanelWidth + kButtonColumnWidth, 0, kPanelWidth, kPanelHeight);
    layout.headerRect = QRect(layout.panelRect.left(), layout.panelRect.top(), layout.panelRect.width(), kHeaderHeight);
    layout.headerCheckRect = QRect(layout.panelRect.left() + 8, layout.panelRect.top() + 8, 24, 24);
    layout.bodyRect = layout.panelRect.adjusted(0, kHeaderHeight, 0, 0);
    layout.visibleRows.clear();

    const int visible = std::max(0, std::min(visibleRowCount(), layout.itemCount - offset));
    layout.visibleRows.reserve(visible);
    for (int i = 0; i < visible; ++i)
    {
        RowLayout row;
        row.itemIndex = offset + i;
        row.rowRect = QRect(layout.panelRect.left(),
                            layout.panelRect.top() + kHeaderHeight + i * kRowHeight,
                            layout.panelRect.width(),
                            kRowHeight);
        row.checkRect = QRect(row.rowRect.left() + 12, row.rowRect.top() + 8, 16, 16);
        layout.visibleRows.append(row);
    }

    layout.hasScrollBar = layout.itemCount > visibleRowCount();
    layout.scrollTrackRect = QRect();
    layout.scrollThumbRect = QRect();
    if (layout.hasScrollBar)
    {
        layout.scrollTrackRect = QRect(layout.panelRect.right() - 9,
                                       layout.panelRect.top() + kHeaderHeight + 8,
                                       4,
                                       layout.panelRect.height() - kHeaderHeight - 16);
        const int thumbH = std::max(30, layout.scrollTrackRect.height() * visibleRowCount() / std::max(1, layout.itemCount));
        const int maxOffset = maxScrollOffset(sourcePanel);
        const int thumbTravel = std::max(0, layout.scrollTrackRect.height() - thumbH);
        const int thumbY = layout.scrollTrackRect.top() + (maxOffset <= 0 ? 0 : thumbTravel * offset / maxOffset);
        layout.scrollThumbRect = QRect(layout.scrollTrackRect.left(), thumbY, layout.scrollTrackRect.width(), thumbH);
    }

    layout.valid = true;
    ++m_layoutBuildCount;
    syncTransferPerfCounters();
    return layout;
}

void AntTransfer::invalidatePanelLayout(bool sourcePanel) const
{
    PanelLayout& layout = sourcePanel ? m_sourceLayout : m_targetLayout;
    layout.valid = false;
    syncTransferPerfCounters();
}

void AntTransfer::invalidatePanelLayouts() const
{
    m_sourceLayout.valid = false;
    m_targetLayout.valid = false;
    syncTransferPerfCounters();
}

void AntTransfer::updateTransferRegion(const QRect& dirty, const QString& mode, bool rowScoped, bool panelScoped)
{
    QRect updateRect = dirty;
    if (!updateRect.isValid() || updateRect.isEmpty())
    {
        updateRect = rect();
    }
    ++m_regionUpdateCount;
    if (rowScoped)
    {
        ++m_rowRegionUpdateCount;
    }
    if (panelScoped)
    {
        ++m_panelRegionUpdateCount;
    }
    setProperty("antTransferLastUpdateMode", mode);
    syncTransferPerfCounters();
    update(updateRect);
}

void AntTransfer::syncTransferPerfCounters() const
{
    auto* self = const_cast<AntTransfer*>(this);
    self->setProperty("antTransferLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antTransferLayoutCacheHitCount", m_layoutCacheHitCount);
    self->setProperty("antTransferRegionUpdateCount", m_regionUpdateCount);
    self->setProperty("antTransferRowRegionUpdateCount", m_rowRegionUpdateCount);
    self->setProperty("antTransferPanelRegionUpdateCount", m_panelRegionUpdateCount);
}
