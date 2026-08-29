#include "AntApp.h"

#include "AntMessage.h"
#include "AntModal.h"
#include "AntNotification.h"

#include <QApplication>
#include <QList>
#include <QTimer>

#include <utility>

AntApp* AntApp::s_instance = nullptr;

namespace
{
QList<AntApp*>& appStack()
{
    static QList<AntApp*> stack;
    return stack;
}

bool feedbackHostCanPresent(QWidget* host)
{
    if (!host)
    {
        return false;
    }

    QWidget* window = host->window();
    return host->isVisible() &&
           (!window || window->isVisible()) &&
           !host->size().isEmpty() &&
           (!window || !window->size().isEmpty());
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

AntMessage* AntApp::lastMessage() const { return m_lastMessage.data(); }
AntModal* AntApp::lastModal() const { return m_lastModal.data(); }
AntNotification* AntApp::lastNotification() const { return m_lastNotification.data(); }

void AntApp::showMessage(const QString& text, int durationMs)
{
    m_lastMessage.clear();
    QWidget* host = feedbackHost();
    if (!feedbackHostCanPresent(host))
    {
        Q_EMIT feedbackFailed(QStringLiteral("message"));
        return;
    }

    m_lastMessage = AntMessage::info(text, host, durationMs);
    if (!m_lastMessage || m_lastMessage->property("antMessageSuppressedForHiddenAnchor").toBool())
    {
        m_lastMessage.clear();
        Q_EMIT feedbackFailed(QStringLiteral("message"));
        return;
    }
    m_lastMessage->setObjectName(QStringLiteral("antAppMessage"));
    Q_EMIT messageShown(m_lastMessage.data());
}

void AntApp::showModal(const QString& title, const QString& body,
                        std::function<void()> onOk, std::function<void()> onCancel)
{
    m_lastModal.clear();
    QWidget* host = feedbackHost();
    if (!feedbackHostCanPresent(host))
    {
        Q_EMIT feedbackFailed(QStringLiteral("modal"));
        return;
    }

    m_lastModal = AntModal::confirm(title, body, host);
    if (!m_lastModal || !m_lastModal->isOpen() || !m_lastModal->isVisible())
    {
        if (m_lastModal)
        {
            m_lastModal->deleteLater();
            m_lastModal.clear();
        }
        Q_EMIT feedbackFailed(QStringLiteral("modal"));
        return;
    }
    m_lastModal->setObjectName(QStringLiteral("antAppModal"));
    AntModal* modal = m_lastModal.data();
    connect(modal, &AntModal::confirmed, modal, [callback = std::move(onOk)]() mutable {
        if (callback)
        {
            callback();
        }
    });
    connect(modal, &AntModal::canceled, modal, [callback = std::move(onCancel)]() mutable {
        if (callback)
        {
            callback();
        }
    });
    connect(modal, &AntModal::openChanged, modal, [modal](bool open) {
        if (!open)
        {
            QTimer::singleShot(260, modal, [modal]() {
                if (!modal->isOpen())
                {
                    modal->deleteLater();
                }
            });
        }
    });
    Q_EMIT modalShown(modal);
}

void AntApp::showNotification(const QString& title, const QString& body)
{
    m_lastNotification.clear();
    QWidget* host = feedbackHost();
    if (!feedbackHostCanPresent(host))
    {
        Q_EMIT feedbackFailed(QStringLiteral("notification"));
        return;
    }

    m_lastNotification = AntNotification::info(title, body, host);
    if (!m_lastNotification ||
        m_lastNotification->property("antNotificationSuppressedForHiddenAnchor").toBool())
    {
        m_lastNotification.clear();
        Q_EMIT feedbackFailed(QStringLiteral("notification"));
        return;
    }
    m_lastNotification->setObjectName(QStringLiteral("antAppNotification"));
    Q_EMIT notificationShown(m_lastNotification.data());
}
