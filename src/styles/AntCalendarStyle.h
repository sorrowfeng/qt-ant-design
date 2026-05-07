#pragma once

#include "core/QtAntDesignExport.h"

#include "core/AntStyleBase.h"

class QT_ANT_DESIGN_EXPORT AntCalendarStyle : public AntStyleBase
{
    Q_OBJECT

public:
    explicit AntCalendarStyle(QStyle* style = nullptr);

    void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                       QPainter* painter, const QWidget* widget) const override;
    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

protected:
    void onThemeUpdate(QWidget* w) override;

private:
    void drawFrame(const QStyleOption* option, QPainter* painter) const;
};
