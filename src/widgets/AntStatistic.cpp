#include "AntStatistic.h"

#include <QDateTime>
#include <QFontMetrics>
#include <QPainter>
#include <QResizeEvent>
#include <QTimer>

#include "core/AntTheme.h"
#include "styles/AntStatisticStyle.h"

AntStatistic::AntStatistic(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntStatisticStyle(style()));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
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

bool AntStatistic::isCountdownMode() const { return m_countdownMode; }

void AntStatistic::setCountdownMode(bool countdown)
{
    if (m_countdownMode == countdown)
    {
        return;
    }
    m_countdownMode = countdown;
    if (m_countdownMode)
    {
        if (!m_countdownTimer)
        {
            m_countdownTimer = new QTimer(this);
            connect(m_countdownTimer, &QTimer::timeout, this, [this]() {
                const double now = QDateTime::currentMSecsSinceEpoch() / 1000.0;
                const double remaining = m_value - now;
                if (remaining <= 0)
                {
                    m_countdownTimer->stop();
                    Q_EMIT countdownFinished();
                }
                update();
            });
        }
        m_countdownTimer->start(1000);
    }
    else
    {
        if (m_countdownTimer)
        {
            m_countdownTimer->stop();
        }
    }
    update();
    Q_EMIT countdownModeChanged(m_countdownMode);
}

QString AntStatistic::countdownFormat() const { return m_countdownFormat; }

void AntStatistic::setCountdownFormat(const QString& format)
{
    if (m_countdownFormat == format)
    {
        return;
    }
    m_countdownFormat = format;
    if (m_countdownMode)
    {
        update();
    }
    Q_EMIT countdownFormatChanged(m_countdownFormat);
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
}

void AntStatistic::resizeEvent(QResizeEvent* event)
{
    syncValueWidgetGeometry();
    QWidget::resizeEvent(event);
}

AntStatistic::Metrics AntStatistic::metrics() const
{
    Metrics m;
    m.padding = 0;
    m.prefixFontSize = m.valueFontSize;
    m.suffixFontSize = m.valueFontSize;
    return m;
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
    // Countdown mode: format remaining time
    if (m_countdownMode)
    {
        const double now = QDateTime::currentMSecsSinceEpoch() / 1000.0;
        const double remaining = qMax(0.0, m_value - now);
        const int totalSecs = static_cast<int>(remaining);

        const int days = totalSecs / 86400;
        const bool hasDayToken = m_countdownFormat.contains(QStringLiteral("DD"));
        const int hours = hasDayToken ? (totalSecs % 86400) / 3600 : totalSecs / 3600;
        const int minutes = (totalSecs % 3600) / 60;
        const int seconds = totalSecs % 60;

        QString result = m_countdownFormat;
        result.replace(QStringLiteral("DD"), QString::number(days));
        result.replace(QStringLiteral("HH"), QString::number(hours).rightJustified(2, QChar('0')));
        result.replace(QStringLiteral("mm"), QString::number(minutes).rightJustified(2, QChar('0')));
        result.replace(QStringLiteral("ss"), QString::number(seconds).rightJustified(2, QChar('0')));
        return result;
    }

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
