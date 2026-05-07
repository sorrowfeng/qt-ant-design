#pragma once

#include "core/QtAntDesignExport.h"

#include "core/AntStyleBase.h"

class QT_ANT_DESIGN_EXPORT AntRadioStyle : public AntStyleBase
{
    Q_OBJECT

public:
    explicit AntRadioStyle(QStyle* style = nullptr);
    ~AntRadioStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr, const QWidget* widget = nullptr) const override;
    bool eventFilter(QObject* watched, QEvent* event) override;
};
