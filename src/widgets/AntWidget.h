#pragma once

#include "core/QtAntDesignExport.h"

#include <QSize>
#include <QWidget>
#include "core/AntTypes.h"

struct AntThemeTokens;
class QStyle;

class QT_ANT_DESIGN_EXPORT AntWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AntWidget(QWidget* parent = nullptr);
    ~AntWidget() override = default;

    const AntThemeTokens& tokens() const;
    Ant::ThemeMode currentTheme() const;

protected:
    virtual void onThemeChanged(Ant::ThemeMode mode);

private:
    void handleThemeChanged(Ant::ThemeMode mode);
    bool refreshCachedHints();
    void refreshThemeCache();
    void syncPerfCounters() const;

    QStyle* m_cachedStyle = nullptr;
    QSize m_cachedSizeHint;
    QSize m_cachedMinimumSizeHint;
    qint64 m_cachedPaletteKey = 0;
    int m_themeChangeCount = 0;
    int m_repolishCount = 0;
    int m_updateGeometryCount = 0;
    int m_surfaceUpdateCount = 0;
};
