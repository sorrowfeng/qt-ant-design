#include "AntToolButton.h"

#include <QAction>
#include <QActionEvent>
#include <QEvent>
#include <QFocusEvent>
#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPointer>
#include <QPropertyAnimation>
#include <QShowEvent>
#include <QSizePolicy>
#include <QStyleOptionToolButton>
#include <QTimer>

#include "../styles/AntToolButtonStyle.h"
#include "AntToolTip.h"
#include "core/AntThemeRefresh_p.h"
#include "core/AntTheme.h"
#include "core/AntWave.h"

namespace
{
int focusPaddingForToolButton()
{
    const auto& token = antTheme->tokens();
    return token.lineWidthFocus + 1;
}

QString cleanActionTextForToolButton(QString text)
{
    const QChar escapedAmpersand(1);
    text.replace(QStringLiteral("&&"), QString(escapedAmpersand));
    text.remove(QLatin1Char('&'));
    text.replace(QString(escapedAmpersand), QStringLiteral("&"));
    return text.trimmed();
}
} // namespace

AntToolButton::AntToolButton(QWidget* parent)
    : QToolButton(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_Hover, true);
    setPopupMode(QToolButton::InstantPopup);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    m_antToolTip = new AntToolTip(this);
    m_antToolTip->setObjectName(QStringLiteral("AntToolButtonToolTip"));
    m_antToolTip->setPlacement(Ant::TooltipPlacement::Top);
    m_antToolTip->setTarget(this);

    auto* style = new AntToolButtonStyle(this->style());
    style->setParent(this);
    setStyle(style);

    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometryFromState(false);
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
        syncAntToolTip();
        update();
    });

    m_spinnerTimer = new QTimer(this);
    connect(m_spinnerTimer, &QTimer::timeout, this, [this]() {
        m_spinnerAngle = (m_spinnerAngle + 6) % 360;
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

    updateCursorState();
    updateGeometryFromState();
    syncAntToolTip();
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
    updateCursorState();
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
    option.icon = icon();
    return style()->sizeFromContents(QStyle::CT_ToolButton, &option, QSize(), this);
}

QSize AntToolButton::minimumSizeHint() const
{
    return sizeHint();
}

bool AntToolButton::event(QEvent* event)
{
    if (event && event->type() == QEvent::ToolTip)
    {
        syncAntToolTip();
        if (m_antToolTip && !m_antToolTip->title().trimmed().isEmpty())
        {
            m_antToolTip->showTooltip();
            event->accept();
            return true;
        }
        if (m_antToolTip)
        {
            m_antToolTip->hideTooltip();
        }
        event->ignore();
        return true;
    }

    if (event && event->type() == QEvent::ToolTipChange)
    {
        const bool handled = QToolButton::event(event);
        syncAntToolTip();
        return handled;
    }

    return QToolButton::event(event);
}

void AntToolButton::actionEvent(QActionEvent* event)
{
    QToolButton::actionEvent(event);
    if (!event)
    {
        return;
    }

    if (event->type() == QEvent::ActionAdded
        || event->type() == QEvent::ActionChanged
        || event->type() == QEvent::ActionRemoved)
    {
        syncAntToolTip();
        updateGeometryFromState();
        update();
    }
}

void AntToolButton::enterEvent(AntEnterEvent* event)
{
    syncAntToolTip();
    m_hovered = true;
    update();
    QToolButton::enterEvent(event);
}

void AntToolButton::leaveEvent(QEvent* event)
{
    if (m_antToolTip)
    {
        m_antToolTip->hideTooltip();
    }
    m_hovered = false;
    m_pressed = false;
    update();
    QToolButton::leaveEvent(event);
}

void AntToolButton::mousePressEvent(QMouseEvent* event)
{
    if (event && event->button() == Qt::LeftButton && isEnabled() && hitButton(event->pos()))
    {
        m_pressed = true;
        m_focusVisible = false;
        update();
    }
    QToolButton::mousePressEvent(event);
}

void AntToolButton::mouseReleaseEvent(QMouseEvent* event)
{
    const bool wasPressed = m_pressed;
    m_pressed = false;
    update();
    const bool shouldWave = event && event->button() == Qt::LeftButton
        && wasPressed && isEnabled() && !m_loading && hitButton(event->pos())
        && !menu()
        && m_buttonType != Ant::ButtonType::Text
        && m_buttonType != Ant::ButtonType::Link;
    QPointer<AntToolButton> guard(this);
    QToolButton::mouseReleaseEvent(event);
    if (shouldWave && guard)
    {
        QTimer::singleShot(0, guard.data(), [guard]() {
            if (!guard)
            {
                return;
            }
            const AntToolButton::Metrics m = guard->metrics();
            const int focusPadding = focusPaddingForToolButton();
            const QRect bodyRect = guard->rect().adjusted(focusPadding, focusPadding, -focusPadding, -focusPadding);
            AntWave::triggerRect(guard, bodyRect, guard->waveColor(), guard->cornerRadius(m));
        });
    }
}

void AntToolButton::keyPressEvent(QKeyEvent* event)
{
    if (event && (event->key() == Qt::Key_Space
                  || event->key() == Qt::Key_Return
                  || event->key() == Qt::Key_Enter))
    {
        m_focusVisible = true;
        update();
    }
    QToolButton::keyPressEvent(event);
}

void AntToolButton::focusInEvent(QFocusEvent* event)
{
    const Qt::FocusReason reason = event ? event->reason() : Qt::OtherFocusReason;
    m_focusVisible = reason == Qt::TabFocusReason
        || reason == Qt::BacktabFocusReason
        || reason == Qt::ShortcutFocusReason;
    syncAntToolTip();
    update();
    QToolButton::focusInEvent(event);
}

void AntToolButton::focusOutEvent(QFocusEvent* event)
{
    if (m_antToolTip)
    {
        m_antToolTip->hideTooltip();
    }
    m_focusVisible = false;
    update();
    QToolButton::focusOutEvent(event);
}

void AntToolButton::changeEvent(QEvent* event)
{
    if (event && (event->type() == QEvent::EnabledChange
                  || event->type() == QEvent::FontChange
                  || event->type() == QEvent::ToolTipChange))
    {
        if (event->type() == QEvent::EnabledChange)
        {
            updateCursorState();
        }
        if (event->type() == QEvent::ToolTipChange)
        {
            syncAntToolTip();
        }
        updateGeometryFromState();
        update();
    }
    QToolButton::changeEvent(event);
}

void AntToolButton::showEvent(QShowEvent* event)
{
    QToolButton::showEvent(event);
    syncAntToolTip();
    updateSpinnerTimer();
}

void AntToolButton::hideEvent(QHideEvent* event)
{
    QToolButton::hideEvent(event);
    if (m_antToolTip)
    {
        m_antToolTip->hideTooltip();
    }
    updateSpinnerTimer();
}

bool AntToolButton::hitButton(const QPoint& pos) const
{
    const int focusPadding = focusPaddingForToolButton();
    return rect().adjusted(focusPadding, focusPadding, -focusPadding, -focusPadding).contains(pos);
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

int AntToolButton::cornerRadius(const Metrics& m) const
{
    return m.radius;
}

QRectF AntToolButton::contentRect(const Metrics& metrics) const
{
    const int focusPadding = focusPaddingForToolButton();
    const QRect bodyRect = rect().adjusted(focusPadding, focusPadding, -focusPadding, -focusPadding);
    return bodyRect.adjusted(metrics.paddingX, 0, -metrics.paddingX, 0);
}

QColor AntToolButton::waveColor() const
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

void AntToolButton::updateCursorState()
{
    if (!isEnabled())
    {
        setCursor(Qt::ForbiddenCursor);
        return;
    }

    setCursor(m_loading ? Qt::ArrowCursor : Qt::PointingHandCursor);
}

void AntToolButton::updateGeometryFromState(bool notifyGeometry)
{
    const Metrics m = metrics();
    QFont f = font();
    f.setPixelSize(m.fontSize);
    if (font() != f)
    {
        setFont(f);
    }
    const int totalHeight = m.height + focusPaddingForToolButton() * 2;
    setMinimumHeight(totalHeight);
    setMaximumHeight(totalHeight);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    if (notifyGeometry)
    {
        updateGeometry();
    }
}

QRect AntToolButton::spinnerIndicatorRect() const
{
    if (!m_loading)
    {
        return {};
    }

    const Metrics m = metrics();
    const int focusPadding = focusPaddingForToolButton();
    const QRect bodyRect = rect().adjusted(focusPadding, focusPadding, -focusPadding, -focusPadding);
    if (!bodyRect.isValid())
    {
        return {};
    }

    QRectF textArea = contentRect(m);
    if (menu() || popupMode() == QToolButton::MenuButtonPopup)
    {
        const qreal arrowW = m.arrowSize + 8;
        textArea.adjust(0, 0, -(arrowW + 2), 0);
    }

    const QRectF body(bodyRect);
    const bool spinnerOnly = text().isEmpty();
    const QRectF spinnerRect = spinnerOnly
        ? QRectF(body.center().x() - m.iconSize / 2.0, body.center().y() - m.iconSize / 2.0, m.iconSize, m.iconSize)
        : QRectF(textArea.left(), textArea.center().y() - m.iconSize / 2.0, m.iconSize, m.iconSize);
    return spinnerRect.toAlignedRect().adjusted(-4, -4, 4, 4).intersected(rect());
}

QRect AntToolButton::arrowIndicatorRect() const
{
    if (!menu() && popupMode() != QToolButton::MenuButtonPopup)
    {
        return {};
    }

    const Metrics m = metrics();
    QRectF content = contentRect(m);
    const qreal arrowW = m.arrowSize + 8;
    const QRectF arrowRect(content.right() - arrowW,
                           content.center().y() - m.arrowSize / 2.0,
                           m.arrowSize,
                           m.arrowSize);
    return arrowRect.toAlignedRect().adjusted(-4, -4, 4, 4).intersected(rect());
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
        m_spinnerTimer->start(16);
    }
    else
    {
        m_spinnerTimer->stop();
    }
    syncToolButtonPerfCounters();
}

QString AntToolButton::effectiveToolTipText() const
{
    const QString ownToolTip = toolTip().trimmed();
    if (!ownToolTip.isEmpty())
    {
        return ownToolTip;
    }

    const QAction* action = defaultAction();
    if (!action)
    {
        return {};
    }

    const QString actionToolTip = action->toolTip().trimmed();
    if (!actionToolTip.isEmpty())
    {
        return actionToolTip;
    }

    const QString statusTip = action->statusTip().trimmed();
    if (!statusTip.isEmpty())
    {
        return statusTip;
    }

    return cleanActionTextForToolButton(action->text());
}

void AntToolButton::syncAntToolTip()
{
    if (!m_antToolTip)
    {
        return;
    }

    const QString text = effectiveToolTipText();
    m_antToolTip->setTitle(text);
    setProperty("antToolButtonToolTipText", text);
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

bool AntToolButton::isFocusVisibleState() const
{
    return m_focusVisible;
}
