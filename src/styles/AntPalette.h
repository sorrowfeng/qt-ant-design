#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QVector>

#include "core/AntTypes.h"

class QT_ANT_DESIGN_EXPORT AntPalette
{
public:
    static QColor fromHex(const char* hex);
    static QColor mix(const QColor& first, const QColor& second, qreal amount);
    static QColor tint(const QColor& color, qreal amount);
    static QColor shade(const QColor& color, qreal amount);
    static QColor alpha(const QColor& color, qreal opacity);
    static QColor solidColor(const QColor& base, qreal overlayOpacity);

    static QVector<QColor> generate(const QColor& base, Ant::ThemeMode mode = Ant::ThemeMode::Default);
    static QColor hoverColor(const QColor& base, Ant::ThemeMode mode = Ant::ThemeMode::Default);
    static QColor activeColor(const QColor& base, Ant::ThemeMode mode = Ant::ThemeMode::Default);
    static QColor backgroundColor(const QColor& base, Ant::ThemeMode mode = Ant::ThemeMode::Default);
    static QColor borderColor(const QColor& base, Ant::ThemeMode mode = Ant::ThemeMode::Default);
    static QColor disabledColor(const QColor& foreground, const QColor& background);

    // Look up Ant Design preset color by name (blue/purple/cyan/green/magenta/pink/
    // red/orange/yellow/volcano/geekblue/gold/lime). Returns invalid QColor if name
    // does not match a preset.
    static QColor presetColor(const QString& name);
};
