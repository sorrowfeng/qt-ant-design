#include "AntProgressStyle.h"

#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QStyleOption>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "widgets/AntProgress.h"

namespace
{
QColor computeProgressColor(const AntProgress* progress)
{
    const auto& token = antTheme->tokens();
    if (!progress->isEnabled())
    {
        return token.colorTextDisabled;
    }
    if (progress->status() == Ant::ProgressStatus::Success || progress->percent() >= 100)
    {
        return token.colorSuccess;
    }
    if (progress->status() == Ant::ProgressStatus::Exception)
    {
        return token.colorError;
    }
    return token.colorPrimary;
}

QColor computeRailColor(const AntProgress* progress)
{
    const auto& token = antTheme->tokens();
    return progress->isEnabled() ? token.colorFillQuaternary : token.colorBgContainerDisabled;
}

QString computeInfoText(const AntProgress* progress)
{
    if (progress->status() == Ant::ProgressStatus::Success || progress->percent() >= 100)
    {
        return QStringLiteral("✓");
    }
    if (progress->status() == Ant::ProgressStatus::Exception)
    {
        return QStringLiteral("!");
    }
    return QStringLiteral("%1%").arg(progress->percent());
}
} // namespace

AntProgressStyle::AntProgressStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntProgress*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->updateGeometry();
                w->update();
            }
        }
    });
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
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (progress->progressType() == Ant::ProgressType::Line)
    {
        // Line progress
        const int percent = progress->percent();
        const int strokeWidth = progress->strokeWidth();
        const bool showInfo = progress->showInfo();
        const int infoWidth = showInfo ? 48 : 0;
        const int w = option->rect.width();
        const int h = option->rect.height();

        QRectF bar(0, (h - strokeWidth) / 2.0, w - infoWidth - 8, strokeWidth);
        if (!showInfo)
        {
            bar.setWidth(w);
        }
        bar = bar.adjusted(1, 0, -1, 0);

        painter->setPen(Qt::NoPen);
        painter->setBrush(computeRailColor(progress));
        painter->drawRoundedRect(bar, strokeWidth / 2.0, strokeWidth / 2.0);

        QRectF filled = bar;
        filled.setWidth(bar.width() * percent / 100.0);
        painter->setBrush(computeProgressColor(progress));
        painter->drawRoundedRect(filled, strokeWidth / 2.0, strokeWidth / 2.0);

        if (progress->status() == Ant::ProgressStatus::Active && percent > 0 && percent < 100)
        {
            QLinearGradient shine(filled.left() - 90, 0, filled.left(), 0);
            shine.setColorAt(0.0, QColor(255, 255, 255, 0));
            shine.setColorAt(0.5, QColor(255, 255, 255, 110));
            shine.setColorAt(1.0, QColor(255, 255, 255, 0));
            painter->setBrush(shine);
            painter->drawRoundedRect(filled, strokeWidth / 2.0, strokeWidth / 2.0);
        }

        if (showInfo)
        {
            QFont f = painter->font();
            f.setPixelSize(token.fontSizeSM);
            f.setWeight((progress->status() == Ant::ProgressStatus::Success || progress->status() == Ant::ProgressStatus::Exception) ? QFont::DemiBold : QFont::Normal);
            painter->setFont(f);
            painter->setPen(progress->status() == Ant::ProgressStatus::Exception ? token.colorError : token.colorTextSecondary);
            painter->drawText(QRectF(w - infoWidth, 0, infoWidth, h), Qt::AlignCenter, computeInfoText(progress));
        }
    }
    else
    {
        // Circle / Dashboard progress
        const int percent = progress->percent();
        const int strokeWidth = progress->strokeWidth();
        const bool showInfo = progress->showInfo();
        const qreal size = std::min(option->rect.width(), option->rect.height());
        const qreal lineWidth = std::max<qreal>(2.0, strokeWidth);
        const QRectF arc((option->rect.width() - size) / 2.0 + lineWidth,
                         (option->rect.height() - size) / 2.0 + lineWidth,
                         size - lineWidth * 2,
                         size - lineWidth * 2);

        const int startAngle = progress->progressType() == Ant::ProgressType::Dashboard ? 225 * 16 : 90 * 16;
        const int fullSpan = progress->progressType() == Ant::ProgressType::Dashboard ? -270 * 16 : -360 * 16;
        const int progressSpan = fullSpan * percent / 100;

        painter->setPen(QPen(computeRailColor(progress), lineWidth, Qt::SolidLine, Qt::RoundCap));
        painter->drawArc(arc, startAngle, fullSpan);
        painter->setPen(QPen(computeProgressColor(progress), lineWidth, Qt::SolidLine, Qt::RoundCap));
        painter->drawArc(arc, startAngle, progressSpan);

        if (showInfo)
        {
            QFont f = painter->font();
            f.setPixelSize(std::max(12, static_cast<int>(size / 7)));
            f.setWeight(QFont::DemiBold);
            painter->setFont(f);
            painter->setPen(progress->status() == Ant::ProgressStatus::Exception ? token.colorError : token.colorText);
            painter->drawText(option->rect, Qt::AlignCenter, computeInfoText(progress));
        }
    }

    painter->restore();
}
