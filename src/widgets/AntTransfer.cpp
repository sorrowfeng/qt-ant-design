#include "AntTransfer.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
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

    connect(antTheme, &AntTheme::themeChanged, this, [this]() { update(); });
}

QStringList AntTransfer::sourceItems() const
{
    QStringList items;
    for (int i = 0; i < m_sourceList->count(); ++i)
        items << m_sourceList->item(i)->text();
    return items;
}

QStringList AntTransfer::targetItems() const
{
    QStringList items;
    for (int i = 0; i < m_targetList->count(); ++i)
        items << m_targetList->item(i)->text();
    return items;
}

void AntTransfer::setSourceItems(const QStringList& items)
{
    m_sourceList->clear();
    m_sourceList->addItems(items);
    setScrollOffset(true, m_sourceScrollOffset);
    const QStringList selectedSource = m_selectedSourceItems;
    for (const QString& selected : selectedSource)
    {
        if (!items.contains(selected))
        {
            m_selectedSourceItems.removeAll(selected);
        }
    }
    update();
    Q_EMIT itemsChanged();
}

void AntTransfer::setTargetItems(const QStringList& items)
{
    m_targetList->clear();
    m_targetList->addItems(items);
    setScrollOffset(false, m_targetScrollOffset);
    const QStringList selectedTarget = m_selectedTargetItems;
    for (const QString& selected : selectedTarget)
    {
        if (!items.contains(selected))
        {
            m_selectedTargetItems.removeAll(selected);
        }
    }
    update();
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

    const QRect sourceRect = panelRect(true);
    const QRect targetRect = panelRect(false);

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

    auto drawPanel = [&](const QRect& rect, const QStringList& items, const QStringList& selectedItems) {
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
        const bool sourcePanel = rect.left() == sourceRect.left();
        const int offset = scrollOffset(sourcePanel);
        const int visible = std::min(visibleRowCount(), itemCount - offset);
        for (int i = 0; i < visible; ++i)
        {
            const int itemIndex = offset + i;
            const QRect row(rect.left(), rect.top() + kHeaderHeight + i * kRowHeight, rect.width(), kRowHeight);
            const bool selected = selectedItems.contains(items.at(itemIndex));
            if (selected)
            {
                painter.setPen(Qt::NoPen);
                painter.setBrush(token.colorPrimaryBg);
                painter.drawRect(row.adjusted(1, 0, -1, 0));
            }
            drawCheckBox(QRect(row.left() + 12, row.top() + 8, 16, 16), selected);
            painter.setPen(token.colorText);
            painter.drawText(QRect(row.left() + 38, row.top(), row.width() - 46, row.height()),
                             Qt::AlignVCenter | Qt::AlignLeft, items.at(itemIndex));
        }

        if (itemCount > visibleRowCount())
        {
            const QRect scrollTrack(rect.right() - 9, rect.top() + kHeaderHeight + 8, 4, rect.height() - kHeaderHeight - 16);
            painter.setPen(Qt::NoPen);
            painter.setBrush(token.colorFillSecondary);
            painter.drawRoundedRect(scrollTrack, 2, 2);
            const int thumbH = std::max(30, scrollTrack.height() * visibleRowCount() / itemCount);
            const int maxOffset = maxScrollOffset(sourcePanel);
            const int thumbTravel = std::max(0, scrollTrack.height() - thumbH);
            const int thumbY = scrollTrack.top() + (maxOffset <= 0 ? 0 : thumbTravel * offset / maxOffset);
            painter.setBrush(token.colorTextDisabled);
            painter.drawRoundedRect(QRect(scrollTrack.left(), thumbY, scrollTrack.width(), thumbH), 2, 2);
        }
    };

    drawPanel(sourceRect, sourceItems(), m_selectedSourceItems);
    drawPanel(targetRect, targetItems(), m_selectedTargetItems);

    auto drawArrowButton = [&](const QRect& rect, Ant::IconType iconType, bool enabled) {
        AntStyleBase::drawCrispRoundedRect(&painter, rect,
            QPen(enabled ? token.colorPrimary : token.colorBorderDisabled, token.lineWidth),
            enabled ? token.colorPrimary : token.colorBgContainerDisabled,
            token.borderRadiusSM, token.borderRadiusSM);
        const QColor iconColor = enabled ? token.colorTextLightSolid : token.colorTextDisabled;
        AntIconPainter::drawIcon(painter, iconType, QRectF(rect).adjusted(7, 7, -7, -7), iconColor);
    };
    drawArrowButton(QRect(kPanelWidth + 8, 74, 24, 24), Ant::IconType::Right, !m_selectedSourceItems.isEmpty());
    drawArrowButton(QRect(kPanelWidth + 8, 106, 24, 24), Ant::IconType::Left, !m_selectedTargetItems.isEmpty());
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
        const QString item = sourceItems().at(sourceRow);
        if (m_selectedSourceItems.contains(item))
        {
            m_selectedSourceItems.removeAll(item);
        }
        else
        {
            m_selectedSourceItems.append(item);
        }
        updateButtons();
        update();
        event->accept();
        return;
    }

    const int targetRow = rowAt(pos, false);
    if (targetRow >= 0)
    {
        const QString item = targetItems().at(targetRow);
        if (m_selectedTargetItems.contains(item))
        {
            m_selectedTargetItems.removeAll(item);
        }
        else
        {
            m_selectedTargetItems.append(item);
        }
        updateButtons();
        update();
        event->accept();
        return;
    }

    if (QRect(188, 74, 24, 24).contains(pos) && !m_selectedSourceItems.isEmpty())
    {
        doTransfer(true);
        event->accept();
        return;
    }

    if (QRect(188, 106, 24, 24).contains(pos) && !m_selectedTargetItems.isEmpty())
    {
        doTransfer(false);
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
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
        update();
        event->accept();
        return;
    }
    QWidget::wheelEvent(event);
}

