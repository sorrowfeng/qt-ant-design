#include "AntWindowsSystemLibrary.h"

#include <QDir>
#include <QFileInfo>

#include <iterator>

#ifdef Q_OS_WIN
#include <qt_windows.h>

#ifndef LOAD_LIBRARY_SEARCH_SYSTEM32
#define LOAD_LIBRARY_SEARCH_SYSTEM32 0x00000800
#endif
#endif

namespace AntWindowsSystemLibrary
{
QString moduleFilePath(ModuleHandle module)
{
#ifdef Q_OS_WIN
    if (!module)
    {
        return {};
    }

    wchar_t path[32768] = {};
    const DWORD length = ::GetModuleFileNameW(reinterpret_cast<HMODULE>(module), path,
                                               static_cast<DWORD>(std::size(path)));
    if (length == 0 || length >= std::size(path))
    {
        return {};
    }
    return QDir::cleanPath(QString::fromWCharArray(path, static_cast<int>(length)));
#else
    Q_UNUSED(module);
    return {};
#endif
}

QString system32Directory()
{
#ifdef Q_OS_WIN
    wchar_t path[32768] = {};
    const UINT length = ::GetSystemDirectoryW(path, static_cast<UINT>(std::size(path)));
    if (length == 0 || length >= std::size(path))
    {
        return {};
    }
    return QDir::cleanPath(QString::fromWCharArray(path, static_cast<int>(length)));
#else
    return {};
#endif
}

bool isLoadedFromSystem32(ModuleHandle module)
{
#ifdef Q_OS_WIN
    const QString loadedPath = moduleFilePath(module);
    const QString systemPath = system32Directory();
    if (loadedPath.isEmpty() || systemPath.isEmpty())
    {
        return false;
    }

    return QFileInfo(loadedPath).absolutePath().compare(systemPath, Qt::CaseInsensitive) == 0;
#else
    Q_UNUSED(module);
    return false;
#endif
}

ModuleHandle loadSystem32Library(const wchar_t* libraryName)
{
#ifdef Q_OS_WIN
    if (!libraryName || !*libraryName)
    {
        return nullptr;
    }

    const QString name = QString::fromWCharArray(libraryName);
    if (name.contains(QLatin1Char('/')) || name.contains(QLatin1Char('\\')) ||
        name.contains(QLatin1Char(':')))
    {
        return nullptr;
    }

    HMODULE module = ::LoadLibraryExW(libraryName, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module)
    {
        return nullptr;
    }
    if (!isLoadedFromSystem32(reinterpret_cast<ModuleHandle>(module)))
    {
        ::FreeLibrary(module);
        return nullptr;
    }
    return reinterpret_cast<ModuleHandle>(module);
#else
    Q_UNUSED(libraryName);
    return nullptr;
#endif
}
} // namespace AntWindowsSystemLibrary
