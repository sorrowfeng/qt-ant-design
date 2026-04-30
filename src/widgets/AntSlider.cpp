#include "AntSlider.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>

#include <algorithm>
#include <cmath>

#include "../styles/AntSliderStyle.h"
#include "core/AntTheme.h"

namespace
{
class SliderValueBubble : public QWidget
{
public:
    explicit SliderValueBubble(QWidget* parent)
        : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
    {
        setObjectName(QStringLiteral("antSliderValueBubble"));
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_ShowWithoutActivating, true);
    }

    void setText(const QString& text)
    {
        if (m_text == text)
        {
            return;
        }
        m_text = text;
        adjustSize();
        update();
    }

    QSize sizeHint() const override
    {
        QFont f = font();
        f.setPixelSize(antTheme->tokens().fontSizeSM);
        const QFontMetrics fm(f);
        return QSize(qMax(32, fm.horizontalAdvance(m_text) + 16), fm.height() + 16);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)

        const auto& token = antTheme->tokens();
        const QRectF bubble = rect().adjusted(0.5, 0.5, -0.5, -6.5);
        const qreal arrowCenter = rect().center().x();
        QPainterPath path;
        path.addRoundedRect(bubble, token.borderRadiusSM, token.borderRadiusSM);
        path.moveTo(arrowCenter - 5, bubble.bottom() - 1);
        path.lineTo(arrowCenter + 5, bubble.bottom() - 1);
        path.lineTo(arrowCenter, rect().bottom() - 1);
        path.closeSubpath();

        QColor bg = antTheme->themeMode() == Ant::ThemeMode::Dark ? QColor("#424242") : QColor("#262626");
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(bg);
        painter.drawPath(path);

        QFont textFont = painter.font();
        textFont.setPixelSize(token.fontSizeSM);
        painter.setFont(textFont);
        painter.setPen(token.colorTextLightSolid);
        painter.drawText(bubble.adjusted(8, 2, -8, -1), Qt::AlignCenter, m_text);
    }

private:
    QString m_text;
};
} // namespace

AntSlider::AntSlider(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_handleAnimation = new QPropertyAnimation(this, "handleScale", this);
    m_handleAnimation->setDuration(140);
    m_handleAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_focusAnimation = new QPropertyAnimation(this, "focusProgress", this);
    m_focusAnimation->setDuration(140);
    m_focusAnimation->setEasingCurve(QEasingCurve::OutCubic);

    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        updateGeometry();
        update();
    });

    auto* sliderStyle = new AntSliderStyle(style());
    sliderStyle->setParent(this);
    setStyle(sliderStyle);

    updateCursor();
}

AntSlider::AntSlider(Qt::Orientation orientation, QWidget* parent)
    : AntSlider(parent)
{
    m_orientation = orientation;
}

int AntSlider::minimum() const { return m_minimum; }

void AntSlider::setMinimum(int minimum)
{
    setRange(minimum, m_maximum);
}

int AntSlider::maximum() const { return m_maximum; }

void AntSlider::setMaximum(int maximum)
{
    setRange(m_minimum, maximum);
}

void AntSlider::setRange(int minimum, int maximum)
{
    if (minimum > maximum)
    {
        std::swap(minimum, maximum);
    }

    if (m_minimum == minimum && m_maximum == maximum)
    {
        return;
    }

    m_minimum = minimum;
    m_maximum = maximum;
    setValue(normalizeValue(m_value));
    setRangeValues(m_rangeStart, m_rangeEnd);
    update();
    Q_EMIT rangeChanged(m_minimum, m_maximum);
}

int AntSlider::value() const { return m_value; }

void AntSlider::setValue(int value)
{
    const int normalized = normalizeValue(value);
    if (m_value == normalized)
    {
        return;
    }

    m_value = normalized;
    update();
    Q_EMIT valueChanged(m_value);
}

int AntSlider::singleStep() const { return m_singleStep; }

