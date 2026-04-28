#include "AntSteps.h"

#include <QMouseEvent>
#include <QPainter>

#include "../styles/AntStepsStyle.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"

AntSteps::AntSteps(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntStepsStyle(style()));
    setMouseTracking(true);
}

int AntSteps::currentIndex() const { return m_currentIndex; }

void AntSteps::setCurrentIndex(int index)
{
    index = qBound(0, index, qMax(0, m_steps.size() - 1));
    if (m_currentIndex == index)
    {
        return;
    }
    m_currentIndex = index;
    update();
    Q_EMIT currentIndexChanged(m_currentIndex);
}

Ant::Orientation AntSteps::direction() const { return m_direction; }

void AntSteps::setDirection(Ant::Orientation direction)
{
    if (m_direction == direction)
    {
        return;
    }
    m_direction = direction;
    updateGeometry();
    update();
    Q_EMIT directionChanged(m_direction);
}

bool AntSteps::isClickable() const { return m_clickable; }

void AntSteps::setClickable(bool clickable)
{
    if (m_clickable == clickable)
    {
        return;
    }
    m_clickable = clickable;
    update();
    Q_EMIT clickableChanged(m_clickable);
}

int AntSteps::count() const { return m_steps.size(); }

AntStepItem AntSteps::stepAt(int index) const
{
    if (index < 0 || index >= m_steps.size())
    {
        return {};
    }
    return m_steps.at(index);
}

void AntSteps::addStep(const AntStepItem& step)
{
    m_steps.push_back(step);
    updateGeometry();
    update();
}

void AntSteps::addStep(const QString& title,
                       const QString& description,
                       const QString& subTitle,
                       Ant::StepStatus status,
                       bool disabled)
{
    addStep({title, description, subTitle, status, disabled});
}

void AntSteps::setStepStatus(int index, Ant::StepStatus status)
{
    if (index < 0 || index >= m_steps.size())
    {
        return;
    }
    if (m_steps[index].status == status)
    {
        return;
    }
    m_steps[index].status = status;
    update();
}

void AntSteps::clearSteps()
{
    m_steps.clear();
    m_currentIndex = 0;
    m_hoveredIndex = -1;
    updateGeometry();
    update();
}

QSize AntSteps::sizeHint() const
{
    const Metrics m = metrics();
    if (m_direction == Ant::Orientation::Vertical)
    {
        return QSize(420, qMax(120, m_steps.size() * 92));
    }
    return QSize(qMax(320, m_steps.size() * 220), 120);
}

QSize AntSteps::minimumSizeHint() const
{
    return QSize(220, 96);
}

void AntSteps::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntSteps::mouseMoveEvent(QMouseEvent* event)
{
    const int index = stepAtPos(event->pos());
    if (m_hoveredIndex != index)
    {
        m_hoveredIndex = index;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void AntSteps::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_clickable)
    {
        const int index = stepAtPos(event->pos());
        if (index >= 0 && index < m_steps.size() && !m_steps.at(index).disabled)
        {
            setCurrentIndex(index);
            Q_EMIT stepClicked(index);
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void AntSteps::leaveEvent(QEvent* event)
{
    if (m_hoveredIndex != -1)
    {
        m_hoveredIndex = -1;
        update();
    }
    QWidget::leaveEvent(event);
}

AntSteps::Metrics AntSteps::metrics() const
{
    Metrics m;
    const auto& token = antTheme->tokens();
    m.iconSize = token.controlHeight;
    m.titleGap = token.marginSM;
    m.tailThickness = 2;
    m.itemGap = token.marginLG;
    m.titleFontSize = token.fontSize;
    m.descFontSize = token.fontSizeSM;
    return m;
}

QVector<QRect> AntSteps::itemRects() const
{
    QVector<QRect> rects;
    if (m_steps.isEmpty())
    {
        return rects;
    }
    if (m_direction == Ant::Orientation::Horizontal)
    {
        const int itemWidth = qMax(160, width() / m_steps.size());
        for (int i = 0; i < m_steps.size(); ++i)
        {
            rects.push_back(QRect(i * itemWidth, 0, itemWidth, height()));
        }
    }
    else
    {
        const int itemHeight = qMax(72, height() / m_steps.size());
        for (int i = 0; i < m_steps.size(); ++i)
        {
            rects.push_back(QRect(0, i * itemHeight, width(), itemHeight));
        }
    }
    return rects;
}

QRect AntSteps::iconRect(const QRect& itemRect) const
{
    const Metrics m = metrics();
    if (m_direction == Ant::Orientation::Horizontal)
    {
        return QRect(itemRect.left(), 8, m.iconSize, m.iconSize);
    }
    return QRect(itemRect.left(), itemRect.top() + 8, m.iconSize, m.iconSize);
}

QRect AntSteps::textRect(const QRect& itemRect) const
{
    const Metrics m = metrics();
    if (m_direction == Ant::Orientation::Horizontal)
    {
        return QRect(itemRect.left() + m.iconSize + 12, 6, itemRect.width() - m.iconSize - 16, itemRect.height() - 12);
    }
    return QRect(itemRect.left() + m.iconSize + 16, itemRect.top() + 4, itemRect.width() - m.iconSize - 20, itemRect.height() - 8);
}

int AntSteps::stepAtPos(const QPoint& pos) const
{
    const QVector<QRect> rects = itemRects();
    for (int i = 0; i < rects.size(); ++i)
    {
        if (rects.at(i).contains(pos))
        {
            return i;
        }
    }
    return -1;
}

Ant::StepStatus AntSteps::effectiveStatus(int index) const
{
    if (index < 0 || index >= m_steps.size())
    {
        return Ant::StepStatus::Wait;
    }
    const AntStepItem& step = m_steps.at(index);
    if (step.status == Ant::StepStatus::Error)
    {
        return Ant::StepStatus::Error;
    }
    if (step.status == Ant::StepStatus::Finish)
    {
        return Ant::StepStatus::Finish;
    }
    if (index < m_currentIndex)
    {
        return Ant::StepStatus::Finish;
    }
    if (index == m_currentIndex)
    {
        return Ant::StepStatus::Process;
    }
    return Ant::StepStatus::Wait;
}

QColor AntSteps::statusColor(Ant::StepStatus status) const
{
    const auto& token = antTheme->tokens();
    switch (status)
    {
    case Ant::StepStatus::Finish:
    case Ant::StepStatus::Process:
        return token.colorPrimary;
    case Ant::StepStatus::Error:
        return token.colorError;
    case Ant::StepStatus::Wait:
    default:
        return token.colorBorder;
    }
}

QString AntSteps::iconText(Ant::StepStatus status, int index) const
{
    switch (status)
    {
    case Ant::StepStatus::Finish:
        return QStringLiteral("✓");
    case Ant::StepStatus::Error:
        return QStringLiteral("×");
    case Ant::StepStatus::Process:
    case Ant::StepStatus::Wait:
    default:
        return QString::number(index + 1);
    }
}
