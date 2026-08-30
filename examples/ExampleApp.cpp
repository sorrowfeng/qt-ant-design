#include "ExampleApp.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QIcon>
#include <QTimer>

#include "ExampleWindow.h"
#include "core/AntDesign.h"
#include "core/AntTheme.h"

void initializeExampleApplication(QApplication& app)
{
    QApplication::setApplicationName(QStringLiteral("qt-ant-design-example"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/qt-ant-design-example/logo.png")));
    AntDesign::initialize(&app);
}

ExampleCommandLineOptions parseExampleCommandLine(QApplication& app)
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("qt-ant-design example application"));
    parser.addHelpOption();

    QCommandLineOption smokeExitOption(QStringLiteral("smoke-exit-ms"),
                                       QStringLiteral("Close the example automatically after <ms>."),
                                       QStringLiteral("ms"));
    QCommandLineOption stressThemeCyclesOption(QStringLiteral("stress-theme-cycles"),
                                               QStringLiteral("Toggle the theme <count> times after startup."),
                                               QStringLiteral("count"));
    QCommandLineOption stressThemeIntervalOption(QStringLiteral("stress-theme-interval-ms"),
                                                 QStringLiteral("Theme stress toggle interval in milliseconds."),
                                                 QStringLiteral("ms"),
                                                 QStringLiteral("25"));
    QCommandLineOption smokeTraversePagesOption(QStringLiteral("smoke-traverse-pages"),
                                                QStringLiteral("Traverse every example page in light and dark mode, grab each viewport, and exit non-zero on an invalid render."));
    QCommandLineOption smokeTraverseStepOption(QStringLiteral("smoke-traverse-step-ms"),
                                               QStringLiteral("Delay between example traversal steps in milliseconds."),
                                               QStringLiteral("ms"),
                                               QStringLiteral("5"));
    QCommandLineOption smokeTraverseExportDirOption(QStringLiteral("smoke-traverse-export-dir"),
                                                    QStringLiteral("Export example traversal PNGs and manifest.tsv to <dir>."),
                                                    QStringLiteral("dir"));
    QCommandLineOption smokeTraverseBaselineDirOption(QStringLiteral("smoke-traverse-baseline-dir"),
                                                      QStringLiteral("Compare example traversal PNGs with baseline images from <dir>."),
                                                      QStringLiteral("dir"));
    QCommandLineOption smokeTraversePageFilterOption(QStringLiteral("smoke-traverse-page-filter"),
                                                     QStringLiteral("Traverse only pages whose name contains <text>, or the page at zero-based index <text>."),
                                                     QStringLiteral("text"));
    QCommandLineOption smokeTraverseMaxMeanOption(QStringLiteral("smoke-traverse-max-mean-delta"),
                                                  QStringLiteral("Maximum allowed mean pixel delta when comparing traversal frames."),
                                                  QStringLiteral("delta"),
                                                  QStringLiteral("18"));
    QCommandLineOption smokeTraverseMaxChangedOption(QStringLiteral("smoke-traverse-max-changed32-ratio"),
                                                     QStringLiteral("Maximum allowed ratio of pixels whose average channel delta exceeds 32."),
                                                     QStringLiteral("ratio"),
                                                     QStringLiteral("0.28"));
    QCommandLineOption startPageNameOption(QStringLiteral("start-page-name"),
                                            QStringLiteral("Open the example at the page with <name>."),
                                            QStringLiteral("name"));
    QCommandLineOption startPageIndexOption(QStringLiteral("start-page-index"),
                                             QStringLiteral("Open the example at zero-based page <index>."),
                                             QStringLiteral("index"));
    parser.addOption(smokeExitOption);
    parser.addOption(stressThemeCyclesOption);
    parser.addOption(stressThemeIntervalOption);
    parser.addOption(smokeTraversePagesOption);
    parser.addOption(smokeTraverseStepOption);
    parser.addOption(smokeTraverseExportDirOption);
    parser.addOption(smokeTraverseBaselineDirOption);
    parser.addOption(smokeTraversePageFilterOption);
    parser.addOption(smokeTraverseMaxMeanOption);
    parser.addOption(smokeTraverseMaxChangedOption);
    parser.addOption(startPageNameOption);
    parser.addOption(startPageIndexOption);
    parser.process(app);

    ExampleCommandLineOptions options;

    bool ok = false;
    const int smokeExitMs = parser.value(smokeExitOption).toInt(&ok);
    if (ok && smokeExitMs >= 0)
    {
        options.smokeExitMs = smokeExitMs;
    }

    options.stressThemeCycles = parser.value(stressThemeCyclesOption).toInt(&ok);
    if (!ok || options.stressThemeCycles < 0)
    {
        options.stressThemeCycles = 0;
    }
    options.stressThemeIntervalMs = parser.value(stressThemeIntervalOption).toInt(&ok);
    if (!ok || options.stressThemeIntervalMs < 1)
    {
        options.stressThemeIntervalMs = 25;
    }

    options.traversePages = parser.isSet(smokeTraversePagesOption);
    options.traversalOptions.exportDir = parser.value(smokeTraverseExportDirOption);
    options.traversalOptions.baselineDir = parser.value(smokeTraverseBaselineDirOption);
    options.traversalOptions.pageFilter = parser.value(smokeTraversePageFilterOption);

    bool stepOk = false;
    options.traversalOptions.stepIntervalMs = parser.value(smokeTraverseStepOption).toInt(&stepOk);
    if (!stepOk)
    {
        options.traversalOptions.stepIntervalMs = 5;
    }

    bool thresholdOk = false;
    const qreal maxMean = parser.value(smokeTraverseMaxMeanOption).toDouble(&thresholdOk);
    if (thresholdOk)
    {
        options.traversalOptions.maxMeanDelta = maxMean;
    }
    const qreal maxChanged = parser.value(smokeTraverseMaxChangedOption).toDouble(&thresholdOk);
    if (thresholdOk)
    {
        options.traversalOptions.maxChanged32Ratio = maxChanged;
    }

    options.startPageName = parser.value(startPageNameOption);
    bool startIndexOk = false;
    const int startIndex = parser.value(startPageIndexOption).toInt(&startIndexOk);
    if (startIndexOk && startIndex >= 0)
    {
        options.startPageIndex = startIndex;
    }

    return options;
}

int runExampleApplication(QApplication& app, const ExampleCommandLineOptions& options)
{
    ExampleWindow window;
    window.setMinimumSize(960, 640);
    window.resize(1200, 800);
    window.show();

    if (!options.startPageName.isEmpty())
    {
        const int count = window.examplePageCount();
        for (int i = 0; i < count; ++i)
        {
            if (window.examplePageName(i) == options.startPageName)
            {
                window.setExamplePageIndex(i);
                break;
            }
        }
    }
    else if (options.startPageIndex >= 0)
    {
        window.setExamplePageIndex(options.startPageIndex);
    }

    if (options.traversePages)
    {
        startExamplePageTraversal(app, window, options.traversalOptions);
    }

    if (options.stressThemeCycles > 0)
    {
        auto* themeTimer = new QTimer(&window);
        QObject::connect(themeTimer, &QTimer::timeout, &window, [themeTimer, remaining = options.stressThemeCycles]() mutable {
            antTheme->toggleThemeMode();
            --remaining;
            if (remaining <= 0)
            {
                themeTimer->stop();
            }
        });
        themeTimer->start(options.stressThemeIntervalMs);
    }

    if (options.smokeExitMs >= 0)
    {
        QTimer::singleShot(options.smokeExitMs, &window, [&window]() { window.forceClose(); });
    }

    return app.exec();
}
