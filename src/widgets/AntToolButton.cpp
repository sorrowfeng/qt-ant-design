#include "AntToolButton.h"

#include <QEvent>
#include <QHideEvent>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QShowEvent>
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
        updateSpinnerRegion();
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
    syncToolButtonPerfCounters();
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

Ant::Size AntToolButton::buttonSize() const { return m_buttonSize; }

void AntToolButton::setButtonSize(Ant::Size size)
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
    updateSpinnerTimer();
    updateGeometryFromState();
    update();
    Q_EMIT loadingChanged(m_loading);
}

qreal AntToolButton::arrowRotation() const { return m_arrowRotation; }

void AntToolButton::setArrowRotation(qreal rotation)
{
    if (qFuzzyCompare(m_arrowRotation, rotation)) return;
    m_arrowRotation = rotation;
    updateArrowRegion();
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

void AntToolButton::showEvent(QShowEvent* event)
{
    QToolButton::showEvent(event);
    updateSpinnerTimer();
}

void AntToolButton::hideEvent(QHideEvent* event)
{
    QToolButton::hideEvent(event);
    updateSpinnerTimer();
}

AntToolButton::Metrics AntToolButton::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    switch (m_buttonSize)
    {
    case Ant::Size::Large:
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
        m.paddingX = token.padding;
        m.radius = token.borderRadiusLG;
        m.iconSize = 16;
        break;
    case Ant::Size::Small:
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSize;
        m.paddingX = token.paddingXS;
        m.radius = token.borderRadiusSM;
        m.iconSize = 12;
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

QRect AntToolButton::spinnerIndicatorRect() const
{
    if (!m_loading)
    {
        return {};
    }

    const Metrics m = metrics();
    QRectF contentRect = QRectF(rect()).adjusted(m.paddingX, 0, -m.paddingX, 0);
    if (!icon().isNull())
    {
        contentRect.adjust(m.iconSize + 6, 0, 0, 0);
    }
    if (menu() || popupMode() == QToolButton::MenuButtonPopup)
    {
        const qreal arrowW = m.arrowSize + 8;
        contentRect.adjust(0, 0, -(arrowW + 2), 0);
    }

    const QRectF spinnerRect(contentRect.left(),
                             contentRect.center().y() - m.iconSize / 2.0,
                             m.iconSize,
                             m.iconSize);
    return spinnerRect.toAlignedRect().adjusted(-3, -3, 3, 3).intersected(rect());
}

QRect AntToolButton::arrowIndicatorRect() const
{
    if (!menu() && popupMode() != QToolButton::MenuButtonPopup)
    {
        return {};
    }

    const Metrics m = metrics();
    QRectF contentRect = QRectF(rect()).adjusted(m.paddingX, 0, -m.paddingX, 0);
    if (!icon().isNull())
    {
        contentRect.adjust(m.iconSize + 6, 0, 0, 0);
    }

    const qreal arrowW = m.arrowSize + 8;
    const QRectF arrowRect(contentRect.right() - arrowW,
                           contentRect.center().y() - m.arrowSize / 2.0,
                           m.arrowSize,
                           m.arrowSize);
    return arrowRect.toAlignedRect().adjusted(-3, -3, 3, 3).intersected(rect());
}

void AntToolButton::updateSpinnerRegion()
{
    updateIndicatorRegion(spinnerIndicatorRect(), m_spinnerRegionUpdateCount);
}

void AntToolButton::updateArrowRegion()
{
    updateIndicatorRegion(arrowIndicatorRect(), m_arrowRegionUpdateCount);
}

void AntToolButton::updateIndicatorRegion(const QRect& rect, int& counter)
{
    if (rect.isValid())
    {
        update(rect);
        ++counter;
        syncToolButtonPerfCounters();
    }
}

void AntToolButton::updateSpinnerTimer()
{
    const bool shouldRun = m_loading && isVisible();
    if (m_spinnerTimer->isActive() == shouldRun)
    {
        syncToolButtonPerfCounters();
        return;
    }

    if (shouldRun)
    {
        m_spinnerTimer->start(80);
    }
    else
    {
        m_spinnerTimer->stop();
    }
    syncToolButtonPerfCounters();
}

void AntToolButton::syncToolButtonPerfCounters() const
{
    auto* self = const_cast<AntToolButton*>(this);
    self->setProperty("antToolButtonSpinnerRegionUpdateCount", m_spinnerRegionUpdateCount);
    self->setProperty("antToolButtonArrowRegionUpdateCount", m_arrowRegionUpdateCount);
    self->setProperty("antToolButtonSpinnerTimerActive",
                      m_spinnerTimer && m_spinnerTimer->isActive());
}

int AntToolButton::spinnerAngle() const
{
    return m_spinnerAngle;
}
