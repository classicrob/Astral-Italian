# Platform framework types must stay out of the header-only trace producer.
# Both the root/platform build and standalone app tests share this one target.
include_guard(GLOBAL)
get_filename_component(_petari_gpu_trace_native "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
add_library(petari_gpu_trace_host STATIC "${_petari_gpu_trace_native}/src/gpu_trace_host.cpp")
target_include_directories(petari_gpu_trace_host PUBLIC "${_petari_gpu_trace_native}/include")
target_compile_features(petari_gpu_trace_host PUBLIC cxx_std_17)
if(APPLE)
    target_link_libraries(petari_gpu_trace_host PUBLIC "-framework CoreFoundation" "-framework IOKit")
endif()
