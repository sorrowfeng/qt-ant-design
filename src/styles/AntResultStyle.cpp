#include "AntResultStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

#include "widgets/AntIcon.h"
#include "widgets/AntResult.h"

namespace
{

struct Metrics
{
    int iconSize = 72;
    int padding = 32;
    int titleFontSize = 24;
    int subTitleFontSize = 14;
    int spacing = 12;
    int extraSpacing = 24;
};

Metrics resultMetrics()
{
    return {};
}

QRect resultIconRect(const QRect& widgetRect)
{
    const Metrics m = resultMetrics();
    const int top = m.padding;
    return QRect((widgetRect.width() - m.iconSize) / 2, top, m.iconSize, m.iconSize);
}

QRect resultTitleRect(const AntResult* result, const QRect& widgetRect, const QFont& widgetFont)
{
    const Metrics m = resultMetrics();
    const QString title = result->title();

    QFont titleFont = widgetFont;
    titleFont.setPixelSize(m.titleFontSize);
    titleFont.setWeight(QFont::DemiBold);
    QFontMetrics titleFm(titleFont);

    int top = m.padding;
    if (result->isIconVisible())
    {
        top += m.iconSize + m.spacing;
    }

    const int textWidth = qMax(120, widgetRect.width() - m.padding * 2);
    const int textHeight = titleFm.boundingRect(QRect(0, 0, textWidth, 200), Qt::AlignCenter | Qt::TextWordWrap, title).height();

    return QRect(m.padding, top, textWidth, textHeight);
}

QRect resultSubTitleRect(const AntResult* result, const QRect& widgetRect, const QFont& widgetFont)
{
    const Metrics m = resultMetrics();
    const QString subTitle = result->subTitle();

    QFont subFont = widgetFont;
    subFont.setPixelSize(m.subTitleFontSize);
    QFontMetrics subFm(subFont);

    const QRect tr = resultTitleRect(result, widgetRect, widgetFont);
    const int top = tr.bottom() + m.spacing;
    const int textWidth = qMax(120, widgetRect.width() - m.padding * 2);
    const int textHeight = subFm.boundingRect(QRect(0, 0, textWidth, 200), Qt::AlignCenter | Qt::TextWordWrap, subTitle).height();

    return QRect(m.padding, top, textWidth, textHeight);
}

QColor resultIconColor(const AntResult* result)
{
    const auto& token = antTheme->tokens();
    switch (result->status())
    {
    case Ant::AlertType::Success:
        return token.colorSuccess;
    case Ant::AlertType::Warning:
        return token.colorWarning;
    case Ant::AlertType::Error:
        return token.colorError;
    case Ant::AlertType::Info:
    default:
        return token.colorPrimary;
    }
}

Ant::IconType resultIconTypeForStatus(const AntResult* result)
{
    switch (result->status())
    {
    case Ant::AlertType::Success:
        return Ant::IconType::CheckCircle;
    case Ant::AlertType::Error:
        return Ant::IconType::CloseCircle;
    case Ant::AlertType::Warning:
        return Ant::IconType::ExclamationCircle;
    case Ant::AlertType::Info:
    default:
        return Ant::IconType::InfoCircle;
    }
}

} // namespace

AntResultStyle::AntResultStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntResult>();
}

void AntResultStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntResult*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntResultStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntResult*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntResultStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntResult*>(widget))
    {
        drawResult(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntResultStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntResultStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* result = qobject_cast<AntResult*>(watched);
    if (result && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(result);
        option.rect = result->rect();
        QPainter painter(result);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, result);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntResultStyle::drawResult(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* result = qobject_cast<const AntResult*>(widget);
    if (!result || !painter || !option)
    {
        return;
    }

    const Metrics m = resultMetrics();
    const auto& token = antTheme->tokens();
    const QFont widgetFont = result->font();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (result->isIconVisible())
    {
        const QRect ir = resultIconRect(option->rect);
        AntIcon icon(resultIconTypeForStatus(result));
        icon.setIconTheme(Ant::IconTheme::Filled);
        icon.setColor(resultIconColor(result));
        icon.setIconSize(m.iconSize);
        icon.resize(ir.size());

        const qreal dpr = widget->devicePixelRatioF();
        QPixmap pixmap(ir.size() * dpr);
        pixmap.setDevicePixelRatio(dpr);
        pixmap.fill(Qt::transparent);
        icon.render(&pixmap);
        painter->drawPixmap(ir.topLeft(), pixmap);
    }

    QFont titleFont = widgetFont;
    titleFont.setPixelSize(m.titleFontSize);
    titleFont.setWeight(QFont::DemiBold);
    painter->setFont(titleFont);
    painter->setPen(token.colorText);
    painter->drawText(resultTitleRect(result, option->rect, widgetFont),
                      Qt::AlignCenter | Qt::TextWordWrap, result->title());

    const QString subTitle = result->subTitle();
    if (!subTitle.isEmpty())
    {
        QFont subFont = widgetFont;
        subFont.setPixelSize(m.subTitleFontSize);
        subFont.setWeight(QFont::Normal);
        painter->setFont(subFont);
        painter->setPen(token.colorTextSecondary);
        painter->drawText(resultSubTitleRect(result, option->rect, widgetFont),
                          Qt::AlignCenter | Qt::TextWordWrap, subTitle);
    }

    painter->restore();
}
