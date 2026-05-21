#include "AntAlertStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntAlert.h"

AntAlertStyle::AntAlertStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntAlertStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntAlert*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntAlertStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntAlert*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntAlertStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntAlert*>(widget))
    {
        drawAlert(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntAlertStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntAlertStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* alert = qobject_cast<AntAlert*>(watched);
    if (!alert)
    {
        return QProxyStyle::eventFilter(watched, event);
    }

    if (event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(alert);
        option.rect = alert->rect();
        QPainter painter(alert);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, alert);
        return true;
    }

    return QProxyStyle::eventFilter(watched, event);
}

void AntAlertStyle::drawAlert(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* alert = qobject_cast<const AntAlert*>(widget);
    if (!alert || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const auto& layout = alert->alertLayout();
    const auto& m = layout.metrics;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // Background and border
    AntStyleBase::drawCrispRoundedRect(painter, option->rect,
        alert->isBanner() ? Qt::NoPen : QPen(alert->borderColor(), token.lineWidth),
        alert->backgroundColor(), m.radius, m.radius);

    // Icon
    if (layout.showIconEffective)
    {
        painter->drawPixmap(layout.iconRect.topLeft(),
                            alert->cachedIconPixmap(alert->iconTypeForAlert(), alert->iconColor(), m.iconSize));
    }

    // Close button and text right boundary
    if (alert->isClosable())
    {
        if (alert->m_hoverClose)
        {
            AntStyleBase::drawCrispRoundedRect(painter, layout.closeRect,
                Qt::NoPen, AntPalette::alpha(token.colorTextTertiary, 0.12),
                token.borderRadiusXS, token.borderRadiusXS);
        }
        painter->setPen(QPen(token.colorTextTertiary, 1.5, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(layout.closeRect.center() + QPoint(-4, -4), layout.closeRect.center() + QPoint(4, 4));
        painter->drawLine(layout.closeRect.center() + QPoint(4, -4), layout.closeRect.center() + QPoint(-4, 4));
    }

    // Title
    QFont titleFont = painter->font();
    titleFont.setPixelSize(m.titleFontSize);
    titleFont.setWeight(layout.hasDescription ? QFont::DemiBold : QFont::Normal);
    painter->setFont(titleFont);
    painter->setPen(alert->titleColor());

    painter->drawText(layout.titleRect,
                      Qt::AlignLeft | (layout.hasDescription ? Qt::AlignTop : Qt::AlignVCenter),
                      alert->title());

    // Description
    if (layout.hasDescription)
    {
        QFont descFont = painter->font();
        descFont.setPixelSize(m.descFontSize);
        descFont.setWeight(QFont::Normal);
        painter->setFont(descFont);
        painter->setPen(alert->descriptionColor());
        painter->drawText(layout.descriptionRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, alert->description());
    }

    painter->restore();
}
