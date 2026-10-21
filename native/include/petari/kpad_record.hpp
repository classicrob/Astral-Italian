#pragma once
// Exact tape recording (docs: native/PLAYTEST.md "Recording a tape"). While a tape is open, every KPADReadEx
// the game makes is run through the SDK's own KPADiRead and its result (count, error, every sample, the
// connection state seen) is appended to <base>.kpd in the format `petari_headless --inputs` consumes
// (tools/headless.py pack, big-endian 240-byte samples, absolute frame numbers = game loops since launch).
// Mod button presses (petari_mod_take_press/held) are appended to <base>.actions. Nothing here changes what
// the game reads: the recording path returns exactly what the unrecorded path would.
#include <revolution/kpad.h>

#include <cstdint>
#include <string>

namespace PetariNative::Input::KpadRecord {

// One sample <-> the 240-byte big-endian KPADStatus image of the tape. encode() is false when the sample
// cannot be represented exactly (non-finite float, an extension union the tape format does not carry).
bool encodeSample(const KPADStatus& sample, unsigned char out[240]);
bool decodeSample(const unsigned char in[240], KPADStatus* out);

bool open(const std::string& base, std::string* error);  // creates <base>.kpd and <base>.actions, exclusively
bool active();
void seam();                                              // game-loop boundary (post-endFrame seam): next frame
std::uint64_t frame();                                    // the loop the current reads belong to
void read(s32 channel, u32 capacity, s32 returned, s32 error, s32 probeError, u32 device, const KPADStatus* samples);
void modPress(int mod, bool press);                       // an answer of petari_mod_take_press
void modHeld(int mod, bool held);                         // an answer of petari_mod_held (the frame's first is kept)
void checkpoint();                                        // at a seam: flush and make the header valid for the loops so far
void close();                                             // writes the final frame count into the header

struct Stats {
    std::uint64_t reads = 0, samples = 0, frames = 0, modLines = 0;
    std::uint64_t unencodable = 0;  // samples the tape format could not carry exactly (should stay 0)
    std::uint64_t lossy = 0;        // samples whose encode->decode differs from what the game saw (should stay 0)
};
Stats stats();

}  // namespace PetariNative::Input::KpadRecord
