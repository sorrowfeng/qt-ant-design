if(NOT DEFINED EXAMPLE_EXE OR EXAMPLE_EXE STREQUAL "")
    message(FATAL_ERROR "EXAMPLE_EXE is required")
endif()

if(NOT EXISTS "${EXAMPLE_EXE}")
    message(FATAL_ERROR "Example executable not found: ${EXAMPLE_EXE}")
endif()

file(READ "${EXAMPLE_EXE}" EXAMPLE_HEX HEX)

function(read_hex_byte OFFSET OUT_VAR)
    math(EXPR POS "${OFFSET} * 2")
    string(SUBSTRING "${EXAMPLE_HEX}" ${POS} 2 BYTE_HEX)
    math(EXPR VALUE "0x${BYTE_HEX}")
    set(${OUT_VAR} ${VALUE} PARENT_SCOPE)
endfunction()

function(read_u16_le OFFSET OUT_VAR)
    read_hex_byte(${OFFSET} B0)
    math(EXPR OFFSET_1 "${OFFSET} + 1")
    read_hex_byte(${OFFSET_1} B1)
    math(EXPR VALUE "${B0} + (${B1} << 8)")
    set(${OUT_VAR} ${VALUE} PARENT_SCOPE)
endfunction()

function(read_u32_le OFFSET OUT_VAR)
    read_hex_byte(${OFFSET} B0)
    math(EXPR OFFSET_1 "${OFFSET} + 1")
    math(EXPR OFFSET_2 "${OFFSET} + 2")
    math(EXPR OFFSET_3 "${OFFSET} + 3")
    read_hex_byte(${OFFSET_1} B1)
    read_hex_byte(${OFFSET_2} B2)
    read_hex_byte(${OFFSET_3} B3)
    math(EXPR VALUE "${B0} + (${B1} << 8) + (${B2} << 16) + (${B3} << 24)")
    set(${OUT_VAR} ${VALUE} PARENT_SCOPE)
endfunction()

read_u32_le(60 PE_HEADER_OFFSET)
math(EXPR OPTIONAL_HEADER_OFFSET "${PE_HEADER_OFFSET} + 24")
math(EXPR SUBSYSTEM_OFFSET "${OPTIONAL_HEADER_OFFSET} + 68")
read_u16_le(${SUBSYSTEM_OFFSET} SUBSYSTEM)

if(NOT SUBSYSTEM EQUAL 2)
    message(FATAL_ERROR
        "qt-ant-design-example must use the Windows GUI subsystem to avoid a console window; got subsystem ${SUBSYSTEM}"
    )
endif()

message(STATUS "qt-ant-design-example uses the Windows GUI subsystem")
