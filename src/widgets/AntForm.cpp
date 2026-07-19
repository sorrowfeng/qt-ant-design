#include "AntForm.h"

#include <QBoxLayout>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMetaMethod>
#include <QMetaProperty>
#include <QPainter>
#include <QPushButton>
#include <utility>
#include <QVBoxLayout>
#include <functional>

#include "../styles/AntFormStyle.h"
#include "core/AntTheme.h"
#include "core/AntThemeRefresh_p.h"

// ── AntFormProvider ──

AntFormProvider::AntFormProvider(QWidget* parent)
    : QWidget(parent)
{
}

AntFormProvider::~AntFormProvider()
{
    for (const auto& entry : std::as_const(m_forms))
    {
        disconnect(entry.fieldConnection);
        disconnect(entry.finishConnection);
        disconnect(entry.destroyedConnection);
    }
    m_forms.clear();
}

void AntFormProvider::addForm(AntForm* form, const QString& name)
{
    if (!form)
    {
        return;
    }
    removeDestroyedForms();
    for (const auto& entry : m_forms)
    {
        if (entry.form == form)
        {
            return;
        }
    }

    FormEntry entry;
    entry.form = form;
    entry.name = name.trimmed();
    if (entry.name.isEmpty())
    {
        entry.name = form->objectName().trimmed();
    }
    if (entry.name.isEmpty())
    {
        int candidate = 0;
        while (true)
        {
            const QString generated = QString::number(candidate++);
            bool used = false;
            for (const auto& existing : std::as_const(m_forms))
            {
                if (existing.name == generated)
                {
                    used = true;
                    break;
                }
            }
            if (!used)
            {
                entry.name = generated;
                break;
            }
        }
    }

    const QString registeredName = entry.name;
    entry.fieldConnection = connect(form, &AntForm::fieldChanged, this,
                                    [this, registeredName](const QString& fieldName, const QVariant& value) {
        Q_EMIT formChanged(registeredName, fieldName, value);
    });
    entry.finishConnection = connect(form, &AntForm::finished, this,
                                     [this, registeredName](const QVariantMap& values) {
        Q_EMIT formFinished(registeredName, values);
    });
    entry.destroyedConnection = connect(form, &QObject::destroyed, this, [this](QObject* destroyedForm) {
        removeDestroyedForms(destroyedForm);
    });
    m_forms.append(entry);
}

void AntFormProvider::removeForm(AntForm* form)
{
    for (int i = 0; i < m_forms.size(); ++i)
    {
        if (m_forms[i].form == form)
        {
            disconnect(m_forms[i].fieldConnection);
            disconnect(m_forms[i].finishConnection);
            disconnect(m_forms[i].destroyedConnection);
            m_forms.removeAt(i);
            return;
        }
    }
}

QList<AntForm*> AntFormProvider::forms() const
{
    QList<AntForm*> result;
    for (const auto& entry : m_forms)
    {
        if (entry.form)
        {
            result.append(entry.form.data());
        }
    }
    return result;
}

void AntFormProvider::removeDestroyedForms(QObject* destroyedForm)
{
    for (int i = m_forms.size() - 1; i >= 0; --i)
    {
        if (m_forms[i].form && m_forms[i].form.data() != destroyedForm)
        {
            continue;
        }
        disconnect(m_forms[i].fieldConnection);
        disconnect(m_forms[i].finishConnection);
        disconnect(m_forms[i].destroyedConnection);
        m_forms.removeAt(i);
    }
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
        updateTheme();
    });

    rebuildLayout();
    updateTheme();
    syncFormItemPerfCounters();
}

QString AntFormItem::label() const { return m_label; }

void AntFormItem::setLabel(const QString& label)
{
    if (m_label == label)
    {
        return;
    }
    m_label = label;
    updateLabelPresentation(true);
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
    updateAssistText(true);
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
    updateAssistText(true);
    updateTheme();
    Q_EMIT extraChanged(m_extra);
}

QString AntFormItem::fieldName() const { return m_fieldName; }

void AntFormItem::setFieldName(const QString& name)
{
    const QString normalized = name.trimmed();
    if (m_fieldName == normalized)
    {
        return;
    }
    m_fieldName = normalized;
    Q_EMIT fieldNameChanged(m_fieldName);
}

