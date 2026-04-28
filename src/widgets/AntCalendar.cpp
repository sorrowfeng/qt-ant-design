#include "AntCalendar.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QTableView>
#include <QVBoxLayout>

#include "../styles/AntCalendarStyle.h"
#include "AntButton.h"
#include "core/AntTheme.h"

// ---- CalendarModel ----

class CalendarModel : public QAbstractTableModel
{
public:
    enum Roles { DateRole = Qt::UserRole + 1, IsTodayRole, IsAdjacentRole, IsDisabledRole };

    CalendarModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {}

    int rowCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : m_rowCount; }
    int columnCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : m_colCount; }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid()) return {};
        int i = index.row() * m_colCount + index.column();
        if (i < 0 || i >= m_items.size()) return {};

        const Item& item = m_items[i];
        if (role == Qt::DisplayRole) return item.text;
        if (role == DateRole) return item.date;
        if (role == IsTodayRole) return item.isToday;
        if (role == IsAdjacentRole) return item.isAdjacent;
        if (role == IsDisabledRole) return item.isDisabled;
        return {};
    }

    struct Item
    {
        QString text;
        QDate date;
        bool isToday = false;
        bool isAdjacent = false;
        bool isDisabled = false;
    };

    void buildDayMode(int year, int month,
                      const QDate& selected, const QDate& today,
                      const QDate& minDate, const QDate& maxDate)
    {
        beginResetModel();
        m_rowCount = 6; m_colCount = 7;
        m_items.clear();
        m_items.reserve(42);

        QDate firstOfMonth(year, month, 1);
        int startDow = firstOfMonth.dayOfWeek() % 7; // 0=Sun

        QDate cur = firstOfMonth.addDays(-startDow);
        for (int i = 0; i < 42; ++i)
        {
            Item item;
            item.text = QString::number(cur.day());
            item.date = cur;
            item.isToday = (cur == today);
            item.isAdjacent = (cur.month() != month);
            item.isDisabled = cur < minDate || cur > maxDate;
            m_items.append(item);
            cur = cur.addDays(1);
        }

        m_firstIndex = startDow; // index of day 1
        endResetModel();
    }

    void buildMonthMode(int year, const QDate& selected, const QDate& today)
    {
        beginResetModel();
        m_rowCount = 3; m_colCount = 4;
        m_items.clear();
        m_items.reserve(12);

        for (int m = 1; m <= 12; ++m)
        {
            QDate d(year, m, 15);
            Item item;
            item.text = QLocale(QLocale::English).monthName(m, QLocale::ShortFormat);
            item.date = d;
            item.isToday = (today.year() == year && today.month() == m);
            item.isAdjacent = false;
            item.isDisabled = false;
            m_items.append(item);
        }
        endResetModel();
    }

    void buildYearMode(int decadeStart, const QDate& selected, const QDate& today)
    {
        beginResetModel();
        m_rowCount = 3; m_colCount = 4;
        m_items.clear();
        m_items.reserve(12);

        for (int y = decadeStart; y < decadeStart + 12; ++y)
        {
            QDate d(y, 6, 15);
            Item item;
            item.text = QString::number(y);
            item.date = d;
            item.isToday = (today.year() == y);
            item.isAdjacent = false;
            item.isDisabled = false;
            m_items.append(item);
        }
        endResetModel();
    }

    QDate dateAt(int row, int col) const
    {
        int i = row * m_colCount + col;
        return i < m_items.size() ? m_items[i].date : QDate();
    }

    int firstDayIndex() const { return m_firstIndex; }

private:
    QList<Item> m_items;
    int m_rowCount = 0;
    int m_colCount = 0;
    int m_firstIndex = 0;
};

// ---- CalendarDelegate ----

class CalendarDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        const auto& token = antTheme->tokens();
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QString text = index.data(Qt::DisplayRole).toString();
        bool isToday = index.data(CalendarModel::IsTodayRole).toBool();
        bool isAdjacent = index.data(CalendarModel::IsAdjacentRole).toBool();
        bool isDisabled = index.data(CalendarModel::IsDisabledRole).toBool();
        bool isSelected = option.state.testFlag(QStyle::State_Selected);

        QRectF r = option.rect.adjusted(2, 2, -2, -2);
        const qreal dateBoxSize = qMin<qreal>(24.0, qMin(r.width(), r.height()));
        QRectF dateBox(r.center().x() - dateBoxSize / 2, r.center().y() - dateBoxSize / 2,
                       dateBoxSize, dateBoxSize);

        // Background fill for selected/today
        if (isSelected)
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(token.colorPrimary);
            painter->drawRoundedRect(dateBox, token.borderRadiusSM, token.borderRadiusSM);
        }
        else if (option.state.testFlag(QStyle::State_MouseOver))
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(token.colorFillQuaternary);
            painter->drawRoundedRect(dateBox, token.borderRadiusSM, token.borderRadiusSM);
        }

        // Today ring
        if (isToday && !isSelected)
        {
            painter->setPen(QPen(token.colorPrimary, token.lineWidth + 1));
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect(dateBox, token.borderRadiusSM, token.borderRadiusSM);
        }

        // Text
        QFont f = painter->font();
        f.setPixelSize(token.fontSizeSM);
        painter->setFont(f);

        QColor textColor = token.colorText;
        if (isSelected) textColor = token.colorTextLightSolid;
        if (isAdjacent && !isSelected) textColor = token.colorTextDisabled;
        if (isDisabled) textColor = token.colorTextDisabled;

        painter->setPen(textColor);
        painter->drawText(r, Qt::AlignCenter, text);

        painter->restore();
    }
};

// ---- AntCalendar ----

AntCalendar::AntCalendar(QWidget* parent)
    : QWidget(parent)
{
    auto* s = new AntCalendarStyle(style());
    s->setParent(this);
    setStyle(s);

    m_selectedDate = QDate::currentDate();
    m_navYear = m_selectedDate.year();
    m_navMonth = m_selectedDate.month();
    m_decadeStart = (m_navYear / 12) * 12;

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // Header
    m_header = new QWidget(this);
    auto* headerLayout = new QHBoxLayout(m_header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);

    m_yearBtn = new AntButton(m_header);
    m_yearBtn->setButtonSize(Ant::Size::Small);
    m_yearBtn->setFixedWidth(80);

    m_monthBtn = new AntButton(m_header);
    m_monthBtn->setButtonSize(Ant::Size::Small);
    m_monthBtn->setFixedWidth(72);

    m_monthModeBtn = new AntButton(QStringLiteral("Month"), m_header);
    m_monthModeBtn->setButtonSize(Ant::Size::Small);
    m_monthModeBtn->setFixedWidth(58);

    m_yearModeBtn = new AntButton(QStringLiteral("Year"), m_header);
    m_yearModeBtn->setButtonSize(Ant::Size::Small);
    m_yearModeBtn->setFixedWidth(48);

    headerLayout->addStretch();
    headerLayout->addWidget(m_yearBtn);
    headerLayout->addWidget(m_monthBtn);
    headerLayout->addWidget(m_monthModeBtn);
    headerLayout->addWidget(m_yearModeBtn);
    mainLayout->addWidget(m_header);

    // Weekday labels
    m_weekdayRow = new QWidget(this);
    auto* weekdayLayout = new QHBoxLayout(m_weekdayRow);
    weekdayLayout->setContentsMargins(0, 0, 0, 0);
    weekdayLayout->setSpacing(0);
    const QStringList dayNames = {QStringLiteral("Su"), QStringLiteral("Mo"), QStringLiteral("Tu"),
                                  QStringLiteral("We"), QStringLiteral("Th"), QStringLiteral("Fr"), QStringLiteral("Sa")};
    for (const auto& name : dayNames)
    {
        auto* label = new QLabel(name, m_weekdayRow);
        label->setAlignment(Qt::AlignCenter);
        label->setFixedHeight(24);
        QFont f = label->font();
        f.setPixelSize(antTheme->tokens().fontSizeSM);
        label->setFont(f);
        QPalette lp = label->palette();
        lp.setColor(QPalette::WindowText, antTheme->tokens().colorTextSecondary);
        label->setPalette(lp);
        weekdayLayout->addWidget(label);
    }
    mainLayout->addWidget(m_weekdayRow);

    // Table view for day/month/year grid
    m_model = new CalendarModel(this);
    m_delegate = new CalendarDelegate(this);
    m_view = new QTableView(this);
    m_view->setModel(m_model);
    m_view->setItemDelegate(m_delegate);
    m_view->horizontalHeader()->hide();
    m_view->verticalHeader()->hide();
    m_view->setShowGrid(false);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_view->setMouseTracking(true);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setFocusPolicy(Qt::NoFocus);
    m_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_view->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    mainLayout->addWidget(m_view, 1);

    setMinimumSize(312, 340);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        if (m_weekdayRow)
        {
            const auto& token = antTheme->tokens();
            for (auto* child : m_weekdayRow->children())
            {
                if (auto* label = qobject_cast<QLabel*>(child))
                {
                    QPalette lp = label->palette();
                    lp.setColor(QPalette::WindowText, token.colorTextSecondary);
                    label->setPalette(lp);
                }
            }
        }
        update();
    });

    connect(qobject_cast<QPushButton*>(m_yearBtn), &QPushButton::clicked, this, [this]() {
        setCalendarMode(Ant::CalendarMode::Year);
    });
    connect(qobject_cast<QPushButton*>(m_monthBtn), &QPushButton::clicked, this, [this]() {
        setCalendarMode(Ant::CalendarMode::Month);
    });
    connect(qobject_cast<QPushButton*>(m_monthModeBtn), &QPushButton::clicked, this, [this]() {
        setCalendarMode(Ant::CalendarMode::Day);
    });
    connect(qobject_cast<QPushButton*>(m_yearModeBtn), &QPushButton::clicked, this, [this]() {
        setCalendarMode(Ant::CalendarMode::Year);
    });

    // View click handler
    connect(m_view, &QTableView::clicked, this, [this](const QModelIndex& idx) {
        switch (m_mode)
        {
        case Ant::CalendarMode::Day:
            handleDayClick(idx.row() * m_model->columnCount() + idx.column());
            break;
        case Ant::CalendarMode::Month:
            handleMonthClick(idx.row() * m_model->columnCount() + idx.column());
            break;
        case Ant::CalendarMode::Year:
            handleYearClick(idx.row() * m_model->columnCount() + idx.column());
            break;
        }
    });

    rebuildModel();
}

