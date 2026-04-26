#include "AntForm.h"

#include <QBoxLayout>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <utility>
#include <QVBoxLayout>
#include <functional>

#include "../styles/AntFormStyle.h"
#include "core/AntTheme.h"

// ── AntFormProvider ──

AntFormProvider::AntFormProvider(QWidget* parent)
    : QWidget(parent)
{
}

void AntFormProvider::addForm(AntForm* form, const QString& name)
{
    if (!form)
        return;
    for (const auto& entry : m_forms)
    {
        if (entry.form == form)
            return;
    }
    FormEntry entry;
    entry.form = form;
    entry.name = name.isEmpty() ? QString::number(m_forms.size()) : name;
    m_forms.append(entry);
}

void AntFormProvider::removeForm(AntForm* form)
{
    for (int i = 0; i < m_forms.size(); ++i)
    {
        if (m_forms[i].form == form)
        {
            m_forms.removeAt(i);
            return;
        }
    }
}

QList<AntForm*> AntFormProvider::forms() const
{
    QList<AntForm*> result;
    for (const auto& entry : m_forms)
        result.append(entry.form);
    return result;
}

// ── AntFormItem ──

AntFormItem::AntFormItem(QWidget* parent)
    : QWidget(parent)
{
    m_labelContainer = new QWidget(this);
    auto* labelLayout = new QHBoxLayout(m_labelContainer);
    labelLayout->setContentsMargins(0, 0, 0, 0);
    labelLayout->setSpacing(4);

    m_requiredLabel = new QLabel(QStringLiteral("*"), m_labelContainer);
    m_labelWidget = new QLabel(m_labelContainer);
    m_labelWidget->setWordWrap(true);
    labelLayout->addStretch();
    labelLayout->addWidget(m_requiredLabel);
    labelLayout->addWidget(m_labelWidget);

    m_fieldColumn = new QWidget(this);
    m_extraLabel = new QLabel(m_fieldColumn);
    m_helpLabel = new QLabel(m_fieldColumn);
    m_extraLabel->setWordWrap(true);
    m_helpLabel->setWordWrap(true);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        rebuildLayout();
        updateTheme();
    });

    rebuildLayout();
    updateTheme();
}

QString AntFormItem::label() const { return m_label; }

void AntFormItem::setLabel(const QString& label)
{
    if (m_label == label)
    {
        return;
    }
    m_label = label;
    rebuildLayout();
    updateTheme();
    Q_EMIT labelChanged(m_label);
}

QString AntFormItem::helpText() const { return m_helpText; }

void AntFormItem::setHelpText(const QString& text)
{
    if (m_helpText == text)
    {
        return;
    }
    m_helpText = text;
    rebuildLayout();
    updateTheme();
    Q_EMIT helpTextChanged(m_helpText);
}

QString AntFormItem::extra() const { return m_extra; }

void AntFormItem::setExtra(const QString& text)
{
    if (m_extra == text)
    {
        return;
    }
    m_extra = text;
    rebuildLayout();
    updateTheme();
    Q_EMIT extraChanged(m_extra);
}

bool AntFormItem::isRequired() const { return m_required; }

void AntFormItem::setRequired(bool required)
{
    if (m_required == required)
    {
        return;
    }
    m_required = required;
    rebuildLayout();
    updateTheme();
    Q_EMIT requiredChanged(m_required);
}

bool AntFormItem::colon() const { return m_colon; }

void AntFormItem::setColon(bool colon)
{
    if (!m_useFormColon && m_colon == colon)
    {
        return;
    }
    m_useFormColon = false;
    m_colon = colon;
    rebuildLayout();
    updateTheme();
    Q_EMIT colonChanged(m_colon);
}

Ant::Status AntFormItem::validateStatus() const { return m_validateStatus; }

void AntFormItem::setValidateStatus(Ant::Status status)
{
    if (m_validateStatus == status)
    {
        return;
    }
    m_validateStatus = status;
    updateTheme();
    Q_EMIT validateStatusChanged(m_validateStatus);
}