QVariant AntFormItem::fieldValue() const
{
    if (!m_fieldWidget || m_fieldValueProperty.isEmpty())
    {
        return QVariant();
    }
    return m_fieldWidget->property(m_fieldValueProperty.constData());
}

void AntFormItem::setFieldValue(const QVariant& value)
{
    if (!m_fieldWidget || m_fieldValueProperty.isEmpty() || fieldValue() == value)
    {
        return;
    }

    const QMetaObject* meta = m_fieldWidget->metaObject();
    const int propertyIndex = meta->indexOfProperty(m_fieldValueProperty.constData());
    if (propertyIndex < 0)
    {
        return;
    }
    const QMetaProperty property = meta->property(propertyIndex);
    if (!property.isWritable() || !property.write(m_fieldWidget.data(), value))
    {
        return;
    }
    if (!property.hasNotifySignal())
    {
        handleFieldWidgetValueChanged();
    }
}

bool AntFormItem::isRequired() const { return m_required; }

void AntFormItem::setRequired(bool required)
{
    if (m_required == required)
    {
        return;
    }
    m_required = required;
    updateLabelPresentation(true);
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
    updateLabelPresentation(true);
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

    if (m_fieldValueConnection)
    {
        disconnect(m_fieldValueConnection);
        m_fieldValueConnection = QMetaObject::Connection();
    }
    m_fieldValueProperty.clear();

    if (m_fieldWidget)
    {
        m_fieldWidget->setParent(nullptr);
    }

    m_fieldWidget = widget;
    if (m_fieldWidget)
    {
        m_fieldWidget->setParent(m_fieldColumn);
    }

    bindFieldWidgetValue();

    rebuildLayout();
    updateTheme();
    Q_EMIT fieldValueChanged(fieldValue());
}

void AntFormItem::handleFieldWidgetValueChanged()
{
    Q_EMIT fieldValueChanged(fieldValue());
}

void AntFormItem::bindFieldWidgetValue()
{
    if (!m_fieldWidget)
    {
        return;
    }

    // Prefer semantic value properties, then common text/selection inputs.
    static constexpr const char* candidates[] = {
        "checked", "value", "text", "currentValue", "currentText",
        "currentIndex", "date", "time", "color"
    };
    const QMetaObject* fieldMeta = m_fieldWidget->metaObject();
    for (const char* candidate : candidates)
    {
        const int propertyIndex = fieldMeta->indexOfProperty(candidate);
        if (propertyIndex < 0)
        {
            continue;
        }
        const QMetaProperty property = fieldMeta->property(propertyIndex);
        if (!property.isReadable())
        {
            continue;
        }

        m_fieldValueProperty = candidate;
        if (property.hasNotifySignal())
        {
            const int slotIndex = metaObject()->indexOfSlot("handleFieldWidgetValueChanged()");
            if (slotIndex >= 0)
            {
                m_fieldValueConnection = QObject::connect(m_fieldWidget.data(),
                                                          property.notifySignal(),
                                                          this,
                                                          metaObject()->method(slotIndex));
            }
        }
        return;
    }
}

