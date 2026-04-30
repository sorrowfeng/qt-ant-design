#include "AntDropdown.h"

#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QFrame>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QHideEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

#include "AntMenu.h"
#include "../styles/AntDropdownStyle.h"
#include "core/AntPopupMotion.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"

namespace
{
QRect availableScreenGeometryFor(const QWidget* widget)
{
    if (widget)
    {
        if (QScreen* screen = widget->screen())
        {
            return screen->availableGeometry();
        }
    }
    if (QScreen* screen = QGuiApplication::primaryScreen())
    {
        return screen->availableGeometry();
    }
    return QRect(0, 0, 1280, 720);
}
}

class AntDropdown::PopupFrame : public QFrame
{
public:
    explicit PopupFrame(AntDropdown* owner)
        : QFrame(nullptr, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
        , m_owner(owner)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_ShowWithoutActivating, true);
        setFocusPolicy(Qt::NoFocus);
        setMouseTracking(true);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        if (!m_owner)
        {
            return;
        }

        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        const Ant::DropdownPlacement placement = m_owner->m_renderPlacement;
        QRect body = rect().adjusted(8, m_owner->m_arrowVisible && placement != Ant::DropdownPlacement::Top &&
                                           placement != Ant::DropdownPlacement::TopLeft &&
                                           placement != Ant::DropdownPlacement::TopRight ? 14 : 8,
                                     -8,
                                     m_owner->m_arrowVisible && (placement == Ant::DropdownPlacement::Top ||
                                                                 placement == Ant::DropdownPlacement::TopLeft ||
                                                                 placement == Ant::DropdownPlacement::TopRight) ? -14 : -8);

        antTheme->drawEffectShadow(&painter, body, 12, token.borderRadiusLG, 0.55);
        painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter.setBrush(token.colorBgElevated);
        painter.drawRoundedRect(body, token.borderRadiusLG, token.borderRadiusLG);

        if (m_owner->m_arrowVisible)
        {
            QPolygonF arrow;
            const qreal centerX = placement == Ant::DropdownPlacement::BottomRight ||
                                          placement == Ant::DropdownPlacement::TopRight
                                      ? body.right() - 24
                                      : (placement == Ant::DropdownPlacement::BottomLeft ||
                                                 placement == Ant::DropdownPlacement::TopLeft
                                             ? body.left() + 24
                                             : body.center().x());

            if (placement == Ant::DropdownPlacement::Top ||
                placement == Ant::DropdownPlacement::TopLeft ||
                placement == Ant::DropdownPlacement::TopRight)
            {
                arrow << QPointF(centerX - 8, body.bottom())
                      << QPointF(centerX + 8, body.bottom())
                      << QPointF(centerX, rect().bottom() - 8);
            }
            else
            {
                arrow << QPointF(centerX - 8, body.top())
                      << QPointF(centerX + 8, body.top())
                      << QPointF(centerX, 8);
            }

            painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
            painter.setBrush(token.colorBgElevated);
            painter.drawPolygon(arrow);
        }
    }

    void enterEvent(QEnterEvent* event) override
    {
        if (m_owner)
        {
            m_owner->handlePopupEnter();
        }
        QFrame::enterEvent(event);
    }

    void leaveEvent(QEvent* event) override
    {
        if (m_owner)
        {
            m_owner->handlePopupLeave();
        }
        QFrame::leaveEvent(event);
    }

    void hideEvent(QHideEvent* event) override
    {
        if (m_owner && m_owner->m_open)
        {
            m_owner->m_open = false;
            qApp->removeEventFilter(m_owner);
            Q_EMIT m_owner->openChanged(false);
        }
        QFrame::hideEvent(event);
    }

private:
    AntDropdown* m_owner = nullptr;
};

