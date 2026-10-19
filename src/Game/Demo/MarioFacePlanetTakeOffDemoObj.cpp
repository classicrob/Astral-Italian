#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Demo/MarioFacePlanetTakeOffDemoObj.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/System/GalaxyWorldMapInfo.hpp"
#include "Game/Demo/MarioFaceDemoParts.hpp"
#include "Game/Demo/UnnamedDemoCallee.hpp"

namespace {
    const char* sPartDemoName = "マリオ顔惑星発進デモ";
    const char* sCameraName = "DemoMarioShipTakeOff";

    MarioFaceDemoParts::PartInfo sParts[] = {
        {"デモ用マリオ顔惑星", "DemoMarioFaceShipPlanet", nullptr, "MarioShipTakeOff"},
        {"デモ用マイスター", "Meister", "MeisterAnim", nullptr},
        {"デモ用発進パネル", "WorldMapAccessPanel", nullptr, nullptr},
        {"デモ用舵", "MarioFaceShipRudder", nullptr, nullptr},
    };
};  // namespace

namespace NrvMarioFacePlanetTakeOffDemoObj {
    NEW_NERVE(MarioFacePlanetTakeOffDemoObjNrvWait, MarioFacePlanetTakeOffDemoObj, Wait);
    NEW_NERVE(MarioFacePlanetTakeOffDemoObjNrvDemo, MarioFacePlanetTakeOffDemoObj, Demo);
};  // namespace NrvMarioFacePlanetTakeOffDemoObj

MarioFacePlanetTakeOffDemoObj::MarioFacePlanetTakeOffDemoObj(const char* pName) : LiveActor(pName), mCameraInfo(nullptr), mParts(nullptr), mCount(0) {
}

void MarioFacePlanetTakeOffDemoObj::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "MarioShipTakeOff", false);
    mCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initAnimCamera(this, mCameraInfo, sCameraName);
    MR::tryRegisterDemoCast(this, rIter);
    mCount = 4;
    mParts = new LiveActor*[mCount];
    for (s32 i = 0; i < mCount; i++) {
        mParts[i] = MarioFaceDemoParts::createPart(this, &sParts[i], sPartDemoName, rIter);
    }
    MR::invalidateClipping(this);
    initNerve(GET_NERVE(MarioFacePlanetTakeOffDemoObj, MarioFacePlanetTakeOffDemoObjNrvWait), 0);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void MarioFacePlanetTakeOffDemoObj::exeWait() {
    if (MR::tryStartTimeKeepDemoMarioPuppetable(this, sPartDemoName, nullptr)) {
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
    MR::extractMtxYDir(MR::getJointMtx(this, "MarioShipTakeOff"), &dir);
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
    return &sParts[index];
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
        pActor = MR::createDemoObjWithAnimArc(pInfo->mObjName, pInfo->mArcName, pInfo->mAnimArcName, "Demo", mtx);
    } else {
        pActor = MR::createDemoObj(pInfo->mObjName, pInfo->mArcName, "Demo", mtx);
    }
    pActor->makeActorDead();
    MR::invalidateClipping(pActor);
    MR::tryRegisterDemoCast(pActor, pName, rIter);
    return pActor;
}
