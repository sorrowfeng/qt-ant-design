#include "AntSteps.h"

#include <QMouseEvent>
#include <QPainter>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"

AntSteps::AntSteps(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
        update();
    });
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

Ant::StepsDirection AntSteps::direction() const { return m_direction; }

void AntSteps::setDirection(Ant::StepsDirection direction)
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
    if (m_direction == Ant::StepsDirection::Vertical)
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
    const auto& token = antTheme->tokens();
    const Metrics m = metrics();
    const QVector<QRect> rects = itemRects();

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    for (int i = 0; i < rects.size(); ++i)
    {
        const QRect itemRect = rects.at(i);
        const QRect circle = iconRect(itemRect);
        const QRect textArea = textRect(itemRect);
        const Ant::StepStatus status = effectiveStatus(i);
        const QColor color = statusColor(status);
        const bool disabled = m_steps.at(i).disabled;
        const bool hovered = i == m_hoveredIndex && m_clickable && !disabled;

        if (i < rects.size() - 1)
        {
            if (m_direction == Ant::StepsDirection::Horizontal)
            {
                const int y = circle.center().y();
                const int x1 = circle.right() + 8;
                const int x2 = rects.at(i + 1).left() - 8;
                painter.setPen(QPen(i < m_currentIndex ? token.colorPrimary : token.colorSplit,
                                    m.tailThickness,
                                    Qt::SolidLine,
                                    Qt::RoundCap));
                painter.drawLine(QPoint(x1, y), QPoint(x2, y));
            }
            else
            {
                const int x = circle.center().x();
                const int y1 = circle.bottom() + 8;
                const int y2 = rects.at(i + 1).top() + 8;
                painter.setPen(QPen(i < m_currentIndex ? token.colorPrimary : token.colorSplit,
                                    m.tailThickness,
                                    Qt::SolidLine,
                                    Qt::RoundCap));
                painter.drawLine(QPoint(x, y1), QPoint(x, y2));
            }
        }

        QColor fill = Qt::transparent;
        QColor border = color;
        QColor numberColor = color;
        if (status == Ant::StepStatus::Process)
        {
            fill = color;
            numberColor = token.colorTextLightSolid;
        }
        else if (status == Ant::StepStatus::Finish)
        {
            fill = token.colorBgContainer;
        }
        else if (status == Ant::StepStatus::Error)
        {
            fill = token.colorBgContainer;
            border = token.colorError;
            numberColor = token.colorError;
        }
        else
        {
            border = disabled ? token.colorBorderDisabled : token.colorBorder;
            numberColor = disabled ? token.colorTextDisabled : token.colorTextTertiary;
        }

        if (hovered)
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(AntPalette::alpha(token.colorPrimary, 0.08));
            painter.drawEllipse(circle.adjusted(-4, -4, 4, 4));
        }

        painter.setPen(QPen(border, 2));
        painter.setBrush(fill);
        painter.drawEllipse(circle);

        QFont iconFont = painter.font();
        iconFont.setPixelSize(14);
        iconFont.setWeight(QFont::DemiBold);
        painter.setFont(iconFont);
        painter.setPen(numberColor);
        painter.drawText(circle, Qt::AlignCenter, iconText(status, i));

        QFont titleFont = painter.font();
        titleFont.setPixelSize(m.titleFontSize);
        titleFont.setWeight(status == Ant::StepStatus::Process ? QFont::DemiBold : QFont::Normal);
        painter.setFont(titleFont);
        painter.setPen(disabled ? token.colorTextDisabled : (status == Ant::StepStatus::Wait ? token.colorTextSecondary : token.colorText));
        QRect titleRect = textArea;
        titleRect.setHeight(m.titleFontSize + 8);
        painter.drawText(titleRect, Qt::AlignLeft | Qt::AlignTop, m_steps.at(i).title);

        if (!m_steps.at(i).subTitle.isEmpty())
        {
            QFont subFont = painter.font();
            subFont.setPixelSize(m.descFontSize);
            painter.setFont(subFont);
            painter.setPen(token.colorTextTertiary);
            QRect subRect = titleRect;
            subRect.moveTop(titleRect.bottom() + 2);
            subRect.setHeight(m.descFontSize + 6);
            painter.drawText(subRect, Qt::AlignLeft | Qt::AlignTop, m_steps.at(i).subTitle);
        }

        if (!m_steps.at(i).description.isEmpty())
        {
            QFont descFont = painter.font();
            descFont.setPixelSize(m.descFontSize);
            descFont.setWeight(QFont::Normal);
            painter.setFont(descFont);
            painter.setPen(disabled ? token.colorTextDisabled : token.colorTextSecondary);
            QRect descRect = textArea;
            descRect.setTop(titleRect.bottom() + (m_steps.at(i).subTitle.isEmpty() ? 6 : 22));
            painter.drawText(descRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, m_steps.at(i).description);
        }
    }
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
    if (m_direction == Ant::StepsDirection::Horizontal)
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
    if (m_direction == Ant::StepsDirection::Horizontal)
    {
        return QRect(itemRect.left(), 8, m.iconSize, m.iconSize);
    }
    return QRect(itemRect.left(), itemRect.top() + 8, m.iconSize, m.iconSize);
}

QRect AntSteps::textRect(const QRect& itemRect) const
{
    const Metrics m = metrics();
    if (m_direction == Ant::StepsDirection::Horizontal)
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
        return QStringLiteral("!");
    case Ant::StepStatus::Process:
    case Ant::StepStatus::Wait:
    default:
        return QString::number(index + 1);
    }
}
