#include "AntPalette.h"

#include <algorithm>

namespace
{
int clampChannel(qreal value)
{
    return std::clamp(static_cast<int>(std::round(value)), 0, 255);
}

// Ant Design 5.x preset color primaries (from ant-design/ant-design .../seed.ts)
struct PresetEntry
{
    const char* name;
    const char* hex;
};

constexpr PresetEntry kPresetColors[] = {
    {"blue",     "#1677FF"},
    {"purple",   "#722ED1"},
    {"cyan",     "#13C2C2"},
    {"green",    "#52C41A"},
    {"magenta",  "#EB2F96"},
    {"pink",     "#EB2F96"},
    {"red",      "#F5222D"},
    {"orange",   "#FA8C16"},
    {"yellow",   "#FADB14"},
    {"volcano",  "#FA541C"},
    {"geekblue", "#2F54EB"},
    {"gold",     "#FAAD14"},
    {"lime",     "#A0D911"},
};
}

QColor AntPalette::fromHex(const char* hex)
{
    return QColor(QString::fromLatin1(hex));
}

QColor AntPalette::mix(const QColor& first, const QColor& second, qreal amount)
{
    const qreal t = std::clamp(amount, 0.0, 1.0);
    return QColor(
        clampChannel(first.red() * (1.0 - t) + second.red() * t),
        clampChannel(first.green() * (1.0 - t) + second.green() * t),
        clampChannel(first.blue() * (1.0 - t) + second.blue() * t),
        clampChannel(first.alpha() * (1.0 - t) + second.alpha() * t));
}

QColor AntPalette::tint(const QColor& color, qreal amount)
{
    return mix(color, QColor(255, 255, 255), amount);
}

QColor AntPalette::shade(const QColor& color, qreal amount)
{
    return mix(color, QColor(0, 0, 0), amount);
}

QColor AntPalette::alpha(const QColor& color, qreal opacity)
{
    QColor result = color;
    result.setAlphaF(std::clamp(opacity, 0.0, 1.0));
    return result;
}

QColor AntPalette::solidColor(const QColor& base, qreal overlayOpacity)
{
    if (base.lightnessF() > 0.5)
    {
        return shade(base, overlayOpacity);
    }
    return tint(base, overlayOpacity);
}

QVector<QColor> AntPalette::generate(const QColor& base, Ant::ThemeMode mode)
{
    const QVector<QColor> lightPalette = {
        tint(base, 0.92),
        tint(base, 0.82),
        tint(base, 0.62),
        tint(base, 0.38),
        tint(base, 0.15),
        base,
        shade(base, 0.15),
        shade(base, 0.28),
        shade(base, 0.40),
        shade(base, 0.52),
    };

    if (mode == Ant::ThemeMode::Default)
    {
        return lightPalette;
    }

    const QColor darkBackground(20, 20, 20);
    return {
        mix(darkBackground, lightPalette[7], 0.15),
        mix(darkBackground, lightPalette[7], 0.25),
        mix(darkBackground, lightPalette[6], 0.35),
        mix(darkBackground, lightPalette[5], 0.45),
        mix(darkBackground, lightPalette[5], 0.65),
        lightPalette[5],
        lightPalette[4],
        lightPalette[5],
        lightPalette[4],
        lightPalette[3],
    };
}

QColor AntPalette::hoverColor(const QColor& base, Ant::ThemeMode mode)
{
    const auto palette = generate(base, mode);
    return mode == Ant::ThemeMode::Dark ? palette[6] : palette[4];
}

QColor AntPalette::activeColor(const QColor& base, Ant::ThemeMode mode)
{
    const auto palette = generate(base, mode);
    return mode == Ant::ThemeMode::Dark ? palette[4] : palette[6];
}

QColor AntPalette::backgroundColor(const QColor& base, Ant::ThemeMode mode)
{
    const auto palette = generate(base, mode);
    return palette[0];
}

QColor AntPalette::borderColor(const QColor& base, Ant::ThemeMode mode)
{
    const auto palette = generate(base, mode);
    return mode == Ant::ThemeMode::Dark ? palette[3] : palette[2];
}

QColor AntPalette::disabledColor(const QColor& foreground, const QColor& background)
{
    return mix(background, foreground, 0.25);
}

QColor AntPalette::presetColor(const QString& name)
{
    const QString key = name.trimmed().toLower();
    for (const auto& entry : kPresetColors)
    {
        if (key == QLatin1String(entry.name))
        {
            return QColor(QLatin1String(entry.hex));
        }
    }
    return QColor();
}
