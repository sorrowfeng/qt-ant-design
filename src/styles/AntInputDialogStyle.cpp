#include "AntInputDialogStyle.h"

#include "widgets/AntInputDialog.h"

AntInputDialogStyle::AntInputDialogStyle(QStyle* style)
    : AntDialogStyle(style)
{
}

void AntInputDialogStyle::polish(QWidget* widget)
{
    AntDialogStyle::polish(widget);
    if (qobject_cast<AntInputDialog*>(widget))
    {
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntInputDialogStyle::unpolish(QWidget* widget)
{
    AntDialogStyle::unpolish(widget);
}

void AntInputDialogStyle::onThemeUpdate(QWidget* w)
{
    AntDialogStyle::onThemeUpdate(w);
}
