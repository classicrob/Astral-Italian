#include "Game/MapObj/PetitPorter.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Camera/CameraTargetMtx.hpp"
#include "Game/MapObj/CollectTicoHolder.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

// BGM state set after the rescue demo: a zero const the target loads from .sbss2 instead of folding it.
const volatile u32 sRescueBgmState = 0;

namespace NrvPetitPorter {
    NEW_NERVE(PetitPorterNrvAppear, PetitPorter, Appear);
    NEW_NERVE(PetitPorterNrvWaitOn, PetitPorter, Wait);
    NEW_NERVE(PetitPorterNrvWait, PetitPorter, Wait);
    NEW_NERVE(PetitPorterNrvEnterDemo, PetitPorter, EnterDemo);
    NEW_NERVE(PetitPorterNrvEnterDemoEnd, PetitPorter, EnterDemoEnd);
    NEW_NERVE(PetitPorterNrvLeaveDemo, PetitPorter, LeaveDemo);
    NEW_NERVE(PetitPorterNrvLeaveWait, PetitPorter, LeaveWait);
    NEW_NERVE(PetitPorterNrvLeaveFinish, PetitPorter, LeaveFinish);
    NEW_NERVE(PetitPorterNrvLeaveCamera, PetitPorter, LeaveCamera);
    NEW_NERVE(PetitPorterNrvLeaveCameraEnd, PetitPorter, LeaveCameraEnd);

    class PetitPorterNrvEnd : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static PetitPorterNrvEnd sInstance;
    };

    PetitPorterNrvEnd PetitPorterNrvEnd::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvPetitPorter



PetitPorter::PetitPorter(const char* pName) : LiveActor(pName) {
    mExitPoint = nullptr;
    mWarpPoint = nullptr;
    mStepLimit = 0x258;
    mAngle = 0.0f;
    mAngleSpeed = 0.0f;
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
    MR::initActor(this, rIter, "PetitPorter", false);
    MR::getJMapInfoArg1NoInit(rIter, &mStepLimit);
    MR::makeMtxTR(mBaseMtx, this);
    JMath::gekko_ps_copy12(&mMtxD8, &mBaseMtx);
    initNerve(GET_NERVE(PetitPorter, PetitPorterNrvWait), 0);

    MR::createSceneObj(0x6D);
    PetitPorterMediator* pMediator = reinterpret_cast<PetitPorterMediator*>(MR::getSceneObjHolder()->getObj(0x6D));
    pMediator->registerPorter(this, nullptr, nullptr, rIter);

    mCameraTargetMtx = new CameraTargetMtx("\203J\203\201\203\211\203^\201[\203Q\203b\203g\203_\203~\201[");
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "\212\356\223_\202\305\203\217\201[\203v\203C\203\223");
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "\203\217\201[\203v\223_\202\305\203\217\201[\203v\203A\203E\203g");
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "\203\217\201[\203v\223_\202\305\203\217\201[\203v\203C\203\223");
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "\212\356\223_\202\305\203\217\201[\203v\203A\203E\203g");

    // NONMATCHING: the JointControlDelegator<PetitPorter> object (0x28 bytes, built from the ptmf at "")
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
        setNerve(GET_NERVE(PetitPorter, PetitPorterNrvAppear));
    } else {
        setNerve(GET_NERVE(PetitPorter, PetitPorterNrvWait));
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
        if (isNerve(GET_NERVE(PetitPorter, PetitPorterNrvWaitOn)) || isNerve(GET_NERVE(PetitPorter, PetitPorterNrvWait))) {
            bEnter = true;
        }

        if (bEnter) {
            setNerve(GET_NERVE(PetitPorter, PetitPorterNrvEnterDemo));
        } else {
            setNerve(GET_NERVE(PetitPorter, PetitPorterNrvLeaveFinish));
        }
        return true;
    }

    if (MR::isMsgUpdateBaseMtx(msg)) {
        MR::setPlayerBaseMtx(mMtxD8);
        return true;
    }

    return false;
}

