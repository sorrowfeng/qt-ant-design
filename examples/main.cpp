#include <QApplication>
#include <QColor>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QEvent>
#include <QEventLoop>
#include <QFile>
#include <QIcon>
#include <QImage>
#include <QScrollBar>
#include <QTextStream>
#include <QTimer>
#include <QVector>

#include "ExampleWindow.h"
#include "core/AntDesign.h"
#include "core/AntTheme.h"
#include "widgets/AntScrollArea.h"

namespace
{
struct ExampleDiffStats
{
    qreal meanDelta = 0.0;
    qreal changed32Ratio = 0.0;
    qreal changed64Ratio = 0.0;
    int maxDelta = 0;
};

struct ScrollProbe
{
    QString name;
    int value = 0;
};

struct ExampleTraversalOptions
{
    int stepIntervalMs = 5;
    QString exportDir;
    QString baselineDir;
    QString pageFilter;
    qreal maxMeanDelta = 18.0;
    qreal maxChanged32Ratio = 0.28;
};

int examplePixelDelta(const QColor& a, const QColor& b)
{
    return (qAbs(a.red() - b.red()) +
            qAbs(a.green() - b.green()) +
            qAbs(a.blue() - b.blue()) +
            qAbs(a.alpha() - b.alpha())) /
           4;
}

ExampleDiffStats compareExampleFrames(const QImage& actual, const QImage& baseline)
{
    ExampleDiffStats stats;
    qint64 totalDelta = 0;
    int changed32 = 0;
    int changed64 = 0;
    const int pixelCount = actual.width() * actual.height();
    for (int y = 0; y < actual.height(); ++y)
    {
        for (int x = 0; x < actual.width(); ++x)
        {
            const int delta = examplePixelDelta(actual.pixelColor(x, y), baseline.pixelColor(x, y));
            totalDelta += delta;
            stats.maxDelta = qMax(stats.maxDelta, delta);
            if (delta > 32)
            {
                ++changed32;
            }
            if (delta > 64)
            {
                ++changed64;
            }
        }
    }
    stats.meanDelta = pixelCount > 0 ? static_cast<qreal>(totalDelta) / pixelCount : 0.0;
    stats.changed32Ratio = pixelCount > 0 ? static_cast<qreal>(changed32) / pixelCount : 0.0;
    stats.changed64Ratio = pixelCount > 0 ? static_cast<qreal>(changed64) / pixelCount : 0.0;
    return stats;
}

QString sanitizeExampleFramePart(const QString& text)
{
    QString out;
    out.reserve(text.size());
    for (const QChar ch : text)
    {
        if (ch.isLetterOrNumber())
        {
            out.append(ch.toLower());
        }
        else if (!out.endsWith(QLatin1Char('-')))
        {
            out.append(QLatin1Char('-'));
        }
    }
    while (out.startsWith(QLatin1Char('-')))
    {
        out.remove(0, 1);
    }
    while (out.endsWith(QLatin1Char('-')))
    {
        out.chop(1);
    }
    return out.isEmpty() ? QStringLiteral("page") : out;
}

QString exampleFrameName(const QString& themeLabel,
                         int pageIndex,
                         const QString& pageName,
                         const QString& scrollProbe)
{
    return QStringLiteral("%1-%2-%3-%4")
        .arg(themeLabel,
             QStringLiteral("%1").arg(pageIndex, 2, 10, QLatin1Char('0')),
             sanitizeExampleFramePart(pageName),
             scrollProbe);
}

void appendTextLine(const QString& path, const QString& line)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        qWarning().noquote() << QStringLiteral("Cannot append traversal output %1").arg(path);
        return;
    }
    QTextStream out(&file);
    out << line << '\n';
}

