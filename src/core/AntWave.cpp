#include "AntWave.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QShowEvent>
#include <QVariantAnimation>

#include "AntTheme.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace
{
constexpr int kWaveExpand = 6;
constexpr int kWaveMargin = 12; // overlay padding so the glow has room to expand
constexpr int kWaveExpandDurationMs = 400;
constexpr int kWaveFadeDurationMs = 2000;
constexpr int kWaveQuickDurationMs = 300;

QColor defaultWaveColor(const QWidget* target)
{
    if (!target)
    {
        return antTheme->tokens().colorPrimary;
    }
    // Prefer foreground color if the widget has a meaningful palette; fall back
    // to theme primary.
    const QColor fg = target->palette().color(target->foregroundRole());
    if (fg.isValid() && fg.alpha() > 0)
    {
        return fg;
    }
    return antTheme->tokens().colorPrimary;
}

#if defined(Q_OS_WIN)
void makeWaveNativeInputTransparent(QWidget* widget)
{
    if (!widget)
    {
        return;
    }

    const WId id = widget->internalWinId();
    if (!id)
    {
        return;
    }

    const HWND hwnd = reinterpret_cast<HWND>(id);
    if (!hwnd)
    {
        return;
    }

    const LONG_PTR currentStyle = ::GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    const LONG_PTR nextStyle = currentStyle | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE;
    if (nextStyle != currentStyle)
    {
        ::SetWindowLongPtrW(hwnd, GWL_EXSTYLE, nextStyle);
        ::SetWindowPos(hwnd,
                       nullptr,
                       0,
                       0,
                       0,
                       0,
                       SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER |
                           SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }

    widget->setProperty("antWaveNativeClickThrough", true);
}
#endif
}

AntWave::AntWave(QWidget* target, const QRect& localRect, const QColor& color, int radius, bool quick)
    : QWidget(target ? target->parentWidget() : nullptr)
    , m_target(target)
    , m_localRect(localRect)
    , m_color(color.isValid() ? color : defaultWaveColor(target))
    , m_radius(radius)
    , m_quick(quick)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::NoFocus);
#if defined(Q_OS_WIN)
    setProperty("antWaveNativeClickThrough", false);
#endif

    if (target)
    {
        // Overlay covers localRect translated into parent coordinates, padded
        // for the glow.
        const QPoint topLeftInParent = target->pos() + m_localRect.topLeft();
        const QRect waveRect(topLeftInParent, m_localRect.size());
        setGeometry(waveRect.adjusted(-kWaveMargin, -kWaveMargin, kWaveMargin, kWaveMargin));
        stackUnder(target);
        raise();
    }
}

void AntWave::trigger(QWidget* target, const QColor& color, int radius, bool quick)
{
    if (!target)
    {
        return;
    }
    triggerRect(target, target->rect(), color, radius, quick);
}

void AntWave::triggerRect(QWidget* target, const QRect& localRect, const QColor& color, int radius, bool quick)
{
    if (!target || !target->parentWidget() || localRect.isEmpty())
    {
        return;
    }
    auto* wave = new AntWave(target, localRect, color, radius, quick);
    wave->show();
#if defined(Q_OS_WIN)
    makeWaveNativeInputTransparent(wave);
#endif

    auto* anim = new QVariantAnimation(wave);
    anim->setDuration(quick ? kWaveQuickDurationMs : kWaveFadeDurationMs);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::Linear);
    QObject::connect(anim, &QVariantAnimation::valueChanged, wave, [wave](const QVariant& v) {
        wave->tick(v.toReal());
    });
    QObject::connect(anim, &QVariantAnimation::finished, wave, &QWidget::close);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AntWave::tick(qreal t)
{
    m_progress = t;
    if (m_target)
    {
        const QPoint topLeftInParent = m_target->pos() + m_localRect.topLeft();
        const QRect waveRect(topLeftInParent, m_localRect.size());
        setGeometry(waveRect.adjusted(-kWaveMargin, -kWaveMargin, kWaveMargin, kWaveMargin));
    }
    update();
}

#if defined(Q_OS_WIN)
void AntWave::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    makeWaveNativeInputTransparent(this);
}

bool AntWave::nativeEvent(const QByteArray& eventType, void* message, AntNativeEventResult* result)
{
    if ((eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") && message)
    {
        auto* msg = static_cast<MSG*>(message);
        if (msg->message == WM_NCHITTEST)
        {
            *result = HTTRANSPARENT;
            setProperty("antWaveNativeClickThrough", true);
            return true;
        }
    }
    return QWidget::nativeEvent(eventType, message, result);
}
#endif

void AntWave::paintEvent(QPaintEvent*)
{
    if (!m_target)
    {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    const QEasingCurve spreadCurve(m_quick ? QEasingCurve::InOutCubic : QEasingCurve::OutCirc);
    const QEasingCurve opacityCurve(m_quick ? QEasingCurve::InOutCubic : QEasingCurve::OutCirc);
    const qreal spreadInput = m_quick
        ? m_progress
        : qMin<qreal>(1.0, m_progress * kWaveFadeDurationMs / kWaveExpandDurationMs);
    const qreal spread = kWaveExpand * spreadCurve.valueForProgress(spreadInput);
    const qreal opacity = 0.2 * (1.0 - opacityCurve.valueForProgress(m_progress));
    if (opacity <= 0.001)
    {
        return;
    }

    const QRectF inner(kWaveMargin, kWaveMargin, m_localRect.width(), m_localRect.height());
    const QRectF outer = inner.adjusted(-spread, -spread, spread, spread);

    QPainterPath outerPath;
    outerPath.addRoundedRect(outer, m_radius + spread, m_radius + spread);
    QPainterPath innerPath;
    innerPath.addRoundedRect(inner, m_radius, m_radius);
    QPainterPath ring = outerPath.subtracted(innerPath);

    QColor fill = m_color;
    fill.setAlphaF(opacity);
    painter.setPen(Qt::NoPen);
    painter.setBrush(fill);
    painter.drawPath(ring);
}
