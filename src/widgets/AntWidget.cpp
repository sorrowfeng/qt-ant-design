#include "AntWidget.h"

#include <QApplication>
#include <QStyle>

#include "core/AntTheme.h"

AntWidget::AntWidget(QWidget* parent)
    : QWidget(parent)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, &AntWidget::handleThemeChanged);
}

const AntThemeTokens& AntWidget::tokens() const
{
    return antTheme->tokens();
}

Ant::ThemeMode AntWidget::currentTheme() const
{
    return antTheme->themeMode();
}

void AntWidget::onThemeChanged(Ant::ThemeMode mode)
{
    Q_UNUSED(mode)
}

void AntWidget::handleThemeChanged(Ant::ThemeMode mode)
{
    style()->unpolish(this);
    style()->polish(this);
    updateGeometry();
    update();
    onThemeChanged(mode);
}