bool prepareTraversalOutput(const ExampleTraversalOptions& options)
{
    if (options.exportDir.isEmpty())
    {
        return true;
    }

    QDir dir(options.exportDir);
    if (!dir.exists() && !QDir().mkpath(options.exportDir))
    {
        qWarning().noquote() << QStringLiteral("Cannot create traversal export dir %1").arg(options.exportDir);
        return false;
    }

    QFile::remove(dir.filePath(QStringLiteral("manifest.tsv")));
    QFile::remove(dir.filePath(QStringLiteral("comparison.tsv")));
    appendTextLine(dir.filePath(QStringLiteral("manifest.tsv")),
                   QStringLiteral("frame\tqtVersion\ttheme\tpageIndex\tpageName\tscrollProbe\tscrollY\twidth\theight"));
    if (!options.baselineDir.isEmpty())
    {
        appendTextLine(dir.filePath(QStringLiteral("comparison.tsv")),
                       QStringLiteral("frame\tactualQtVersion\tbaselineDir\tmeanDelta\tmaxMeanDelta\tchanged32Ratio\tmaxChanged32Ratio\tchanged64Ratio\tmaxDelta"));
    }
    return true;
}

bool exampleFrameLooksVisible(const QImage& image, QString* reason)
{
    if (image.isNull() || image.width() < 320 || image.height() < 240)
    {
        if (reason)
        {
            *reason = QStringLiteral("invalid image size %1x%2").arg(image.width()).arg(image.height());
        }
        return false;
    }

    int samples = 0;
    int opaque = 0;
    int minLuma = 255;
    int maxLuma = 0;
    for (int y = 0; y < image.height(); y += 6)
    {
        for (int x = 0; x < image.width(); x += 6)
        {
            const QColor color = image.pixelColor(x, y);
            ++samples;
            if (color.alpha() <= 24)
            {
                continue;
            }
            ++opaque;
            const int luma = qRound(0.2126 * color.red() + 0.7152 * color.green() + 0.0722 * color.blue());
            minLuma = qMin(minLuma, luma);
            maxLuma = qMax(maxLuma, luma);
        }
    }

    if (opaque < samples * 8 / 10)
    {
        if (reason)
        {
            *reason = QStringLiteral("too few opaque samples: %1/%2").arg(opaque).arg(samples);
        }
        return false;
    }
    if (maxLuma - minLuma < 24)
    {
        if (reason)
        {
            *reason = QStringLiteral("too little luminance variation: %1..%2").arg(minLuma).arg(maxLuma);
        }
        return false;
    }
    return true;
}

QList<ScrollProbe> pageScrollProbes(QWidget* pageWrapper, bool stableProbes)
{
    QList<ScrollProbe> positions{{QStringLiteral("top"), 0}};
    auto* scroll = pageWrapper ? pageWrapper->findChild<AntScrollArea*>() : nullptr;
    auto* bar = scroll ? scroll->verticalScrollBar() : nullptr;
    if (stableProbes || (bar && bar->maximum() > 0))
    {
        positions.append({QStringLiteral("bottom"), bar ? bar->maximum() : 0});
    }
    return positions;
}

