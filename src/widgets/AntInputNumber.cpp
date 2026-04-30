#include "AntInputNumber.h"

#include <QEasingCurve>
#include <QEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QStyleOptionSpinBox>

#include "../styles/AntInputNumberStyle.h"
#include "core/AntTheme.h"
#include "styles/AntIconPainter.h"

namespace
{
int handlerWidthForInput(const AntInputNumber* input)
{
    const auto& token = antTheme->tokens();
    int handlerWidth = token.fontSize + token.paddingSM;
    if (!input)
    {
        return handlerWidth;
    }

    switch (input->inputSize())
    {
    case Ant::Size::Large:
        handlerWidth = token.fontSizeLG + token.padding;
        break;
    case Ant::Size::Small:
        handlerWidth = token.fontSize + token.paddingXS + token.paddingXXS;
        break;
    case Ant::Size::Middle:
        break;
    }
    return handlerWidth;
}

class InputNumberControlsOverlay : public QWidget
{
public:
    explicit InputNumberControlsOverlay(AntInputNumber* input)
        : QWidget(input)
        , m_input(input)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TranslucentBackground);
        setAutoFillBackground(false);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        if (!m_input || !m_input->controlsVisible() || !m_input->isEnabled() || m_input->controlsProgress() <= 0.01)
        {
            return;
        }

        const auto& token = antTheme->tokens();
        const qreal progress = m_input->controlsProgress();
        const int panelWidth = handlerWidthForInput(m_input);
        const QRect panelRect(width() - panelWidth, 0, panelWidth, height());
        const QRect upRect(panelRect.left(), 1, panelRect.width(), height() / 2 - 1);
        const QRect downRect(panelRect.left(), height() / 2, panelRect.width(), height() / 2 - 1);
        const bool upHovered = m_input->activeSubControl() == QStyle::SC_SpinBoxUp;
        const bool downHovered = m_input->activeSubControl() == QStyle::SC_SpinBoxDown;

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter.setOpacity(progress);
        painter.setClipRect(rect());
        painter.translate((1.0 - progress) * panelWidth, 0);

        painter.setPen(QPen(token.colorSplit, token.lineWidth));
        painter.drawLine(QPointF(panelRect.left() + 0.5, 3),
                         QPointF(panelRect.left() + 0.5, height() - 3));
        painter.drawLine(QPointF(panelRect.left() + 2, height() / 2.0),
                         QPointF(panelRect.right() - 2, height() / 2.0));

        const QColor hoverColor = m_input->isStepPressed() ? token.colorFillQuaternary : token.colorFillTertiary;
        painter.setPen(Qt::NoPen);
        if (upHovered)
        {
            painter.fillRect(upRect.adjusted(1, 0, -1, 0), hoverColor);
        }
        if (downHovered)
        {
            painter.fillRect(downRect.adjusted(1, 0, -1, 0), hoverColor);
        }

        const QColor iconColor = token.colorTextTertiary;
        const int iconInsetX = qMin(6, qMax(2, panelWidth / 4));
        AntIconPainter::drawIcon(painter, Ant::IconType::Up, upRect.adjusted(iconInsetX, 2, -iconInsetX, -2), iconColor);
        AntIconPainter::drawIcon(painter, Ant::IconType::Down, downRect.adjusted(iconInsetX, 2, -iconInsetX, -2), iconColor);
    }

private:
    AntInputNumber* m_input = nullptr;
};
}

AntInputNumber::AntInputNumber(QWidget* parent)
    : QDoubleSpinBox(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);
    setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setFrame(false);
    setAccelerated(true);
    setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
    setDecimals(0);
    setRange(-999999, 999999);

    auto* inputStyle = new AntInputNumberStyle(style());
    inputStyle->setParent(this);
    setStyle(inputStyle);

    m_controlsAnimation = new QPropertyAnimation(this, "controlsProgress", this);
    m_controlsAnimation->setDuration(160);
    m_controlsAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_controlsOverlay = new InputNumberControlsOverlay(this);
    m_controlsOverlay->hide();

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateEditStyle();
        style()->unpolish(this);
        style()->polish(this);
        updateControlsOverlayGeometry();
        updateGeometry();
        update();
    });

    updateEditStyle();
    if (lineEdit())
    {
        lineEdit()->installEventFilter(this);
        lineEdit()->setMouseTracking(true);
    }
}

Ant::Size AntInputNumber::inputSize() const
{
    return m_inputSize;
}

void AntInputNumber::setInputSize(Ant::Size size)
{
    if (m_inputSize == size)
    {
        return;
    }
    m_inputSize = size;
    updateEditStyle();
    updateGeometry();
    update();
    Q_EMIT inputSizeChanged(m_inputSize);
}

Ant::Status AntInputNumber::status() const
{
    return m_status;
}

