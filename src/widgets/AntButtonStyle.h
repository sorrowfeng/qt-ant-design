#pragma once

#include <QProxyStyle>

class AntButtonStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit AntButtonStyle(QStyle* style = nullptr);
    ~AntButtonStyle() override = default;

    void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr, const QWidget* widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget = nullptr) const override;

private:
    void drawButton(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
