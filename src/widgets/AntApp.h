#pragma once

#include "core/QtAntDesignExport.h"

#include <QObject>
#include <QPointer>
#include <QWidget>

#include <functional>

class AntMessage;
class AntModal;
class AntNotification;

class QT_ANT_DESIGN_EXPORT AntApp : public QObject
{
    Q_OBJECT

public:
    explicit AntApp(QWidget* rootWidget, QObject* parent = nullptr);
    ~AntApp() override;

    QWidget* rootWidget() const;
    QWidget* feedbackHost() const;

    static AntApp* instance();

    void showMessage(const QString& text, int durationMs = 3000);
    void showModal(const QString& title, const QString& body,
                   std::function<void()> onOk = nullptr, std::function<void()> onCancel = nullptr);
    void showNotification(const QString& title, const QString& body);

private:
    QPointer<QWidget> m_root;
    mutable QPointer<QWidget> m_feedbackHost;
    mutable bool m_feedbackHostResolved = false;
    mutable int m_feedbackHostResolveCount = 0;

    static AntApp* s_instance;
};
