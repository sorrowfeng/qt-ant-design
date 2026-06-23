#pragma once

#include "ExampleSmoke.h"

class QApplication;

struct ExampleCommandLineOptions
{
    int smokeExitMs = -1;
    int stressThemeCycles = 0;
    int stressThemeIntervalMs = 25;
    bool traversePages = false;
    ExampleTraversalOptions traversalOptions;
};

void initializeExampleApplication(QApplication& app);
ExampleCommandLineOptions parseExampleCommandLine(QApplication& app);
int runExampleApplication(QApplication& app, const ExampleCommandLineOptions& options);
