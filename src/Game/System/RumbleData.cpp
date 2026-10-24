#include "Game/System/WPadRumbleData.hpp"
#include "Game/Util/HashUtil.hpp"

#ifdef GALAXY_NATIVE
extern RumblePattern sRumblePatterns[14];
#else
extern RumblePattern sRumblePatterns[13];
#endif

const RumblePattern* RumbleData::getPattern(const char* pName) {
    RumblePattern* pattern;
    u32 hash = MR::getHashCode(pName);
    RumblePattern* patterns = sRumblePatterns;
    for (u16 i = 0; i < 13; i++) {
        pattern = &patterns[i];
        if (hash == pattern->mHash) {
            return pattern;
        }
    }
    return nullptr;
}

void RumbleData::initHashValue() {
    RumblePattern* pattern;
    for (u16 i = 0; i < 13; i++) {
        pattern = &sRumblePatterns[i];
        pattern->mHash = MR::getHashCode(pattern->mName);
    }
}

void RumbleData::checkHashCollision() {
}
