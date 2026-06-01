#include "AntInputDialog.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QShowEvent>
#include <QSizePolicy>
#include <QTimer>
#include <QVBoxLayout>

#include "../styles/AntInputDialogStyle.h"
#include "AntButton.h"
#include "AntInput.h"
#include "AntInputNumber.h"
#include "AntPlainTextEdit.h"
#include "AntSelect.h"
#include "AntTypography.h"
#include "core/AntTheme.h"
#include "core/AntTypes.h"

namespace
{
constexpr int kInputDialogDefaultWidth = 440;
constexpr int kInputDialogDefaultHeight = 220;

template <typename T>
T normalizedMinimum(T min, T max)
{
    return min <= max ? min : max;
}

template <typename T>
T normalizedMaximum(T min, T max)
{
    return max >= min ? max : min;
}

bool stringListsEqual(const QStringList& lhs, const QStringList& rhs)
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }
    for (int i = 0; i < lhs.size(); ++i)
    {
        if (lhs.at(i) != rhs.at(i))
        {
            return false;
        }
    }
    return true;
}
} // namespace

AntInputDialog::AntInputDialog(QWidget* parent, Qt::WindowFlags flags)
    : AntDialog(parent, flags),
      m_okButtonText(QStringLiteral("OK")),
      m_cancelButtonText(QStringLiteral("Cancel"))
{
    qRegisterMetaType<AntInputDialog::InputMode>("AntInputDialog::InputMode");
    qRegisterMetaType<AntInputDialog::InputDialogOptions>("AntInputDialog::InputDialogOptions");

    setWindowTitle(QStringLiteral("Input"));
    initializeAntStyle();
    buildUi();
    wireUi();
    updateInputVisibility();
    updateButtonRow();
    syncChildControls();
}

QString AntInputDialog::getText(QWidget* parent,
                                const QString& title,
                                const QString& label,
                                QLineEdit::EchoMode mode,
                                const QString& text,
                                bool* ok,
                                Qt::WindowFlags flags,
                                Qt::InputMethodHints inputMethodHints)
{
    AntInputDialog dialog(parent, flags);
    dialog.setWindowTitle(title);
    dialog.setLabelText(label);
    dialog.setInputMode(TextInput);
    dialog.setTextEchoMode(mode);
    dialog.setTextValue(text);
    dialog.setInputMethodHints(inputMethodHints);

    const bool accepted = dialog.exec() == QDialog::Accepted;
    if (ok)
    {
        *ok = accepted;
    }
    return accepted ? dialog.textValue() : text;
}

int AntInputDialog::getInt(QWidget* parent,
                           const QString& title,
                           const QString& label,
                           int value,
                           int minValue,
                           int maxValue,
                           int step,
                           bool* ok,
                           Qt::WindowFlags flags)
{
    AntInputDialog dialog(parent, flags);
    dialog.setWindowTitle(title);
    dialog.setLabelText(label);
    dialog.setInputMode(IntInput);
    dialog.setIntRange(minValue, maxValue);
    dialog.setIntStep(step);
    dialog.setIntValue(value);

    const bool accepted = dialog.exec() == QDialog::Accepted;
    if (ok)
    {
        *ok = accepted;
    }
    return accepted ? dialog.intValue() : value;
}

double AntInputDialog::getDouble(QWidget* parent,
                                 const QString& title,
                                 const QString& label,
                                 double value,
                                 double minValue,
                                 double maxValue,
                                 int decimals,
                                 bool* ok,
                                 Qt::WindowFlags flags)
{
    AntInputDialog dialog(parent, flags);
    dialog.setWindowTitle(title);
    dialog.setLabelText(label);
    dialog.setInputMode(DoubleInput);
    dialog.setDoubleRange(minValue, maxValue);
    dialog.setDoubleDecimals(decimals);
    dialog.setDoubleValue(value);

    const bool accepted = dialog.exec() == QDialog::Accepted;
    if (ok)
    {
        *ok = accepted;
    }
    return accepted ? dialog.doubleValue() : value;
}

