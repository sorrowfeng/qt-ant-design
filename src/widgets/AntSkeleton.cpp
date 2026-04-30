#include "AntSkeleton.h"

#include <QLinearGradient>
#include <QPainter>
#include <QResizeEvent>
#include <QTimer>
#include <QVBoxLayout>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "styles/AntSkeletonStyle.h"

AntSkeleton::AntSkeleton(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntSkeletonStyle(style()));
    setAttribute(Qt::WA_Hover, false);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_shimmerOffset = (m_shimmerOffset + 12) % qMax(180, width() + 180);
        update();
    });

    updateTimerState();
}

bool AntSkeleton::isActive() const { return m_active; }

void AntSkeleton::setActive(bool active)
{
    if (m_active == active)
    {
        return;
    }
    m_active = active;
    updateTimerState();
    update();
    Q_EMIT activeChanged(m_active);
}

bool AntSkeleton::isLoading() const { return m_loading; }

void AntSkeleton::setLoading(bool loading)
{
    if (m_loading == loading)
    {
        return;
    }
    m_loading = loading;
    if (m_contentWidget)
    {
        m_contentWidget->setVisible(!m_loading);
    }
    updateTimerState();
    updateGeometry();
    update();
    Q_EMIT loadingChanged(m_loading);
}

bool AntSkeleton::isRound() const { return m_round; }

void AntSkeleton::setRound(bool round)
{
    if (m_round == round)
    {
        return;
    }
    m_round = round;
    update();
    Q_EMIT roundChanged(m_round);
}

bool AntSkeleton::avatarVisible() const { return m_avatarVisible; }

void AntSkeleton::setAvatarVisible(bool visible)
{
    if (m_avatarVisible == visible)
    {
        return;
    }
    m_avatarVisible = visible;
    updateGeometry();
    update();
    Q_EMIT avatarVisibleChanged(m_avatarVisible);
}

Ant::AvatarShape AntSkeleton::avatarShape() const { return m_avatarShape; }

void AntSkeleton::setAvatarShape(Ant::AvatarShape shape)
{
    if (m_avatarShape == shape)
    {
        return;
    }
    m_avatarShape = shape;
    update();
    Q_EMIT avatarShapeChanged(m_avatarShape);
}

bool AntSkeleton::titleVisible() const { return m_titleVisible; }

void AntSkeleton::setTitleVisible(bool visible)
{
    if (m_titleVisible == visible)
    {
        return;
    }
    m_titleVisible = visible;
    updateGeometry();
    update();
    Q_EMIT titleVisibleChanged(m_titleVisible);
}

bool AntSkeleton::paragraphVisible() const { return m_paragraphVisible; }

void AntSkeleton::setParagraphVisible(bool visible)
{
    if (m_paragraphVisible == visible)
    {
        return;
    }
    m_paragraphVisible = visible;
    updateGeometry();
    update();
    Q_EMIT paragraphVisibleChanged(m_paragraphVisible);
}

int AntSkeleton::paragraphRows() const { return m_paragraphRows; }

void AntSkeleton::setParagraphRows(int rows)
{
    rows = qMax(0, rows);
    if (m_paragraphRows == rows)
    {
        return;
    }
    m_paragraphRows = rows;
    updateGeometry();
    update();
    Q_EMIT paragraphRowsChanged(m_paragraphRows);
}

Ant::SkeletonElement AntSkeleton::element() const { return m_element; }

void AntSkeleton::setElement(Ant::SkeletonElement element)
{
    if (m_element == element)
    {
        return;
    }
    m_element = element;
    updateGeometry();
    update();
    Q_EMIT elementChanged(m_element);
}

QWidget* AntSkeleton::contentWidget() const
{
    return m_contentWidget.data();
}

void AntSkeleton::setContentWidget(QWidget* widget)
{
    if (m_contentWidget == widget)
    {
        return;
    }
    if (m_contentWidget)
    {
        m_contentWidget->hide();
        m_contentWidget->setParent(nullptr);
    }
    m_contentWidget = widget;
    if (m_contentWidget)
    {
        m_contentWidget->setParent(this);
        m_contentWidget->setVisible(!m_loading);
    }
    syncContentGeometry();
    updateGeometry();
    update();
}

void AntSkeleton::setTitleWidthRatio(qreal ratio)
{
    ratio = qBound(0.15, ratio, 1.0);
    if (qFuzzyCompare(m_titleWidthRatio, ratio))
    {
        return;
    }
    m_titleWidthRatio = ratio;
    update();
}

qreal AntSkeleton::titleWidthRatio() const
{
    return m_titleWidthRatio;
}

void AntSkeleton::setParagraphWidthRatios(const QList<qreal>& ratios)
{
    m_paragraphWidthRatios = ratios;
    update();
}

QList<qreal> AntSkeleton::paragraphWidthRatios() const
{
    return m_paragraphWidthRatios;
}

int AntSkeleton::shimmerOffset() const
{
    return m_shimmerOffset;
}

