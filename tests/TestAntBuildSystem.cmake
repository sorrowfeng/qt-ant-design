if(NOT DEFINED ANT_SOURCE_DIR)
    message(FATAL_ERROR "ANT_SOURCE_DIR is required")
endif()

set(files_to_scan
    "${ANT_SOURCE_DIR}/CMakeLists.txt"
    "${ANT_SOURCE_DIR}/src/CMakeLists.txt"
    "${ANT_SOURCE_DIR}/tests/CMakeLists.txt"
    "${ANT_SOURCE_DIR}/cmake/qt-ant-designConfig.cmake.in"
)

foreach(file_path IN LISTS files_to_scan)
    if(NOT EXISTS "${file_path}")
        message(FATAL_ERROR "Missing expected build file: ${file_path}")
    endif()
endforeach()

file(READ "${ANT_SOURCE_DIR}/CMakeLists.txt" root_cmake)
file(READ "${ANT_SOURCE_DIR}/src/CMakeLists.txt" src_cmake)
file(READ "${ANT_SOURCE_DIR}/tests/CMakeLists.txt" tests_cmake)
file(READ "${ANT_SOURCE_DIR}/cmake/qt-ant-designConfig.cmake.in" package_config)

if(src_cmake MATCHES "add_library[ \t\r\n]*\\([ \t\r\n]*qt-ant-design[ \t\r\n]+STATIC")
    message(FATAL_ERROR "qt-ant-design must not hard-code STATIC; use BUILD_SHARED_LIBS / library type options")
endif()

foreach(content IN ITEMS "${root_cmake}" "${src_cmake}" "${tests_cmake}" "${package_config}")
    if(content MATCHES "find_package[ \t\r\n]*\\([ \t\r\n]*Qt6" OR content MATCHES "Qt6::" OR content MATCHES "Qt6_DIR")
        message(FATAL_ERROR "Build files must use detected Qt major version instead of hard-coded Qt6 references")
    endif()
endforeach()

if(NOT root_cmake MATCHES "find_package[ \t\r\n]*\\([ \t\r\n]*QT[ \t\r\n]+NAMES[ \t\r\n]+Qt6[ \t\r\n]+Qt5")
    message(FATAL_ERROR "Top-level CMake must detect Qt with find_package(QT NAMES Qt6 Qt5 ...)")
endif()

if(NOT package_config MATCHES "find_dependency[ \t\r\n]*\\([ \t\r\n]*QT[ \t\r\n]+NAMES")
    message(FATAL_ERROR "Installed package config must detect Qt with find_dependency(QT NAMES Qt6 Qt5 ...)")
endif()

if(src_cmake MATCHES "WINDOWS_EXPORT_ALL_SYMBOLS")
    message(FATAL_ERROR "Shared Windows builds must use explicit export macros instead of auto-exporting every symbol")
endif()

if(NOT src_cmake MATCHES "QT_ANT_DESIGN_LIBRARY" OR NOT src_cmake MATCHES "QT_ANT_DESIGN_STATIC_DEFINE")
    message(FATAL_ERROR "Static/shared builds must define the qt-ant-design export macro state")
endif()

if(NOT EXISTS "${ANT_SOURCE_DIR}/src/core/QtAntDesignExport.h")
    message(FATAL_ERROR "Missing public export header for shared-library consumers")
endif()
