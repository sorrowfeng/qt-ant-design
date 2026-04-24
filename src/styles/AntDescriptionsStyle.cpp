#include "AntDescriptionsStyle.h"

#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "widgets/AntDescriptions.h"

AntDescriptionsStyle::AntDescriptionsStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntDescriptions*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->updateGeometry();
                w->update();
            }
        }
    });
}

void AntDescriptionsStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntDescriptions*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntDescriptionsStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntDescriptions*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntDescriptionsStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntDescriptions*>(widget))
    {
        drawDescriptions(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntDescriptionsStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntDescriptionsStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* descriptions = qobject_cast<AntDescriptions*>(watched);
    if (descriptions && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(descriptions);
        option.rect = descriptions->rect();
        QPainter painter(descriptions);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, descriptions);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntDescriptionsStyle::drawDescriptions(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* descriptions = qobject_cast<const AntDescriptions*>(widget);
    if (!descriptions || !painter || !option)
    {
        return;
    }

    // AntDescriptions::paintEvent delegates to QWidget::paintEvent — no custom
    // painting is performed. The component uses child widgets (QLabel, QGridLayout)
    // for all visual content. Nothing to draw in the style.
    Q_UNUSED(descriptions)
    Q_UNUSED(option)
    Q_UNUSED(painter)
}
