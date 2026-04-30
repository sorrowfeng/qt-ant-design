#include "AntProgress.h"

#include <QHideEvent>
#include <QPainter>
#include <QShowEvent>
#include <QTimer>

#include <algorithm>

#include "core/AntTheme.h"
#include "styles/AntIconPainter.h"
#include "styles/AntProgressStyle.h"
#include "styles/AntPalette.h"

namespace
{
bool progressHasStatusIcon(Ant::ProgressStatus status, int percent)
{
    return status == Ant::ProgressStatus::Success ||
           status == Ant::ProgressStatus::Exception ||
           percent >= 100;
}

QColor progressInfoColor(Ant::ProgressStatus status, int percent)
{
    const auto& token = antTheme->tokens();
    if (status == Ant::ProgressStatus::Success || percent >= 100)
    {
        return token.colorSuccess;
    }
    if (status == Ant::ProgressStatus::Exception)
    {
        return token.colorError;
    }
    return token.colorTextSecondary;
}

void drawProgressStatusIcon(QPainter& painter, const QRectF& rect, Ant::ProgressStatus status, int percent)
{
    const QColor color = progressInfoColor(status, percent);
    const Ant::IconType iconType = status == Ant::ProgressStatus::Exception
        ? Ant::IconType::CloseCircle
        : Ant::IconType::CheckCircle;
    AntIconPainter::drawIcon(painter,
                             iconType,
                             rect,
                             color,
                             Ant::IconTheme::Filled,
                             antTheme->tokens().colorTextLightSolid);
}
} // namespace

AntProgress::AntProgress(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntProgressStyle>(this);
    m_activeTimer = new QTimer(this);
    connect(m_activeTimer, &QTimer::timeout, this, [this]() {
        m_activeOffset = (m_activeOffset + 6) % 120;
        update();
    });

    updateAnimationState();
}

int AntProgress::percent() const { return m_percent; }

void AntProgress::setPercent(int percent)
{
    percent = std::clamp(percent, 0, 100);
    if (m_percent == percent)
    {
        return;
    }
    m_percent = percent;
    update();
    Q_EMIT percentChanged(m_percent);
}

Ant::ProgressType AntProgress::progressType() const { return m_progressType; }

void AntProgress::setProgressType(Ant::ProgressType type)
{
    if (m_progressType == type)
    {
        return;
    }
    m_progressType = type;
    updateAnimationState();
    updateGeometry();
    update();
    Q_EMIT progressTypeChanged(m_progressType);
}

Ant::ProgressStatus AntProgress::status() const { return m_status; }

void AntProgress::setStatus(Ant::ProgressStatus status)
{
    if (m_status == status)
    {
        return;
    }
    m_status = status;
    updateAnimationState();
    update();
    Q_EMIT statusChanged(m_status);
}

bool AntProgress::showInfo() const { return m_showInfo; }

void AntProgress::setShowInfo(bool showInfo)
{
    if (m_showInfo == showInfo)
    {
        return;
    }
    m_showInfo = showInfo;
    updateGeometry();
    update();
    Q_EMIT showInfoChanged(m_showInfo);
}

int AntProgress::strokeWidth() const { return m_strokeWidth; }

void AntProgress::setStrokeWidth(int width)
{
    width = std::clamp(width, 2, 24);
    if (m_strokeWidth == width)
    {
        return;
    }
    m_strokeWidth = width;
    updateGeometry();
    update();
    Q_EMIT strokeWidthChanged(m_strokeWidth);
}

int AntProgress::circleSize() const { return m_circleSize; }

void AntProgress::setCircleSize(int size)
{
    size = std::clamp(size, 48, 240);
    if (m_circleSize == size)
    {
        return;
    }
    m_circleSize = size;
    updateGeometry();
    update();
    Q_EMIT circleSizeChanged(m_circleSize);
}

QSize AntProgress::sizeHint() const
{
    if (m_progressType == Ant::ProgressType::Line)
    {
        return QSize(260, std::max(24, m_strokeWidth + 12));
    }
    return QSize(m_circleSize, m_circleSize);
}

QSize AntProgress::minimumSizeHint() const
{
    if (m_progressType == Ant::ProgressType::Line)
    {
        return QSize(96, std::max(20, m_strokeWidth + 8));
    }
    return QSize(48, 48);
}

void AntProgress::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntProgress::showEvent(QShowEvent* event)
{
    updateAnimationState();
    QWidget::showEvent(event);
}

void AntProgress::hideEvent(QHideEvent* event)
{
    m_activeTimer->stop();
    QWidget::hideEvent(event);
}

void AntProgress::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        update();
    }
    QWidget::changeEvent(event);
}

QColor AntProgress::progressColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorTextDisabled;
    }
    if (m_status == Ant::ProgressStatus::Success || m_percent >= 100)
    {
        return token.colorSuccess;
    }
    if (m_status == Ant::ProgressStatus::Exception)
    {
        return token.colorError;
    }
    return token.colorPrimary;
}

