#pragma once

#include "core/AntStyleBase.h"

class AntSegmentedStyle : public AntStyleBase
{
    Q_OBJECT

public:
    explicit AntSegmentedStyle(QStyle* style = nullptr);
    ~AntSegmentedStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget = nullptr) const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void drawSegmented(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