AntDropdown::AntDropdown(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntDropdownStyle(style()));
    m_popup = new PopupFrame(this);
    auto* layout = new QVBoxLayout(m_popup);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(0);

    m_menu = new AntMenu(m_popup);
    m_menu->setMenuTheme(Ant::MenuTheme::Light);
    m_menu->setMode(Ant::MenuMode::Vertical);
    m_menu->setSelectable(false);
    m_menu->setCompact(true);
    layout->addWidget(m_menu);

    connect(m_menu, &AntMenu::itemClicked, this, [this](const QString& key) {
        Q_EMIT itemTriggered(key);
        setOpen(false);
    });

    m_openTimer = new QTimer(this);
    m_openTimer->setSingleShot(true);
    connect(m_openTimer, &QTimer::timeout, this, [this]() {
        if (m_trigger == Ant::DropdownTrigger::Hover)
        {
            setOpen(true);
        }
    });

    m_closeTimer = new QTimer(this);
    m_closeTimer->setSingleShot(true);
    connect(m_closeTimer, &QTimer::timeout, this, [this]() {
        if (m_trigger == Ant::DropdownTrigger::Hover)
        {
            setOpen(false);
        }
    });

    // Hover mode: once open, poll cursor position. Close only when the cursor
    // has stayed outside of (target ∪ popup) for `threshold` consecutive
    // ticks. This avoids edge flicker caused by Qt's Enter/Leave being
    // dispatched as the popup window appears on top.
    m_hoverTicker = new QTimer(this);
    m_hoverTicker->setInterval(60);
    connect(m_hoverTicker, &QTimer::timeout, this, [this]() {
        if (m_trigger != Ant::DropdownTrigger::Hover || !m_open)
        {
            m_hoverTicker->stop();
            return;
        }
        const QPoint gp = QCursor::pos();
        const bool inTarget = m_target &&
                              QRect(m_target->mapToGlobal(QPoint(0, 0)), m_target->size()).contains(gp);
        const bool inPopup = m_popup && m_popup->isVisible() && m_popup->geometry().contains(gp);
        if (inTarget || inPopup)
        {
            m_offTicks = 0;
        }
        else if (++m_offTicks >= 3) // ~180ms grace period
        {
            setOpen(false);
        }
    });

}

AntDropdown::~AntDropdown()
{
    uninstallTarget();
    if (m_popup)
    {
        m_popup->deleteLater();
    }
}

QStringList AntDropdown::itemLabels() const
{
    return m_itemLabels;
}

void AntDropdown::setItemLabels(const QStringList& labels)
{
    if (m_itemLabels == labels)
    {
        return;
    }
    clearItems();
    for (int i = 0; i < labels.size(); ++i)
    {
        addItem(QStringLiteral("item-%1").arg(i), labels.at(i));
    }
    Q_EMIT itemLabelsChanged(m_itemLabels);
}

Ant::DropdownPlacement AntDropdown::placement() const
{
    return m_placement;
}

Ant::DropdownPlacement AntDropdown::renderPlacement() const
{
    return m_renderPlacement;
}

void AntDropdown::setPlacement(Ant::DropdownPlacement placement)
{
    if (m_placement == placement)
    {
        return;
    }
    m_placement = placement;
    m_renderPlacement = placement;
    if (m_open)
    {
        updatePopupGeometry();
    }
    Q_EMIT placementChanged(m_placement);
}

Ant::DropdownTrigger AntDropdown::trigger() const
{
    return m_trigger;
}

void AntDropdown::setTrigger(Ant::DropdownTrigger trigger)
{
    if (m_trigger == trigger)
    {
        return;
    }
    m_trigger = trigger;
    Q_EMIT triggerChanged(m_trigger);
}

bool AntDropdown::arrowVisible() const
{
    return m_arrowVisible;
}

void AntDropdown::setArrowVisible(bool visible)
{
    if (m_arrowVisible == visible)
    {
        return;
    }
    m_arrowVisible = visible;
    if (m_open)
    {
        updatePopupGeometry();
    }
    Q_EMIT arrowVisibleChanged(m_arrowVisible);
}

bool AntDropdown::isOpen() const
{
    return m_open;
}

void AntDropdown::setOpen(bool open)
{
    if (m_open == open)
    {
        return;
    }
    m_open = open;
    m_openTimer->stop();
    m_closeTimer->stop();

    if (m_open && m_target)
    {
        updatePopupGeometry(m_lastContextPos);
        AntPopupMotion::show(m_popup, AntPopupMotion::fromDropdownPlacement(m_renderPlacement));
        qApp->installEventFilter(this);
        if (m_trigger == Ant::DropdownTrigger::Hover)
        {
            m_offTicks = 0;
            m_hoverTicker->start();
        }
    }
    else
    {
        m_hoverTicker->stop();
        qApp->removeEventFilter(this);
        AntPopupMotion::hide(m_popup, AntPopupMotion::fromDropdownPlacement(m_renderPlacement));
    }

    Q_EMIT openChanged(m_open);
}

QWidget* AntDropdown::target() const
{
    return m_target.data();
}

void AntDropdown::setTarget(QWidget* target)
{
    if (m_target == target)
    {
        return;
    }
    uninstallTarget();
    installTarget(target);
}

