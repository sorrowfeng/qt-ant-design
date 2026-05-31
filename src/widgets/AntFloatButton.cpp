#include "AntFloatButton.h"

#include <QCursor>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QScrollBar>

#include "core/AntTheme.h"
#include "core/AntWave.h"
#include "styles/AntFloatButtonStyle.h"

AntFloatButton::AntFloatButton(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntFloatButtonStyle>(this);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setFixedSize(sizeHint());

    m_pressAnimation = new QPropertyAnimation(this, "pressProgress", this);

    m_positionTimer = new QTimer(this);
    m_positionTimer->setSingleShot(true);
    m_positionTimer->setInterval(50);
    connect(m_positionTimer, &QTimer::timeout, this, &AntFloatButton::updatePosition);

    installEventFilter(this);
    if (parentWidget())
    {
        parentWidget()->installEventFilter(this);
    }

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateContentSize();
        m_childLayoutDirty = true;
        if (m_open)
        {
            layoutChildren();
        }
        update();
    });

    syncFloatButtonPerfCounters();
}

QString AntFloatButton::icon() const { return m_icon; }

void AntFloatButton::setIcon(const QString& icon)
{
    if (m_icon == icon) return;
    m_icon = icon;
    update();
    Q_EMIT iconChanged(m_icon);
}

QString AntFloatButton::content() const { return m_content; }

void AntFloatButton::setContent(const QString& content)
{
    if (m_content == content) return;
    m_content = content;
    updateContentSize();
    update();
    Q_EMIT contentChanged(m_content);
}

Ant::FloatButtonType AntFloatButton::floatButtonType() const { return m_type; }

void AntFloatButton::setFloatButtonType(Ant::FloatButtonType type)
{
    if (m_type == type) return;
    m_type = type;
    update();
    Q_EMIT floatButtonTypeChanged(m_type);
}

Ant::FloatButtonShape AntFloatButton::floatButtonShape() const { return m_shape; }

void AntFloatButton::setFloatButtonShape(Ant::FloatButtonShape shape)
{
    if (m_shape == shape) return;
    m_shape = shape;
    updateContentSize();
    update();
    Q_EMIT floatButtonShapeChanged(m_shape);
}

bool AntFloatButton::badgeDot() const { return m_badgeDot; }

void AntFloatButton::setBadgeDot(bool dot)
{
    if (m_badgeDot == dot && (!dot || m_badgeCount == 0))
    {
        return;
    }
    m_badgeDot = dot;
    m_badgeCount = 0;
    update();
}

int AntFloatButton::badgeCount() const { return m_badgeCount; }

void AntFloatButton::setBadgeCount(int count)
{
    if (m_badgeCount == count && !m_badgeDot)
    {
        return;
    }
    m_badgeCount = count;
    m_badgeDot = false;
    update();
}

void AntFloatButton::addChild(AntFloatButton* child)
{
    if (!child || m_children.contains(child)) return;
    m_children.append(child);
    child->setParent(parentWidget() ? parentWidget() : this);
    child->hide();
    child->installEventFilter(this);
    m_childLayoutDirty = true;
    if (m_open)
    {
        layoutChildren();
    }
}

void AntFloatButton::removeChild(AntFloatButton* child)
{
    if (!child) return;
    child->removeEventFilter(this);
    m_children.removeOne(child);
    child->hide();
    m_childLayoutDirty = true;
    if (m_open)
    {
        layoutChildren();
    }
}

QVector<AntFloatButton*> AntFloatButton::childButtons() const { return m_children; }

Ant::Trigger AntFloatButton::groupTrigger() const { return m_groupTrigger; }

void AntFloatButton::setGroupTrigger(Ant::Trigger trigger)
{
    if (m_groupTrigger == trigger)
    {
        return;
    }
    m_groupTrigger = trigger;
}

bool AntFloatButton::isOpen() const { return m_open; }

void AntFloatButton::setOpen(bool open)
{
    if (m_open == open) return;
    m_open = open;
    m_childLayoutDirty = true;
    layoutChildren();
    update();
    Q_EMIT openChanged(m_open);
}

QString AntFloatButton::closeIcon() const { return m_closeIcon; }

void AntFloatButton::setCloseIcon(const QString& icon)
{
    if (m_closeIcon == icon)
    {
        return;
    }
    m_closeIcon = icon;
    if (m_open) update();
}

bool AntFloatButton::isBackTop() const { return m_backTop; }

void AntFloatButton::setBackTop(bool backTop)
{
    if (m_backTop == backTop) return;
    m_backTop = backTop;
    if (m_backTop)
    {
        if (m_icon.isEmpty()) m_icon = QStringLiteral("up");
        hide();
    }
    update();
}

int AntFloatButton::visibilityHeight() const { return m_visibilityHeight; }

void AntFloatButton::setVisibilityHeight(int height)
{
    if (m_visibilityHeight == height)
    {
        return;
    }
    m_visibilityHeight = height;
}

int AntFloatButton::scrollDuration() const { return m_scrollDuration; }

void AntFloatButton::setScrollDuration(int duration)
{
    if (m_scrollDuration == duration)
    {
        return;
    }
    m_scrollDuration = duration;
}

