#include "AntStepsStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntIconPainter.h"
#include "widgets/AntSteps.h"

AntStepsStyle::AntStepsStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntStepsStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
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
    AntStyleBase::unpolish(widget);
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

void AntStepsStyle::drawSteps(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* steps = qobject_cast<const AntSteps*>(widget);
    if (!steps || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const auto m = steps->metrics();
    const auto layouts = steps->layoutItems();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    for (int i = 0; i < layouts.size(); ++i)
    {
        const auto& layout = layouts.at(i);
        const QRect itemRect = layout.itemRect;
        const QRect circle = layout.iconRect;
        const QRect textArea = layout.textRect;
        const Ant::StepStatus status = steps->effectiveStatus(i);
        const QColor color = steps->statusColor(status);
        const AntStepItem step = steps->stepAt(i);
        const bool disabled = step.disabled;
        QFont titleFontForItem = painter->font();
        titleFontForItem.setPixelSize(m.titleFontSize);
        titleFontForItem.setWeight(status == Ant::StepStatus::Process ? QFont::DemiBold : QFont::Normal);
        QFont descFontForItem = painter->font();
        descFontForItem.setPixelSize(m.descFontSize);
        descFontForItem.setWeight(QFont::Normal);

        // Tail line
        if (i < layouts.size() - 1)
        {
            if (steps->direction() == Ant::Orientation::Horizontal)
            {
                const int y = circle.center().y();
                int headerWidth = QFontMetrics(titleFontForItem).horizontalAdvance(step.title);
                if (!step.subTitle.isEmpty())
                {
                    headerWidth += token.marginXS + QFontMetrics(descFontForItem).horizontalAdvance(step.subTitle);
                }
                const int x1 = textArea.left() + headerWidth + token.margin;
                const int x2 = itemRect.right() - token.marginXS;
                painter->setPen(QPen(i < steps->currentIndex() ? token.colorPrimary : token.colorSplit,
                                    m.tailThickness, Qt::SolidLine, Qt::RoundCap));
                if (x2 > x1)
                {
                    painter->drawLine(QPoint(x1, y), QPoint(x2, y));
                }
            }
            else
            {
                const int x = circle.center().x();
                const int y1 = circle.bottom() + 8;
                const int y2 = layouts.at(i + 1).itemRect.top() + 8;
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
            fill = token.colorPrimaryBg;
            border = token.colorPrimaryBg;
        }
        else if (status == Ant::StepStatus::Error)
        {
            fill = token.colorError;
            border = token.colorError;
            numberColor = token.colorTextLightSolid;
        }
        else
        {
            fill = token.colorFillQuaternary;
            border = disabled ? token.colorBorderDisabled : token.colorBorder;
            numberColor = disabled ? token.colorTextDisabled : token.colorTextTertiary;
        }

        painter->setPen(QPen(border, 2));
        painter->setBrush(fill);
        painter->drawEllipse(circle);

        // Icon / number
        if (status == Ant::StepStatus::Finish || status == Ant::StepStatus::Error)
        {
            const Ant::IconType iconType = status == Ant::StepStatus::Error ? Ant::IconType::Close : Ant::IconType::Check;
            const QColor iconColor = status == Ant::StepStatus::Finish ? token.colorPrimary : token.colorTextLightSolid;
            AntIconPainter::drawIcon(*painter, iconType, QRectF(circle).adjusted(8, 8, -8, -8), iconColor);
        }
        else
        {
            QFont iconFont = painter->font();
            iconFont.setPixelSize(14);
            iconFont.setWeight(QFont::DemiBold);
            painter->setFont(iconFont);
            painter->setPen(numberColor);
            painter->drawText(circle, Qt::AlignCenter, steps->iconText(status, i));
        }

        // Title
        painter->setFont(titleFontForItem);
        painter->setPen(disabled ? token.colorTextDisabled : (status == Ant::StepStatus::Error ? token.colorError : (status == Ant::StepStatus::Wait ? token.colorTextSecondary : token.colorText)));
        QRect titleRect = textArea;
        if (steps->direction() == Ant::Orientation::Horizontal)
        {
            titleRect.setTop(circle.top());
            titleRect.setHeight(m.iconSize);
            painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, step.title);
        }
        else
        {
            titleRect.setHeight(m.titleFontSize + 8);
            painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignTop, step.title);
        }

        // Sub-title
        if (!step.subTitle.isEmpty())
        {
            painter->setFont(descFontForItem);
            painter->setPen(token.colorTextTertiary);
            if (steps->direction() == Ant::Orientation::Horizontal)
            {
                const int titleWidth = QFontMetrics(titleFontForItem).horizontalAdvance(step.title);
                QRect subRect(titleRect.left() + titleWidth + token.marginXS,
                              titleRect.top(),
                              qMax(0, titleRect.width() - titleWidth - token.marginXS),
                              titleRect.height());
                painter->drawText(subRect, Qt::AlignLeft | Qt::AlignVCenter, step.subTitle);
            }
            else
            {
                QRect subRect = titleRect;
                subRect.moveTop(titleRect.bottom() + 2);
                subRect.setHeight(m.descFontSize + 6);
                painter->drawText(subRect, Qt::AlignLeft | Qt::AlignTop, step.subTitle);
            }
        }

        // Description
        if (!step.description.isEmpty())
        {
            painter->setFont(descFontForItem);
            painter->setPen(disabled ? token.colorTextDisabled : (status == Ant::StepStatus::Error ? token.colorError : token.colorTextSecondary));
            QRect descRect = textArea;
            if (steps->direction() == Ant::Orientation::Horizontal)
            {
                descRect.setTop(titleRect.bottom());
            }
            else
            {
                descRect.setTop(titleRect.bottom() + (step.subTitle.isEmpty() ? 6 : 22));
            }
            painter->drawText(descRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, step.description);
        }
    }

    painter->restore();
}
