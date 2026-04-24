#include "AntStatistic.h"

#include <QFontMetrics>
#include <QPainter>
#include <QResizeEvent>

#include "core/AntTheme.h"

AntStatistic::AntStatistic(QWidget* parent)
    : QWidget(parent)
{
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
        update();
    });
}

AntStatistic::AntStatistic(const QString& title, QWidget* parent)
    : AntStatistic(parent)
{
    m_title = title;
}

QString AntStatistic::title() const { return m_title; }

void AntStatistic::setTitle(const QString& title)
{
    if (m_title == title)
    {
        return;
    }
    m_title = title;
    updateGeometry();
    update();
    Q_EMIT titleChanged(m_title);
}

double AntStatistic::value() const { return m_value; }

void AntStatistic::setValue(double value)
{
    if (qFuzzyCompare(m_value, value))
    {
        return;
    }
    m_value = value;
    update();
    Q_EMIT valueChanged(m_value);
}

int AntStatistic::precision() const { return m_precision; }

void AntStatistic::setPrecision(int precision)
{
    const int clamped = qMax(0, precision);
    if (m_precision == clamped)
    {
        return;
    }
    m_precision = clamped;
    updateGeometry();
    update();
    Q_EMIT precisionChanged(m_precision);
}

QString AntStatistic::groupSeparator() const { return m_groupSeparator; }

void AntStatistic::setGroupSeparator(const QString& separator)
{
    if (m_groupSeparator == separator)
    {
        return;
    }
    m_groupSeparator = separator;
    update();
    Q_EMIT groupSeparatorChanged(m_groupSeparator);
}

QString AntStatistic::prefix() const { return m_prefix; }

void AntStatistic::setPrefix(const QString& prefix)
{
    if (m_prefix == prefix)
    {
        return;
    }
    m_prefix = prefix;
    updateGeometry();
    update();
    Q_EMIT prefixChanged(m_prefix);
}

QString AntStatistic::suffix() const { return m_suffix; }

void AntStatistic::setSuffix(const QString& suffix)
{
    if (m_suffix == suffix)
    {
        return;
    }
    m_suffix = suffix;
    updateGeometry();
    update();
    Q_EMIT suffixChanged(m_suffix);
}

void AntStatistic::setValueWidget(QWidget* widget)
{
    if (m_valueWidget == widget)
    {
        return;
    }
    if (m_valueWidget)
    {
        m_valueWidget->setParent(nullptr);
    }
    m_valueWidget = widget;
    if (m_valueWidget)
    {
        m_valueWidget->setParent(this);
        m_valueWidget->show();
    }
    syncValueWidgetGeometry();
    updateGeometry();
    update();
}

QSize AntStatistic::sizeHint() const
{
    const Metrics m = metrics();
    const auto& token = antTheme->tokens();

    QFont titleFont = font();
    titleFont.setPixelSize(m.titleFontSize);
    QFontMetrics titleFm(titleFont);

    QFont valueFont = font();
    valueFont.setPixelSize(m.valueFontSize);
    QFontMetrics valueFm(valueFont);

    int width = m.padding * 2;

    const QString formatted = formattedValue();
    int valueWidth = valueFm.horizontalAdvance(formatted);
    if (!m_prefix.isEmpty())
    {
        QFont prefixFont = font();
        prefixFont.setPixelSize(m.prefixFontSize);
        valueWidth += QFontMetrics(prefixFont).horizontalAdvance(m_prefix) + m.spacing;
    }
    if (!m_suffix.isEmpty())
    {
        QFont suffixFont = font();
        suffixFont.setPixelSize(m.suffixFontSize);
        valueWidth += m.spacing + QFontMetrics(suffixFont).horizontalAdvance(m_suffix);
    }

    width += qMax(titleFm.horizontalAdvance(m_title), valueWidth);

    int height = m.padding;
    if (!m_title.isEmpty())
    {
        height += titleFm.height() + m.spacing;
    }
    height += valueFm.height();
    height += m.padding;

    return QSize(qMax(120, width), height);
}

QSize AntStatistic::minimumSizeHint() const
{
    return QSize(80, 56);
}