void AntFormItem::applyFormSettings(Ant::FormLayout layoutMode,
                                    Ant::FormLabelAlign labelAlign,
                                    bool showColon,
                                    bool showRequiredMark,
                                    int labelWidth)
{
    const int normalizedLabelWidth = qMax(40, labelWidth);
    const bool nextColon = m_useFormColon ? showColon : m_colon;
    const bool layoutChanged = m_layoutMode != layoutMode;
    const bool changed = layoutChanged || m_labelAlign != labelAlign ||
                         m_showRequiredMark != showRequiredMark ||
                         m_labelWidth != normalizedLabelWidth || m_colon != nextColon;
    if (!changed)
    {
        ++m_settingsSkipCount;
        syncFormItemPerfCounters();
        return;
    }

    m_layoutMode = layoutMode;
    m_labelAlign = labelAlign;
    m_showRequiredMark = showRequiredMark;
    m_labelWidth = normalizedLabelWidth;
    m_colon = nextColon;
    if (layoutChanged)
    {
        rebuildLayout();
    }
    else
    {
        updateLabelPresentation();
    }
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
    ++m_layoutRebuildCount;
    delete m_rootLayout;
    m_rootLayout = nullptr;
    delete m_fieldColumnLayout;
    m_fieldColumnLayout = nullptr;

    const auto& token = antTheme->tokens();
    if (m_layoutMode == Ant::FormLayout::Horizontal)
    {
        m_rootLayout = new QHBoxLayout(this);
        m_rootLayout->setContentsMargins(0, 0, 0, 0);
        m_rootLayout->setSpacing(token.marginXS);
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

    if (m_layoutMode == Ant::FormLayout::Horizontal)
    {
        m_rootLayout->addWidget(m_labelContainer, 0, Qt::AlignTop);
        m_rootLayout->addWidget(m_fieldColumn, 1);
    }
    else
    {
        m_rootLayout->addWidget(m_labelContainer);
        m_rootLayout->addWidget(m_fieldColumn);
    }

    if (m_fieldWidget)
    {
        m_fieldColumnLayout->addWidget(m_fieldWidget);
    }
    m_fieldColumnLayout->addWidget(m_extraLabel);
    m_fieldColumnLayout->addWidget(m_helpLabel);

    updateLabelPresentation();
    updateAssistText();
    syncFormItemPerfCounters();
}

void AntFormItem::updateLabelPresentation(bool countUpdate)
{
    if (!m_labelContainer || !m_requiredLabel || !m_labelWidget)
    {
        return;
    }

    m_labelContainer->setVisible(!m_label.isEmpty());
    m_requiredLabel->setVisible(m_showRequiredMark && m_required);
    m_labelWidget->setText(effectiveLabelText());

    if (m_layoutMode == Ant::FormLayout::Horizontal)
    {
        m_labelContainer->setFixedWidth(m_label.isEmpty() ? 0 : m_labelWidth);
    }
    else
    {
        m_labelContainer->setFixedWidth(QWIDGETSIZE_MAX);
    }

    if (countUpdate)
    {
        ++m_inlineTextUpdateCount;
    }
    updateGeometry();
    update();
    syncFormItemPerfCounters();
}

void AntFormItem::updateAssistText(bool countUpdate)
{
    if (!m_extraLabel || !m_helpLabel)
    {
        return;
    }

    m_extraLabel->setText(m_extra);
    m_extraLabel->setVisible(!m_extra.isEmpty());
    m_helpLabel->setText(m_helpText);
    m_helpLabel->setVisible(!m_helpText.isEmpty());

    if (countUpdate)
    {
        ++m_inlineTextUpdateCount;
    }
    updateGeometry();
    update();
    syncFormItemPerfCounters();
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

    if (m_rootLayout)
    {
        m_rootLayout->setSpacing(token.marginXS);
    }
    if (m_fieldColumnLayout)
    {
        m_fieldColumnLayout->setSpacing(token.marginXS);
    }

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

void AntFormItem::syncFormItemPerfCounters() const
{
    const_cast<AntFormItem*>(this)->setProperty("antFormItemLayoutRebuildCount", m_layoutRebuildCount);
    const_cast<AntFormItem*>(this)->setProperty("antFormItemInlineTextUpdateCount", m_inlineTextUpdateCount);
    const_cast<AntFormItem*>(this)->setProperty("antFormItemSettingsSkipCount", m_settingsSkipCount);
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
    installAntStyle<AntFormStyle>(this);
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(m_itemSpacing);
    syncFormPerfCounters();
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
    if (m_layout)
    {
        m_layout->setSpacing(m_itemSpacing);
    }
    ++m_spacingUpdateCount;
    syncFormPerfCounters();
    updateGeometry();
    Q_EMIT itemSpacingChanged(m_itemSpacing);
}

QList<AntFormItem*> AntForm::items() const
{
    QList<AntFormItem*> result;
    result.reserve(m_items.size());
    for (const auto& item : m_items)
    {
        if (item)
        {
            result.append(item.data());
        }
    }
    return result;
}

void AntForm::addItem(AntFormItem* item)
{
    if (!item || m_items.contains(item))
    {
        return;
    }
    item->setParent(this);
    m_items.append(item);
    connectItem(item);
    if (m_layout)
    {
        if (m_formLayout == Ant::FormLayout::Inline && m_layout->count() > 0)
        {
            m_layout->insertWidget(qMax(0, m_layout->count() - 1), item);
        }
        else
        {
            m_layout->addWidget(item);
        }
    }
    item->applyFormSettings(m_formLayout, m_labelAlign, m_colon, m_requiredMark, m_labelWidth);
    item->setEnabled(isEnabled());
    ++m_itemSettingsApplyCount;
    syncFormPerfCounters();
    updateGeometry();
}

AntFormItem* AntForm::addItem(const QString& label, QWidget* fieldWidget, bool required)
{
    auto* item = new AntFormItem(this);
    item->setLabel(label);
    item->setFieldName(label);
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
            disconnect(item, nullptr, this, nullptr);
            if (m_layout)
            {
                m_layout->removeWidget(item);
            }
            item->deleteLater();
        }
    }
    m_items.clear();
    syncFormPerfCounters();
    updateGeometry();
}