QString AntInputDialog::getItem(QWidget* parent,
                                const QString& title,
                                const QString& label,
                                const QStringList& items,
                                int current,
                                bool editable,
                                bool* ok,
                                Qt::WindowFlags flags,
                                Qt::InputMethodHints inputMethodHints)
{
    AntInputDialog dialog(parent, flags);
    dialog.setWindowTitle(title);
    dialog.setLabelText(label);
    dialog.setInputMode(TextInput);
    dialog.setComboBoxEditable(editable);
    dialog.setComboBoxItems(items);
    if (current >= 0 && current < items.size())
    {
        dialog.setTextValue(items.at(current));
    }
    dialog.setInputMethodHints(inputMethodHints);

    const bool accepted = dialog.exec() == QDialog::Accepted;
    if (ok)
    {
        *ok = accepted;
    }
    return accepted ? dialog.textValue() : QString();
}

AntInputDialog::InputMode AntInputDialog::inputMode() const
{
    return m_inputMode;
}

void AntInputDialog::setInputMode(InputMode mode)
{
    if (m_inputMode == mode)
    {
        return;
    }
    m_inputMode = mode;
    updateInputVisibility();
    updateOkButtonState();
    updateGeometry();
    update();
    Q_EMIT inputModeChanged(m_inputMode);
}

QString AntInputDialog::labelText() const
{
    return m_labelText;
}

void AntInputDialog::setLabelText(const QString& text)
{
    if (m_labelText == text)
    {
        return;
    }
    m_labelText = text;
    updateLabel();
    updateGeometry();
    Q_EMIT labelTextChanged(m_labelText);
}

QString AntInputDialog::textValue() const
{
    if (m_inputMode == TextInput)
    {
        if (useComboBoxForTextInput())
        {
            return comboTextValue();
        }
        if (usePlainTextEditForTextInput() && m_plainTextInput)
        {
            return m_plainTextInput->toPlainText();
        }
        if (m_textInput)
        {
            return m_textInput->text();
        }
    }
    return m_textValue;
}

void AntInputDialog::setTextValue(const QString& text)
{
    const bool changed = m_textValue != text;
    m_textValue = text;
    applyTextValueToEditors();
    updateOkButtonState();
    if (changed)
    {
        Q_EMIT textValueChanged(m_textValue);
    }
}

QLineEdit::EchoMode AntInputDialog::textEchoMode() const
{
    return m_textEchoMode;
}

void AntInputDialog::setTextEchoMode(QLineEdit::EchoMode mode)
{
    if (m_textEchoMode == mode)
    {
        return;
    }
    m_textEchoMode = mode;
    if (m_textInput)
    {
        m_textInput->setEchoMode(mode);
    }
    Q_EMIT textEchoModeChanged(m_textEchoMode);
}

QString AntInputDialog::placeholderText() const
{
    return m_placeholderText;
}

void AntInputDialog::setPlaceholderText(const QString& text)
{
    if (m_placeholderText == text)
    {
        return;
    }
    m_placeholderText = text;
    if (m_textInput)
    {
        m_textInput->setPlaceholderText(text);
    }
    if (m_plainTextInput)
    {
        m_plainTextInput->setPlaceholderText(text);
    }
    if (m_numberInput)
    {
        m_numberInput->setPlaceholderText(text);
    }
    if (m_comboBox)
    {
        m_comboBox->setPlaceholderText(text);
    }
    Q_EMIT placeholderTextChanged(m_placeholderText);
}

Qt::InputMethodHints AntInputDialog::inputMethodHints() const
{
    return AntDialog::inputMethodHints();
}

void AntInputDialog::setInputMethodHints(Qt::InputMethodHints hints)
{
    AntDialog::setInputMethodHints(hints);
    applyInputMethodHintsToEditors();
}

int AntInputDialog::intValue() const
{
    return m_intValue;
}

void AntInputDialog::setIntValue(int value)
{
    const int normalized = qBound(m_intMinimum, value, m_intMaximum);
    const bool changed = m_intValue != normalized;
    m_intValue = normalized;
    if (m_inputMode == IntInput && m_numberInput)
    {
        const QSignalBlocker blocker(m_numberInput);
        m_numberInput->setValue(m_intValue);
    }
    if (changed)
    {
        Q_EMIT intValueChanged(m_intValue);
    }
}