void AntStatistic::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    const Metrics m = metrics();
    const auto& token = antTheme->tokens();

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    if (!m_title.isEmpty())
    {
        QFont titleFont = painter.font();
        titleFont.setPixelSize(m.titleFontSize);
        painter.setFont(titleFont);
        painter.setPen(token.colorTextSecondary);
        painter.drawText(titleRect(), Qt::AlignLeft | Qt::AlignVCenter, m_title);
    }

    const QRect vr = valueRect();

    QFont valueFont = painter.font();
    valueFont.setPixelSize(m.valueFontSize);
    valueFont.setWeight(QFont::DemiBold);
    QFontMetrics valueFm(valueFont);

    const QString formatted = formattedValue();
    int contentWidth = valueFm.horizontalAdvance(formatted);

    if (!m_prefix.isEmpty())
    {
        QFont prefixFont = painter.font();
        prefixFont.setPixelSize(m.prefixFontSize);
        prefixFont.setWeight(QFont::DemiBold);
        QFontMetrics prefixFm(prefixFont);
        contentWidth += prefixFm.horizontalAdvance(m_prefix) + m.spacing;
    }
    if (!m_suffix.isEmpty())
    {
        QFont suffixFont = painter.font();
        suffixFont.setPixelSize(m.suffixFontSize);
        suffixFont.setWeight(QFont::Normal);
        QFontMetrics suffixFm(suffixFont);
        contentWidth += m.spacing + suffixFm.horizontalAdvance(m_suffix);
    }

    int x = vr.left();
    const int centerY = vr.top() + vr.height() / 2;

    if (!m_prefix.isEmpty())
    {
        QFont prefixFont = painter.font();
        prefixFont.setPixelSize(m.prefixFontSize);
        prefixFont.setWeight(QFont::DemiBold);
        painter.setFont(prefixFont);
        painter.setPen(token.colorText);
        QFontMetrics prefixFm(prefixFont);
        const int prefixBaseline = centerY + prefixFm.ascent() / 2 - prefixFm.descent() / 2;
        painter.drawText(x, prefixBaseline, m_prefix);
        x += prefixFm.horizontalAdvance(m_prefix) + m.spacing;
    }

    painter.setFont(valueFont);
    painter.setPen(token.colorText);
    const int valueBaseline = centerY + valueFm.ascent() / 2 - valueFm.descent() / 2;
    painter.drawText(x, valueBaseline, formatted);
    x += valueFm.horizontalAdvance(formatted);

    if (!m_suffix.isEmpty())
    {
        x += m.spacing;
        QFont suffixFont = painter.font();
        suffixFont.setPixelSize(m.suffixFontSize);
        suffixFont.setWeight(QFont::Normal);
        painter.setFont(suffixFont);
        painter.setPen(token.colorTextSecondary);
        QFontMetrics suffixFm(suffixFont);
        const int suffixBaseline = centerY + suffixFm.ascent() / 2 - suffixFm.descent() / 2;
        painter.drawText(x, suffixBaseline, m_suffix);
    }
}

void AntStatistic::resizeEvent(QResizeEvent* event)
{
    syncValueWidgetGeometry();
    QWidget::resizeEvent(event);
}

AntStatistic::Metrics AntStatistic::metrics() const
{
    return {};
}

QRect AntStatistic::titleRect() const
{
    const Metrics m = metrics();
    const auto& token = antTheme->tokens();

    QFont titleFont = font();
    titleFont.setPixelSize(m.titleFontSize);
    QFontMetrics titleFm(titleFont);

    return QRect(m.padding, m.padding, width() - m.padding * 2, titleFm.height());
}

QRect AntStatistic::valueRect() const
{
    const Metrics m = metrics();
    int top = m.padding;

    if (!m_title.isEmpty())
    {
        QFont titleFont = font();
        titleFont.setPixelSize(m.titleFontSize);
        top += QFontMetrics(titleFont).height() + m.spacing;
    }

    QFont valueFont = font();
    valueFont.setPixelSize(m.valueFontSize);
    const int valueHeight = QFontMetrics(valueFont).height();

    return QRect(m.padding, top, width() - m.padding * 2, valueHeight);
}

QString AntStatistic::formattedValue() const
{
    QString numStr;

    if (m_precision > 0)
    {
        numStr = QString::number(m_value, 'f', m_precision);
    }
    else
    {
        numStr = QString::number(static_cast<qlonglong>(m_value));
    }

    if (m_groupSeparator.isEmpty())
    {
        return numStr;
    }

    int dotIdx = numStr.indexOf('.');
    QString intPart = dotIdx >= 0 ? numStr.left(dotIdx) : numStr;
    QString fracPart = dotIdx >= 0 ? numStr.mid(dotIdx) : QString();

    bool negative = intPart.startsWith('-');
    if (negative)
    {
        intPart.remove(0, 1);
    }

    for (int i = intPart.size() - 3; i > 0; i -= 3)
    {
        intPart.insert(i, m_groupSeparator);
    }

    if (negative)
    {
        intPart.prepend('-');
    }

    return intPart + fracPart;
}

void AntStatistic::syncValueWidgetGeometry()
{
    if (m_valueWidget)
    {
        const QRect vr = valueRect();
        const QSize sz = m_valueWidget->sizeHint();
        m_valueWidget->setGeometry(QRect(vr.left(), vr.top(), sz.width(), sz.height()));
        m_valueWidget->show();
    }
}
