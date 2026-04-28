#include "AntTimelineStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntTimeline.h"

AntTimelineStyle::AntTimelineStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntTimeline>();
}

void AntTimelineStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntTimeline*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntTimelineStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntTimeline*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntTimelineStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntTimeline*>(widget))
    {
        drawTimeline(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntTimelineStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntTimelineStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* timeline = qobject_cast<AntTimeline*>(watched);
    if (timeline && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(timeline);
        option.rect = timeline->rect();
        QPainter painter(timeline);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, timeline);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

namespace
{
struct TimelineMetrics
{
    int dotSize = 10;
    int dotBorderWidth = 2;
    int lineWidth = 2;
    int gap = 16;
    int itemSpacing = 20;
    int titleFontSize = 14;
    int contentFontSize = 12;
    int sideMargin = 16;
};

TimelineMetrics timelineMetrics()
{
    TimelineMetrics m;
    const auto& token = antTheme->tokens();
    m.titleFontSize = token.fontSize;
    m.contentFontSize = token.fontSizeSM;
    m.sideMargin = token.padding;
    m.gap = token.paddingSM;
    m.itemSpacing = token.marginSM;
    return m;
}

QColor timelineDotColor(const QString& color)
{
    const auto& token = antTheme->tokens();
    if (color.isEmpty() || color == QStringLiteral("blue"))
    {
        return token.colorPrimary;
    }
    if (color == QStringLiteral("red"))
    {
        return token.colorError;
    }
    if (color == QStringLiteral("green"))
    {
        return token.colorSuccess;
    }
    if (color == QStringLiteral("gray"))
    {
        return token.colorBorder;
    }
    const QColor c(color);
    if (c.isValid())
    {
        return c;
    }
    return token.colorPrimary;
}

int verticalDotX(const AntTimeline* timeline, int index, int widgetWidth, const TimelineMetrics& m)
{
    if (timeline->mode() == Ant::TimelineMode::Start)
    {
        return m.sideMargin;
    }
    if (timeline->mode() == Ant::TimelineMode::End)
    {
        return widgetWidth - m.sideMargin - m.dotSize;
    }
    return (index % 2 == 0) ? m.sideMargin : (widgetWidth - m.sideMargin - m.dotSize);
}

int verticalTextX(const AntTimeline* timeline, int index, int dotX, const TimelineMetrics& m)
{
    if (timeline->mode() == Ant::TimelineMode::Start)
    {
        return dotX + m.dotSize + m.gap;
    }
    if (timeline->mode() == Ant::TimelineMode::End)
    {
        return m.sideMargin;
    }
    if (index % 2 == 0)
    {
        return dotX + m.dotSize + m.gap;
    }
    return m.sideMargin;
}

int verticalTextWidth(int textX, int widgetWidth, const TimelineMetrics& m)
{
    return widgetWidth - textX - m.sideMargin;
}

int horizontalDotY(const AntTimeline* timeline, int index, int widgetHeight, const TimelineMetrics& m)
{
    if (timeline->mode() == Ant::TimelineMode::Start)
    {
        return m.sideMargin;
    }
    if (timeline->mode() == Ant::TimelineMode::End)
    {
        return widgetHeight - m.sideMargin - m.dotSize;
    }
    return (index % 2 == 0) ? m.sideMargin : (widgetHeight - m.sideMargin - m.dotSize);
}

int itemHeightForVertical(const AntTimelineItem& item, const TimelineMetrics& m, int textWidth)
{
    QFont titleFont;
    titleFont.setPixelSize(m.titleFontSize);
    titleFont.setWeight(QFont::Normal);
    const QFontMetrics titleFm(titleFont);
    int totalHeight = titleFm.height() + 4;

    if (!item.content.isEmpty())
    {
        QFont contentFont;
        contentFont.setPixelSize(m.contentFontSize);
        const QFontMetrics contentFm(contentFont);
        const QRect contentBounding = contentFm.boundingRect(
            QRect(0, 0, qMax(40, textWidth), 10000), Qt::TextWordWrap, item.content);
        totalHeight += contentBounding.height() + 8;
    }

    totalHeight = qMax(totalHeight, m.dotSize);
    totalHeight += m.itemSpacing;
    return totalHeight;
}
} // namespace

void AntTimelineStyle::drawTimeline(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* timeline = qobject_cast<const AntTimeline*>(widget);
    if (!timeline || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const TimelineMetrics m = timelineMetrics();
    const int count = timeline->count();
    if (count == 0)
    {
        return;
    }

    const int widgetWidth = option->rect.width();
    const int widgetHeight = option->rect.height();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (timeline->orientation() == Ant::TimelineOrientation::Vertical)
    {
        // Pre-calculate item heights for dynamic sizing
        QVector<int> itemHeights;
        itemHeights.reserve(count);
        for (int i = 0; i < count; ++i)
        {
            const int estimatedTextWidth = widgetWidth / 2 - m.sideMargin - m.dotSize - m.gap;
            itemHeights.push_back(itemHeightForVertical(timeline->itemAt(i), m, estimatedTextWidth));
        }

        int y = m.sideMargin;
        for (int i = 0; i < count; ++i)
        {
            const int displayIndex = timeline->isReverse() ? (count - 1 - i) : i;
            const AntTimelineItem item = timeline->itemAt(displayIndex);
            const QColor dotColor = timelineDotColor(item.color);
            const int currentHeight = itemHeights.at(i);

            const int dotX = verticalDotX(timeline, i, widgetWidth, m);
            const int dotCenterX = dotX + m.dotSize / 2;
            const int dotTopY = y;

            // Draw tail line connecting to next dot
            if (i < count - 1)
            {
                const int nextDotX = verticalDotX(timeline, i + 1, widgetWidth, m);
                const int nextCenterX = nextDotX + m.dotSize / 2;
                const int lineY1 = dotTopY + m.dotSize;
                const int lineY2 = y + currentHeight;

                if (item.loading)
                {
                    painter->setPen(QPen(token.colorSplit, m.lineWidth, Qt::DashLine));
                }
                else
                {
                    painter->setPen(QPen(token.colorSplit, m.lineWidth, Qt::SolidLine, Qt::RoundCap));
                }
                painter->drawLine(dotCenterX, lineY1, nextCenterX, lineY2);
            }

            // Draw dot
            if (timeline->dotVariant() == Ant::TimelineDotVariant::Filled)
            {
                painter->setPen(Qt::NoPen);
                painter->setBrush(dotColor);
                painter->drawEllipse(dotX, dotTopY, m.dotSize, m.dotSize);
            }
            else
            {
                painter->setPen(QPen(dotColor, m.dotBorderWidth));
                painter->setBrush(token.colorBgContainer);
                painter->drawEllipse(dotX, dotTopY, m.dotSize, m.dotSize);
            }

            // Loading indicator inside dot
            if (item.loading)
            {
                painter->setPen(QPen(dotColor, 1, Qt::SolidLine, Qt::RoundCap));
                painter->setBrush(Qt::NoBrush);
                const QRectF arcRect(dotX + 1, dotTopY + 1, m.dotSize - 2, m.dotSize - 2);
                painter->drawArc(arcRect, 0, 270 * 16);
            }

            // Calculate text area
            const int textX = verticalTextX(timeline, i, dotX, m);
            const int textW = verticalTextWidth(textX, widgetWidth, m);

            // Draw title
            QFont titleFont;
            titleFont.setPixelSize(m.titleFontSize);
            titleFont.setWeight(QFont::Normal);
            painter->setFont(titleFont);
            painter->setPen(token.colorText);
            const QRect titleRect(textX, dotTopY, textW, m.titleFontSize + 4);
            painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignTop, item.title);

            // Draw content
            if (!item.content.isEmpty())
            {
                QFont contentFont;
                contentFont.setPixelSize(m.contentFontSize);
                contentFont.setWeight(QFont::Normal);
                painter->setFont(contentFont);
                painter->setPen(token.colorTextSecondary);
                const QRect contentRect(textX, dotTopY + m.titleFontSize + 8, textW,
                                        currentHeight - m.titleFontSize - m.itemSpacing - 8);
                painter->drawText(contentRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, item.content);
            }

            y += currentHeight;
        }
    }
    else // Horizontal
    {
        const int itemWidth = qMax(120, (widgetWidth - 2 * m.sideMargin) / qMax(1, count));

        int x = m.sideMargin;
        for (int i = 0; i < count; ++i)
        {
            const int displayIndex = timeline->isReverse() ? (count - 1 - i) : i;
            const AntTimelineItem item = timeline->itemAt(displayIndex);
            const QColor dotColor = timelineDotColor(item.color);

            const int dotY = horizontalDotY(timeline, i, widgetHeight, m);
            const int dotX = x;
            const int dotCenterY = dotY + m.dotSize / 2;

            // Draw tail line connecting to next dot
            if (i < count - 1)
            {
                const int nextDotY = horizontalDotY(timeline, i + 1, widgetHeight, m);
                const int nextCenterY = nextDotY + m.dotSize / 2;
                const int lineX1 = dotX + m.dotSize;
                const int lineX2 = x + itemWidth;

                if (item.loading)
                {
                    painter->setPen(QPen(token.colorSplit, m.lineWidth, Qt::DashLine));
                }
                else
                {
                    painter->setPen(QPen(token.colorSplit, m.lineWidth, Qt::SolidLine, Qt::RoundCap));
                }
                painter->drawLine(lineX1, dotCenterY, lineX2, nextCenterY);
            }

            // Draw dot
            if (timeline->dotVariant() == Ant::TimelineDotVariant::Filled)
            {
                painter->setPen(Qt::NoPen);
                painter->setBrush(dotColor);
                painter->drawEllipse(dotX, dotY, m.dotSize, m.dotSize);
            }
            else
            {
                painter->setPen(QPen(dotColor, m.dotBorderWidth));
                painter->setBrush(token.colorBgContainer);
                painter->drawEllipse(dotX, dotY, m.dotSize, m.dotSize);
            }

            // Loading indicator inside dot
            if (item.loading)
            {
                painter->setPen(QPen(dotColor, 1, Qt::SolidLine, Qt::RoundCap));
                painter->setBrush(Qt::NoBrush);
                const QRectF arcRect(dotX + 1, dotY + 1, m.dotSize - 2, m.dotSize - 2);
                painter->drawArc(arcRect, 0, 270 * 16);
            }

            // Calculate text area
            int textY;
            int textH;
            if (timeline->mode() == Ant::TimelineMode::Start)
            {
                textY = dotY + m.dotSize + m.gap;
                textH = widgetHeight - textY - m.sideMargin;
            }
            else if (timeline->mode() == Ant::TimelineMode::End)
            {
                textY = m.sideMargin;
                textH = dotY - m.gap - m.sideMargin;
            }
            else
            {
                if (i % 2 == 0)
                {
                    textY = dotY + m.dotSize + m.gap;
                    textH = widgetHeight - textY - m.sideMargin;
                }
                else
                {
                    textY = m.sideMargin;
                    textH = dotY - m.gap - m.sideMargin;
                }
            }

            // Draw title
            QFont titleFont;
            titleFont.setPixelSize(m.titleFontSize);
            titleFont.setWeight(QFont::Normal);
            painter->setFont(titleFont);
            painter->setPen(token.colorText);
            const QRect titleRect(dotX, textY, itemWidth, m.titleFontSize + 4);
            painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignTop, item.title);

            // Draw content
            if (!item.content.isEmpty())
            {
                QFont contentFont;
                contentFont.setPixelSize(m.contentFontSize);
                contentFont.setWeight(QFont::Normal);
                painter->setFont(contentFont);
                painter->setPen(token.colorTextSecondary);
                const QRect contentRect(dotX, textY + m.titleFontSize + 8, itemWidth,
                                        qMax(0, textH - m.titleFontSize - 8));
                painter->drawText(contentRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, item.content);
            }

            x += itemWidth;
        }
    }

    painter->restore();
}
