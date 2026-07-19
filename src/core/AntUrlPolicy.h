#pragma once

#include "QtAntDesignExport.h"

#include <QStringList>
#include <QUrl>

#include <functional>

class QT_ANT_DESIGN_EXPORT AntUrlPolicy
{
public:
    using ApprovalCallback = std::function<bool(const QUrl&)>;

    static QStringList allowedSchemes();
    static void setAllowedSchemes(const QStringList& schemes);
    static void setApprovalCallback(ApprovalCallback callback);
    static void reset();

    static bool isExternalUrlAllowed(const QUrl& url);
    static bool openExternalUrl(const QUrl& url);
};
