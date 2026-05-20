#pragma once

#include "core/QtAntDesignExport.h"

#include "core/AntStyleBase.h"

#include <QHash>
#include <QString>

class QFont;

class QT_ANT_DESIGN_EXPORT AntMenuBarStyle : public AntStyleBase
{
    Q_OBJECT

public:
    explicit AntMenuBarStyle(QStyle* style = nullptr);

    void drawControl(ControlElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption* option,
                    const QWidget* widget) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                           const QSize& size, const QWidget* widget) const override;

protected:
    void onThemeUpdate(QWidget* w) override;

private:
    struct CachedTextMetric
    {
        QString displayText;
        int width = 0;
    };

    CachedTextMetric cachedTextMetric(const QString& text, const QFont& font, const QWidget* widget) const;
    void syncTextMetricCounters(const QWidget* widget) const;
    void clearTextMetricCache() const;

    mutable QHash<QString, CachedTextMetric> m_textMetricCache;
    mutable int m_textMetricCacheBuildCount = 0;
    mutable int m_textMetricCacheHitCount = 0;
};
