# Opt-in M1/macOS 15 toolchain; see docs/dev/M1_LLVM.md.
# No system compiler, SDK, or existing preset is changed.
set(PETARI_M1_LLVM_ROOT "$ENV{HOME}/toolchains/LLVM-20.1.8-macOS-ARM64" CACHE PATH "Official upstream LLVM 20.1.8 arm64 tree")
foreach(_required bin/clang bin/clang++ bin/ld64.lld include/c++/v1/__config_site lib/libc++.a lib/libc++abi.a lib/libunwind.a)
    if(NOT EXISTS "${PETARI_M1_LLVM_ROOT}/${_required}")
        message(FATAL_ERROR "Incomplete LLVM20 M1 toolchain: ${PETARI_M1_LLVM_ROOT}/${_required}")
    endif()
endforeach()
set(CMAKE_C_COMPILER "${PETARI_M1_LLVM_ROOT}/bin/clang" CACHE FILEPATH "")
set(CMAKE_CXX_COMPILER "${PETARI_M1_LLVM_ROOT}/bin/clang++" CACHE FILEPATH "")
set(CMAKE_OBJC_COMPILER "${PETARI_M1_LLVM_ROOT}/bin/clang" CACHE FILEPATH "")
set(CMAKE_OBJCXX_COMPILER "${PETARI_M1_LLVM_ROOT}/bin/clang++" CACHE FILEPATH "")
set(CMAKE_ASM_COMPILER "${PETARI_M1_LLVM_ROOT}/bin/clang" CACHE FILEPATH "")
set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "")
set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0" CACHE STRING "")
# Avoid SDK C++ headers and implicit -lc++; use a matched ABI/unwind trio.
set(CMAKE_CXX_FLAGS_INIT "-nostdinc++ -isystem \"${PETARI_M1_LLVM_ROOT}/include/c++/v1\" -nostdlib++")
set(CMAKE_OBJCXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT}")
# Apple ld loads this LLVM's libLTO and hits a libc++ ABI lookup failure.
# Use the release-matched linker. Static runtimes avoid two libc++ images
# serving the app. Keep executable symbols private so dyld cannot coalesce
# upstream stream/locale vtables with the system libc++ used by frameworks.
set(_petari_m1_link "-fuse-ld=lld")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${_petari_m1_link} -Wl,-no_exported_symbols")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_petari_m1_link}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${_petari_m1_link}")
set(CMAKE_CXX_STANDARD_LIBRARIES "\"${PETARI_M1_LLVM_ROOT}/lib/libc++.a\" \"${PETARI_M1_LLVM_ROOT}/lib/libc++abi.a\" \"${PETARI_M1_LLVM_ROOT}/lib/libunwind.a\"" CACHE STRING "")
set(CMAKE_OBJCXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES}" CACHE STRING "")
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES PETARI_M1_LLVM_ROOT)
