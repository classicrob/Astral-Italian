#include "Game/MapObj/PetitPorter.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/JointController.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

extern "C" char lbl_806A7900[];
extern "C" char lbl_806A7944[];
extern "C" char lbl_806A7974[];
extern "C" char lbl_806A7990[];
extern "C" char lbl_806A7A60[];
extern "C" u32 lbl_807D4078;
extern "C" u32 lbl_807D407C;
extern "C" u32 lbl_807D4080;
extern "C" u32 lbl_807D4084;
extern "C" u32 lbl_807D4088;
extern "C" u32 lbl_807D4090;
extern "C" u32 lbl_807D408C;
extern "C" u32 lbl_807D4094;
extern "C" u32 lbl_807D4098;
extern "C" u32 lbl_807D409C;
extern "C" u32 lbl_807D40A0;
extern "C" f32 lbl_807DDF00;
extern "C" f32 lbl_807DDF04;
extern "C" f32 lbl_807DDF08;
extern "C" f32 lbl_807DDF0C;
extern "C" f32 lbl_807DDF10;
extern "C" f32 lbl_807DDF14;
extern "C" f32 lbl_807DDF18;
extern "C" f64 lbl_80649618;
extern "C" s32 lbl_807E30B0;

// Helpers not ported yet (unnamed SMG2 symbols)
extern "C" void fn_8004F640(void*);
extern "C" bool fn_8004FFD0();
extern "C" s32 fn_8001B350();
extern "C" s32 fn_8001B3A0();
extern "C" void fn_802D9800(PetitPorterMediator*, PetitPorter*, PetitPorterExitPoint*, PetitPorterWarpPoint*, const JMapInfoIter&);
extern "C" bool fn_802D9510(LiveActor*);

#define PP_NERVE(n) reinterpret_cast<const Nerve*>(&n)

// CameraTargetMtx is only forward-declared in the shared headers; its ctor and 0x94 size are used here.
class CameraTargetMtx {
public:
    CameraTargetMtx(const char*);

    u8 _0[0x94];
};

namespace JMath {
    void gekko_ps_copy12(void*, const void*);
};  // namespace JMath

namespace MR {
    void adjustPorterBGGM(s32);
    void launchRescuedStrayTico(const TPos3f&);
};  // namespace MR

static void* camTargetPos(CameraTargetMtx* pTarget) {
    return reinterpret_cast<u8*>(pTarget) + 0x14;
}

PetitPorter::PetitPorter(const char* pName) : LiveActor(pName) {
    mExitPoint = nullptr;
    mWarpPoint = nullptr;
    mStepLimit = 0x258;
    mAngle = lbl_807DDF04;
    mAngleSpeed = lbl_807DDF04;
    mBindTarget = nullptr;
    mBaseMtx.identity();
    mMtx108.identity();
    mMtxD8.identity();
    mCameraInfo = nullptr;
    mCameraTargetMtx = nullptr;
    mJointController = nullptr;
    mBgmHandle1 = 0;
    mBgmHandle2 = -1;
}

void PetitPorter::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, lbl_806A7900 + 0xC, false);
    MR::getJMapInfoArg1NoInit(rIter, &mStepLimit);
    MR::makeMtxTR(mBaseMtx, this);
    JMath::gekko_ps_copy12(&mMtxD8, &mBaseMtx);
    initNerve(PP_NERVE(lbl_807D4080), 0);

    MR::createSceneObj(0x6D);
    PetitPorterMediator* pMediator = reinterpret_cast<PetitPorterMediator*>(MR::getSceneObjHolder()->getObj(0x6D));
    fn_802D9800(pMediator, this, nullptr, nullptr, rIter);

    mCameraTargetMtx = new CameraTargetMtx(lbl_806A7900 + 0x18);
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, lbl_806A7900 + 0x30);
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, lbl_806A7900 + 0x44);
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, lbl_806A7900 + 0x5C);
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, lbl_806A7900 + 0x74);

    // NONMATCHING: the JointControlDelegator<PetitPorter> object (0x28 bytes, built from the ptmf at lbl_806A7900)
    // is not ported yet, so mJointController stays null here and setJointControllerParam is skipped.

    MR::initUseStageSwitchWriteA(this, rIter);
    MR::initUseStageSwitchWriteB(this, rIter);
    if (!MR::useStageSwitchSyncAppear(this, rIter)) {
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
    }
}

