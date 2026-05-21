#include "AntTimePicker.h"

#include <QFocusEvent>
#include <QFrame>
#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QSizePolicy>
#include <QStringList>
#include <QVector>
#include <QWheelEvent>

#include <algorithm>

#include "../styles/AntTimePickerStyle.h"
#include "core/AntPopupMotion.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"

namespace
{
constexpr int kTimePickerPopupShadowMargin = 32;
constexpr int kTimePickerPopupTopMargin = 12;
constexpr int kTimePickerPopupPanelWidth = 168;
constexpr int kTimePickerPopupPanelHeight = 274;
}

class AntTimePickerPopup : public QFrame
{
public:
    explicit AntTimePickerPopup(AntTimePicker* owner)
        : QFrame(owner, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
          m_owner(owner)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setObjectName(QStringLiteral("AntTimePickerPopup"));
        setMouseTracking(true);
        resize(kTimePickerPopupPanelWidth + kTimePickerPopupShadowMargin * 2,
               kTimePickerPopupPanelHeight + kTimePickerPopupTopMargin + kTimePickerPopupShadowMargin);
        syncPopupPerfCounters();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        if (!m_owner)
        {
            return;
        }

        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

        const QRectF panel = panelRect();
        antTheme->drawEffectShadow(&painter, panel.toAlignedRect(), 12, token.borderRadiusLG, 0.55);
        painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter.setBrush(token.colorBgElevated);
        painter.drawRoundedRect(panel, token.borderRadiusLG, token.borderRadiusLG);

        drawColumns(painter, panel);
        drawFooter(painter, panel);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        const int oldColumn = m_hoveredColumn;
        const int oldRow = m_hoveredRow;
        m_hoveredColumn = columnAt(event->position());
        m_hoveredRow = rowAt(event->position());
        const bool clickable = (m_hoveredColumn >= 0 && m_hoveredRow >= 0)
            || nowRect().contains(event->position())
            || okRect().contains(event->position());
        if (m_cursorClickable != clickable)
        {
            m_cursorClickable = clickable;
            setCursor(clickable ? Qt::PointingHandCursor : Qt::ArrowCursor);
        }
        if (oldColumn != m_hoveredColumn || oldRow != m_hoveredRow)
        {
            ++m_scopedRowUpdateCount;
            updatePopupRegion(rowDirtyRect(oldColumn, oldRow).united(rowDirtyRect(m_hoveredColumn, m_hoveredRow)),
                              QStringLiteral("hover"));
        }
        QFrame::mouseMoveEvent(event);
    }

    void leaveEvent(QEvent* event) override
    {
        const int oldColumn = m_hoveredColumn;
        const int oldRow = m_hoveredRow;
        m_hoveredColumn = -1;
        m_hoveredRow = -1;
        if (m_cursorClickable)
        {
            m_cursorClickable = false;
            setCursor(Qt::ArrowCursor);
        }
        if (oldColumn >= 0 && oldRow >= 0)
        {
            ++m_scopedRowUpdateCount;
            updatePopupRegion(rowDirtyRect(oldColumn, oldRow), QStringLiteral("hover"));
        }
        QFrame::leaveEvent(event);
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (!m_owner || event->button() != Qt::LeftButton)
        {
            QFrame::mousePressEvent(event);
            return;
        }

        if (m_owner->m_showNow && nowRect().contains(event->position()))
        {
            m_owner->setPanelTime(QTime::currentTime());
            m_owner->setSelectedTime(m_owner->m_panelTime);
            event->accept();
            return;
        }
        if (okRect().contains(event->position()))
        {
            m_owner->acceptPanelTime();
            event->accept();
            return;
        }

        const int col = columnAt(event->position());
        const int row = rowAt(event->position());
        if (col >= 0 && row >= 0)
        {
            QTime next = m_owner->m_panelTime;
            if (!next.isValid())
            {
                next = QTime::currentTime();
            }
            const int value = valueForRow(col, row);
            if (col == 0)
            {
                next.setHMS(value, next.minute(), next.second());
            }
            else if (col == 1)
            {
                next.setHMS(next.hour(), value, next.second());
            }
            else
            {
                next.setHMS(next.hour(), next.minute(), value);
            }
            m_owner->setPanelTime(next);
            m_owner->setSelectedTime(next);
            event->accept();
            return;
        }

        QFrame::mousePressEvent(event);
    }

