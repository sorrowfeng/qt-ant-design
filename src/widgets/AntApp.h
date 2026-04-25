#pragma once

#include <QObject>
#include <QWidget>

class AntMessage;
class AntModal;
class AntNotification;

class AntApp : public QObject
{
    Q_OBJECT

public:
    explicit AntApp(QWidget* rootWidget, QObject* parent = nullptr);

    QWidget* rootWidget() const;

    static AntApp* instance();

    void showMessage(const QString& text, int durationMs = 3000);
    void showModal(const QString& title, const QString& body,
                   std::function<void()> onOk = nullptr, std::function<void()> onCancel = nullptr);
    void showNotification(const QString& title, const QString& body);

private:
    QWidget* m_root = nullptr;
    static AntApp* s_instance;
};
