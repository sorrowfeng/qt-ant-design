#pragma once

#include <QString>

namespace AntWindowsSystemLibrary
{
using ModuleHandle = void*;

ModuleHandle loadSystem32Library(const wchar_t* libraryName);
QString moduleFilePath(ModuleHandle module);
QString system32Directory();
bool isLoadedFromSystem32(ModuleHandle module);
} // namespace AntWindowsSystemLibrary
