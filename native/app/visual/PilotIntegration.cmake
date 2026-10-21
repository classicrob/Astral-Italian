# Source-only integration helper; root must include/call it explicitly after app targets exist.
function(petari_enable_draw_pilot_integration)
    if(TARGET petari_headless_traps)
        message(FATAL_ERROR "Manual partial-link inventory must bind the observer archive before integration")
    endif()
    petari_link_draw_observer_to_game()
    add_library(petari_draw_pilot_core STATIC
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/draw_pilot_bridge.cpp")
    target_link_libraries(petari_draw_pilot_core PUBLIC petari_draw_observer_core)
    if(TARGET petari_native_config)
        add_library(petari_draw_pilot_host STATIC
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/draw_pilot_host.cpp"
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/draw_snapshot_no_xfb.cpp")
        target_link_libraries(petari_draw_pilot_host PRIVATE petari_native_config
            PUBLIC petari_draw_pilot_core)
        if(TARGET petari_app_host)
            target_link_libraries(petari_app_host PRIVATE petari_draw_pilot_host)
            target_compile_definitions(petari_app_host PRIVATE PETARI_DRAW_PILOT_HOST_SEAM=1)
        endif()
    endif()
endfunction()
