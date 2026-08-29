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

    AntMessage* lastMessage() const;
    AntModal* lastModal() const;
    AntNotification* lastNotification() const;

    void showMessage(const QString& text, int durationMs = 3000);
    void showModal(const QString& title, const QString& body,
                   std::function<void()> onOk = nullptr, std::function<void()> onCancel = nullptr);
    void showNotification(const QString& title, const QString& body);

Q_SIGNALS:
    void messageShown(AntMessage* message);
    void modalShown(AntModal* modal);
    void notificationShown(AntNotification* notification);
    void feedbackFailed(const QString& feedbackType);

private:
    QPointer<QWidget> m_root;
    mutable QPointer<QWidget> m_feedbackHost;
    mutable bool m_feedbackHostResolved = false;
    mutable int m_feedbackHostResolveCount = 0;
    QPointer<AntMessage> m_lastMessage;
    QPointer<AntModal> m_lastModal;
    QPointer<AntNotification> m_lastNotification;

    static AntApp* s_instance;
};
