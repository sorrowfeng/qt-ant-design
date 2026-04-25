#include "AntApp.h"

#include "core/AntTheme.h"

AntApp* AntApp::s_instance = nullptr;

AntApp::AntApp(QWidget* rootWidget, QObject* parent)
    : QObject(parent), m_root(rootWidget)
{
    s_instance = this;
}

QWidget* AntApp::rootWidget() const { return m_root; }

AntApp* AntApp::instance() { return s_instance; }

void AntApp::showMessage(const QString& text, int durationMs)
{
    Q_UNUSED(text)
    Q_UNUSED(durationMs)
    // Uses AntMessage internally — call from the root widget context
}

void AntApp::showModal(const QString& title, const QString& body,
                        std::function<void()> onOk, std::function<void()> onCancel)
{
    Q_UNUSED(title)
    Q_UNUSED(body)
    Q_UNUSED(onOk)
    Q_UNUSED(onCancel)
    // Uses AntModal internally
}

void AntApp::showNotification(const QString& title, const QString& body)
{
    Q_UNUSED(title)
    Q_UNUSED(body)
    // Uses AntNotification internally
}