QDate AntCalendar::selectedDate() const { return m_selectedDate; }

void AntCalendar::setSelectedDate(QDate date)
{
    if (!date.isValid()) return;
    if (m_minimumDate.isValid() && date < m_minimumDate) date = m_minimumDate;
    if (m_maximumDate.isValid() && date > m_maximumDate) date = m_maximumDate;
    if (m_selectedDate == date) return;
    m_selectedDate = date;
    if (m_mode == Ant::CalendarMode::Day)
    {
        m_navYear = date.year();
        m_navMonth = date.month();
    }
    rebuildModel();
    Q_EMIT selectedDateChanged(m_selectedDate);
}

QDate AntCalendar::minimumDate() const { return m_minimumDate; }
void AntCalendar::setMinimumDate(QDate date)
{
    if (m_minimumDate == date) return;
    m_minimumDate = date;
    if (m_selectedDate < m_minimumDate) setSelectedDate(m_minimumDate);
    rebuildModel();
    Q_EMIT minimumDateChanged(m_minimumDate);
}

QDate AntCalendar::maximumDate() const { return m_maximumDate; }
void AntCalendar::setMaximumDate(QDate date)
{
    if (m_maximumDate == date) return;
    m_maximumDate = date;
    if (m_selectedDate > m_maximumDate) setSelectedDate(m_maximumDate);
    rebuildModel();
    Q_EMIT maximumDateChanged(m_maximumDate);
}

Ant::CalendarMode AntCalendar::calendarMode() const { return m_mode; }

void AntCalendar::setCalendarMode(Ant::CalendarMode mode)
{
    if (m_mode == mode) return;
    m_mode = mode;

    // Adjust nav state for the new mode
    if (mode == Ant::CalendarMode::Day) { m_navYear = m_selectedDate.year(); m_navMonth = m_selectedDate.month(); }
    else if (mode == Ant::CalendarMode::Month) { m_navYear = m_selectedDate.year(); }
    else if (mode == Ant::CalendarMode::Year) { m_decadeStart = (m_selectedDate.year() / 12) * 12; }

    m_weekdayRow->setVisible(mode == Ant::CalendarMode::Day);

    rebuildModel();
    Q_EMIT calendarModeChanged(m_mode);
}

