cmake_minimum_required(VERSION 3.16)

project(qt-ant-design
    VERSION 0.1.0
    LANGUAGES CXX
)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(QT_ANT_DESIGN_HEADERS
    src/core/AntTypes.h
    src/core/AntTheme.h
    src/styles/AntPalette.h
    src/widgets/AntAvatar.h
    src/widgets/AntBadge.h
    src/widgets/AntBreadcrumb.h
    src/widgets/AntButton.h
    src/widgets/AntButtonStyle.h
    src/widgets/AntCheckbox.h
    src/widgets/AntDatePicker.h
    src/widgets/AntDivider.h
    src/widgets/AntInput.h
    src/widgets/AntMessage.h
    src/widgets/AntMenu.h
    src/widgets/AntNotification.h
    src/widgets/AntPagination.h
    src/widgets/AntProgress.h
    src/widgets/AntRadio.h
    src/widgets/AntSelect.h
    src/widgets/AntSlider.h
    src/widgets/AntSpin.h
    src/widgets/AntSwitch.h
    src/widgets/AntTabs.h
    src/widgets/AntTag.h
    src/widgets/AntTimePicker.h
    src/widgets/AntCard.h
)

set(QT_ANT_DESIGN_SOURCES
    src/core/AntTheme.cpp
    src/styles/AntPalette.cpp
    src/widgets/AntAvatar.cpp
    src/widgets/AntBadge.cpp
    src/widgets/AntBreadcrumb.cpp
    src/widgets/AntButton.cpp
    src/widgets/AntButtonStyle.cpp
    src/widgets/AntCheckbox.cpp
    src/widgets/AntDatePicker.cpp
    src/widgets/AntDivider.cpp
    src/widgets/AntInput.cpp
    src/widgets/AntMessage.cpp
    src/widgets/AntMenu.cpp
    src/widgets/AntNotification.cpp
    src/widgets/AntPagination.cpp
    src/widgets/AntProgress.cpp
    src/widgets/AntRadio.cpp
    src/widgets/AntSelect.cpp
    src/widgets/AntSlider.cpp
    src/widgets/AntSpin.cpp
    src/widgets/AntSwitch.cpp
    src/widgets/AntTabs.cpp
    src/widgets/AntTag.cpp
    src/widgets/AntTimePicker.cpp
    src/widgets/AntCard.cpp
    resources/qt-ant-design.qrc
)

add_library(qt-ant-design STATIC
    ${QT_ANT_DESIGN_HEADERS}
    ${QT_ANT_DESIGN_SOURCES}
)

target_include_directories(qt-ant-design
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qt-ant-design>
)

target_link_libraries(qt-ant-design
    PUBLIC
        Qt6::Core
        Qt6::Widgets
)

add_executable(qt-ant-design-example
    examples/main.cpp
    examples/ExampleWindow.h
    examples/ExampleWindow.cpp
)

target_link_libraries(qt-ant-design-example
    PRIVATE
        qt-ant-design
)

install(TARGETS qt-ant-design qt-ant-design-example
    EXPORT qt-ant-design-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
    src/core/AntTypes.h
    src/core/AntTheme.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qt-ant-design/core
)

install(FILES
    src/styles/AntPalette.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qt-ant-design/styles
)

install(FILES
    src/widgets/AntAvatar.h
    src/widgets/AntBadge.h
    src/widgets/AntBreadcrumb.h
    src/widgets/AntButton.h
    src/widgets/AntButtonStyle.h
    src/widgets/AntCheckbox.h
    src/widgets/AntDatePicker.h
    src/widgets/AntDivider.h
    src/widgets/AntInput.h
    src/widgets/AntMessage.h
    src/widgets/AntMenu.h
    src/widgets/AntNotification.h
    src/widgets/AntPagination.h
    src/widgets/AntProgress.h
    src/widgets/AntRadio.h
    src/widgets/AntSelect.h
    src/widgets/AntSlider.h
    src/widgets/AntSpin.h
    src/widgets/AntSwitch.h
    src/widgets/AntTabs.h
    src/widgets/AntTag.h
    src/widgets/AntTimePicker.h
    src/widgets/AntCard.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qt-ant-design/widgets
)

install(EXPORT qt-ant-design-targets
    NAMESPACE qt-ant-design::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qt-ant-design
)

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT_EXECUTABLE)
        install(CODE "
            execute_process(
                COMMAND \"${WINDEPLOYQT_EXECUTABLE}\" \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/qt-ant-design-example${CMAKE_EXECUTABLE_SUFFIX}\"
                RESULT_VARIABLE windeployqt_result
            )
            if(NOT windeployqt_result EQUAL 0)
                message(WARNING \"windeployqt failed with exit code \${windeployqt_result}\")
            endif()
        ")
    else()
        message(WARNING \"windeployqt was not found. Installed example may require Qt DLLs in PATH.\")
    endif()
endif()
