cmake_minimum_required(VERSION 3.25)
project(PetariLibUSB LANGUAGES C)
include(ExternalProject)
find_program(PETARI_MAKE make PATHS /usr/bin REQUIRED NO_DEFAULT_PATH)
set(_flags "")
foreach(_arch IN LISTS CMAKE_OSX_ARCHITECTURES)
    string(APPEND _flags " -arch ${_arch}")
endforeach()
if(CMAKE_OSX_DEPLOYMENT_TARGET)
    string(APPEND _flags " -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()
if(CMAKE_OSX_SYSROOT)
    string(APPEND _flags " -isysroot ${CMAKE_OSX_SYSROOT}")
endif()
# A parent architecture/SDK/deployment change needs fresh objects, even if make
# would otherwise regard the previously configured C objects as up to date.
string(SHA256 _configuration "${CMAKE_C_COMPILER}|${_flags}|${PETARI_LIBUSB_SHA256}|no-pipe2")
ExternalProject_Add(libusb
    URL "${PETARI_LIBUSB_URL}"
    URL_HASH "SHA256=${PETARI_LIBUSB_SHA256}"
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    PREFIX "${CMAKE_BINARY_DIR}"
    SOURCE_DIR "${CMAKE_BINARY_DIR}/src/libusb"
    BINARY_DIR "${CMAKE_BINARY_DIR}/build-${_configuration}"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
    CONFIGURE_COMMAND "${CMAKE_COMMAND}" -E env
        "CC=${CMAKE_C_COMPILER}" "CFLAGS=-O2${_flags}" "LDFLAGS=${_flags}"
        "CPPFLAGS=" "LIBS=" "PKG_CONFIG_PATH=" "PKG_CONFIG_LIBDIR="
        # pipe2 first shipped in macOS 26: an SDK that declares it makes configure pick it, and libusb then calls the
        # (weakly linked, null) symbol on older systems. Its pipe()+fcntl fallback works everywhere.
        <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --disable-static --enable-shared ac_cv_func_pipe2=no
    BUILD_COMMAND "${PETARI_MAKE}" -j2
    INSTALL_COMMAND "${PETARI_MAKE}" install
    COMMAND /usr/bin/install_name_tool -id @rpath/libusb-1.0.0.dylib <INSTALL_DIR>/lib/libusb-1.0.0.dylib
    BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/install/lib/libusb-1.0.0.dylib"
    LOG_DOWNLOAD TRUE LOG_CONFIGURE TRUE LOG_BUILD TRUE LOG_INSTALL TRUE)
