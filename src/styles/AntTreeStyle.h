#pragma once

#include <QProxyStyle>

class AntTreeStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit AntTreeStyle(QStyle* style = nullptr);
    ~AntTreeStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void drawTree(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
