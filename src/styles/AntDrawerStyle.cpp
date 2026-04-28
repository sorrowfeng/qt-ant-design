#include "AntDrawerStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntDrawer.h"

namespace
{
void drawPanelShadow(QPainter* painter, const QStyleOption* option, const AntDrawer* drawer)
{
    const QRect panel = drawer->currentPanelGeometry();
    if (panel.isEmpty())
    {
        return;
    }

    constexpr int ShadowSize = 32;
    const qreal progress = drawer->maskProgress();
    QColor nearEdge = AntPalette::alpha(Qt::black, 0.16 * progress);
    QColor midEdge = AntPalette::alpha(Qt::black, 0.08 * progress);
    QColor farEdge = AntPalette::alpha(Qt::black, 0.0);

    QRect shadowRect;
    QLinearGradient gradient;
    switch (drawer->placement())
    {
    case Ant::DrawerPlacement::Right:
        shadowRect = QRect(panel.left() - ShadowSize, panel.top(), ShadowSize, panel.height());
        gradient = QLinearGradient(shadowRect.topLeft(), shadowRect.topRight());
        gradient.setColorAt(0.0, farEdge);
        gradient.setColorAt(0.55, midEdge);
        gradient.setColorAt(1.0, nearEdge);
        break;
    case Ant::DrawerPlacement::Left:
        shadowRect = QRect(panel.right() + 1, panel.top(), ShadowSize, panel.height());
        gradient = QLinearGradient(shadowRect.topRight(), shadowRect.topLeft());
        gradient.setColorAt(0.0, farEdge);
        gradient.setColorAt(0.55, midEdge);
        gradient.setColorAt(1.0, nearEdge);
        break;
    case Ant::DrawerPlacement::Bottom:
        shadowRect = QRect(panel.left(), panel.top() - ShadowSize, panel.width(), ShadowSize);
        gradient = QLinearGradient(shadowRect.topLeft(), shadowRect.bottomLeft());
        gradient.setColorAt(0.0, farEdge);
        gradient.setColorAt(0.55, midEdge);
        gradient.setColorAt(1.0, nearEdge);
        break;
    case Ant::DrawerPlacement::Top:
        shadowRect = QRect(panel.left(), panel.bottom() + 1, panel.width(), ShadowSize);
        gradient = QLinearGradient(shadowRect.bottomLeft(), shadowRect.topLeft());
        gradient.setColorAt(0.0, farEdge);
        gradient.setColorAt(0.55, midEdge);
        gradient.setColorAt(1.0, nearEdge);
        break;
    }

    shadowRect = shadowRect.intersected(option->rect);
    if (shadowRect.isEmpty())
    {
        return;
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(gradient);
    painter->drawRect(shadowRect);
}
} // namespace

AntDrawerStyle::AntDrawerStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntDrawer>();
}

void AntDrawerStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntDrawer*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntDrawerStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntDrawer*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntDrawerStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntDrawer*>(widget))
    {
        drawDrawer(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntDrawerStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntDrawerStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* drawer = qobject_cast<AntDrawer*>(watched);
    if (drawer && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(drawer);
        option.rect = drawer->rect();
        QPainter painter(drawer);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, drawer);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntDrawerStyle::drawDrawer(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* drawer = qobject_cast<const AntDrawer*>(widget);
    if (!drawer || !painter || !option)
    {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    const qreal baseOpacity = antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.58 : 0.45;
    const qreal opacity = baseOpacity * drawer->maskProgress();
    painter->fillRect(option->rect, AntPalette::alpha(Qt::black, opacity));
    drawPanelShadow(painter, option, drawer);
    painter->restore();
}
