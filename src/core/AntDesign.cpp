#include "AntDesign.h"

#include "AntFont.h"
#include "AntTheme.h"

void AntDesign::initialize(QApplication* application, int pixelSize)
{
    Q_INIT_RESOURCE(qt_ant_design);

    AntFont::applyToApplication(application, pixelSize);
    AntTheme::instance();
}
