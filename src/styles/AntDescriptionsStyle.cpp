#include "AntDescriptionsStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntDescriptions.h"

AntDescriptionsStyle::AntDescriptionsStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntDescriptionsStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    installPaintFilter<AntDescriptions>(widget);
}

void AntDescriptionsStyle::unpolish(QWidget* widget)
{
    removePaintFilter<AntDescriptions>(widget);
    AntStyleBase::unpolish(widget);
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

bool AntDescriptionsStyle::drawWidget(QWidget* widget, QPaintEvent* event)
{
    auto* descriptions = qobject_cast<AntDescriptions*>(widget);
    if (!descriptions)
    {
        return false;
    }

    QStyleOption option;
    option.initFrom(descriptions);
    option.rect = descriptions->rect();
    QPainter painter(descriptions);
    drawPrimitive(QStyle::PE_Widget, &option, &painter, descriptions);

    return true;
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
