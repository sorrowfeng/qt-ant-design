#include "AntDialogStyle.h"

#include <QFont>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>

#include "widgets/AntDialog.h"

AntDialogStyle::AntDialogStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntDialog>();
}

void AntDialogStyle::polish(QWidget* widget)
{
    AntStyleBase::polish(widget);
    installPaintFilter<AntDialog>(widget);
}

void AntDialogStyle::unpolish(QWidget* widget)
{
    removePaintFilter<AntDialog>(widget);
    AntStyleBase::unpolish(widget);
}

bool AntDialogStyle::drawWidget(QWidget* widget, QPaintEvent* event)
{
    auto* dialog = qobject_cast<AntDialog*>(widget);
    if (!dialog)
    {
        return false;
    }

    QPainter painter(dialog);
    if (event)
    {
        painter.setClipRegion(event->region());
    }
    drawDialog(dialog, &painter);
    return true;
}

QSize AntDialogStyle::sizeFromContents(ContentsType type, const QStyleOption* option,
                                       const QSize& size, const QWidget* widget) const
{
    if (qobject_cast<const AntDialog*>(widget) && type == QStyle::CT_CustomBase)
    {
        return size;
    }
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

void AntDialogStyle::onThemeUpdate(QWidget* w)
{
    if (auto* dialog = qobject_cast<AntDialog*>(w))
    {
        dialog->refreshAntStyle();
    }
    AntStyleBase::onThemeUpdate(w);
}

void AntDialogStyle::drawDialog(AntDialog* dialog, QPainter* painter) const
{
    if (!dialog || !painter)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const QRect surface = dialog->surfaceRect().adjusted(0, 0, -1, -1);
    if (!surface.isValid())
    {
        return;
    }

    const bool rounded = dialog->usesRoundedCorners();
    const int radius = dialog->effectiveCornerRadius();
    if (rounded)
    {
        painter->save();
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(dialog->rect(), Qt::transparent);
        painter->restore();
    }

    painter->save();
    if (rounded)
    {
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        if (dialog->shadowMargin() > 0)
        {
            antTheme->drawEffectShadow(painter,
                                       surface,
                                       dialog->shadowMargin(),
                                       radius,
                                       antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.58 : 0.72);
        }
        AntStyleBase::drawCrispRoundedRect(painter, surface,
                                           QPen(token.colorBorderSecondary, token.lineWidth),
                                           token.colorBgContainer,
                                           radius, radius);
    }
    else
    {
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->fillRect(surface, token.colorBgContainer);
        painter->setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(surface);
    }
    painter->restore();

    drawTitleBar(dialog, painter);
}

void AntDialogStyle::drawTitleBar(AntDialog* dialog, QPainter* painter) const
{
    if (!dialog || !painter || !dialog->isTitleBarVisible())
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const QRect titleRect = dialog->titleBarRect();
    if (!titleRect.isValid())
    {
        return;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    if (dialog->usesRoundedCorners())
    {
        QPainterPath clip;
        const QRectF surface = dialog->surfaceRect().adjusted(0, 0, -1, -1);
        const int radius = dialog->effectiveCornerRadius();
        clip.addRoundedRect(surface, radius, radius);
        painter->setClipPath(clip, Qt::IntersectClip);
    }
    painter->fillRect(titleRect, token.colorBgElevated);

    painter->setPen(QPen(token.colorSplit, token.lineWidth));
    painter->drawLine(titleRect.bottomLeft(), titleRect.bottomRight());

    QFont titleFont = dialog->font();
    titleFont.setPixelSize(token.fontSizeLG);
    titleFont.setWeight(QFont::DemiBold);
    painter->setFont(titleFont);
    painter->setPen(token.colorText);
    painter->drawText(dialog->titleBarTextRect(),
                      Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
                      dialog->windowTitle());
    painter->restore();

    drawCloseButton(dialog, painter);
}

void AntDialogStyle::drawCloseButton(AntDialog* dialog, QPainter* painter) const
{
    if (!dialog || !painter || !dialog->isTitleBarVisible() || !dialog->isCloseButtonVisible())
    {
        return;
    }

    const QRect buttonRect = dialog->titleBarCloseButtonRect();
    if (!buttonRect.isValid())
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const bool hovered = dialog->hoveredTitleBarButton() == AntDialog::TitleBarButton::Close;
    const bool pressed = dialog->pressedTitleBarButton() == AntDialog::TitleBarButton::Close;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    if (dialog->usesRoundedCorners())
    {
        QPainterPath clip;
        const QRectF surface = dialog->surfaceRect().adjusted(0, 0, -1, -1);
        const int radius = dialog->effectiveCornerRadius();
        clip.addRoundedRect(surface, radius, radius);
        painter->setClipPath(clip, Qt::IntersectClip);
    }
    if (hovered || pressed)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(hovered ? token.colorError : token.colorErrorActive);
        painter->drawRect(buttonRect);
    }

    const QColor iconColor = hovered || pressed ? QColor(Qt::white) : token.colorText;
    const QPoint center = buttonRect.center();
    const int half = 5;
    painter->setPen(QPen(iconColor, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(center + QPoint(-half, -half), center + QPoint(half, half));
    painter->drawLine(center + QPoint(half, -half), center + QPoint(-half, half));
    painter->restore();
}
