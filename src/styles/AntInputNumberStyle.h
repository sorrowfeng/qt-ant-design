#pragma once

#include <QProxyStyle>

class AntInputNumberStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit AntInputNumberStyle(QStyle* style = nullptr);
    ~AntInputNumberStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawComplexControl(ComplexControl control,
                            const QStyleOptionComplex* option,
                            QPainter* painter,
                            const QWidget* widget = nullptr) const override;
    QRect subControlRect(ComplexControl control,
                         const QStyleOptionComplex* option,
                         SubControl subControl,
                         const QWidget* widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type,
                           const QStyleOption* option,
                           const QSize& contentsSize,
                           const QWidget* widget = nullptr) const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void drawSpinBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;
};
