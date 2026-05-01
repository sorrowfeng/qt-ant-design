#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QTimer>

#include "ExampleWindow.h"
#include "core/AntFont.h"
#include "core/AntTheme.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("qt-ant-design-example"));
    AntFont::applyToApplication(&app);

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
    parser.addOption(smokeExitOption);
    parser.addOption(stressThemeCyclesOption);
    parser.addOption(stressThemeIntervalOption);
    parser.process(app);

    ExampleWindow window;
    window.setMinimumSize(960, 640);
    window.resize(1200, 800);
    window.show();

    bool ok = false;
    const int stressThemeCycles = parser.value(stressThemeCyclesOption).toInt(&ok);
    if (ok && stressThemeCycles > 0)
    {
        const int intervalMs = qMax(1, parser.value(stressThemeIntervalOption).toInt(&ok));
        auto* themeTimer = new QTimer(&window);
        QObject::connect(themeTimer, &QTimer::timeout, &window, [themeTimer, remaining = stressThemeCycles]() mutable {
            antTheme->toggleThemeMode();
            --remaining;
            if (remaining <= 0)
            {
                themeTimer->stop();
            }
        });
        themeTimer->start(intervalMs);
    }

    const int smokeExitMs = parser.value(smokeExitOption).toInt(&ok);
    if (ok && smokeExitMs >= 0)
    {
        QTimer::singleShot(smokeExitMs, &window, &QWidget::close);
    }

    return app.exec();
}
