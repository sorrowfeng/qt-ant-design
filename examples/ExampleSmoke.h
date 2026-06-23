#pragma once

#include <QString>

class QApplication;
class ExampleWindow;

struct ExampleTraversalOptions
{
    int stepIntervalMs = 5;
    QString exportDir;
    QString baselineDir;
    QString pageFilter;
    qreal maxMeanDelta = 18.0;
    qreal maxChanged32Ratio = 0.28;
};

void startExamplePageTraversal(QApplication& app, ExampleWindow& window, const ExampleTraversalOptions& options);
