#include "AntTransfer.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QVBoxLayout>

#include <algorithm>

#include "AntButton.h"
#include "core/AntStyleBase.h"
#include "core/AntTheme.h"

AntTransfer::AntTransfer(QWidget* parent)
    : QWidget(parent)
{
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
    m_toTargetBtn = new AntButton(QStringLiteral(">"));
    m_toTargetBtn->setButtonType(Ant::ButtonType::Default);
    m_toTargetBtn->setFixedSize(32, 32);
    btnCol->addWidget(m_toTargetBtn);
    m_toSourceBtn = new AntButton(QStringLiteral("<"));
    m_toSourceBtn->setButtonType(Ant::ButtonType::Default);
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
    update();
    Q_EMIT itemsChanged();
}

void AntTransfer::setTargetItems(const QStringList& items)
{
    m_targetList->clear();
    m_targetList->addItems(items);
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

    const int panelW = 180;
    const int panelH = 200;
    const int headerH = 40;
    const int buttonColW = 40;
    const QRect sourceRect(0, 0, panelW, panelH);
    const QRect targetRect(panelW + buttonColW, 0, panelW, panelH);

    auto drawCheckBox = [&](const QRect& box, bool checked) {
        AntStyleBase::drawCrispRoundedRect(&painter, box, QPen(token.colorBorder, token.lineWidth),
            token.colorBgContainer, token.borderRadiusSM, token.borderRadiusSM);
        if (checked)
        {
            painter.setPen(QPen(token.colorPrimary, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawLine(box.left() + 3, box.center().y(), box.center().x() - 1, box.bottom() - 4);
            painter.drawLine(box.center().x() - 1, box.bottom() - 4, box.right() - 3, box.top() + 4);
        }
    };

    auto drawPanel = [&](const QRect& rect, const QStringList& items) {
        AntStyleBase::drawCrispRoundedRect(&painter, rect.adjusted(0, 0, -1, -1),
            QPen(token.colorBorder, token.lineWidth), token.colorBgContainer,
            token.borderRadius, token.borderRadius);

        painter.setPen(QPen(token.colorSplit, token.lineWidth));
        painter.drawLine(rect.left(), rect.top() + headerH, rect.right(), rect.top() + headerH);

        drawCheckBox(QRect(rect.left() + 12, rect.top() + 12, 16, 16), false);
        QFont headerFont = painter.font();
        headerFont.setPixelSize(token.fontSize);
        painter.setFont(headerFont);
        painter.setPen(token.colorText);
        painter.drawText(QRect(rect.left() + 38, rect.top(), rect.width() - 50, headerH),
                         Qt::AlignVCenter | Qt::AlignLeft,
                         QStringLiteral("%1 item%2").arg(items.size()).arg(items.size() <= 1 ? QString() : QStringLiteral("s")));

        if (items.isEmpty())
        {
            painter.setPen(token.colorTextTertiary);
            painter.drawText(rect.adjusted(0, headerH, 0, 0), Qt::AlignCenter, QStringLiteral("No data"));
            return;
        }

        const int itemCount = static_cast<int>(items.size());
        const int visible = std::min(5, itemCount);
        for (int i = 0; i < visible; ++i)
        {
            const QRect row(rect.left(), rect.top() + headerH + i * 32, rect.width(), 32);
            drawCheckBox(QRect(row.left() + 12, row.top() + 8, 16, 16), false);
            painter.setPen(token.colorText);
            painter.drawText(QRect(row.left() + 38, row.top(), row.width() - 46, row.height()),
                             Qt::AlignVCenter | Qt::AlignLeft, items.at(i));
        }

        if (itemCount > visible)
        {
            const QRect scrollTrack(rect.right() - 9, rect.top() + headerH + 8, 4, rect.height() - headerH - 16);
            painter.setPen(Qt::NoPen);
            painter.setBrush(token.colorFillSecondary);
            painter.drawRoundedRect(scrollTrack, 2, 2);
            const int thumbH = std::max(30, scrollTrack.height() * visible / itemCount);
            painter.setBrush(token.colorTextDisabled);
            painter.drawRoundedRect(QRect(scrollTrack.left(), scrollTrack.top(), scrollTrack.width(), thumbH), 2, 2);
        }
    };

    drawPanel(sourceRect, sourceItems());
    drawPanel(targetRect, targetItems());

    auto drawArrowButton = [&](const QRect& rect, const QString& text) {
        AntStyleBase::drawCrispRoundedRect(&painter, rect, QPen(token.colorBorderDisabled, token.lineWidth),
            token.colorBgContainerDisabled, token.borderRadiusSM, token.borderRadiusSM);
        painter.setPen(token.colorTextDisabled);
        painter.drawText(rect, Qt::AlignCenter, text);
    };
    drawArrowButton(QRect(panelW + 8, 74, 24, 24), QStringLiteral(">"));
    drawArrowButton(QRect(panelW + 8, 106, 24, 24), QStringLiteral("<"));
}

void AntTransfer::doTransfer(bool toTarget)
{
    auto* from = toTarget ? m_sourceList : m_targetList;
    auto* to = toTarget ? m_targetList : m_sourceList;

    const auto selected = from->selectedItems();
    for (auto* item : selected)
    {
        to->addItem(item->text());
        delete from->takeItem(from->row(item));
    }
    Q_EMIT itemsChanged();
}

void AntTransfer::updateButtons()
{
    m_toTargetBtn->setEnabled(!m_sourceList->selectedItems().isEmpty());
    m_toSourceBtn->setEnabled(!m_targetList->selectedItems().isEmpty());
}
