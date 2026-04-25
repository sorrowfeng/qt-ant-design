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
    Q_PROPERTY(Ant::SelectSize selectSize READ selectSize WRITE setSelectSize NOTIFY selectSizeChanged)
    Q_PROPERTY(Ant::SelectStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(Ant::SelectVariant variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
    Q_PROPERTY(bool allowClear READ allowClear WRITE setAllowClear NOTIFY allowClearChanged)
    Q_PROPERTY(bool loading READ isLoading WRITE setLoading NOTIFY loadingChanged)
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int maxVisibleItems READ maxVisibleItems WRITE setMaxVisibleItems NOTIFY maxVisibleItemsChanged)
    Q_PROPERTY(qreal arrowRotation READ arrowRotation WRITE setArrowRotation)

public:
    explicit AntSelect(QWidget* parent = nullptr);
    ~AntSelect() override;

    Ant::SelectSize selectSize() const;
    void setSelectSize(Ant::SelectSize size);

    Ant::SelectStatus status() const;
    void setStatus(Ant::SelectStatus status);

    Ant::SelectVariant variant() const;
    void setVariant(Ant::SelectVariant variant);

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

    QString currentText() const;
    QVariant currentValue() const;

    int count() const;
    AntSelectOption optionAt(int index) const;
    void addOption(const QString& label, const QVariant& value = QVariant(), bool disabled = false);
    void addOptions(const QStringList& labels);
    void clearOptions();

    int maxVisibleItems() const;
    void setMaxVisibleItems(int count);

    qreal arrowRotation() const;
    void setArrowRotation(qreal rotation);
    bool isHoveredState() const;
    bool isPressedState() const;
    int loadingAngle() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void selectSizeChanged(Ant::SelectSize size);
    void statusChanged(Ant::SelectStatus status);
    void variantChanged(Ant::SelectVariant variant);
    void placeholderTextChanged(const QString& text);
    void allowClearChanged(bool allowClear);
    void editableChanged(bool editable);
    void loadingChanged(bool loading);
    void openChanged(bool open);
    void currentIndexChanged(int index);
    void currentTextChanged(const QString& text);
    void currentValueChanged(const QVariant& value);
    void optionSelected(int index, const QVariant& value);
    void cleared();
    void maxVisibleItemsChanged(int count);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
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
    void setHighlightedIndex(int index);
    int nextEnabledIndex(int start, int direction) const;
    void animateArrow(bool open);
    void updateCursor();
    bool canClear() const;

    QList<AntSelectOption> m_options;
    int m_currentIndex = -1;
    int m_highlightedIndex = -1;
    int m_maxVisibleItems = 6;
    Ant::SelectSize m_selectSize = Ant::SelectSize::Middle;
    Ant::SelectStatus m_status = Ant::SelectStatus::Normal;
    Ant::SelectVariant m_variant = Ant::SelectVariant::Outlined;
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
