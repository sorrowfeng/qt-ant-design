#include "AntButton.h"

#include <QEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QPointer>
#include <QStyleOptionButton>

#include "../styles/AntButtonStyle.h"
#include "core/AntTheme.h"
#include "core/AntWave.h"

namespace
{
int focusPaddingFor()
{
    const auto& token = antTheme->tokens();
    return token.lineWidthFocus + 1;
}
} // namespace

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
        m_spinnerAngle = (m_spinnerAngle + 6) % 360;
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
    if (event->button() == Qt::LeftButton && isEnabled() && hitButton(event->pos()))
    {
        m_pressed = true;
        m_focusVisible = false;
        update();
    }
    QPushButton::mousePressEvent(event);
}

void AntButton::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        m_focusVisible = true;
        update();
    }
    QPushButton::keyPressEvent(event);
}

void AntButton::focusInEvent(QFocusEvent* event)
{
    const Qt::FocusReason reason = event->reason();
    m_focusVisible = reason == Qt::TabFocusReason
        || reason == Qt::BacktabFocusReason
        || reason == Qt::ShortcutFocusReason;
    update();
    QPushButton::focusInEvent(event);
}

void AntButton::focusOutEvent(QFocusEvent* event)
{
    m_focusVisible = false;
    update();
    QPushButton::focusOutEvent(event);
}

bool AntButton::hitButton(const QPoint& pos) const
{
    const int focusPadding = focusPaddingFor();
    return rect().adjusted(focusPadding, focusPadding, -focusPadding, -focusPadding).contains(pos);
}

void AntButton::mouseReleaseEvent(QMouseEvent* event)
{
    const bool wasPressed = m_pressed;
    m_pressed = false;
    update();
    const bool shouldWave = event->button() == Qt::LeftButton
        && wasPressed && isEnabled() && !m_loading && hitButton(event->pos())
        && m_buttonType != Ant::ButtonType::Text
        && m_buttonType != Ant::ButtonType::Link;
    QPointer<AntButton> guard(this);
    QPushButton::mouseReleaseEvent(event);
    if (shouldWave && guard)
    {
        QTimer::singleShot(0, guard.data(), [guard]() {
            if (!guard)
            {
                return;
            }
            const AntButton::Metrics m = guard->metrics();
            const int focusPadding = focusPaddingFor();
            const QRect bodyRect = guard->rect().adjusted(focusPadding, focusPadding, -focusPadding, -focusPadding);
            AntWave::triggerRect(guard, bodyRect, guard->waveColor(), guard->cornerRadius(m));
        });
    }
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
    const int focusPadding = focusPaddingFor();
    const QRect bodyRect = rect().adjusted(focusPadding, focusPadding, -focusPadding, -focusPadding);
    if (m_buttonShape == Ant::ButtonShape::Circle)
        return bodyRect;
    return bodyRect.adjusted(metrics.paddingX, 0, -metrics.paddingX, 0);
}

QColor AntButton::waveColor() const
{
    const auto& token = antTheme->tokens();
    const bool hovered = m_hovered || underMouse();
    const QColor accent = m_danger ? token.colorError : token.colorPrimary;
    const QColor accentHover = m_danger ? token.colorErrorHover : token.colorPrimaryHover;

    if (m_buttonType == Ant::ButtonType::Primary)
    {
        return hovered ? accentHover : accent;
    }
    if (m_buttonType == Ant::ButtonType::Default || m_buttonType == Ant::ButtonType::Dashed)
    {
        if (m_danger)
        {
            return hovered ? token.colorErrorHover : token.colorError;
        }
        return hovered ? token.colorPrimaryHover : token.colorBorder;
    }

    return hovered ? accentHover : accent;
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
    const int totalHeight = m.height + focusPaddingFor() * 2;
    setMinimumHeight(totalHeight);
    setMaximumHeight(totalHeight);
    setSizePolicy(m_block ? QSizePolicy::Expanding : QSizePolicy::Preferred, QSizePolicy::Fixed);
    updateGeometry();
}

int AntButton::spinnerAngle() const
{
    return m_spinnerAngle;
}

bool AntButton::isFocusVisibleState() const
{
    return m_focusVisible;
}
