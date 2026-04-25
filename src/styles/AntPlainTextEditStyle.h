#pragma once

#include <QProxyStyle>

class AntPlainTextEditStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit AntPlainTextEditStyle(QStyle* style = nullptr);

    void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                       QPainter* painter, const QWidget* widget) const override;
    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void drawFrame(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
