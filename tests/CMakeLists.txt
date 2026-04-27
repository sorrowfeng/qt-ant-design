set(CMAKE_AUTOMOC ON)

function(add_ant_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} PRIVATE qt-ant-design Qt6::Test)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
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