void AntSlider::setSingleStep(int step)
{
    step = std::max(1, step);
    if (m_singleStep == step)
    {
        return;
    }

    m_singleStep = step;
    setValue(m_value);
    update();
    Q_EMIT singleStepChanged(m_singleStep);
}

Qt::Orientation AntSlider::orientation() const { return m_orientation; }

void AntSlider::setOrientation(Qt::Orientation orientation)
{
    if (m_orientation == orientation)
    {
        return;
    }

    m_orientation = orientation;
    updateGeometry();
    update();
    Q_EMIT orientationChanged(m_orientation);
}

bool AntSlider::isReverse() const { return m_reverse; }

void AntSlider::setReverse(bool reverse)
{
    if (m_reverse == reverse)
    {
        return;
    }

    m_reverse = reverse;
    update();
    Q_EMIT reverseChanged(m_reverse);
}

bool AntSlider::dots() const { return m_dots; }

void AntSlider::setDots(bool dots)
{
    if (m_dots == dots)
    {
        return;
    }

    m_dots = dots;
    update();
    Q_EMIT dotsChanged(m_dots);
}

bool AntSlider::included() const { return m_included; }

void AntSlider::setIncluded(bool included)
{
    if (m_included == included)
    {
        return;
    }

    m_included = included;
    update();
    Q_EMIT includedChanged(m_included);
}

bool AntSlider::keyboard() const { return m_keyboard; }

void AntSlider::setKeyboard(bool enabled)
{
    if (m_keyboard == enabled)
    {
        return;
    }

    m_keyboard = enabled;
    Q_EMIT keyboardChanged(m_keyboard);
}

bool AntSlider::isRangeMode() const { return m_rangeMode; }

void AntSlider::setRangeMode(bool rangeMode)
{
    if (m_rangeMode == rangeMode)
    {
        return;
    }
    m_rangeMode = rangeMode;
    setRangeValues(m_rangeStart, m_rangeEnd);
    update();
    Q_EMIT rangeModeChanged(m_rangeMode);
}

int AntSlider::rangeStart() const { return m_rangeStart; }

int AntSlider::rangeEnd() const { return m_rangeEnd; }

void AntSlider::setRangeValues(int start, int end)
{
    start = normalizeValue(start);
    end = normalizeValue(end);
    if (start > end)
    {
        std::swap(start, end);
    }
    if (m_rangeStart == start && m_rangeEnd == end)
    {
        return;
    }
    m_rangeStart = start;
    m_rangeEnd = end;
    update();
    Q_EMIT rangeValuesChanged(m_rangeStart, m_rangeEnd);
}

QMap<int, QString> AntSlider::marks() const { return m_marks; }

void AntSlider::setMarks(const QMap<int, QString>& marks)
{
    if (m_marks == marks)
    {
        return;
    }
    m_marks = marks;
    updateGeometry();
    update();
    Q_EMIT marksChanged();
}

qreal AntSlider::handleScale() const { return m_handleScale; }

void AntSlider::setHandleScale(qreal scale)
{
    m_handleScale = std::clamp(scale, 1.0, 1.2);
    update();
}

qreal AntSlider::focusProgress() const { return m_focusProgress; }

void AntSlider::setFocusProgress(qreal progress)
{
    m_focusProgress = std::clamp(progress, 0.0, 1.0);
    update();
}

bool AntSlider::isHoveredState() const { return m_hovered; }

bool AntSlider::isPressedState() const { return m_pressed; }

QSize AntSlider::sizeHint() const
{
    const Metrics m = metrics();
    if (m_orientation == Qt::Vertical)
    {
        return QSize(m.controlSize * 3, 180);
    }
    return QSize(180, m_marks.isEmpty() ? m.controlSize * 3 : 46);
}

QSize AntSlider::minimumSizeHint() const
{
    const Metrics m = metrics();
    if (m_orientation == Qt::Vertical)
    {
        return QSize(m.controlSize * 3, 96);
    }
    return QSize(96, m.controlSize * 3);
}

