#pragma once

#include "core/QtAntDesignExport.h"

#include <QLineEdit>
#include <QStringList>

#include "AntDialog.h"

class AntButton;
class AntInput;
class AntInputNumber;
class AntPlainTextEdit;
class AntSelect;
class AntTypography;
class QEvent;
class QHBoxLayout;
class QLineEdit;
class QShowEvent;
class QVBoxLayout;
class QWidget;

class QT_ANT_DESIGN_EXPORT AntInputDialog : public AntDialog
{
    Q_OBJECT
    Q_PROPERTY(InputMode inputMode READ inputMode WRITE setInputMode NOTIFY inputModeChanged)
    Q_PROPERTY(QString labelText READ labelText WRITE setLabelText NOTIFY labelTextChanged)
    Q_PROPERTY(QString textValue READ textValue WRITE setTextValue NOTIFY textValueChanged)
    Q_PROPERTY(QLineEdit::EchoMode textEchoMode READ textEchoMode WRITE setTextEchoMode NOTIFY textEchoModeChanged)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
    Q_PROPERTY(int intValue READ intValue WRITE setIntValue NOTIFY intValueChanged)
    Q_PROPERTY(int intMinimum READ intMinimum WRITE setIntMinimum NOTIFY intRangeChanged)
    Q_PROPERTY(int intMaximum READ intMaximum WRITE setIntMaximum NOTIFY intRangeChanged)
    Q_PROPERTY(int intStep READ intStep WRITE setIntStep NOTIFY intStepChanged)
    Q_PROPERTY(double doubleValue READ doubleValue WRITE setDoubleValue NOTIFY doubleValueChanged)
    Q_PROPERTY(double doubleMinimum READ doubleMinimum WRITE setDoubleMinimum NOTIFY doubleRangeChanged)
    Q_PROPERTY(double doubleMaximum READ doubleMaximum WRITE setDoubleMaximum NOTIFY doubleRangeChanged)
    Q_PROPERTY(int doubleDecimals READ doubleDecimals WRITE setDoubleDecimals NOTIFY doubleDecimalsChanged)
    Q_PROPERTY(QStringList comboBoxItems READ comboBoxItems WRITE setComboBoxItems NOTIFY comboBoxItemsChanged)
    Q_PROPERTY(bool comboBoxEditable READ isComboBoxEditable WRITE setComboBoxEditable NOTIFY comboBoxEditableChanged)
    Q_PROPERTY(QString okButtonText READ okButtonText WRITE setOkButtonText NOTIFY okButtonTextChanged)
    Q_PROPERTY(QString cancelButtonText READ cancelButtonText WRITE setCancelButtonText NOTIFY cancelButtonTextChanged)
    Q_PROPERTY(InputDialogOptions options READ options WRITE setOptions NOTIFY optionsChanged)

public:
    enum InputMode
    {
        TextInput,
        IntInput,
        DoubleInput
    };
    Q_ENUM(InputMode)

    enum InputDialogOption
    {
        NoButtons = 0x00000001,
        UseListViewForComboBoxItems = 0x00000002,
        UsePlainTextEditForTextInput = 0x00000004
    };
    Q_DECLARE_FLAGS(InputDialogOptions, InputDialogOption)
    Q_FLAG(InputDialogOptions)

    explicit AntInputDialog(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::Dialog);