AntMenu* AntDropdown::menu() const
{
    return m_menu;
}

void AntDropdown::clearItems()
{
    m_itemLabels.clear();
    m_menu->clearItems();
}

void AntDropdown::addItem(const QString& key, const QString& label, const QString& iconText, bool disabled)
{
    m_itemLabels.append(label);
    m_menu->addItem(key, label, iconText, QString(), disabled, false);
}

void AntDropdown::addDivider()
{
    m_menu->addDivider();
}

bool AntDropdown::eventFilter(QObject* watched, QEvent* event)
{
    // Global click-outside handling (installed on qApp while popup is open).
    if (m_open && event->type() == QEvent::MouseButtonPress)
    {
        auto* me = static_cast<QMouseEvent*>(event);
        const QPoint gp = me->globalPosition().toPoint();
        const bool insidePopup = m_popup && m_popup->isVisible() &&
                                 m_popup->geometry().contains(gp);
        const bool insideTarget = m_target &&
                                  QRect(m_target->mapToGlobal(QPoint(0, 0)), m_target->size()).contains(gp);
        if (!insidePopup && !insideTarget)
        {
            setOpen(false);
            // Do not swallow — let the underlying widget still receive it.
        }
    }

    if (watched == m_target)
    {
        switch (event->type())
        {
        case QEvent::Enter:
            handleTargetEnter();
            break;
        case QEvent::Leave:
            handleTargetLeave();
            break;
        case QEvent::MouseButtonPress:
            if (m_trigger == Ant::DropdownTrigger::Click)
            {
                setOpen(!m_open);
                return true;
            }
            if (m_trigger == Ant::DropdownTrigger::ContextMenu)
            {
                auto* mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::RightButton)
                {
                    m_lastContextPos = mouseEvent->globalPosition().toPoint();
                    setOpen(true);
                    return true;
                }
            }
            break;
        case QEvent::ContextMenu:
            if (m_trigger == Ant::DropdownTrigger::ContextMenu)
            {
                setOpen(true);
                return true;
            }
            break;
        case QEvent::Move:
        case QEvent::Resize:
            if (m_open)
            {
                updatePopupGeometry();
            }
            break;
        case QEvent::Hide:
            setOpen(false);
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

QRect AntDropdown::popupGeometry(const QRect& targetRect, const QSize& popupSize, Ant::DropdownPlacement placement) const
{
    // PopupFrame keeps an 8px transparent shadow margin. A negative window gap
    // gives the painted panel the AntD-like 4px visual distance from target.
    const int gap = -4;
    switch (placement)
    {
    case Ant::DropdownPlacement::Top:
        return QRect(targetRect.center().x() - popupSize.width() / 2,
                     targetRect.top() - popupSize.height() - gap,
                     popupSize.width(),
                     popupSize.height());
    case Ant::DropdownPlacement::TopLeft:
        return QRect(targetRect.left(),
                     targetRect.top() - popupSize.height() - gap,
                     popupSize.width(),
                     popupSize.height());
    case Ant::DropdownPlacement::TopRight:
        return QRect(targetRect.right() - popupSize.width(),
                     targetRect.top() - popupSize.height() - gap,
                     popupSize.width(),
                     popupSize.height());
    case Ant::DropdownPlacement::Bottom:
        return QRect(targetRect.center().x() - popupSize.width() / 2,
                     targetRect.bottom() + gap,
                     popupSize.width(),
                     popupSize.height());
    case Ant::DropdownPlacement::BottomRight:
        return QRect(targetRect.right() - popupSize.width(),
                     targetRect.bottom() + gap,
                     popupSize.width(),
                     popupSize.height());
    case Ant::DropdownPlacement::BottomLeft:
    default:
        return QRect(targetRect.left(),
                     targetRect.bottom() + gap,
                     popupSize.width(),
                     popupSize.height());
    }
}

Ant::DropdownPlacement AntDropdown::resolvedPlacement(const QRect& targetRect,
                                                      const QSize& popupSize,
                                                      const QRect& screenRect) const
{
    const bool wantsTop = m_placement == Ant::DropdownPlacement::Top ||
                          m_placement == Ant::DropdownPlacement::TopLeft ||
                          m_placement == Ant::DropdownPlacement::TopRight;
    if (wantsTop && targetRect.top() - popupSize.height() + 4 < screenRect.top())
    {
        if (m_placement == Ant::DropdownPlacement::TopLeft)
            return Ant::DropdownPlacement::BottomLeft;
        if (m_placement == Ant::DropdownPlacement::TopRight)
            return Ant::DropdownPlacement::BottomRight;
        return Ant::DropdownPlacement::Bottom;
    }

    const bool wantsBottom = !wantsTop;
    if (wantsBottom && targetRect.bottom() + popupSize.height() - 4 > screenRect.bottom())
    {
        if (m_placement == Ant::DropdownPlacement::BottomLeft)
            return Ant::DropdownPlacement::TopLeft;
        if (m_placement == Ant::DropdownPlacement::BottomRight)
            return Ant::DropdownPlacement::TopRight;
        return Ant::DropdownPlacement::Top;
    }
    return m_placement;
}

int AntDropdown::popupContentWidth() const
{
    const auto& token = antTheme->tokens();
    QFont f = m_menu ? m_menu->font() : font();
    f.setPixelSize(token.fontSize);
    const QFontMetrics fm(f);
    int textWidth = 0;
    for (const QString& label : m_itemLabels)
    {
        textWidth = qMax(textWidth, fm.horizontalAdvance(label));
    }
    return qBound(96, textWidth + token.paddingSM * 2 + token.paddingXS, 260);
}

void AntDropdown::updatePopupGeometry(const QPoint& contextPos)
{
    if (!m_target)
    {
        return;
    }

    const bool topPlacement = m_placement == Ant::DropdownPlacement::Top ||
                              m_placement == Ant::DropdownPlacement::TopLeft ||
                              m_placement == Ant::DropdownPlacement::TopRight;
    const int topMargin = m_arrowVisible && !topPlacement ? 14 : 8;
    const int bottomMargin = m_arrowVisible && topPlacement ? 14 : 8;
    if (auto* box = qobject_cast<QVBoxLayout*>(m_popup->layout()))
    {
        box->setContentsMargins(12, topMargin + 4, 12, bottomMargin + 4);
    }
    m_menu->setFixedWidth(popupContentWidth());
    m_popup->adjustSize();
    const QSize popupSize = m_popup->sizeHint().expandedTo(
        QSize(m_menu->width() + 24, m_menu->sizeHint().height() + topMargin + bottomMargin + 8));
    QRect targetRect(m_target->mapToGlobal(QPoint(0, 0)), m_target->size());
    if (m_trigger == Ant::DropdownTrigger::ContextMenu && !contextPos.isNull())
    {
        targetRect = QRect(contextPos, QSize(1, 1));
    }
    const QRect screenRect = availableScreenGeometryFor(m_target);
    const Ant::DropdownPlacement placement = resolvedPlacement(targetRect, popupSize, screenRect);
    m_renderPlacement = placement;
    QRect geometry = popupGeometry(targetRect, popupSize, placement);
    geometry.moveLeft(qBound(screenRect.left() + 4, geometry.left(), screenRect.right() - geometry.width() - 4));
    geometry.moveTop(qBound(screenRect.top() + 4, geometry.top(), screenRect.bottom() - geometry.height() - 4));
    m_popup->setGeometry(geometry);
    m_popup->update();
}

void AntDropdown::installTarget(QWidget* target)
{
    m_target = target;
    if (m_target)
    {
        m_target->installEventFilter(this);
        m_target->setMouseTracking(true);
    }
}

void AntDropdown::uninstallTarget()
{
    if (m_target)
    {
        m_target->removeEventFilter(this);
    }
    m_target = nullptr;
}

void AntDropdown::handleTargetEnter()
{
    if (m_trigger == Ant::DropdownTrigger::Hover)
    {
        m_closeTimer->stop();
        m_offTicks = 0;
        if (!m_open)
        {
            m_openTimer->start(120);
        }
    }
}

void AntDropdown::handleTargetLeave()
{
    if (m_trigger != Ant::DropdownTrigger::Hover)
    {
        return;
    }
    m_openTimer->stop();
    // Do NOT start a close timer here. Closing is decided by m_hoverTicker,
    // which polls the real cursor position against (target ∪ popup). This
    // avoids flicker when Qt dispatches Leave to the target at the moment
    // the popup window is raised over the UI.
}

void AntDropdown::handlePopupEnter()
{
    if (m_trigger == Ant::DropdownTrigger::Hover)
    {
        m_closeTimer->stop();
        m_offTicks = 0;
    }
}

void AntDropdown::handlePopupLeave()
{
    // Hover ticker handles close; nothing to do here.
    Q_UNUSED(0);
}