QWidget* AntFormItem::fieldWidget() const
{
    return m_fieldWidget.data();
}

void AntFormItem::setFieldWidget(QWidget* widget)
{
    if (m_fieldWidget == widget)
    {
        return;
    }

    if (m_fieldWidget)
    {
        m_fieldWidget->setParent(nullptr);
    }

    m_fieldWidget = widget;
    if (m_fieldWidget)
    {
        m_fieldWidget->setParent(m_fieldColumn);
    }

    rebuildLayout();
    updateTheme();
}

void AntFormItem::applyFormSettings(Ant::FormLayout layoutMode,
                                    Ant::FormLabelAlign labelAlign,
                                    bool showColon,
                                    bool showRequiredMark,
                                    int labelWidth)
{
    m_layoutMode = layoutMode;
    m_labelAlign = labelAlign;
    m_showRequiredMark = showRequiredMark;
    m_labelWidth = qMax(40, labelWidth);
    if (m_useFormColon)
    {
        m_colon = showColon;
    }
    rebuildLayout();
    updateTheme();
}

void AntFormItem::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        updateTheme();
    }
    QWidget::changeEvent(event);
}

void AntFormItem::rebuildLayout()
{
    delete m_rootLayout;
    delete m_fieldColumnLayout;

    const auto& token = antTheme->tokens();
    if (m_layoutMode == Ant::FormLayout::Horizontal)
    {
        m_rootLayout = new QHBoxLayout(this);
        m_rootLayout->setContentsMargins(0, 0, 0, 0);
        m_rootLayout->setSpacing(token.margin);
    }
    else
    {
        m_rootLayout = new QVBoxLayout(this);
        m_rootLayout->setContentsMargins(0, 0, 0, 0);
        m_rootLayout->setSpacing(token.marginXS);
    }

    m_fieldColumnLayout = new QVBoxLayout(m_fieldColumn);
    m_fieldColumnLayout->setContentsMargins(0, 0, 0, 0);
    m_fieldColumnLayout->setSpacing(token.marginXS);

    while (QLayoutItem* item = m_rootLayout->takeAt(0))
    {
        delete item;
    }
    while (QLayoutItem* item = m_fieldColumnLayout->takeAt(0))
    {
        delete item;
    }

    m_labelContainer->setVisible(!m_label.isEmpty());
    m_requiredLabel->setVisible(m_showRequiredMark && m_required);
    m_labelWidget->setText(effectiveLabelText());

    if (m_layoutMode == Ant::FormLayout::Horizontal)
    {
        m_labelContainer->setFixedWidth(m_label.isEmpty() ? 0 : m_labelWidth);
        m_rootLayout->addWidget(m_labelContainer, 0, Qt::AlignTop);
        m_rootLayout->addWidget(m_fieldColumn, 1);
    }
    else
    {
        m_labelContainer->setFixedWidth(QWIDGETSIZE_MAX);
        m_rootLayout->addWidget(m_labelContainer);
        m_rootLayout->addWidget(m_fieldColumn);
    }

    if (m_fieldWidget)
    {
        m_fieldColumnLayout->addWidget(m_fieldWidget);
    }
    m_extraLabel->setText(m_extra);
    m_extraLabel->setVisible(!m_extra.isEmpty());
    m_helpLabel->setText(m_helpText);
    m_helpLabel->setVisible(!m_helpText.isEmpty());
    if (!m_extra.isEmpty())
    {
        m_fieldColumnLayout->addWidget(m_extraLabel);
    }
    if (!m_helpText.isEmpty())
    {
        m_fieldColumnLayout->addWidget(m_helpLabel);
    }
}