bool PetitPorter::calcJointMtx(MtxPtr pMtx) {
    TPos3f tmp;
    PSMTXIdentity(tmp);
    TVec3f rot(0.0f, mAngle, 0.0f);
    MR::makeMtxRotate(tmp, rot);
    PSMTXConcat(pMtx, tmp, pMtx);
    return true;
}

// NONMATCHING: the exe* bodies below are written from the m2c draft and the asm call order; not diffed one by one
// (float clamps in exeWait and the mtx copy loops may differ in scheduling). 
void PetitPorter::exeAppear() {
    if (MR::isFirstStep(this)) {
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
        MR::startAction(this, "Appear");
    }

    if (MR::isGreaterEqualStep(this, 0x5A)) {
        setNerve(GET_NERVE(PetitPorter, PetitPorterNrvWait));
    }
}

void PetitPorter::exeWait() {
    if (isNerve(GET_NERVE(PetitPorter, PetitPorterNrvWaitOn))) {
        f32 t = mAngleSpeed + 0.2f;
        mAngleSpeed = (t >= 3.6f) ? t : 3.6f;
    } else {
        f32 t = mAngleSpeed - 0.2f;
        mAngleSpeed = (t <= 1.2f) ? t : 1.2f;
    }

    f32 sum = mAngle + mAngleSpeed;
    mAngle = 0.0f + (f32)fmod(360.0f + (sum - 0.0f), 360.0);

    if (MR::isFirstStep(this)) {
        if (isNerve(GET_NERVE(PetitPorter, PetitPorterNrvWaitOn))) {
            MR::startAction(this, "WaitActive");
        } else {
            MR::startAction(this, "Wait");
        }
        MR::validateHitSensor(this, "Body");
        MR::invalidateHitSensor(this, "Binder");
    }

    MR::startActionSound(this, "Rotate", -1, -1, -1);

    if (isNerve(GET_NERVE(PetitPorter, PetitPorterNrvWaitOn))) {
        if (MR::isPlayerSkating()) {
            if (MR::isOnPlayer(this)) {
                if (MR::isValidSwitchB(this)) {
                    MR::onSwitchB(this);
                }
                MR::startAction(this, "Spin");
                MR::validateHitSensor(this, "Binder");
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

    if (isNerve(GET_NERVE(PetitPorter, PetitPorterNrvWaitOn))) {
        if (!MR::isOnPlayer(this)) {
            setNerve(GET_NERVE(PetitPorter, PetitPorterNrvWait));
        }
    } else {
        if (MR::isOnPlayer(this)) {
            setNerve(GET_NERVE(PetitPorter, PetitPorterNrvWaitOn));
        }
    }
}

void PetitPorter::exeEnterDemo() {
    if (MR::isFirstStep(this)) {
        MR::changePlayerAnimationAndBva("PetitPorterStart");
        JMath::gekko_ps_copy12(&mCameraTargetMtx->mMatrix, &mMtxD8);
        MR::startMultiActorCameraTargetMtx(this, mCameraInfo, "\212\356\223_\202\305\203\217\201[\203v\203C\203\223", mCameraTargetMtx, -1);
        MR::startActionSound(this, "WarpStart", -1, -1, -1);
        if (MR::isPlayingStageBgm()) {
            mBgmHandle1 = MR::getPlayingStageBgmId();
            mBgmHandle2 = MR::getStageBgmState();
        }
        MR::stopStageBGM(0);
    }

    f32 rate = MR::calcNerveRate(this, 0x14);
    f32 blend = (1.0f >= rate) ? 1.0f : rate;

    TPos3f tmp;
    tmp = mBaseMtx;
    MR::addTransMtxLocalY(tmp, 50.0f);
    MR::blendMtx(mMtx108, tmp, blend, mMtxD8);

    if (MR::isStep(this, 0x1E)) {
        MR::startActionSound(this, "MarioDisappear", -1, -1, -1);
    }

    if (MR::isBckStoppedPlayer()) {
        setNerve(GET_NERVE(PetitPorter, PetitPorterNrvEnterDemoEnd));
    }
}

void PetitPorter::exeEnterDemoEnd() {
    if (MR::isFirstStep(this)) {
        JMath::gekko_ps_copy12(&mMtxD8, mWarpPoint->getBaseMtx());
        JMath::gekko_ps_copy12(&mCameraTargetMtx->mMatrix, &mMtxD8);
        MR::startMultiActorCameraTargetMtx(this, mCameraInfo, "\203\217\201[\203v\223_\202\305\203\217\201[\203v\203A\203E\203g", mCameraTargetMtx, -1);
        MR::hidePlayerTryDeleteEffect();
    }

    if (MR::isCameraInterpolatingNearlyEnd()) {
        setNerve(GET_NERVE(PetitPorter, PetitPorterNrvLeaveDemo));
    }
}

void PetitPorter::exeLeaveDemo() {
    if (MR::isFirstStep(this)) {
        MR::showPlayer();
        MR::changePlayerAnimationAndBva("PetitPorterEnd");
        MR::startStageBGM("BGM_SMG2_EV_PORTER", false);
        MR::launchRescuedStrayTico(mMtxD8);
    }

    if (MR::isStep(this, 0x19)) {
        MR::startActionSound(this, "TimerStart", -1, -1, -1);
    }

    if (MR::isBckStoppedPlayer()) {
        MR::endMultiActorCamera(this, mCameraInfo, "\212\356\223_\202\305\203\217\201[\203v\203C\203\223", true, -1);
        MR::endMultiActorCamera(this, mCameraInfo, "\203\217\201[\203v\223_\202\305\203\217\201[\203v\203A\203E\203g", true, -1);
        mBindTarget = nullptr;
        MR::endBindAndPlayerWait(this);
        setNerve(GET_NERVE(PetitPorter, PetitPorterNrvLeaveWait));
    }
}

void PetitPorter::exeLeaveWait() {
    if (MR::isFirstStep(this)) {
        MR::emitEffectPlayer("\203v\203`\203|\201[\203^\201[");
    }

    if (!MR::isPlayerDead()) {
        MR::adjustPorterBGGM(getNerveStep());
        if (MR::isStep(this, mStepLimit - 0x33)) {
            MR::startActionSound(this, "TimerNearlyUp", -1, -1, -1);
        } else if (MR::isGreaterEqualStep(this, mStepLimit - 1)) {
            if (MR::isValidSwitchB(this)) {
                MR::offSwitchB(this);
            }
            MR::validateHitSensor(this, "Binder");
            MR::startActionSound(this, "TimerEnd", -1, -1, -1);
            MR::deleteEffectPlayer("\203v\203`\203|\201[\203^\201[");
        }
    }
}

void PetitPorter::exeLeaveCamera() {
    if (MR::isFirstStep(this)) {
        if (mExitPoint != nullptr) {
            JMath::gekko_ps_copy12(&mMtxD8, mExitPoint->getBaseMtx());
        } else {
            JMath::gekko_ps_copy12(&mMtxD8, &mBaseMtx);
            MR::addTransMtxLocalY(mMtxD8, 50.0f);
        }
        JMath::gekko_ps_copy12(&mCameraTargetMtx->mMatrix, &mMtxD8);
        MR::startMultiActorCameraTargetMtx(this, mCameraInfo, "\212\356\223_\202\305\203\217\201[\203v\203A\203E\203g", mCameraTargetMtx, -1);
        if (mBgmHandle1 != 0) {
            // SMG2 binary calls the const char* overload with the stored id (startStageBGM__2MRFPCcb).
            MR::startStageBGM(reinterpret_cast<const char*>(mBgmHandle1), false);
        }
        if (mBgmHandle2 > 0) {
            MR::setStageBGMState(mBgmHandle2, sRescueBgmState);
        }
        MR::hidePlayerTryDeleteEffect();
        MR::launchRescuedStrayTico(mMtxD8);
    }

    if (MR::isCameraInterpolatingNearlyEnd()) {
        setNerve(GET_NERVE(PetitPorter, PetitPorterNrvLeaveCameraEnd));
    }
}

void PetitPorter::exeLeaveCameraEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Return");
        MR::showPlayer();
        MR::changePlayerAnimationAndBva("PetitPorterEnd");
    }

    if (MR::isBckStoppedPlayer()) {
        if (MR::isValidSwitchA(this)) {
            MR::offSwitchA(this);
        }
        MR::endMultiActorCamera(this, mCameraInfo, "\203\217\201[\203v\223_\202\305\203\217\201[\203v\203C\203\223", true, -1);
        MR::endMultiActorCamera(this, mCameraInfo, "\212\356\223_\202\305\203\217\201[\203v\203A\203E\203g", true, -1);
        mBindTarget = nullptr;
        MR::endBindAndPlayerWait(this);
        MR::validateClipping(this);
        setNerve(GET_NERVE(PetitPorter, PetitPorterNrvEnd));
    }
}

void PetitPorter::exeLeaveFinish() {
    if (MR::isFirstStep(this)) {
        MR::changePlayerAnimationAndBva("PetitPorterStart");
        JMath::gekko_ps_copy12(&mCameraTargetMtx->mMatrix, &mMtxD8);
        MR::startMultiActorCameraTargetMtx(this, mCameraInfo, "\203\217\201[\203v\223_\202\305\203\217\201[\203v\203C\203\223", mCameraTargetMtx, -1);
        MR::stopStageBGM(0);
    }

    if (MR::isStep(this, 0x1E)) {
        MR::startActionSound(this, "MarioDisappear", -1, -1, -1);
    }

    if (MR::isBckStoppedPlayer()) {
        setNerve(GET_NERVE(PetitPorter, PetitPorterNrvLeaveCamera));
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
    pMediator->registerPorter(nullptr, nullptr, this, rIter);
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
    pMediator->registerPorter(nullptr, this, nullptr, rIter);
    makeActorDead();
}

MtxPtr PetitPorterExitPoint::getBaseMtx() const {
    return const_cast<TPos3f&>(mBaseMtx);
}

PetitPorterMediator::PetitPorterMediator() : NameObj("\203v\203`\203|\201[\203^\201[\222\207\211\356\216\322") {
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

void PetitPorterMediator::registerPorter(PetitPorter* pPorter, PetitPorterExitPoint* pExit, PetitPorterWarpPoint* pWarp, const JMapInfoIter& rIter) {
    s32 id = -1;
    MR::getJMapInfoArg0NoInit(rIter, &id);

    for (s32 i = 0; i < mCount; i++) {
        PetitPorterLink* pLink = &mLinks[i];
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

    PetitPorterLink* pNew = &mLinks[mCount];
    pNew->mPorter = pPorter;
    pNew->mExitPoint = pExit;
    pNew->mWarpPoint = pWarp;
    pNew->mId = id;
    mCount++;
}

bool PetitPorter::isDemo() const {
    bool bBusy = false;
    if (isNerve(GET_NERVE(PetitPorter, PetitPorterNrvEnterDemo)) || isNerve(GET_NERVE(PetitPorter, PetitPorterNrvEnterDemoEnd)) ||
        isNerve(GET_NERVE(PetitPorter, PetitPorterNrvLeaveDemo)) || isNerve(GET_NERVE(PetitPorter, PetitPorterNrvLeaveFinish)) ||
        isNerve(GET_NERVE(PetitPorter, PetitPorterNrvLeaveCamera)) || isNerve(GET_NERVE(PetitPorter, PetitPorterNrvLeaveCameraEnd))) {
        bBusy = true;
    }
    return bBusy;
}

bool PetitPorterMediator::isAnyPorterDemo() const {
    for (s32 i = 0; i < mCount; i++) {
        if (mLinks[i].mPorter->isDemo()) {
            return true;
        }
    }
    return false;
}

bool PetitPorterFunction::isDemo() {
    if (MR::isExistSceneObj(0x6D)) {
        return reinterpret_cast<PetitPorterMediator*>(MR::getSceneObjHolder()->getObj(0x6D))->isAnyPorterDemo();
    }
    return false;
}

PetitPorter::~PetitPorter() {}
PetitPorterWarpPoint::~PetitPorterWarpPoint() {}
PetitPorterExitPoint::~PetitPorterExitPoint() {}
PetitPorterMediator::~PetitPorterMediator() {}

// The original translation unit emits this member callback delegator.
template class JointControlDelegator<PetitPorter>;
