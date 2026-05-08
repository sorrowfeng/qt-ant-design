#pragma once

#include "QtAntDesignExport.h"

#include "AntTypes.h"

class QApplication;

class QT_ANT_DESIGN_EXPORT AntDesign
{
public:
    static void initialize(QApplication* application = nullptr, int pixelSize = Ant::FontSize);
};
