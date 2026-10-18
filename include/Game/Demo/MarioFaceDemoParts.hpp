#pragma once

#include "Game/LiveActor/LiveActor.hpp"

// Shared part table used by the MarioFace demo objects (MarioFaceAfterGrandStarTakeOff, MarioFacePlanetTakeOff).
namespace MarioFaceDemoParts {
    struct PartInfo {
        const char* mObjName;
        const char* mArcName;
        const char* mAnimArcName;
        const char* mJointName;
    };

    s32 getFixedPartCount();
    const PartInfo* getPartInfo(s32 index);
    LiveActor* createPart(LiveActor* pOwner, const PartInfo* pInfo, const char* pName, const JMapInfoIter& rIter);
    void closeWhiteFadeAfterGrandStar();
};  // namespace MarioFaceDemoParts
