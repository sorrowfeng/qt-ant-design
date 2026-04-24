#include "AntFormStyle.h"

#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "widgets/AntForm.h"

AntFormStyle::AntFormStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntForm*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->updateGeometry();
                w->update();
            }
        }
    });
}

void AntFormStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntForm*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntFormStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntForm*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntFormStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntForm*>(widget))
    {
        drawForm(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntFormStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntFormStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* form = qobject_cast<AntForm*>(watched);
    if (form && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(form);
        option.rect = form->rect();
        QPainter painter(form);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, form);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntFormStyle::drawForm(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* form = qobject_cast<const AntForm*>(widget);
    if (!form || !painter || !option)
    {
        return;
    }

    // AntForm does not override paintEvent. It uses child AntFormItem widgets
    // arranged via QBoxLayout for all visual content. Nothing to draw here.
    Q_UNUSED(form)
    Q_UNUSED(option)
    Q_UNUSED(painter)
}