bool renderExamplePageForSmoke(QApplication& app,
                               ExampleWindow& window,
                               int pageIndex,
                               const ScrollProbe& scrollProbe,
                               const QString& themeLabel,
                               const ExampleTraversalOptions& options)
{
    if (!window.setExamplePageIndex(pageIndex))
    {
        qWarning().noquote() << QStringLiteral("Example traversal failed to select page %1").arg(pageIndex);
        return false;
    }

    QWidget* pageWrapper = window.examplePageWidget(pageIndex);
    auto* scroll = pageWrapper ? pageWrapper->findChild<AntScrollArea*>() : nullptr;
    if (scroll && scroll->verticalScrollBar())
    {
        scroll->verticalScrollBar()->setValue(scrollProbe.value);
    }

    QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 25);
    window.repaint();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 25);

    QString reason;
    const QImage image = window.grab().toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (!exampleFrameLooksVisible(image, &reason))
    {
        qWarning().noquote()
            << QStringLiteral("Example traversal rendered invalid frame at %1 page %2 (%3), scroll %4: %5")
                   .arg(themeLabel)
                   .arg(pageIndex)
                   .arg(window.examplePageName(pageIndex))
                   .arg(scrollProbe.value)
                   .arg(reason);
        return false;
    }

    const QString frameName = exampleFrameName(themeLabel,
                                               pageIndex,
                                               window.examplePageName(pageIndex),
                                               scrollProbe.name);
    if (!options.exportDir.isEmpty())
    {
        QDir dir(options.exportDir);
        const QString imagePath = dir.filePath(frameName + QStringLiteral(".png"));
        if (!image.save(imagePath))
        {
            qWarning().noquote() << QStringLiteral("Cannot save traversal frame %1").arg(imagePath);
            return false;
        }
        appendTextLine(dir.filePath(QStringLiteral("manifest.tsv")),
                       QStringLiteral("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9")
                           .arg(frameName,
                                QStringLiteral(QT_VERSION_STR),
                                themeLabel,
                                QString::number(pageIndex),
                                window.examplePageName(pageIndex),
                                scrollProbe.name,
                                QString::number(scrollProbe.value),
                                QString::number(image.width()),
                                QString::number(image.height())));
    }

    if (!options.baselineDir.isEmpty())
    {
        const QString baselinePath = QDir(options.baselineDir).filePath(frameName + QStringLiteral(".png"));
        const QImage baseline(baselinePath);
        if (baseline.isNull())
        {
            qWarning().noquote() << QStringLiteral("Missing traversal baseline %1").arg(baselinePath);
            return false;
        }
        if (baseline.size() != image.size())
        {
            qWarning().noquote()
                << QStringLiteral("Traversal frame size mismatch for %1: actual %2x%3, baseline %4x%5")
                       .arg(frameName)
                       .arg(image.width())
                       .arg(image.height())
                       .arg(baseline.width())
                       .arg(baseline.height());
            return false;
        }

        const ExampleDiffStats stats = compareExampleFrames(image, baseline);
        if (!options.exportDir.isEmpty())
        {
            appendTextLine(QDir(options.exportDir).filePath(QStringLiteral("comparison.tsv")),
                           QStringLiteral("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9")
                               .arg(frameName,
                                    QStringLiteral(QT_VERSION_STR),
                                    QDir::toNativeSeparators(options.baselineDir),
                                    QString::number(stats.meanDelta, 'f', 3),
                                    QString::number(options.maxMeanDelta, 'f', 3),
                                    QString::number(stats.changed32Ratio, 'f', 4),
                                    QString::number(options.maxChanged32Ratio, 'f', 4),
                                    QString::number(stats.changed64Ratio, 'f', 4),
                                    QString::number(stats.maxDelta)));
        }

        if (stats.meanDelta > options.maxMeanDelta ||
            stats.changed32Ratio > options.maxChanged32Ratio ||
            stats.changed64Ratio > options.maxChanged32Ratio / 2.0)
        {
            qWarning().noquote()
                << QStringLiteral("Traversal frame %1 exceeded visual threshold: mean %2/%3, changed32 %4/%5, changed64 %6/%7, max %8")
                       .arg(frameName)
                       .arg(stats.meanDelta, 0, 'f', 3)
                       .arg(options.maxMeanDelta, 0, 'f', 3)
                       .arg(stats.changed32Ratio, 0, 'f', 4)
                       .arg(options.maxChanged32Ratio, 0, 'f', 4)
                       .arg(stats.changed64Ratio, 0, 'f', 4)
                       .arg(options.maxChanged32Ratio / 2.0, 0, 'f', 4)
                       .arg(stats.maxDelta);
            return false;
        }
    }

    Q_UNUSED(app);
    return true;
}

