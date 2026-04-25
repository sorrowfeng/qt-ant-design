#pragma once

#include <QObject>
#include <QColor>

#include "core/AntTypes.h"

class AntConfigProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Ant::ThemeMode themeMode READ themeMode WRITE setThemeMode NOTIFY themeModeChanged)
    Q_PROPERTY(QColor primaryColor READ primaryColor WRITE setPrimaryColor NOTIFY primaryColorChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius NOTIFY borderRadiusChanged)

public:
    explicit AntConfigProvider(QObject* parent = nullptr);

    Ant::ThemeMode themeMode() const;
    void setThemeMode(Ant::ThemeMode mode);

    QColor primaryColor() const;
    void setPrimaryColor(const QColor& color);

    int fontSize() const;
    void setFontSize(int size);

    int borderRadius() const;
    void setBorderRadius(int radius);

    void apply();

Q_SIGNALS:
    void themeModeChanged(Ant::ThemeMode mode);
    void primaryColorChanged(const QColor& color);
    void fontSizeChanged(int size);
    void borderRadiusChanged(int radius);

private:
    Ant::ThemeMode m_themeMode = Ant::ThemeMode::Default;
    QColor m_primaryColor;
    int m_fontSize = 14;
    int m_borderRadius = 6;
};
