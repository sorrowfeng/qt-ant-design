#include "AntModalStyle.h"

#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "widgets/AntModal.h"

AntModalStyle::AntModalStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntModal*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->updateGeometry();
                w->update();
            }
        }
    });
}

void AntModalStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntModal*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntModalStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntModal*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntModalStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntModal*>(widget))
    {
        drawModal(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntModalStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntModalStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* modal = qobject_cast<AntModal*>(watched);
    if (modal && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(modal);
        option.rect = modal->rect();
        QPainter painter(modal);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, modal);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntModalStyle::drawModal(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* modal = qobject_cast<const AntModal*>(widget);
    if (!modal || !painter || !option)
    {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    const qreal baseOpacity = antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.58 : 0.45;
    // Fade mask alongside the open/close animation
    const qreal opacity = baseOpacity * modal->animationProgress();
    painter->fillRect(option->rect, AntPalette::alpha(Qt::black, opacity));
    painter->restore();
}
