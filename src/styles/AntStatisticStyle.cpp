#include "AntStatisticStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntStatistic.h"

namespace
{

struct Metrics
{
    int padding = 16;
    int titleFontSize = 14;
    int valueFontSize = 24;
    int prefixFontSize = 16;
    int suffixFontSize = 14;
    int spacing = 4;
};

Metrics statisticMetrics()
{
    Metrics m;
    m.padding = 0;
    m.prefixFontSize = m.valueFontSize;
    m.suffixFontSize = m.valueFontSize;
    return m;
}

QRect statisticTitleRect(const AntStatistic* stat, const QRect& widgetRect, const QFont& widgetFont)
{
    const Metrics m = statisticMetrics();
    const QFont titleFont = AntStyleBase::withPixelSize(widgetFont, m.titleFontSize);
    QFontMetrics titleFm(titleFont);
    return QRect(m.padding, m.padding, widgetRect.width() - m.padding * 2, titleFm.height());
}

QRect statisticValueRect(const AntStatistic* stat, const QRect& widgetRect, const QFont& widgetFont)
{
    const Metrics m = statisticMetrics();
    int top = m.padding;

    if (!stat->title().isEmpty())
    {
        const QFont titleFont = AntStyleBase::withPixelSize(widgetFont, m.titleFontSize);
        top += QFontMetrics(titleFont).height() + m.spacing;
    }

    const QFont valueFont = AntStyleBase::withPixelSize(widgetFont, m.valueFontSize);
    const int valueHeight = QFontMetrics(valueFont).height();

    return QRect(m.padding, top, widgetRect.width() - m.padding * 2, valueHeight);
}

} // namespace

AntStatisticStyle::AntStatisticStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntStatistic>();
}

void AntStatisticStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    installPaintFilter<AntStatistic>(widget);
}

void AntStatisticStyle::unpolish(QWidget* widget)
{
    removePaintFilter<AntStatistic>(widget);
    AntStyleBase::unpolish(widget);
}

void AntStatisticStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntStatistic*>(widget))
    {
        drawStatistic(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntStatisticStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntStatisticStyle::drawWidget(QWidget* widget, QPaintEvent* event)
{
    auto* stat = qobject_cast<AntStatistic*>(widget);
    if (!stat)
    {
        return false;
    }

    QStyleOption option;
    option.initFrom(stat);
    option.rect = stat->rect();
    QPainter painter(stat);
    drawPrimitive(QStyle::PE_Widget, &option, &painter, stat);

    return true;
}

void AntStatisticStyle::drawStatistic(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* stat = qobject_cast<const AntStatistic*>(widget);
    if (!stat || !painter || !option)
    {
        return;
    }

    const Metrics m = statisticMetrics();
    const auto& token = antTheme->tokens();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const QString title = stat->title();
    const QString prefix = stat->prefix();
    const QString suffix = stat->suffix();
    const QFont widgetFont = stat->font();

    if (!title.isEmpty())
    {
        const QFont titleFont = AntStyleBase::withPixelSize(widgetFont, m.titleFontSize);
        painter->setFont(titleFont);
        painter->setPen(token.colorTextSecondary);
        painter->drawText(statisticTitleRect(stat, option->rect, widgetFont), Qt::AlignLeft | Qt::AlignVCenter, title);
    }

    const QRect vr = statisticValueRect(stat, option->rect, widgetFont);

    const QFont valueFont = AntStyleBase::withPixelSize(widgetFont, m.valueFontSize, QFont::Normal);
    QFontMetrics valueFm(valueFont);

    const QString formatted = stat->formattedValue();
    int contentWidth = valueFm.horizontalAdvance(formatted);

    if (!prefix.isEmpty())
    {
        const QFont prefixFont = AntStyleBase::withPixelSize(widgetFont, m.prefixFontSize, QFont::Normal);
        QFontMetrics prefixFm(prefixFont);
        contentWidth += prefixFm.horizontalAdvance(prefix) + m.spacing;
    }
    if (!suffix.isEmpty())
    {
        const QFont suffixFont = AntStyleBase::withPixelSize(widgetFont, m.suffixFontSize, QFont::Normal);
        QFontMetrics suffixFm(suffixFont);
        contentWidth += m.spacing + suffixFm.horizontalAdvance(suffix);
    }

    int x = vr.left();
    const int centerY = vr.top() + vr.height() / 2;

    if (!prefix.isEmpty())
    {
        const QFont prefixFont = AntStyleBase::withPixelSize(widgetFont, m.prefixFontSize, QFont::Normal);
        painter->setFont(prefixFont);
        painter->setPen(token.colorText);
        QFontMetrics prefixFm(prefixFont);
        const int prefixBaseline = centerY + prefixFm.ascent() / 2 - prefixFm.descent() / 2;
        painter->drawText(x, prefixBaseline, prefix);
        x += prefixFm.horizontalAdvance(prefix) + m.spacing;
    }

    painter->setFont(valueFont);
    painter->setPen(token.colorText);
    const int valueBaseline = centerY + valueFm.ascent() / 2 - valueFm.descent() / 2;
    painter->drawText(x, valueBaseline, formatted);
    x += valueFm.horizontalAdvance(formatted);

    if (!suffix.isEmpty())
    {
        x += m.spacing;
        const QFont suffixFont = AntStyleBase::withPixelSize(widgetFont, m.suffixFontSize, QFont::Normal);
        painter->setFont(suffixFont);
        painter->setPen(token.colorTextSecondary);
        QFontMetrics suffixFm(suffixFont);
        const int suffixBaseline = centerY + suffixFm.ascent() / 2 - suffixFm.descent() / 2;
        painter->drawText(x, suffixBaseline, suffix);
    }

    painter->restore();
}
