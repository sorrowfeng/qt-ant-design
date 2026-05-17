#pragma once

#include "core/QtAntDesignExport.h"

#include <QDockWidget>
#include <QRect>

class QEvent;
class QWidget;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;

class QT_ANT_DESIGN_EXPORT AntDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit AntDockWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    explicit AntDockWidget(const QString& title, QWidget* parent = nullptr,
                           Qt::WindowFlags flags = Qt::WindowFlags());

    void setWidget(QWidget* widget);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;
#if defined(Q_OS_WIN)
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
#endif

private:
    void setupTitleBar();
    void updateTheme();
    void updateFloatingFrame();
    QRect floatingPanelRect() const;
    int floatingShadowMargin() const;
    int floatingCornerRadius() const;

    bool m_floatingFrameActive = false;
};
