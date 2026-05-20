#include "AntMenuBarStyle.h"

#include <QFontMetrics>
#include <QPainter>
#include <QRegularExpression>
#include <QStyleOptionMenuItem>

#include "styles/AntPalette.h"
#include "widgets/AntMenuBar.h"

AntMenuBarStyle::AntMenuBarStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntMenuBar>();
}

void AntMenuBarStyle::onThemeUpdate(QWidget* w)
{
    clearTextMetricCache();
    w->update();
}

AntMenuBarStyle::CachedTextMetric AntMenuBarStyle::cachedTextMetric(const QString& text,
                                                                     const QFont& font,
                                                                     const QWidget* widget) const
{
    const QString cacheKey = text + QLatin1Char('\n') + font.toString();
    const auto it = m_textMetricCache.constFind(cacheKey);
    if (it != m_textMetricCache.constEnd())
    {
        ++m_textMetricCacheHitCount;
        syncTextMetricCounters(widget);
        return it.value();
    }

    CachedTextMetric metric;
    metric.displayText = text;
    metric.displayText.remove(QRegularExpression(QStringLiteral("&(?=\\S)")));
    metric.width = QFontMetrics(font).horizontalAdvance(metric.displayText);
    m_textMetricCache.insert(cacheKey, metric);
    ++m_textMetricCacheBuildCount;
    syncTextMetricCounters(widget);
    return metric;
}

void AntMenuBarStyle::syncTextMetricCounters(const QWidget* widget) const
{
    if (!widget)
    {
        return;
    }

    auto* mutableWidget = const_cast<QWidget*>(widget);
    mutableWidget->setProperty("antMenuBarTextMetricCacheBuildCount", m_textMetricCacheBuildCount);
    mutableWidget->setProperty("antMenuBarTextMetricCacheHitCount", m_textMetricCacheHitCount);
    mutableWidget->setProperty("antMenuBarTextMetricCacheSize", m_textMetricCache.size());
}

void AntMenuBarStyle::clearTextMetricCache() const
{
    m_textMetricCache.clear();
}

void AntMenuBarStyle::drawControl(ControlElement element, const QStyleOption* option,
                                   QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::CE_MenuBarEmptyArea && qobject_cast<const AntMenuBar*>(widget))
    {
        painter->save();
        painter->fillRect(option->rect, antTheme->tokens().colorBgContainer);
        painter->restore();
        return;
    }

    if (element == QStyle::CE_MenuBarItem && qobject_cast<const AntMenuBar*>(widget))
    {
        const auto* menuItem = qstyleoption_cast<const QStyleOptionMenuItem*>(option);
        if (!menuItem || menuItem->text.isEmpty()) return;

        const auto& token = antTheme->tokens();
        const bool active = menuItem->state.testFlag(QStyle::State_Selected);
        const bool pressed = menuItem->state.testFlag(QStyle::State_Sunken);
        const bool enabled = menuItem->state.testFlag(QStyle::State_Enabled);
        const QRectF r = menuItem->rect;

        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        // Background
        if (active || pressed)
        {
            QColor bg = active ? token.colorFillTertiary : token.colorFillQuaternary;
            AntStyleBase::drawCrispRoundedRect(painter, r.toRect().adjusted(4, 3, -4, -3),
                Qt::NoPen, bg, 4, 4);
        }

        // Text
        QColor textColor = token.colorText;
        if (active) textColor = token.colorPrimary;
        if (!enabled) textColor = token.colorTextDisabled;

        QFont f = widget ? widget->font() : painter->font();
        f.setPixelSize(token.fontSize);
        painter->setFont(f);
        painter->setPen(textColor);

        const CachedTextMetric textMetric = cachedTextMetric(menuItem->text, f, widget);
        const QString& displayText = textMetric.displayText;

        // Icon
        qreal offsetX = 8;
        if (!menuItem->icon.isNull())
        {
            const int iconSz = static_cast<int>(r.height() * 0.65);
            const QRectF iconRect(r.left() + offsetX, r.center().y() - iconSz / 2.0, iconSz, iconSz);
            menuItem->icon.paint(painter, iconRect.toRect(), Qt::AlignCenter,
                                 enabled ? QIcon::Normal : QIcon::Disabled);
            offsetX += iconSz + 6;
        }

        painter->drawText(r.adjusted(offsetX, 0, -8, 0), Qt::AlignLeft | Qt::AlignVCenter, displayText);

        painter->restore();
        return;
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}

int AntMenuBarStyle::pixelMetric(PixelMetric metric, const QStyleOption* option,
                                  const QWidget* widget) const
{
    if (metric == QStyle::PM_MenuBarHMargin)
        return 0;
    if (metric == QStyle::PM_MenuBarItemSpacing)
        return 4;
    return QProxyStyle::pixelMetric(metric, option, widget);
}

QSize AntMenuBarStyle::sizeFromContents(ContentsType type, const QStyleOption* option,
                                         const QSize& size, const QWidget* widget) const
{
    if (type == QStyle::CT_MenuBar)
        return QSize(size.width(), widget ? widget->height() : antTheme->tokens().controlHeight);
    if (type == QStyle::CT_MenuBarItem && qobject_cast<const AntMenuBar*>(widget))
    {
        const auto* menuItem = qstyleoption_cast<const QStyleOptionMenuItem*>(option);
        if (!menuItem)
        {
            return QProxyStyle::sizeFromContents(type, option, size, widget);
        }

        const auto& token = antTheme->tokens();
        QFont f = widget ? widget->font() : QFont();
        f.setPixelSize(token.fontSize);
        const CachedTextMetric textMetric = cachedTextMetric(menuItem->text, f, widget);
        int width = textMetric.width + 16;
        if (!menuItem->icon.isNull())
        {
            const int iconSize = qMax(14, token.fontSize + 2);
            width += iconSize + 6;
        }
        const int height = qMax(size.height(), token.controlHeight);
        return QSize(width, height);
    }
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}
