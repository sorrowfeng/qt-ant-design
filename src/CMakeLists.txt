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
    ${QT_ANT_DESIGN_VERSION_HEADER}
)

get_target_property(QT_ANT_DESIGN_TARGET_TYPE qt-ant-design TYPE)
if(QT_ANT_DESIGN_TARGET_TYPE STREQUAL "STATIC_LIBRARY")
    target_compile_definitions(qt-ant-design PUBLIC QT_ANT_DESIGN_STATIC_DEFINE)
    if(MSVC)
        if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.25")
            set_property(TARGET qt-ant-design PROPERTY
                MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>"
            )
        else()
            target_compile_options(qt-ant-design PRIVATE
                "$<$<CONFIG:Debug,RelWithDebInfo>:/Z7>"
            )
        endif()
    endif()
else()
    target_compile_definitions(qt-ant-design PRIVATE QT_ANT_DESIGN_LIBRARY)
endif()

target_include_directories(qt-ant-design
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${QT_ANT_DESIGN_GENERATED_INCLUDE_DIR}>
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
    PATTERN "*_p.h" EXCLUDE
    PATTERN "AntSelectPopup.h" EXCLUDE
)

install(FILES ${QT_ANT_DESIGN_VERSION_HEADER}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qt-ant-design/core
)
