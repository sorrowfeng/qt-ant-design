#pragma once

#include "core/QtAntDesignExport.h"

#include <QPointer>
#include <QMetaObject>
#include <QVariantMap>
#include <QWidget>

#include "core/AntTypes.h"

class QLabel;
class QBoxLayout;
class QPushButton;
class QVBoxLayout;

class QT_ANT_DESIGN_EXPORT AntFormItem : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)
    Q_PROPERTY(QString helpText READ helpText WRITE setHelpText NOTIFY helpTextChanged)
    Q_PROPERTY(QString extra READ extra WRITE setExtra NOTIFY extraChanged)
    Q_PROPERTY(QString fieldName READ fieldName WRITE setFieldName NOTIFY fieldNameChanged)
    Q_PROPERTY(QVariant fieldValue READ fieldValue WRITE setFieldValue NOTIFY fieldValueChanged)
    Q_PROPERTY(bool required READ isRequired WRITE setRequired NOTIFY requiredChanged)
    Q_PROPERTY(bool colon READ colon WRITE setColon NOTIFY colonChanged)
    Q_PROPERTY(Ant::Status validateStatus READ validateStatus WRITE setValidateStatus NOTIFY validateStatusChanged)

public:
    explicit AntFormItem(QWidget* parent = nullptr);

    QString label() const;
    void setLabel(const QString& label);

    QString helpText() const;
    void setHelpText(const QString& text);

    QString extra() const;
    void setExtra(const QString& text);

    QString fieldName() const;
    void setFieldName(const QString& name);

    QVariant fieldValue() const;
    void setFieldValue(const QVariant& value);

    bool isRequired() const;
    void setRequired(bool required);

    bool colon() const;
    void setColon(bool colon);

    Ant::Status validateStatus() const;
    void setValidateStatus(Ant::Status status);

    QWidget* fieldWidget() const;
    void setFieldWidget(QWidget* widget);

    void applyFormSettings(Ant::FormLayout layoutMode,
                           Ant::FormLabelAlign labelAlign,
                           bool showColon,
                           bool showRequiredMark,
                           int labelWidth);

Q_SIGNALS:
    void labelChanged(const QString& label);
    void helpTextChanged(const QString& text);
    void extraChanged(const QString& text);
    void fieldNameChanged(const QString& name);
    void fieldValueChanged(const QVariant& value);
    void requiredChanged(bool required);
    void colonChanged(bool colon);
    void validateStatusChanged(Ant::Status status);

protected:
    void changeEvent(QEvent* event) override;

private:
    Q_SLOT void handleFieldWidgetValueChanged();
    void bindFieldWidgetValue();
    void rebuildLayout();
    void updateLabelPresentation(bool countUpdate = false);
    void updateAssistText(bool countUpdate = false);
    void updateTheme();
    void syncFormItemPerfCounters() const;
    QString effectiveLabelText() const;
    QColor helpColor() const;

    QString m_label;
    QString m_helpText;
    QString m_extra;
    QString m_fieldName;
    bool m_required = false;
    bool m_colon = true;
    bool m_useFormColon = true;
    Ant::Status m_validateStatus = Ant::Status::Normal;
    Ant::FormLayout m_layoutMode = Ant::FormLayout::Horizontal;
    Ant::FormLabelAlign m_labelAlign = Ant::FormLabelAlign::Right;
    bool m_showRequiredMark = true;
    int m_labelWidth = 96;
    QBoxLayout* m_rootLayout = nullptr;
    QWidget* m_labelContainer = nullptr;
    QLabel* m_requiredLabel = nullptr;
    QLabel* m_labelWidget = nullptr;
    QWidget* m_fieldColumn = nullptr;
    QBoxLayout* m_fieldColumnLayout = nullptr;
    QPointer<QWidget> m_fieldWidget;
    QByteArray m_fieldValueProperty;
    QMetaObject::Connection m_fieldValueConnection;
    QLabel* m_extraLabel = nullptr;
    QLabel* m_helpLabel = nullptr;
    int m_layoutRebuildCount = 0;
    int m_inlineTextUpdateCount = 0;
    int m_settingsSkipCount = 0;
};

class AntForm; // forward declaration

class QT_ANT_DESIGN_EXPORT AntFormProvider : public QWidget
{
    Q_OBJECT

public:
    explicit AntFormProvider(QWidget* parent = nullptr);
    ~AntFormProvider() override;

    void addForm(AntForm* form, const QString& name = QString());
    void removeForm(AntForm* form);
    QList<AntForm*> forms() const;

Q_SIGNALS:
    void formChanged(const QString& formName, const QString& fieldName, const QVariant& value);
    void formFinished(const QString& formName, const QVariantMap& values);

private:
    struct FormEntry
    {
        // QObject avoids a typed QPointer downcast while Qt5 emits destroyed().
        QPointer<QObject> form;
        QString name;
        QMetaObject::Connection fieldConnection;
        QMetaObject::Connection finishConnection;
        QMetaObject::Connection destroyedConnection;
    };
    void removeDestroyedForms(QObject* destroyedForm = nullptr);
    QList<FormEntry> m_forms;
};