QSize AntSkeleton::sizeHint() const
{
    if (!m_loading && m_contentWidget)
    {
        return m_contentWidget->sizeHint();
    }

    // Element mode: return fixed sizes
    if (m_element != Ant::SkeletonElement::Default)
    {
        const Metrics m = metrics();
        switch (m_element)
        {
        case Ant::SkeletonElement::Button:
            return QSize(120, 32);
        case Ant::SkeletonElement::Avatar:
            return QSize(m.avatarSize, m.avatarSize);
        case Ant::SkeletonElement::Input:
            return QSize(200, 32);
        case Ant::SkeletonElement::Image:
            return QSize(200, 200);
        case Ant::SkeletonElement::Node:
            if (m_contentWidget)
                return m_contentWidget->sizeHint();
            return QSize(200, 100);
        default:
            break;
        }
    }

    const Metrics m = metrics();
    const auto& token = antTheme->tokens();
    int height = 0;

    if (m_avatarVisible)
    {
        height = qMax(height, m.avatarSize);
    }

    int textHeight = 0;
    if (m_titleVisible)
    {
        textHeight += m.titleHeight;
    }
    if (m_paragraphVisible && m_paragraphRows > 0)
    {
        if (textHeight > 0)
        {
            textHeight += token.marginSM;
        }
        textHeight += m_paragraphRows * m.paragraphHeight;
        textHeight += qMax(0, m_paragraphRows - 1) * m.rowSpacing;
    }

    if (m_avatarVisible)
    {
        height = qMax(height, textHeight);
    }
    else
    {
        height = textHeight;
    }

    return QSize(320, qMax(24, height));
}

QSize AntSkeleton::minimumSizeHint() const
{
    return QSize(180, 24);
}

void AntSkeleton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntSkeleton::resizeEvent(QResizeEvent* event)
{
    syncContentGeometry();
    QWidget::resizeEvent(event);
}

AntSkeleton::Metrics AntSkeleton::metrics() const
{
    Metrics m;
    const auto& token = antTheme->tokens();
    m.avatarSize = token.controlHeightLG;
    m.titleHeight = token.fontSizeLG;
    m.paragraphHeight = token.fontSizeSM + 2;
    m.rowSpacing = token.marginSM;
    m.columnGap = token.margin;
    m.radius = token.borderRadiusSM;
    return m;
}

QList<QRectF> AntSkeleton::placeholderRects() const
{
    QList<QRectF> rects;
    const Metrics m = metrics();

    // Element mode
    if (m_element != Ant::SkeletonElement::Default)
    {
        const QSize hint = sizeHint();
        const qreal w = qMin(static_cast<qreal>(width()), static_cast<qreal>(hint.width()));
        const qreal h = hint.height();
        switch (m_element)
        {
        case Ant::SkeletonElement::Button:
            rects.append(QRectF(0, 0, w, h));
            break;
        case Ant::SkeletonElement::Avatar:
            rects.append(QRectF(0, 0, m.avatarSize, m.avatarSize));
            break;
        case Ant::SkeletonElement::Input:
            rects.append(QRectF(0, 0, w, h));
            break;
        case Ant::SkeletonElement::Image:
        {
            const qreal sz = qMin(w, h);
            rects.append(QRectF((w - sz) / 2, 0, sz, sz));
            break;
        }
        case Ant::SkeletonElement::Node:
            rects.append(QRectF(0, 0, w, h));
            break;
        default:
            break;
        }
        return rects;
    }

    // Default paragraph mode
    int textLeft = 0;
    int textWidth = width();
    const int totalHeight = sizeHint().height();
    int top = 0;

    if (m_avatarVisible)
    {
        const int avatarY = qMax(0, (totalHeight - m.avatarSize) / 2);
        rects.append(QRectF(0, avatarY, m.avatarSize, m.avatarSize));
        textLeft = m.avatarSize + m.columnGap;
        textWidth = qMax(40, width() - textLeft);
    }

    if (m_titleVisible)
    {
        const int titleWidth = qMax(48, static_cast<int>(textWidth * m_titleWidthRatio));
        rects.append(QRectF(textLeft, top + m.titleTop, titleWidth, m.titleHeight));
        top += m.titleHeight;
    }

    if (m_paragraphVisible && m_paragraphRows > 0)
    {
        if (m_titleVisible)
        {
            top += antTheme->tokens().marginSM;
        }
        for (int i = 0; i < m_paragraphRows; ++i)
        {
            const int rowWidth = qMax(56, static_cast<int>(textWidth * rowWidthRatio(i)));
            rects.append(QRectF(textLeft, top, rowWidth, m.paragraphHeight));
            top += m.paragraphHeight + m.rowSpacing;
        }
    }

    return rects;
}

qreal AntSkeleton::rowWidthRatio(int rowIndex) const
{
    if (rowIndex >= 0 && rowIndex < m_paragraphWidthRatios.size())
    {
        return qBound(0.15, m_paragraphWidthRatios.at(rowIndex), 1.0);
    }
    if (rowIndex == m_paragraphRows - 1)
    {
        return 0.62;
    }
    return 1.0;
}

void AntSkeleton::syncContentGeometry()
{
    if (m_contentWidget)
    {
        m_contentWidget->setGeometry(rect());
    }
}

void AntSkeleton::updateTimerState()
{
    if (m_active && m_loading)
    {
        m_timer->start(40);
    }
    else
    {
        m_timer->stop();
    }
}