QColor AntProgress::railColor() const
{
    const auto& token = antTheme->tokens();
    return isEnabled() ? token.colorFillQuaternary : token.colorBgContainerDisabled;
}

QString AntProgress::infoText() const
{
    if (m_status == Ant::ProgressStatus::Success || m_percent >= 100)
    {
        return QString();
    }
    if (m_status == Ant::ProgressStatus::Exception)
    {
        return QString();
    }
    return QStringLiteral("%1%").arg(m_percent);
}

void AntProgress::updateAnimationState()
{
    const bool shouldAnimate = isVisible() && isEnabled() && m_progressType == Ant::ProgressType::Line && m_status == Ant::ProgressStatus::Active && m_percent < 100;
    if (shouldAnimate && !m_activeTimer->isActive())
    {
        m_activeTimer->start(40);
    }
    else if (!shouldAnimate)
    {
        m_activeTimer->stop();
    }
}

void AntProgress::drawLineProgress(QPainter& painter) const
{
    const auto& token = antTheme->tokens();
    const int infoWidth = m_showInfo ? 48 : 0;
    QRectF bar(0, (height() - m_strokeWidth) / 2.0, width() - infoWidth - 8, m_strokeWidth);
    if (!m_showInfo)
    {
        bar.setWidth(width());
    }
    bar = bar.adjusted(1, 0, -1, 0);

    painter.setPen(Qt::NoPen);
    painter.setBrush(railColor());
    painter.drawRoundedRect(bar, m_strokeWidth / 2.0, m_strokeWidth / 2.0);

    QRectF filled = bar;
    filled.setWidth(bar.width() * m_percent / 100.0);
    painter.setBrush(progressColor());
    painter.drawRoundedRect(filled, m_strokeWidth / 2.0, m_strokeWidth / 2.0);

    if (m_status == Ant::ProgressStatus::Active && m_percent > 0 && m_percent < 100)
    {
        QLinearGradient shine(filled.left() + m_activeOffset - 90, 0, filled.left() + m_activeOffset, 0);
        shine.setColorAt(0.0, QColor(255, 255, 255, 0));
        shine.setColorAt(0.5, QColor(255, 255, 255, 110));
        shine.setColorAt(1.0, QColor(255, 255, 255, 0));
        painter.setBrush(shine);
        painter.drawRoundedRect(filled, m_strokeWidth / 2.0, m_strokeWidth / 2.0);
    }

    if (m_showInfo)
    {
        const QRectF infoRect(width() - infoWidth, 0, infoWidth, height());
        if (progressHasStatusIcon(m_status, m_percent))
        {
            const qreal mark = 12.0;
            drawProgressStatusIcon(painter,
                                   QRectF(infoRect.center().x() - mark / 2.0,
                                          infoRect.center().y() - mark / 2.0,
                                          mark,
                                          mark),
                                   m_status,
                                   m_percent);
        }
        else
        {
            QFont f = painter.font();
            f.setPixelSize(token.fontSizeSM);
            f.setWeight(QFont::Normal);
            painter.setFont(f);
            painter.setPen(progressInfoColor(m_status, m_percent));
            painter.drawText(infoRect, Qt::AlignCenter, infoText());
        }
    }
}

void AntProgress::drawCircleProgress(QPainter& painter) const
{
    const auto& token = antTheme->tokens();
    const qreal size = std::min(width(), height());
    const qreal lineWidth = std::max<qreal>(2.0, m_strokeWidth);
    const QRectF arc((width() - size) / 2.0 + lineWidth,
                     (height() - size) / 2.0 + lineWidth,
                     size - lineWidth * 2,
                     size - lineWidth * 2);

    const int startAngle = m_progressType == Ant::ProgressType::Dashboard ? 225 * 16 : 90 * 16;
    const int fullSpan = m_progressType == Ant::ProgressType::Dashboard ? -270 * 16 : -360 * 16;
    const int progressSpan = fullSpan * m_percent / 100;

    painter.setPen(QPen(railColor(), lineWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(arc, startAngle, fullSpan);
    painter.setPen(QPen(progressColor(), lineWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(arc, startAngle, progressSpan);

    if (m_showInfo)
    {
        if (progressHasStatusIcon(m_status, m_percent))
        {
            const qreal mark = std::max<qreal>(18.0, size / 3.6);
            const QPointF c = rect().center();
            drawProgressStatusIcon(painter, QRectF(c.x() - mark / 2.0, c.y() - mark / 2.0, mark, mark), m_status, m_percent);
        }
        else
        {
            QFont f = painter.font();
            f.setPixelSize(std::max(12, static_cast<int>(size / 7)));
            f.setWeight(QFont::DemiBold);
            painter.setFont(f);
            painter.setPen(token.colorText);
            painter.drawText(rect(), Qt::AlignCenter, infoText());
        }
    }
}
