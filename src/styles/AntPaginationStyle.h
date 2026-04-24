#pragma once

#include <QProxyStyle>

class AntPaginationStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit AntPaginationStyle(QStyle* style = nullptr);
    ~AntPaginationStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget = nullptr) const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void drawPagination(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
