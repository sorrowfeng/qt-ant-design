#pragma once

#include "core/QtAntDesignExport.h"

#include "AntDialogStyle.h"

class QT_ANT_DESIGN_EXPORT AntInputDialogStyle : public AntDialogStyle
{
public:
    explicit AntInputDialogStyle(QStyle* style = nullptr);

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;

protected:
    void onThemeUpdate(QWidget* w) override;
};