int AntInputDialog::intMinimum() const
{
    return m_intMinimum;
}

void AntInputDialog::setIntMinimum(int min)
{
    setIntRange(min, m_intMaximum);
}

int AntInputDialog::intMaximum() const
{
    return m_intMaximum;
}

void AntInputDialog::setIntMaximum(int max)
{
    setIntRange(m_intMinimum, max);
}

void AntInputDialog::setIntRange(int min, int max)
{
    const int normalizedMin = normalizedMinimum(min, max);
    const int normalizedMax = normalizedMaximum(min, max);
    const bool changed = m_intMinimum != normalizedMin || m_intMaximum != normalizedMax;
    m_intMinimum = normalizedMin;
    m_intMaximum = normalizedMax;
    if (m_intValue < m_intMinimum || m_intValue > m_intMaximum)
    {
        setIntValue(m_intValue);
    }
    updateNumberEditor();
    if (changed)
    {
        Q_EMIT intRangeChanged(m_intMinimum, m_intMaximum);
    }
}

int AntInputDialog::intStep() const
{
    return m_intStep;
}

void AntInputDialog::setIntStep(int step)
{
    const int normalized = qMax(1, step);
    if (m_intStep == normalized)
    {
        return;
    }
    m_intStep = normalized;
    updateNumberEditor();
    Q_EMIT intStepChanged(m_intStep);
}

double AntInputDialog::doubleValue() const
{
    return m_doubleValue;
}

void AntInputDialog::setDoubleValue(double value)
{
    const double normalized = qBound(m_doubleMinimum, value, m_doubleMaximum);
    const bool changed = !qFuzzyCompare(m_doubleValue + 1.0, normalized + 1.0);
    m_doubleValue = normalized;
    if (m_inputMode == DoubleInput && m_numberInput)
    {
        const QSignalBlocker blocker(m_numberInput);
        m_numberInput->setValue(m_doubleValue);
    }
    if (changed)
    {
        Q_EMIT doubleValueChanged(m_doubleValue);
    }
}

double AntInputDialog::doubleMinimum() const
{
    return m_doubleMinimum;
}

void AntInputDialog::setDoubleMinimum(double min)
{
    setDoubleRange(min, m_doubleMaximum);
}

double AntInputDialog::doubleMaximum() const
{
    return m_doubleMaximum;
}

void AntInputDialog::setDoubleMaximum(double max)
{
    setDoubleRange(m_doubleMinimum, max);
}

void AntInputDialog::setDoubleRange(double min, double max)
{
    const double normalizedMin = normalizedMinimum(min, max);
    const double normalizedMax = normalizedMaximum(min, max);
    const bool changed = !qFuzzyCompare(m_doubleMinimum + 1.0, normalizedMin + 1.0) ||
                         !qFuzzyCompare(m_doubleMaximum + 1.0, normalizedMax + 1.0);
    m_doubleMinimum = normalizedMin;
    m_doubleMaximum = normalizedMax;
    if (m_doubleValue < m_doubleMinimum || m_doubleValue > m_doubleMaximum)
    {
        setDoubleValue(m_doubleValue);
    }
    updateNumberEditor();
    if (changed)
    {
        Q_EMIT doubleRangeChanged(m_doubleMinimum, m_doubleMaximum);
    }
}

int AntInputDialog::doubleDecimals() const
{
    return m_doubleDecimals;
}

void AntInputDialog::setDoubleDecimals(int decimals)
{
    const int normalized = qMax(0, decimals);
    if (m_doubleDecimals == normalized)
    {
        return;
    }
    m_doubleDecimals = normalized;
    updateNumberEditor();
    Q_EMIT doubleDecimalsChanged(m_doubleDecimals);
}

QStringList AntInputDialog::comboBoxItems() const
{
    return m_comboBoxItems;
}

void AntInputDialog::setComboBoxItems(const QStringList& items)
{
    if (stringListsEqual(m_comboBoxItems, items))
    {
        return;
    }
    m_comboBoxItems = items;
    updateComboItems();
    updateInputVisibility();
    updateOkButtonState();
    Q_EMIT comboBoxItemsChanged(m_comboBoxItems);
}

bool AntInputDialog::isComboBoxEditable() const
{
    return m_comboBoxEditable;
}