    void wheelEvent(QWheelEvent* event) override
    {
        const int col = columnAt(event->position());
        if (!m_owner || col < 0)
        {
            QFrame::wheelEvent(event);
            return;
        }

        QTime next = m_owner->m_panelTime.isValid() ? m_owner->m_panelTime : QTime::currentTime();
        const int direction = event->angleDelta().y() > 0 ? -1 : 1;
        if (col == 0)
        {
            next = next.addSecs(direction * m_owner->m_hourStep * 3600);
        }
        else if (col == 1)
        {
            next = next.addSecs(direction * m_owner->m_minuteStep * 60);
        }
        else
        {
            next = next.addSecs(direction * m_owner->m_secondStep);
        }
        m_owner->setPanelTime(next);
        m_owner->setSelectedTime(next);
        event->accept();
    }

    void hideEvent(QHideEvent* event) override
    {
        if (m_owner && m_owner->isOpen())
        {
            m_owner->setOpen(false);
        }
        QFrame::hideEvent(event);
    }

private:
    struct PopupLayoutCache
    {
        QSize widgetSize;
        QRectF panelRect;
        QRectF columnsRect;
        QRectF footerRect;
        QRectF nowRect;
        QRectF okRect;
        QVector<QRectF> columnRects;
        QVector<QVector<QRectF>> rowRects;
        bool valid = false;
    };

    QRectF panelRect() const
    {
        return layoutCache().panelRect;
    }

    QRectF columnsRect() const
    {
        return layoutCache().columnsRect;
    }

    QRectF footerRect() const
    {
        return layoutCache().footerRect;
    }

    QRectF nowRect() const
    {
        return layoutCache().nowRect;
    }

    QRectF okRect() const
    {
        return layoutCache().okRect;
    }

    int columnAt(const QPointF& pos) const
    {
        const auto& cache = layoutCache();
        if (!cache.columnsRect.contains(pos))
        {
            return -1;
        }
        for (int column = 0; column < cache.columnRects.size(); ++column)
        {
            if (cache.columnRects.at(column).contains(pos))
            {
                return column;
            }
        }
        return -1;
    }

    int rowAt(const QPointF& pos) const
    {
        const int column = columnAt(pos);
        if (column < 0)
        {
            return -1;
        }
        const auto& rows = layoutCache().rowRects.at(column);
        for (int row = 0; row < rows.size(); ++row)
        {
            if (rows.at(row).contains(pos))
            {
                return row;
            }
        }
        return -1;
    }

    int valueForRow(int column, int row) const
    {
        const QTime time = m_owner->m_panelTime.isValid() ? m_owner->m_panelTime : QTime::currentTime();
        const int center = column == 0 ? time.hour() : (column == 1 ? time.minute() : time.second());
        const int step = column == 0 ? m_owner->m_hourStep : (column == 1 ? m_owner->m_minuteStep : m_owner->m_secondStep);
        const int max = column == 0 ? 24 : 60;
        int value = center + row * step;
        while (value < 0)
        {
            value += max;
        }
        return value % max;
    }

    void drawColumns(QPainter& painter, const QRectF& panel) const
    {
        Q_UNUSED(panel)
        const auto& token = antTheme->tokens();
        const auto& cache = layoutCache();

        QFont valueFont = painter.font();
        valueFont.setPixelSize(token.fontSize);
        painter.setFont(valueFont);

        for (int c = 0; c < 3; ++c)
        {
            const QRectF colRect = cache.columnRects.at(c);
            if (c > 0)
            {
                painter.setPen(QPen(token.colorSplit, token.lineWidth));
                painter.drawLine(QPointF(colRect.left(), colRect.top()), QPointF(colRect.left(), colRect.bottom()));
            }

            for (int r = 0; r < 8; ++r)
            {
                const QRectF rowRect = cache.rowRects.at(c).at(r);
                const bool selected = r == 0;
                const bool hovered = c == m_hoveredColumn && r == m_hoveredRow;
                if (selected)
                {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(AntPalette::alpha(token.colorPrimary, antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.26 : 0.12));
                    painter.drawRoundedRect(rowRect, token.borderRadiusSM, token.borderRadiusSM);
                }
                else if (hovered)
                {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(token.colorFillQuaternary);
                    painter.drawRoundedRect(rowRect, token.borderRadiusSM, token.borderRadiusSM);
                }

                painter.setPen(selected ? token.colorPrimary : token.colorText);
                painter.drawText(rowRect, Qt::AlignCenter, QStringLiteral("%1").arg(valueForRow(c, r), 2, 10, QLatin1Char('0')));
            }
        }
    }

