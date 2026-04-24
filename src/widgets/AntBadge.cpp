#include "AntBadge.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QTimer>

#include <algorithm>

#include "core/AntTheme.h"
#include "styles/AntBadgeStyle.h"

AntBadge::AntBadge(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntBadgeStyle(style()));
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, [this]() {
        m_pulse = (m_pulse + 6) % 100;
        update();
    });

    updateAnimationState();
}

AntBadge::AntBadge(int count, QWidget* parent)
    : AntBadge(parent)
{
    m_count = count;
}

int AntBadge::count() const { return m_count; }

void AntBadge::setCount(int count)
{
    if (m_count == count)
    {
        return;
    }
    m_count = count;
    updateGeometry();
    update();
    Q_EMIT countChanged(m_count);
}

QString AntBadge::text() const { return m_text; }

void AntBadge::setText(const QString& text)
{
    if (m_text == text)
    {
        return;
    }
    m_text = text;
    updateGeometry();
    update();
    Q_EMIT textChanged(m_text);
}

QString AntBadge::color() const { return m_color; }

void AntBadge::setColor(const QString& color)
{
    if (m_color == color)
    {
        return;
    }
    m_color = color;
    update();
    Q_EMIT colorChanged(m_color);
}

bool AntBadge::isDot() const { return m_dot; }

void AntBadge::setDot(bool dot)
{
    if (m_dot == dot)
    {
        return;
    }
    m_dot = dot;
    updateGeometry();
    update();
    Q_EMIT dotChanged(m_dot);
}

bool AntBadge::showZero() const { return m_showZero; }

void AntBadge::setShowZero(bool showZero)
{
    if (m_showZero == showZero)
    {
        return;
    }
    m_showZero = showZero;
    update();
    Q_EMIT showZeroChanged(m_showZero);
}

int AntBadge::overflowCount() const { return m_overflowCount; }

void AntBadge::setOverflowCount(int overflowCount)
{
    overflowCount = std::max(0, overflowCount);
    if (m_overflowCount == overflowCount)
    {
        return;
    }
    m_overflowCount = overflowCount;
    updateGeometry();
    update();
    Q_EMIT overflowCountChanged(m_overflowCount);
}

QPoint AntBadge::offset() const { return m_offset; }

void AntBadge::setOffset(const QPoint& offset)
{
    if (m_offset == offset)
    {
        return;
    }
    m_offset = offset;
    updateGeometry();
    updateContentGeometry();
    update();
    Q_EMIT offsetChanged(m_offset);
}

Ant::BadgeSize AntBadge::badgeSize() const { return m_badgeSize; }

void AntBadge::setBadgeSize(Ant::BadgeSize size)
{
    if (m_badgeSize == size)
    {
        return;
    }
    m_badgeSize = size;
    updateGeometry();
    updateContentGeometry();
    update();
    Q_EMIT badgeSizeChanged(m_badgeSize);
}

Ant::BadgeStatus AntBadge::status() const { return m_status; }

void AntBadge::setStatus(Ant::BadgeStatus status)
{
    if (m_status == status)
    {
        return;
    }
    m_status = status;
    updateAnimationState();
    updateGeometry();
    update();
    Q_EMIT statusChanged(m_status);
}

QWidget* AntBadge::contentWidget() const { return m_contentWidget; }

void AntBadge::setContentWidget(QWidget* widget)
{
    if (m_contentWidget == widget)
    {
        return;
    }
    if (m_contentWidget)
    {
        m_contentWidget->removeEventFilter(this);
        m_contentWidget->setParent(nullptr);
    }
    m_contentWidget = widget;
    if (m_contentWidget)
    {
        m_contentWidget->setParent(this);
        m_contentWidget->show();
    }
    updateContentGeometry();
    updateGeometry();
    update();
}

QSize AntBadge::sizeHint() const
{
    if (isStatusMode() && !m_contentWidget)
    {
        const auto& token = antTheme->tokens();
        QFont f = font();
        f.setPixelSize(token.fontSize);
        const int textWidth = m_text.isEmpty() ? 0 : QFontMetrics(f).horizontalAdvance(m_text);
        return QSize(statusDotSize() + (m_text.isEmpty() ? 0 : token.marginXS + textWidth), token.controlHeightSM);
    }

    const int w = indicatorWidth();
    if (m_contentWidget)
    {
        const QSize content = m_contentWidget->sizeHint().expandedTo(m_contentWidget->minimumSizeHint());
        return QSize(content.width() + contentRightReserve(), content.height() + contentTopReserve());
    }
    return QSize(w + 2, indicatorHeight() + 2);
}

