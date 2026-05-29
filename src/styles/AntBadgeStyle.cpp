#include "AntBadgeStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include <algorithm>

#include "core/AntStyleBase.h"
#include "widgets/AntBadge.h"

AntBadgeStyle::AntBadgeStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntBadgeStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    if (qobject_cast<AntBadge*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntBadgeStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntBadge*>(widget))
    {
        widget->removeEventFilter(this);
    }
    AntStyleBase::unpolish(widget);
}

void AntBadgeStyle::drawPrimitive(PrimitiveElement element,
                                  const QStyleOption* option,
                                  QPainter* painter,
                                  const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntBadge*>(widget))
    {
        drawBadge(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntBadgeStyle::sizeFromContents(ContentsType type,
                                      const QStyleOption* option,
                                      const QSize& size,
                                      const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntBadgeStyle::eventFilter(QObject* watched, QEvent* event)
{
    Q_UNUSED(watched)
    Q_UNUSED(event)
    // AntBadge paints through an overlay child so the indicator stays above
    // opaque content widgets. The owner widget keeps this filter only for
    // hover delivery and style consistency.
    return QProxyStyle::eventFilter(watched, event);
}

void AntBadgeStyle::drawBadge(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* badge = qobject_cast<const AntBadge*>(widget);
    if (!badge || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const auto& metrics = badge->badgePaintCache(option->rect);
    const bool enabled = option->state & QStyle::State_Enabled;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    if (badge->isStatusMode() && !badge->contentWidget())
    {
        const QColor color = enabled ? metrics.statusColor : token.colorTextDisabled;
        if (badge->status() == Ant::BadgeStatus::Processing)
        {
            const qreal pulse = badge->processingPulseProgress();
            QColor ring = color;
            ring.setAlphaF(std::max(0.0, 0.45 * (1.0 - pulse)));
            const qreal scale = 1.0 + 1.4 * pulse;
            const QRectF dot = metrics.statusDotRect;
            const QRectF ringRect(dot.center().x() - dot.width() * scale / 2.0,
                                  dot.center().y() - dot.height() * scale / 2.0,
                                  dot.width() * scale,
                                  dot.height() * scale);
            painter->setPen(QPen(ring, token.lineWidth));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(ringRect);
        }

        painter->setPen(Qt::NoPen);
        painter->setBrush(color);
        painter->drawEllipse(metrics.statusDotRect);

        if (!metrics.text.isEmpty())
        {
            QFont f = painter->font();
            f.setPixelSize(token.fontSize);
            painter->setFont(f);
            painter->setPen(enabled ? token.colorText : token.colorTextDisabled);
            painter->drawText(metrics.statusTextRect, Qt::AlignLeft | Qt::AlignVCenter, metrics.text);
        }
        painter->restore();
        return;
    }

    if (badge->badgeMode() == Ant::BadgeMode::Ribbon && !metrics.ribbonText.isEmpty())
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(metrics.ribbonFillColor);
        painter->drawPath(metrics.ribbonPath);
        painter->setBrush(metrics.ribbonFoldColor);
        painter->drawPath(metrics.ribbonFoldPath);

        QFont f = painter->font();
        f.setPixelSize(token.fontSizeSM);
        f.setWeight(QFont::DemiBold);
        painter->setFont(f);
        painter->setPen(token.colorTextLightSolid);
        painter->drawText(metrics.ribbonTextRect, Qt::AlignCenter, metrics.ribbonText);
        painter->restore();
        return;
    }

    if (badge->shouldShowIndicator())
    {
        const QRectF r = metrics.indicatorRect;
        const QColor fill = enabled ? metrics.badgeColor : token.colorTextDisabled;
        painter->setPen(QPen(token.colorBgContainer, token.lineWidth));
        painter->setBrush(fill);
        if (badge->isDot())
        {
            painter->drawEllipse(r);
            painter->restore();
            return;
        }

        AntStyleBase::drawCrispRoundedRect(painter,
                                           r.toRect(),
                                           QPen(token.colorBgContainer, token.lineWidth),
                                           fill,
                                           r.height() / 2.0,
                                           r.height() / 2.0);

        QFont f = painter->font();
        f.setPixelSize(token.fontSizeSM);
        f.setWeight(QFont::Normal);
        painter->setFont(f);
        painter->setPen(token.colorTextLightSolid);
        painter->drawText(r.adjusted(token.paddingXXS, 0, -token.paddingXXS, 0),
                          Qt::AlignCenter,
                          metrics.displayText);
    }

    painter->restore();
}