    void drawFooter(QPainter& painter, const QRectF& panel) const
    {
        const auto& token = antTheme->tokens();
        const QRectF footer = footerRect();
        painter.setPen(QPen(token.colorSplit, token.lineWidth));
        painter.drawLine(QPointF(panel.left(), footer.top()), QPointF(panel.right(), footer.top()));

        QFont font = painter.font();
        font.setPixelSize(token.fontSize);
        font.setWeight(QFont::DemiBold);
        painter.setFont(font);

        if (m_owner->m_showNow)
        {
            painter.setPen(token.colorPrimary);
            painter.drawText(nowRect(), Qt::AlignCenter, QStringLiteral("Now"));
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(token.colorPrimary);
        painter.drawRoundedRect(okRect(), token.borderRadiusSM, token.borderRadiusSM);
        painter.setPen(token.colorTextLightSolid);
        painter.drawText(okRect(), Qt::AlignCenter, QStringLiteral("OK"));
    }

    const PopupLayoutCache& layoutCache() const
    {
        if (m_layoutCache.valid && m_layoutCache.widgetSize == size())
        {
            ++m_layoutCacheHitCount;
            syncPopupPerfCounters();
            return m_layoutCache;
        }

        m_layoutCache.widgetSize = size();
        m_layoutCache.panelRect = rect().adjusted(kTimePickerPopupShadowMargin,
                                                  kTimePickerPopupTopMargin,
                                                  -kTimePickerPopupShadowMargin,
                                                  -kTimePickerPopupShadowMargin);
        m_layoutCache.columnsRect = QRectF(m_layoutCache.panelRect.left(),
                                           m_layoutCache.panelRect.top() + 8,
                                           m_layoutCache.panelRect.width(),
                                           8 * 28);
        m_layoutCache.footerRect = QRectF(m_layoutCache.panelRect.left(),
                                          m_layoutCache.panelRect.bottom() - 48,
                                          m_layoutCache.panelRect.width(),
                                          48);
        m_layoutCache.nowRect = QRectF(m_layoutCache.footerRect.left() + 12,
                                       m_layoutCache.footerRect.top() + 8,
                                       64,
                                       30);
        m_layoutCache.okRect = QRectF(m_layoutCache.footerRect.right() - 70,
                                      m_layoutCache.footerRect.top() + 8,
                                      56,
                                      30);
        m_layoutCache.columnRects.clear();
        m_layoutCache.rowRects.clear();
        m_layoutCache.columnRects.reserve(3);
        m_layoutCache.rowRects.reserve(3);

        const qreal columnWidth = m_layoutCache.columnsRect.width() / 3.0;
        for (int column = 0; column < 3; ++column)
        {
            const QRectF columnRect(m_layoutCache.columnsRect.left() + column * columnWidth,
                                    m_layoutCache.columnsRect.top(),
                                    columnWidth,
                                    m_layoutCache.columnsRect.height());
            m_layoutCache.columnRects.append(columnRect);
            QVector<QRectF> rows;
            rows.reserve(8);
            for (int row = 0; row < 8; ++row)
            {
                rows.append(QRectF(columnRect.left() + 4,
                                   columnRect.top() + row * 28,
                                   columnRect.width() - 8,
                                   24));
            }
            m_layoutCache.rowRects.append(rows);
        }

        m_layoutCache.valid = true;
        ++m_layoutBuildCount;
        syncPopupPerfCounters();
        return m_layoutCache;
    }

    QRect rowDirtyRect(int column, int row) const
    {
        const auto& cache = layoutCache();
        if (column < 0 || column >= cache.rowRects.size()
            || row < 0 || row >= cache.rowRects.at(column).size())
        {
            return QRect();
        }
        return cache.rowRects.at(column).at(row).adjusted(-3, -3, 3, 3).toAlignedRect().intersected(rect());
    }

    QRect columnDirtyRect(int column) const
    {
        const auto& cache = layoutCache();
        if (column < 0 || column >= cache.columnRects.size())
        {
            return QRect();
        }
        return cache.columnRects.at(column).adjusted(-2, -2, 2, 2).toAlignedRect().intersected(rect());
    }

    void updatePopupRegion(const QRect& dirty, const QString& mode)
    {
        QRect updateRect = dirty;
        if (!updateRect.isValid() || updateRect.isEmpty())
        {
            updateRect = rect();
        }
        ++m_regionUpdateCount;
        setProperty("antTimePickerPopupLastUpdateMode", mode);
        syncPopupPerfCounters();
        update(updateRect);
    }

public:
    void refreshForPanelTimeChange(const QTime& oldTime, const QTime& newTime)
    {
        QRect dirty;
        for (int column = 0; column < 3; ++column)
        {
            const int oldValue = column == 0 ? oldTime.hour() : (column == 1 ? oldTime.minute() : oldTime.second());
            const int newValue = column == 0 ? newTime.hour() : (column == 1 ? newTime.minute() : newTime.second());
            if (!oldTime.isValid() || !newTime.isValid() || oldValue != newValue)
            {
                dirty = dirty.united(columnDirtyRect(column));
            }
        }
        ++m_scopedColumnUpdateCount;
        updatePopupRegion(dirty, QStringLiteral("time"));
    }

    void refreshForStepChange(int column)
    {
        ++m_scopedColumnUpdateCount;
        updatePopupRegion(columnDirtyRect(column), QStringLiteral("step"));
    }

private:
    void syncPopupPerfCounters() const
    {
        auto* self = const_cast<AntTimePickerPopup*>(this);
        self->setProperty("antTimePickerPopupLayoutBuildCount", m_layoutBuildCount);
        self->setProperty("antTimePickerPopupLayoutCacheHitCount", m_layoutCacheHitCount);
        self->setProperty("antTimePickerPopupScopedRowUpdateCount", m_scopedRowUpdateCount);
        self->setProperty("antTimePickerPopupScopedColumnUpdateCount", m_scopedColumnUpdateCount);
        self->setProperty("antTimePickerPopupRegionUpdateCount", m_regionUpdateCount);
        if (m_owner)
        {
            m_owner->setProperty("antTimePickerPopupLayoutBuildCount", m_layoutBuildCount);
            m_owner->setProperty("antTimePickerPopupLayoutCacheHitCount", m_layoutCacheHitCount);
            m_owner->setProperty("antTimePickerPopupScopedRowUpdateCount", m_scopedRowUpdateCount);
            m_owner->setProperty("antTimePickerPopupScopedColumnUpdateCount", m_scopedColumnUpdateCount);
            m_owner->setProperty("antTimePickerPopupRegionUpdateCount", m_regionUpdateCount);
            m_owner->setProperty("antTimePickerPopupLastUpdateMode", property("antTimePickerPopupLastUpdateMode"));
        }
    }

    AntTimePicker* m_owner = nullptr;
    int m_hoveredColumn = -1;
    int m_hoveredRow = -1;
    bool m_cursorClickable = false;
    mutable PopupLayoutCache m_layoutCache;
    mutable int m_layoutBuildCount = 0;
    mutable int m_layoutCacheHitCount = 0;
    int m_scopedRowUpdateCount = 0;
    int m_scopedColumnUpdateCount = 0;
    int m_regionUpdateCount = 0;
};

AntTimePicker::AntTimePicker(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntTimePickerStyle>(this);
    setAttribute(Qt::WA_Hover, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_panelTime = QTime::currentTime();
    m_popup = new AntTimePickerPopup(this);

    updateCursor();
    syncTimePickerPerfCounters();
}

AntTimePicker::~AntTimePicker()
{
    delete m_popup;
    m_popup = nullptr;
}

QTime AntTimePicker::selectedTime() const { return m_selectedTime; }

void AntTimePicker::setSelectedTime(const QTime& time)
{
    const QTime nextTime = boundedTime(time);
    if (m_selectedTime == nextTime)
    {
        return;
    }
    const QTime oldPanelTime = m_panelTime;
    m_selectedTime = nextTime;
    if (m_selectedTime.isValid())
    {
        m_panelTime = m_selectedTime;
    }
    update();
    if (m_popup)
    {
        if (oldPanelTime != m_panelTime)
        {
            static_cast<AntTimePickerPopup*>(m_popup)->refreshForPanelTimeChange(oldPanelTime, m_panelTime);
        }
        else
        {
            m_popup->update();
        }
    }
    Q_EMIT selectedTimeChanged(m_selectedTime);
    Q_EMIT timeChanged(m_selectedTime);
    Q_EMIT timeStringChanged(timeString());
}

QTime AntTimePicker::time() const { return selectedTime(); }

void AntTimePicker::setTime(const QTime& time)
{
    setSelectedTime(time);
}

QTime AntTimePicker::minimumTime() const { return m_minimumTime; }

void AntTimePicker::setMinimumTime(const QTime& time)
{
    setTimeRange(time, m_maximumTime);
}

QTime AntTimePicker::maximumTime() const { return m_maximumTime; }

void AntTimePicker::setMaximumTime(const QTime& time)
{
    setTimeRange(m_minimumTime, time);
}

void AntTimePicker::setTimeRange(const QTime& minTime, const QTime& maxTime)
{
    QTime nextMin = minTime.isValid() ? minTime : QTime(0, 0, 0, 0);
    QTime nextMax = maxTime.isValid() ? maxTime : QTime(23, 59, 59, 999);
    if (nextMin > nextMax)
    {
        nextMax = nextMin;
    }

    const bool minChanged = m_minimumTime != nextMin;
    const bool maxChanged = m_maximumTime != nextMax;
    if (!minChanged && !maxChanged)
    {
        return;
    }

    m_minimumTime = nextMin;
    m_maximumTime = nextMax;
    if (m_selectedTime.isValid())
    {
        setSelectedTime(m_selectedTime);
    }
    if (m_startTime.isValid())
    {
        m_startTime = boundedTime(m_startTime);
    }
    if (m_endTime.isValid())
    {
        m_endTime = boundedTime(m_endTime);
    }
    if (m_panelTime.isValid())
    {
        m_panelTime = boundedTime(m_panelTime);
    }

    updateGeometry();
    update();
    if (m_popup)
    {
        m_popup->update();
    }
    if (minChanged)
    {
        Q_EMIT minimumTimeChanged(m_minimumTime);
    }
    if (maxChanged)
    {
        Q_EMIT maximumTimeChanged(m_maximumTime);
    }
    Q_EMIT timeRangeChanged(m_minimumTime, m_maximumTime);
    Q_EMIT timeStringChanged(timeString());
}

void AntTimePicker::clearMinimumTime()
{
    setMinimumTime(QTime(0, 0, 0, 0));
}

void AntTimePicker::clearMaximumTime()
{
    setMaximumTime(QTime(23, 59, 59, 999));
}

bool AntTimePicker::hasSelectedTime() const
{
    if (m_rangeMode)
    {
        return m_startTime.isValid() || m_endTime.isValid();
    }
    return m_selectedTime.isValid();
}

void AntTimePicker::clear()
{
    if (!hasSelectedTime())
    {
        return;
    }
    const bool hadSelectedTime = m_selectedTime.isValid();
    m_selectedTime = QTime();
    if (m_rangeMode)
    {
        m_startTime = QTime();
        m_endTime = QTime();
        m_pickingEnd = false;
        Q_EMIT startTimeChanged(m_startTime);
        Q_EMIT endTimeChanged(m_endTime);
    }
    if (hadSelectedTime)
    {
        Q_EMIT selectedTimeChanged(m_selectedTime);
    }
    update();
    if (m_popup)
    {
        m_popup->update();
    }
    Q_EMIT timeStringChanged(timeString());
    Q_EMIT cleared();
}

QString AntTimePicker::displayFormat() const { return m_displayFormat; }

void AntTimePicker::setDisplayFormat(const QString& format)
{
    if (format.isEmpty() || m_displayFormat == format)
    {
        return;
    }
    m_displayFormat = format;
    update();
    Q_EMIT displayFormatChanged(m_displayFormat);
    Q_EMIT timeStringChanged(timeString());
}

QString AntTimePicker::placeholderText() const { return m_placeholderText; }

void AntTimePicker::setPlaceholderText(const QString& text)
{
    if (m_placeholderText == text)
    {
        return;
    }
    m_placeholderText = text;
    update();
    Q_EMIT placeholderTextChanged(m_placeholderText);
}

Ant::Size AntTimePicker::pickerSize() const { return m_pickerSize; }

void AntTimePicker::setPickerSize(Ant::Size size)
{
    if (m_pickerSize == size)
    {
        return;
    }
    m_pickerSize = size;
    updateGeometry();
    update();
    Q_EMIT pickerSizeChanged(m_pickerSize);
}

Ant::Status AntTimePicker::status() const { return m_status; }

void AntTimePicker::setStatus(Ant::Status status)
{
    if (m_status == status)
    {
        return;
    }
    m_status = status;
    update();
    Q_EMIT statusChanged(m_status);
}

Ant::Variant AntTimePicker::variant() const { return m_variant; }

void AntTimePicker::setVariant(Ant::Variant variant)
{
    if (m_variant == variant)
    {
        return;
    }
    m_variant = variant;
    update();
    Q_EMIT variantChanged(m_variant);
}

bool AntTimePicker::allowClear() const { return m_allowClear; }

void AntTimePicker::setAllowClear(bool allowClear)
{
    if (m_allowClear == allowClear)
    {
        return;
    }
    m_allowClear = allowClear;
    update();
    Q_EMIT allowClearChanged(m_allowClear);
}

bool AntTimePicker::isOpen() const { return m_open; }

void AntTimePicker::setOpen(bool open)
{
    if (m_open == open)
    {
        return;
    }
    m_open = open;
    if (m_open)
    {
        if (m_selectedTime.isValid())
        {
            m_panelTime = m_selectedTime;
        }
        updatePopupGeometry();
        AntPopupMotion::show(m_popup);
    }
    else if (m_popup->isVisible())
    {
        AntPopupMotion::hide(m_popup);
    }
    update();
    Q_EMIT openChanged(m_open);
}

int AntTimePicker::hourStep() const { return m_hourStep; }

void AntTimePicker::setHourStep(int step)
{
    step = normalizeStep(step, 23);
    if (m_hourStep == step)
    {
        return;
    }
    m_hourStep = step;
    if (m_popup)
    {
        static_cast<AntTimePickerPopup*>(m_popup)->refreshForStepChange(0);
    }
    Q_EMIT hourStepChanged(m_hourStep);
}

int AntTimePicker::minuteStep() const { return m_minuteStep; }

void AntTimePicker::setMinuteStep(int step)
{
    step = normalizeStep(step, 59);
    if (m_minuteStep == step)
    {
        return;
    }
    m_minuteStep = step;
    if (m_popup)
    {
        static_cast<AntTimePickerPopup*>(m_popup)->refreshForStepChange(1);
    }
    Q_EMIT minuteStepChanged(m_minuteStep);
}

int AntTimePicker::secondStep() const { return m_secondStep; }

void AntTimePicker::setSecondStep(int step)
{
    step = normalizeStep(step, 59);
    if (m_secondStep == step)
    {
        return;
    }
    m_secondStep = step;
    if (m_popup)
    {
        static_cast<AntTimePickerPopup*>(m_popup)->refreshForStepChange(2);
    }
    Q_EMIT secondStepChanged(m_secondStep);
}

bool AntTimePicker::showNow() const { return m_showNow; }

void AntTimePicker::setShowNow(bool show)
{
    if (m_showNow == show)
    {
        return;
    }
    m_showNow = show;
    if (m_popup)
    {
        m_popup->update();
    }
    Q_EMIT showNowChanged(m_showNow);
}

QString AntTimePicker::timeString() const
{
    if (m_rangeMode)
    {
        const QString startText = m_startTime.isValid() ? m_startTime.toString(m_displayFormat) : QString();
        const QString endText = m_endTime.isValid() ? m_endTime.toString(m_displayFormat) : QString();
        if (startText.isEmpty() && endText.isEmpty())
        {
            return QString();
        }
        return QStringLiteral("%1 - %2").arg(startText, endText);
    }
    return m_selectedTime.isValid() ? m_selectedTime.toString(m_displayFormat) : QString();
}

bool AntTimePicker::isHoveredState() const { return m_hovered; }

QSize AntTimePicker::sizeHint() const
{
    return QSize(m_rangeMode ? 260 : 160, metrics().height);
}

QSize AntTimePicker::minimumSizeHint() const
{
    return QSize(120, metrics().height);
}

void AntTimePicker::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntTimePicker::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void AntTimePicker::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
    QWidget::leaveEvent(event);
}

void AntTimePicker::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isEnabled())
    {
        if (canClear() && iconRect(metrics()).contains(event->position()))
        {
            clear();
            event->accept();
            return;
        }
        setFocus(Qt::MouseFocusReason);
        setOpen(!m_open);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntTimePicker::keyPressEvent(QKeyEvent* event)
{
    if (!isEnabled())
    {
        QWidget::keyPressEvent(event);
        return;
    }
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        if (m_open)
        {
            acceptPanelTime();
        }
        else
        {
            setOpen(true);
        }
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Escape)
    {
        setOpen(false);
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void AntTimePicker::focusInEvent(QFocusEvent* event)
{
    update();
    QWidget::focusInEvent(event);
}

void AntTimePicker::focusOutEvent(QFocusEvent* event)
{
    if (!m_popup->isVisible())
    {
        setOpen(false);
    }
    update();
    QWidget::focusOutEvent(event);
}

void AntTimePicker::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        updateCursor();
        if (!isEnabled())
        {
            setOpen(false);
        }
        update();
    }
    QWidget::changeEvent(event);
}

