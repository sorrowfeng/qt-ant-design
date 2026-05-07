#pragma once

#include "core/QtAntDesignExport.h"

#include "core/AntStyleBase.h"

class QT_ANT_DESIGN_EXPORT AntScrollBarStyle : public AntStyleBase
{
    Q_OBJECT

public:
    explicit AntScrollBarStyle(QStyle* style = nullptr);
    ~AntScrollBarStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;

    void drawComplexControl(ComplexControl control,
                            const QStyleOptionComplex* option,
                            QPainter* painter,
                            const QWidget* widget = nullptr) const override;

    QRect subControlRect(ComplexControl control,
                         const QStyleOptionComplex* option,
                         SubControl subControl,
                         const QWidget* widget = nullptr) const override;

    int pixelMetric(PixelMetric metric,
                    const QStyleOption* option = nullptr,
                    const QWidget* widget = nullptr) const override;

private:
    void drawScrollBar(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;
};
