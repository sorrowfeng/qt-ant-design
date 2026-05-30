#pragma once

#include "core/QtAntDesignExport.h"

#include "core/AntStyleBase.h"

class QT_ANT_DESIGN_EXPORT AntStackedWidgetStyle : public AntStyleBase
{
public:
    explicit AntStackedWidgetStyle(QStyle* style = nullptr);

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                       QPainter* painter, const QWidget* widget) const override;
    bool drawWidget(QWidget* widget, QPaintEvent* event) override;

private:
    void drawStackedWidgetFrame(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