AntTimePicker::Metrics AntTimePicker::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    m.height = token.controlHeight;
    m.fontSize = token.fontSize;
    m.radius = token.borderRadius;
    m.paddingX = token.paddingSM - token.lineWidth;
    m.iconWidth = token.fontSize + token.paddingXS * 2;
    if (m_pickerSize == Ant::Size::Large)
    {
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
    }
    else if (m_pickerSize == Ant::Size::Small)
    {
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSizeSM;
        m.radius = token.borderRadiusSM;
        m.paddingX = token.paddingXS;
    }
    return m;
}

QRectF AntTimePicker::controlRect() const
{
    const Metrics m = metrics();
    return QRectF(1, (height() - m.height) / 2.0, width() - 2, m.height);
}

QRectF AntTimePicker::iconRect(const Metrics& metrics) const
{
    const QRectF control = controlRect();
    return QRectF(control.right() - metrics.iconWidth, control.top(), metrics.iconWidth, control.height());
}

QColor AntTimePicker::borderColor() const
{
    const auto& token = antTheme->tokens();
    const bool focused = hasFocus() || m_open;
    if (!isEnabled())
    {
        return token.colorBorderDisabled;
    }
    if (m_status == Ant::Status::Error)
    {
        return focused ? token.colorError : (m_hovered ? token.colorErrorHover : token.colorError);
    }
    if (m_status == Ant::Status::Warning)
    {
        return focused ? token.colorWarning : (m_hovered ? token.colorWarningHover : token.colorWarning);
    }
    if (focused)
    {
        return token.colorPrimary;
    }
    if (m_hovered)
    {
        return token.colorPrimaryHover;
    }
    return token.colorBorder;
}