void AntFormItem::updateTheme()
{
    const auto& token = antTheme->tokens();

    QFont labelFont = font();
    labelFont.setPixelSize(token.fontSize);
    labelFont.setWeight(QFont::Normal);
    m_labelWidget->setFont(labelFont);
    m_requiredLabel->setFont(labelFont);

    QPalette labelPalette = m_labelWidget->palette();
    labelPalette.setColor(QPalette::WindowText, isEnabled() ? token.colorText : token.colorTextDisabled);
    m_labelWidget->setPalette(labelPalette);

    QPalette reqPalette = m_requiredLabel->palette();
    reqPalette.setColor(QPalette::WindowText, token.colorError);
    m_requiredLabel->setPalette(reqPalette);

    QFont assistFont = font();
    assistFont.setPixelSize(token.fontSizeSM);
    m_extraLabel->setFont(assistFont);
    m_helpLabel->setFont(assistFont);

    QPalette extraPalette = m_extraLabel->palette();
    extraPalette.setColor(QPalette::WindowText, token.colorTextSecondary);
    m_extraLabel->setPalette(extraPalette);

    QPalette helpPalette = m_helpLabel->palette();
    helpPalette.setColor(QPalette::WindowText, helpColor());
    m_helpLabel->setPalette(helpPalette);

    if (auto* labelLayout = qobject_cast<QHBoxLayout*>(m_labelContainer->layout()))
    {
        if (m_layoutMode == Ant::FormLayout::Horizontal && m_labelAlign == Ant::FormLabelAlign::Right)
        {
            labelLayout->setDirection(QBoxLayout::LeftToRight);
            labelLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);
        }
        else
        {
            labelLayout->setDirection(QBoxLayout::LeftToRight);
            labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        }
    }
}

QString AntFormItem::effectiveLabelText() const
{
    if (m_label.isEmpty())
    {
        return QString();
    }
    return m_colon ? QStringLiteral("%1:").arg(m_label) : m_label;
}

QColor AntFormItem::helpColor() const
{
    const auto& token = antTheme->tokens();
    switch (m_validateStatus)
    {
    case Ant::Status::Error:
        return token.colorError;
    case Ant::Status::Warning:
        return token.colorWarning;
    case Ant::Status::Normal:
    default:
        return token.colorTextSecondary;
    }
}

AntForm::AntForm(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntFormStyle(style()));
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(m_itemSpacing);

}

Ant::FormLayout AntForm::formLayout() const { return m_formLayout; }

void AntForm::setFormLayout(Ant::FormLayout layout)
{
    if (m_formLayout == layout)
    {
        return;
    }
    m_formLayout = layout;
    rebuildLayout();
    applyItemSettings();
    Q_EMIT formLayoutChanged(m_formLayout);
}

Ant::FormLabelAlign AntForm::labelAlign() const { return m_labelAlign; }

void AntForm::setLabelAlign(Ant::FormLabelAlign align)
{
    if (m_labelAlign == align)
    {
        return;
    }
    m_labelAlign = align;
    applyItemSettings();
    Q_EMIT labelAlignChanged(m_labelAlign);
}

bool AntForm::colon() const { return m_colon; }

void AntForm::setColon(bool colon)
{
    if (m_colon == colon)
    {
        return;
    }
    m_colon = colon;
    applyItemSettings();
    Q_EMIT colonChanged(m_colon);
}

bool AntForm::requiredMark() const { return m_requiredMark; }

void AntForm::setRequiredMark(bool show)
{
    if (m_requiredMark == show)
    {
        return;
    }
    m_requiredMark = show;
    applyItemSettings();
    Q_EMIT requiredMarkChanged(m_requiredMark);
}

int AntForm::labelWidth() const { return m_labelWidth; }

void AntForm::setLabelWidth(int width)
{
    width = qMax(40, width);
    if (m_labelWidth == width)
    {
        return;
    }
    m_labelWidth = width;
    applyItemSettings();
    Q_EMIT labelWidthChanged(m_labelWidth);
}

int AntForm::itemSpacing() const { return m_itemSpacing; }

void AntForm::setItemSpacing(int spacing)
{
    spacing = qMax(0, spacing);
    if (m_itemSpacing == spacing)
    {
        return;
    }
    m_itemSpacing = spacing;
    rebuildLayout();
    Q_EMIT itemSpacingChanged(m_itemSpacing);
}

