#include "AntEmptyStyle.h"

#include <QEvent>
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QStyleOption>
#include <QtMath>

#include "core/AntStyleBase.h"
#include "styles/AntPalette.h"
#include "widgets/AntEmpty.h"

namespace
{

QString colorKey(const QColor& color)
{
    return QString::number(color.rgba(), 16);
}

QString emptyIllustrationCacheKey(const AntEmpty* empty, const QSize& logicalSize, qreal dpr)
{
    const auto& token = antTheme->tokens();
    return QStringLiteral("AntEmpty:%1:%2x%3:%4:%5:%6:%7:%8:%9")
        .arg(empty->isSimple() ? 1 : 0)
        .arg(logicalSize.width())
        .arg(logicalSize.height())
        .arg(qRound(dpr * 100.0))
        .arg(static_cast<int>(antTheme->themeMode()))
        .arg(colorKey(token.colorTextTertiary),
             colorKey(token.colorFillQuaternary),
             colorKey(token.colorPrimary),
             colorKey(token.colorBgContainer));
}

} // namespace

AntEmptyStyle::AntEmptyStyle(QStyle* style)
    : AntStyleBase(style)
{
}

void AntEmptyStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    if (qobject_cast<AntEmpty*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntEmptyStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntEmpty*>(widget))
    {
        widget->removeEventFilter(this);
    }
    AntStyleBase::unpolish(widget);
}

void AntEmptyStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntEmpty*>(widget))
    {
        drawEmpty(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntEmptyStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntEmptyStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* empty = qobject_cast<AntEmpty*>(watched);
    if (empty && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(empty);
        option.rect = empty->rect();
        QPainter painter(empty);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, empty);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntEmptyStyle::drawEmpty(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* empty = qobject_cast<const AntEmpty*>(widget);
    if (!empty || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    const auto& cache = empty->emptyLayoutCache(option->rect.size());
    if (empty->imageVisible())
    {
        const QRect r = cache.imageRect;
        const qreal dpr = painter->device()->devicePixelRatioF();
        const QString key = emptyIllustrationCacheKey(empty, r.size(), dpr);
        QPixmap pixmap;
        auto* mutableEmpty = const_cast<AntEmpty*>(empty);
        if (QPixmapCache::find(key, &pixmap))
        {
            ++mutableEmpty->m_illustrationPixmapHitCount;
        }
        else
        {
            pixmap = QPixmap(qCeil(r.width() * dpr), qCeil(r.height() * dpr));
            pixmap.setDevicePixelRatio(dpr);
            pixmap.fill(Qt::transparent);
            QPainter pixmapPainter(&pixmap);
            AntStyleBase::drawEmptyIllustration(&pixmapPainter,
                QRectF(QPointF(0, 0), r.size()), empty->isSimple());
            QPixmapCache::insert(key, pixmap);
            ++mutableEmpty->m_illustrationPixmapBuildCount;
        }
        mutableEmpty->syncEmptyPerfCounters();
        painter->drawPixmap(r.topLeft(), pixmap);
    }

    QFont descFont = painter->font();
    descFont.setPixelSize(token.fontSize);
    descFont.setWeight(QFont::Normal);
    painter->setFont(descFont);
    painter->setPen(token.colorTextTertiary);
    painter->drawText(cache.descriptionRect, Qt::AlignCenter | Qt::TextWordWrap, empty->description());

    painter->restore();
}
