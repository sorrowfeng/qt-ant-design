#include "AntSwitch.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#include <algorithm>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"

AntSwitch::AntSwitch(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);

    m_progressAnimation = new QPropertyAnimation(this, "handleProgress", this);
    m_progressAnimation->setDuration(180);
    m_progressAnimation->setEasingCurve(QEasingCurve::InOutSine);

    m_stretchAnimation = new QPropertyAnimation(this, "handleStretch", this);
    m_stretchAnimation->setDuration(120);
    m_stretchAnimation->setEasingCurve(QEasingCurve::InOutSine);

    m_loadingTimer = new QTimer(this);
    connect(m_loadingTimer, &QTimer::timeout, this, [this]() {
        m_loadingAngle = (m_loadingAngle + 30) % 360;
        update();
    });

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometryFromState();
        update();
    });

    updateGeometryFromState();
}

bool AntSwitch::isChecked() const { return m_checked; }

void AntSwitch::setChecked(bool checked)
{
    if (m_checked == checked)
    {
        return;
    }

    m_checked = checked;
    animateToChecked(m_checked);
    Q_EMIT checkedChanged(m_checked);
    Q_EMIT toggled(m_checked);
}

Ant::SwitchSize AntSwitch::switchSize() const { return m_switchSize; }

void AntSwitch::setSwitchSize(Ant::SwitchSize size)
{
    if (m_switchSize == size)
    {
        return;
    }

    m_switchSize = size;
    updateGeometryFromState();
    update();
    Q_EMIT switchSizeChanged(m_switchSize);
}

bool AntSwitch::isLoading() const { return m_loading; }

void AntSwitch::setLoading(bool loading)
{
    if (m_loading == loading)
    {
        return;
    }

    m_loading = loading;
    m_loading ? m_loadingTimer->start(80) : m_loadingTimer->stop();
    update();
    Q_EMIT loadingChanged(m_loading);
}

QString AntSwitch::checkedText() const { return m_checkedText; }

void AntSwitch::setCheckedText(const QString& text)
{
    if (m_checkedText == text)
    {
        return;
    }

    m_checkedText = text;
    updateGeometryFromState();
    update();
    Q_EMIT checkedTextChanged(m_checkedText);
}

QString AntSwitch::uncheckedText() const { return m_uncheckedText; }

void AntSwitch::setUncheckedText(const QString& text)
{
    if (m_uncheckedText == text)
    {
        return;
    }

    m_uncheckedText = text;
    updateGeometryFromState();
    update();
    Q_EMIT uncheckedTextChanged(m_uncheckedText);
}

qreal AntSwitch::handleProgress() const { return m_handleProgress; }

void AntSwitch::setHandleProgress(qreal progress)
{
    m_handleProgress = std::clamp(progress, 0.0, 1.0);
    update();
}

qreal AntSwitch::handleStretch() const { return m_handleStretch; }

void AntSwitch::setHandleStretch(qreal stretch)
{
    m_handleStretch = std::clamp(stretch, 0.0, 1.0);
    update();
}

QSize AntSwitch::sizeHint() const
{
    const Metrics m = metrics();
    return QSize(m.trackMinWidth, m.trackHeight);
}

QSize AntSwitch::minimumSizeHint() const
{
    return sizeHint();
}

