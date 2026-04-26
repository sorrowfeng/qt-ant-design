#include "AntEmptyStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include "core/AntStyleBase.h"
#include "styles/AntPalette.h"
#include "widgets/AntEmpty.h"

namespace
{

QSize emptyEffectiveImageSize(const AntEmpty* empty)
{
    const QSize imageSize = empty->imageSize();
    if (imageSize.isValid() && !imageSize.isEmpty())
    {
        return imageSize;
    }
    return empty->isSimple() ? QSize(96, 56) : QSize(128, 80);
}

QRect emptyImageRect(const AntEmpty* empty, const QRect& widgetRect)
{
    const auto& token = antTheme->tokens();
    const QSize image = emptyEffectiveImageSize(empty);
    return QRect((widgetRect.width() - image.width()) / 2, token.padding, image.width(), image.height());
}

QRect emptyDescriptionRect(const AntEmpty* empty, const QRect& widgetRect)
{
    const auto& token = antTheme->tokens();
    const int top = empty->imageVisible() ? emptyImageRect(empty, widgetRect).bottom() + token.marginSM : token.padding;
    QWidget* extra = empty->extraWidget();
    const int bottom = extra ? widgetRect.height() - token.padding - extra->sizeHint().height() - token.marginSM
                             : widgetRect.height() - token.padding;
    return QRect(token.paddingSM, top, qMax(40, widgetRect.width() - token.paddingSM * 2),
                 qMax(token.fontSize + 4, bottom - top));
}

} // namespace

AntEmptyStyle::AntEmptyStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntEmpty>();
}

void AntEmptyStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
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
    QProxyStyle::unpolish(widget);
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

    if (empty->imageVisible())
    {
        const QRect r = emptyImageRect(empty, option->rect);
        const bool isDark = antTheme->themeMode() == Ant::ThemeMode::Dark;
        const bool simple = empty->isSimple();
        const QColor primary = AntPalette::alpha(token.colorTextTertiary, isDark ? 0.5 : 0.32);
        const QColor fill = AntPalette::alpha(token.colorFillQuaternary, isDark ? 0.78 : 1.0);
        const QColor line = AntPalette::alpha(token.colorTextTertiary, isDark ? 0.68 : 0.45);

        painter->save();
        painter->translate(r.topLeft());
        painter->scale(r.width() / 128.0, r.height() / 80.0);

        painter->setPen(Qt::NoPen);
        painter->setBrush(AntPalette::alpha(token.colorPrimary, simple ? 0.12 : 0.08));
        painter->drawEllipse(QRectF(16, 58, 96, 14));

        AntStyleBase::drawCrispRoundedRect(painter, QRect(34, 10, 60, 46), Qt::NoPen, fill, 10, 10);
        AntStyleBase::drawCrispRoundedRect(painter, QRect(42, 18, 44, 30), Qt::NoPen,
            AntPalette::alpha(token.colorBgContainer, 0.88), 6, 6);

        painter->setPen(QPen(primary, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawArc(QRectF(10, 20, 26, 26), 35 * 16, 260 * 16);
        painter->drawArc(QRectF(92, 18, 22, 22), 220 * 16, 220 * 16);

        painter->setPen(QPen(line, 2.2, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(49, 27), QPointF(79, 27));
        painter->drawLine(QPointF(49, 34), QPointF(73, 34));
        if (!simple)
        {
            painter->drawLine(QPointF(49, 41), QPointF(67, 41));
            painter->setBrush(AntPalette::alpha(token.colorPrimary, 0.16));
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(QRectF(20, 12, 8, 8));
            painter->drawEllipse(QRectF(100, 42, 6, 6));
        }
        painter->restore();
    }

    QFont descFont = painter->font();
    descFont.setPixelSize(token.fontSize);
    descFont.setWeight(QFont::Normal);
    painter->setFont(descFont);
    painter->setPen(token.colorTextSecondary);
    painter->drawText(emptyDescriptionRect(empty, option->rect), Qt::AlignCenter | Qt::TextWordWrap, empty->description());

    painter->restore();
}
