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
    Q_PROPERTY(QDate date READ date WRITE setDate NOTIFY dateChanged)
    Q_PROPERTY(QDate selectedDate READ selectedDate WRITE setSelectedDate NOTIFY selectedDateChanged)
    Q_PROPERTY(QDate minimumDate READ minimumDate WRITE setMinimumDate NOTIFY minimumDateChanged)
    Q_PROPERTY(QDate maximumDate READ maximumDate WRITE setMaximumDate NOTIFY maximumDateChanged)
    Q_PROPERTY(QString displayFormat READ displayFormat WRITE setDisplayFormat NOTIFY displayFormatChanged)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
    Q_PROPERTY(Ant::Size pickerSize READ pickerSize WRITE setPickerSize NOTIFY pickerSizeChanged)
    Q_PROPERTY(Ant::Status status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(Ant::Variant variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(bool allowClear READ allowClear WRITE setAllowClear NOTIFY allowClearChanged)
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)

public:
    explicit AntDatePicker(QWidget* parent = nullptr);
    ~AntDatePicker() override;

    QDate selectedDate() const;
    void setSelectedDate(const QDate& date);
    QDate date() const;
    void setDate(const QDate& date);
    QDate minimumDate() const;
    void setMinimumDate(const QDate& date);
    QDate maximumDate() const;
    void setMaximumDate(const QDate& date);
    void setDateRange(const QDate& minDate, const QDate& maxDate);
    void clearMinimumDate();
    void clearMaximumDate();
    bool hasSelectedDate() const;
    void clear();

    QString displayFormat() const;
    void setDisplayFormat(const QString& format);

    QString placeholderText() const;
    void setPlaceholderText(const QString& text);

    Ant::Size pickerSize() const;
    void setPickerSize(Ant::Size size);

    Ant::Status status() const;
    void setStatus(Ant::Status status);

    Ant::Variant variant() const;
    void setVariant(Ant::Variant variant);

    bool allowClear() const;
    void setAllowClear(bool allowClear);

    bool isRangeMode() const;
    void setRangeMode(bool rangeMode);

    QDate startDate() const;
    void setStartDate(const QDate& date);

    QDate endDate() const;
    void setEndDate(const QDate& date);

    bool isOpen() const;
    void setOpen(bool open);

    QString dateString() const;
    bool isHoveredState() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void selectedDateChanged(const QDate& date);
    void dateChanged(const QDate& date);
    void minimumDateChanged(const QDate& date);
    void maximumDateChanged(const QDate& date);
    void dateRangeChanged(const QDate& minDate, const QDate& maxDate);
    void dateStringChanged(const QString& text);
    void displayFormatChanged(const QString& format);
    void placeholderTextChanged(const QString& text);
    void pickerSizeChanged(Ant::Size size);
    void statusChanged(Ant::Status status);
    void variantChanged(Ant::Variant variant);
    void allowClearChanged(bool allowClear);
    void rangeModeChanged(bool rangeMode);
    void startDateChanged(const QDate& date);
    void endDateChanged(const QDate& date);
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
    QDate boundedDate(const QDate& date) const;
    bool isDateInRange(const QDate& date) const;

    QDate m_selectedDate;
    QDate m_panelDate;
    QDate m_startDate;
    QDate m_endDate;
    QDate m_minimumDate = QDate(100, 1, 1);
    QDate m_maximumDate = QDate(9999, 12, 31);
    QString m_displayFormat = QStringLiteral("yyyy-MM-dd");
    QString m_placeholderText = QStringLiteral("Select date");
    Ant::Size m_pickerSize = Ant::Size::Middle;
    Ant::Status m_status = Ant::Status::Normal;
    Ant::Variant m_variant = Ant::Variant::Outlined;
    bool m_allowClear = true;
    bool m_rangeMode = false;
    bool m_pickingEnd = false;
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_open = false;
    QFrame* m_popup = nullptr;
};
