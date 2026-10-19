#include "platform.hpp"
#include <petari/platform/virtual_time.hpp>
#include <petari/platform/audio.hpp>
#include <petari/platform/dvd.hpp>
#include <petari/platform/nand.hpp>
#include <petari/platform/sc.hpp>
#include <petari/kpad_replay.hpp>
#include <revolution/dvd.h>
#include <revolution/nand.h>
#include <cstring>
#include <filesystem>
#include <limits>
#include <algorithm>

namespace PetariNative::Headless {
void advanceDevices(std::uint64_t target) {
    namespace OS=PetariNative::Platform::OS;
    namespace Audio=PetariNative::Platform::Audio;
    if (target > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) throw ReplayError("virtual time exceeds signed SDK clock");
    static std::uint64_t audioFraction = 0;
    static std::uint64_t nextInputTick = 303750;
    auto now = static_cast<std::uint64_t>(OS::virtualSystemTime());
    std::int16_t discarded[128];
    while (now < target) {
        const auto next = std::min({target, now + 60750, nextInputTick});
        OS::advanceVirtualTimeTo(static_cast<std::int64_t>(next));
        if (next == nextInputTick) {
            PetariNative::Input::serviceReplayCommands();
            nextInputTick += 303750;
        }
        audioFraction += (next-now)*Audio::outputRate();
        auto frames=audioFraction/60750000; audioFraction%=60750000;
        while (frames) {
            const auto amount=std::min<std::uint64_t>(frames,64);
            const auto pulled=Audio::pull(discarded,amount);
            if (pulled!=amount) throw ReplayError("null AI pull did not consume its fixed sample budget");
            Audio::drainInterrupts();
            frames-=pulled;
        }
        now=next;
    }
}
void configurePlatform(const char* disc, const char* user) {
    namespace P=PetariNative::Platform;
    std::string error;
    P::DVD::MountOptions mount; mount.root=disc;
    if (!P::DVD::mount(mount,&error)) throw ReplayError("disc: "+error);
    const std::filesystem::path nand=std::filesystem::path(user)/"NAND";
    std::filesystem::create_directories(nand);
    if (!P::NAND::mount(nand,&error)) throw ReplayError("isolated NAND: "+error);
    if (!P::SC::setStore(std::filesystem::path(user)/"settings.txt",&error)) throw ReplayError("settings: "+error);
    P::Audio::setRegistrationWaitLimit(0);
    P::Audio::setSink({});
}
}
