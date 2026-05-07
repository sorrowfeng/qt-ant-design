#pragma once

#include "core/QtAntDesignExport.h"

#include "core/AntStyleBase.h"

class QT_ANT_DESIGN_EXPORT AntResultStyle : public AntStyleBase
{
    Q_OBJECT

public:
    explicit AntResultStyle(QStyle* style = nullptr);
    ~AntResultStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget = nullptr) const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void drawResult(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
