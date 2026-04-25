#include "AntToolBar.h"

#include <QLayout>

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
