#include <QApplication>

#include "ExampleApp.h"
#include "core/AntDesign.h"

int main(int argc, char* argv[])
{
    AntDesign::configureHighDpi();

    QApplication app(argc, argv);
    initializeExampleApplication(app);

    const ExampleCommandLineOptions options = parseExampleCommandLine(app);
    return runExampleApplication(app, options);
}
