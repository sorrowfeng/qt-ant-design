#pragma once

#include "core/QtAntDesignExport.h"

#include "core/AntStyleBase.h"

class QT_ANT_DESIGN_EXPORT AntBorderBeamStyle : public AntStyleBase
{
    Q_OBJECT

public:
    explicit AntBorderBeamStyle(QStyle* style = nullptr);
    ~AntBorderBeamStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    bool drawWidget(QWidget* widget, QPaintEvent* event) override;
};