void PetitPorter::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::showModel(this);
    if (MR::isValidSwitchAppear(this)) {
        setNerve(PP_NERVE(lbl_807D4078));
    } else {
        setNerve(PP_NERVE(lbl_807D4080));
    }
}

void PetitPorter::calcAndSetBaseMtx() {
    mJointController->registerCallBack();
    LiveActor::calcAndSetBaseMtx();
}

void PetitPorter::updateHitSensor(HitSensor* pSensor) {
    MR::setSensorPos(pSensor, *MR::getPlayerPos());
}

bool PetitPorter::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgAutoRushBegin(msg) && mBindTarget == nullptr) {
        mBindTarget = MR::getSensorHost(pReceiver);
        JMath::gekko_ps_copy12(&mMtxD8, MR::getPlayerBaseMtx());
        JMath::gekko_ps_copy12(&mMtx108, &mMtxD8);

        bool bEnter = false;
        if (isNerve(PP_NERVE(lbl_807D407C)) || isNerve(PP_NERVE(lbl_807D4080))) {
            bEnter = true;
        }

        if (bEnter) {
            setNerve(PP_NERVE(lbl_807D4084));
        } else {
            setNerve(PP_NERVE(lbl_807D4094));
        }
        return true;
    }

    if (MR::isMsgUpdateBaseMtx(msg)) {
        MR::setPlayerBaseMtx(mMtxD8);
        return true;
    }

    return false;
}

// Nerve Wait-like body of PetitPorter (fn_802D89D0 is the matrix helper used by the state loop)
extern "C" s32 fn_802D89D0(PetitPorter* pPorter, MtxPtr pMtx) {
    TPos3f tmp;
    PSMTXIdentity(tmp);
    TVec3f rot(0.0f, pPorter->mAngle, 0.0f);
    MR::makeMtxRotate(tmp, rot);
    PSMTXConcat(pMtx, tmp, pMtx);
    return 1;
}

// NONMATCHING: the exe* bodies below are written from the m2c draft and the asm call order; not diffed one by one
// (float clamps in exeWait and the mtx copy loops may differ in scheduling). fn_802D8E80 and fn_802D91E0 match by namefn.
void PetitPorter::exeAppear() {
    if (MR::isFirstStep(this)) {
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
        MR::startAction(this, lbl_806A7990);
    }

    if (MR::isGreaterEqualStep(this, 0x5A)) {
        setNerve(PP_NERVE(lbl_807D4080));
    }
}

void PetitPorter::exeWait() {
    if (isNerve(PP_NERVE(lbl_807D407C))) {
        f32 t = mAngleSpeed + lbl_807DDF14;
        mAngleSpeed = (t >= lbl_807DDF0C) ? t : lbl_807DDF0C;
    } else {
        f32 t = mAngleSpeed - lbl_807DDF14;
        mAngleSpeed = (t <= lbl_807DDF10) ? t : lbl_807DDF10;
    }

    f32 sum = mAngle + mAngleSpeed;
    mAngle = lbl_807DDF04 + (f32)fmod(lbl_807DDF18 + (sum - lbl_807DDF04), lbl_80649618);

    if (MR::isFirstStep(this)) {
        if (isNerve(PP_NERVE(lbl_807D407C))) {
            MR::startAction(this, lbl_806A7900 + 0x98);
        } else {
            MR::startAction(this, lbl_806A7900 + 0xA4);
        }
        MR::validateHitSensor(this, lbl_806A7900 + 0xAC);
        MR::invalidateHitSensor(this, lbl_806A7900 + 0xB4);
    }

    MR::startActionSound(this, lbl_806A7900 + 0xBC, -1, -1, -1);

    if (isNerve(PP_NERVE(lbl_807D407C))) {
        if (fn_8004FFD0()) {
            if (MR::isOnPlayer(this)) {
                if (MR::isValidSwitchB(this)) {
                    MR::onSwitchB(this);
                }
                MR::startAction(this, lbl_806A7900 + 0xC4);
                MR::validateHitSensor(this, lbl_806A7900 + 0xB4);
                return;
            }
        }
    }

    if (MR::isPlayerCarrySensorType(0)) {
        if (MR::isOnPlayer(this)) {
            if (MR::isPadSwing(0)) {
                MR::tryPlayerDropTakingActor();
                return;
            }
        }
    }

    if (isNerve(PP_NERVE(lbl_807D407C))) {
        if (!MR::isOnPlayer(this)) {
            setNerve(PP_NERVE(lbl_807D4080));
        }
    } else {
        if (MR::isOnPlayer(this)) {
            setNerve(PP_NERVE(lbl_807D407C));
        }
    }
}

