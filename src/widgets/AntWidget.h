#pragma once

#include <QWidget>
#include "core/AntTypes.h"

struct AntThemeTokens;

class AntWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AntWidget(QWidget* parent = nullptr);
    ~AntWidget() override = default;

    const AntThemeTokens& tokens() const;
    Ant::ThemeMode currentTheme() const;

protected:
    virtual void onThemeChanged(Ant::ThemeMode mode);

private:
    void handleThemeChanged(Ant::ThemeMode mode);
};
