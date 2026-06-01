if(NOT DEFINED ANT_SOURCE_DIR)
    message(FATAL_ERROR "ANT_SOURCE_DIR is required")
endif()

set(ANT_SCAN_DIRS
    "${ANT_SOURCE_DIR}/src"
    "${ANT_SOURCE_DIR}/examples"
    "${ANT_SOURCE_DIR}/tests"
    "${ANT_SOURCE_DIR}/resources"
)

set(ANT_SOURCE_PATTERNS
    "*.h" "*.hpp" "*.hh" "*.hxx"
    "*.cpp" "*.cc" "*.cxx"
    "*.ui" "*.qrc"
)

set(ANT_FILES)
foreach(ANT_DIR IN LISTS ANT_SCAN_DIRS)
    if(EXISTS "${ANT_DIR}")
        foreach(ANT_PATTERN IN LISTS ANT_SOURCE_PATTERNS)
            file(GLOB_RECURSE ANT_MATCHES
                "${ANT_DIR}/${ANT_PATTERN}"
            )
            list(APPEND ANT_FILES ${ANT_MATCHES})
        endforeach()
    endif()
endforeach()

set(ANT_QSS_FILES)
foreach(ANT_DIR IN LISTS ANT_SCAN_DIRS)
    if(EXISTS "${ANT_DIR}")
        file(GLOB_RECURSE ANT_MATCHES "${ANT_DIR}/*.qss")
        list(APPEND ANT_QSS_FILES ${ANT_MATCHES})
    endif()
endforeach()

if(ANT_QSS_FILES)
    list(REMOVE_DUPLICATES ANT_QSS_FILES)
    list(SORT ANT_QSS_FILES)
    string(REPLACE ";" "\n  " ANT_QSS_LIST "${ANT_QSS_FILES}")
    message(FATAL_ERROR "QSS files are not allowed in qt-ant-design:\n  ${ANT_QSS_LIST}")
endif()

set(ANT_VIOLATIONS)
list(REMOVE_DUPLICATES ANT_FILES)
list(SORT ANT_FILES)
foreach(ANT_FILE IN LISTS ANT_FILES)
    file(READ "${ANT_FILE}" ANT_CONTENT)
    if(ANT_CONTENT MATCHES "setStyleSheet[ \t\r\n]*\\(" OR
       ANT_CONTENT MATCHES "QStyleSheet" OR
       ANT_CONTENT MATCHES "styleSheet[ \t\r\n]*:")
        file(RELATIVE_PATH ANT_RELATIVE "${ANT_SOURCE_DIR}" "${ANT_FILE}")
        list(APPEND ANT_VIOLATIONS "${ANT_RELATIVE}")
    endif()
endforeach()

if(ANT_VIOLATIONS)
    list(REMOVE_DUPLICATES ANT_VIOLATIONS)
    list(SORT ANT_VIOLATIONS)
    string(REPLACE ";" "\n  " ANT_VIOLATION_LIST "${ANT_VIOLATIONS}")
    message(FATAL_ERROR "QSS/QStyleSheet usage is not allowed in source, examples, tests, or resources:\n  ${ANT_VIOLATION_LIST}")
endif()

message(STATUS "No QSS/QStyleSheet usage found in source, examples, tests, or resources.")