void PetitPorter::exeEnterDemo() {
    if (MR::isFirstStep(this)) {
        fn_8004F640(lbl_806A7900 + 0xCC);
        JMath::gekko_ps_copy12(camTargetPos(mCameraTargetMtx), &mMtxD8);
        MR::startMultiActorCameraTargetMtx(this, mCameraInfo, lbl_806A7900 + 0x30, mCameraTargetMtx, -1);
        MR::startActionSound(this, lbl_806A7900 + 0xE0, -1, -1, -1);
        if (MR::isPlayingStageBgm()) {
            mBgmHandle1 = fn_8001B350();
            mBgmHandle2 = fn_8001B3A0();
        }
        MR::stopStageBGM(0);
    }

    f32 rate = MR::calcNerveRate(this, 0x14);
    f32 blend = (lbl_807DDF00 >= rate) ? lbl_807DDF00 : rate;

    TPos3f tmp;
    tmp = mBaseMtx;
    MR::addTransMtxLocalY(tmp, lbl_807DDF08);
    MR::blendMtx(mMtx108, tmp, blend, mMtxD8);

    if (MR::isStep(this, 0x1E)) {
        MR::startActionSound(this, lbl_806A7900 + 0xEC, -1, -1, -1);
    }

    if (MR::isBckStoppedPlayer()) {
        setNerve(PP_NERVE(lbl_807D4088));
    }
}

void PetitPorter::exeEnterDemoEnd() {
    if (MR::isFirstStep(this)) {
        JMath::gekko_ps_copy12(&mMtxD8, mWarpPoint->getBaseMtx());
        JMath::gekko_ps_copy12(camTargetPos(mCameraTargetMtx), &mMtxD8);
        MR::startMultiActorCameraTargetMtx(this, mCameraInfo, lbl_806A7944, mCameraTargetMtx, -1);
        MR::hidePlayerTryDeleteEffect();
    }

    if (MR::isCameraInterpolatingNearlyEnd()) {
        setNerve(PP_NERVE(lbl_807D408C));
    }
}

void PetitPorter::exeLeaveDemo() {
    if (MR::isFirstStep(this)) {
        MR::showPlayer();
        fn_8004F640(lbl_806A7900 + 0xFC);
        MR::startStageBGM(lbl_806A7900 + 0x10C, false);
        MR::launchRescuedStrayTico(mMtxD8);
    }

    if (MR::isStep(this, 0x19)) {
        MR::startActionSound(this, lbl_806A7900 + 0x120, -1, -1, -1);
    }

    if (MR::isBckStoppedPlayer()) {
        MR::endMultiActorCamera(this, mCameraInfo, lbl_806A7900 + 0x30, true, -1);
        MR::endMultiActorCamera(this, mCameraInfo, lbl_806A7900 + 0x44, true, -1);
        mBindTarget = nullptr;
        MR::endBindAndPlayerWait(this);
        setNerve(PP_NERVE(lbl_807D4090));
    }
}

