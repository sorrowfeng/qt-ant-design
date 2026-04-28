#include "AntAlertStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntAlert.h"
#include "widgets/AntIcon.h"

namespace
{
struct Metrics
{
    int minHeight = 40;
    int radius = 8;
    int paddingX = 16;
    int paddingY = 10;
    int iconSize = 16;
    int titleFontSize = 14;
    int descFontSize = 14;
    int closeSize = 20;
    int actionSpacing = 12;
};

Metrics computeMetrics(const AntAlert* alert)
{
    const auto& token = antTheme->tokens();
    Metrics m;
    m.minHeight = alert->description().isEmpty() ? token.controlHeightLG : 72;
    m.radius = alert->isBanner() ? 0 : token.borderRadiusLG;
    m.paddingX = alert->isBanner() ? token.paddingLG : token.padding;
    m.paddingY = alert->description().isEmpty() ? token.paddingXS : token.paddingSM;
    m.iconSize = alert->description().isEmpty() ? 16 : 18;
    m.titleFontSize = token.fontSize;
    m.descFontSize = token.fontSize;
    m.closeSize = 20;
    return m;
}

QColor computeBackgroundColor(Ant::AlertType type)
{
    const auto& token = antTheme->tokens();
    switch (type)
    {
    case Ant::AlertType::Success:
        return token.colorSuccessBg;
    case Ant::AlertType::Warning:
        return token.colorWarningBg;
    case Ant::AlertType::Error:
        return token.colorErrorBg;
    case Ant::AlertType::Info:
    default:
        return token.colorPrimaryBg;
    }
}

QColor computeBorderColor(Ant::AlertType type)
{
    switch (type)
    {
    case Ant::AlertType::Success:
        return AntPalette::tint(antTheme->tokens().colorSuccess, 0.62);
    case Ant::AlertType::Warning:
        return AntPalette::tint(antTheme->tokens().colorWarning, 0.62);
    case Ant::AlertType::Error:
        return AntPalette::tint(antTheme->tokens().colorError, 0.62);
    case Ant::AlertType::Info:
    default:
        return AntPalette::tint(antTheme->tokens().colorPrimary, 0.62);
    }
}

QColor computeIconColor(Ant::AlertType type)
{
    switch (type)
    {
    case Ant::AlertType::Success:
        return antTheme->tokens().colorSuccess;
    case Ant::AlertType::Warning:
        return antTheme->tokens().colorWarning;
    case Ant::AlertType::Error:
        return antTheme->tokens().colorError;
    case Ant::AlertType::Info:
    default:
        return antTheme->tokens().colorPrimary;
    }
}

Ant::IconType computeIconType(Ant::AlertType type)
{
    switch (type)
    {
    case Ant::AlertType::Success:
        return Ant::IconType::CheckCircle;
    case Ant::AlertType::Warning:
        return Ant::IconType::ExclamationCircle;
    case Ant::AlertType::Error:
        return Ant::IconType::CloseCircle;
    case Ant::AlertType::Info:
    default:
        return Ant::IconType::InfoCircle;
    }
}

QRect computeContentRect(const QRect& rect, const Metrics& m)
{
    return rect.adjusted(m.paddingX, m.paddingY, -m.paddingX, -m.paddingY);
}

QRect computeCloseRect(const AntAlert* alert, const Metrics& m, const QRect& content)
{
    return QRect(alert->width() - m.paddingX - m.closeSize,
                 content.top(),
                 m.closeSize,
                 m.closeSize);
}
} // namespace

