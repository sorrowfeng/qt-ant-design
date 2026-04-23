#include "AntButton.h"

#include <QEvent>
#include <QMouseEvent>
#include <QStyleOptionButton>

#include "AntButtonStyle.h"
#include "core/AntTheme.h"

AntButton::AntButton(QWidget* parent)
    : QPushButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_Hover, true);
    setFlat(true);

    auto* buttonStyle = new AntButtonStyle(style());
    buttonStyle->setParent(this);
    setStyle(buttonStyle);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometryFromState();
        update();
    });
    connect(&m_spinnerTimer, &QTimer::timeout, this, [this]() {
        m_spinnerAngle = (m_spinnerAngle + 30) % 360;
        update();
    });

    updateGeometryFromState();
}

AntButton::AntButton(const QString& text, QWidget* parent)
    : AntButton(parent)
{
    setText(text);
}

Ant::ButtonType AntButton::buttonType() const { return m_buttonType; }

void AntButton::setButtonType(Ant::ButtonType type)
{
    if (m_buttonType == type)
        return;
    m_buttonType = type;
    updateGeometryFromState();
    update();
    Q_EMIT buttonTypeChanged(m_buttonType);
}

Ant::ButtonSize AntButton::buttonSize() const { return m_buttonSize; }

void AntButton::setButtonSize(Ant::ButtonSize size)
{
    if (m_buttonSize == size)
        return;
    m_buttonSize = size;
    updateGeometryFromState();
    update();
    Q_EMIT buttonSizeChanged(m_buttonSize);
}

Ant::ButtonShape AntButton::buttonShape() const { return m_buttonShape; }

void AntButton::setButtonShape(Ant::ButtonShape shape)
{
    if (m_buttonShape == shape)
        return;
    m_buttonShape = shape;
    updateGeometryFromState();
    update();
    Q_EMIT buttonShapeChanged(m_buttonShape);
}

bool AntButton::isLoading() const { return m_loading; }

void AntButton::setLoading(bool loading)
{
    if (m_loading == loading)
        return;
    m_loading = loading;
    m_loading ? m_spinnerTimer.start(80) : m_spinnerTimer.stop();
    updateGeometryFromState();
    update();
    Q_EMIT loadingChanged(m_loading);
}

bool AntButton::isDanger() const { return m_danger; }

void AntButton::setDanger(bool danger)
{
    if (m_danger == danger)
        return;
    m_danger = danger;
    update();
    Q_EMIT dangerChanged(m_danger);
}

bool AntButton::isGhost() const { return m_ghost; }

void AntButton::setGhost(bool ghost)
{
    if (m_ghost == ghost)
        return;
    m_ghost = ghost;
    update();
    Q_EMIT ghostChanged(m_ghost);
}

bool AntButton::isBlock() const { return m_block; }

void AntButton::setBlock(bool block)
{
    if (m_block == block)
        return;
    m_block = block;
    setSizePolicy(block ? QSizePolicy::Expanding : QSizePolicy::Preferred, QSizePolicy::Fixed);
    updateGeometry();
    Q_EMIT blockChanged(m_block);
}

QSize AntButton::sizeHint() const
{
    QStyleOptionButton option;
    option.initFrom(this);
    option.text = text();
    return style()->sizeFromContents(QStyle::CT_PushButton, &option, QSize(), this);
}

QSize AntButton::minimumSizeHint() const
{
    return sizeHint();
}

void AntButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QPushButton::enterEvent(event);
}

void AntButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    update();
    QPushButton::leaveEvent(event);
}

void AntButton::mousePressEvent(QMouseEvent* event)
{
    m_pressed = true;
    update();
    QPushButton::mousePressEvent(event);
}

void AntButton::mouseReleaseEvent(QMouseEvent* event)
{
    m_pressed = false;
    update();
    QPushButton::mouseReleaseEvent(event);
}

void AntButton::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange || event->type() == QEvent::FontChange)
    {
        updateGeometryFromState();
        update();
    }
    QPushButton::changeEvent(event);
}

AntButton::Metrics AntButton::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    switch (m_buttonSize)
    {
    case Ant::ButtonSize::Large:
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
        m.paddingX = token.padding;
        m.radius = token.borderRadiusLG;
        m.iconSize = 16;
        break;
    case Ant::ButtonSize::Small:
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSize;
        m.paddingX = token.paddingXS;
        m.radius = token.borderRadiusSM;
        m.iconSize = 12;
        break;
    case Ant::ButtonSize::Middle:
        m.height = token.controlHeight;
        m.fontSize = token.fontSize;
        m.paddingX = token.paddingSM + token.lineWidth * 3;
        m.radius = token.borderRadius;
        m.iconSize = 14;
        break;
    }
    return m;
}

int AntButton::cornerRadius(const Metrics& metrics) const
{
    if (m_buttonShape == Ant::ButtonShape::Circle || m_buttonShape == Ant::ButtonShape::Round)
        return metrics.height / 2;
    return metrics.radius;
}

QRectF AntButton::contentRect(const Metrics& metrics) const
{
    if (m_buttonShape == Ant::ButtonShape::Circle)
        return rect();
    return rect().adjusted(metrics.paddingX, 0, -metrics.paddingX, 0);
}

void AntButton::updateGeometryFromState()
{
    const Metrics m = metrics();
    QFont f = font();
    f.setPixelSize(m.fontSize);
    setFont(f);
    setMinimumHeight(m.height);
    setMaximumHeight(m.height);
    setSizePolicy(m_block ? QSizePolicy::Expanding : QSizePolicy::Preferred, QSizePolicy::Fixed);
    updateGeometry();
}

int AntButton::spinnerAngle() const
{
    return m_spinnerAngle;
}