void PetitPorter::exeLeaveWait() {
    if (MR::isFirstStep(this)) {
        MR::emitEffectPlayer(lbl_806A7900 + 0x12C);
    }

    if (!MR::isPlayerDead()) {
        MR::adjustPorterBGGM(getNerveStep());
        if (MR::isStep(this, mStepLimit - 0x33)) {
            MR::startActionSound(this, lbl_806A7900 + 0x13C, -1, -1, -1);
        } else if (MR::isGreaterEqualStep(this, mStepLimit - 1)) {
            if (MR::isValidSwitchB(this)) {
                MR::offSwitchB(this);
            }
            MR::validateHitSensor(this, lbl_806A7900 + 0xB4);
            MR::startActionSound(this, lbl_806A7900 + 0x14C, -1, -1, -1);
            MR::deleteEffectPlayer(lbl_806A7900 + 0x12C);
        }
    }
}

void PetitPorter::exeLeaveCamera() {
    if (MR::isFirstStep(this)) {
        if (mExitPoint != nullptr) {
            JMath::gekko_ps_copy12(&mMtxD8, mExitPoint->getBaseMtx());
        } else {
            JMath::gekko_ps_copy12(&mMtxD8, &mBaseMtx);
            MR::addTransMtxLocalY(mMtxD8, lbl_807DDF08);
        }
        JMath::gekko_ps_copy12(camTargetPos(mCameraTargetMtx), &mMtxD8);
        MR::startMultiActorCameraTargetMtx(this, mCameraInfo, lbl_806A7974, mCameraTargetMtx, -1);
        if (mBgmHandle1 != 0) {
            MR::startStageBGM(mBgmHandle1, false);
        }
        if (mBgmHandle2 > 0) {
            MR::setStageBGMState(mBgmHandle2, lbl_807E30B0);
        }
        MR::hidePlayerTryDeleteEffect();
        MR::launchRescuedStrayTico(mMtxD8);
    }

    if (MR::isCameraInterpolatingNearlyEnd()) {
        setNerve(PP_NERVE(lbl_807D409C));
    }
}

void PetitPorter::exeLeaveCameraEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, lbl_806A7900 + 0x158);
        MR::showPlayer();
        fn_8004F640(lbl_806A7900 + 0xFC);
    }

    if (MR::isBckStoppedPlayer()) {
        if (MR::isValidSwitchA(this)) {
            MR::offSwitchA(this);
        }
        MR::endMultiActorCamera(this, mCameraInfo, lbl_806A7900 + 0x5C, true, -1);
        MR::endMultiActorCamera(this, mCameraInfo, lbl_806A7900 + 0x74, true, -1);
        mBindTarget = nullptr;
        MR::endBindAndPlayerWait(this);
        MR::validateClipping(this);
        setNerve(PP_NERVE(lbl_807D40A0));
    }
}

void PetitPorter::exeLeaveFinish() {
    if (MR::isFirstStep(this)) {
        fn_8004F640(lbl_806A7900 + 0xCC);
        JMath::gekko_ps_copy12(camTargetPos(mCameraTargetMtx), &mMtxD8);
        MR::startMultiActorCameraTargetMtx(this, mCameraInfo, lbl_806A7900 + 0x5C, mCameraTargetMtx, -1);
        MR::stopStageBGM(0);
    }

    if (MR::isStep(this, 0x1E)) {
        MR::startActionSound(this, lbl_806A7900 + 0xEC, -1, -1, -1);
    }

    if (MR::isBckStoppedPlayer()) {
        setNerve(PP_NERVE(lbl_807D4098));
    }
}

PetitPorterWarpPoint::PetitPorterWarpPoint(const char* pName) : LiveActor(pName) {
    mBaseMtx.identity();
}

void PetitPorterWarpPoint::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::makeMtxTR(mBaseMtx, this);
    MR::createSceneObj(0x6D);
    PetitPorterMediator* pMediator = reinterpret_cast<PetitPorterMediator*>(MR::getSceneObjHolder()->getObj(0x6D));
    fn_802D9800(pMediator, nullptr, nullptr, this, rIter);
    makeActorDead();
}

