#pragma once

#include "core/QtAntDesignExport.h"

#include "core/AntStyleBase.h"

class QT_ANT_DESIGN_EXPORT AntSelectStyle : public AntStyleBase
{
    Q_OBJECT

public:
    explicit AntSelectStyle(QStyle* style = nullptr);
    ~AntSelectStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void drawSelect(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
