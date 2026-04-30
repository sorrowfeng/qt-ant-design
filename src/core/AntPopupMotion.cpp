#include "AntPopupMotion.h"

#include <QAbstractAnimation>
#include <QHash>
#include <QParallelAnimationGroup>
#include <QPointer>
#include <QPropertyAnimation>
#include <QSet>

namespace
{
constexpr int kEnterDuration = 160;
constexpr int kLeaveDuration = 120;

QHash<const QWidget*, QPointer<QAbstractAnimation>>& activeAnimations()
{
    static QHash<const QWidget*, QPointer<QAbstractAnimation>> animations;
    return animations;
}

QSet<const QWidget*>& closingPopups()
{
    static QSet<const QWidget*> popups;
    return popups;
}

QPoint motionOffset(AntPopupMotion::Placement placement, int distance)
{
    switch (placement)
    {
    case AntPopupMotion::Placement::Top:
        return QPoint(0, distance);
    case AntPopupMotion::Placement::Bottom:
        return QPoint(0, -distance);
    case AntPopupMotion::Placement::Left:
        return QPoint(distance, 0);
    case AntPopupMotion::Placement::Right:
        return QPoint(-distance, 0);
    case AntPopupMotion::Placement::Center:
    default:
        return QPoint(0, 0);
    }
}

QPropertyAnimation* makeAnimation(QObject* target,
                                  const QByteArray& property,
                                  const QVariant& start,
                                  const QVariant& end,
                                  int duration,
                                  QEasingCurve easing,
                                  QObject* parent)
{
    auto* animation = new QPropertyAnimation(target, property, parent);
    animation->setDuration(duration);
    animation->setEasingCurve(easing);
    animation->setStartValue(start);
    animation->setEndValue(end);
    return animation;
}
} // namespace

void AntPopupMotion::show(QWidget* popup, Placement placement, int distance)
{
    if (!popup)
    {
        return;
    }

    stop(popup);
    closingPopups().remove(popup);

    const QPoint finalPos = popup->pos();
    const QPoint startPos = finalPos + motionOffset(placement, distance);

    popup->setWindowOpacity(0.0);
    popup->move(startPos);
    popup->show();
    popup->raise();

    auto* group = new QParallelAnimationGroup(popup);
    group->addAnimation(makeAnimation(popup, "windowOpacity", 0.0, 1.0, kEnterDuration, QEasingCurve::OutCubic, group));
    group->addAnimation(makeAnimation(popup, "pos", startPos, finalPos, kEnterDuration, QEasingCurve::OutCubic, group));

    activeAnimations().insert(popup, group);
    QObject::connect(popup, &QObject::destroyed, group, [popup]() {
        activeAnimations().remove(popup);
        closingPopups().remove(popup);
    });
    QObject::connect(group, &QParallelAnimationGroup::finished, group, [popup, group, finalPos]() {
        if (activeAnimations().value(popup) == group)
        {
            activeAnimations().remove(popup);
            if (popup)
            {
                popup->move(finalPos);
                popup->setWindowOpacity(1.0);
            }
        }
        group->deleteLater();
    });
    group->start();
}

void AntPopupMotion::hide(QWidget* popup, Placement placement, int distance)
{
    if (!popup || !popup->isVisible())
    {
        return;
    }

    stop(popup);
    closingPopups().insert(popup);

    const QPoint startPos = popup->pos();
    const QPoint finalPos = startPos + motionOffset(placement, distance);

    auto* group = new QParallelAnimationGroup(popup);
    group->addAnimation(makeAnimation(popup, "windowOpacity", popup->windowOpacity(), 0.0, kLeaveDuration, QEasingCurve::OutCubic, group));
    group->addAnimation(makeAnimation(popup, "pos", startPos, finalPos, kLeaveDuration, QEasingCurve::OutCubic, group));

    activeAnimations().insert(popup, group);
    QObject::connect(popup, &QObject::destroyed, group, [popup]() {
        activeAnimations().remove(popup);
        closingPopups().remove(popup);
    });
    QObject::connect(group, &QParallelAnimationGroup::finished, group, [popup, group, startPos]() {
        if (activeAnimations().value(popup) == group)
        {
            activeAnimations().remove(popup);
            closingPopups().remove(popup);
            if (popup)
            {
                popup->hide();
                popup->move(startPos);
                popup->setWindowOpacity(1.0);
            }
        }
        group->deleteLater();
    });
    group->start();
}

