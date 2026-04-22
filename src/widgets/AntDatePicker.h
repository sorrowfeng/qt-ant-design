#pragma once

#include <QDate>
#include <QWidget>

#include "core/AntTypes.h"

class QEnterEvent;
class QEvent;
class QFocusEvent;
class QFrame;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;

class AntDatePicker : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QDate selectedDate READ selectedDate WRITE setSelectedDate NOTIFY selectedDateChanged)
    Q_PROPERTY(QString displayFormat READ displayFormat WRITE setDisplayFormat NOTIFY displayFormatChanged)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
    Q_PROPERTY(Ant::DatePickerSize pickerSize READ pickerSize WRITE setPickerSize NOTIFY pickerSizeChanged)
    Q_PROPERTY(Ant::DatePickerStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(Ant::DatePickerVariant variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(bool allowClear READ allowClear WRITE setAllowClear NOTIFY allowClearChanged)
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)

public:
    explicit AntDatePicker(QWidget* parent = nullptr);
    ~AntDatePicker() override;

    QDate selectedDate() const;
    void setSelectedDate(const QDate& date);
    bool hasSelectedDate() const;
    void clear();

    QString displayFormat() const;
    void setDisplayFormat(const QString& format);

    QString placeholderText() const;
    void setPlaceholderText(const QString& text);

    Ant::DatePickerSize pickerSize() const;
    void setPickerSize(Ant::DatePickerSize size);

    Ant::DatePickerStatus status() const;
    void setStatus(Ant::DatePickerStatus status);

    Ant::DatePickerVariant variant() const;
    void setVariant(Ant::DatePickerVariant variant);

    bool allowClear() const;
    void setAllowClear(bool allowClear);

    bool isOpen() const;
    void setOpen(bool open);

    QString dateString() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void selectedDateChanged(const QDate& date);
    void dateStringChanged(const QString& text);
    void displayFormatChanged(const QString& format);
    void placeholderTextChanged(const QString& text);
    void pickerSizeChanged(Ant::DatePickerSize size);
    void statusChanged(Ant::DatePickerStatus status);
    void variantChanged(Ant::DatePickerVariant variant);
    void allowClearChanged(bool allowClear);
    void openChanged(bool open);
    void cleared();

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    friend class AntDatePickerPopup;

    struct Metrics
    {
        int height = 32;
        int fontSize = 14;
        int radius = 6;
        int paddingX = 11;
        int iconWidth = 30;
    };

    Metrics metrics() const;
    QRectF controlRect() const;
    QRectF iconRect(const Metrics& metrics) const;
    QColor borderColor() const;
    QColor backgroundColor() const;
    bool canClear() const;
    void updatePopupGeometry();
    void setPanelDate(const QDate& date);
    void selectDateFromPopup(const QDate& date);
    void updateCursor();

    QDate m_selectedDate;
    QDate m_panelDate;
    QString m_displayFormat = QStringLiteral("yyyy-MM-dd");
    QString m_placeholderText = QStringLiteral("Select date");
    Ant::DatePickerSize m_pickerSize = Ant::DatePickerSize::Middle;
    Ant::DatePickerStatus m_status = Ant::DatePickerStatus::Normal;
    Ant::DatePickerVariant m_variant = Ant::DatePickerVariant::Outlined;
    bool m_allowClear = true;
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_open = false;
    QFrame* m_popup = nullptr;
};
