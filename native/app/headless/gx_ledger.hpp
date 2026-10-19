#pragma once
#include "replay.hpp"
#include <cstdint>
#include <cstring>
#include <limits>
namespace PetariNative::Headless {
struct GxStats { std::uint64_t fifoBytes, stateCalls, displayListBytes; };
GxStats gxStats();
class GxLedger {
public:
    void begin(void* buffer, std::uint32_t capacity) {
        if (mBuffer || !buffer || !capacity) throw ReplayError("invalid/nested null GX display list");
        mBuffer = static_cast<std::uint8_t*>(buffer); mCapacity = capacity; mUsed = 0;
    }
    void write(const void* bytes, std::uint32_t count) {
        if (mBuffer) {
            if (count > mCapacity - mUsed) throw ReplayError("null GX display list overflow");
            std::memcpy(mBuffer + mUsed, bytes, count); mUsed += count;
        } else {
            if (count > std::numeric_limits<std::uint64_t>::max() - mStream) throw ReplayError("null GX FIFO overflow");
            mStream += count;
        }
    }
    std::uint32_t end() {
        if (!mBuffer) throw ReplayError("null GX display list was not started");
        const std::uint8_t zero[32]{};
        write(zero, (32 - mUsed % 32) % 32);
        const auto size = mUsed; mBuffer = nullptr; mUsed = mCapacity = 0; return size;
    }
    bool recording() const { return mBuffer != nullptr; }
    std::uint64_t bytes() const { return mStream; }
private:
    std::uint8_t* mBuffer = nullptr;
    std::uint32_t mCapacity = 0, mUsed = 0;
    std::uint64_t mStream = 0;
};
}
