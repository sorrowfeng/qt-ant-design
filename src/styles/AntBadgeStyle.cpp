#include "AntBadgeStyle.h"

#include <QApplication>
#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include <algorithm>

#include "core/AntTheme.h"
#include "widgets/AntBadge.h"

namespace
{

int badgeIndicatorHeight(const AntBadge* badge)
{
    return badge->badgeSize() == Ant::BadgeSize::Small ? antTheme->tokens().fontSize : 20;
}

int badgeDotSize()
{
    return std::max(6, antTheme->tokens().fontSizeSM / 2);
}

int badgeStatusDotSize()
{
    return badgeDotSize();
}

QString badgeDisplayText(const AntBadge* badge)
{
    if (badge->count() > badge->overflowCount())
    {
        return QStringLiteral("%1+").arg(badge->overflowCount());
    }
    return QString::number(badge->count());
}

int badgeIndicatorWidth(const AntBadge* badge)
{
    if (badge->isDot())
    {
        return badgeDotSize();
    }
    const int h = badgeIndicatorHeight(badge);
    QFont f;
    f.setPixelSize(antTheme->tokens().fontSizeSM);
    const int textWidth = QFontMetrics(f).horizontalAdvance(badgeDisplayText(badge));
    return std::max(h, textWidth + antTheme->tokens().paddingXS * 2);
}

bool badgeIsStatusMode(const AntBadge* badge)
{
    return badge->status() != Ant::BadgeStatus::None;
}

bool badgeShouldShowIndicator(const AntBadge* badge)
{
    if (badgeIsStatusMode(badge))
    {
        return true;
    }
    if (badge->isDot())
    {
        return true;
    }
    return badge->count() != 0 || badge->showZero();
}

int badgeContentTopReserve(const AntBadge* badge)
{
    constexpr int kShadowMargin = 8;
    if (!badgeShouldShowIndicator(badge))
    {
        return kShadowMargin;
    }
    return std::max(badgeIndicatorHeight(badge) / 2 + 4 + std::max(0, -badge->offset().y()),
                    kShadowMargin);
}

int badgeContentRightReserve(const AntBadge* badge)
{
    constexpr int kShadowMargin = 8;
    if (!badgeShouldShowIndicator(badge))
    {
        return kShadowMargin;
    }
    return std::max(badgeIndicatorWidth(badge) / 2 + 4 + std::max(0, badge->offset().x()),
                    kShadowMargin);
}

QRect badgeContentRect(const AntBadge* badge, const QRect& widgetRect)
{
    QWidget* cw = badge->contentWidget();
    if (!cw)
    {
        return widgetRect;
    }
    const QSize hint = cw->sizeHint().expandedTo(cw->minimumSizeHint());
    const int topReserve = badgeContentTopReserve(badge);
    // Must match AntBadge::contentRect (kShadowMargin = 8 from AntBadge.cpp).
    constexpr int kShadowMargin = 8;
    return QRect(kShadowMargin, topReserve, hint.width(), hint.height());
}

QRectF badgeIndicatorRect(const AntBadge* badge, const QRect& widgetRect)
{
    const int w = badgeIndicatorWidth(badge);
    const int h = badge->isDot() ? badgeDotSize() : badgeIndicatorHeight(badge);
    if (!badge->contentWidget())
    {
        return QRectF((widgetRect.width() - w) / 2.0, (widgetRect.height() - h) / 2.0, w, h);
    }
    const QRect content = badgeContentRect(badge, widgetRect);
    return QRectF(content.right() + 1 - w / 2.0 + badge->offset().x(),
                  content.top() - h / 2.0 + badge->offset().y(),
                  w,
                  h);
}

QRectF badgeStandaloneStatusDotRect(const QRect& widgetRect)
{
    const int d = badgeStatusDotSize();
    return QRectF(1, (widgetRect.height() - d) / 2.0, d, d);
}

QColor badgeColor(const AntBadge* badge)
{
    const QString color = badge->color();
    const QColor custom(color);
    if (custom.isValid())
    {
        return custom;
    }
    if (color.compare(QStringLiteral("success"), Qt::CaseInsensitive) == 0)
    {
        return antTheme->tokens().colorSuccess;
    }
    if (color.compare(QStringLiteral("warning"), Qt::CaseInsensitive) == 0)
    {
        return antTheme->tokens().colorWarning;
    }
    if (color.compare(QStringLiteral("processing"), Qt::CaseInsensitive) == 0 ||
        color.compare(QStringLiteral("blue"), Qt::CaseInsensitive) == 0)
    {
        return antTheme->tokens().colorPrimary;
    }
    return antTheme->tokens().colorError;
}

QColor badgeStatusColor(const AntBadge* badge)
{
    const QString color = badge->color();
    const QColor custom(color);
    if (custom.isValid())
    {
        return custom;
    }
    const auto& token = antTheme->tokens();
    switch (badge->status())
    {
    case Ant::BadgeStatus::Success:
        return token.colorSuccess;
    case Ant::BadgeStatus::Processing:
        return token.colorPrimary;
    case Ant::BadgeStatus::Default:
        return token.colorTextPlaceholder;
    case Ant::BadgeStatus::Error:
        return token.colorError;
    case Ant::BadgeStatus::Warning:
        return token.colorWarning;
    case Ant::BadgeStatus::None:
        return token.colorError;
    }
    return token.colorError;
}

void drawBadgeIndicator(const QStyleOption* option, QPainter* painter, const AntBadge* badge)
{
    const auto& token = antTheme->tokens();
    const QRectF r = badgeIndicatorRect(badge, option->rect);
    const bool hovered = option->state & QStyle::State_MouseOver;
    const bool enabled = option->state & QStyle::State_Enabled;
    QColor fill = enabled ? badgeColor(badge) : token.colorTextDisabled;
    if (hovered)
    {
        fill = antTheme->hoverColor(fill);
    }

    painter->setPen(QPen(token.colorBgContainer, token.lineWidth));
    painter->setBrush(fill);
    if (badge->isDot())
    {
        painter->drawEllipse(r);
        return;
    }

    painter->drawRoundedRect(r, r.height() / 2.0, r.height() / 2.0);

    QFont f = painter->font();
    f.setPixelSize(token.fontSizeSM);
    f.setWeight(QFont::Normal);
    painter->setFont(f);
    painter->setPen(token.colorTextLightSolid);
    painter->drawText(r.adjusted(token.paddingXXS, 0, -token.paddingXXS, 0), Qt::AlignCenter, badgeDisplayText(badge));
}

void drawBadgeStatus(const QStyleOption* option, QPainter* painter, const AntBadge* badge)
{
    const auto& token = antTheme->tokens();
    const QRectF dot = badgeStandaloneStatusDotRect(option->rect);
    const bool enabled = option->state & QStyle::State_Enabled;
    const QColor color = enabled ? badgeStatusColor(badge) : token.colorTextDisabled;

    if (badge->status() == Ant::BadgeStatus::Processing)
    {
        // Static rendering: pulse at 0 (ring at full opacity, scale 1.0)
        QColor ring = color;
        ring.setAlphaF(0.45);
        const qreal scale = 1.0;
        QRectF ringRect(dot.center().x() - dot.width() * scale / 2.0,
                        dot.center().y() - dot.height() * scale / 2.0,
                        dot.width() * scale,
                        dot.height() * scale);
        painter->setPen(QPen(ring, token.lineWidth));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(ringRect);
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawEllipse(dot);

    const QString text = badge->text();
    if (!text.isEmpty())
    {
        QFont f = painter->font();
        f.setPixelSize(token.fontSize);
        painter->setFont(f);
        painter->setPen(enabled ? token.colorText : token.colorTextDisabled);
        const QRectF textRect(dot.right() + token.marginXS, 0, option->rect.width() - dot.right() - token.marginXS, option->rect.height());
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
}

void drawBadgeRibbon(const QStyleOption* option, QPainter* painter, const AntBadge* badge)
{
    if (badge->badgeMode() != Ant::BadgeMode::Ribbon || badge->ribbonText().isEmpty())
        return;

    const auto& token = antTheme->tokens();
    const QRect& r = option->rect;

    // Ribbon size
    QFont f = painter->font();
    f.setPixelSize(token.fontSizeSM);
    f.setWeight(QFont::DemiBold);
    const QFontMetrics fm(f);
    const int textW = fm.horizontalAdvance(badge->ribbonText());
    const int ribbonW = textW + token.paddingXS * 2;
    const int ribbonH = fm.height() + token.paddingXXS * 2;
    const int foldSize = ribbonH / 2;

    // Position: top-right corner
    const int rx = r.width() - ribbonW;
    const int ry = 0;

    // Resolve ribbon color
    QColor color;
    const QString rc = badge->ribbonColor();
    const QColor custom(rc);
    if (custom.isValid())
        color = custom;
    else if (rc.compare(QStringLiteral("success"), Qt::CaseInsensitive) == 0)
        color = token.colorSuccess;
    else if (rc.compare(QStringLiteral("warning"), Qt::CaseInsensitive) == 0)
        color = token.colorWarning;
    else if (rc.compare(QStringLiteral("processing"), Qt::CaseInsensitive) == 0)
        color = token.colorPrimary;
    else
        color = token.colorError;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    // Main ribbon body
    QPainterPath ribbonPath;
    ribbonPath.moveTo(rx, ry);
    ribbonPath.lineTo(rx + ribbonW, ry);
    ribbonPath.lineTo(rx + ribbonW, ry + ribbonH);
    ribbonPath.lineTo(rx + ribbonW - foldSize, ry + ribbonH - foldSize);
    ribbonPath.lineTo(rx, ry + ribbonH - foldSize);
    ribbonPath.closeSubpath();

    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPath(ribbonPath);

    // Fold shadow triangle
    QPainterPath foldPath;
    foldPath.moveTo(rx + ribbonW - foldSize, ry + ribbonH - foldSize);
    foldPath.lineTo(rx + ribbonW, ry + ribbonH);
    foldPath.lineTo(rx + ribbonW - foldSize, ry + ribbonH);
    foldPath.closeSubpath();
    QColor shadow = color.darker(130);
    painter->setBrush(shadow);
    painter->drawPath(foldPath);

    // Text
    painter->setFont(f);
    painter->setPen(token.colorTextLightSolid);
    QRect textRect(rx, ry, ribbonW - foldSize, ribbonH - foldSize);
    painter->drawText(textRect, Qt::AlignCenter, badge->ribbonText());

    painter->restore();
}

} // namespace

AntBadgeStyle::AntBadgeStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntBadge*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->updateGeometry();
                w->update();
            }
        }
    });
}

void AntBadgeStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
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
    QProxyStyle::unpolish(widget);
}

void AntBadgeStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntBadge*>(widget))
    {
        drawBadge(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntBadgeStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntBadgeStyle::eventFilter(QObject* watched, QEvent* event)
{
    Q_UNUSED(watched)
    Q_UNUSED(event)
    // AntBadge paints the indicator through a dedicated overlay child widget
    // (see AntBadge::m_indicatorOverlay). Painting from the style eventFilter
    // here would hit AntBadge's surface, which is drawn _before_ children,
    // so the indicator would be covered by any opaque content widget
    // (AntButton, etc.). Leave it to the overlay.
    return QProxyStyle::eventFilter(watched, event);
}

void AntBadgeStyle::drawBadge(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* badge = qobject_cast<const AntBadge*>(widget);
    if (!badge || !painter || !option)
    {
        return;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    if (badgeIsStatusMode(badge) && !badge->contentWidget())
    {
        drawBadgeStatus(option, painter, badge);
        painter->restore();
        return;
    }

    if (badge->badgeMode() == Ant::BadgeMode::Ribbon)
    {
        drawBadgeRibbon(option, painter, badge);
        painter->restore();
        return;
    }

    if (badgeShouldShowIndicator(badge))
    {
        drawBadgeIndicator(option, painter, badge);
    }

    painter->restore();
}
