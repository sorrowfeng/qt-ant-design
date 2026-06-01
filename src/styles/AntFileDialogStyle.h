#pragma once

#include "core/QtAntDesignExport.h"

#include "AntDialogStyle.h"

class QT_ANT_DESIGN_EXPORT AntFileDialogStyle : public AntDialogStyle
{
public:
    explicit AntFileDialogStyle(QStyle* style = nullptr);

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                       QPainter* painter, const QWidget* widget) const override;
    void drawControl(ControlElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const override;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                            QPainter* painter, const QWidget* widget) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption* option,
                    const QWidget* widget) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                           const QSize& size, const QWidget* widget) const override;

protected:
    void onThemeUpdate(QWidget* w) override;

private:
    bool isFileDialogScoped(const QWidget* widget) const;
    void drawDialogSurface(const QStyleOption* option, QPainter* painter) const;
    void drawLineEditPanel(const QStyleOption* option, QPainter* painter,
                           const QWidget* widget) const;
    void drawButtonPanel(const QStyleOption* option, QPainter* painter,
                         const QWidget* widget, bool primary) const;
    void drawPushButton(const QStyleOption* option, QPainter* painter,
                        const QWidget* widget) const;
    void drawToolButton(const QStyleOptionComplex* option, QPainter* painter,
                        const QWidget* widget) const;
    void drawComboBox(const QStyleOptionComplex* option, QPainter* painter,
                      const QWidget* widget) const;
    void drawItemViewItem(const QStyleOption* option, QPainter* painter,
                          const QWidget* widget) const;
    void drawHeaderSection(const QStyleOption* option, QPainter* painter,
                           const QWidget* widget) const;
    void drawHeaderLabel(const QStyleOption* option, QPainter* painter,
                         const QWidget* widget) const;
};
