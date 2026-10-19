#include "Game/System/WPadRumbleData.hpp"
#include "Game/Util/HashUtil.hpp"

extern RumblePattern sRumblePatterns[13];

const RumblePattern* RumbleData::getPattern(const char* pName) {
    u32 hash = MR::getHashCode(pName);
    for (u16 i = 0; i < 13; i++) {
        RumblePattern* pattern = &sRumblePatterns[i];
        if (hash == pattern->mHash) {
            return pattern;
        }
    }
    return nullptr;
}

void RumbleData::initHashValue() {
    for (u16 i = 0; i < 13; i++) {
        RumblePattern* pattern = &sRumblePatterns[i];
        pattern->mHash = MR::getHashCode(pattern->mName);
    }
}

void RumbleData::checkHashCollision() {
}
