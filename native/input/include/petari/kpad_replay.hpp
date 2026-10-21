#pragma once
#include <revolution/kpad.h>
namespace PetariNative::Input {
struct KpadReplayHooks {
    s32 (*read)(s32 channel, KPADStatus* buffer, u32 capacity, s32* error, void* user) = nullptr;
    void* user = nullptr;
};
// Mod buttons (petari_mod_take_press / petari_mod_held) come from the host action layer, not from KPAD. A tape
// replay answers them from the recorded <tape>.actions instead; `press` is asked once per frame and mod (a press is
// consumed by the first call, like Input::takeActionPresses), `held` as often as the game asks.
struct ModReplayHooks {
    bool (*press)(int mod, void* user) = nullptr;
    bool (*held)(int mod, void* user) = nullptr;
    void* user = nullptr;
};
void setModReplayHooks(const ModReplayHooks& hooks);
// Install before WPADInit. A replay owns all input; host report pumping is disabled.
void setKpadReplayHooks(const KpadReplayHooks& hooks);
// Host interrupt thread only. States must come from the input log, not live devices.
void replayConnection(s32 channel, s32 probeError, u32 deviceType);
void serviceReplayCommands();
}
extern "C" BOOL petari_kpad_replay_read(s32 channel, KPADStatus* buffer, u32 capacity, s32* error, s32* returned);
