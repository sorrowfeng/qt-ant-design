#pragma once

#include "core/QtAntDesignExport.h"

#include <QWidget>
#include <QDate>

#include "core/AntTypes.h"

class QTableView;
class QAbstractTableModel;
class QStyledItemDelegate;
class QResizeEvent;
class QModelIndex;

class QT_ANT_DESIGN_EXPORT AntCalendar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QDate selectedDate READ selectedDate WRITE setSelectedDate NOTIFY selectedDateChanged)
    Q_PROPERTY(QDate minimumDate READ minimumDate WRITE setMinimumDate NOTIFY minimumDateChanged)
    Q_PROPERTY(QDate maximumDate READ maximumDate WRITE setMaximumDate NOTIFY maximumDateChanged)
    Q_PROPERTY(Ant::CalendarMode calendarMode READ calendarMode WRITE setCalendarMode NOTIFY calendarModeChanged)

public:
    explicit AntCalendar(QWidget* parent = nullptr);

    QDate selectedDate() const;
    void setSelectedDate(QDate date);

    QDate minimumDate() const;
    void setMinimumDate(QDate date);

    QDate maximumDate() const;
    void setMaximumDate(QDate date);

    Ant::CalendarMode calendarMode() const;
    void setCalendarMode(Ant::CalendarMode mode);

    QSize sizeHint() const override;

Q_SIGNALS:
    void clicked(QDate date);
    void selectedDateChanged(QDate date);
    void minimumDateChanged(QDate date);
    void maximumDateChanged(QDate date);
    void calendarModeChanged(Ant::CalendarMode mode);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void rebuildModel();
    void updateHeaderText();
    void updateViewMetrics();
    void applyThemePalette();
    QModelIndex modelIndexForDate(const QDate& date) const;
    void updateSelectedDateInView(const QDate& previousDate = QDate());
    void invalidateViewMetricsCache();
    void requestCalendarUpdate(const QRect& region, const QString& mode, bool selectionScoped = false);
    void syncCalendarPerfCounters() const;
    void navigateToMonth(int year, int month);
    void handleDayClick(int dayIndex);
    void handleMonthClick(int monthIndex);
    void handleYearClick(int yearIndex);

    QTableView* m_view = nullptr;
    QAbstractTableModel* m_model = nullptr;
    QStyledItemDelegate* m_delegate = nullptr;
    QWidget* m_header = nullptr;
    class AntButton* m_yearBtn = nullptr;
    class AntButton* m_monthBtn = nullptr;
    class AntButton* m_monthModeBtn = nullptr;
    class AntButton* m_yearModeBtn = nullptr;
    QWidget* m_weekdayRow = nullptr;

    Ant::CalendarMode m_mode = Ant::CalendarMode::Day;
    QDate m_selectedDate;
    QDate m_minimumDate = QDate(1924, 1, 1);
    QDate m_maximumDate = QDate(2124, 12, 31);
    int m_navYear = 0;
    int m_navMonth = 0;
    int m_decadeStart = 0;
    bool m_viewMetricsCacheValid = false;
    int m_viewMetricsRows = 0;
    int m_viewMetricsRowHeight = 0;
    QString m_lastYearHeaderText;
    QString m_lastMonthHeaderText;
    bool m_headerModeValid = false;
    bool m_lastMonthModeActive = false;
    int m_modelBuildCount = 0;
    int m_selectionRegionUpdateCount = 0;
    int m_viewMetricsBuildCount = 0;
    int m_viewMetricsCacheHitCount = 0;
    int m_regionUpdateCount = 0;
    QString m_lastUpdateMode;
};
