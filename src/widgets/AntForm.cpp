#include "AntForm.h"

#include <QBoxLayout>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <utility>
#include <QVBoxLayout>

#include "core/AntTheme.h"

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

Ant::InputStatus AntFormItem::validateStatus() const { return m_validateStatus; }

void AntFormItem::setValidateStatus(Ant::InputStatus status)
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
    case Ant::InputStatus::Error:
        return token.colorError;
    case Ant::InputStatus::Warning:
        return token.colorWarning;
    case Ant::InputStatus::Normal:
    default:
        return token.colorTextSecondary;
    }
}

AntForm::AntForm(QWidget* parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(m_itemSpacing);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        rebuildLayout();
        applyItemSettings();
    });
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