QVariant AntForm::fieldValue(const QString& fieldName) const
{
    const QString normalized = fieldName.trimmed();
    if (m_customFieldValues.contains(normalized))
    {
        return m_customFieldValues.value(normalized);
    }
    for (const auto& item : m_items)
    {
        if (item && effectiveFieldName(item.data()) == normalized)
        {
            return item->fieldValue();
        }
    }
    return QVariant();
}

QVariantMap AntForm::values() const
{
    QVariantMap result;
    for (const auto& item : m_items)
    {
        if (!item)
        {
            continue;
        }
        const QString name = effectiveFieldName(item.data());
        if (!name.isEmpty())
        {
            result.insert(name, item->fieldValue());
        }
    }
    for (auto it = m_customFieldValues.constBegin(); it != m_customFieldValues.constEnd(); ++it)
    {
        result.insert(it.key(), it.value());
    }
    return result;
}

void AntForm::notifyFieldChanged(const QString& fieldName, const QVariant& value)
{
    const QString normalized = fieldName.trimmed();
    if (normalized.isEmpty())
    {
        return;
    }
    if (m_customFieldValues.value(normalized) == value && m_customFieldValues.contains(normalized))
    {
        return;
    }
    m_customFieldValues.insert(normalized, value);
    Q_EMIT fieldChanged(normalized, value);
}

void AntForm::finish()
{
    Q_EMIT finished(values());
}

void AntForm::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        for (AntFormItem* item : std::as_const(m_items))
        {
            if (item)
            {
                item->setEnabled(isEnabled());
            }
        }
    }
    QWidget::changeEvent(event);
}

void AntForm::rebuildLayout()
{
    ++m_layoutRebuildCount;
    delete m_layout;
    m_layout = nullptr;
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
    syncFormPerfCounters();
}

void AntForm::applyItemSettings()
{
    ++m_itemSettingsApplyCount;
    for (AntFormItem* item : std::as_const(m_items))
    {
        if (item)
        {
            item->applyFormSettings(m_formLayout, m_labelAlign, m_colon, m_requiredMark, m_labelWidth);
            item->setEnabled(isEnabled());
        }
    }
    syncFormPerfCounters();
}

void AntForm::syncFormPerfCounters() const
{
    const_cast<AntForm*>(this)->setProperty("antFormLayoutRebuildCount", m_layoutRebuildCount);
    const_cast<AntForm*>(this)->setProperty("antFormItemSettingsApplyCount", m_itemSettingsApplyCount);
    const_cast<AntForm*>(this)->setProperty("antFormSpacingUpdateCount", m_spacingUpdateCount);
}

void AntForm::connectItem(AntFormItem* item)
{
    connect(item, &AntFormItem::fieldValueChanged, this, [this, item](const QVariant& value) {
        const QString name = effectiveFieldName(item);
        if (name.isEmpty())
        {
            return;
        }
        m_customFieldValues.remove(name);
        Q_EMIT fieldChanged(name, value);
    });
    connect(item, &QObject::destroyed, this, [this](QObject* destroyedItem) {
        removeDestroyedItems(destroyedItem);
    });
}

QString AntForm::effectiveFieldName(const AntFormItem* item) const
{
    if (!item)
    {
        return QString();
    }
    return item->fieldName().isEmpty() ? item->label().trimmed() : item->fieldName();
}

