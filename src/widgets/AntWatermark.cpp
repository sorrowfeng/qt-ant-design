#include "AntWatermark.h"

#include <QFontMetrics>
#include <QPainter>
#include <QtMath>

#include "core/AntTheme.h"
#include "styles/AntWatermarkStyle.h"

AntWatermark::AntWatermark(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntWatermarkStyle>(this);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateWatermarkPixmap();
        requestWatermarkUpdate(QStringLiteral("theme"));
    });
    syncWatermarkPerfCounters();
}

QStringList AntWatermark::content() const { return m_content; }

void AntWatermark::setContent(const QStringList& lines)
{
    if (m_content == lines) return;
    m_content = lines;
    invalidateWatermarkPixmap();
    requestWatermarkUpdate(QStringLiteral("content"));
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
    invalidateWatermarkPixmap();
    requestWatermarkUpdate(QStringLiteral("rotate"));
    Q_EMIT rotateChanged(m_rotate);
}

AntWatermarkFont AntWatermark::watermarkFont() const { return m_font; }

void AntWatermark::setWatermarkFont(const AntWatermarkFont& f)
{
    if (m_font == f) return;
    m_font = f;
    invalidateWatermarkPixmap();
    requestWatermarkUpdate(QStringLiteral("font"));
}

QSize AntWatermark::gap() const { return m_gap; }

void AntWatermark::setGap(const QSize& gap)
{
    if (m_gap == gap) return;
    m_gap = gap;
    invalidateWatermarkPixmap();
    requestWatermarkUpdate(QStringLiteral("gap"));
}

QPoint AntWatermark::offset() const { return m_offset; }