void startExamplePageTraversal(QApplication& app, ExampleWindow& window, const ExampleTraversalOptions& options)
{
    QVector<int> pageIndices;
    const int availablePageCount = window.examplePageCount();
    const QString normalizedPageFilter = options.pageFilter.trimmed().toLower();
    for (int i = 0; i < availablePageCount; ++i)
    {
        const QString pageName = window.examplePageName(i);
        if (normalizedPageFilter.isEmpty() ||
            pageName.toLower().contains(normalizedPageFilter) ||
            QString::number(i) == normalizedPageFilter)
        {
            pageIndices.append(i);
        }
    }

    const int pageCount = pageIndices.size();
    if (pageCount <= 0)
    {
        qWarning("Example traversal found no pages.");
        QTimer::singleShot(0, &app, [&app]() { app.exit(2); });
        return;
    }
    if (!prepareTraversalOutput(options))
    {
        QTimer::singleShot(0, &app, [&app]() { app.exit(5); });
        return;
    }

    auto* timer = new QTimer(&window);
    timer->setInterval(qMax(1, options.stepIntervalMs));
    QObject::connect(timer, &QTimer::timeout, &window, [&app, &window, timer, pageCount,
                                                        pageIndices,
                                                        options,
                                                        pageIndex = 0,
                                                        themePass = 0,
                                                        scrollPositions = QList<ScrollProbe>(),
                                                        scrollIndex = 0]() mutable {
        if (themePass >= 2)
        {
            timer->stop();
            window.forceClose();
            app.exit(0);
            return;
        }

        if (pageIndex == 0 && scrollIndex == 0 && scrollPositions.isEmpty())
        {
            antTheme->setThemeMode(themePass == 0 ? Ant::ThemeMode::Default : Ant::ThemeMode::Dark);
            QCoreApplication::processEvents(QEventLoop::AllEvents, 25);
        }

        if (scrollPositions.isEmpty())
        {
            const int actualPageIndex = pageIndices.at(pageIndex);
            if (!window.setExamplePageIndex(actualPageIndex))
            {
                qWarning().noquote() << QStringLiteral("Example traversal failed to select page %1").arg(actualPageIndex);
                app.exit(3);
                timer->stop();
                return;
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents, 25);
            scrollPositions = pageScrollProbes(window.examplePageWidget(actualPageIndex),
                                               !options.exportDir.isEmpty() || !options.baselineDir.isEmpty());
            scrollIndex = 0;
        }

        const QString themeLabel = themePass == 0 ? QStringLiteral("light") : QStringLiteral("dark");
        const ScrollProbe scrollProbe = scrollPositions.value(scrollIndex, {QStringLiteral("top"), 0});
        const int actualPageIndex = pageIndices.at(pageIndex);
        if (!renderExamplePageForSmoke(app, window, actualPageIndex, scrollProbe, themeLabel, options))
        {
            app.exit(4);
            timer->stop();
            return;
        }

        ++scrollIndex;
        if (scrollIndex >= scrollPositions.size())
        {
            scrollPositions.clear();
            scrollIndex = 0;
            ++pageIndex;
            if (pageIndex >= pageCount)
            {
                pageIndex = 0;
                ++themePass;
            }
        }
    });
    timer->start();
}
} // namespace

int main(int argc, char* argv[])
{
    AntDesign::configureHighDpi();

    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("qt-ant-design-example"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/qt-ant-design-example/logo.png")));
    AntDesign::initialize(&app);

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
    parser.process(app);

    ExampleWindow window;
    window.setMinimumSize(960, 640);
    window.resize(1200, 800);
    window.show();

    if (parser.isSet(smokeTraversePagesOption))
    {
        bool stepOk = false;
        ExampleTraversalOptions traversalOptions;
        traversalOptions.stepIntervalMs = parser.value(smokeTraverseStepOption).toInt(&stepOk);
        if (!stepOk)
        {
            traversalOptions.stepIntervalMs = 5;
        }
        traversalOptions.exportDir = parser.value(smokeTraverseExportDirOption);
        traversalOptions.baselineDir = parser.value(smokeTraverseBaselineDirOption);
        traversalOptions.pageFilter = parser.value(smokeTraversePageFilterOption);

        bool thresholdOk = false;
        const qreal maxMean = parser.value(smokeTraverseMaxMeanOption).toDouble(&thresholdOk);
        if (thresholdOk)
        {
            traversalOptions.maxMeanDelta = maxMean;
        }
        const qreal maxChanged = parser.value(smokeTraverseMaxChangedOption).toDouble(&thresholdOk);
        if (thresholdOk)
        {
            traversalOptions.maxChanged32Ratio = maxChanged;
        }
        startExamplePageTraversal(app, window, traversalOptions);
    }

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
        QTimer::singleShot(smokeExitMs, &window, &AntWindow::forceClose);
    }

    return app.exec();
}
