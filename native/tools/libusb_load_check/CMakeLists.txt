cmake_minimum_required(VERSION 3.25)
project(PetariLibUSBLoadCheck LANGUAGES C OBJC)
if(NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "Target architecture" FORCE)
endif()
get_filename_component(_native "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
include(FetchContent)
# Only the two Android patch scripts are needed from Aurora; never configure GX.
FetchContent_Declare(aurora
    URL https://github.com/encounter/aurora/archive/08122911e8621acb7ded6563813b264bec1494b5.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE)
FetchContent_GetProperties(aurora)
if(NOT aurora_POPULATED)
    FetchContent_Populate(aurora)
endif()
include("${_native}/cmake/PadDependencies.cmake")
FetchContent_MakeAvailable(SDL)
include("${aurora_SOURCE_DIR}/cmake/AuroraDependencyVersions.cmake")
include("${_native}/cmake/GraphicsDependencies.cmake")
file(WRITE "${CMAKE_BINARY_DIR}/dependency-paths.tsv" "")
petari_record_pad_dependencies("${CMAKE_BINARY_DIR}/dependency-paths.tsv")
add_executable(libusb_load_check MACOSX_BUNDLE main.c)
set_target_properties(libusb_load_check PROPERTIES OUTPUT_NAME check MACOSX_BUNDLE_BUNDLE_NAME check)
target_link_libraries(libusb_load_check PRIVATE SDL3::SDL3-static)
petari_bundle_libusb(libusb_load_check)
include(CTest)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME native_libusb_bundle COMMAND "${Python3_EXECUTABLE}"
    "${_native}/tests/libusb_bundle_tests.py"
    --executable "$<TARGET_FILE:libusb_load_check>"
    --library "$<TARGET_BUNDLE_DIR:libusb_load_check>/Contents/Frameworks/libusb-1.0.0.dylib")
set_tests_properties(native_libusb_bundle PROPERTIES TIMEOUT 180 LABELS build-tooling)