QList<AntFormItem*> AntForm::items() const
{
    return m_items;
}

void AntForm::addItem(AntFormItem* item)
{
    if (!item || m_items.contains(item))
    {
        return;
    }
    item->setParent(this);
    m_items.append(item);
    rebuildLayout();
    applyItemSettings();
}

AntFormItem* AntForm::addItem(const QString& label, QWidget* fieldWidget, bool required)
{
    auto* item = new AntFormItem(this);
    item->setLabel(label);
    item->setRequired(required);
    item->setFieldWidget(fieldWidget);
    addItem(item);
    return item;
}

void AntForm::clearItems()
{
    for (AntFormItem* item : std::as_const(m_items))
    {
        if (item)
        {
            item->deleteLater();
        }
    }
    m_items.clear();
    rebuildLayout();
}

void AntForm::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        applyItemSettings();
    }
    QWidget::changeEvent(event);
}

void AntForm::rebuildLayout()
{
    delete m_layout;
    if (m_formLayout == Ant::FormLayout::Inline)
    {
        m_layout = new QHBoxLayout(this);
    }
    else
    {
        m_layout = new QVBoxLayout(this);
    }
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(m_itemSpacing);

    for (AntFormItem* item : std::as_const(m_items))
    {
        if (!item)
        {
            continue;
        }
        m_layout->addWidget(item, m_formLayout == Ant::FormLayout::Inline ? 0 : 0);
    }

    if (m_formLayout == Ant::FormLayout::Inline)
    {
        m_layout->addStretch();
    }
}

void AntForm::applyItemSettings()
{
    for (AntFormItem* item : std::as_const(m_items))
    {
        if (item)
        {
            item->applyFormSettings(m_formLayout, m_labelAlign, m_colon, m_requiredMark, m_labelWidth);
            item->setEnabled(isEnabled());
        }
    }
}

// ── AntFormList ──

AntFormList::AntFormList(QWidget* parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(12);

    m_addButton = new QPushButton(QStringLiteral("+ Add"), this);
    m_addButton->setCursor(Qt::PointingHandCursor);
    m_addButton->setFixedHeight(32);
    connect(m_addButton, &QPushButton::clicked, this, [this]() { addItem(); });

    const auto& token = antTheme->tokens();
    QFont btnFont = m_addButton->font();
    btnFont.setPixelSize(token.fontSizeSM);
    m_addButton->setFont(btnFont);
    m_addButton->setStyleSheet(QStringLiteral(
        "QPushButton { border: 1px dashed %1; border-radius: 6px; color: %2; background: transparent; }"
        "QPushButton:hover { border-color: %3; color: %3; }")
        .arg(token.colorBorder.name(), token.colorPrimary.name(), token.colorPrimaryHover.name()));

    m_layout->addWidget(m_addButton);
    updateAddButton();

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        rebuildAll();
    });
}

int AntFormList::minCount() const { return m_minCount; }

void AntFormList::setMinCount(int count)
{
    count = qMax(0, count);
    if (m_minCount == count)
    {
        return;
    }
    m_minCount = count;
    // Add items if below minimum
    while (m_items.size() < m_minCount)
    {
        addItem();
    }
    updateAddButton();
    Q_EMIT minCountChanged(m_minCount);
}

int AntFormList::maxCount() const { return m_maxCount; }

void AntFormList::setMaxCount(int count)
{
    count = qMax(0, count);
    if (m_maxCount == count)
    {
        return;
    }
    m_maxCount = count;
    // Remove items if above maximum
    while (m_maxCount > 0 && m_items.size() > m_maxCount)
    {
        removeItem(m_items.size() - 1);
    }
    updateAddButton();
    Q_EMIT maxCountChanged(m_maxCount);
}

int AntFormList::count() const { return m_items.size(); }

void AntFormList::setItemFactory(AntFormListItemFactory factory)
{
    m_factory = std::move(factory);
    rebuildAll();
}

