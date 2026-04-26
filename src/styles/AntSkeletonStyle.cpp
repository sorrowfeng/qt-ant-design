#include "AntSkeletonStyle.h"

#include <QEvent>
#include <QLinearGradient>
#include <QPainter>
#include <QStyleOption>

#include "core/AntStyleBase.h"
#include "styles/AntPalette.h"
#include "widgets/AntSkeleton.h"

namespace
{

struct Metrics
{
    int avatarSize = 40;
    int titleHeight = 16;
    int paragraphHeight = 12;
    int rowSpacing = 12;
    int columnGap = 16;
    int radius = 6;
    int titleTop = 4;
};

Metrics skeletonMetrics()
{
    Metrics m;
    const auto& token = antTheme->tokens();
    m.avatarSize = token.controlHeightLG;
    m.titleHeight = token.fontSizeLG;
    m.paragraphHeight = token.fontSizeSM + 2;
    m.rowSpacing = token.marginSM;
    m.columnGap = token.margin;
    m.radius = token.borderRadiusSM;
    return m;
}

qreal skeletonRowWidthRatio(const AntSkeleton* skeleton, int rowIndex)
{
    const QList<qreal> ratios = skeleton->paragraphWidthRatios();
    const int rows = skeleton->paragraphRows();
    if (rowIndex >= 0 && rowIndex < ratios.size())
    {
        return qBound(0.15, ratios.at(rowIndex), 1.0);
    }
    if (rowIndex == rows - 1)
    {
        return 0.62;
    }
    return 1.0;
}

QList<QRectF> skeletonPlaceholderRects(const AntSkeleton* skeleton, int width)
{
    QList<QRectF> rects;
    const Metrics m = skeletonMetrics();

    // Element mode
    if (skeleton->element() != Ant::SkeletonElement::Default)
    {
        const QSize hint = skeleton->sizeHint();
        const qreal w = qMin(static_cast<qreal>(width), static_cast<qreal>(hint.width()));
        const qreal h = hint.height();
        switch (skeleton->element())
        {
        case Ant::SkeletonElement::Button:
            rects.append(QRectF(0, 0, w, h));
            break;
        case Ant::SkeletonElement::Avatar:
            rects.append(QRectF(0, 0, m.avatarSize, m.avatarSize));
            break;
        case Ant::SkeletonElement::Input:
            rects.append(QRectF(0, 0, w, h));
            break;
        case Ant::SkeletonElement::Image:
        {
            const qreal sz = qMin(w, h);
            rects.append(QRectF((w - sz) / 2, 0, sz, sz));
            break;
        }
        case Ant::SkeletonElement::Node:
            rects.append(QRectF(0, 0, w, h));
            break;
        default:
            break;
        }
        return rects;
    }

    // Default paragraph mode
    int textLeft = 0;
    int textWidth = width;
    const int totalHeight = skeleton->sizeHint().height();
    int top = 0;

    if (skeleton->avatarVisible())
    {
        const int avatarY = qMax(0, (totalHeight - m.avatarSize) / 2);
        rects.append(QRectF(0, avatarY, m.avatarSize, m.avatarSize));
        textLeft = m.avatarSize + m.columnGap;
        textWidth = qMax(40, width - textLeft);
    }

    if (skeleton->titleVisible())
    {
        const qreal titleWidthRatio = skeleton->titleWidthRatio();
        const int titleWidth = qMax(48, static_cast<int>(textWidth * titleWidthRatio));
        rects.append(QRectF(textLeft, top + m.titleTop, titleWidth, m.titleHeight));
        top += m.titleHeight;
    }

    if (skeleton->paragraphVisible() && skeleton->paragraphRows() > 0)
    {
        if (skeleton->titleVisible())
        {
            top += antTheme->tokens().marginSM;
        }
        for (int i = 0; i < skeleton->paragraphRows(); ++i)
        {
            const int rowWidth = qMax(56, static_cast<int>(textWidth * skeletonRowWidthRatio(skeleton, i)));
            rects.append(QRectF(textLeft, top, rowWidth, m.paragraphHeight));
            top += m.paragraphHeight + m.rowSpacing;
        }
    }

    return rects;
}

} // namespace

AntSkeletonStyle::AntSkeletonStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntSkeleton>();
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

    const auto& token = antTheme->tokens();
    const QList<QRectF> rects = skeletonPlaceholderRects(skeleton, option->rect.width());
    if (rects.isEmpty())
    {
        return;
    }

    const QColor baseColor = token.colorFillTertiary;
    const QColor highlight = AntPalette::mix(token.colorBgContainer, token.colorFillQuaternary, 0.45);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter->setPen(Qt::NoPen);

    for (const QRectF& rect : rects)
    {
        QBrush brush(baseColor);
        if (skeleton->isActive())
        {
            const int shimmerOffset = 0;
            QLinearGradient gradient(rect.topLeft(), rect.topRight());
            const qreal widthValue = qMax<qreal>(rect.width(), 1.0);
            const qreal offset = static_cast<qreal>(shimmerOffset % static_cast<int>(widthValue + 120)) / widthValue;
            gradient.setColorAt(qMax(0.0, offset - 0.6), baseColor);
            gradient.setColorAt(qBound(0.0, offset - 0.2, 1.0), baseColor);
            gradient.setColorAt(qBound(0.0, offset, 1.0), highlight);
            gradient.setColorAt(qBound(0.0, offset + 0.2, 1.0), baseColor);
            gradient.setColorAt(1.0, baseColor);
            brush = QBrush(gradient);
        }

        const Metrics met = skeletonMetrics();
        qreal radius = skeleton->isRound() ? rect.height() / 2.0 : met.radius;
        if (skeleton->element() == Ant::SkeletonElement::Avatar ||
            (skeleton->element() == Ant::SkeletonElement::Default &&
             skeleton->avatarVisible() &&
             rect.height() == met.avatarSize && rect.width() == met.avatarSize &&
             skeleton->avatarShape() == Ant::AvatarShape::Circle))
        {
            radius = rect.width() / 2.0;
        }
        if (skeleton->element() == Ant::SkeletonElement::Button || skeleton->element() == Ant::SkeletonElement::Input)
        {
            radius = rect.height() / 2.0;
        }
        AntStyleBase::drawCrispRoundedRect(painter, rect.toRect(), Qt::NoPen, brush, radius, radius);

        // Image element: draw centered placeholder icon
        if (skeleton->element() == Ant::SkeletonElement::Image)
        {
            painter->setPen(highlight);
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
