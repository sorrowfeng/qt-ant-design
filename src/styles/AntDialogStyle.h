#pragma once

#include "core/QtAntDesignExport.h"

#include "core/AntStyleBase.h"

class AntDialog;

class QT_ANT_DESIGN_EXPORT AntDialogStyle : public AntStyleBase
{
    Q_OBJECT

public:
    explicit AntDialogStyle(QStyle* style = nullptr);

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    bool drawWidget(QWidget* widget, QPaintEvent* event) override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                           const QSize& size, const QWidget* widget) const override;

protected:
    void onThemeUpdate(QWidget* w) override;

private:
    void drawDialog(AntDialog* dialog, QPainter* painter) const;
    void drawTitleBar(AntDialog* dialog, QPainter* painter) const;
    void drawCloseButton(AntDialog* dialog, QPainter* painter) const;
};
