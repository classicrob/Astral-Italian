# Build-time check (cmake -P): SDL's generated build configuration enabled dynamic libusb.
# The headers are written by SDL's file(GENERATE), after the configure step that records the dependencies.
# Required: -DSDL_BINARY_DIR=<SDL build directory>
file(GLOB _configs "${SDL_BINARY_DIR}/include-config-*/build_config/SDL_build_config.h")
if(NOT _configs)
    message(FATAL_ERROR "SDL generated build configuration is missing under ${SDL_BINARY_DIR}")
endif()
foreach(_config IN LISTS _configs)
    file(READ "${_config}" _contents)
    if(NOT _contents MATCHES "#define HAVE_LIBUSB 1"
            OR NOT _contents MATCHES "#define SDL_LIBUSB_DYNAMIC \"")
        message(FATAL_ERROR "SDL did not enable dynamic libusb in ${_config}")
    endif()
endforeach()
