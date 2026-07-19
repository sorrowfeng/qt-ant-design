if(NOT DEFINED ANT_SOURCE_DIR)
    message(FATAL_ERROR "ANT_SOURCE_DIR is required")
endif()

set(workflow_path "${ANT_SOURCE_DIR}/.github/workflows/ci.yml")
set(dependabot_path "${ANT_SOURCE_DIR}/.github/dependabot.yml")
if(NOT EXISTS "${workflow_path}" OR NOT EXISTS "${dependabot_path}")
    message(FATAL_ERROR "CI workflow and Dependabot configuration are required")
endif()

file(READ "${workflow_path}" workflow)
file(READ "${dependabot_path}" dependabot)
file(STRINGS "${workflow_path}" action_lines REGEX "^[ \\t-]*uses:")
if(NOT action_lines)
    message(FATAL_ERROR "CI workflow does not use any actions")
endif()
foreach(action_line IN LISTS action_lines)
    string(REGEX REPLACE ".*uses:[ \\t]*[^@]+@([^ #]+).*" "\\1" action_ref "${action_line}")
    string(LENGTH "${action_ref}" action_ref_length)
    if(NOT action_ref MATCHES "^[0-9a-f]+$" OR NOT action_ref_length EQUAL 40)
        message(FATAL_ERROR "GitHub Action is not pinned to a full commit SHA: ${action_line}")
    endif()
endforeach()

if(NOT workflow MATCHES "if:[ \t]+matrix\\.qt\\.asan == 'OFF' && matrix\\.qt\\.ubsan == 'OFF'")
    message(FATAL_ERROR "Sanitizer-only static builds must not be installed as consumer packages")
endif()

if(NOT workflow MATCHES "-E [^\n]*TestAntInstallConsumer")
    message(FATAL_ERROR "Windows targeted test pass must not rerun the installed-consumer test")
endif()

foreach(required_pattern IN ITEMS
        "permissions:"
        "contents:[ \\t]+read"
        "persist-credentials:[ \\t]+false"
        "QT_ANT_DESIGN_BUILD_TESTS=ON"
        "QT_ANT_DESIGN_ENABLE_ADDRESS_SANITIZER"
        "QT_ANT_DESIGN_ENABLE_UNDEFINED_SANITIZER"
        "TestAntInstallConsumer"
        "Run installed example smoke"
        "runs-on: ubuntu-22.04"
        "runs-on: macos-14"
        "BUILD_SHARED_LIBS")
    if(NOT workflow MATCHES "${required_pattern}")
        message(FATAL_ERROR "CI workflow is missing required policy: ${required_pattern}")
    endif()
endforeach()

if(NOT dependabot MATCHES "package-ecosystem:[ \\t]+github-actions")
    message(FATAL_ERROR "Dependabot must manage pinned GitHub Actions")
endif()
