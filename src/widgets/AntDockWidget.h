#pragma once

#include "core/QtAntDesignExport.h"

#include <QDockWidget>

class QWidget;

class QT_ANT_DESIGN_EXPORT AntDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit AntDockWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    explicit AntDockWidget(const QString& title, QWidget* parent = nullptr,
                           Qt::WindowFlags flags = Qt::WindowFlags());

    void setWidget(QWidget* widget);

protected:
#if defined(Q_OS_WIN)
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
#endif

private:
    void setupTitleBar();
    void updateTheme();
};
