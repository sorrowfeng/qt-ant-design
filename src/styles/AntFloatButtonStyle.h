#pragma once

#include "core/QtAntDesignExport.h"

#include "core/AntStyleBase.h"

class QT_ANT_DESIGN_EXPORT AntFloatButtonStyle : public AntStyleBase
{
    Q_OBJECT

public:
    explicit AntFloatButtonStyle(QStyle* style = nullptr);
    ~AntFloatButtonStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget = nullptr) const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void drawFloatButton(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
    void drawMainButton(const QStyleOption* option, QPainter* painter, const class AntFloatButton* fb) const;
    void drawChildButton(const QStyleOption* option, QPainter* painter, const class AntFloatButton* fb, int index) const;
};
