#include "AntToolBar.h"

#include <QActionEvent>
#include <QLayout>
#include <QPalette>
#include <QToolButton>

#include "../styles/AntToolBarStyle.h"
#include "core/AntTheme.h"

AntToolBar::AntToolBar(QWidget* parent)
    : QToolBar(parent)
{
    auto* s = new AntToolBarStyle(style());
    s->setParent(this);
    setStyle(s);

    setMovable(true);
    setFloatable(true);
    setMouseTracking(true);
    setAttribute(Qt::WA_TranslucentBackground);

    if (auto* lay = layout())
    {
        lay->setSpacing(8);
        lay->setContentsMargins(4, 4, 4, 4);
    }

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateActionButtons();
        update();
    });

    connect(this, &QToolBar::topLevelChanged, this, [this](bool floating) {
        Q_UNUSED(floating)
        update();
    });
}

AntToolBar::AntToolBar(const QString& title, QWidget* parent)
    : AntToolBar(parent)
{
    setWindowTitle(title);
}

void AntToolBar::actionEvent(QActionEvent* event)
{
    QToolBar::actionEvent(event);
    updateActionButtons();
}

void AntToolBar::updateActionButtons()
{
    const auto& token = antTheme->tokens();
    QPalette pal = palette();
    pal.setColor(QPalette::ButtonText, token.colorText);
    pal.setColor(QPalette::WindowText, token.colorText);
    pal.setColor(QPalette::Button, token.colorBgContainer);
    pal.setColor(QPalette::Text, token.colorText);
    setPalette(pal);

    const auto buttons = findChildren<QToolButton*>(QString(), Qt::FindDirectChildrenOnly);
    for (QToolButton* button : buttons)
    {
        if (!button)
        {
            continue;
        }
        button->setProperty("antToolBarButton", true);
        button->setAutoRaise(false);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setCursor(Qt::PointingHandCursor);
        button->setMouseTracking(true);
        button->setAttribute(Qt::WA_Hover, true);
        button->setPalette(pal);
        if (button->style() != style())
        {
            button->setStyle(style());
        }
        button->updateGeometry();
        button->update();
    }
}
