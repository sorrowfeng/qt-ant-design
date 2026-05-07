#pragma once

#include "QtAntDesignExport.h"

#include <QFont>
#include <QStringList>

#include "AntTypes.h"

class QApplication;

class QT_ANT_DESIGN_EXPORT AntFont
{
public:
    static QStringList families();
    static QStringList monoFamilies();

    static QFont defaultFont(int pixelSize = Ant::FontSize, QFont::Weight weight = QFont::Normal);
    static QFont monospaceFont(int pixelSize = 13, QFont::Weight weight = QFont::Normal);

    static bool installBundledFonts();
    static void applyToApplication(QApplication* application = nullptr, int pixelSize = Ant::FontSize);
};
