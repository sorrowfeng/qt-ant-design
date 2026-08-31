#pragma once

#include "core/QtAntDesignExport.h"

#include <QObject>
#include <QColor>
#include <Qt>

#include "core/AntTypes.h"

class QT_ANT_DESIGN_EXPORT AntConfigProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Ant::ThemeMode themeMode READ themeMode WRITE setThemeMode NOTIFY themeModeChanged)
    Q_PROPERTY(QColor primaryColor READ primaryColor WRITE setPrimaryColor NOTIFY primaryColorChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius NOTIFY borderRadiusChanged)
    Q_PROPERTY(Ant::ThemeDensity density READ density WRITE setDensity NOTIFY densityChanged)
    Q_PROPERTY(Qt::LayoutDirection direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(int revision READ revision NOTIFY configChanged)

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

    // 主题密度（对应上游 compactAlgorithm）。
    Ant::ThemeDensity density() const;
    void setDensity(Ant::ThemeDensity density);

    // 全局布局方向（对应上游 direction="rtl"），apply() 时写入 QApplication。
    Qt::LayoutDirection direction() const;
    void setDirection(Qt::LayoutDirection direction);

    int revision() const;

    void apply();

Q_SIGNALS:
    void themeModeChanged(Ant::ThemeMode mode);
    void primaryColorChanged(const QColor& color);
    void fontSizeChanged(int size);
    void borderRadiusChanged(int radius);
    void densityChanged(Ant::ThemeDensity density);
    void directionChanged(Qt::LayoutDirection direction);
    void configChanged();

private:
    void scheduleConfigChanged();

    Ant::ThemeMode m_themeMode = Ant::ThemeMode::Default;
    QColor m_primaryColor;
    int m_fontSize = 14;
    int m_borderRadius = 6;
    Ant::ThemeDensity m_density = Ant::ThemeDensity::Default;
    Qt::LayoutDirection m_direction = Qt::LeftToRight;
    bool m_configChangedScheduled = false;
    int m_revision = 0;
};