QColor AntTimePicker::backgroundColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorBgContainerDisabled;
    }
    if (m_variant == Ant::Variant::Filled)
    {
        return (hasFocus() || m_open)
            ? token.colorBgContainer
            : (m_hovered ? token.colorFillSecondary : token.colorFillTertiary);
    }
    if (m_variant == Ant::Variant::Borderless || m_variant == Ant::Variant::Underlined)
    {
        return QColor(0, 0, 0, 0);
    }
    return token.colorBgContainer;
}

bool AntTimePicker::canClear() const
{
    return isEnabled() && m_allowClear && m_hovered && m_selectedTime.isValid();
}

void AntTimePicker::updatePopupGeometry()
{
    if (!m_popup)
    {
        return;
    }
    const QPoint target = mapToGlobal(QPoint(-kTimePickerPopupShadowMargin,
                                             height() + 4 - kTimePickerPopupShadowMargin));
    if (m_hasPopupGeometryTarget && m_lastPopupGeometryTarget == target)
    {
        ++m_popupGeometrySkipCount;
        if (m_popup->pos() != target)
        {
            m_popup->move(target);
        }
        syncTimePickerPerfCounters();
        return;
    }
    if (m_popup->pos() != target)
    {
        m_popup->move(target);
    }
    m_lastPopupGeometryTarget = target;
    m_hasPopupGeometryTarget = true;
    ++m_popupGeometryApplyCount;
    syncTimePickerPerfCounters();
}

