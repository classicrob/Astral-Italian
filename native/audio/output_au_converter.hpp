#pragma once
#include "output_au_core.hpp"
#include <AudioToolbox/AudioConverter.h>
#include <array>

namespace PetariNative::AudioOutputAU::Detail {
// Producer-owned only. Its input callback never pulls AI. Two slots preserve
// AudioConverter's input-buffer lifetime through the NEXT input callback.
class Converter {
public:
    static constexpr unsigned kInputFrames=256;
    static constexpr OSStatus kNeedInput='pIn?';
    ~Converter();
    bool open(unsigned guestRate, double deviceRate, unsigned channels, unsigned outputFrames);
    bool prewarm(); // control only; finite silence exercise, then reset + zero ledger
    bool push(const std::int16_t*, unsigned frames);
    bool convert(Ring&, bool eof=false);
    bool hasPendingInput() const { return pendingFrames!=0; }
    std::uint64_t consumed() const { return accepted; }
    std::uint64_t output() const { return produced; }
    AudioConverterPrimeInfo primeInfo() const { return prime; }
    OSStatus error() const { return lastError; }
    unsigned inputCapacity() const { return kInputFrames; }
private:
    static OSStatus supply(AudioConverterRef, UInt32*, AudioBufferList*, AudioStreamPacketDescription**, void*);
    AudioConverterRef converter=nullptr;
    AudioConverterPrimeInfo prime{};
    std::array<std::array<std::int16_t,kInputFrames*2>,2> input{};
    unsigned pendingSlot=0, heldSlot=1, pendingFrames=0, outputLimit=0, channels=0;
    bool ending=false;
    std::unique_ptr<float[]> converted;
    std::uint64_t accepted=0, produced=0;
    OSStatus lastError=0;
};
}
