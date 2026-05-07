#pragma once

#include "core/QtAntDesignExport.h"

#include "core/AntStyleBase.h"

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
};