void AntSlider::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    if (isEnabled())
    {
        animateHandle(1.2);
    }
    update();
    QWidget::enterEvent(event);
}

void AntSlider::leaveEvent(QEvent* event)
{
    m_hovered = false;
    if (!m_pressed && !hasFocus())
    {
        animateHandle(1.0);
    }
    update();
    QWidget::leaveEvent(event);
}

void AntSlider::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isEnabled())
    {
        m_pressed = true;
        setFocus(Qt::MouseFocusReason);
        animateHandle(1.2);
        if (m_rangeMode)
        {
            const Metrics m = metrics();
            const qreal startDistance = QLineF(event->position(), handleRectForValue(m, m_rangeStart).center()).length();
            const qreal endDistance = QLineF(event->position(), handleRectForValue(m, m_rangeEnd).center()).length();
            m_activeRangeHandle = startDistance <= endDistance ? 0 : 1;
        }
        setValueFromUser(valueFromPosition(event->position()), false);
        updateValueBubble();
        Q_EMIT sliderPressed();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntSlider::mouseMoveEvent(QMouseEvent* event)
{
    if (m_pressed && isEnabled())
    {
        setValueFromUser(valueFromPosition(event->position()), false);
        updateValueBubble();
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void AntSlider::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed)
    {
        m_pressed = false;
        setValueFromUser(valueFromPosition(event->position()), true);
        if (!m_hovered && !hasFocus())
        {
            animateHandle(1.0);
        }
        hideValueBubble();
        Q_EMIT sliderReleased();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void AntSlider::keyPressEvent(QKeyEvent* event)
{
    if (!m_keyboard || !isEnabled())
    {
        QWidget::keyPressEvent(event);
        return;
    }

    const int step = std::max(1, m_singleStep);
    int delta = 0;
    bool absolute = false;
    int target = m_value;

    switch (event->key())
    {
    case Qt::Key_Left:
    case Qt::Key_Down:
        delta = -step;
        break;
    case Qt::Key_Right:
    case Qt::Key_Up:
        delta = step;
        break;
    case Qt::Key_PageDown:
        delta = -step * 10;
        break;
    case Qt::Key_PageUp:
        delta = step * 10;
        break;
    case Qt::Key_Home:
        target = m_minimum;
        absolute = true;
        break;
    case Qt::Key_End:
        target = m_maximum;
        absolute = true;
        break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }

    if (!absolute)
    {
        const bool invert = (m_orientation == Qt::Horizontal && m_reverse) || (m_orientation == Qt::Vertical && !m_reverse);
        target = m_value + (invert ? -delta : delta);
    }

    setValueFromUser(target, true);
    event->accept();
}

void AntSlider::focusInEvent(QFocusEvent* event)
{
    animateFocus(isEnabled() ? 1.0 : 0.0);
    if (isEnabled())
    {
        animateHandle(1.2);
    }
    QWidget::focusInEvent(event);
}

void AntSlider::focusOutEvent(QFocusEvent* event)
{
    animateFocus(0.0);
    if (!m_hovered && !m_pressed)
    {
        animateHandle(1.0);
    }
    QWidget::focusOutEvent(event);
}

void AntSlider::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        updateCursor();
        if (!isEnabled())
        {
            hideValueBubble();
        }
        update();
    }
    QWidget::changeEvent(event);
}

AntSlider::Metrics AntSlider::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    m.controlSize = token.controlHeightLG / 4;
    m.railSize = 4;
    m.handleSize = m.controlSize;
    m.handleSizeHover = token.controlHeightSM / 2;
    m.dotSize = 8;
    m.margin = token.marginXS;
    return m;
}

