#include "AntButton.h"

#include <QEvent>
#include <QMouseEvent>
#include <QStyleOptionButton>

#include "../styles/AntButtonStyle.h"
#include "core/AntTheme.h"
#include "core/AntWave.h"

AntButton::AntButton(QWidget* parent)
    : QPushButton(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_Hover, true);

    auto* buttonStyle = new AntButtonStyle(style());
    buttonStyle->setParent(this);
    setStyle(buttonStyle);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometryFromState();
        update();
    });
    connect(&m_spinnerTimer, &QTimer::timeout, this, [this]() {
        m_spinnerAngle = (m_spinnerAngle - 6 + 360) % 360;
        update();
    });

    updateCursorState();
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

Ant::Size AntButton::buttonSize() const { return m_buttonSize; }

void AntButton::setButtonSize(Ant::Size size)
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
    updateCursorState();
    m_loading ? m_spinnerTimer.start(16) : m_spinnerTimer.stop();
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

Ant::IconType AntButton::buttonIconType() const { return m_buttonIconType; }

void AntButton::setButtonIconType(Ant::IconType iconType)
{
    if (m_buttonIconType == iconType)
        return;
    m_buttonIconType = iconType;
    updateGeometryFromState();
    update();
    Q_EMIT buttonIconTypeChanged(m_buttonIconType);
}

QColor AntButton::buttonIconColor() const { return m_buttonIconColor; }

void AntButton::setButtonIconColor(const QColor& color)
{
    if (m_buttonIconColor == color)
        return;
    m_buttonIconColor = color;
    update();
    Q_EMIT buttonIconColorChanged(m_buttonIconColor);
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

void AntButton::keyPressEvent(QKeyEvent* event)
{
    QPushButton::keyPressEvent(event);
}

bool AntButton::hitButton(const QPoint& pos) const
{
    return QPushButton::hitButton(pos);
}

void AntButton::mouseReleaseEvent(QMouseEvent* event)
{
    const bool wasPressed = m_pressed;
    m_pressed = false;
    update();
    if (wasPressed && isEnabled() && !m_loading && rect().contains(event->pos())
        && m_buttonType != Ant::ButtonType::Text
        && m_buttonType != Ant::ButtonType::Link)
    {
        const auto& token = antTheme->tokens();
        const QColor tint = m_danger ? token.colorError : token.colorPrimary;
        const Metrics m = metrics();
        AntWave::trigger(this, tint, cornerRadius(m));
    }
    QPushButton::mouseReleaseEvent(event);
}

void AntButton::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange || event->type() == QEvent::FontChange)
    {
        if (event->type() == QEvent::EnabledChange)
        {
            updateCursorState();
        }
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
    case Ant::Size::Large:
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
        m.paddingX = token.padding - token.lineWidth;
        m.radius = token.borderRadiusLG;
        m.iconSize = 16;
        break;
    case Ant::Size::Small:
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSize;
        m.paddingX = token.paddingXS - token.lineWidth;
        m.radius = token.borderRadiusSM;
        m.iconSize = 14;
        break;
    case Ant::Size::Middle:
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

void AntButton::updateCursorState()
{
    if (!isEnabled())
    {
        setCursor(Qt::ForbiddenCursor);
        return;
    }

    setCursor(m_loading ? Qt::ArrowCursor : Qt::PointingHandCursor);
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
