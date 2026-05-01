set(CMAKE_AUTOMOC ON)

function(add_ant_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} PRIVATE qt-ant-design Qt6::Test)
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
add_ant_test(TestAntCheckbox)
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

# Installed package consumer coverage
add_test(NAME TestAntInstallConsumer
    COMMAND ${CMAKE_COMMAND}
        -DANT_BINARY_DIR=${CMAKE_BINARY_DIR}
        -DANT_TEST_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DANT_CONFIG=$<CONFIG>
        -DANT_QT6_DIR=${Qt6_DIR}
        -DANT_CMAKE_GENERATOR=${CMAKE_GENERATOR}
        -DANT_CMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}
        -DANT_CMAKE_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}
        -DANT_CMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/TestAntInstallConsumer.cmake
)

if(TARGET qt-ant-design-example)
    add_test(NAME TestAntExampleCloseStress
        COMMAND $<TARGET_FILE:qt-ant-design-example>
            --stress-theme-cycles 16
            --stress-theme-interval-ms 20
            --smoke-exit-ms 800
    )
    set_tests_properties(TestAntExampleCloseStress PROPERTIES TIMEOUT 10)
endif()

# Coverage inventory guard
add_ant_inventory_test(TestAntCoverageInventory)
