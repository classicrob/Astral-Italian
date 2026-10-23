include_guard(GLOBAL)
include(FetchContent)

set(PETARI_LIBUSB_VERSION "1.0.30")
# Offline/pinned builds may point this at a local copy (file:///...); the SHA256 below is verified either way.
if(NOT PETARI_LIBUSB_URL)
    set(PETARI_LIBUSB_URL "https://github.com/libusb/libusb/releases/download/v1.0.30/libusb-1.0.30.tar.bz2")
endif()
set(PETARI_LIBUSB_SHA256 "fea36f34f9156400209595e300840767ab1a385ede1dc7ee893015aea9c6dbaf")
set(PETARI_SDL3_REF "refs/tags/release-3.4.10")
get_filename_component(_petari_native "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if(NOT APPLE)
    message(FATAL_ERROR "Petari's bundled libusb recipe requires macOS")
endif()

# SDL's FindLibUSB runs during FetchContent configuration, before build targets
# can run. A small, stamped ExternalProject sub-build supplies the real header
# and dylib now; subsequent configures let ExternalProject's stamps do no work.
set(_petari_usb_build "${CMAKE_BINARY_DIR}/petari-libusb")
set(PETARI_LIBUSB_PREFIX "${_petari_usb_build}/install")
set(PETARI_LIBUSB_LIBRARY "${PETARI_LIBUSB_PREFIX}/lib/libusb-1.0.0.dylib" CACHE INTERNAL "Bundled libusb" FORCE)
execute_process(COMMAND "${CMAKE_COMMAND}"
    -S "${CMAKE_CURRENT_LIST_DIR}/libusb" -B "${_petari_usb_build}" -G Ninja
    "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
    "-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}"
    "-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}"
    "-DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}"
    "-DPETARI_LIBUSB_URL=${PETARI_LIBUSB_URL}"
    "-DPETARI_LIBUSB_SHA256=${PETARI_LIBUSB_SHA256}"
    RESULT_VARIABLE _petari_usb_result
    OUTPUT_FILE "${CMAKE_BINARY_DIR}/petari-libusb-configure.log"
    ERROR_FILE "${CMAKE_BINARY_DIR}/petari-libusb-configure-errors.log")
if(NOT _petari_usb_result EQUAL 0)
    message(FATAL_ERROR "libusb configure failed; see ${CMAKE_BINARY_DIR}/petari-libusb-configure*.log")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" --build "${_petari_usb_build}" --parallel 2
    RESULT_VARIABLE _petari_usb_result
    OUTPUT_FILE "${CMAKE_BINARY_DIR}/petari-libusb-build.log"
    ERROR_FILE "${CMAKE_BINARY_DIR}/petari-libusb-build-errors.log")
if(NOT _petari_usb_result EQUAL 0 OR NOT EXISTS "${PETARI_LIBUSB_LIBRARY}")
    message(FATAL_ERROR "libusb build failed; see ${CMAKE_BINARY_DIR}/petari-libusb-build*.log")
endif()
set(PETARI_LIBUSB_COPYING "${_petari_usb_build}/src/libusb/COPYING" CACHE INTERNAL "libusb licence" FORCE)
set(LibUSB_INCLUDE_PATH "${PETARI_LIBUSB_PREFIX}/include/libusb-1.0" CACHE PATH "Bundled libusb headers" FORCE)
set(LibUSB_LIBRARY "${PETARI_LIBUSB_LIBRARY}" CACHE FILEPATH "Bundled libusb shared library" FORCE)
foreach(_option COMPILE_OPTIONS LINK_LIBRARIES LINK_FLAGS LINK_OPTIONS)
    set(LibUSB_${_option} "" CACHE STRING "No system libusb flags" FORCE)
endforeach()
set(SDL_HIDAPI ON CACHE BOOL "Enable HIDAPI" FORCE)
set(SDL_HIDAPI_LIBUSB ON CACHE BOOL "Enable bundled libusb" FORCE)
set(SDL_HIDAPI_LIBUSB_SHARED ON CACHE BOOL "Load libusb dynamically" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build static SDL" FORCE)
set(SDL_SHARED OFF CACHE BOOL "Build static SDL" FORCE)

# Aurora has been declared, but is populated only by MakeAvailable below. Its
# source directory will exist when SDL's PATCH_COMMAND runs inside Aurora.
if(FETCHCONTENT_SOURCE_DIR_AURORA)
    set(_petari_aurora "${FETCHCONTENT_SOURCE_DIR_AURORA}")
else()
    set(_petari_aurora "${FETCHCONTENT_BASE_DIR}/aurora-src")
endif()
file(GLOB PETARI_SDL_PATCHES CONFIGURE_DEPENDS "${_petari_native}/gx/sdl-patches/*.patch")
list(SORT PETARI_SDL_PATCHES)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${PETARI_SDL_PATCHES}
    "${CMAKE_CURRENT_LIST_DIR}/ApplySDLPatches.cmake"
    "${CMAKE_CURRENT_LIST_DIR}/libusb/CMakeLists.txt")