void AntFormList::addItem()
{
    if (m_maxCount > 0 && m_items.size() >= m_maxCount)
    {
        return;
    }

    const int index = m_items.size();
    const auto& token = antTheme->tokens();

    auto* container = new QWidget(this);
    auto* rowLayout = new QHBoxLayout(container);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(8);

    QWidget* content = nullptr;
    if (m_factory)
    {
        content = m_factory(index);
    }
    if (!content)
    {
        content = new QWidget(container);
    }
    content->setParent(container);
    rowLayout->addWidget(content, 1);

    auto* removeBtn = new QPushButton(QStringLiteral("Remove"), container);
    removeBtn->setCursor(Qt::PointingHandCursor);
    removeBtn->setFixedSize(64, 28);
    QFont btnFont = removeBtn->font();
    btnFont.setPixelSize(token.fontSizeSM);
    removeBtn->setFont(btnFont);
    removeBtn->setStyleSheet(QStringLiteral(
        "QPushButton { border: 1px solid %1; border-radius: 4px; color: %2; background: transparent; }"
        "QPushButton:hover { border-color: %3; color: %3; background: %4; }")
        .arg(token.colorBorder.name(), token.colorTextSecondary.name(),
             token.colorError.name(), token.colorErrorBg.name()));
    connect(removeBtn, &QPushButton::clicked, this, [this, index]() {
        // Find the actual index at click time (indices shift after removal)
        for (int i = 0; i < m_items.size(); ++i)
        {
            if (m_items[i].removeButton == sender())
            {
                removeItem(i);
                return;
            }
        }
    });
    rowLayout->addWidget(removeBtn, 0, Qt::AlignTop);

    ListItem item;
    item.container = container;
    item.content = content;
    item.removeButton = removeBtn;
    m_items.append(item);

    m_layout->insertWidget(m_layout->count() - 1, container);
    updateAddButton();
    Q_EMIT itemAdded(index);
    Q_EMIT countChanged(m_items.size());
    Q_EMIT fieldsChanged(itemValues());
}

void AntFormList::removeItem(int index)
{
    if (index < 0 || index >= m_items.size())
    {
        return;
    }
    if (m_items.size() <= m_minCount)
    {
        return;
    }

    ListItem item = m_items.takeAt(index);
    m_layout->removeWidget(item.container);
    item.container->deleteLater();

    updateAddButton();
    Q_EMIT itemRemoved(index);
    Q_EMIT countChanged(m_items.size());
    Q_EMIT fieldsChanged(itemValues());
}

void AntFormList::clearItems()
{
    while (!m_items.isEmpty())
    {
        ListItem item = m_items.takeLast();
        m_layout->removeWidget(item.container);
        item.container->deleteLater();
    }
    updateAddButton();
    Q_EMIT countChanged(0);
    Q_EMIT fieldsChanged(QList<QVariantMap>());
}

QList<QVariantMap> AntFormList::itemValues() const
{
    QList<QVariantMap> values;
    for (const ListItem& item : m_items)
    {
        Q_UNUSED(item)
        // Each item's content widget values would need to be extracted
        // by the factory-created widget. This is a structural placeholder.
        values.append(QVariantMap());
    }
    return values;
}

void AntFormList::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        for (const ListItem& item : std::as_const(m_items))
        {
            if (item.container)
            {
                item.container->setEnabled(isEnabled());
            }
        }
        m_addButton->setEnabled(isEnabled());
    }
    QWidget::changeEvent(event);
}

void AntFormList::rebuildAll()
{
    // Recreate all items with the factory
    const int oldCount = m_items.size();
    clearItems();
    for (int i = 0; i < qMax(m_minCount, oldCount); ++i)
    {
        addItem();
    }
}

void AntFormList::updateAddButton()
{
    const bool canAdd = m_maxCount == 0 || m_items.size() < m_maxCount;
    m_addButton->setVisible(canAdd);
    m_addButton->setEnabled(isEnabled() && canAdd);
}
