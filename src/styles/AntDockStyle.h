#pragma once

#include "core/QtAntDesignExport.h"

#include "core/AntStyleBase.h"

class QT_ANT_DESIGN_EXPORT AntDockStyle : public AntStyleBase
{
public:
    explicit AntDockStyle(QStyle* style = nullptr);

    void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                       QPainter* painter, const QWidget* widget = nullptr) const override;
    void drawControl(ControlElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget = nullptr) const override;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                            QPainter* painter, const QWidget* widget = nullptr) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr,
                    const QWidget* widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                           const QSize& size, const QWidget* widget = nullptr) const override;

private:
    bool isDockManagedWidget(const QWidget* widget) const;
    void drawDockTabPane(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
    void drawDockTab(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
    void drawDockTabShape(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
    void drawDockTabLabel(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
    void drawDockToolButton(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;
    void drawDockSplitter(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
