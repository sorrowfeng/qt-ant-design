set(CMAKE_AUTOMOC ON)

function(add_ant_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} PRIVATE qt-ant-design Qt${QT_VERSION_MAJOR}::Test)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

function(add_ant_inventory_test TEST_NAME)
    add_ant_test(${TEST_NAME})
    target_compile_definitions(${TEST_NAME} PRIVATE
        ANT_WIDGETS_DIR="${CMAKE_SOURCE_DIR}/src/widgets"
        ANT_TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )
endfunction()

# Core types
add_ant_test(TestAntTypes)

# General (4 widgets)
add_ant_test(TestAntButton)
add_ant_test(TestAntIcon)
add_ant_test(TestAntTypography)
add_ant_test(TestAntFloatButton)

# Data Entry A (6 widgets)
add_ant_test(TestAntCheckBox)
add_ant_test(TestAntSwitch)
add_ant_test(TestAntSelect)
add_ant_test(TestAntInput)
add_ant_test(TestAntDataEntryA)

# Data Entry B (7 widgets)
add_ant_test(TestAntDataEntryB)

# Data Display A (8 widgets)
add_ant_test(TestAntBadge)
add_ant_test(TestAntDataDisplayA)

# Data Display B (10 widgets)
add_ant_test(TestAntDataDisplayB)
add_ant_test(TestAntTag)

# Feedback (13 widgets)
add_ant_test(TestAntFeedback)
add_ant_test(TestAntModal)

# Navigation (7 widgets)
add_ant_test(TestAntNavigation)

# Layout (7 widgets)
add_ant_test(TestAntLayout)

# Qt Extensions (18 widgets)
add_ant_test(TestAntQtExtensions)

# Qt-style widget aliases
add_ant_test(TestAntAliases)

# Object tree ownership
add_ant_test(TestAntObjectTree)

# Assigned child ownership
add_ant_test(TestAntChildOwnership)

# Theme switching lifecycle
add_ant_test(TestAntThemeLifecycle)

# Meta-property coverage
add_ant_test(TestAntMetaProperties)

# Complex interaction coverage
add_ant_test(TestAntInteractions)
add_ant_test(TestAntAdvancedInteractions)

# Motion and animation coverage
add_ant_test(TestAntMotion)

# Popup lifecycle coverage
add_ant_test(TestAntPopupLifecycle)

# Repeated lifecycle stress coverage
add_ant_test(TestAntStressLifecycle)

# Render smoke coverage
add_ant_test(TestAntRenderSmoke)

# Automated visual regression coverage
add_ant_test(TestAntVisualRegression)
add_ant_inventory_test(TestAntQtVersionVisualParity)
foreach(ANT_VISUAL_PARITY_SCALE IN ITEMS 1.25 1.5)
    string(REPLACE "." "_" ANT_VISUAL_PARITY_SCALE_NAME "${ANT_VISUAL_PARITY_SCALE}")
    add_test(NAME TestAntQtVersionVisualParity_Scale_${ANT_VISUAL_PARITY_SCALE_NAME}
        COMMAND $<TARGET_FILE:TestAntQtVersionVisualParity>
    )
    set_tests_properties(TestAntQtVersionVisualParity_Scale_${ANT_VISUAL_PARITY_SCALE_NAME}
        PROPERTIES
            ENVIRONMENT "QT_SCALE_FACTOR=${ANT_VISUAL_PARITY_SCALE};QT_ENABLE_HIGHDPI_SCALING=1"
            TIMEOUT 45
    )
endforeach()
add_ant_test(TestAntQtVersionMetricAudit)

# Windows / Qt high-DPI logical geometry coverage
add_executable(TestAntHighDpiScaling TestAntHighDpiScaling.cpp)
target_link_libraries(TestAntHighDpiScaling PRIVATE qt-ant-design Qt${QT_VERSION_MAJOR}::Test)
foreach(ANT_HIGH_DPI_SCALE IN ITEMS 1.0 1.25 1.5)
    string(REPLACE "." "_" ANT_HIGH_DPI_SCALE_NAME "${ANT_HIGH_DPI_SCALE}")
    add_test(NAME TestAntHighDpiScaling_${ANT_HIGH_DPI_SCALE_NAME}
        COMMAND $<TARGET_FILE:TestAntHighDpiScaling>
    )
    set_tests_properties(TestAntHighDpiScaling_${ANT_HIGH_DPI_SCALE_NAME}
        PROPERTIES ENVIRONMENT "QT_SCALE_FACTOR=${ANT_HIGH_DPI_SCALE};QT_ENABLE_HIGHDPI_SCALING=1"
    )
endforeach()
add_test(NAME TestAntHighDpiScaling_InitializeDefault
    COMMAND $<TARGET_FILE:TestAntHighDpiScaling>
)
set_tests_properties(TestAntHighDpiScaling_InitializeDefault
    PROPERTIES ENVIRONMENT "ANT_TEST_INITIALIZE_CONFIGURES_HIGHDPI=1;QT_SCALE_FACTOR=1.25;QT_ENABLE_HIGHDPI_SCALING=1"
)

# Installed package consumer coverage
add_test(NAME TestAntInstallConsumer
    COMMAND ${CMAKE_COMMAND}
        -DANT_BINARY_DIR=${CMAKE_BINARY_DIR}
        -DANT_TEST_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DANT_CONFIG=$<CONFIG>
        -DANT_QT_PACKAGE=${QT_ANT_DESIGN_QT_PACKAGE}
        -DANT_QT_PACKAGE_DIR=${${QT_ANT_DESIGN_QT_PACKAGE}_DIR}
        -DANT_CMAKE_GENERATOR=${CMAKE_GENERATOR}
        -DANT_CMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}
        -DANT_CMAKE_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}
        -DANT_CMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/TestAntInstallConsumer.cmake
)

# Build-system option coverage
add_test(NAME TestAntBuildSystem
    COMMAND ${CMAKE_COMMAND}
        -DANT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/TestAntBuildSystem.cmake
)

# No-QSS / no-QStyleSheet styling guard
add_test(NAME TestAntNoStyleSheetUsage
    COMMAND ${CMAKE_COMMAND}
        -DANT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/TestAntNoStyleSheetUsage.cmake
)

if(TARGET qt-ant-design-example)
    if(WIN32)
        add_test(NAME TestAntExampleGuiSubsystem
            COMMAND ${CMAKE_COMMAND}
                -DEXAMPLE_EXE=$<TARGET_FILE:qt-ant-design-example>
                -P ${CMAKE_CURRENT_SOURCE_DIR}/TestAntExampleGuiSubsystem.cmake
        )
    endif()

    add_test(NAME TestAntExampleCloseStress
        COMMAND $<TARGET_FILE:qt-ant-design-example>
            --stress-theme-cycles 16
            --stress-theme-interval-ms 20
            --smoke-exit-ms 800
    )
    set_tests_properties(TestAntExampleCloseStress PROPERTIES TIMEOUT 10)

    add_test(NAME TestAntExamplePageTraversal
        COMMAND $<TARGET_FILE:qt-ant-design-example>
            --smoke-traverse-pages
            --smoke-traverse-step-ms 1
    )
    set_tests_properties(TestAntExamplePageTraversal PROPERTIES TIMEOUT 45)
endif()

# Coverage inventory guard
add_ant_inventory_test(TestAntCoverageInventory)
