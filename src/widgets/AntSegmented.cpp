#include "AntSegmented.h"

#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QVariantAnimation>

#include "core/AntTheme.h"
#include "styles/AntSegmentedStyle.h"

AntSegmented::AntSegmented(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntSegmentedStyle(style()));
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void AntSegmented::setOptions(const QVector<AntSegmentedOption>& options)
{
    m_options = options;
    if (!m_value.isEmpty())
    {
        int idx = -1;
        for (int i = 0; i < m_options.size(); ++i)
        {
            if (m_options[i].value == m_value)
            {
                idx = i;
                break;
            }
        }
        if (idx < 0 && !m_options.isEmpty())
        {
            m_value = m_options[0].value;
            idx = 0;
        }
        m_thumbPos = static_cast<qreal>(idx);
    }
    else if (!m_options.isEmpty())
    {
        m_value = m_options[0].value;
        m_thumbPos = 0;
    }
    updateGeometry();
    update();
}

QVector<AntSegmentedOption> AntSegmented::options() const { return m_options; }

QString AntSegmented::value() const { return m_value; }

void AntSegmented::setValue(const QString& value)
{
    for (int i = 0; i < m_options.size(); ++i)
    {
        if (m_options[i].value == value && !m_options[i].disabled)
        {
            if (m_value == value) return;
            m_value = value;
            startThumbAnimation(i);
            update();
            Q_EMIT valueChanged(m_value);
            return;
        }
    }
}

bool AntSegmented::isBlock() const { return m_block; }

void AntSegmented::setBlock(bool block)
{
    if (m_block == block) return;
    m_block = block;
    updateGeometry();
    update();
    Q_EMIT blockChanged(m_block);
}

Ant::SegmentedSize AntSegmented::segmentedSize() const { return m_size; }

void AntSegmented::setSegmentedSize(Ant::SegmentedSize size)
{
    if (m_size == size) return;
    m_size = size;
    updateGeometry();
    update();
    Q_EMIT segmentedSizeChanged(m_size);
}

bool AntSegmented::isVertical() const { return m_vertical; }

void AntSegmented::setVertical(bool vertical)
{
    if (m_vertical == vertical) return;
    m_vertical = vertical;
    updateGeometry();
    update();
    Q_EMIT verticalChanged(m_vertical);
}

Ant::SegmentedShape AntSegmented::shape() const { return m_shape; }

void AntSegmented::setShape(Ant::SegmentedShape shape)
{
    if (m_shape == shape) return;
    m_shape = shape;
    update();
    Q_EMIT shapeChanged(m_shape);
}

QSize AntSegmented::sizeHint() const
{
    const auto& token = antTheme->tokens();
    int h;
    int fontSize;
    switch (m_size)
    {
    case Ant::SegmentedSize::Small:
        h = token.controlHeightSM;
        fontSize = token.fontSizeSM;
        break;
    case Ant::SegmentedSize::Large:
        h = token.controlHeightLG;
        fontSize = token.fontSizeLG;
        break;
    default:
        h = token.controlHeight;
        fontSize = token.fontSize;
        break;
    }

    if (m_vertical)
    {
        const int n = m_options.size();
        return QSize(h + 16, n * h);
    }
    else
    {
        QFont f = font();
        f.setPixelSize(fontSize);
        QFontMetrics fm(f);
        int totalWidth = 0;
        for (const auto& opt : m_options)
        {
            totalWidth += fm.horizontalAdvance(opt.label) + token.paddingSM * 2;
        }
        return QSize(totalWidth, h);
    }
}

QSize AntSegmented::minimumSizeHint() const
{
    return sizeHint();
}

void AntSegmented::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntSegmented::mouseMoveEvent(QMouseEvent* event)
{
    const int idx = segmentIndexAt(event->pos());
    if (m_hoveredIndex != idx)
    {
        m_hoveredIndex = idx;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void AntSegmented::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const int idx = segmentIndexAt(event->pos());
        if (idx >= 0 && idx < m_options.size() && !m_options[idx].disabled)
        {
            setValue(m_options[idx].value);
        }
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntSegmented::leaveEvent(QEvent* event)
{
    m_hoveredIndex = -1;
    update();
    QWidget::leaveEvent(event);
}

void AntSegmented::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

QVector<QRectF> AntSegmented::segmentRects() const
{
    QVector<QRectF> rects;
    const int n = m_options.size();
    if (n == 0) return rects;

    if (m_vertical)
    {
        const qreal segH = static_cast<qreal>(height()) / n;
        for (int i = 0; i < n; ++i)
        {
            rects.append(QRectF(0, i * segH, width(), segH));
        }
    }
    else
    {
        const qreal segW = static_cast<qreal>(width()) / n;
        for (int i = 0; i < n; ++i)
        {
            rects.append(QRectF(i * segW, 0, segW, height()));
        }
    }
    return rects;
}

int AntSegmented::segmentIndexAt(const QPoint& pos) const
{
    const auto rects = segmentRects();
    for (int i = 0; i < rects.size(); ++i)
    {
        if (rects[i].contains(pos)) return i;
    }
    return -1;
}

int AntSegmented::selectedIndex() const
{
    for (int i = 0; i < m_options.size(); ++i)
    {
        if (m_options[i].value == m_value) return i;
    }
    return -1;
}

int AntSegmented::hoveredIndex() const
{
    return m_hoveredIndex;
}

qreal AntSegmented::thumbPosition() const
{
    return m_thumbPos;
}

void AntSegmented::startThumbAnimation(int newIndex)
{
    if (!m_thumbAnimation)
    {
        m_thumbAnimation = new QVariantAnimation(this);
        m_thumbAnimation->setDuration(200);
        m_thumbAnimation->setEasingCurve(QEasingCurve::OutCubic);
        connect(m_thumbAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
            m_thumbPos = v.toReal();
            update();
        });
    }
    m_thumbAnimation->stop();
    m_thumbAnimation->setStartValue(m_thumbPos);
    m_thumbAnimation->setEndValue(static_cast<qreal>(newIndex));
    m_thumbAnimation->start();
}
