include_guard(GLOBAL)
find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter)
get_filename_component(_petari_legacy_source_root "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(PETARI_LEGACY_ROOT "${CMAKE_BINARY_DIR}/legacy-sjis")
set(_petari_encoding_script "${_petari_legacy_source_root}/native/tools/encode_legacy_sources.py")
execute_process(
    COMMAND "${Python3_EXECUTABLE}" "${_petari_encoding_script}"
        --root "${_petari_legacy_source_root}" --output "${PETARI_LEGACY_ROOT}"
    RESULT_VARIABLE _petari_encoding_result)
if(NOT _petari_encoding_result EQUAL 0)
    message(FATAL_ERROR "Could not prepare Shift-JIS legacy sources")
endif()
set(_petari_legacy_patterns)
foreach(_directory src include libs)
    foreach(_extension c cc cpp cxx h hh hpp inc)
        list(APPEND _petari_legacy_patterns "${_petari_legacy_source_root}/${_directory}/*.${_extension}")
    endforeach()
endforeach()
file(GLOB_RECURSE _petari_legacy_inputs CONFIGURE_DEPENDS ${_petari_legacy_patterns})
set(_petari_legacy_outputs)
foreach(_input IN LISTS _petari_legacy_inputs)
    file(RELATIVE_PATH _relative "${_petari_legacy_source_root}" "${_input}")
    set(_output "${PETARI_LEGACY_ROOT}/${_relative}")
    # File-level producer edges let Ninja dirty consumers before encoding runs.
    # Touch also advances unchanged output after a touch-only input edit (Make).
    add_custom_command(OUTPUT "${_output}"
        COMMAND "${Python3_EXECUTABLE}" "${_petari_encoding_script}"
            --root "${_petari_legacy_source_root}" --output "${PETARI_LEGACY_ROOT}" --file "${_relative}"
        COMMAND "${CMAKE_COMMAND}" -E touch "${_output}"
        DEPENDS "${_input}" "${_petari_encoding_script}"
        COMMENT "Encoding native legacy source ${_relative}"
        VERBATIM)
    list(APPEND _petari_legacy_outputs "${_output}")
endforeach()
add_custom_target(petari_legacy_sources DEPENDS ${_petari_legacy_outputs})

function(petari_encode_target_sources target)
    get_target_property(_sources "${target}" SOURCES)
    set(_encoded)
    foreach(_source IN LISTS _sources)
        if(IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _relative "${_petari_legacy_source_root}" "${_source}")
        else()
            set(_relative "${_source}")
        endif()
        if(_relative MATCHES "^src/")
            list(APPEND _encoded "${PETARI_LEGACY_ROOT}/${_relative}")
        else()
            list(APPEND _encoded "${_source}")
        endif()
    endforeach()
    set_property(TARGET "${target}" PROPERTY SOURCES "${_encoded}")
    add_dependencies("${target}" petari_legacy_sources)
endfunction()
