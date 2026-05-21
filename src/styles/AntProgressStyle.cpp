#include "AntProgressStyle.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntIconPainter.h"
#include "styles/AntPalette.h"
#include "widgets/AntProgress.h"

namespace
{
void drawStatusIcon(QPainter* painter, const QRectF& rect, Ant::ProgressStatus status, const QColor& color)
{
    const Ant::IconType iconType = status == Ant::ProgressStatus::Exception
        ? Ant::IconType::CloseCircle
        : Ant::IconType::CheckCircle;
    AntIconPainter::drawIcon(*painter,
                             iconType,
                             rect,
                             color,
                             Ant::IconTheme::Filled,
                             antTheme->tokens().colorTextLightSolid);
}
} // namespace

AntProgressStyle::AntProgressStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntProgressStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntProgress*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntProgressStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntProgress*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntProgressStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntProgress*>(widget))
    {
        drawProgress(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntProgressStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntProgressStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* progress = qobject_cast<AntProgress*>(watched);
    if (progress && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(progress);
        option.rect = progress->rect();
        QPainter painter(progress);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, progress);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntProgressStyle::drawProgress(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* progress = qobject_cast<const AntProgress*>(widget);
    if (!progress || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const auto& layout = progress->progressLayout();
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (layout.progressType == Ant::ProgressType::Line)
    {
        const qreal radius = layout.strokeWidth / 2.0;
        AntStyleBase::drawCrispRoundedRect(painter, layout.lineBarRect.toRect(),
            Qt::NoPen, layout.railColor, radius, radius);

        AntStyleBase::drawCrispRoundedRect(painter, layout.lineFilledRect.toRect(),
            Qt::NoPen, layout.progressColor, radius, radius);

        if (layout.status == Ant::ProgressStatus::Active && layout.percent > 0 && layout.percent < 100)
        {
            QLinearGradient shine(layout.lineFilledRect.left() + progress->m_activeOffset - 90,
                                  0,
                                  layout.lineFilledRect.left() + progress->m_activeOffset,
                                  0);
            shine.setColorAt(0.0, QColor(255, 255, 255, 0));
            shine.setColorAt(0.5, QColor(255, 255, 255, 110));
            shine.setColorAt(1.0, QColor(255, 255, 255, 0));
            AntStyleBase::drawCrispRoundedRect(painter, layout.lineFilledRect.toRect(),
                Qt::NoPen, shine, radius, radius);
        }

        if (layout.showInfo)
        {
            if (layout.hasStatusIcon)
            {
                drawStatusIcon(painter, layout.lineStatusIconRect, layout.status, layout.infoColor);
            }
            else
            {
                QFont f = painter->font();
                f.setPixelSize(token.fontSizeSM);
                f.setWeight(QFont::Normal);
                painter->setFont(f);
                painter->setPen(layout.infoColor);
                painter->drawText(layout.lineInfoRect, Qt::AlignCenter, layout.infoText);
            }
        }
    }
    else
    {
        painter->setPen(QPen(layout.railColor, layout.circleLineWidth, Qt::SolidLine, Qt::RoundCap));
        painter->drawArc(layout.circleArcRect, layout.circleStartAngle, layout.circleFullSpan);
        painter->setPen(QPen(layout.progressColor, layout.circleLineWidth, Qt::SolidLine, Qt::RoundCap));
        painter->drawArc(layout.circleArcRect, layout.circleStartAngle, layout.circleProgressSpan);

        if (layout.showInfo)
        {
            if (layout.hasStatusIcon)
            {
                drawStatusIcon(painter, layout.circleStatusIconRect, layout.status, layout.infoColor);
            }
            else
            {
                QFont f = painter->font();
                f.setPixelSize(layout.circleInfoFontSize);
                f.setWeight(QFont::DemiBold);
                painter->setFont(f);
                painter->setPen(token.colorText);
                painter->drawText(option->rect, Qt::AlignCenter, layout.infoText);
            }
        }
    }

    painter->restore();
}