    static QString getText(QWidget* parent,
                           const QString& title,
                           const QString& label,
                           QLineEdit::EchoMode mode = QLineEdit::Normal,
                           const QString& text = QString(),
                           bool* ok = nullptr,
                           Qt::WindowFlags flags = Qt::WindowFlags(),
                           Qt::InputMethodHints inputMethodHints = Qt::ImhNone);
    static int getInt(QWidget* parent,
                      const QString& title,
                      const QString& label,
                      int value = 0,
                      int minValue = -2147483647,
                      int maxValue = 2147483647,
                      int step = 1,
                      bool* ok = nullptr,
                      Qt::WindowFlags flags = Qt::WindowFlags());
    static double getDouble(QWidget* parent,
                            const QString& title,
                            const QString& label,
                            double value = 0,
                            double minValue = -2147483647,
                            double maxValue = 2147483647,
                            int decimals = 1,
                            bool* ok = nullptr,
                            Qt::WindowFlags flags = Qt::WindowFlags());
    static QString getItem(QWidget* parent,
                           const QString& title,
                           const QString& label,
                           const QStringList& items,
                           int current = 0,
                           bool editable = true,
                           bool* ok = nullptr,
                           Qt::WindowFlags flags = Qt::WindowFlags(),
                           Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

    InputMode inputMode() const;
    void setInputMode(InputMode mode);

    QString labelText() const;
    void setLabelText(const QString& text);

    QString textValue() const;
    void setTextValue(const QString& text);

    QLineEdit::EchoMode textEchoMode() const;
    void setTextEchoMode(QLineEdit::EchoMode mode);

    QString placeholderText() const;
    void setPlaceholderText(const QString& text);
    Qt::InputMethodHints inputMethodHints() const;
    void setInputMethodHints(Qt::InputMethodHints hints);

    int intValue() const;
    void setIntValue(int value);
    int intMinimum() const;
    void setIntMinimum(int min);
    int intMaximum() const;
    void setIntMaximum(int max);
    void setIntRange(int min, int max);
    int intStep() const;
    void setIntStep(int step);

    double doubleValue() const;
    void setDoubleValue(double value);
    double doubleMinimum() const;
    void setDoubleMinimum(double min);
    double doubleMaximum() const;
    void setDoubleMaximum(double max);
    void setDoubleRange(double min, double max);
    int doubleDecimals() const;
    void setDoubleDecimals(int decimals);

    QStringList comboBoxItems() const;
    void setComboBoxItems(const QStringList& items);
    bool isComboBoxEditable() const;
    void setComboBoxEditable(bool editable);

    QString okButtonText() const;
    void setOkButtonText(const QString& text);
    QString cancelButtonText() const;
    void setCancelButtonText(const QString& text);

    void setOption(InputDialogOption option, bool on = true);
    bool testOption(InputDialogOption option) const;
    void setOptions(InputDialogOptions options);
    InputDialogOptions options() const;

    void refreshAntStyle() override;

Q_SIGNALS:
    void inputModeChanged(AntInputDialog::InputMode mode);
    void labelTextChanged(const QString& text);
    void textValueChanged(const QString& text);
    void textValueSelected(const QString& text);
    void textEchoModeChanged(QLineEdit::EchoMode mode);
    void placeholderTextChanged(const QString& text);
    void intValueChanged(int value);
    void intValueSelected(int value);
    void intRangeChanged(int min, int max);
    void intStepChanged(int step);
    void doubleValueChanged(double value);
    void doubleValueSelected(double value);
    void doubleRangeChanged(double min, double max);
    void doubleDecimalsChanged(int decimals);
    void comboBoxItemsChanged(const QStringList& items);
    void comboBoxEditableChanged(bool editable);
    void okButtonTextChanged(const QString& text);
    void cancelButtonTextChanged(const QString& text);
    void optionsChanged(AntInputDialog::InputDialogOptions options);

public Q_SLOTS:
    void done(int result) override;

protected:
    bool event(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void initializeAntStyle();
    void buildUi();
    void wireUi();
    void updateInputVisibility();
    void updateLabel();
    void updateNumberEditor();
    void updateButtonRow();
    void updateComboItems();
    void updateOkButtonState();
    void applyTextValueToEditors();
    void applyInputMethodHintsToEditors();
    void syncValueFromActiveEditor();
    QString comboTextValue() const;
    bool useComboBoxForTextInput() const;
    bool usePlainTextEditForTextInput() const;
    void wireComboEditField();
    void handleTextEdited(const QString& text);
    void handleIntEdited(int value);
    void handleDoubleEdited(double value);
    void scheduleChildSync();
    void syncChildControls();

    AntTypography* m_label = nullptr;
    AntInput* m_textInput = nullptr;
    AntPlainTextEdit* m_plainTextInput = nullptr;
    AntInputNumber* m_numberInput = nullptr;
    AntSelect* m_comboBox = nullptr;
    QWidget* m_buttonRow = nullptr;
    AntButton* m_okButton = nullptr;
    AntButton* m_cancelButton = nullptr;
    QLineEdit* m_comboEditField = nullptr;

    InputMode m_inputMode = TextInput;
    QString m_labelText;
    QString m_textValue;
    QString m_placeholderText;
    QLineEdit::EchoMode m_textEchoMode = QLineEdit::Normal;
    int m_intValue = 0;
    int m_intMinimum = -2147483647;
    int m_intMaximum = 2147483647;
    int m_intStep = 1;
    double m_doubleValue = 0.0;
    double m_doubleMinimum = -2147483647.0;
    double m_doubleMaximum = 2147483647.0;
    int m_doubleDecimals = 1;
    QStringList m_comboBoxItems;
    bool m_comboBoxEditable = true;
    QString m_okButtonText;
    QString m_cancelButtonText;
    InputDialogOptions m_options;
    bool m_updatingEditors = false;
    bool m_childSyncQueued = false;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AntInputDialog::InputDialogOptions)
