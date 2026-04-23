#pragma once

#include <QProxyStyle>

class AntSpinStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit AntSpinStyle(QStyle* style = nullptr);
    ~AntSpinStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void drawSpin(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
