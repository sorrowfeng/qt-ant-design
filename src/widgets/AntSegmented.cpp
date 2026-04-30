#include "AntSegmented.h"

#include <QMouseEvent>
#include <QPainter>
#include <QFontMetrics>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QVariantAnimation>

#include <algorithm>

#include "core/AntTheme.h"
#include "styles/AntSegmentedStyle.h"

namespace
{
constexpr int kTrackPadding = 2;
constexpr int kIconSize = 14;
constexpr int kIconGap = 6;

int segmentedFontSize(Ant::Size size)
{
    const auto& token = antTheme->tokens();
    switch (size)
    {
    case Ant::Size::Small: return token.fontSizeSM;
    case Ant::Size::Large: return token.fontSizeLG;
    default: return token.fontSize;
    }
}

int segmentedHeight(Ant::Size size)
{
    const auto& token = antTheme->tokens();
    switch (size)
    {
    case Ant::Size::Small: return token.controlHeightSM;
    case Ant::Size::Large: return token.controlHeightLG;
    default: return token.controlHeight;
    }
}

int segmentedPaddingHorizontal(Ant::Size size)
{
    const auto& token = antTheme->tokens();
    return (size == Ant::Size::Small ? token.paddingXS : token.paddingSM) - token.lineWidth;
}

int segmentWidth(const AntSegmentedOption& option, const QFontMetrics& fm, Ant::Size size)
{
    const int textWidth = fm.horizontalAdvance(option.label);
    const int iconWidth = option.icon.isEmpty() ? 0 : kIconSize + (option.label.isEmpty() ? 0 : kIconGap);
    return textWidth + iconWidth + segmentedPaddingHorizontal(size) * 2;
}
}

AntSegmented::AntSegmented(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntSegmentedStyle(style()));
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
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
    setSizePolicy(m_block ? QSizePolicy::Expanding : QSizePolicy::Fixed, QSizePolicy::Fixed);
    updateGeometry();
    update();
    Q_EMIT blockChanged(m_block);
}

Ant::Size AntSegmented::segmentedSize() const { return m_size; }

void AntSegmented::setSegmentedSize(Ant::Size size)
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
    case Ant::Size::Small:
        h = token.controlHeightSM;
        fontSize = token.fontSizeSM;
        break;
    case Ant::Size::Large:
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
        QFont f = font();
        f.setPixelSize(fontSize);
        QFontMetrics fm(f);
        int maxWidth = h;
        for (const auto& opt : m_options)
        {
            maxWidth = std::max(maxWidth, segmentWidth(opt, fm, m_size));
        }
        return QSize(maxWidth + kTrackPadding * 2, n * h);
    }
    else
    {
        QFont f = font();
        f.setPixelSize(fontSize);
        QFontMetrics fm(f);
        int totalWidth = kTrackPadding * 2;
        for (const auto& opt : m_options)
        {
            totalWidth += segmentWidth(opt, fm, m_size);
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
    const int pressedIdx = (event->buttons() & Qt::LeftButton) ? idx : -1;
    if (m_hoveredIndex != idx || m_pressedIndex != pressedIdx)
    {
        m_hoveredIndex = idx;
        m_pressedIndex = pressedIdx;
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
            m_pressedIndex = idx;
            update();
        }
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntSegmented::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const int idx = segmentIndexAt(event->pos());
        const int pressedIdx = m_pressedIndex;
        m_pressedIndex = -1;
        if (idx == pressedIdx && idx >= 0 && idx < m_options.size() && !m_options[idx].disabled)
        {
            setValue(m_options[idx].value);
        }
        update();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void AntSegmented::leaveEvent(QEvent* event)
{
    m_hoveredIndex = -1;
    m_pressedIndex = -1;
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

    const QRectF track = QRectF(rect()).adjusted(kTrackPadding, kTrackPadding, -kTrackPadding, -kTrackPadding);

    if (m_vertical)
    {
        const qreal segH = track.height() / n;
        for (int i = 0; i < n; ++i)
        {
            rects.append(QRectF(track.x(), track.y() + i * segH, track.width(), segH));
        }
    }
    else
    {
        if (m_block)
        {
            const qreal segW = track.width() / n;
            for (int i = 0; i < n; ++i)
            {
                rects.append(QRectF(track.x() + i * segW, track.y(), segW, track.height()));
            }
        }
        else
        {
            QFont f = font();
            f.setPixelSize(segmentedFontSize(m_size));
            QFontMetrics fm(f);
            qreal x = track.x();
            for (const auto& opt : m_options)
            {
                const qreal segW = segmentWidth(opt, fm, m_size);
                rects.append(QRectF(x, track.y(), segW, track.height()));
                x += segW;
            }
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

int AntSegmented::pressedIndex() const
{
    return m_pressedIndex;
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
        m_thumbAnimation->setDuration(300);
        m_thumbAnimation->setEasingCurve(QEasingCurve::InOutCubic);
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