void AntSwitch::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    const auto& token = antTheme->tokens();
    const Metrics m = metrics();
    const QRectF track = trackRect();
    const QRectF handle = handleRect(m);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    painter.setPen(Qt::NoPen);
    painter.setBrush(trackColor());
    painter.drawRoundedRect(track, track.height() / 2.0, track.height() / 2.0);

    const QString label = m_checked ? m_checkedText : m_uncheckedText;
    if (!label.isEmpty() && m_switchSize == Ant::SwitchSize::Middle)
    {
        QRectF textRect = track.adjusted(m.trackPadding + m.innerMinMargin, 0, -(m.trackPadding + m.innerMinMargin), 0);
        if (m_checked)
        {
            textRect.setRight(handle.left() - 2);
        }
        else
        {
            textRect.setLeft(handle.right() + 2);
        }

        QFont f = painter.font();
        f.setPixelSize(m.fontSize);
        painter.setFont(f);
        QColor textColor = token.colorTextLightSolid;
        if (!isEnabled() || m_loading)
        {
            textColor.setAlphaF(0.9);
        }
        painter.setPen(textColor);
        painter.drawText(textRect, Qt::AlignCenter, label);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(token.colorTextLightSolid);
    painter.drawRoundedRect(handle, handle.height() / 2.0, handle.height() / 2.0);

    QColor shadow(0, 35, 11, 50);
    painter.setPen(QPen(shadow, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(handle.adjusted(0.5, 0.5, -0.5, -0.5), handle.height() / 2.0, handle.height() / 2.0);

    if (m_loading)
    {
        drawLoading(painter, handle.adjusted(4, 4, -4, -4));
    }

    if (hasFocus() && isEnabled() && !m_loading)
    {
        QColor focus = AntPalette::alpha(token.colorPrimary, 0.22);
        painter.setPen(QPen(focus, token.controlOutlineWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(track.adjusted(1, 1, -1, -1), track.height() / 2.0, track.height() / 2.0);
    }
}

void AntSwitch::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void AntSwitch::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    animateStretch(0.0);
    update();
    QWidget::leaveEvent(event);
}

void AntSwitch::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isEnabled() && !m_loading)
    {
        m_pressed = true;
        animateStretch(1.0);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntSwitch::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed)
    {
        m_pressed = false;
        animateStretch(0.0);
        if (rect().contains(event->pos()))
        {
            setChecked(!m_checked);
            Q_EMIT clicked(m_checked);
        }
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void AntSwitch::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        setCursor(isEnabled() && !m_loading ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
    QWidget::changeEvent(event);
}

void AntSwitch::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && isEnabled() && !m_loading)
    {
        setChecked(!m_checked);
        Q_EMIT clicked(m_checked);
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

AntSwitch::Metrics AntSwitch::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    const qreal height = token.fontSize * token.lineHeight;
    if (m_switchSize == Ant::SwitchSize::Small)
    {
        m.trackHeight = token.controlHeight / 2;
        m.trackPadding = 2;
        m.handleSize = m.trackHeight - m.trackPadding * 2;
        m.trackMinWidth = m.handleSize * 2 + m.trackPadding * 2;
        m.fontSize = token.fontSizeSM;
        m.innerMinMargin = m.handleSize / 2;
        m.innerMaxMargin = m.handleSize + m.trackPadding * 3;
    }
    else
    {
        m.trackHeight = static_cast<int>(std::round(height));
        m.trackPadding = 2;
        m.handleSize = m.trackHeight - m.trackPadding * 2;
        m.trackMinWidth = m.handleSize * 2 + m.trackPadding * 4;
        m.fontSize = token.fontSizeSM;
        m.innerMinMargin = m.handleSize / 2;
        m.innerMaxMargin = m.handleSize + m.trackPadding * 3;
    }

    QFont f = font();
    f.setPixelSize(m.fontSize);
    QFontMetrics fm(f);
    const int labelWidth = std::max(fm.horizontalAdvance(m_checkedText), fm.horizontalAdvance(m_uncheckedText));
    if (labelWidth > 0)
    {
        m.trackMinWidth = std::max(m.trackMinWidth, labelWidth + m.handleSize + m.trackPadding * 8);
    }

    return m;
}

QRectF AntSwitch::trackRect() const
{
    const Metrics m = metrics();
    return QRectF((width() - m.trackMinWidth) / 2.0, (height() - m.trackHeight) / 2.0, m.trackMinWidth, m.trackHeight);
}

QRectF AntSwitch::handleRect(const Metrics& m) const
{
    const QRectF track = trackRect();
    const qreal left = track.left() + m.trackPadding;
    const qreal right = track.right() - m.trackPadding - m.handleSize;
    qreal x = left + (right - left) * m_handleProgress;
    qreal width = m.handleSize + m.trackPadding * 2 * m_handleStretch;

    if (m_checked)
    {
        x -= m.trackPadding * 2 * m_handleStretch;
    }

    return QRectF(x, track.top() + m.trackPadding, width, m.handleSize);
}

QColor AntSwitch::trackColor() const
{
    const auto& token = antTheme->tokens();
    QColor color = m_checked ? token.colorPrimary : token.colorTextDisabled;
    if (m_checked && m_hovered && isEnabled() && !m_loading)
    {
        color = token.colorPrimaryHover;
    }
    else if (!m_checked && m_hovered && isEnabled() && !m_loading)
    {
        color = token.colorTextTertiary;
    }

    if (!isEnabled() || m_loading)
    {
        color.setAlphaF(0.65);
    }

    return color;
}

void AntSwitch::animateToChecked(bool checked)
{
    m_progressAnimation->stop();
    m_progressAnimation->setStartValue(m_handleProgress);
    m_progressAnimation->setEndValue(checked ? 1.0 : 0.0);
    m_progressAnimation->start();
}

void AntSwitch::animateStretch(qreal endValue)
{
    m_stretchAnimation->stop();
    m_stretchAnimation->setStartValue(m_handleStretch);
    m_stretchAnimation->setEndValue(endValue);
    m_stretchAnimation->start();
}

void AntSwitch::updateGeometryFromState()
{
    const Metrics m = metrics();
    setMinimumSize(m.trackMinWidth, m.trackHeight);
    updateGeometry();
}

void AntSwitch::drawLoading(QPainter& painter, const QRectF& rect) const
{
    painter.save();
    QColor color = m_checked ? antTheme->tokens().colorPrimary : antTheme->tokens().colorTextTertiary;
    painter.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(rect, m_loadingAngle * 16, 270 * 16);
    painter.restore();
}
