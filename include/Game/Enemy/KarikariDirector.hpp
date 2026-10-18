#pragma once

#include "Game/LiveActor/LiveActorGroup.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class Karikari;
class FixedPosition;

class KarikariDirector : public LiveActorGroup {
public:
    KarikariDirector(const char*);

    virtual void movement();

    bool isMaxNumCling() const;
    void removeAllClingingKarikari();
    void blowOutAllClingingKarikari(const TVec3f&);
    void electricKillAllClingingKarikari();
    bool requestRelease(Karikari*, TVec2f, f32);
    bool registCling(Karikari*);
    bool unregistCling(Karikari*);
    bool fn_801B0DB0(Karikari*);

    inline MtxPtr getClingMtx(int idx) const {
        return (MtxPtr)&mClingJointMtxs[idx];
    }

    /* 0x20 */ s32 mClingNum;
    /* 0x24 */ TPos3f* mClingJointMtxs;  // array
    /* 0x28 */ FixedPosition** mClingPositions;
    /* 0x2C */ Karikari** mClingingKarikari;
    /* 0x30 */ s32 mRumbleTime;
};

struct ClingData {
    /* 0x00 */ const char* mJointName;
    /* 0x04 */ Vec mPos;
    /* 0x10 */ Vec mRotDegree;
};

namespace MR {
    s32 getKarikariClingNum();
    s32 getClingNumMax();
    void removeAllClingingKarikari();
};  // namespace MR
