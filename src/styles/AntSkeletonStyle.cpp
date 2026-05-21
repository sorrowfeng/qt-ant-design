#include "AntSkeletonStyle.h"

#include <QEvent>
#include <QLinearGradient>
#include <QPainter>
#include <QStyleOption>

#include "core/AntStyleBase.h"
#include "widgets/AntSkeleton.h"

AntSkeletonStyle::AntSkeletonStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntSkeletonStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntSkeleton*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntSkeletonStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntSkeleton*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntSkeletonStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntSkeleton*>(widget))
    {
        drawSkeleton(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntSkeletonStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntSkeletonStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* skeleton = qobject_cast<AntSkeleton*>(watched);
    if (skeleton && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(skeleton);
        option.rect = skeleton->rect();
        QPainter painter(skeleton);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, skeleton);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntSkeletonStyle::drawSkeleton(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* skeleton = qobject_cast<const AntSkeleton*>(widget);
    if (!skeleton || !painter || !option)
    {
        return;
    }

    if (!skeleton->isLoading())
    {
        return;
    }

    Q_UNUSED(option)

    const auto& layout = skeleton->skeletonLayout();
    if (!layout.loading || layout.placeholders.isEmpty())
    {
        return;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter->setPen(Qt::NoPen);

    for (const auto& item : layout.placeholders)
    {
        const QRectF& rect = item.rect;
        QBrush brush(layout.baseColor);
        if (layout.active)
        {
            const qreal bandWidth = qMax<qreal>(80.0, rect.width() * 0.72);
            const qreal travel = rect.width() + bandWidth * 2.0;
            const qreal phase = static_cast<qreal>(skeleton->shimmerOffset() % qMax(1, static_cast<int>(travel)))
                                / travel;
            const qreal bandLeft = rect.left() - bandWidth + travel * phase;
            QLinearGradient gradient(QPointF(bandLeft, rect.center().y()),
                                     QPointF(bandLeft + bandWidth, rect.center().y()));
            gradient.setColorAt(0.0, layout.baseColor);
            gradient.setColorAt(0.42, layout.baseColor);
            gradient.setColorAt(0.5, layout.highlightColor);
            gradient.setColorAt(0.58, layout.baseColor);
            gradient.setColorAt(1.0, layout.baseColor);
            brush = QBrush(gradient);
        }

        painter->fillPath(item.path, brush);

        if (item.imageElement)
        {
            painter->setPen(layout.highlightColor);
            painter->setBrush(Qt::NoBrush);
            const qreal iconSize = qMin(rect.width(), rect.height()) * 0.3;
            const qreal cx = rect.center().x();
            const qreal cy = rect.center().y();
            QRectF iconRect(cx - iconSize / 2, cy - iconSize / 2, iconSize, iconSize);
            painter->drawRect(iconRect);
            // Draw mountain/triangle icon
            QPolygonF tri;
            tri << QPointF(iconRect.left(), iconRect.bottom())
                << QPointF(iconRect.center().x(), iconRect.top() + iconSize * 0.2)
                << QPointF(iconRect.right(), iconRect.bottom());
            painter->drawPolygon(tri);
        }
    }

    painter->restore();
}