QWidget* AntFloatButton::scrollTarget() const { return m_scrollTarget; }

void AntFloatButton::setScrollTarget(QWidget* target)
{
    if (m_scrollTarget == target) return;
    if (m_scrollTarget)
    {
        m_scrollTarget->removeEventFilter(this);
    }
    m_scrollTarget = target;
    if (m_scrollTarget)
    {
        m_scrollTarget->installEventFilter(this);
    }
}

Ant::FloatButtonPlacement AntFloatButton::placement() const { return m_placement; }

void AntFloatButton::setPlacement(Ant::FloatButtonPlacement placement)
{
    if (m_placement == placement) return;
    m_placement = placement;
    m_positionTimer->start();
    Q_EMIT placementChanged(m_placement);
}

QSize AntFloatButton::sizeHint() const
{
    return visualButtonSize() + QSize(ShadowMargin * 2, ShadowMargin * 2);
}

QSize AntFloatButton::minimumSizeHint() const
{
    return sizeHint();
}

void AntFloatButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntFloatButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (!buttonRect().contains(event->pos()))
        {
            QWidget::mousePressEvent(event);
            return;
        }

        m_pressed = true;
        startPressAnimation(1.0, 90);
        update();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntFloatButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed)
    {
        const bool clickedInside = buttonRect().contains(event->pos());
        m_pressed = false;
        startPressAnimation(0.0, 170);
        update();

        if (!clickedInside)
        {
            event->accept();
            return;
        }

        const QRect waveRect = buttonRect();
        const int waveRadius = m_shape == Ant::FloatButtonShape::Circle
            ? waveRect.height() / 2
            : antTheme->tokens().borderRadius;
        AntWave::triggerRect(this, waveRect, antTheme->tokens().colorPrimary, waveRadius, true);

        if (m_backTop)
        {
            animateScrollToTop();
            Q_EMIT backTopClicked();
            Q_EMIT clicked();
            return;
        }

        if (!m_children.isEmpty())
        {
            setOpen(!m_open);
        }

        Q_EMIT clicked();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void AntFloatButton::mouseMoveEvent(QMouseEvent* event)
{
    updateHoverState(event->pos());
    QWidget::mouseMoveEvent(event);
}

void AntFloatButton::enterEvent(AntEnterEvent* event)
{
    updateHoverState(mapFromGlobal(QCursor::pos()));
    if (!m_children.isEmpty() && m_groupTrigger == Ant::Trigger::Hover)
    {
        setOpen(true);
    }
    update();
    QWidget::enterEvent(event);
}

void AntFloatButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    if (m_pressed || m_pressProgress > 0.0)
    {
        m_pressed = false;
        startPressAnimation(0.0, 170);
    }
    // Don't close on hover leave immediately — let user move to children
    update();
    QWidget::leaveEvent(event);
}

void AntFloatButton::showEvent(QShowEvent* event)
{
    updatePosition();
    QWidget::showEvent(event);
}