QSize AntBadge::minimumSizeHint() const
{
    if (m_contentWidget)
    {
        return m_contentWidget->minimumSizeHint() + QSize(contentRightReserve(), contentTopReserve());
    }
    return QSize(dotSize() + 2, dotSize() + 2);
}

void AntBadge::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntBadge::resizeEvent(QResizeEvent* event)
{
    updateContentGeometry();
    QWidget::resizeEvent(event);
}

void AntBadge::mouseMoveEvent(QMouseEvent* event)
{
    const bool hovered = indicatorRect().contains(event->pos());
    if (m_hovered != hovered)
    {
        m_hovered = hovered;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void AntBadge::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && shouldShowIndicator() && indicatorRect().contains(event->pos()))
    {
        Q_EMIT clicked();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntBadge::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
    QWidget::leaveEvent(event);
}

void AntBadge::showEvent(QShowEvent* event)
{
    updateAnimationState();
    QWidget::showEvent(event);
}

void AntBadge::hideEvent(QHideEvent* event)
{
    m_animationTimer->stop();
    QWidget::hideEvent(event);
}

int AntBadge::indicatorHeight() const
{
    return m_badgeSize == Ant::BadgeSize::Small ? antTheme->tokens().fontSize : 20;
}

int AntBadge::dotSize() const
{
    return std::max(6, antTheme->tokens().fontSizeSM / 2);
}

int AntBadge::statusDotSize() const
{
    return dotSize();
}

int AntBadge::indicatorWidth() const
{
    if (m_dot)
    {
        return dotSize();
    }
    const int h = indicatorHeight();
    QFont f = font();
    f.setPixelSize(m_badgeSize == Ant::BadgeSize::Small ? antTheme->tokens().fontSizeSM : antTheme->tokens().fontSizeSM);
    const int textWidth = QFontMetrics(f).horizontalAdvance(displayText());
    return std::max(h, textWidth + antTheme->tokens().paddingXS * 2);
}

int AntBadge::contentTopReserve() const
{
    return shouldShowIndicator() ? indicatorHeight() / 2 + 2 + std::max(0, -m_offset.y()) : 0;
}

int AntBadge::contentRightReserve() const
{
    return shouldShowIndicator() ? indicatorWidth() / 2 + 2 + std::max(0, m_offset.x()) : 0;
}

QString AntBadge::displayText() const
{
    if (m_count > m_overflowCount)
    {
        return QStringLiteral("%1+").arg(m_overflowCount);
    }
    return QString::number(m_count);
}

QRect AntBadge::contentRect() const
{
    if (!m_contentWidget)
    {
        return rect();
    }
    const QSize hint = m_contentWidget->sizeHint().expandedTo(m_contentWidget->minimumSizeHint());
    return QRect(0, contentTopReserve(), width() - contentRightReserve(), height() - contentTopReserve()).adjusted(0, 0, 0, 0).intersected(QRect(0, contentTopReserve(), hint.width(), hint.height()));
}

QRectF AntBadge::indicatorRect() const
{
    const int w = indicatorWidth();
    const int h = m_dot ? dotSize() : indicatorHeight();
    if (!m_contentWidget)
    {
        return QRectF((width() - w) / 2.0, (height() - h) / 2.0, w, h);
    }
    const QRect content = contentRect();
    return QRectF(content.right() + 1 - w / 2.0 + m_offset.x(),
                  content.top() - h / 2.0 + m_offset.y(),
                  w,
                  h);
}

QRectF AntBadge::standaloneStatusDotRect() const
{
    const int d = statusDotSize();
    return QRectF(1, (height() - d) / 2.0, d, d);
}

QColor AntBadge::badgeColor() const
{
    const QColor custom(m_color);
    if (custom.isValid())
    {
        return custom;
    }
    if (m_color.compare(QStringLiteral("success"), Qt::CaseInsensitive) == 0)
    {
        return antTheme->tokens().colorSuccess;
    }
    if (m_color.compare(QStringLiteral("warning"), Qt::CaseInsensitive) == 0)
    {
        return antTheme->tokens().colorWarning;
    }
    if (m_color.compare(QStringLiteral("processing"), Qt::CaseInsensitive) == 0 || m_color.compare(QStringLiteral("blue"), Qt::CaseInsensitive) == 0)
    {
        return antTheme->tokens().colorPrimary;
    }
    return antTheme->tokens().colorError;
}

QColor AntBadge::statusColor() const
{
    const QColor custom(m_color);
    if (custom.isValid())
    {
        return custom;
    }
    const auto& token = antTheme->tokens();
    switch (m_status)
    {
    case Ant::BadgeStatus::Success:
        return token.colorSuccess;
    case Ant::BadgeStatus::Processing:
        return token.colorPrimary;
    case Ant::BadgeStatus::Default:
        return token.colorTextPlaceholder;
    case Ant::BadgeStatus::Error:
        return token.colorError;
    case Ant::BadgeStatus::Warning:
        return token.colorWarning;
    case Ant::BadgeStatus::None:
        return token.colorError;
    }
    return token.colorError;
}

bool AntBadge::shouldShowIndicator() const
{
    if (isStatusMode())
    {
        return true;
    }
    if (m_dot)
    {
        return true;
    }
    return m_count != 0 || m_showZero;
}

bool AntBadge::isStatusMode() const
{
    return m_status != Ant::BadgeStatus::None;
}

void AntBadge::updateContentGeometry()
{
    if (!m_contentWidget)
    {
        return;
    }
    m_contentWidget->setGeometry(contentRect());
}

void AntBadge::updateAnimationState()
{
    const bool animate = isVisible() && isEnabled() && m_status == Ant::BadgeStatus::Processing;
    if (animate && !m_animationTimer->isActive())
    {
        m_animationTimer->start(40);
    }
    else if (!animate)
    {
        m_animationTimer->stop();
    }
}

void AntBadge::drawIndicator(QPainter& painter)
{
    const auto& token = antTheme->tokens();
    const QRectF r = indicatorRect();
    QColor fill = isEnabled() ? badgeColor() : token.colorTextDisabled;
    if (m_hovered)
    {
        fill = antTheme->hoverColor(fill);
    }

    painter.setPen(QPen(token.colorBgContainer, token.lineWidth));
    painter.setBrush(fill);
    if (m_dot)
    {
        painter.drawEllipse(r);
        return;
    }

    painter.drawRoundedRect(r, r.height() / 2.0, r.height() / 2.0);

    QFont f = painter.font();
    f.setPixelSize(token.fontSizeSM);
    f.setWeight(QFont::Normal);
    painter.setFont(f);
    painter.setPen(token.colorTextLightSolid);
    painter.drawText(r.adjusted(token.paddingXXS, 0, -token.paddingXXS, 0), Qt::AlignCenter, displayText());
}

void AntBadge::drawStatus(QPainter& painter)
{
    const auto& token = antTheme->tokens();
    const QRectF dot = standaloneStatusDotRect();
    const QColor color = isEnabled() ? statusColor() : token.colorTextDisabled;

    if (m_status == Ant::BadgeStatus::Processing)
    {
        QColor ring = color;
        ring.setAlphaF(std::max(0.0, 0.45 * (1.0 - m_pulse / 100.0)));
        const qreal scale = 1.0 + 1.4 * (m_pulse / 100.0);
        QRectF ringRect(dot.center().x() - dot.width() * scale / 2.0,
                        dot.center().y() - dot.height() * scale / 2.0,
                        dot.width() * scale,
                        dot.height() * scale);
        painter.setPen(QPen(ring, token.lineWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(ringRect);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(dot);

    if (!m_text.isEmpty())
    {
        QFont f = painter.font();
        f.setPixelSize(token.fontSize);
        painter.setFont(f);
        painter.setPen(isEnabled() ? token.colorText : token.colorTextDisabled);
        const QRectF textRect(dot.right() + token.marginXS, 0, width() - dot.right() - token.marginXS, height());
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_text);
    }
}
