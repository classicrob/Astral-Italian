#pragma once

#include <revolution.h>

class JMapInfoIter;

class JMapIdInfo {
public:
    JMapIdInfo() {
    }

    // SMG2 has out-of-line copies of the copy ctor and operator= that a few TUs call (bl). Define
    // JMAPIDINFO_NOINLINE_COPY / JMAPIDINFO_NOINLINE_ASSIGN before the includes of such a TU.
#if defined(JMAPIDINFO_TRIVIAL_COPY)
    // No user-declared copy ctor: the class stays trivially copyable, so MWCC returns it in r3:r4 (like the target's
    // MR::createJMapIdInfoFromGroupId) instead of through a hidden sret pointer.
#elif defined(JMAPIDINFO_NOINLINE_COPY)
    JMapIdInfo(const JMapIdInfo& rInfo);
#else
    JMapIdInfo(const JMapIdInfo& rInfo) __attribute__((forceinline)) {
        _0 = rInfo._0;
        mZoneID = rInfo.mZoneID;
    }
#endif

    JMapIdInfo(s32, s32);
    JMapIdInfo(s32, const JMapInfoIter&);

    void initialize(s32, const JMapInfoIter&);

#ifdef JMAPIDINFO_NOINLINE_ASSIGN
    void operator=(const JMapIdInfo& rhs);
#else
    void operator=(const JMapIdInfo& rhs) {
        _0 = rhs._0;
        mZoneID = rhs.mZoneID;
    }
#endif

#ifdef JMAPIDINFO_NOINLINE_EQ
    bool operator==(const JMapIdInfo& rhs) const NO_INLINE {
#else
    bool operator==(const JMapIdInfo& rhs) const {
#endif
        return _0 == rhs._0 && mZoneID == rhs.mZoneID;
    }

    int getZoneID() const {
        return mZoneID;
    }

    s32 _0;
    int mZoneID;  // 0x04
};

namespace MR {
    JMapIdInfo createJMapIdInfoFromGroupId(const JMapInfoIter&);
    JMapIdInfo createJMapIdInfoFromClippingGroupId(const JMapInfoIter&);
};  // namespace MR