void AntInputDialog::setComboBoxEditable(bool editable)
{
    if (m_comboBoxEditable == editable)
    {
        return;
    }
    m_comboBoxEditable = editable;
    if (m_comboBox)
    {
        m_comboBox->setEditable(m_comboBoxEditable);
        wireComboEditField();
    }
    applyTextValueToEditors();
    updateOkButtonState();
    Q_EMIT comboBoxEditableChanged(m_comboBoxEditable);
}

QString AntInputDialog::okButtonText() const
{
    return m_okButtonText;
}

void AntInputDialog::setOkButtonText(const QString& text)
{
    const QString normalized = text.isEmpty() ? QStringLiteral("OK") : text;
    if (m_okButtonText == normalized)
    {
        return;
    }
    m_okButtonText = normalized;
    if (m_okButton)
    {
        m_okButton->setText(m_okButtonText);
    }
    Q_EMIT okButtonTextChanged(m_okButtonText);
}

QString AntInputDialog::cancelButtonText() const
{
    return m_cancelButtonText;
}

void AntInputDialog::setCancelButtonText(const QString& text)
{
    const QString normalized = text.isEmpty() ? QStringLiteral("Cancel") : text;
    if (m_cancelButtonText == normalized)
    {
        return;
    }
    m_cancelButtonText = normalized;
    if (m_cancelButton)
    {
        m_cancelButton->setText(m_cancelButtonText);
    }
    Q_EMIT cancelButtonTextChanged(m_cancelButtonText);
}

void AntInputDialog::setOption(InputDialogOption option, bool on)
{
    InputDialogOptions next = m_options;
    if (on)
    {
        next |= option;
    }
    else
    {
        next &= ~InputDialogOptions(option);
    }
    setOptions(next);
}

bool AntInputDialog::testOption(InputDialogOption option) const
{
    return m_options.testFlag(option);
}

void AntInputDialog::setOptions(InputDialogOptions options)
{
    if (m_options == options)
    {
        return;
    }
    m_options = options;
    updateInputVisibility();
    updateButtonRow();
    updateGeometry();
    Q_EMIT optionsChanged(m_options);
}

AntInputDialog::InputDialogOptions AntInputDialog::options() const
{
    return m_options;
}

void AntInputDialog::refreshAntStyle()
{
    AntDialog::refreshAntStyle();
    syncChildControls();
}

void AntInputDialog::done(int result)
{
    if (result == QDialog::Accepted)
    {
        syncValueFromActiveEditor();
        switch (m_inputMode)
        {
        case TextInput:
            Q_EMIT textValueSelected(m_textValue);
            break;
        case IntInput:
            Q_EMIT intValueSelected(m_intValue);
            break;
        case DoubleInput:
            Q_EMIT doubleValueSelected(m_doubleValue);
            break;
        }
    }
    AntDialog::done(result);
}

bool AntInputDialog::event(QEvent* event)
{
    const bool handled = AntDialog::event(event);
    switch (event->type())
    {
    case QEvent::ChildAdded:
    case QEvent::LayoutRequest:
    case QEvent::PaletteChange:
    case QEvent::Polish:
    case QEvent::StyleChange:
        scheduleChildSync();
        break;
    default:
        break;
    }
    return handled;
}

void AntInputDialog::showEvent(QShowEvent* event)
{
    AntDialog::showEvent(event);
    scheduleChildSync();
}

void AntInputDialog::changeEvent(QEvent* event)
{
    QDialog::changeEvent(event);
    if (event->type() == QEvent::EnabledChange ||
        event->type() == QEvent::FontChange ||
        event->type() == QEvent::PaletteChange)
    {
        scheduleChildSync();
    }
}

