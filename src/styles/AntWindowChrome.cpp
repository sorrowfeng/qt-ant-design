#include "AntWindowChrome.h"

#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QWidget>

#include "core/AntStyleBase.h"
#include "core/AntTheme.h"
#include "styles/AntIconSvgRenderer.h"

namespace
{
bool drawAntdIcon(const QString& iconName, const QRectF& iconRect, const QColor& color, QPainter* painter)
{
    return painter && AntIconSvgRenderer::drawIcon(*painter, iconName, iconRect, color);
}

QString iconNameForButton(const AntWindowChrome::TitleBarButtonState& state,
                          const AntWindowChrome::PaintOptions& options)
{
    switch (state.button)
    {
    case AntWindowChrome::TitleBarButton::Pin:
        return state.active ? QStringLiteral("PushpinFilled") : QStringLiteral("PushpinOutlined");
    case AntWindowChrome::TitleBarButton::Theme:
        return antTheme->themeMode() == Ant::ThemeMode::Dark
            ? QStringLiteral("SunOutlined")
            : QStringLiteral("MoonOutlined");
    case AntWindowChrome::TitleBarButton::Minimize:
        return QStringLiteral("MinusOutlined");
    case AntWindowChrome::TitleBarButton::Maximize:
        return options.maximized ? QStringLiteral("FullscreenExitOutlined")
                                 : QStringLiteral("FullscreenOutlined");
    case AntWindowChrome::TitleBarButton::Close:
        return QStringLiteral("CloseOutlined");
    default:
        return QString();
    }
}

QColor buttonIconColor(const AntWindowChrome::TitleBarButtonState& state)
{
    const auto& token = antTheme->tokens();
    if (state.destructive && (state.hovered || state.pressed))
    {
        return QColor(Qt::white);
    }
    if (state.active)
    {
        return token.colorPrimary;
    }
    return (state.hovered || state.pressed) ? token.colorTextSecondary : token.colorText;
}

void drawButtonBackground(QPainter* painter, const AntWindowChrome::TitleBarButtonState& state)
{
    if (!painter || !state.rect.isValid() || (!state.hovered && !state.pressed && !state.active))
    {
        return;
    }

    const auto& token = antTheme->tokens();
    painter->setPen(Qt::NoPen);
    if (state.destructive && (state.hovered || state.pressed))
    {
        painter->setBrush(state.pressed ? token.colorErrorActive : token.colorError);
    }
    else if (state.active)
    {
        painter->setBrush(token.colorPrimaryBg);
    }
    else
    {
        painter->setBrush(token.colorFillTertiary);
    }
    painter->drawRect(state.rect);
}

void drawSurfaceBorder(QPainter* painter, const QRect& rect, int cornerRadius, const QColor& borderColor)
{
    if (!painter || !rect.isValid())
    {
        return;
    }

    const auto& token = antTheme->tokens();
    QPen borderPen(borderColor.isValid() ? borderColor : token.colorBorderSecondary, token.lineWidth);
    borderPen.setCosmetic(true);
    painter->setPen(borderPen);
    painter->setBrush(Qt::NoBrush);
    if (cornerRadius > 0)
    {
        AntStyleBase::drawCrispRoundedRect(painter, rect, borderPen, Qt::NoBrush, cornerRadius, cornerRadius);
    }
    else
    {
        painter->drawRect(QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5));
    }
}
} // namespace

namespace AntWindowChrome
{
QRectF centeredIconRect(const QRect& buttonRect, qreal iconSize)
{
    if (buttonRect.isNull())
    {
        return {};
    }

    return QRectF(buttonRect.center().x() - iconSize / 2.0,
                  buttonRect.center().y() - iconSize / 2.0,
                  iconSize,
                  iconSize);
}

QColor legacyOutlineColor()
{
    return antTheme->themeMode() == Ant::ThemeMode::Dark
        ? QColor(255, 255, 255, 54)
        : QColor(0, 0, 0, 88);
}

void drawChrome(QPainter* painter, const PaintOptions& options)
{
    if (!painter || !options.surfaceRect.isValid())
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const QRect surface = options.surfaceRect;
    const int cornerRadius = qMax(0, options.cornerRadius);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath surfacePath;
    if (cornerRadius > 0)
    {
        surfacePath.addRoundedRect(QRectF(surface), cornerRadius, cornerRadius);
        painter->setClipPath(surfacePath);
    }
    else
    {
        surfacePath.addRect(QRectF(surface));
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(token.colorBgContainer);
    painter->drawPath(surfacePath);

    const bool hasTitleBar = options.titleBarRect.isValid();
    if (hasTitleBar)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorBgContainer);
        painter->drawRect(options.titleBarRect);

        painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter->setBrush(Qt::NoBrush);
        const qreal borderY = options.titleBarRect.bottom() + 0.5;
        painter->drawLine(QPointF(options.titleBarRect.left(), borderY),
                          QPointF(options.titleBarRect.right() + 1, borderY));

        const int iconSize = 16;
        const int iconLeft = options.titleBarRect.left() + 12;
        if (!options.icon.isNull())
        {
            const int iconTop = options.titleBarRect.top() + (options.titleBarRect.height() - iconSize) / 2;
            painter->drawPixmap(iconLeft, iconTop, options.icon.pixmap(iconSize, iconSize));
        }

        if (!options.title.isEmpty())
        {
            int controlsLeft = options.titleBarRect.right() + 1;
            for (const TitleBarButtonState& button : options.buttons)
            {
                if (button.rect.isValid())
                {
                    controlsLeft = qMin(controlsLeft, button.rect.left());
                }
            }

            QFont titleFont = options.widget ? options.widget->font() : painter->font();
            titleFont.setPixelSize(token.fontSizeLG);
            titleFont.setWeight(QFont::DemiBold);
            painter->setFont(titleFont);
            painter->setPen(token.colorText);

            const int textLeft = options.titleBarRect.left() + (options.icon.isNull() ? 12 : 12 + iconSize + 8);
            const int textMaxWidth = qMax(0, controlsLeft - textLeft - 12);
            const QRect textRect(textLeft, options.titleBarRect.top(), textMaxWidth, options.titleBarRect.height());
            painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, options.title);
        }

        for (const TitleBarButtonState& button : options.buttons)
        {
            if (!button.rect.isValid())
            {
                continue;
            }

            drawButtonBackground(painter, button);
            const QString iconName = iconNameForButton(button, options);
            if (!iconName.isEmpty())
            {
                drawAntdIcon(iconName, centeredIconRect(button.rect), buttonIconColor(button), painter);
            }
        }
    }

    if (options.drawSurfaceBorder)
    {
        drawSurfaceBorder(painter, surface, cornerRadius, options.surfaceBorderColor);
    }

    if (options.drawLegacyOutline)
    {
        QPen outlinePen(legacyOutlineColor());
        outlinePen.setCosmetic(true);
        outlinePen.setWidth(1);
        painter->setPen(outlinePen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(QRectF(surface).adjusted(0.5, 0.5, -0.5, -0.5));
    }

    painter->restore();
}
} // namespace AntWindowChrome
