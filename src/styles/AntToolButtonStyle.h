#pragma once

#include "core/QtAntDesignExport.h"

#include "core/AntStyleBase.h"

class QT_ANT_DESIGN_EXPORT AntToolButtonStyle : public AntStyleBase
{
    Q_OBJECT

public:
    explicit AntToolButtonStyle(QStyle* style = nullptr);

    void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                            QPainter* painter, const QWidget* widget) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption* option,
                    const QWidget* widget) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                           const QSize& size, const QWidget* widget) const override;

private:
    void drawToolButton(const QStyleOptionComplex* option, QPainter* painter,
                        const QWidget* widget) const;
};