void AntTimePicker::setPanelTime(const QTime& time)
{
    const QTime nextTime = boundedTime(time);
    if (!nextTime.isValid())
    {
        return;
    }
    const QTime nextPanelTime(nextTime.hour(), nextTime.minute(), nextTime.second(), nextTime.msec());
    if (m_panelTime == nextPanelTime)
    {
        return;
    }
    const QTime oldPanelTime = m_panelTime;
    m_panelTime = nextPanelTime;
    if (m_popup)
    {
        static_cast<AntTimePickerPopup*>(m_popup)->refreshForPanelTimeChange(oldPanelTime, m_panelTime);
    }
}

void AntTimePicker::acceptPanelTime()
{
    if (!m_panelTime.isValid())
    {
        m_panelTime = QTime::currentTime();
    }
    if (m_rangeMode)
    {
        if (!m_pickingEnd || !m_startTime.isValid())
        {
            m_startTime = m_panelTime;
            m_endTime = QTime();
            m_pickingEnd = true;
            update();
            Q_EMIT startTimeChanged(m_startTime);
            Q_EMIT endTimeChanged(m_endTime);
            Q_EMIT timeStringChanged(timeString());
        }
        else
        {
            m_endTime = m_panelTime;
            m_pickingEnd = false;
            setSelectedTime(m_startTime);
            setOpen(false);
            Q_EMIT startTimeChanged(m_startTime);
            Q_EMIT endTimeChanged(m_endTime);
            Q_EMIT accepted(m_selectedTime);
        }
    }
    else
    {
        setSelectedTime(m_panelTime);
        setOpen(false);
        Q_EMIT accepted(m_selectedTime);
    }
}

