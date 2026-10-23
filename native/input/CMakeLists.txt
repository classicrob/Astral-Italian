# Native keyboard and mouse input behind the SDK's WPAD/KPAD API.
#
# Root integration:   add_subdirectory(native/input)   (after native/platform)
# Standalone build:   cmake -S native/input -B build/input -DPETARI_SANITIZERS=ON
#                     cmake --build build/input && ctest --test-dir build/input
cmake_minimum_required(VERSION 3.25)

set(_petari_input_standalone OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(_petari_input_standalone ON)
    project(PetariNativeInput LANGUAGES C CXX)
    if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "Target architecture" FORCE)
    endif()
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    option(PETARI_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
    include(CTest)
endif()

get_filename_component(PETARI_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

if(_petari_input_standalone)
    # The root build's shared targets, recreated for a standalone build.
    include("${PETARI_REPO_ROOT}/native/cmake/LegacySources.cmake")

    add_library(petari_native_config INTERFACE)
    target_compile_definitions(petari_native_config INTERFACE GALAXY_NATIVE=1)
    target_include_directories(petari_native_config INTERFACE
        "${PETARI_LEGACY_ROOT}/include" "${PETARI_LEGACY_ROOT}/libs/RVL_SDK/include"
        "${PETARI_LEGACY_ROOT}/libs/JSystem/include" "${PETARI_LEGACY_ROOT}/libs/nw4r/include"
        ${PETARI_REPO_ROOT}/native/include ${PETARI_REPO_ROOT}/include
        ${PETARI_REPO_ROOT}/libs/RVL_SDK/include ${PETARI_REPO_ROOT}/libs/JSystem/include
        ${PETARI_REPO_ROOT}/libs/nw4r/include)
    target_compile_options(petari_native_config INTERFACE
        -fno-strict-aliasing -ffp-contract=off -fcheck-new -Werror=return-type -Wno-register)
    if(PETARI_SANITIZERS)
        target_compile_options(petari_native_config INTERFACE
            -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(petari_native_config INTERFACE -fsanitize=address,undefined)
    endif()

    add_library(petari_host_runtime STATIC
        ${PETARI_REPO_ROOT}/native/src/host_allocation.cpp ${PETARI_REPO_ROOT}/native/text/utf16.cpp)
    target_link_libraries(petari_host_runtime PUBLIC petari_native_config)
    add_library(petari_memory STATIC
        ${PETARI_REPO_ROOT}/native/memory/address_api.cpp ${PETARI_REPO_ROOT}/native/memory/arena.cpp
        ${PETARI_REPO_ROOT}/src/RVL_SDK/os/OSAlloc.c)
    set_source_files_properties(${PETARI_REPO_ROOT}/src/RVL_SDK/os/OSAlloc.c PROPERTIES LANGUAGE CXX)
    target_link_libraries(petari_memory PUBLIC petari_native_config petari_host_runtime)
    add_library(petari_core STATIC
        ${PETARI_REPO_ROOT}/native/src/mtx.cpp ${PETARI_REPO_ROOT}/native/src/intrinsics.cpp
        ${PETARI_REPO_ROOT}/src/JSystem/JMath/JMath.cpp ${PETARI_REPO_ROOT}/src/JSystem/JMath/JMATrigonometric.cpp)
    petari_encode_target_sources(petari_core)
    target_link_libraries(petari_core PUBLIC petari_native_config)
    set(_petari_testing "${BUILD_TESTING}")
    set(BUILD_TESTING OFF)
    add_subdirectory(${PETARI_REPO_ROOT}/native/platform ${CMAKE_BINARY_DIR}/platform)
    set(BUILD_TESTING "${_petari_testing}")
endif()

# The SDK's KPAD.c, compiled unchanged. It relies on memcpy and memset
# without including a header, which CodeWarrior's prefix file provided.
set(PETARI_KPAD_SOURCE ${PETARI_REPO_ROOT}/src/RVL_SDK/kpad/KPAD.c)
if(_petari_input_standalone AND BUILD_TESTING)
    set(PETARI_KPAD_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/kpad_test_support.cpp)
    set_source_files_properties(${PETARI_KPAD_SOURCE} PROPERTIES COMPILE_OPTIONS "-Wno-unknown-pragmas")
endif()
add_library(petari_input STATIC
    bindings.cpp
    camera_input.cpp
    editor_input.cpp
    editor_pick.cpp
    kpad_record.cpp
    mods.cpp
    remap.cpp
    point_move.cpp
    remote_model.cpp
    smg2_extensions.cpp
    skip.cpp
    wpad_host.cpp
    ${PETARI_KPAD_SOURCE})
if(_petari_input_standalone AND BUILD_TESTING)
    target_compile_definitions(petari_input PRIVATE PETARI_INPUT_KPAD_TEST_SUPPORT=1
        PETARI_KPAD_TEST_SOURCE="${PETARI_LEGACY_ROOT}/src/RVL_SDK/kpad/KPAD.c")
endif()
petari_encode_target_sources(petari_input)
set_source_files_properties("${PETARI_LEGACY_ROOT}/src/RVL_SDK/kpad/KPAD.c" PROPERTIES
    LANGUAGE CXX
    COMPILE_OPTIONS "-include;cstring;-Wno-unknown-pragmas")
target_include_directories(petari_input PUBLIC include)
# petari_platform_vi: input activity resets the screen-saver dimming, as WPAD
# does on the Wii.
target_link_libraries(petari_input PUBLIC petari_platform_os petari_platform_sc petari_platform_vi petari_core
    petari_native_config petari_host_runtime)
target_compile_features(petari_input PUBLIC cxx_std_17)

# SDL3 event adapter (header-only; needs SDL3's headers, not its library).
# Linked by the application together with its own SDL3 target.
add_library(petari_input_sdl3 INTERFACE)
target_include_directories(petari_input_sdl3 INTERFACE include)
target_link_libraries(petari_input_sdl3 INTERFACE petari_input)
set(PETARI_SDL3_INCLUDE_DIR "" CACHE PATH "SDL3 include directory for the input adapter test (optional)")
if(NOT PETARI_SDL3_INCLUDE_DIR AND EXISTS "${PETARI_REPO_ROOT}/build/aurora/_deps/sdl-src/include/SDL3/SDL_events.h")
    set(PETARI_SDL3_INCLUDE_DIR "${PETARI_REPO_ROOT}/build/aurora/_deps/sdl-src/include")
endif()

if(BUILD_TESTING)
    # The game's own Wii Remote classes read the emulated KPAD output, so the
    # tests check what gameplay sees. Dead stripping drops the parts of these
    # files that reach the rest of the game.
    set(PETARI_INPUT_GAME_SOURCES
        # UBSan's vptr checks in WPadHolder reference GameSystem's base RTTI.
        ${PETARI_REPO_ROOT}/src/Game/System/NerveExecutor.cpp
        ${PETARI_REPO_ROOT}/src/Game/System/WPad.cpp
        ${PETARI_REPO_ROOT}/src/Game/System/WPadAcceleration.cpp
        ${PETARI_REPO_ROOT}/src/Game/System/WPadButton.cpp
        ${PETARI_REPO_ROOT}/src/Game/System/WPadHVSwing.cpp
        ${PETARI_REPO_ROOT}/src/Game/System/WPadHolder.cpp
        ${PETARI_REPO_ROOT}/src/Game/System/WPadInfoChecker.cpp
        ${PETARI_REPO_ROOT}/src/Game/System/WPadLeaveWatcher.cpp
        ${PETARI_REPO_ROOT}/src/Game/System/WPadPointer.cpp
        ${PETARI_REPO_ROOT}/src/Game/System/WPadRumble.cpp
        ${PETARI_REPO_ROOT}/src/Game/System/RumbleData.cpp
        ${PETARI_REPO_ROOT}/src/Game/System/RumbleDataNative.cpp
        ${PETARI_REPO_ROOT}/src/Game/System/WPadStick.cpp
        ${PETARI_REPO_ROOT}/src/Game/Util/GamePadUtil.cpp
        ${PETARI_REPO_ROOT}/src/Game/Util/HashUtil.cpp
        ${PETARI_REPO_ROOT}/src/Game/Util/MemoryUtil.cpp
        ${PETARI_REPO_ROOT}/src/Game/Util/TriggerChecker.cpp)
    add_executable(petari_input_tests ${PETARI_REPO_ROOT}/native/tests/input_tests.cpp ${PETARI_INPUT_GAME_SOURCES})
    petari_encode_target_sources(petari_input_tests)
    target_include_directories(petari_input_tests PRIVATE ${PETARI_REPO_ROOT})
    target_compile_options(petari_input_tests PRIVATE -Wno-inconsistent-missing-override)
    target_link_options(petari_input_tests PRIVATE -Wl,-dead_strip)
    target_link_libraries(petari_input_tests PRIVATE petari_input)
    if(PETARI_SDL3_INCLUDE_DIR)
        target_include_directories(petari_input_tests PRIVATE ${PETARI_SDL3_INCLUDE_DIR})
        target_compile_definitions(petari_input_tests PRIVATE PETARI_INPUT_TEST_SDL3=1)
    endif()
    add_test(NAME native_input COMMAND petari_input_tests)
    add_test(NAME native_cappy_input COMMAND petari_input_tests --cappy-only)
    if(PETARI_SDL3_INCLUDE_DIR)
        add_test(NAME native_input_sdl3 COMMAND petari_input_tests --sdl3-only)
    endif()

    add_executable(petari_smg2_sdk_tests ${PETARI_REPO_ROOT}/native/tests/smg2_sdk_tests.cpp)
    target_link_libraries(petari_smg2_sdk_tests PRIVATE petari_input)
    add_test(NAME native_smg2_sdk COMMAND petari_smg2_sdk_tests)

    # Remapping from the Home menu: capture, conflicts, the Home guard, controls.txt.
    # Exact tape recorder: sample encoding vs tools/headless.py, .kpd/.actions files.
    add_executable(petari_kpad_record_tests ${PETARI_REPO_ROOT}/native/tests/kpad_record_tests.cpp)
    target_link_libraries(petari_kpad_record_tests PRIVATE petari_input)
    add_test(NAME native_kpad_record COMMAND petari_kpad_record_tests)

    add_executable(petari_input_remap_tests ${PETARI_REPO_ROOT}/native/tests/input_remap_tests.cpp)
    target_link_libraries(petari_input_remap_tests PRIVATE petari_input)
    add_test(NAME native_input_remap COMMAND petari_input_remap_tests)

    add_executable(petari_smg2_controls_tests ${PETARI_REPO_ROOT}/native/tests/smg2_controls_tests.cpp)
    target_link_libraries(petari_smg2_controls_tests PRIVATE petari_input)
    add_test(NAME native_smg2_controls COMMAND petari_smg2_controls_tests)

    # Switch Pro Controller layouts: label tables, Galaxy/Odyssey switch, controls.txt sections, gyro, shake.
    add_executable(petari_pro_controller_tests ${PETARI_REPO_ROOT}/native/tests/pro_controller_tests.cpp)
    target_link_libraries(petari_pro_controller_tests PRIVATE petari_input)
    if(PETARI_SDL3_INCLUDE_DIR)
        target_include_directories(petari_pro_controller_tests PRIVATE ${PETARI_SDL3_INCLUDE_DIR})
        target_compile_definitions(petari_pro_controller_tests PRIVATE PETARI_INPUT_TEST_SDL3=1)
    endif()
    add_test(NAME native_pro_controller COMMAND petari_pro_controller_tests)

    add_executable(petari_pad_log_tests ${PETARI_REPO_ROOT}/native/tests/pad_log_tests.cpp)
    target_compile_features(petari_pad_log_tests PRIVATE cxx_std_17)
    if(PETARI_SANITIZERS)
        target_compile_options(petari_pad_log_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(petari_pad_log_tests PRIVATE -fsanitize=address,undefined)
    endif()
    add_test(NAME native_pad_log COMMAND petari_pad_log_tests)
    if(_petari_input_standalone AND PETARI_SDL3_INCLUDE_DIR)
        # The real app event dispatcher with recording SDL/input fakes, no GX or SDL library.
        add_executable(petari_app_input_events_tests ${PETARI_REPO_ROOT}/native/tests/app_input_events_tests.cpp
            ${PETARI_REPO_ROOT}/native/app/input_events.cpp bindings.cpp remap.cpp)
        target_include_directories(petari_app_input_events_tests PRIVATE ${PETARI_REPO_ROOT}/native/include
            include ${PETARI_SDL3_INCLUDE_DIR})
        target_compile_features(petari_app_input_events_tests PRIVATE cxx_std_17)
        if(PETARI_SANITIZERS)
            target_compile_options(petari_app_input_events_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(petari_app_input_events_tests PRIVATE -fsanitize=address,undefined)
        endif()
        add_test(NAME native_app_input_events COMMAND petari_app_input_events_tests)
    endif()

    # ShootEnemy mod: shared nearest-target policy and one-press transaction.
    add_executable(petari_shoot_nearest_enemy_tests ${PETARI_REPO_ROOT}/native/tests/shoot_nearest_enemy_tests.cpp)
    target_include_directories(petari_shoot_nearest_enemy_tests PRIVATE ${PETARI_REPO_ROOT}/native/include)
    target_compile_features(petari_shoot_nearest_enemy_tests PRIVATE cxx_std_17)
    add_test(NAME native_shoot_nearest_enemy COMMAND petari_shoot_nearest_enemy_tests)

    # OdysseyMovement mod: the SMO movement model alone (header-only).
    add_executable(petari_odyssey_move_tests ${PETARI_REPO_ROOT}/native/tests/odyssey_move_tests.cpp)
    target_include_directories(petari_odyssey_move_tests PRIVATE ${PETARI_REPO_ROOT}/native/include)
    target_compile_features(petari_odyssey_move_tests PRIVATE cxx_std_17)
    add_test(NAME native_odyssey_move COMMAND petari_odyssey_move_tests)

    # CollectStarBits: production eligibility, scene registry and press window.
    add_executable(petari_star_bits_collect_tests ${PETARI_REPO_ROOT}/native/tests/star_bits_collect_tests.cpp)
    target_link_libraries(petari_star_bits_collect_tests PRIVATE petari_native_config)
    target_compile_features(petari_star_bits_collect_tests PRIVATE cxx_std_17)
    add_test(NAME native_star_bits_collect COMMAND petari_star_bits_collect_tests)

    # Cappy mod: the cap's flight and bounce model alone (header-only).
    add_executable(petari_cappy_tests ${PETARI_REPO_ROOT}/native/tests/cappy_tests.cpp)
    target_include_directories(petari_cappy_tests PRIVATE ${PETARI_REPO_ROOT}/native/include)
    target_compile_features(petari_cappy_tests PRIVATE cxx_std_17)
    add_test(NAME native_cappy COMMAND petari_cappy_tests)
endif()