set(_petari_patch_identity "")
file(SHA256 "${CMAKE_CURRENT_LIST_DIR}/ApplySDLPatches.cmake" _patch_script_hash)
string(APPEND _petari_patch_identity "${_patch_script_hash}\n")
foreach(_patch IN LISTS PETARI_SDL_PATCHES)
    file(SHA256 "${_patch}" _hash)
    get_filename_component(_name "${_patch}" NAME)
    string(APPEND _petari_patch_identity "${_name}:${_hash}\n")
endforeach()
string(SHA256 PETARI_SDL_PATCH_SET_SHA256 "${_petari_patch_identity}")
# First declaration wins. Keep Aurora's two patches as well as our ordered set.
# Including the patch-set hash in PATCH_COMMAND invalidates its build stamp
# when a patch changes or a new patch (e.g. 0002) is added.
FetchContent_Declare(SDL
    URL "https://github.com/libsdl-org/SDL/archive/${PETARI_SDL3_REF}.tar.gz"
    URL_HASH SHA256=0dc11d980ba17250200718fa4e28011da293f27ed92f92203afffe396811f307
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    PATCH_COMMAND "${CMAKE_COMMAND}" -DSDL_SOURCE_DIR=<SOURCE_DIR>
        -P "${_petari_aurora}/cmake/patches/apply-sdl3-android-nintendo-auto-mapping.cmake"
    COMMAND "${CMAKE_COMMAND}" -DSDL_SOURCE_DIR=<SOURCE_DIR>
        -P "${_petari_aurora}/cmake/patches/apply-sdl3-android-security-exception.cmake"
    COMMAND "${CMAKE_COMMAND}" -DSDL_SOURCE_DIR=<SOURCE_DIR>
        "-DPATCH_DIR=${_petari_native}/gx/sdl-patches"
        "-DPATCH_SET_SHA256=${PETARI_SDL_PATCH_SET_SHA256}"
        -P "${CMAKE_CURRENT_LIST_DIR}/ApplySDLPatches.cmake"
    EXCLUDE_FROM_ALL)

# Shared by the full app and the asset-free loader check, so bundle layout,
# licence payload, rpath and nested signing receive the same verification.
function(petari_bundle_libusb target)
    get_filename_component(_petari_native "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/.." ABSOLUTE)
    target_sources(${target} PRIVATE "${PETARI_LIBUSB_LIBRARY}")
    set_source_files_properties("${PETARI_LIBUSB_LIBRARY}" PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
    set(_licenses "${CMAKE_CURRENT_BINARY_DIR}/libusb-licenses")
    file(MAKE_DIRECTORY "${_licenses}")
    configure_file("${PETARI_LIBUSB_COPYING}" "${_licenses}/COPYING" COPYONLY)
    configure_file("${_petari_native}/licenses/THIRD_PARTY_NOTICES.txt" "${_licenses}/THIRD_PARTY_NOTICES.txt" COPYONLY)
    foreach(_license COPYING THIRD_PARTY_NOTICES.txt)
        target_sources(${target} PRIVATE "${_licenses}/${_license}")
        set_source_files_properties("${_licenses}/${_license}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources/licenses)
    endforeach()
    set_property(TARGET ${target} APPEND PROPERTY BUILD_RPATH "@executable_path/../Frameworks")
    set_property(TARGET ${target} APPEND PROPERTY INSTALL_RPATH "@executable_path/../Frameworks")
    # Resource-only changes must rerun the final signature as well.
    set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${PETARI_LIBUSB_LIBRARY}"
        "${_licenses}/COPYING" "${_licenses}/THIRD_PARTY_NOTICES.txt")
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND /usr/bin/codesign --force --sign - "$<TARGET_BUNDLE_DIR:${target}>/Contents/Frameworks/libusb-1.0.0.dylib"
        COMMAND /usr/bin/codesign --force --sign - "$<TARGET_BUNDLE_DIR:${target}>"
        VERBATIM)
endfunction()