void AntInputNumber::setStatus(Ant::Status status)
{
    if (m_status == status)
    {
        return;
    }
    m_status = status;
    update();
    Q_EMIT statusChanged(m_status);
}

Ant::Variant AntInputNumber::variant() const
{
    return m_variant;
}

void AntInputNumber::setVariant(Ant::Variant variant)
{
    if (m_variant == variant)
    {
        return;
    }
    m_variant = variant;
    updateEditStyle();
    update();
    Q_EMIT variantChanged(m_variant);
}

bool AntInputNumber::controlsVisible() const
{
    return m_controlsVisible;
}

void AntInputNumber::setControlsVisible(bool visible)
{
    if (m_controlsVisible == visible)
    {
        return;
    }
    m_controlsVisible = visible;
    setButtonSymbols(visible ? QAbstractSpinBox::UpDownArrows : QAbstractSpinBox::NoButtons);
    animateControls(shouldShowControls());
    updateEditStyle();
    updateGeometry();
    update();
    Q_EMIT controlsVisibleChanged(m_controlsVisible);
}

qreal AntInputNumber::controlsProgress() const
{
    return m_controlsProgress;
}

void AntInputNumber::setControlsProgress(qreal progress)
{
    progress = qBound<qreal>(0.0, progress, 1.0);
    if (qFuzzyCompare(m_controlsProgress, progress))
    {
        return;
    }
    m_controlsProgress = progress;
    updateEditStyle();
    if (lineEdit())
    {
        lineEdit()->update();
    }
    updateControlsOverlayGeometry();
    if (m_controlsOverlay)
    {
        m_controlsOverlay->update();
    }
    update();
}

QString AntInputNumber::placeholderText() const
{
    return lineEdit() ? lineEdit()->placeholderText() : QString();
}

void AntInputNumber::setPlaceholderText(const QString& text)
{
    if (lineEdit())
    {
        lineEdit()->setPlaceholderText(text);
    }
}

void AntInputNumber::setPrefixText(const QString& text)
{
    setPrefix(text);
}

void AntInputNumber::setSuffixText(const QString& text)
{
    setSuffix(text);
}

QString AntInputNumber::addonAfterText() const
{
    return m_addonAfterText;
}

void AntInputNumber::setAddonAfterText(const QString& text)
{
    if (m_addonAfterText == text)
    {
        return;
    }
    m_addonAfterText = text;
    updateEditStyle();
    updateGeometry();
    update();
}

void AntInputNumber::setPrecision(int decimals)
{
    setDecimals(qMax(0, decimals));
}

QSize AntInputNumber::sizeHint() const
{
    return QSize(140, metrics().height);
}

QSize AntInputNumber::minimumSizeHint() const
{
    return QSize(96, metrics().height);
}

bool AntInputNumber::isHoveredState() const
{
    return m_hovered;
}

QStyle::SubControl AntInputNumber::activeSubControl() const
{
    return m_activeSubControl;
}

bool AntInputNumber::isStepPressed() const
{
    return m_stepPressed;
}

QAbstractSpinBox::StepEnabled AntInputNumber::stepEnabledFlags() const
{
    return stepEnabled();
}

void AntInputNumber::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    animateControls(true);
    update();
    QDoubleSpinBox::enterEvent(event);
}

void AntInputNumber::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_activeSubControl = QStyle::SC_None;
    m_stepPressed = false;
    animateControls(shouldShowControls());
    update();
    QDoubleSpinBox::leaveEvent(event);
}

void AntInputNumber::mouseMoveEvent(QMouseEvent* event)
{
    m_activeSubControl = hitSubControl(event->pos());
    if (m_controlsOverlay)
    {
        m_controlsOverlay->update();
    }
    update();
    QDoubleSpinBox::mouseMoveEvent(event);
}

void AntInputNumber::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_activeSubControl = hitSubControl(event->pos());
        m_stepPressed = m_activeSubControl == QStyle::SC_SpinBoxUp || m_activeSubControl == QStyle::SC_SpinBoxDown;
        if (m_controlsOverlay)
        {
            m_controlsOverlay->update();
        }
        update();
    }
    QDoubleSpinBox::mousePressEvent(event);
}

void AntInputNumber::mouseReleaseEvent(QMouseEvent* event)
{
    m_activeSubControl = hitSubControl(event->pos());
    m_stepPressed = false;
    if (m_controlsOverlay)
    {
        m_controlsOverlay->update();
    }
    update();
    QDoubleSpinBox::mouseReleaseEvent(event);
}

void AntInputNumber::focusInEvent(QFocusEvent* event)
{
    animateControls(true);
    update();
    QDoubleSpinBox::focusInEvent(event);
}

void AntInputNumber::focusOutEvent(QFocusEvent* event)
{
    animateControls(m_hovered);
    update();
    QDoubleSpinBox::focusOutEvent(event);
}