bool AntTimePicker::isRangeMode() const { return m_rangeMode; }
void AntTimePicker::setRangeMode(bool rangeMode)
{
    if (m_rangeMode == rangeMode) return;
    m_rangeMode = rangeMode;
    m_pickingEnd = false;
    updateGeometry();
    update();
    Q_EMIT rangeModeChanged(m_rangeMode);
    Q_EMIT timeStringChanged(timeString());
}

QTime AntTimePicker::startTime() const { return m_startTime; }
void AntTimePicker::setStartTime(const QTime& time)
{
    const QTime nextTime = boundedTime(time);
    if (m_startTime == nextTime) return;
    m_startTime = nextTime;
    updateGeometry();
    update();
    Q_EMIT startTimeChanged(m_startTime);
    Q_EMIT timeStringChanged(timeString());
}

QTime AntTimePicker::endTime() const { return m_endTime; }
void AntTimePicker::setEndTime(const QTime& time)
{
    const QTime nextTime = boundedTime(time);
    if (m_endTime == nextTime) return;
    m_endTime = nextTime;
    updateGeometry();
    update();
    Q_EMIT endTimeChanged(m_endTime);
    Q_EMIT timeStringChanged(timeString());
}

void AntTimePicker::updateCursor()
{
    setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

int AntTimePicker::normalizeStep(int step, int maximum) const
{
    return std::clamp(step, 1, maximum);
}

QTime AntTimePicker::boundedTime(const QTime& time) const
{
    if (!time.isValid())
    {
        return QTime();
    }
    if (time < m_minimumTime)
    {
        return m_minimumTime;
    }
    if (time > m_maximumTime)
    {
        return m_maximumTime;
    }
    return time;
}

void AntTimePicker::syncTimePickerPerfCounters() const
{
    auto* self = const_cast<AntTimePicker*>(this);
    self->setProperty("antTimePickerPopupGeometryApplyCount", m_popupGeometryApplyCount);
    self->setProperty("antTimePickerPopupGeometrySkipCount", m_popupGeometrySkipCount);
}
