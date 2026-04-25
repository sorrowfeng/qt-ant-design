#include "AntMenuBar.h"

#include <QMenu>

#include "../styles/AntMenuBarStyle.h"
#include "core/AntTheme.h"

AntMenuBar::AntMenuBar(QWidget* parent)
    : QMenuBar(parent)
{
    auto* s = new AntMenuBarStyle(style());
    s->setParent(this);
    setStyle(s);
    setMouseTracking(true);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        update();
    });
}

QMenu* AntMenuBar::addMenu(const QString& title)
{
    auto* menu = new QMenu(title, this);
    QMenuBar::addMenu(menu);
    return menu;
}

QMenu* AntMenuBar::addMenu(const QIcon& icon, const QString& title)
{
    auto* menu = new QMenu(title, this);
    menu->setIcon(icon);
    QMenuBar::addMenu(menu);
    return menu;
}
