#include "AntStepsStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntSteps.h"

AntStepsStyle::AntStepsStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntSteps>();
}

void AntStepsStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntSteps*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntStepsStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntSteps*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntStepsStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntSteps*>(widget))
    {
        drawSteps(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntStepsStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntStepsStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* steps = qobject_cast<AntSteps*>(watched);
    if (steps && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(steps);
        option.rect = steps->rect();
        QPainter painter(steps);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, steps);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

namespace
{
struct StepsMetrics
{
    int iconSize = 32;
    int titleGap = 12;
    int tailThickness = 2;
    int itemGap = 20;
    int titleFontSize = 14;
    int descFontSize = 12;
};

StepsMetrics stepsMetrics()
{
    StepsMetrics m;
    const auto& token = antTheme->tokens();
    m.iconSize = token.controlHeight;
    m.titleGap = token.marginSM;
    m.tailThickness = 2;
    m.itemGap = token.marginLG;
    m.titleFontSize = token.fontSize;
    m.descFontSize = token.fontSizeSM;
    return m;
}

QVector<QRect> stepsItemRects(const AntSteps* steps, const QRect& bounds)
{
    QVector<QRect> rects;
    const int count = steps->count();
    if (count == 0)
    {
        return rects;
    }
    if (steps->direction() == Ant::Orientation::Horizontal)
    {
        const int itemWidth = qMax(160, bounds.width() / count);
        for (int i = 0; i < count; ++i)
        {
            rects.push_back(QRect(i * itemWidth, 0, itemWidth, bounds.height()));
        }
    }
    else
    {
        const int itemHeight = qMax(72, bounds.height() / count);
        for (int i = 0; i < count; ++i)
        {
            rects.push_back(QRect(0, i * itemHeight, bounds.width(), itemHeight));
        }
    }
    return rects;
}

QRect stepsIconRect(const QRect& itemRect, Ant::Orientation direction)
{
    const StepsMetrics m = stepsMetrics();
    if (direction == Ant::Orientation::Horizontal)
    {
        return QRect(itemRect.left(), 8, m.iconSize, m.iconSize);
    }
    return QRect(itemRect.left(), itemRect.top() + 8, m.iconSize, m.iconSize);
}

QRect stepsTextRect(const QRect& itemRect, Ant::Orientation direction)
{
    const StepsMetrics m = stepsMetrics();
    if (direction == Ant::Orientation::Horizontal)
    {
        return QRect(itemRect.left() + m.iconSize + 12, 6, itemRect.width() - m.iconSize - 16, itemRect.height() - 12);
    }
    return QRect(itemRect.left() + m.iconSize + 16, itemRect.top() + 4, itemRect.width() - m.iconSize - 20, itemRect.height() - 8);
}

Ant::StepStatus stepsEffectiveStatus(const AntSteps* steps, int index)
{
    const int count = steps->count();
    if (index < 0 || index >= count)
    {
        return Ant::StepStatus::Wait;
    }
    const AntStepItem step = steps->stepAt(index);
    if (step.status == Ant::StepStatus::Error)
    {
        return Ant::StepStatus::Error;
    }
    if (step.status == Ant::StepStatus::Finish)
    {
        return Ant::StepStatus::Finish;
    }
    if (index < steps->currentIndex())
    {
        return Ant::StepStatus::Finish;
    }
    if (index == steps->currentIndex())
    {
        return Ant::StepStatus::Process;
    }
    return Ant::StepStatus::Wait;
}

QColor stepsStatusColor(Ant::StepStatus status)
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

QString stepsIconText(Ant::StepStatus status, int index)
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
} // namespace

void AntStepsStyle::drawSteps(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* steps = qobject_cast<const AntSteps*>(widget);
    if (!steps || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const StepsMetrics m = stepsMetrics();
    const int count = steps->count();
    const auto rects = stepsItemRects(steps, option->rect);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    for (int i = 0; i < rects.size(); ++i)
    {
        const QRect itemRect = rects.at(i);
        const QRect circle = stepsIconRect(itemRect, steps->direction());
        const QRect textArea = stepsTextRect(itemRect, steps->direction());
        const Ant::StepStatus status = stepsEffectiveStatus(steps, i);
        const QColor color = stepsStatusColor(status);
        const AntStepItem step = steps->stepAt(i);
        const bool disabled = step.disabled;

        // Tail line
        if (i < rects.size() - 1)
        {
            if (steps->direction() == Ant::Orientation::Horizontal)
            {
                const int y = circle.center().y();
                const int x1 = circle.right() + 8;
                const int x2 = rects.at(i + 1).left() - 8;
                painter->setPen(QPen(i < steps->currentIndex() ? token.colorPrimary : token.colorSplit,
                                    m.tailThickness, Qt::SolidLine, Qt::RoundCap));
                painter->drawLine(QPoint(x1, y), QPoint(x2, y));
            }
            else
            {
                const int x = circle.center().x();
                const int y1 = circle.bottom() + 8;
                const int y2 = rects.at(i + 1).top() + 8;
                painter->setPen(QPen(i < steps->currentIndex() ? token.colorPrimary : token.colorSplit,
                                    m.tailThickness, Qt::SolidLine, Qt::RoundCap));
                painter->drawLine(QPoint(x, y1), QPoint(x, y2));
            }
        }

        // Circle fill/border
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

        painter->setPen(QPen(border, 2));
        painter->setBrush(fill);
        painter->drawEllipse(circle);

        // Icon text
        QFont iconFont = painter->font();
        iconFont.setPixelSize(14);
        iconFont.setWeight(QFont::DemiBold);
        painter->setFont(iconFont);
        painter->setPen(numberColor);
        painter->drawText(circle, Qt::AlignCenter, stepsIconText(status, i));

        // Title
        QFont titleFont = painter->font();
        titleFont.setPixelSize(m.titleFontSize);
        titleFont.setWeight(status == Ant::StepStatus::Process ? QFont::DemiBold : QFont::Normal);
        painter->setFont(titleFont);
        painter->setPen(disabled ? token.colorTextDisabled : (status == Ant::StepStatus::Wait ? token.colorTextSecondary : token.colorText));
        QRect titleRect = textArea;
        titleRect.setHeight(m.titleFontSize + 8);
        painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignTop, step.title);

        // Sub-title
        if (!step.subTitle.isEmpty())
        {
            QFont subFont = painter->font();
            subFont.setPixelSize(m.descFontSize);
            painter->setFont(subFont);
            painter->setPen(token.colorTextTertiary);
            QRect subRect = titleRect;
            subRect.moveTop(titleRect.bottom() + 2);
            subRect.setHeight(m.descFontSize + 6);
            painter->drawText(subRect, Qt::AlignLeft | Qt::AlignTop, step.subTitle);
        }

        // Description
        if (!step.description.isEmpty())
        {
            QFont descFont = painter->font();
            descFont.setPixelSize(m.descFontSize);
            descFont.setWeight(QFont::Normal);
            painter->setFont(descFont);
            painter->setPen(disabled ? token.colorTextDisabled : token.colorTextSecondary);
            QRect descRect = textArea;
            descRect.setTop(titleRect.bottom() + (step.subTitle.isEmpty() ? 6 : 22));
            painter->drawText(descRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, step.description);
        }
    }

    painter->restore();
}
