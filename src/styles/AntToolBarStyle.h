#pragma once

#include <QProxyStyle>

class AntToolBarStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit AntToolBarStyle(QStyle* style = nullptr);

    void drawControl(ControlElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                       QPainter* painter, const QWidget* widget) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption* option,
                    const QWidget* widget) const override;
};
