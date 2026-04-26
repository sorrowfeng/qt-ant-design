#pragma once

#include "core/AntStyleBase.h"

class AntTagStyle : public AntStyleBase
{
    Q_OBJECT

public:
    explicit AntTagStyle(QStyle* style = nullptr);
    ~AntTagStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget = nullptr) const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void drawTag(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
