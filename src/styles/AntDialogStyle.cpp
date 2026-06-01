#include "AntDialogStyle.h"

#include <QPainter>
#include <QPaintEvent>

#include "styles/AntWindowChrome.h"
#include "widgets/AntDialog.h"

static_assert(AntDialog::TitleBarHeight == AntWindowChrome::TitleBarHeight,
              "AntDialog must reuse AntWindow chrome height.");
static_assert(AntDialog::TitleBarButtonWidth == AntWindowChrome::TitleBarButtonWidth,
              "AntDialog must reuse AntWindow chrome button width.");

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

    if (rounded)
    {
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        if (dialog->shadowMargin() > 0)
        {
            antTheme->drawEffectShadow(painter,
                                       surface,
                                       dialog->shadowMargin(),
                                       radius,
                                       antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.58 : 0.72);
        }
        painter->restore();
    }

    AntWindowChrome::PaintOptions chrome;
    chrome.surfaceRect = surface;
    chrome.titleBarRect = dialog->isTitleBarVisible() ? dialog->titleBarRect() : QRect();
    chrome.widget = dialog;
    chrome.title = dialog->windowTitle();
    chrome.icon = dialog->windowIcon();
    chrome.cornerRadius = radius;
    chrome.drawSurfaceBorder = true;
    chrome.surfaceBorderColor = dialog->usesLegacyOpaquePath()
        ? AntWindowChrome::legacyOutlineColor()
        : antTheme->tokens().colorBorderSecondary;

    if (dialog->isTitleBarVisible() && dialog->isCloseButtonVisible())
    {
        AntWindowChrome::TitleBarButtonState closeButton;
        closeButton.button = AntWindowChrome::TitleBarButton::Close;
        closeButton.rect = dialog->titleBarCloseButtonRect();
        closeButton.hovered = dialog->hoveredTitleBarButton() == AntDialog::TitleBarButton::Close;
        closeButton.pressed = dialog->pressedTitleBarButton() == AntDialog::TitleBarButton::Close;
        closeButton.destructive = true;
        chrome.buttons.append(closeButton);
    }

    AntWindowChrome::drawChrome(painter, chrome);
}