void AntInputDialog::initializeAntStyle()
{
    installAntStyle<AntInputDialogStyle>(this);
    setAttribute(Qt::WA_Hover, true);
    setAutoFillBackground(false);
    setMinimumSize(360, 170);
    resize(kInputDialogDefaultWidth, kInputDialogDefaultHeight);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void AntInputDialog::buildUi()
{
    const auto& token = antTheme->tokens();

    auto* rootLayout = new QVBoxLayout(contentWidget());
    rootLayout->setContentsMargins(token.paddingLG, token.paddingLG, token.paddingLG, token.paddingLG);
    rootLayout->setSpacing(token.marginSM);

    m_label = new AntTypography(contentWidget());
    m_label->setObjectName(QStringLiteral("antInputDialogLabel"));
    m_label->setWordWrap(true);
    m_label->setType(Ant::TypographyType::Default);
    m_label->setPixelSize(token.fontSize);
    rootLayout->addWidget(m_label);

    m_textInput = new AntInput(contentWidget());
    m_textInput->setObjectName(QStringLiteral("antInputDialogTextInput"));
    m_textInput->setAllowClear(true);
    m_textInput->setEchoMode(m_textEchoMode);
    rootLayout->addWidget(m_textInput);

    m_plainTextInput = new AntPlainTextEdit(contentWidget());
    m_plainTextInput->setObjectName(QStringLiteral("antInputDialogPlainTextInput"));
    m_plainTextInput->setMinimumHeight(96);
    rootLayout->addWidget(m_plainTextInput);

    m_numberInput = new AntInputNumber(contentWidget());
    m_numberInput->setObjectName(QStringLiteral("antInputDialogNumberInput"));
    m_numberInput->setMinimumWidth(180);
    rootLayout->addWidget(m_numberInput);

    m_comboBox = new AntSelect(contentWidget());
    m_comboBox->setObjectName(QStringLiteral("antInputDialogComboBox"));
    m_comboBox->setEditable(m_comboBoxEditable);
    m_comboBox->setMaxVisibleItems(8);
    rootLayout->addWidget(m_comboBox);

    rootLayout->addStretch();

    m_buttonRow = new QWidget(contentWidget());
    m_buttonRow->setObjectName(QStringLiteral("antInputDialogButtonRow"));
    auto* buttonLayout = new QHBoxLayout(m_buttonRow);
    buttonLayout->setContentsMargins(0, token.marginXS, 0, 0);
    buttonLayout->setSpacing(token.marginSM);
    buttonLayout->addStretch();

    m_cancelButton = new AntButton(m_cancelButtonText, m_buttonRow);
    m_cancelButton->setObjectName(QStringLiteral("antInputDialogCancelButton"));
    buttonLayout->addWidget(m_cancelButton);

    m_okButton = new AntButton(m_okButtonText, m_buttonRow);
    m_okButton->setObjectName(QStringLiteral("antInputDialogOkButton"));
    m_okButton->setButtonType(Ant::ButtonType::Primary);
    m_okButton->setDefault(true);
    buttonLayout->addWidget(m_okButton);
    rootLayout->addWidget(m_buttonRow);

    updateLabel();
    updateNumberEditor();
    updateComboItems();
    applyTextValueToEditors();
    applyInputMethodHintsToEditors();
}

void AntInputDialog::wireUi()
{
    connect(m_textInput, &AntInput::textChanged, this, &AntInputDialog::handleTextEdited);
    connect(m_textInput, &AntInput::returnPressed, this, [this]() {
        if (m_inputMode == TextInput && !usePlainTextEditForTextInput())
        {
            accept();
        }
    });

    connect(m_plainTextInput, &QPlainTextEdit::textChanged, this, [this]() {
        if (!m_updatingEditors)
        {
            handleTextEdited(m_plainTextInput->toPlainText());
        }
    });

    connect(m_numberInput,
            QOverload<double>::of(&AntInputNumber::valueChanged),
            this,
            [this](double value) {
                if (m_updatingEditors)
                {
                    return;
                }
                if (m_inputMode == IntInput)
                {
                    handleIntEdited(qRound(value));
                }
                else if (m_inputMode == DoubleInput)
                {
                    handleDoubleEdited(value);
                }
            });

    connect(m_comboBox, &AntSelect::currentTextChanged, this, [this](const QString& text) {
        if (m_updatingEditors)
        {
            return;
        }
        if (m_comboEditField)
        {
            const QSignalBlocker blocker(m_comboEditField);
            m_comboEditField->setText(text);
        }
        handleTextEdited(text);
    });
    wireComboEditField();

    connect(m_cancelButton, &AntButton::clicked, this, &QDialog::reject);
    connect(m_okButton, &AntButton::clicked, this, &QDialog::accept);
}

void AntInputDialog::updateInputVisibility()
{
    const bool textMode = m_inputMode == TextInput;
    const bool comboMode = textMode && useComboBoxForTextInput();
    const bool plainMode = textMode && usePlainTextEditForTextInput();

    if (m_textInput)
    {
        m_textInput->setVisible(textMode && !comboMode && !plainMode);
    }
    if (m_plainTextInput)
    {
        m_plainTextInput->setVisible(plainMode);
    }
    if (m_comboBox)
    {
        m_comboBox->setVisible(comboMode);
    }
    if (m_numberInput)
    {
        m_numberInput->setVisible(m_inputMode == IntInput || m_inputMode == DoubleInput);
    }

    updateNumberEditor();
    applyTextValueToEditors();
}

void AntInputDialog::updateLabel()
{
    if (m_label)
    {
        m_label->setText(m_labelText);
        m_label->setVisible(!m_labelText.isEmpty());
    }
}

void AntInputDialog::updateNumberEditor()
{
    if (!m_numberInput)
    {
        return;
    }

    const QSignalBlocker blocker(m_numberInput);
    if (m_inputMode == IntInput)
    {
        m_numberInput->setDecimals(0);
        m_numberInput->setRange(m_intMinimum, m_intMaximum);
        m_numberInput->setSingleStep(m_intStep);
        m_numberInput->setValue(m_intValue);
    }
    else
    {
        m_numberInput->setDecimals(m_doubleDecimals);
        m_numberInput->setRange(m_doubleMinimum, m_doubleMaximum);
        m_numberInput->setSingleStep(1.0);
        m_numberInput->setValue(m_doubleValue);
    }
}

void AntInputDialog::updateButtonRow()
{
    if (m_buttonRow)
    {
        m_buttonRow->setVisible(!testOption(NoButtons));
    }
    updateOkButtonState();
}

void AntInputDialog::updateComboItems()
{
    if (!m_comboBox)
    {
        return;
    }

    const QSignalBlocker blocker(m_comboBox);
    m_comboBox->clearOptions();
    m_comboBox->addOptions(m_comboBoxItems);
    m_comboBox->setEditable(m_comboBoxEditable);

    if (!m_comboBoxItems.isEmpty())
    {
        const int textIndex = m_comboBox->findText(m_textValue);
        m_comboBox->setCurrentIndex(textIndex >= 0 ? textIndex : 0);
    }
    else
    {
        m_comboBox->setCurrentIndex(-1);
    }
    wireComboEditField();
    applyTextValueToEditors();
}

void AntInputDialog::updateOkButtonState()
{
    if (!m_okButton)
    {
        return;
    }

    const bool comboNeedsSelection = m_inputMode == TextInput &&
                                     useComboBoxForTextInput() &&
                                     !m_comboBoxEditable &&
                                     m_comboBox &&
                                     m_comboBox->currentIndex() < 0;
    m_okButton->setEnabled(!comboNeedsSelection);
}

void AntInputDialog::applyTextValueToEditors()
{
    m_updatingEditors = true;

    if (m_textInput)
    {
        const QSignalBlocker blocker(m_textInput);
        m_textInput->setText(m_textValue);
    }
    if (m_plainTextInput)
    {
        const QSignalBlocker blocker(m_plainTextInput);
        m_plainTextInput->setPlainText(m_textValue);
    }
    if (m_comboBox)
    {
        const QSignalBlocker blocker(m_comboBox);
        const int index = m_comboBox->findText(m_textValue);
        if (index >= 0)
        {
            m_comboBox->setCurrentIndex(index);
            if (m_comboEditField)
            {
                const QSignalBlocker editBlocker(m_comboEditField);
                m_comboEditField->setText(m_textValue);
            }
        }
        else if (m_comboBoxEditable)
        {
            m_comboBox->setCurrentIndex(-1);
            wireComboEditField();
            if (m_comboEditField)
            {
                const QSignalBlocker editBlocker(m_comboEditField);
                m_comboEditField->setText(m_textValue);
            }
        }
    }

    m_updatingEditors = false;
}

void AntInputDialog::applyInputMethodHintsToEditors()
{
    const Qt::InputMethodHints hints = inputMethodHints();
    if (m_textInput && m_textInput->lineEdit())
    {
        m_textInput->lineEdit()->setInputMethodHints(hints);
    }
    if (m_plainTextInput)
    {
        m_plainTextInput->setInputMethodHints(hints);
    }
    if (m_numberInput)
    {
        if (auto* numberEdit = m_numberInput->findChild<QLineEdit*>())
        {
            numberEdit->setInputMethodHints(hints);
        }
    }
    if (m_comboBox)
    {
        m_comboBox->setInputMethodHints(hints);
    }
    if (m_comboEditField)
    {
        m_comboEditField->setInputMethodHints(hints);
    }
}

void AntInputDialog::syncValueFromActiveEditor()
{
    switch (m_inputMode)
    {
    case TextInput:
        if (useComboBoxForTextInput())
        {
            setTextValue(comboTextValue());
        }
        else if (usePlainTextEditForTextInput() && m_plainTextInput)
        {
            setTextValue(m_plainTextInput->toPlainText());
        }
        else if (m_textInput)
        {
            setTextValue(m_textInput->text());
        }
        break;
    case IntInput:
        if (m_numberInput)
        {
            setIntValue(qRound(m_numberInput->value()));
        }
        break;
    case DoubleInput:
        if (m_numberInput)
        {
            setDoubleValue(m_numberInput->value());
        }
        break;
    }
}

QString AntInputDialog::comboTextValue() const
{
    if (!m_comboBox)
    {
        return m_textValue;
    }
    if (m_comboBoxEditable && m_comboEditField &&
        (m_comboBox->currentIndex() < 0 || !m_comboEditField->text().isEmpty()))
    {
        return m_comboEditField->text();
    }
    return m_comboBox->currentText();
}

bool AntInputDialog::useComboBoxForTextInput() const
{
    return !m_comboBoxItems.isEmpty();
}

bool AntInputDialog::usePlainTextEditForTextInput() const
{
    return testOption(UsePlainTextEditForTextInput) && !useComboBoxForTextInput();
}

void AntInputDialog::wireComboEditField()
{
    if (!m_comboBox || !m_comboBoxEditable)
    {
        return;
    }

    auto* editField = m_comboBox->findChild<QLineEdit*>();
    if (!editField || editField == m_comboEditField)
    {
        return;
    }

    m_comboEditField = editField;
    m_comboEditField->setInputMethodHints(inputMethodHints());
    connect(m_comboEditField,
            &QLineEdit::textChanged,
            this,
            &AntInputDialog::handleTextEdited,
            Qt::UniqueConnection);
}

void AntInputDialog::handleTextEdited(const QString& text)
{
    if (m_updatingEditors)
    {
        return;
    }

    if (m_textValue == text)
    {
        updateOkButtonState();
        return;
    }
    m_textValue = text;
    updateOkButtonState();
    Q_EMIT textValueChanged(m_textValue);
}

void AntInputDialog::handleIntEdited(int value)
{
    const int normalized = qBound(m_intMinimum, value, m_intMaximum);
    if (m_intValue == normalized)
    {
        return;
    }
    m_intValue = normalized;
    Q_EMIT intValueChanged(m_intValue);
}

void AntInputDialog::handleDoubleEdited(double value)
{
    const double normalized = qBound(m_doubleMinimum, value, m_doubleMaximum);
    if (qFuzzyCompare(m_doubleValue + 1.0, normalized + 1.0))
    {
        return;
    }
    m_doubleValue = normalized;
    Q_EMIT doubleValueChanged(m_doubleValue);
}

void AntInputDialog::scheduleChildSync()
{
    if (m_childSyncQueued)
    {
        return;
    }
    m_childSyncQueued = true;
    QTimer::singleShot(0, this, [this]() {
        m_childSyncQueued = false;
        syncChildControls();
    });
}

void AntInputDialog::syncChildControls()
{
    applyInputMethodHintsToEditors();
    setProperty("antInputDialogUsesNativeDialog", false);
    setProperty("antInputDialogChildSyncCount", property("antInputDialogChildSyncCount").toInt() + 1);
}
