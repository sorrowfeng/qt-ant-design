#pragma once

#include <QProxyStyle>

class AntMenuBarStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit AntMenuBarStyle(QStyle* style = nullptr);

    void drawControl(ControlElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption* option,
                    const QWidget* widget) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                           const QSize& size, const QWidget* widget) const override;
};