void AntForm::removeDestroyedItems(QObject* destroyedItem)
{
    for (int i = m_items.size() - 1; i >= 0; --i)
    {
        if (!m_items[i] || m_items[i].data() == destroyedItem)
        {
            m_items.removeAt(i);
        }
    }
    syncFormPerfCounters();
}

AntForm::~AntForm()
{
    // QObject destroys child items after derived members have already been
    // torn down. Disconnect callbacks that access m_items before that phase.
    for (const auto& item : std::as_const(m_items))
    {
        if (item)
        {
            disconnect(item.data(), nullptr, this, nullptr);
        }
    }
    m_items.clear();
}

// ── Custom button classes for AntFormList ──

namespace
{

class FormAddButton : public QPushButton
{
public:
    explicit FormAddButton(QWidget* parent = nullptr) : QPushButton(parent)
    {
        setCursor(Qt::PointingHandCursor);
        setFixedHeight(32);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const bool hovered = m_hovered;
        QColor borderColor = hovered ? token.colorPrimaryHover : token.colorBorder;
        QColor textColor = hovered ? token.colorPrimaryHover : token.colorPrimary;

        p.setPen(QPen(borderColor, 1, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 6, 6);

        p.setPen(textColor);
        QFont f = font();
        f.setPixelSize(token.fontSizeSM);
        p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, text());
    }

    void enterEvent(AntEnterEvent*) override { m_hovered = true; update(); }
    void leaveEvent(QEvent*) override { m_hovered = false; update(); }

private:
    bool m_hovered = false;
};

class FormRemoveButton : public QPushButton
{
public:
    explicit FormRemoveButton(QWidget* parent = nullptr) : QPushButton(parent)
    {
        setCursor(Qt::PointingHandCursor);
        setFixedSize(64, 28);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const bool hovered = m_hovered;
        QColor borderColor = hovered ? token.colorError : token.colorBorder;
        QColor textColor = hovered ? token.colorError : token.colorTextSecondary;
        QColor bgColor = hovered ? token.colorErrorBg : Qt::transparent;

        p.setPen(QPen(borderColor, 1));
        p.setBrush(bgColor);
        p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 4, 4);

        p.setPen(textColor);
        QFont f = font();
        f.setPixelSize(token.fontSizeSM);
        p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, text());
    }

    void enterEvent(AntEnterEvent*) override { m_hovered = true; update(); }
    void leaveEvent(QEvent*) override { m_hovered = false; update(); }

private:
    bool m_hovered = false;
};

} // namespace

// ── AntFormList ──

AntFormList::AntFormList(QWidget* parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(12);

    m_addButton = new FormAddButton(this);
    m_addButton->setText(QStringLiteral("+ Add"));
    connect(m_addButton, &QPushButton::clicked, this, [this]() { addItem(); });

    const auto& token = antTheme->tokens();
    QFont btnFont = m_addButton->font();
    btnFont.setPixelSize(token.fontSizeSM);
    m_addButton->setFont(btnFont);

    m_layout->addWidget(m_addButton);
    updateAddButton();

    connect(antTheme, &AntTheme::themeAboutToChange, this, [this]() {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        refreshTheme();
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
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

    auto* removeBtn = new FormRemoveButton(container);
    removeBtn->setText(QStringLiteral("Remove"));
    QFont btnFont = removeBtn->font();
    btnFont.setPixelSize(token.fontSizeSM);
    removeBtn->setFont(btnFont);
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

void AntFormList::refreshTheme()
{
    const auto& token = antTheme->tokens();
    const auto applyButtonFont = [&token](QPushButton* button) {
        if (!button)
        {
            return;
        }
        QFont btnFont = button->font();
        btnFont.setPixelSize(token.fontSizeSM);
        if (button->font() != btnFont)
        {
            button->setFont(btnFont);
        }
        button->update();
    };

    applyButtonFont(m_addButton);
    for (const ListItem& item : std::as_const(m_items))
    {
        applyButtonFont(item.removeButton);
        if (item.container)
        {
            item.container->update();
        }
    }
    update();
}

void AntFormList::updateAddButton()
{
    const bool canAdd = m_maxCount == 0 || m_items.size() < m_maxCount;
    m_addButton->setVisible(canAdd);
    m_addButton->setEnabled(isEnabled() && canAdd);
}