AntAlertStyle::AntAlertStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntAlert>();
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

    // Track close button hover state
    if (event->type() == QEvent::MouseMove && alert->isClosable())
    {
        auto* me = static_cast<QMouseEvent*>(event);
        const Metrics m = computeMetrics(alert);
        const QRect content = computeContentRect(alert->rect(), m);
        const QRect close = computeCloseRect(alert, m, content);
        const bool hover = close.contains(me->pos());
        const bool wasHover = alert->property("antStyle_closeHover").toBool();
        if (hover != wasHover)
        {
            alert->setProperty("antStyle_closeHover", hover);
            alert->update(close);
        }
    }

    if (event->type() == QEvent::Leave)
    {
        if (alert->property("antStyle_closeHover").toBool())
        {
            alert->setProperty("antStyle_closeHover", false);
            alert->update();
        }
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
    const Metrics m = computeMetrics(alert);
    const Ant::AlertType alertType = alert->alertType();
    const bool hasDescription = !alert->description().isEmpty();
    const bool showIcon = alert->showIcon() || alert->isBanner();
    const bool closable = alert->isClosable();
    const bool hoverClose = alert->property("antStyle_closeHover").toBool();

    const QRect body = option->rect;
    const QRect content = computeContentRect(option->rect, m);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // Background and border
    AntStyleBase::drawCrispRoundedRect(painter, body,
        alert->isBanner() ? Qt::NoPen : QPen(computeBorderColor(alertType), token.lineWidth),
        computeBackgroundColor(alertType), m.radius, m.radius);

    // Icon
    int textLeft = content.left();
    if (showIcon)
    {
        const QRect iconRect(textLeft,
                             content.top() + (hasDescription ? 2 : (content.height() - m.iconSize) / 2),
                             m.iconSize,
                             m.iconSize);
        AntIcon icon(computeIconType(alertType));
        icon.setIconTheme(Ant::IconTheme::Filled);
        icon.setColor(computeIconColor(alertType));
        icon.setIconSize(m.iconSize);
        icon.resize(iconRect.size());

        QPixmap pixmap(iconRect.size() * widget->devicePixelRatioF());
        pixmap.setDevicePixelRatio(widget->devicePixelRatioF());
        pixmap.fill(Qt::transparent);
        icon.render(&pixmap);
        painter->drawPixmap(iconRect.topLeft(), pixmap);
        textLeft = iconRect.right() + 12;
    }

    // Close button and text right boundary
    int textRight = content.right();
    if (closable)
    {
        const QRect close = computeCloseRect(alert, m, content);
        textRight = qMin(textRight, close.left() - 8);

        if (hoverClose)
        {
            AntStyleBase::drawCrispRoundedRect(painter, close,
                Qt::NoPen, AntPalette::alpha(token.colorTextTertiary, 0.12),
                token.borderRadiusXS, token.borderRadiusXS);
        }
        painter->setPen(QPen(token.colorTextTertiary, 1.5, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(close.center() + QPoint(-4, -4), close.center() + QPoint(4, 4));
        painter->drawLine(close.center() + QPoint(4, -4), close.center() + QPoint(-4, 4));
    }

    // Title
    QFont titleFont = painter->font();
    titleFont.setPixelSize(m.titleFontSize);
    titleFont.setWeight(hasDescription ? QFont::DemiBold : QFont::Normal);
    painter->setFont(titleFont);
    painter->setPen(token.colorText);

    QRect titleRect(textLeft,
                    content.top(),
                    qMax(40, textRight - textLeft),
                    hasDescription ? titleFont.pixelSize() + 6 : content.height());
    painter->drawText(titleRect,
                      Qt::AlignLeft | (hasDescription ? Qt::AlignTop : Qt::AlignVCenter),
                      alert->title());

    // Description
    if (hasDescription)
    {
        QFont descFont = painter->font();
        descFont.setPixelSize(m.descFontSize);
        descFont.setWeight(QFont::Normal);
        painter->setFont(descFont);
        painter->setPen(token.colorTextSecondary);
        QRect descRect(textLeft,
                       titleRect.bottom() + 6,
                       qMax(40, textRight - textLeft),
                       content.bottom() - titleRect.bottom() - 6);
        painter->drawText(descRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, alert->description());
    }

    painter->restore();
}
