#pragma once

#include <QProxyStyle>

class AntCheckboxStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit AntCheckboxStyle(QStyle* style = nullptr);
    ~AntCheckboxStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr, const QWidget* widget = nullptr) const override;
    bool eventFilter(QObject* watched, QEvent* event) override;
};