QRectF AntSlider::grooveRect(const Metrics& metrics) const
{
    const qreal halfHandle = metrics.handleSizeHover / 2.0 + 2.0;
    if (m_orientation == Qt::Vertical)
    {
        const qreal h = std::max<qreal>(0.0, height() - halfHandle * 2.0);
        return QRectF((width() - metrics.railSize) / 2.0, halfHandle, metrics.railSize, h);
    }

    const qreal w = std::max<qreal>(0.0, width() - halfHandle * 2.0);
    return QRectF(halfHandle, (height() - metrics.railSize) / 2.0, w, metrics.railSize);
}

QRectF AntSlider::trackRect(const Metrics& metrics) const
{
    const QRectF groove = grooveRect(metrics);
    if (m_rangeMode)
    {
        const qreal startRatio = ratioForValue(m_rangeStart);
        const qreal endRatio = ratioForValue(m_rangeEnd);
        if (m_orientation == Qt::Vertical)
        {
            const qreal top = groove.bottom() - groove.height() * std::max(startRatio, endRatio);
            const qreal bottom = groove.bottom() - groove.height() * std::min(startRatio, endRatio);
            return QRectF(groove.left(), top, groove.width(), bottom - top);
        }
        const qreal left = groove.left() + groove.width() * std::min(startRatio, endRatio);
        const qreal right = groove.left() + groove.width() * std::max(startRatio, endRatio);
        return QRectF(left, groove.top(), right - left, groove.height());
    }

    const qreal ratio = valueRatio();

    if (m_orientation == Qt::Vertical)
    {
        const qreal handleY = groove.bottom() - groove.height() * ratio;
        if (m_reverse)
        {
            return QRectF(groove.left(), groove.top(), groove.width(), handleY - groove.top());
        }
        return QRectF(groove.left(), handleY, groove.width(), groove.bottom() - handleY);
    }

    const qreal handleX = groove.left() + groove.width() * ratio;
    if (m_reverse)
    {
        return QRectF(handleX, groove.top(), groove.right() - handleX, groove.height());
    }
    return QRectF(groove.left(), groove.top(), handleX - groove.left(), groove.height());
}

QRectF AntSlider::handleRect(const Metrics& metrics) const
{
    return handleRectForValue(metrics, m_value);
}

QRectF AntSlider::handleRectForValue(const Metrics& metrics, int value) const
{
    const QRectF groove = grooveRect(metrics);
    const qreal size = metrics.handleSize * m_handleScale;
    QPointF center;

    if (m_orientation == Qt::Vertical)
    {
        center = QPointF(groove.center().x(), groove.bottom() - groove.height() * ratioForValue(value));
    }
    else
    {
        center = QPointF(groove.left() + groove.width() * ratioForValue(value), groove.center().y());
    }

    return QRectF(center.x() - size / 2.0, center.y() - size / 2.0, size, size);
}

qreal AntSlider::valueRatio() const
{
    return ratioForValue(m_value);
}

qreal AntSlider::ratioForValue(int value) const
{
    if (m_maximum == m_minimum)
    {
        return 0.0;
    }

    qreal ratio = (value - m_minimum) / static_cast<qreal>(m_maximum - m_minimum);
    if (m_reverse)
    {
        ratio = 1.0 - ratio;
    }
    return std::clamp(ratio, 0.0, 1.0);
}

int AntSlider::valueFromPosition(const QPointF& pos) const
{
    const Metrics m = metrics();
    const QRectF groove = grooveRect(m);
    qreal ratio = 0.0;

    if (m_orientation == Qt::Vertical)
    {
        ratio = (groove.bottom() - pos.y()) / std::max<qreal>(1.0, groove.height());
    }
    else
    {
        ratio = (pos.x() - groove.left()) / std::max<qreal>(1.0, groove.width());
    }

    ratio = std::clamp(ratio, 0.0, 1.0);
    if (m_reverse)
    {
        ratio = 1.0 - ratio;
    }

    const qreal raw = m_minimum + ratio * (m_maximum - m_minimum);
    return steppedValue(static_cast<int>(std::round(raw)));
}

