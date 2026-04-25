#include "AntTransfer.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

#include "AntButton.h"
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
    Q_EMIT itemsChanged();
}

void AntTransfer::setTargetItems(const QStringList& items)
{
    m_targetList->clear();
    m_targetList->addItems(items);
    Q_EMIT itemsChanged();
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
