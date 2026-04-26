#include "AntCalendar.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
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
            item.text = QLocale().monthName(m, QLocale::ShortFormat);
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
        const qreal cellSz = std::min(r.width(), r.height());
        QRectF circle(r.center().x() - cellSz / 2, r.center().y() - cellSz / 2, cellSz, cellSz);

        // Background fill for selected/today
        if (isSelected)
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(token.colorPrimary);
            painter->drawEllipse(circle);
        }
        else if (option.state.testFlag(QStyle::State_MouseOver))
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(token.colorFillQuaternary);
            painter->drawEllipse(circle);
        }

        // Today ring
        if (isToday && !isSelected)
        {
            painter->setPen(QPen(token.colorPrimary, token.lineWidth + 1));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(circle);
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
    mainLayout->setSpacing(4);

    // Header
    m_header = new QWidget(this);
    auto* headerLayout = new QHBoxLayout(m_header);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    m_prevBtn = new AntButton(QStringLiteral("<"), m_header);
    m_prevBtn->setButtonType(Ant::ButtonType::Text);
    m_prevBtn->setButtonSize(Ant::Size::Small);
    m_prevBtn->setFixedSize(28, 28);

    m_titleBtn = new AntButton(m_header);
    m_titleBtn->setButtonType(Ant::ButtonType::Text);
    m_titleBtn->setFixedHeight(28);

    m_nextBtn = new AntButton(QStringLiteral(">"), m_header);
    m_nextBtn->setButtonType(Ant::ButtonType::Text);
    m_nextBtn->setButtonSize(Ant::Size::Small);
    m_nextBtn->setFixedSize(28, 28);

    headerLayout->addWidget(m_prevBtn);
    headerLayout->addWidget(m_titleBtn, 1);
    headerLayout->addWidget(m_nextBtn);
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
        label->setStyleSheet(QStringLiteral("color:%1").arg(antTheme->tokens().colorTextSecondary.name()));
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
    mainLayout->addWidget(m_view, 1);

    setFixedSize(312, 340);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        if (m_weekdayRow)
        {
            const auto& token = antTheme->tokens();
            for (auto* child : m_weekdayRow->children())
            {
                if (auto* label = qobject_cast<QLabel*>(child))
                    label->setStyleSheet(QStringLiteral("color:%1").arg(token.colorTextSecondary.name()));
            }
        }
        update();
    });

    // Header button connections
    connect(qobject_cast<QPushButton*>(m_prevBtn), &QPushButton::clicked, this, [this]() {
        switch (m_mode)
        {
        case Ant::CalendarMode::Day:
            if (m_navMonth == 1) { --m_navYear; m_navMonth = 12; }
            else --m_navMonth;
            break;
        case Ant::CalendarMode::Month:
            --m_navYear;
            break;
        case Ant::CalendarMode::Year:
            m_decadeStart -= 12;
            break;
        }
        rebuildModel();
    });

    connect(qobject_cast<QPushButton*>(m_nextBtn), &QPushButton::clicked, this, [this]() {
        switch (m_mode)
        {
        case Ant::CalendarMode::Day:
            if (m_navMonth == 12) { ++m_navYear; m_navMonth = 1; }
            else ++m_navMonth;
            break;
        case Ant::CalendarMode::Month:
            ++m_navYear;
            break;
        case Ant::CalendarMode::Year:
            m_decadeStart += 12;
            break;
        }
        rebuildModel();
    });

    connect(qobject_cast<QPushButton*>(m_titleBtn), &QPushButton::clicked, this, [this]() {
        switch (m_mode)
        {
        case Ant::CalendarMode::Day:
            setCalendarMode(Ant::CalendarMode::Month);
            break;
        case Ant::CalendarMode::Month:
            m_decadeStart = (m_navYear / 12) * 12;
            setCalendarMode(Ant::CalendarMode::Year);
            break;
        case Ant::CalendarMode::Year:
            // Select year -> go back to Month mode
            break;
        }
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
        for (int c = 0; c < cols; ++c) m_view->setColumnWidth(c, 40);
        for (int r = 0; r < 6; ++r) m_view->setRowHeight(r, 40);

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
        for (int c = 0; c < 4; ++c) m_view->setColumnWidth(c, 70);
        for (int r = 0; r < 3; ++r) m_view->setRowHeight(r, 72);
        m_view->clearSelection();
        break;
    case Ant::CalendarMode::Year:
        calModel->buildYearMode(m_decadeStart, m_selectedDate, today);
        for (int c = 0; c < 4; ++c) m_view->setColumnWidth(c, 70);
        for (int r = 0; r < 3; ++r) m_view->setRowHeight(r, 72);
        m_view->clearSelection();
        break;
    }

    updateHeaderText();
}

void AntCalendar::updateHeaderText()
{
    QString title;
    switch (m_mode)
    {
    case Ant::CalendarMode::Day:
        title = QStringLiteral("%1 %2").arg(QLocale().monthName(m_navMonth), QString::number(m_navYear));
        break;
    case Ant::CalendarMode::Month:
        title = QString::number(m_navYear);
        break;
    case Ant::CalendarMode::Year:
        title = QStringLiteral("%1 - %2").arg(QString::number(m_decadeStart), QString::number(m_decadeStart + 11));
        break;
    }
    if (auto* btn = qobject_cast<AntButton*>(m_titleBtn))
        btn->setText(title);
    else
        qobject_cast<QPushButton*>(m_titleBtn)->setText(title);
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