void AntTransfer::doTransfer(bool toTarget)
{
    QStringList fromItems = toTarget ? sourceItems() : targetItems();
    QStringList toItems = toTarget ? targetItems() : sourceItems();
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
        m_sourceList->clear();
        m_sourceList->addItems(fromItems);
        m_targetList->clear();
        m_targetList->addItems(toItems);
    }
    else
    {
        m_targetList->clear();
        m_targetList->addItems(fromItems);
        m_sourceList->clear();
        m_sourceList->addItems(toItems);
    }
    setScrollOffset(true, m_sourceScrollOffset);
    setScrollOffset(false, m_targetScrollOffset);
    updateButtons();
    update();
    Q_EMIT itemsChanged();
}

void AntTransfer::updateButtons()
{
    m_toTargetBtn->setEnabled(!m_selectedSourceItems.isEmpty());
    m_toSourceBtn->setEnabled(!m_selectedTargetItems.isEmpty());
}

int AntTransfer::rowAt(const QPoint& pos, bool sourcePanel) const
{
    const QRect rect = panelRect(sourcePanel);
    if (!rect.contains(pos) || pos.y() < rect.top() + kHeaderHeight)
    {
        return -1;
    }
    const int row = (pos.y() - rect.top() - kHeaderHeight) / kRowHeight;
    const int count = sourcePanel ? sourceItems().size() : targetItems().size();
    const int itemIndex = scrollOffset(sourcePanel) + row;
    return row >= 0 && row < visibleRowCount() && itemIndex < count ? itemIndex : -1;
}

QRect AntTransfer::panelRect(bool sourcePanel) const
{
    return sourcePanel ? QRect(0, 0, kPanelWidth, kPanelHeight) : QRect(kPanelWidth + kButtonColumnWidth, 0, kPanelWidth, kPanelHeight);
}

QRect AntTransfer::headerCheckRect(bool sourcePanel) const
{
    const QRect rect = panelRect(sourcePanel);
    return QRect(rect.left() + 8, rect.top() + 8, 24, 24);
}

int AntTransfer::visibleRowCount() const
{
    return (kPanelHeight - kHeaderHeight) / kRowHeight;
}

int AntTransfer::maxScrollOffset(bool sourcePanel) const
{
    const int count = sourcePanel ? sourceItems().size() : targetItems().size();
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
        m_sourceScrollOffset = offset;
    }
    else
    {
        m_targetScrollOffset = offset;
    }
}

void AntTransfer::togglePanelSelection(bool sourcePanel)
{
    const QStringList items = sourcePanel ? sourceItems() : targetItems();
    QStringList& selectedItems = sourcePanel ? m_selectedSourceItems : m_selectedTargetItems;
    selectedItems = selectedItems.size() >= items.size() ? QStringList() : items;
    updateButtons();
    update();
}
