#include "AntToolButton.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QStyleOptionToolButton>
#include <QTimer>

#include "../styles/AntToolButtonStyle.h"
#include "core/AntTheme.h"
#include "core/AntWave.h"

AntToolButton::AntToolButton(QWidget* parent)
    : QToolButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_Hover, true);
    setPopupMode(QToolButton::InstantPopup);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto* style = new AntToolButtonStyle(this->style());
    style->setParent(this);
    setStyle(style);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometryFromState();
        update();
    });

    m_spinnerTimer = new QTimer(this);
    connect(m_spinnerTimer, &QTimer::timeout, this, [this]() {
        m_spinnerAngle = (m_spinnerAngle + 30) % 360;
        update();
    });

    m_arrowAnimation = new QPropertyAnimation(this, "arrowRotation", this);
    m_arrowAnimation->setDuration(160);
    m_arrowAnimation->setEasingCurve(QEasingCurve::OutCubic);

    connect(this, &QToolButton::pressed, this, [this]() {
        if (menu())
        {
            m_arrowAnimation->stop();
            m_arrowAnimation->setEndValue(180.0);
            m_arrowAnimation->start();
        }
    });

    updateGeometryFromState();
}

AntToolButton::AntToolButton(const QString& text, QWidget* parent)
    : AntToolButton(parent)
{
    setText(text);
}

Ant::ButtonType AntToolButton::buttonType() const { return m_buttonType; }

void AntToolButton::setButtonType(Ant::ButtonType type)
{
    if (m_buttonType == type) return;
    m_buttonType = type;
    updateGeometryFromState();
    update();
    Q_EMIT buttonTypeChanged(m_buttonType);
}

Ant::ButtonSize AntToolButton::buttonSize() const { return m_buttonSize; }

void AntToolButton::setButtonSize(Ant::ButtonSize size)
{
    if (m_buttonSize == size) return;
    m_buttonSize = size;
    updateGeometryFromState();
    update();
    Q_EMIT buttonSizeChanged(m_buttonSize);
}

bool AntToolButton::isDanger() const { return m_danger; }

void AntToolButton::setDanger(bool danger)
{
    if (m_danger == danger) return;
    m_danger = danger;
    update();
    Q_EMIT dangerChanged(m_danger);
}

bool AntToolButton::isLoading() const { return m_loading; }

void AntToolButton::setLoading(bool loading)
{
    if (m_loading == loading) return;
    m_loading = loading;
    m_loading ? m_spinnerTimer->start(80) : m_spinnerTimer->stop();
    updateGeometryFromState();
    update();
    Q_EMIT loadingChanged(m_loading);
}

qreal AntToolButton::arrowRotation() const { return m_arrowRotation; }

void AntToolButton::setArrowRotation(qreal rotation)
{
    if (qFuzzyCompare(m_arrowRotation, rotation)) return;
    m_arrowRotation = rotation;
    update();
    Q_EMIT arrowRotationChanged(m_arrowRotation);
}

QSize AntToolButton::sizeHint() const
{
    QStyleOptionToolButton option;
    option.initFrom(this);
    option.text = text();
    return style()->sizeFromContents(QStyle::CT_ToolButton, &option, QSize(), this);
}

QSize AntToolButton::minimumSizeHint() const
{
    return sizeHint();
}

void AntToolButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QToolButton::enterEvent(event);
}

void AntToolButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    update();
    QToolButton::leaveEvent(event);
}

void AntToolButton::mousePressEvent(QMouseEvent* event)
{
    m_pressed = true;
    update();
    QToolButton::mousePressEvent(event);
}

void AntToolButton::mouseReleaseEvent(QMouseEvent* event)
{
    const bool wasPressed = m_pressed;
    m_pressed = false;
    update();
    if (wasPressed && isEnabled() && !m_loading && rect().contains(event->pos()) && !menu())
    {
        const auto& token = antTheme->tokens();
        QColor tint = m_danger ? token.colorError : token.colorPrimary;
        const Metrics m = metrics();
        AntWave::trigger(this, tint, cornerRadius(m));
    }
    QToolButton::mouseReleaseEvent(event);
}

void AntToolButton::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange || event->type() == QEvent::FontChange)
    {
        updateGeometryFromState();
        update();
    }
    QToolButton::changeEvent(event);
}

AntToolButton::Metrics AntToolButton::metrics() const
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

int AntToolButton::cornerRadius(const Metrics& m) const
{
    return m.radius;
}

void AntToolButton::updateGeometryFromState()
{
    const Metrics m = metrics();
    QFont f = font();
    f.setPixelSize(m.fontSize);
    setFont(f);
    setMinimumHeight(m.height);
    setMaximumHeight(m.height);
    updateGeometry();
}

int AntToolButton::spinnerAngle() const
{
    return m_spinnerAngle;
}
