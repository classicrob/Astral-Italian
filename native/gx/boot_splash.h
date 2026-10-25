#pragma once
#include <cstddef>
// AppKit only; these never record a GX packet or submit a Metal draw.
extern "C" void petari_boot_splash_progress(std::size_t published, std::size_t total);
extern "C" void petari_boot_splash_presented(); // render worker: schedule main-thread removal once
