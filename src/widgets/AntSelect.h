#pragma once

#include <QList>
#include <QStringList>
#include <QVariant>
#include <QWidget>

#include "core/AntTypes.h"

class QEnterEvent;
class QEvent;
class QLineEdit;
class QFrame;
class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QPropertyAnimation;
class QTimer;
class QVBoxLayout;

struct AntSelectOption
{
    QString label;
    QVariant value;
    bool disabled = false;
};

class AntSelect : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Ant::Size selectSize READ selectSize WRITE setSelectSize NOTIFY selectSizeChanged)
    Q_PROPERTY(Ant::Status status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(Ant::Variant variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
    Q_PROPERTY(bool allowClear READ allowClear WRITE setAllowClear NOTIFY allowClearChanged)
    Q_PROPERTY(bool loading READ isLoading WRITE setLoading NOTIFY loadingChanged)
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int maxVisibleItems READ maxVisibleItems WRITE setMaxVisibleItems NOTIFY maxVisibleItemsChanged)
    Q_PROPERTY(Ant::SelectMode selectMode READ selectMode WRITE setSelectMode NOTIFY selectModeChanged)
    Q_PROPERTY(int maxTagCount READ maxTagCount WRITE setMaxTagCount NOTIFY maxTagCountChanged)
    Q_PROPERTY(qreal arrowRotation READ arrowRotation WRITE setArrowRotation)

public:
    explicit AntSelect(QWidget* parent = nullptr);
    ~AntSelect() override;

    Ant::Size selectSize() const;
    void setSelectSize(Ant::Size size);

    Ant::Status status() const;
    void setStatus(Ant::Status status);

    Ant::Variant variant() const;
    void setVariant(Ant::Variant variant);

    QString placeholderText() const;
    void setPlaceholderText(const QString& text);

    bool allowClear() const;
    void setAllowClear(bool allowClear);

    bool isEditable() const;
    void setEditable(bool editable);

    bool isLoading() const;
    void setLoading(bool loading);

    bool isOpen() const;
    void setOpen(bool open);

    int currentIndex() const;
    void setCurrentIndex(int index);
    void setCurrentText(const QString& text);

    QString currentText() const;
    QVariant currentValue() const;
    QVariant currentData(int role = Qt::UserRole) const;

    int count() const;
    AntSelectOption optionAt(int index) const;
    void addOption(const QString& label, const QVariant& value = QVariant(), bool disabled = false);
    void addOptions(const QStringList& labels);
    void clearOptions();
    void addItem(const QString& text, const QVariant& userData = QVariant());
    void addItems(const QStringList& texts);
    void insertItem(int index, const QString& text, const QVariant& userData = QVariant());
    void insertItems(int index, const QStringList& texts);
    void removeItem(int index);
    QString itemText(int index) const;
    void setItemText(int index, const QString& text);
    QVariant itemData(int index, int role = Qt::UserRole) const;
    void setItemData(int index, const QVariant& value, int role = Qt::UserRole);
    int findText(const QString& text,
                 Qt::MatchFlags flags = Qt::MatchExactly | Qt::MatchCaseSensitive) const;
    int findData(const QVariant& data,
                 int role = Qt::UserRole,
                 Qt::MatchFlags flags = Qt::MatchExactly | Qt::MatchCaseSensitive) const;
    void clear();

    int maxVisibleItems() const;
    void setMaxVisibleItems(int count);

    Ant::SelectMode selectMode() const;
    void setSelectMode(Ant::SelectMode mode);

    int maxTagCount() const;
    void setMaxTagCount(int count);

    QList<int> selectedIndices() const;
    QList<QVariant> selectedValues() const;
    QStringList selectedTexts() const;
    void setSelectedIndices(const QList<int>& indices);
    void addSelectedIndex(int index);
    void removeSelectedIndex(int index);
    void clearSelection();

    void addTag(const QString& text);

    qreal arrowRotation() const;
    void setArrowRotation(qreal rotation);
    bool isHoveredState() const;
    bool isPressedState() const;
    int loadingAngle() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void selectSizeChanged(Ant::Size size);
    void statusChanged(Ant::Status status);
    void variantChanged(Ant::Variant variant);
    void placeholderTextChanged(const QString& text);
    void allowClearChanged(bool allowClear);
    void editableChanged(bool editable);
    void loadingChanged(bool loading);
    void openChanged(bool open);
    void currentIndexChanged(int index);
    void currentTextChanged(const QString& text);
    void currentValueChanged(const QVariant& value);
    void optionSelected(int index, const QVariant& value);
    void activated(int index);
    void textActivated(const QString& text);
    void highlighted(int index);
    void textHighlighted(const QString& text);
    void cleared();
    void maxVisibleItemsChanged(int count);
    void selectModeChanged(Ant::SelectMode mode);
    void maxTagCountChanged(int count);
    void selectionChanged(const QList<QVariant>& values);
    void tagRemoved(const QVariant& value);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    friend class AntSelectPopup;
    friend class AntSelectOptionWidget;

    struct Metrics
    {
        int height = 32;
        int fontSize = 14;
        int radius = 6;
        int paddingX = 11;
        int arrowWidth = 28;
        int optionHeight = 32;
    };

    Metrics metrics() const;
    QRectF controlRect() const;
    QRectF clearButtonRect(const Metrics& metrics) const;
    QColor borderColor() const;
    QColor backgroundColor() const;
    void rebuildPopup();
    void updatePopupGeometry();
    void selectOptionFromPopup(int index);
    void toggleOptionFromPopup(int index);
    void setHighlightedIndex(int index);
    int nextEnabledIndex(int start, int direction) const;
    void animateArrow(bool open);
    void updateCursor();
    bool canClear() const;

    QList<AntSelectOption> m_options;
    int m_currentIndex = -1;
    int m_highlightedIndex = -1;
    int m_maxVisibleItems = 6;
    Ant::SelectMode m_selectMode = Ant::SelectMode::Single;
    QList<int> m_selectedIndices;
    int m_maxTagCount = 0; // 0 = show all
    Ant::Size m_selectSize = Ant::Size::Middle;
    Ant::Status m_status = Ant::Status::Normal;
    Ant::Variant m_variant = Ant::Variant::Outlined;
    QString m_placeholderText;
    bool m_allowClear = false;
    bool m_editable = false;
    bool m_loading = false;
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_open = false;
    int m_loadingAngle = 0;
    qreal m_arrowRotation = 0.0;
    QFrame* m_popup = nullptr;
    QVBoxLayout* m_popupLayout = nullptr;
    QPropertyAnimation* m_arrowAnimation = nullptr;
    QTimer* m_loadingTimer = nullptr;
    QLineEdit* m_editField = nullptr;
};
