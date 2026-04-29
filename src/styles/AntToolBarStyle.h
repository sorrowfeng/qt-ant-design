#pragma once

#include "core/AntStyleBase.h"

class AntToolBarStyle : public AntStyleBase
{
    Q_OBJECT

public:
    explicit AntToolBarStyle(QStyle* style = nullptr);

    void drawControl(ControlElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const override;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                            QPainter* painter, const QWidget* widget) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                           const QSize& size, const QWidget* widget) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                       QPainter* painter, const QWidget* widget) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption* option,
                    const QWidget* widget) const override;

protected:
    void onThemeUpdate(QWidget* w) override;

private:
    void drawToolBarButton(const QStyleOptionComplex* option, QPainter* painter,
                           const QWidget* widget) const;
};
