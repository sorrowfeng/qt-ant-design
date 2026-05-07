#pragma once

#include "core/QtAntDesignExport.h"

#include <QTime>
#include <QWidget>

#include "core/AntTypes.h"

class QEnterEvent;
class QEvent;
class QFocusEvent;
class QFrame;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;

class QT_ANT_DESIGN_EXPORT AntTimePicker : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QTime time READ time WRITE setTime NOTIFY timeChanged)
    Q_PROPERTY(QTime selectedTime READ selectedTime WRITE setSelectedTime NOTIFY selectedTimeChanged)
    Q_PROPERTY(QTime minimumTime READ minimumTime WRITE setMinimumTime NOTIFY minimumTimeChanged)
    Q_PROPERTY(QTime maximumTime READ maximumTime WRITE setMaximumTime NOTIFY maximumTimeChanged)
    Q_PROPERTY(QString displayFormat READ displayFormat WRITE setDisplayFormat NOTIFY displayFormatChanged)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
    Q_PROPERTY(Ant::Size pickerSize READ pickerSize WRITE setPickerSize NOTIFY pickerSizeChanged)
    Q_PROPERTY(Ant::Status status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(Ant::Variant variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(bool allowClear READ allowClear WRITE setAllowClear NOTIFY allowClearChanged)
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)
    Q_PROPERTY(int hourStep READ hourStep WRITE setHourStep NOTIFY hourStepChanged)
    Q_PROPERTY(int minuteStep READ minuteStep WRITE setMinuteStep NOTIFY minuteStepChanged)
    Q_PROPERTY(int secondStep READ secondStep WRITE setSecondStep NOTIFY secondStepChanged)
    Q_PROPERTY(bool showNow READ showNow WRITE setShowNow NOTIFY showNowChanged)

public:
    explicit AntTimePicker(QWidget* parent = nullptr);
    ~AntTimePicker() override;

    QTime selectedTime() const;
    void setSelectedTime(const QTime& time);
    QTime time() const;
    void setTime(const QTime& time);
    QTime minimumTime() const;
    void setMinimumTime(const QTime& time);
    QTime maximumTime() const;
    void setMaximumTime(const QTime& time);
    void setTimeRange(const QTime& minTime, const QTime& maxTime);
    void clearMinimumTime();
    void clearMaximumTime();
    bool hasSelectedTime() const;
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

    QTime startTime() const;
    void setStartTime(const QTime& time);

    QTime endTime() const;
    void setEndTime(const QTime& time);

    bool isOpen() const;
    void setOpen(bool open);

    int hourStep() const;
    void setHourStep(int step);
    int minuteStep() const;
    void setMinuteStep(int step);
    int secondStep() const;
    void setSecondStep(int step);

    bool showNow() const;
    void setShowNow(bool show);

    QString timeString() const;
    bool isHoveredState() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void selectedTimeChanged(const QTime& time);
    void timeChanged(const QTime& time);
    void minimumTimeChanged(const QTime& time);
    void maximumTimeChanged(const QTime& time);
    void timeRangeChanged(const QTime& minTime, const QTime& maxTime);
    void timeStringChanged(const QString& text);
    void displayFormatChanged(const QString& format);
    void placeholderTextChanged(const QString& text);
    void pickerSizeChanged(Ant::Size size);
    void statusChanged(Ant::Status status);
    void variantChanged(Ant::Variant variant);
    void allowClearChanged(bool allowClear);
    void rangeModeChanged(bool rangeMode);
    void startTimeChanged(const QTime& time);
    void endTimeChanged(const QTime& time);
    void openChanged(bool open);
    void hourStepChanged(int step);
    void minuteStepChanged(int step);
    void secondStepChanged(int step);
    void showNowChanged(bool show);
    void cleared();
    void accepted(const QTime& time);

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
    friend class AntTimePickerPopup;

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
    void setPanelTime(const QTime& time);
    void acceptPanelTime();
    void updateCursor();
    int normalizeStep(int step, int maximum) const;
    QTime boundedTime(const QTime& time) const;

    QTime m_selectedTime;
    QTime m_panelTime;
    QTime m_startTime;
    QTime m_endTime;
    QTime m_minimumTime = QTime(0, 0, 0, 0);
    QTime m_maximumTime = QTime(23, 59, 59, 999);
    QString m_displayFormat = QStringLiteral("HH:mm:ss");
    QString m_placeholderText = QStringLiteral("Select time");
    Ant::Size m_pickerSize = Ant::Size::Middle;
    Ant::Status m_status = Ant::Status::Normal;
    Ant::Variant m_variant = Ant::Variant::Outlined;
    bool m_allowClear = true;
    bool m_rangeMode = false;
    bool m_pickingEnd = false;
    bool m_hovered = false;
    bool m_open = false;
    bool m_showNow = true;
    int m_hourStep = 1;
    int m_minuteStep = 1;
    int m_secondStep = 1;
    QFrame* m_popup = nullptr;
};
