#pragma once

#include <QProxyStyle>

class AntCascaderStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit AntCascaderStyle(QStyle* style = nullptr);
    ~AntCascaderStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void drawCascader(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
