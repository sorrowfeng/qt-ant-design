#include "AntWatermark.h"

#include <QPainter>

#include "core/AntTheme.h"
#include "styles/AntWatermarkStyle.h"

AntWatermark::AntWatermark(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntWatermarkStyle>(this);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

QStringList AntWatermark::content() const { return m_content; }

void AntWatermark::setContent(const QStringList& lines)
{
    if (m_content == lines) return;
    m_content = lines;
    update();
    Q_EMIT contentChanged(m_content);
}

void AntWatermark::setContent(const QString& text)
{
    setContent(text.split(QStringLiteral("\n")));
}

qreal AntWatermark::rotate() const { return m_rotate; }

void AntWatermark::setRotate(qreal degrees)
{
    if (qFuzzyCompare(m_rotate, degrees)) return;
    m_rotate = degrees;
    update();
    Q_EMIT rotateChanged(m_rotate);
}

AntWatermarkFont AntWatermark::watermarkFont() const { return m_font; }

void AntWatermark::setWatermarkFont(const AntWatermarkFont& f)
{
    if (m_font == f) return;
    m_font = f;
    update();
}

QSize AntWatermark::gap() const { return m_gap; }

void AntWatermark::setGap(const QSize& gap)
{
    if (m_gap == gap) return;
    m_gap = gap;
    update();
}

QPoint AntWatermark::offset() const { return m_offset; }

void AntWatermark::setOffset(const QPoint& offset)
{
    if (m_offset == offset) return;
    m_offset = offset;
    update();
}

QSize AntWatermark::sizeHint() const
{
    if (parentWidget())
        return parentWidget()->size();
    return QSize(200, 200);
}

QSize AntWatermark::minimumSizeHint() const
{
    return QSize(100, 100);
}

void AntWatermark::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}