MtxPtr PetitPorterWarpPoint::getBaseMtx() const {
    return const_cast<TPos3f&>(mBaseMtx);
}

PetitPorterExitPoint::PetitPorterExitPoint(const char* pName) : LiveActor(pName) {
    mBaseMtx.identity();
}

void PetitPorterExitPoint::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::makeMtxTR(mBaseMtx, this);
    MR::createSceneObj(0x6D);
    PetitPorterMediator* pMediator = reinterpret_cast<PetitPorterMediator*>(MR::getSceneObjHolder()->getObj(0x6D));
    fn_802D9800(pMediator, nullptr, this, nullptr, rIter);
    makeActorDead();
}

MtxPtr PetitPorterExitPoint::getBaseMtx() const {
    return const_cast<TPos3f&>(mBaseMtx);
}

PetitPorterMediator::PetitPorterMediator() : NameObj(lbl_806A7A60) {
    mCount = 0;
    mLinks = nullptr;
    mLinks = new PetitPorterLink[0x20];
    for (s32 i = 0; i < 0x20; i++) {
        mLinks[i].mPorter = nullptr;
        mLinks[i].mWarpPoint = nullptr;
        mLinks[i].mId = -1;
    }
}

void PetitPorterMediator::initAfterPlacement() {
    for (s32 i = 0; i < mCount; i++) {
        PetitPorterLink* pLink = &mLinks[i];
        if (pLink->mPorter != nullptr && pLink->mWarpPoint != nullptr) {
            if (pLink->mExitPoint != nullptr) {
                pLink->mPorter->mExitPoint = pLink->mExitPoint;
            }
            pLink->mPorter->mWarpPoint = pLink->mWarpPoint;
        }
    }
}

extern "C" void fn_802D9800(PetitPorterMediator* pMediator, PetitPorter* pPorter, PetitPorterExitPoint* pExit,
                            PetitPorterWarpPoint* pWarp, const JMapInfoIter& rIter) {
    s32 id = -1;
    MR::getJMapInfoArg0NoInit(rIter, &id);

    for (s32 i = 0; i < pMediator->mCount; i++) {
        PetitPorterLink* pLink = &pMediator->mLinks[i];
        if (id == pLink->mId) {
            if (pPorter != nullptr) {
                pLink->mPorter = pPorter;
            }
            if (pExit != nullptr) {
                pLink->mExitPoint = pExit;
            }
            if (pWarp != nullptr) {
                pLink->mWarpPoint = pWarp;
            }
            return;
        }
    }

    PetitPorterLink* pNew = &pMediator->mLinks[pMediator->mCount];
    pNew->mPorter = pPorter;
    pNew->mExitPoint = pExit;
    pNew->mWarpPoint = pWarp;
    pNew->mId = id;
    pMediator->mCount++;
}

extern "C" bool fn_802D9510(LiveActor* pPorter) {
    bool bBusy = false;
    if (pPorter->isNerve(PP_NERVE(lbl_807D4084)) || pPorter->isNerve(PP_NERVE(lbl_807D4088)) ||
        pPorter->isNerve(PP_NERVE(lbl_807D408C)) || pPorter->isNerve(PP_NERVE(lbl_807D4094)) ||
        pPorter->isNerve(PP_NERVE(lbl_807D409C))) {
        bBusy = true;
    }
    return bBusy;
}

extern "C" bool fn_802D9940(PetitPorterMediator* pMediator) {
    for (s32 i = 0; i < pMediator->mCount; i++) {
        if (fn_802D9510(pMediator->mLinks[i].mPorter)) {
            return true;
        }
    }
    return false;
}

extern "C" bool fn_802D99B0() {
    if (MR::isExistSceneObj(0x6D)) {
        return fn_802D9940(reinterpret_cast<PetitPorterMediator*>(MR::getSceneObjHolder()->getObj(0x6D)));
    }
    return false;
}