void AntInputNumber::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        animateControls(shouldShowControls());
        updateEditStyle();
        update();
    }
    QDoubleSpinBox::changeEvent(event);
}

void AntInputNumber::resizeEvent(QResizeEvent* event)
{
    QDoubleSpinBox::resizeEvent(event);
    updateControlsOverlayGeometry();
}

bool AntInputNumber::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == lineEdit())
    {
        switch (event->type())
        {
        case QEvent::Enter:
        case QEvent::FocusIn:
            animateControls(true);
            update();
            break;
        case QEvent::Leave:
            animateControls(shouldShowControls());
            update();
            break;
        case QEvent::FocusOut:
            animateControls(m_hovered);
            update();
            break;
        default:
            break;
        }
    }
    return QDoubleSpinBox::eventFilter(watched, event);
}

AntInputNumber::Metrics AntInputNumber::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    switch (m_inputSize)
    {
    case Ant::Size::Large:
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
        m.radius = token.borderRadiusLG;
        m.paddingX = token.paddingSM;
        break;
    case Ant::Size::Small:
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSizeSM;
        m.radius = token.borderRadiusSM;
        m.paddingX = token.paddingXS;
        break;
    case Ant::Size::Middle:
        m.height = token.controlHeight;
        m.fontSize = token.fontSize;
        m.radius = token.borderRadius;
        m.paddingX = token.paddingSM - token.lineWidth;
        break;
    }
    return m;
}

void AntInputNumber::updateEditStyle()
{
    if (!lineEdit())
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const Metrics m = metrics();

    QFont font = lineEdit()->font();
    font.setPixelSize(m.fontSize);
    lineEdit()->setFont(font);
    lineEdit()->setFrame(false);
    lineEdit()->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QPalette lePalette = lineEdit()->palette();
    lePalette.setColor(QPalette::Base, Qt::transparent);
    lePalette.setColor(QPalette::Text, isEnabled() ? token.colorText : token.colorTextDisabled);
    lePalette.setColor(QPalette::Highlight, token.colorPrimary);
    lePalette.setColor(QPalette::HighlightedText, token.colorTextLightSolid);
    lineEdit()->setPalette(lePalette);
    lineEdit()->setAttribute(Qt::WA_TranslucentBackground, true);
    lineEdit()->setStyleSheet(QStringLiteral("QLineEdit { background: transparent; border: none; padding: 0; }"));
    lineEdit()->setTextMargins(0, 0, controlsInsetWidth(), 0);

    setMinimumHeight(m.height);
    setMaximumHeight(m.height);
}

QStyle::SubControl AntInputNumber::hitSubControl(const QPoint& pos) const
{
    if (!m_controlsVisible)
    {
        return QStyle::SC_None;
    }

    QStyleOptionSpinBox option;
    option.initFrom(this);
    option.rect = rect();
    const QRect upRect = style()->subControlRect(QStyle::CC_SpinBox, &option, QStyle::SC_SpinBoxUp, this);
    if (upRect.contains(pos))
    {
        return QStyle::SC_SpinBoxUp;
    }
    const QRect downRect = style()->subControlRect(QStyle::CC_SpinBox, &option, QStyle::SC_SpinBoxDown, this);
    if (downRect.contains(pos))
    {
        return QStyle::SC_SpinBoxDown;
    }
    return QStyle::SC_None;
}

bool AntInputNumber::shouldShowControls() const
{
    return m_controlsVisible && isEnabled() && (m_hovered || hasFocus() || (lineEdit() && lineEdit()->hasFocus()));
}

void AntInputNumber::animateControls(bool visible)
{
    visible = visible && m_controlsVisible && isEnabled();
    const qreal end = visible ? 1.0 : 0.0;
    if (qFuzzyCompare(m_controlsProgress, end) && m_controlsAnimation->state() != QAbstractAnimation::Running)
    {
        return;
    }

    m_controlsAnimation->stop();
    m_controlsAnimation->setStartValue(m_controlsProgress);
    m_controlsAnimation->setEndValue(end);
    m_controlsAnimation->start();
}

int AntInputNumber::controlsInsetWidth() const
{
    if (!m_controlsVisible || !isEnabled())
    {
        return 0;
    }

    return qRound(handlerWidthForInput(this) * m_controlsProgress);
}

void AntInputNumber::updateControlsOverlayGeometry()
{
    if (!m_controlsOverlay)
    {
        return;
    }

    const int handlerWidth = handlerWidthForInput(this);
    const QRect control = rect().adjusted(1, 1, -1, -1);
    m_controlsOverlay->setGeometry(control.right() - handlerWidth + 1,
                                   control.top(),
                                   handlerWidth,
                                   control.height());
    m_controlsOverlay->setVisible(m_controlsVisible && isEnabled() && m_controlsProgress > 0.01);
    m_controlsOverlay->raise();
}