class QT_ANT_DESIGN_EXPORT AntForm : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Ant::FormLayout formLayout READ formLayout WRITE setFormLayout NOTIFY formLayoutChanged)
    Q_PROPERTY(Ant::FormLabelAlign labelAlign READ labelAlign WRITE setLabelAlign NOTIFY labelAlignChanged)
    Q_PROPERTY(bool colon READ colon WRITE setColon NOTIFY colonChanged)
    Q_PROPERTY(bool requiredMark READ requiredMark WRITE setRequiredMark NOTIFY requiredMarkChanged)
    Q_PROPERTY(int labelWidth READ labelWidth WRITE setLabelWidth NOTIFY labelWidthChanged)
    Q_PROPERTY(int itemSpacing READ itemSpacing WRITE setItemSpacing NOTIFY itemSpacingChanged)

public:
    explicit AntForm(QWidget* parent = nullptr);
    ~AntForm() override;

    Ant::FormLayout formLayout() const;
    void setFormLayout(Ant::FormLayout layout);

    Ant::FormLabelAlign labelAlign() const;
    void setLabelAlign(Ant::FormLabelAlign align);

    bool colon() const;
    void setColon(bool colon);

    bool requiredMark() const;
    void setRequiredMark(bool show);

    int labelWidth() const;
    void setLabelWidth(int width);

    int itemSpacing() const;
    void setItemSpacing(int spacing);

    QList<AntFormItem*> items() const;
    void addItem(AntFormItem* item);
    AntFormItem* addItem(const QString& label, QWidget* fieldWidget, bool required = false);
    void clearItems();

    QVariant fieldValue(const QString& fieldName) const;
    QVariantMap values() const;

public Q_SLOTS:
    // Explicitly report a custom field that does not expose a Qt property with
    // a NOTIFY signal. Standard Ant/Qt inputs are observed automatically.
    void notifyFieldChanged(const QString& fieldName, const QVariant& value);
    void finish();

Q_SIGNALS:
    void formLayoutChanged(Ant::FormLayout layout);
    void labelAlignChanged(Ant::FormLabelAlign align);
    void colonChanged(bool colon);
    void requiredMarkChanged(bool show);
    void labelWidthChanged(int width);
    void itemSpacingChanged(int spacing);
    void fieldChanged(const QString& fieldName, const QVariant& value);
    void finished(const QVariantMap& values);

protected:
    void changeEvent(QEvent* event) override;

private:
    void rebuildLayout();
    void applyItemSettings();
    void syncFormPerfCounters() const;
    void connectItem(AntFormItem* item);
    QString effectiveFieldName(const AntFormItem* item) const;
    void removeDestroyedItems(QObject* destroyedItem = nullptr);

    Ant::FormLayout m_formLayout = Ant::FormLayout::Horizontal;
    Ant::FormLabelAlign m_labelAlign = Ant::FormLabelAlign::Right;
    bool m_colon = true;
    bool m_requiredMark = true;
    int m_labelWidth = 96;
    int m_itemSpacing = 16;
    QBoxLayout* m_layout = nullptr;
    // QObject avoids a typed QPointer downcast while Qt5 emits destroyed().
    QList<QPointer<QObject>> m_items;
    QVariantMap m_customFieldValues;
    int m_layoutRebuildCount = 0;
    int m_itemSettingsApplyCount = 0;
    int m_spacingUpdateCount = 0;
};

using AntFormListItemFactory = std::function<QWidget*(int index)>;

class QT_ANT_DESIGN_EXPORT AntFormList : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int minCount READ minCount WRITE setMinCount NOTIFY minCountChanged)
    Q_PROPERTY(int maxCount READ maxCount WRITE setMaxCount NOTIFY maxCountChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit AntFormList(QWidget* parent = nullptr);

    int minCount() const;
    void setMinCount(int count);

    int maxCount() const;
    void setMaxCount(int count);

    int count() const;

    void setItemFactory(AntFormListItemFactory factory);

    void addItem();
    void removeItem(int index);
    void clearItems();

    QList<QVariantMap> itemValues() const;

Q_SIGNALS:
    void minCountChanged(int count);
    void maxCountChanged(int count);
    void countChanged(int count);
    void itemAdded(int index);
    void itemRemoved(int index);
    void fieldsChanged(const QList<QVariantMap>& values);

protected:
    void changeEvent(QEvent* event) override;

private:
    struct ListItem
    {
        QWidget* container = nullptr;
        QWidget* content = nullptr;
        QPushButton* removeButton = nullptr;
    };

    void rebuildAll();
    void refreshTheme();
    void updateAddButton();

    int m_minCount = 0;
    int m_maxCount = 0; // 0 = unlimited
    AntFormListItemFactory m_factory;
    QVBoxLayout* m_layout = nullptr;
    QPushButton* m_addButton = nullptr;
    QList<ListItem> m_items;
};
