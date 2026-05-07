file(GLOB_RECURSE QT_ANT_DESIGN_HEADERS CONFIGURE_DEPENDS "*.h")
file(GLOB_RECURSE QT_ANT_DESIGN_SOURCES CONFIGURE_DEPENDS "*.cpp")

# Add Qt resource file
set(QT_ANT_DESIGN_RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/qt-ant-design.qrc
)

add_library(qt-ant-design
    ${QT_ANT_DESIGN_HEADERS}
    ${QT_ANT_DESIGN_SOURCES}
    ${QT_ANT_DESIGN_RESOURCES}
)

get_target_property(QT_ANT_DESIGN_TARGET_TYPE qt-ant-design TYPE)
if(QT_ANT_DESIGN_TARGET_TYPE STREQUAL "STATIC_LIBRARY")
    target_compile_definitions(qt-ant-design PUBLIC QT_ANT_DESIGN_STATIC_DEFINE)
else()
    target_compile_definitions(qt-ant-design PRIVATE QT_ANT_DESIGN_LIBRARY)
endif()

target_include_directories(qt-ant-design
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qt-ant-design>
)

target_link_libraries(qt-ant-design
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Svg
)

# Install headers preserving directory structure
install(DIRECTORY core styles widgets
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qt-ant-design
    FILES_MATCHING PATTERN "*.h"
)