void AntPopupMotion::close(QWidget* popup, Placement placement, int distance)
{
    if (!popup)
    {
        return;
    }
    if (!popup->isVisible())
    {
        popup->close();
        return;
    }

    stop(popup);
    closingPopups().insert(popup);

    const QPoint startPos = popup->pos();
    const QPoint finalPos = startPos + motionOffset(placement, distance);

    auto* group = new QParallelAnimationGroup(popup);
    group->addAnimation(makeAnimation(popup, "windowOpacity", popup->windowOpacity(), 0.0, kLeaveDuration, QEasingCurve::OutCubic, group));
    group->addAnimation(makeAnimation(popup, "pos", startPos, finalPos, kLeaveDuration, QEasingCurve::OutCubic, group));

    activeAnimations().insert(popup, group);
    QObject::connect(popup, &QObject::destroyed, group, [popup]() {
        activeAnimations().remove(popup);
        closingPopups().remove(popup);
    });
    QObject::connect(group, &QParallelAnimationGroup::finished, group, [popup, group, startPos]() {
        if (activeAnimations().value(popup) == group)
        {
            activeAnimations().remove(popup);
            closingPopups().remove(popup);
            if (popup)
            {
                popup->move(startPos);
                popup->setWindowOpacity(1.0);
                popup->close();
            }
        }
        group->deleteLater();
    });
    group->start();
}

void AntPopupMotion::stop(QWidget* popup)
{
    if (!popup)
    {
        return;
    }

    if (QPointer<QAbstractAnimation> animation = activeAnimations().take(popup))
    {
        animation->stop();
        animation->deleteLater();
    }
    popup->setWindowOpacity(1.0);
    closingPopups().remove(popup);
}

bool AntPopupMotion::isClosing(const QWidget* popup)
{
    return popup && closingPopups().contains(popup);
}

AntPopupMotion::Placement AntPopupMotion::fromPlacement(Ant::Placement placement)
{
    switch (placement)
    {
    case Ant::Placement::Top:
    case Ant::Placement::TopLeft:
    case Ant::Placement::TopRight:
        return Placement::Top;
    case Ant::Placement::Bottom:
    case Ant::Placement::BottomLeft:
    case Ant::Placement::BottomRight:
    default:
        return Placement::Bottom;
    }
}

AntPopupMotion::Placement AntPopupMotion::fromDropdownPlacement(Ant::DropdownPlacement placement)
{
    switch (placement)
    {
    case Ant::DropdownPlacement::Top:
    case Ant::DropdownPlacement::TopLeft:
    case Ant::DropdownPlacement::TopRight:
        return Placement::Top;
    case Ant::DropdownPlacement::Bottom:
    case Ant::DropdownPlacement::BottomLeft:
    case Ant::DropdownPlacement::BottomRight:
    default:
        return Placement::Bottom;
    }
}

AntPopupMotion::Placement AntPopupMotion::fromTooltipPlacement(Ant::TooltipPlacement placement)
{
    switch (placement)
    {
    case Ant::TooltipPlacement::Top:
    case Ant::TooltipPlacement::TopLeft:
    case Ant::TooltipPlacement::TopRight:
        return Placement::Top;
    case Ant::TooltipPlacement::Bottom:
    case Ant::TooltipPlacement::BottomLeft:
    case Ant::TooltipPlacement::BottomRight:
        return Placement::Bottom;
    case Ant::TooltipPlacement::Left:
        return Placement::Left;
    case Ant::TooltipPlacement::Right:
    default:
        return Placement::Right;
    }
}
