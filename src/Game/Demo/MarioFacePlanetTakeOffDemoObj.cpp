#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Demo/MarioFacePlanetTakeOffDemoObj.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/System/GalaxyWorldMapInfo.hpp"
#include "Game/Demo/MarioFaceDemoParts.hpp"
#include "Game/Demo/UnnamedDemoCallee.hpp"

extern "C" char lbl_8067FCD0[];
extern "C" char lbl_8067FCE4[];
extern "C" const char* lbl_807CF718;
extern "C" const char* lbl_807CF71C;

// Part table (entries at lbl_8067FC90, 0x10 bytes each).
extern "C" const MarioFaceDemoParts::PartInfo lbl_8067FC90[];

namespace NrvMarioFacePlanetTakeOffDemoObj {
    NEW_NERVE(MarioFacePlanetTakeOffDemoObjNrvWait, MarioFacePlanetTakeOffDemoObj, Wait);
    NEW_NERVE(MarioFacePlanetTakeOffDemoObjNrvDemo, MarioFacePlanetTakeOffDemoObj, Demo);
};  // namespace NrvMarioFacePlanetTakeOffDemoObj

MarioFacePlanetTakeOffDemoObj::MarioFacePlanetTakeOffDemoObj(const char* pName) : LiveActor(pName), mCameraInfo(nullptr), mParts(nullptr), mCount(0) {
}

void MarioFacePlanetTakeOffDemoObj::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, lbl_8067FCD0, false);
    mCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initAnimCamera(this, mCameraInfo, lbl_807CF71C);
    MR::tryRegisterDemoCast(this, rIter);
    mCount = 4;
    mParts = new LiveActor*[mCount];
    for (s32 i = 0; i < mCount; i++) {
        mParts[i] = MarioFaceDemoParts::createPart(this, &lbl_8067FC90[i], lbl_807CF718, rIter);
    }
    MR::invalidateClipping(this);
    initNerve(GET_NERVE(MarioFacePlanetTakeOffDemoObj, MarioFacePlanetTakeOffDemoObjNrvWait), 0);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void MarioFacePlanetTakeOffDemoObj::exeWait() {
    if (MR::tryStartTimeKeepDemoMarioPuppetable(this, lbl_807CF718, nullptr)) {
        MR::setPlayerBaseMtx(getBaseMtx());
        setNerve(GET_NERVE(MarioFacePlanetTakeOffDemoObj, MarioFacePlanetTakeOffDemoObjNrvDemo));
        for (s32 i = 0; i < mCount; i++) {
            mParts[i]->makeActorAppeared();
        }
        MR::overlayWithPreviousScreen(2);
    }
}

void MarioFacePlanetTakeOffDemoObj::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::setPlayerBaseMtx(getBaseMtx());
    }
    TVec3f dir;
    MR::extractMtxYDir(MR::getJointMtx(this, lbl_8067FCD0), &dir);
    dir.negate();
    for (s32 i = 0; i < mCount; i++) {
        LiveActor* pPart = mParts[i];
        pPart->mGravity.x = dir.x;
        pPart->mGravity.y = dir.y;
        pPart->mGravity.z = dir.z;
    }
    if (MR::isDemoLastStep()) {
        MR::closeSystemWipeFadeWithCaptureScreen(0);
        GameSequenceFunction::requestChangeStageWorldMapEnterType0();
        kill();
    }
}

MarioFacePlanetTakeOffDemoObj::~MarioFacePlanetTakeOffDemoObj() {
}

s32 MarioFaceDemoParts::getFixedPartCount() {
    return 4;
}

const MarioFaceDemoParts::PartInfo* MarioFaceDemoParts::getPartInfo(s32 index) {
    return &lbl_8067FC90[index];
}

LiveActor* MarioFaceDemoParts::createPart(LiveActor* pOwner, const PartInfo* pInfo, const char* pName, const JMapInfoIter& rIter) {
    MtxPtr mtx;
    if (pInfo->mJointName != nullptr) {
        mtx = MR::getJointMtx(pOwner, pInfo->mJointName);
    } else {
        mtx = pOwner->getBaseMtx();
    }
    DemoObj* pActor;
    if (pInfo->mAnimArcName != nullptr) {
        pActor = MR::createDemoObjWithAnimArc(pInfo->mObjName, pInfo->mArcName, pInfo->mAnimArcName, lbl_8067FCE4, mtx);
    } else {
        pActor = MR::createDemoObj(pInfo->mObjName, pInfo->mArcName, lbl_8067FCE4, mtx);
    }
    pActor->makeActorDead();
    MR::invalidateClipping(pActor);
    MR::tryRegisterDemoCast(pActor, pName, rIter);
    return pActor;
}
