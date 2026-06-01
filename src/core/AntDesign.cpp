#include "AntDesign.h"

#include "AntFont.h"
#include "AntTheme.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QLoggingCategory>

namespace
{
Q_LOGGING_CATEGORY(lcAntDesignStartup, "qt.ant.design.startup")

bool s_highDpiConfigured = false;
} // namespace

void AntDesign::configureHighDpi()
{
    if (QCoreApplication::instance())
    {
        qCWarning(lcAntDesignStartup)
            << "AntDesign::configureHighDpi() must be called before QApplication is created.";
        return;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    s_highDpiConfigured = true;
}

void AntDesign::initialize(QApplication* application, int pixelSize)
{
    Q_INIT_RESOURCE(qt_ant_design);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const bool highDpiAttributesEnabled =
        QCoreApplication::testAttribute(Qt::AA_EnableHighDpiScaling) &&
        QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps);
    if (!s_highDpiConfigured && !highDpiAttributesEnabled)
    {
        qCWarning(lcAntDesignStartup)
            << "Qt 5 High DPI scaling was not configured. Call AntDesign::configureHighDpi() before QApplication.";
    }
#endif

    AntFont::applyToApplication(application, pixelSize);
    AntTheme::instance();
}
