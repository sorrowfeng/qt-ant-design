#include "AntFont.h"

#include <QApplication>
#include <QFontDatabase>

namespace
{
bool s_fontLoadAttempted = false;
bool s_bundledFontsLoaded = false;

QFont makeFont(const QStringList& families, int pixelSize, QFont::Weight weight)
{
    QFont font;
    font.setFamilies(families);
    font.setPixelSize(pixelSize);
    font.setWeight(weight);
    font.setStyleStrategy(QFont::PreferAntialias);
    return font;
}
}

QStringList AntFont::families()
{
    return {
        QStringLiteral("AlibabaSans"),
        QStringLiteral("Segoe UI"),
        QStringLiteral("Roboto"),
        QStringLiteral("Helvetica Neue"),
        QStringLiteral("Arial"),
        QStringLiteral("Noto Sans"),
        QStringLiteral("Microsoft YaHei UI"),
        QStringLiteral("Microsoft YaHei"),
        QStringLiteral("sans-serif"),
    };
}

QStringList AntFont::monoFamilies()
{
    return {
        QStringLiteral("SFMono-Regular"),
        QStringLiteral("Consolas"),
        QStringLiteral("Liberation Mono"),
        QStringLiteral("Menlo"),
        QStringLiteral("Courier"),
        QStringLiteral("monospace"),
    };
}

QFont AntFont::defaultFont(int pixelSize, QFont::Weight weight)
{
    return makeFont(families(), pixelSize, weight);
}

QFont AntFont::monospaceFont(int pixelSize, QFont::Weight weight)
{
    return makeFont(monoFamilies(), pixelSize, weight);
}

bool AntFont::installBundledFonts()
{
    if (s_fontLoadAttempted)
    {
        return s_bundledFontsLoaded;
    }

    s_fontLoadAttempted = true;
    s_bundledFontsLoaded =
        QFontDatabase::addApplicationFont(QStringLiteral(":/qt-ant-design/fonts/NotoSans.ttf")) >= 0;
    return s_bundledFontsLoaded;
}

void AntFont::applyToApplication(QApplication* application, int pixelSize)
{
    installBundledFonts();

    if (application || QApplication::instance())
    {
        QApplication::setFont(defaultFont(pixelSize));
    }
}
