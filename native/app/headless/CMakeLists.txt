cmake_minimum_required(VERSION 3.25)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(MacioGalaxy2HeadlessComponents LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    include(CTest)
endif()
get_filename_component(_headless_root "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
find_package(Threads REQUIRED)
find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter)
add_library(petari_headless_core STATIC replay.cpp trace.cpp fp_mode.cpp stage_sync.cpp actor_trace.cpp)
target_compile_features(petari_headless_core PUBLIC cxx_std_17)
target_compile_options(petari_headless_core PUBLIC -ffp-contract=off)
target_include_directories(petari_headless_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${_headless_root}/native/include")
if(PETARI_SANITIZERS)
    target_compile_options(petari_headless_core PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(petari_headless_core PUBLIC -fsanitize=address,undefined)
endif()
add_custom_target(petari_headless_components DEPENDS petari_headless_core)
if(BUILD_TESTING)
    add_executable(petari_headless_unit_tests "${_headless_root}/native/tests/headless_tests.cpp")
    target_link_libraries(petari_headless_unit_tests PRIVATE petari_headless_core Threads::Threads)
    add_test(NAME native_headless_core COMMAND petari_headless_unit_tests)
    add_test(NAME native_headless_tools COMMAND "${Python3_EXECUTABLE}" "${_headless_root}/native/tests/headless_tools_tests.py")
    add_test(NAME native_headless_trap_generator COMMAND "${Python3_EXECUTABLE}" "${_headless_root}/native/tests/trap_undefined_tests.py")
    add_executable(petari_headless_sync_tests "${_headless_root}/native/tests/headless_sync_tests.cpp")
    target_link_libraries(petari_headless_sync_tests PRIVATE petari_headless_core)
    add_test(NAME native_headless_sync COMMAND petari_headless_sync_tests)
    add_executable(petari_headless_actor_trace_tests "${_headless_root}/native/tests/headless_actor_trace_tests.cpp")
    target_link_libraries(petari_headless_actor_trace_tests PRIVATE petari_headless_core)
    add_test(NAME native_headless_actor_trace COMMAND petari_headless_actor_trace_tests)
    add_test(NAME native_headless_actor_receipt COMMAND "${Python3_EXECUTABLE}" "${_headless_root}/native/tests/actor_receipt_tests.py")
    add_dependencies(petari_headless_components petari_headless_unit_tests petari_headless_sync_tests petari_headless_actor_trace_tests)
endif()
if(NOT PETARI_BUILD_HEADLESS)
    return()
endif()
if(NOT TARGET petari_native_config)
    message(FATAL_ERROR "The game harness requires the repository root configuration with PETARI_BUILD_HEADLESS=ON")
endif()
option(GALAXY_NATIVE_TRAP_UNDEFINED "Diagnostic abort functions for unresolved headless function symbols" OFF)
set(GALAXY_NATIVE_TRAP_INVENTORY "" CACHE FILEPATH "Fresh partial-link undefined-symbol inventory JSON")
set(GALAXY_NATIVE_TRAP_INPUTS "" CACHE FILEPATH "Available-object partial-link inputs JSON from the same build")
set(GALAXY_NATIVE_TRAP_MODE "ABORT" CACHE STRING "Diagnostic function policy: ABORT or COLLECT")
set_property(CACHE GALAXY_NATIVE_TRAP_MODE PROPERTY STRINGS ABORT COLLECT)
add_library(petari_headless_sdk STATIC kpad_decode.cpp)
target_link_libraries(petari_headless_sdk PUBLIC petari_headless_core petari_native_config)
if(BUILD_TESTING)
    add_executable(petari_headless_sdk_tests "${_headless_root}/native/tests/headless_sdk_tests.cpp")
    target_link_libraries(petari_headless_sdk_tests PRIVATE petari_headless_sdk)
    add_test(NAME native_headless_sdk COMMAND petari_headless_sdk_tests)
    add_executable(petari_headless_clock_tests "${_headless_root}/native/tests/headless_clock_tests.cpp")
    target_link_libraries(petari_headless_clock_tests PRIVATE petari_headless_core petari_platform_vi)
    add_test(NAME native_headless_clock COMMAND petari_headless_clock_tests)
    add_dependencies(petari_headless_components petari_headless_sdk_tests petari_headless_clock_tests)
endif()
file(GLOB _gx_headers CONFIGURE_DEPENDS "${_headless_root}/libs/RVL_SDK/include/revolution/gx/*.h")
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gx_noops.inc"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/gen_gx_noops.py"
        "${_headless_root}/libs/RVL_SDK/include/revolution/gx" "${CMAKE_CURRENT_BINARY_DIR}/gx_noops.inc"
        --manual "${CMAKE_CURRENT_SOURCE_DIR}/gx_null.cpp"
        --manual "${_headless_root}/native/platform/gx_sync/gx_sync.cpp"
    DEPENDS gen_gx_noops.py gx_null.cpp "${_headless_root}/native/platform/gx_sync/gx_sync.cpp" ${_gx_headers}
    VERBATIM)
add_library(petari_gx_null STATIC gx_null.cpp "${_headless_root}/native/gx/sync_backend.cpp" "${CMAKE_CURRENT_BINARY_DIR}/gx_noops.inc")
target_include_directories(petari_gx_null PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(petari_gx_null PUBLIC petari_native_config petari_headless_core petari_platform_gx_sync)
file(GLOB _gd_sources CONFIGURE_DEPENDS "${_headless_root}/src/RVL_SDK/gd/*.c")
add_library(petari_headless_gd STATIC ${_gd_sources})
petari_encode_target_sources(petari_headless_gd)
foreach(_source IN LISTS _gd_sources)
    file(RELATIVE_PATH _relative "${_headless_root}" "${_source}")
    set_source_files_properties("${PETARI_LEGACY_ROOT}/${_relative}" PROPERTIES LANGUAGE CXX)
endforeach()
target_link_libraries(petari_headless_gd PUBLIC petari_native_config petari_platform_os)
file(GLOB _j3d_sources CONFIGURE_DEPENDS "${_headless_root}/src/JSystem/J3DGraphBase/*.cpp"
    "${_headless_root}/src/JSystem/J3DGraphAnimator/*.cpp" "${_headless_root}/src/JSystem/J3DGraphLoader/*.cpp")
add_library(petari_headless_j3d STATIC ${_j3d_sources} "${_headless_root}/native/gx/model_overrides.cpp"
    "${_headless_root}/src/JSystem/JUtility/JUTNameTab.cpp" "${_headless_root}/src/JSystem/JUtility/JUTAssert.cpp")
if(EXISTS "${_headless_root}/src/JSystem/JUtility/JUTNativeResource.cpp")
    target_sources(petari_headless_j3d PRIVATE "${_headless_root}/src/JSystem/JUtility/JUTNativeResource.cpp")
endif()
petari_encode_target_sources(petari_headless_j3d)
target_compile_options(petari_headless_j3d PRIVATE -fno-rtti)
target_link_libraries(petari_headless_j3d PUBLIC petari_kernel petari_core petari_resources petari_headless_gd petari_gx_null)
file(GLOB_RECURSE _libraries CONFIGURE_DEPENDS "${_headless_root}/src/JSystem/*.cpp" "${_headless_root}/src/nw4r/*.cpp")
list(FILTER _libraries EXCLUDE REGEX "/JSystem/(JKernel|JMath|J3DGraphBase|J3DGraphAnimator|J3DGraphLoader)/")
list(FILTER _libraries EXCLUDE REGEX "/JASSeqReader\\.cpp$")
list(FILTER _libraries EXCLUDE REGEX "/JSU(List|InputStream|FileStream)\\.cpp$")
list(FILTER _libraries EXCLUDE REGEX "/JUT(NameTab|NativeResource|Assert)\\.cpp$")
file(GLOB _libms CONFIGURE_DEPENDS "${_headless_root}/src/libms/*.c")
add_library(petari_headless_game_libraries STATIC ${_libraries} ${_libms}
    "${_headless_root}/native/gx/particle_overrides.cpp" "${_headless_root}/src/RVL_SDK/wenc/wenc.c")
petari_encode_target_sources(petari_headless_game_libraries)
foreach(_source IN LISTS _libms)
    file(RELATIVE_PATH _relative "${_headless_root}" "${_source}")
    set_source_files_properties("${PETARI_LEGACY_ROOT}/${_relative}" PROPERTIES LANGUAGE CXX)
endforeach()
set_source_files_properties("${PETARI_LEGACY_ROOT}/src/RVL_SDK/wenc/wenc.c" PROPERTIES LANGUAGE CXX)
target_compile_options(petari_headless_game_libraries PRIVATE -fno-rtti)
target_link_libraries(petari_headless_game_libraries PUBLIC petari_headless_j3d petari_input petari_platform)
add_library(petari_headless_platform STATIC platform.cpp)
target_link_libraries(petari_headless_platform PUBLIC petari_headless_sdk petari_boot petari_platform petari_input)
if(GALAXY_NATIVE_TRAP_UNDEFINED)
    set(_trap_mode_args)
    if(GALAXY_NATIVE_TRAP_MODE STREQUAL "COLLECT")
        list(APPEND _trap_mode_args --collect-functions)
    elseif(NOT GALAXY_NATIVE_TRAP_MODE STREQUAL "ABORT")
        message(FATAL_ERROR "GALAXY_NATIVE_TRAP_MODE must be ABORT or COLLECT")
    endif()
    foreach(_manifest IN ITEMS GALAXY_NATIVE_TRAP_INVENTORY GALAXY_NATIVE_TRAP_INPUTS)
        if(NOT EXISTS "${${_manifest}}")
            message(FATAL_ERROR "${_manifest} must name a fresh existing diagnostic manifest; normal compilation must be attempted first")
        endif()
    endforeach()
    set(_trap_source "${CMAKE_CURRENT_BINARY_DIR}/undefined_traps.cpp")
    set(_trap_report "${CMAKE_CURRENT_BINARY_DIR}/undefined_traps.json")
    set(_trap_tool "${_headless_root}/native/tools/trap_undefined.py")
    add_custom_command(OUTPUT "${_trap_source}" "${_trap_report}"
        COMMAND "${Python3_EXECUTABLE}" "${_trap_tool}" generate
            --inventory "${GALAXY_NATIVE_TRAP_INVENTORY}" --inputs "${GALAXY_NATIVE_TRAP_INPUTS}"
            --symbols "${_headless_root}/config/SB4E01/symbols.txt"
            --extra-kinds "${CMAKE_CURRENT_SOURCE_DIR}/trap_symbol_kinds.json"
            --guard-data
            ${_trap_mode_args}
            --output "${_trap_source}" --report "${_trap_report}"
        DEPENDS "${_trap_tool}" "${GALAXY_NATIVE_TRAP_INVENTORY}" "${GALAXY_NATIVE_TRAP_INPUTS}"
            "${_headless_root}/config/SB4E01/symbols.txt"
            "${CMAKE_CURRENT_SOURCE_DIR}/trap_symbol_kinds.json"
            "${CMAKE_CURRENT_SOURCE_DIR}/trap_guard_runtime.inc"
        VERBATIM)
    add_library(petari_headless_traps OBJECT "${_trap_source}")
    target_compile_definitions(petari_headless_traps PRIVATE GALAXY_NATIVE=1)
    target_compile_options(petari_headless_traps PRIVATE -fno-sanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off)
    set(_trap_binary "${CMAKE_CURRENT_BINARY_DIR}/petari_headless")
    add_custom_command(OUTPUT "${_trap_binary}"
        COMMAND "${Python3_EXECUTABLE}" "${_trap_tool}" link
            --inputs "${GALAXY_NATIVE_TRAP_INPUTS}" --object "$<TARGET_OBJECTS:petari_headless_traps>"
            --output "${_trap_binary}" --log "${CMAKE_CURRENT_BINARY_DIR}/trap-link.log"
            --status "${CMAKE_CURRENT_BINARY_DIR}/trap-link-status.json"
        DEPENDS petari_headless_traps $<TARGET_OBJECTS:petari_headless_traps> "${_trap_tool}" "${GALAXY_NATIVE_TRAP_INPUTS}"
        COMMAND_EXPAND_LISTS VERBATIM)
    add_custom_target(petari_headless DEPENDS "${_trap_binary}")
    message(WARNING "PLACEHOLDER(port): diagnostic headless link uses available objects; unresolved data/vtables are PROT_NONE guarded ranges, never usable values")
    return()
endif()
add_executable(petari_headless EXCLUDE_FROM_ALL main.cpp observe.cpp "${_headless_root}/native/app/milestone.cpp"
    "${_headless_root}/native/app/actor_observe.cpp" "${_headless_root}/native/app/play_ready.cpp")
target_compile_definitions(petari_headless PRIVATE PETARI_HEADLESS_SOURCE_ROOT="${_headless_root}")
# Game classes are compiled without RTTI (petari_game_objects): TUs that emit vtables for them must not need their typeinfo.
target_compile_options(petari_headless PRIVATE -fno-rtti)
target_link_libraries(petari_headless PRIVATE petari_headless_platform petari_game_objects petari_headless_game_libraries
    petari_headless_core petari_gx_null petari_home_menu petari_sdk_mem petari_rfl Threads::Threads)
target_link_options(petari_headless PRIVATE -Wl,-dead_strip)
add_custom_command(TARGET petari_headless POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E rm -f "$<TARGET_FILE:petari_headless>.diagnostic.json" VERBATIM)