int AntSlider::normalizeValue(int value) const
{
    return std::clamp(steppedValue(value), m_minimum, m_maximum);
}

int AntSlider::steppedValue(int value) const
{
    if (m_singleStep <= 1)
    {
        return std::clamp(value, m_minimum, m_maximum);
    }

    const int offset = value - m_minimum;
    const int steps = static_cast<int>(std::round(offset / static_cast<double>(m_singleStep)));
    return std::clamp(m_minimum + steps * m_singleStep, m_minimum, m_maximum);
}

void AntSlider::setValueFromUser(int value, bool finalChange)
{
    if (m_rangeMode)
    {
        const int oldStart = m_rangeStart;
        const int oldEnd = m_rangeEnd;
        if (m_activeRangeHandle == 0)
        {
            setRangeValues(std::min(value, m_rangeEnd), m_rangeEnd);
        }
        else
        {
            setRangeValues(m_rangeStart, std::max(value, m_rangeStart));
        }
        const int current = m_activeRangeHandle == 0 ? m_rangeStart : m_rangeEnd;
        if (oldStart != m_rangeStart || oldEnd != m_rangeEnd)
        {
            Q_EMIT sliderMoved(current);
        }
        if (finalChange)
        {
            Q_EMIT changeComplete(current);
        }
        return;
    }

    const int oldValue = m_value;
    setValue(value);
    if (oldValue != m_value)
    {
        Q_EMIT sliderMoved(m_value);
    }
    if (finalChange)
    {
        Q_EMIT changeComplete(m_value);
    }
}

void AntSlider::animateHandle(qreal scale)
{
    m_handleAnimation->stop();
    m_handleAnimation->setStartValue(m_handleScale);
    m_handleAnimation->setEndValue(scale);
    m_handleAnimation->start();
}

void AntSlider::animateFocus(qreal progress)
{
    m_focusAnimation->stop();
    m_focusAnimation->setStartValue(m_focusProgress);
    m_focusAnimation->setEndValue(progress);
    m_focusAnimation->start();
}

int AntSlider::activeDisplayValue() const
{
    if (!m_rangeMode)
    {
        return m_value;
    }
    return m_activeRangeHandle == 0 ? m_rangeStart : m_rangeEnd;
}

QPointF AntSlider::activeHandleCenter() const
{
    const Metrics m = metrics();
    return handleRectForValue(m, activeDisplayValue()).center();
}

void AntSlider::updateValueBubble()
{
    if (!m_pressed || !isEnabled())
    {
        return;
    }
    if (!m_valueBubble)
    {
        m_valueBubble = new SliderValueBubble(this);
    }

    auto* bubble = static_cast<SliderValueBubble*>(m_valueBubble);
    bubble->setText(QString::number(activeDisplayValue()));
    bubble->adjustSize();

    const QPoint globalCenter = mapToGlobal(activeHandleCenter().toPoint());
    QPoint topLeft;
    if (m_orientation == Qt::Vertical)
    {
        topLeft = QPoint(globalCenter.x() + 12, globalCenter.y() - bubble->height() / 2);
    }
    else
    {
        topLeft = QPoint(globalCenter.x() - bubble->width() / 2, globalCenter.y() - bubble->height() - 10);
    }

    if (QScreen* screen = this->screen())
    {
        const QRect available = screen->availableGeometry();
        topLeft.setX(qBound(available.left() + 4, topLeft.x(), available.right() - bubble->width() - 4));
        topLeft.setY(qBound(available.top() + 4, topLeft.y(), available.bottom() - bubble->height() - 4));
    }

    bubble->move(topLeft);
    if (!bubble->isVisible())
    {
        bubble->show();
    }
}

void AntSlider::hideValueBubble()
{
    if (m_valueBubble)
    {
        m_valueBubble->hide();
    }
}

void AntSlider::updateCursor()
{
    setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
}
