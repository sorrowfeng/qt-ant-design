#include "AntApp.h"

#include <QApplication>
#include <QList>

AntApp* AntApp::s_instance = nullptr;

namespace
{
QList<AntApp*>& appStack()
{
    static QList<AntApp*> stack;
    return stack;
}
}

AntApp::AntApp(QWidget* rootWidget, QObject* parent)
    : QObject(parent), m_root(rootWidget)
{
    appStack().removeAll(this);
    appStack().append(this);
    s_instance = this;
}

AntApp::~AntApp()
{
    auto& stack = appStack();
    stack.removeAll(this);
    s_instance = stack.isEmpty() ? nullptr : stack.constLast();
}

QWidget* AntApp::rootWidget() const { return m_root.data(); }

QWidget* AntApp::feedbackHost() const
{
    if (m_feedbackHostResolved && m_feedbackHost)
    {
        const_cast<AntApp*>(this)->setProperty("antAppFeedbackHostCacheHit", true);
        return m_feedbackHost.data();
    }
    if (m_feedbackHostResolved && !m_feedbackHost)
    {
        m_feedbackHostResolved = false;
    }

    QWidget* host = m_root.data();
    if (!host)
    {
        host = qobject_cast<QWidget*>(parent());
    }
    if (!host)
    {
        host = QApplication::activeWindow();
    }

    m_feedbackHost = host;
    m_feedbackHostResolved = true;
    ++m_feedbackHostResolveCount;

    auto* self = const_cast<AntApp*>(this);
    if (host)
    {
        connect(host, &QObject::destroyed, self, [this]() {
            m_feedbackHost.clear();
            m_feedbackHostResolved = false;
        });
    }
    self->setProperty("antAppFeedbackHostCacheHit", false);
    self->setProperty("antAppFeedbackHostResolveCount", m_feedbackHostResolveCount);
    return host;
}

AntApp* AntApp::instance() { return s_instance; }

void AntApp::showMessage(const QString& text, int durationMs)
{
    QWidget* host = feedbackHost();
    Q_UNUSED(host)
    Q_UNUSED(text)
    Q_UNUSED(durationMs)
}

void AntApp::showModal(const QString& title, const QString& body,
                        std::function<void()> onOk, std::function<void()> onCancel)
{
    QWidget* host = feedbackHost();
    Q_UNUSED(host)
    Q_UNUSED(title)
    Q_UNUSED(body)
    Q_UNUSED(onOk)
    Q_UNUSED(onCancel)
}

void AntApp::showNotification(const QString& title, const QString& body)
{
    QWidget* host = feedbackHost();
    Q_UNUSED(host)
    Q_UNUSED(title)
    Q_UNUSED(body)
}
