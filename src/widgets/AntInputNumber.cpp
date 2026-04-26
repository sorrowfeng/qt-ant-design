#include "AntInputNumber.h"

#include <QLineEdit>
#include <QMouseEvent>
#include <QStyleOptionSpinBox>

#include "../styles/AntInputNumberStyle.h"
#include "core/AntTheme.h"

AntInputNumber::AntInputNumber(QWidget* parent)
    : QDoubleSpinBox(parent)
{
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

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateEditStyle();
        style()->unpolish(this);
        style()->polish(this);
        updateGeometry();
        update();
    });

    updateEditStyle();
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
    updateEditStyle();
    updateGeometry();
    update();
    Q_EMIT controlsVisibleChanged(m_controlsVisible);
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
    update();
    QDoubleSpinBox::enterEvent(event);
}

void AntInputNumber::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_activeSubControl = QStyle::SC_None;
    m_stepPressed = false;
    update();
    QDoubleSpinBox::leaveEvent(event);
}

void AntInputNumber::mouseMoveEvent(QMouseEvent* event)
{
    m_activeSubControl = hitSubControl(event->pos());
    update();
    QDoubleSpinBox::mouseMoveEvent(event);
}

void AntInputNumber::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_activeSubControl = hitSubControl(event->pos());
        m_stepPressed = m_activeSubControl == QStyle::SC_SpinBoxUp || m_activeSubControl == QStyle::SC_SpinBoxDown;
        update();
    }
    QDoubleSpinBox::mousePressEvent(event);
}

void AntInputNumber::mouseReleaseEvent(QMouseEvent* event)
{
    m_activeSubControl = hitSubControl(event->pos());
    m_stepPressed = false;
    update();
    QDoubleSpinBox::mouseReleaseEvent(event);
}

void AntInputNumber::focusInEvent(QFocusEvent* event)
{
    update();
    QDoubleSpinBox::focusInEvent(event);
}

void AntInputNumber::focusOutEvent(QFocusEvent* event)
{
    update();
    QDoubleSpinBox::focusOutEvent(event);
}

void AntInputNumber::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        updateEditStyle();
        update();
    }
    QDoubleSpinBox::changeEvent(event);
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

    const QString textColor = isEnabled() ? token.colorText.name(QColor::HexArgb) : token.colorTextDisabled.name(QColor::HexArgb);
    lineEdit()->setStyleSheet(
        QStringLiteral("QLineEdit { background: transparent; border: none; color: %1; selection-background-color: %2; padding: 0; }")
            .arg(textColor, token.colorPrimary.name(QColor::HexArgb)));

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