QSize AntCalendar::sizeHint() const { return QSize(312, 340); }

void AntCalendar::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateViewMetrics();
}

void AntCalendar::rebuildModel()
{
    auto* calModel = static_cast<CalendarModel*>(m_model);
    QDate today = QDate::currentDate();

    switch (m_mode)
    {
    case Ant::CalendarMode::Day:
    {
        int cols = 7;
        calModel->buildDayMode(m_navYear, m_navMonth, m_selectedDate, today, m_minimumDate, m_maximumDate);
        int selRow = -1, selCol = -1;
        if (m_selectedDate.year() == m_navYear && m_selectedDate.month() == m_navMonth)
        {
            int idx = m_selectedDate.day() - 1 + calModel->firstDayIndex();
            selRow = idx / cols;
            selCol = idx % cols;
        }
        if (selRow >= 0)
        {
            auto idx = m_model->index(selRow, selCol);
            m_view->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
        }
        break;
    }
    case Ant::CalendarMode::Month:
        calModel->buildMonthMode(m_navYear, m_selectedDate, today);
        m_view->clearSelection();
        break;
    case Ant::CalendarMode::Year:
        calModel->buildYearMode(m_decadeStart, m_selectedDate, today);
        m_view->clearSelection();
        break;
    }

    updateViewMetrics();
    updateHeaderText();
}

void AntCalendar::updateHeaderText()
{
    const QString yearText = QStringLiteral("%1  v").arg(m_navYear);
    const QString monthText = QStringLiteral("%1  v").arg(QLocale(QLocale::English).monthName(m_navMonth, QLocale::ShortFormat));

    if (m_yearBtn)
    {
        m_yearBtn->setText(yearText);
    }
    if (m_monthBtn)
    {
        m_monthBtn->setText(monthText);
    }

    const bool monthModeActive = m_mode != Ant::CalendarMode::Year;
    m_monthModeBtn->setButtonType(monthModeActive ? Ant::ButtonType::Primary : Ant::ButtonType::Default);
    m_monthModeBtn->setGhost(monthModeActive);
    m_yearModeBtn->setButtonType(monthModeActive ? Ant::ButtonType::Default : Ant::ButtonType::Primary);
    m_yearModeBtn->setGhost(!monthModeActive);
}

void AntCalendar::updateViewMetrics()
{
    if (!m_model || !m_view)
    {
        return;
    }

    const int cols = m_model->columnCount();
    const int rows = m_model->rowCount();
    if (cols <= 0 || rows <= 0)
    {
        return;
    }

    const int rowHeight = (m_mode == Ant::CalendarMode::Day) ? 40 : 64;
    for (int r = 0; r < rows; ++r)
    {
        m_view->setRowHeight(r, rowHeight);
    }
}

void AntCalendar::handleDayClick(int dayIndex)
{
    auto* calModel = static_cast<CalendarModel*>(m_model);
    int row = dayIndex / calModel->columnCount();
    int col = dayIndex % calModel->columnCount();
    QDate date = calModel->dateAt(row, col);
    if (!date.isValid()) return;
    if (m_minimumDate.isValid() && date < m_minimumDate) return;
    if (m_maximumDate.isValid() && date > m_maximumDate) return;
    m_selectedDate = date;
    m_navYear = date.year();
    m_navMonth = date.month();
    rebuildModel();
    Q_EMIT clicked(date);
    Q_EMIT selectedDateChanged(date);
}

void AntCalendar::handleMonthClick(int monthIndex)
{
    // monthIndex 0-11 -> month 1-12
    int month = monthIndex + 1;
    m_navMonth = month;
    setCalendarMode(Ant::CalendarMode::Day);
}

void AntCalendar::handleYearClick(int yearIndex)
{
    int year = m_decadeStart + yearIndex;
    m_navYear = year;
    setCalendarMode(Ant::CalendarMode::Month);
}
