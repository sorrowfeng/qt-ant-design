#pragma once

#include "core/QtAntDesignExport.h"

#include <QPointer>
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
    void requiredChanged(bool required);
    void colonChanged(bool colon);
    void validateStatusChanged(Ant::Status status);

protected:
    void changeEvent(QEvent* event) override;

private:
    void rebuildLayout();
    void updateTheme();
    QString effectiveLabelText() const;
    QColor helpColor() const;

    QString m_label;
    QString m_helpText;
    QString m_extra;
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
    QLabel* m_extraLabel = nullptr;
    QLabel* m_helpLabel = nullptr;
};

class AntForm; // forward declaration

class QT_ANT_DESIGN_EXPORT AntFormProvider : public QWidget
{
    Q_OBJECT

public:
    explicit AntFormProvider(QWidget* parent = nullptr);

    void addForm(AntForm* form, const QString& name = QString());
    void removeForm(AntForm* form);
    QList<AntForm*> forms() const;

Q_SIGNALS:
    void formChanged(const QString& formName, const QString& fieldName, const QVariant& value);
    void formFinished(const QString& formName, const QVariantMap& values);

private:
    struct FormEntry
    {
        AntForm* form = nullptr;
        QString name;
    };
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

Q_SIGNALS:
    void formLayoutChanged(Ant::FormLayout layout);
    void labelAlignChanged(Ant::FormLabelAlign align);
    void colonChanged(bool colon);
    void requiredMarkChanged(bool show);
    void labelWidthChanged(int width);
    void itemSpacingChanged(int spacing);

protected:
    void changeEvent(QEvent* event) override;

private:
    void rebuildLayout();
    void applyItemSettings();

    Ant::FormLayout m_formLayout = Ant::FormLayout::Horizontal;
    Ant::FormLabelAlign m_labelAlign = Ant::FormLabelAlign::Right;
    bool m_colon = true;
    bool m_requiredMark = true;
    int m_labelWidth = 96;
    int m_itemSpacing = 16;
    QBoxLayout* m_layout = nullptr;
    QList<AntFormItem*> m_items;
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
    void updateAddButton();

    int m_minCount = 0;
    int m_maxCount = 0; // 0 = unlimited
    AntFormListItemFactory m_factory;
    QVBoxLayout* m_layout = nullptr;
    QPushButton* m_addButton = nullptr;
    QList<ListItem> m_items;
};
