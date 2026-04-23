#pragma once

#include <QProxyStyle>

class AntSliderStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit AntSliderStyle(QStyle* style = nullptr);
    ~AntSliderStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void drawSlider(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
