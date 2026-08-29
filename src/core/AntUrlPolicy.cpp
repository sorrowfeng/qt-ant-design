#include "AntUrlPolicy.h"

#include <QDesktopServices>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QWriteLocker>

#include <utility>

namespace
{
struct UrlPolicyState
{
    QReadWriteLock lock;
    QStringList allowedSchemes{QStringLiteral("http"), QStringLiteral("https")};
    AntUrlPolicy::ApprovalCallback approvalCallback;
};

UrlPolicyState& policyState()
{
    static UrlPolicyState state;
    return state;
}

QStringList normalizedSchemes(const QStringList& schemes)
{
    QStringList normalized;
    for (const QString& value : schemes)
    {
        const QString scheme = value.trimmed().toLower();
        if (scheme.isEmpty() || scheme.contains(QLatin1Char(':')) ||
            scheme.contains(QLatin1Char('/')) || scheme.contains(QLatin1Char('\\')))
        {
            continue;
        }
        if (!normalized.contains(scheme))
        {
            normalized.append(scheme);
        }
    }
    return normalized;
}
} // namespace

QStringList AntUrlPolicy::allowedSchemes()
{
    UrlPolicyState& state = policyState();
    QReadLocker locker(&state.lock);
    return state.allowedSchemes;
}

void AntUrlPolicy::setAllowedSchemes(const QStringList& schemes)
{
    UrlPolicyState& state = policyState();
    QWriteLocker locker(&state.lock);
    state.allowedSchemes = normalizedSchemes(schemes);
}

void AntUrlPolicy::setApprovalCallback(ApprovalCallback callback)
{
    UrlPolicyState& state = policyState();
    QWriteLocker locker(&state.lock);
    state.approvalCallback = std::move(callback);
}

void AntUrlPolicy::reset()
{
    UrlPolicyState& state = policyState();
    QWriteLocker locker(&state.lock);
    state.allowedSchemes = QStringList{QStringLiteral("http"), QStringLiteral("https")};
    state.approvalCallback = {};
}

bool AntUrlPolicy::isExternalUrlAllowed(const QUrl& url)
{
    if (!url.isValid() || url.isRelative() || url.isLocalFile() || url.scheme().isEmpty())
    {
        return false;
    }

    ApprovalCallback approvalCallback;
    {
        UrlPolicyState& state = policyState();
        QReadLocker locker(&state.lock);
        if (state.allowedSchemes.contains(url.scheme().toLower()))
        {
            return true;
        }
        approvalCallback = state.approvalCallback;
    }
    return approvalCallback && approvalCallback(url);
}

bool AntUrlPolicy::openExternalUrl(const QUrl& url)
{
    return isExternalUrlAllowed(url) && QDesktopServices::openUrl(url);
}