bool AntFloatButton::eventFilter(QObject* watched, QEvent* event)
{
    if (m_backTop && watched == m_scrollTarget)
    {
        if (event->type() == QEvent::Wheel || event->type() == QEvent::Resize ||
            event->type() == QEvent::Scroll)
        {
            checkBackTopVisibility();
        }
    }
    if (watched == this && event->type() == QEvent::ParentChange)
    {
        if (parentWidget())
        {
            parentWidget()->installEventFilter(this);
        }
        updatePosition();
    }
    if (watched == parentWidget() &&
        (event->type() == QEvent::Resize || event->type() == QEvent::Show))
    {
        m_positionTimer->start();
    }
    // Close group on click outside
    if (m_open && event->type() == QEvent::MouseButtonPress && watched != this)
    {
        bool clickOnChild = false;
        for (auto* child : m_children)
        {
            if (watched == child)
            {
                clickOnChild = true;
                break;
            }
        }
        if (!clickOnChild)
        {
            setOpen(false);
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntFloatButton::updatePosition()
{
    QWidget* p = parentWidget();
    if (!p) return;

    const int margin = antTheme->tokens().margin;
    const int x = (m_placement == Ant::FloatButtonPlacement::BottomRight || m_placement == Ant::FloatButtonPlacement::TopRight)
                      ? p->width() - width() - margin + ShadowMargin
                      : margin - ShadowMargin;
    const int y = (m_placement == Ant::FloatButtonPlacement::BottomRight || m_placement == Ant::FloatButtonPlacement::BottomLeft)
                      ? p->height() - height() - margin + ShadowMargin
                      : margin - ShadowMargin;

    const QPoint targetPos(x, y);
    const bool moved = pos() != targetPos;
    if (moved)
    {
        move(targetPos);
        m_childLayoutDirty = true;
        ++m_positionApplyCount;
    }
    else
    {
        ++m_positionSkipCount;
    }

    if (m_open && (moved || m_childLayoutDirty))
    {
        layoutChildren();
    }
    syncFloatButtonPerfCounters();
}

void AntFloatButton::layoutChildren()
{
    const int spacing = 12;
    int yOffset = 0;
    bool changed = false;

    for (int i = m_children.size() - 1; i >= 0; --i)
    {
        auto* child = m_children[i];
        if (m_open)
        {
            const QSize childSize = child->sizeHint();
            yOffset -= (VisualButtonSize + spacing);
            const QRect childGeometry(pos().x() + (width() - childSize.width()) / 2,
                                      pos().y() + yOffset,
                                      childSize.width(),
                                      childSize.height());
            bool childChanged = false;
            if (child->geometry() != childGeometry)
            {
                child->setGeometry(childGeometry);
                childChanged = true;
            }
            if (!child->isVisible())
            {
                child->show();
                childChanged = true;
            }
            if (childChanged)
            {
                child->raise();
                changed = true;
            }
        }
        else
        {
            if (child->isVisible())
            {
                child->hide();
                changed = true;
            }
        }
    }
    if (changed)
    {
        raise();
        ++m_childLayoutApplyCount;
    }
    else
    {
        ++m_childLayoutSkipCount;
    }
    m_childLayoutDirty = false;
    syncFloatButtonPerfCounters();
}

void AntFloatButton::updateContentSize()
{
    const QSize targetSize = sizeHint();

    if (size() == targetSize && minimumSize() == targetSize && maximumSize() == targetSize)
    {
        return;
    }

    setFixedSize(targetSize);
    m_childLayoutDirty = true;
    ++m_contentSizeApplyCount;
    syncFloatButtonPerfCounters();
}

QSize AntFloatButton::visualButtonSize() const
{
    QFont f = font();
    f.setPixelSize(antTheme->tokens().fontSizeSM);
    const int textWidth = m_content.isEmpty() ? 0 : QFontMetrics(f).horizontalAdvance(m_content);
    if (!m_content.isEmpty() && m_shape == Ant::FloatButtonShape::Square)
    {
        return QSize(qMax(VisualButtonSize, textWidth + 32), VisualButtonSize);
    }
    return QSize(VisualButtonSize, VisualButtonSize);
}

bool AntFloatButton::isHoveredState() const
{
    return m_hovered;
}

bool AntFloatButton::isPressedState() const
{
    return m_pressed;
}

qreal AntFloatButton::pressProgress() const
{
    return m_pressProgress;
}

void AntFloatButton::setPressProgress(qreal progress)
{
    const qreal normalized = qBound<qreal>(0.0, progress, 1.0);
    if (qFuzzyCompare(m_pressProgress, normalized))
    {
        return;
    }

    m_pressProgress = normalized;
    update();
}

QRect AntFloatButton::buttonRect() const
{
    const QSize visualSize = visualButtonSize();
    return QRect(QPoint((width() - visualSize.width()) / 2,
                        (height() - visualSize.height()) / 2),
                 visualSize);
}

void AntFloatButton::startPressAnimation(qreal endValue, int duration)
{
    if (!m_pressAnimation)
    {
        return;
    }

    m_pressAnimation->stop();
    m_pressAnimation->setDuration(duration);
    m_pressAnimation->setStartValue(m_pressProgress);
    m_pressAnimation->setEndValue(qBound<qreal>(0.0, endValue, 1.0));
    m_pressAnimation->setEasingCurve(endValue > m_pressProgress ? QEasingCurve::OutCubic
                                                                 : QEasingCurve::OutBack);
    m_pressAnimation->start();
}

void AntFloatButton::updateHoverState(const QPoint& pos)
{
    const bool hovered = buttonRect().contains(pos);
    if (m_hovered == hovered)
    {
        return;
    }
    m_hovered = hovered;
    update();
}

void AntFloatButton::checkBackTopVisibility()
{
    if (!m_backTop || !m_scrollTarget) return;

    QScrollArea* area = qobject_cast<QScrollArea*>(m_scrollTarget);
    int scrollValue = 0;
    if (area && area->verticalScrollBar())
    {
        scrollValue = area->verticalScrollBar()->value();
    }

    if (scrollValue > m_visibilityHeight)
    {
        if (!isVisible())
        {
            show();
            raise();
        }
    }
    else
    {
        if (isVisible()) hide();
    }
}

void AntFloatButton::animateScrollToTop()
{
    QScrollArea* area = qobject_cast<QScrollArea*>(m_scrollTarget);
    if (!area || !area->verticalScrollBar()) return;

    QScrollBar* bar = area->verticalScrollBar();
    auto* anim = new QPropertyAnimation(bar, "value", this);
    anim->setDuration(m_scrollDuration);
    anim->setStartValue(bar->value());
    anim->setEndValue(0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AntFloatButton::syncFloatButtonPerfCounters() const
{
    auto* self = const_cast<AntFloatButton*>(this);
    self->setProperty("antFloatButtonPositionApplyCount", m_positionApplyCount);
    self->setProperty("antFloatButtonPositionSkipCount", m_positionSkipCount);
    self->setProperty("antFloatButtonChildLayoutApplyCount", m_childLayoutApplyCount);
    self->setProperty("antFloatButtonChildLayoutSkipCount", m_childLayoutSkipCount);
    self->setProperty("antFloatButtonContentSizeApplyCount", m_contentSizeApplyCount);
}