void AntWatermark::setOffset(const QPoint& offset)
{
    if (m_offset == offset) return;
    m_offset = offset;
    invalidateWatermarkPixmap();
    requestWatermarkUpdate(QStringLiteral("offset"));
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

QPixmap AntWatermark::watermarkPixmap(qreal devicePixelRatio) const
{
    const qreal dpr = qMax<qreal>(1.0, devicePixelRatio);
    const QColor textColor = resolvedTextColor();
    if (m_pixmapCache.valid &&
        m_pixmapCache.widgetSize == size() &&
        qFuzzyCompare(m_pixmapCache.devicePixelRatio, dpr) &&
        m_pixmapCache.themeMode == antTheme->themeMode() &&
        m_pixmapCache.content == m_content &&
        m_pixmapCache.font == m_font &&
        qFuzzyCompare(m_pixmapCache.rotate, m_rotate) &&
        m_pixmapCache.gap == m_gap &&
        m_pixmapCache.offset == m_offset &&
        m_pixmapCache.resolvedColor == textColor)
    {
        ++m_pixmapCacheHitCount;
        syncWatermarkPerfCounters();
        return m_pixmapCache.pixmap;
    }

    ++m_pixmapBuildCount;
    WatermarkPixmapCache cache;
    cache.valid = true;
    cache.widgetSize = size();
    cache.devicePixelRatio = dpr;
    cache.themeMode = antTheme->themeMode();
    cache.content = m_content;
    cache.font = m_font;
    cache.rotate = m_rotate;
    cache.gap = m_gap;
    cache.offset = m_offset;
    cache.resolvedColor = textColor;

    const QSize logicalSize = size().expandedTo(QSize(1, 1));
    cache.pixmap = QPixmap(QSize(qCeil(logicalSize.width() * dpr), qCeil(logicalSize.height() * dpr)));
    cache.pixmap.setDevicePixelRatio(dpr);
    cache.pixmap.fill(Qt::transparent);

    if (!m_content.isEmpty())
    {
        QPainter painter(&cache.pixmap);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        QFont font;
        if (!m_font.fontFamily.isEmpty())
        {
            font.setFamily(m_font.fontFamily);
        }
        font.setPixelSize(m_font.fontSize);
        font.setWeight(QFont::Weight(m_font.fontWeight));

        const QSize textSize = watermarkTextSize(&painter, font);
        if (!textSize.isEmpty())
        {
            const qreal tileW = qMax<qreal>(1.0, textSize.width() + m_gap.width());
            const qreal tileH = qMax<qreal>(1.0, textSize.height() + m_gap.height());

            const qreal radians = qDegreesToRadians(qAbs(m_rotate));
            const qreal expandedW = tileW * qCos(radians) + tileH * qSin(radians);
            const qreal expandedH = tileW * qSin(radians) + tileH * qCos(radians);
            const qreal margin = qMax(expandedW, expandedH);

            const QRectF bounds(QPointF(0, 0), QSizeF(logicalSize));
            const qreal startX = bounds.left() - margin;
            const qreal startY = bounds.top() - margin;
            const qreal endX = bounds.right() + margin;
            const qreal endY = bounds.bottom() + margin;

            for (qreal ty = startY; ty < endY; ty += tileH)
            {
                for (qreal tx = startX; tx < endX; tx += tileW)
                {
                    painter.save();
                    const QPointF tileCenter(tx + tileW / 2.0 + m_offset.x(),
                                             ty + tileH / 2.0 + m_offset.y());
                    painter.translate(tileCenter);
                    painter.rotate(m_rotate);
                    drawWatermarkPattern(&painter, QPointF(0, 0), font, textColor);
                    painter.restore();
                }
            }
        }
    }

    m_pixmapCache = cache;
    syncWatermarkPerfCounters();
    return m_pixmapCache.pixmap;
}

QSize AntWatermark::watermarkTextSize(QPainter* painter, const QFont& font) const
{
    if (!painter || m_content.isEmpty())
    {
        return QSize(0, 0);
    }

    painter->setFont(font);
    QFontMetrics fm(font);

    int maxWidth = 0;
    int totalHeight = 0;
    for (const QString& line : m_content)
    {
        maxWidth = qMax(maxWidth, fm.horizontalAdvance(line));
        totalHeight += fm.height() + 2;
    }
    return QSize(maxWidth, totalHeight);
}

void AntWatermark::drawWatermarkPattern(QPainter* painter,
                                        const QPointF& center,
                                        const QFont& font,
                                        const QColor& color) const
{
    if (!painter || m_content.isEmpty())
    {
        return;
    }

    painter->setFont(font);
    painter->setPen(color);
    QFontMetrics fm(font);

    const qreal lineHeight = fm.height() + 2;
    qreal y = center.y() - (m_content.size() * lineHeight) / 2.0;
    for (const QString& line : m_content)
    {
        const int textWidth = fm.horizontalAdvance(line);
        painter->drawText(QRectF(center.x() - textWidth / 2.0, y, textWidth, lineHeight),
                          Qt::AlignCenter,
                          line);
        y += lineHeight;
    }
}

QColor AntWatermark::resolvedTextColor() const
{
    QColor textColor = m_font.color;
    if (!textColor.isValid())
    {
        const auto& token = antTheme->tokens();
        textColor = token.colorText;
        textColor.setAlphaF(0.18);
    }
    return textColor;
}

void AntWatermark::invalidateWatermarkPixmap() const
{
    m_pixmapCache.valid = false;
}

void AntWatermark::requestWatermarkUpdate(const QString& mode)
{
    m_lastUpdateMode = mode;
    ++m_regionUpdateCount;
    syncWatermarkPerfCounters();
    update();
}

void AntWatermark::syncWatermarkPerfCounters() const
{
    auto* self = const_cast<AntWatermark*>(this);
    self->setProperty("antWatermarkPixmapBuildCount", m_pixmapBuildCount);
    self->setProperty("antWatermarkPixmapCacheHitCount", m_pixmapCacheHitCount);
    self->setProperty("antWatermarkRegionUpdateCount", m_regionUpdateCount);
    self->setProperty("antWatermarkLastUpdateMode", m_lastUpdateMode);
}
