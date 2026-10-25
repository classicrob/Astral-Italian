#include "Game/Util/Functor.hpp"
#include "Game/Player/YoshiNest.hpp"
#include "Game/Player/YoshiEgg.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/Util.hpp"
#include "Game/Util/YoshiUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/GameDataFunction.hpp"

#pragma auto_inline off

namespace NrvYoshiEgg {
    class State0 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiEgg*>(spine->mExecutor)->fn_80416530(); }
        static State0 sInstance;
    };
    State0 State0::sInstance ATTRIBUTE_WEAK;
    class State1 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiEgg*>(spine->mExecutor)->fn_804165D0(); }
        static State1 sInstance;
    };
    State1 State1::sInstance ATTRIBUTE_WEAK;
    class State2 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiEgg*>(spine->mExecutor)->fn_80416680(); }
        static State2 sInstance;
    };
    State2 State2::sInstance ATTRIBUTE_WEAK;
    class State3 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiEgg*>(spine->mExecutor)->fn_80416720(); }
        static State3 sInstance;
    };
    State3 State3::sInstance ATTRIBUTE_WEAK;
    class State4 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiEgg*>(spine->mExecutor)->fn_80416880(); }
        static State4 sInstance;
    };
    State4 State4::sInstance ATTRIBUTE_WEAK;
    class State5 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiEgg*>(spine->mExecutor)->fn_80416880(); }
        static State5 sInstance;
    };
    State5 State5::sInstance ATTRIBUTE_WEAK;
    class State6 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiEgg*>(spine->mExecutor)->fn_80416970(); }
        static State6 sInstance;
    };
    State6 State6::sInstance ATTRIBUTE_WEAK;
    class State7 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiEgg*>(spine->mExecutor)->fn_80416A00(); }
        static State7 sInstance;
    };
    State7 State7::sInstance ATTRIBUTE_WEAK;
    class State8 : public Nerve {
    public:
        virtual void execute(Spine* spine) const;
        static State8 sInstance;
    };
    State8 State8::sInstance ATTRIBUTE_WEAK;
    class State9 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiEgg*>(spine->mExecutor)->fn_80416B30(); }
        static State9 sInstance;
    };
    State9 State9::sInstance ATTRIBUTE_WEAK;
    class State10 : public Nerve {
    public:
        virtual void execute(Spine* spine) const;
        static State10 sInstance;
    };
    State10 State10::sInstance ATTRIBUTE_WEAK;
    class State11 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiEgg*>(spine->mExecutor)->fn_80416C30(); }
        static State11 sInstance;
    };
    State11 State11::sInstance ATTRIBUTE_WEAK;
    inline void State8::execute(Spine* spine) const {
        YoshiEgg* egg = static_cast<YoshiEgg*>(spine->mExecutor);
        if (MR::isActionEnd(egg)) {
            MR::fn_80422480(egg);
            egg->setNerve(&State0::sInstance);
        }
    }
    inline void State10::execute(Spine* spine) const {
        YoshiEgg* egg = static_cast<YoshiEgg*>(spine->mExecutor);
        if (MR::isActionEnd(egg)) {
            MR::fn_80422480(egg);
            egg->setNerve(&State0::sInstance);
        }
    }
}

YoshiNest::YoshiNest(const char* name, const char* resource, MtxPtr matrix, s32 a, s32 b, s32 c, bool d) : ModelObj(name, resource, matrix, a, b, c, d) {
    mConnector = nullptr;
    mConnected = false;
    mConnector = new MapObjConnector(this);
}
YoshiNest::~YoshiNest() {}
void YoshiNest::calcAnim() {
    if (!MR::isNoCalcAnim(this)) {
        if (mConnected) mConnector->connect();
        LiveActor::calcAnim();
    }
}

YoshiEgg::YoshiEgg(const char* name) : LiveActor(name) {
    mNest = nullptr;
    mShell = nullptr;
    mInitialPosition.x = 0.0f;
    mInitialPosition.y = 0.0f;
    mInitialPosition.z = 0.0f;
    mInitialRotation.x = 0.0f;
    mInitialRotation.y = 0.0f;
    mInitialRotation.z = 0.0f;
    mSwitchEnabled = true;
    mCameraInfo = nullptr;
    mCameraActive = false;
    mCameraFrames = 0;
    mShouldAppear = true;
}
YoshiEgg::~YoshiEgg() {}

void YoshiEgg::init(const JMapInfoIter& iter) {
    MR::initDefaultPos(this, iter);
    MR::initActor(this, iter, "YoshiEgg", false);
    initHitSensor(1);
    MR::addHitSensorMapObj(this, "Body", 32, 70.0f, TVec3f(0.0f, 70.0f, 0.0f));
    MR::connectToSceneNoShadowedMapObj(this);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
    MR::onCalcGravity(this);
    MR::calcGravity(this);
    initEffectKeeper(0, nullptr, false);
    initSound(4, nullptr, nullptr, TVec3f(0.0f));
    initNerve(&NrvYoshiEgg::State1::sInstance, 0);
    MR::initStarPointerTarget(this, 100.0f, TVec3f(0.0f, 50.0f, 0.0f));
    MR::initActorCamera(this, iter, &mCameraInfo);
    mNest = new YoshiNest("ヨッシーの巣", "YoshiNest", nullptr, -2, -2, -2, false);
    MR::setPosition(mNest, mPosition);
    MR::setRotation(mNest, mRotation);
    MR::setScale(mNest, mScale);
    mNest->initWithoutIter();
    mShell = new ModelObj("ヨッシーたまごの殻", "YoshiEggBreak", getBaseMtx(), -2, -2, -2, false);
    mShell->initWithoutIter();
    mInitialPosition.set(mPosition);
    mInitialRotation.set(mRotation);
    if (MR::tryRegisterDemoCast(this, iter)) {
        MR::tryRegisterDemoActionFunctor(this, MR::Functor(this, &YoshiEgg::meetYoshiDemo), nullptr);
        MR::fn_804223C0(iter);
        MR::tryRegisterDemoCast(mShell, iter);
    }
    s32 argument = 0;
    MR::getJMapInfoArg0NoInit(iter, &argument);
    if (MR::initUseStageSwitchReadAppear(this, iter)) {
        mNest->makeActorDead();
        mShell->makeActorDead();
        LiveActor::makeActorDead();
        MR::listenStageSwitchOnAppear(this, MR::Functor(this, &YoshiEgg::fn_80416DB0));
        mSwitchEnabled = false;
    } else if (argument == 1) {
        mNest->makeActorAppeared();
        mShell->makeActorDead();
        LiveActor::makeActorDead();
    } else {
        mNest->makeActorAppeared();
        mShell->makeActorDead();
        LiveActor::makeActorAppeared();
    }
    MR::initUseStageSwitchReadA(this, iter);
    MR::useStageSwitchAwake(this, iter);
    MR::registerMarioFaceShipNpc(this, iter);
}

void YoshiEgg::initAfterPlacement() {
    MR::addYoshiEgg(this);
    if (MR::isExistActorCamera(mCameraInfo)) {
        mCameraFrames = MR::getActorCameraFrames(this, mCameraInfo);
        if (mCameraFrames == 0) mCameraFrames = 120;
    }
    mNest->mConnector->attachToUnder();
    mNest->mConnected = true;
}

void YoshiEgg::fn_80416530() {
    if (MR::isFirstStep(this)) {
        mShell->makeActorDead();
        MR::showModel(this);
        MR::startAction(this, "Appear");
    }
    if (MR::isActionEnd(this) && fn_80417450()) {
        setNerve(&NrvYoshiEgg::State1::sInstance);
        fn_80417470();
    }
}
void YoshiEgg::fn_804165D0() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::validateClipping(mShell);
        MR::startAction(this, "Wait");
    }
    if (PSVECDistance(MR::getPlayerPos(), &mPosition) < 500.0f) {
        setNerve(&NrvYoshiEgg::State2::sInstance);
        return;
    }
    if (!MR::isValidSwitchA(this) || MR::isOnSwitchA(this)) MR::isStarPointerPointing2PAttachTarget(this);
}
void YoshiEgg::fn_80416680() {
    if (MR::isFirstStep(this)) MR::startAction(this, "WaitNear");
    if (PSVECDistance(MR::getPlayerPos(), &mPosition) > 800.0f) {
        setNerve(&NrvYoshiEgg::State1::sInstance);
        return;
    }
    if (!MR::isValidSwitchA(this) || MR::isOnSwitchA(this)) MR::isStarPointerPointing2PAttachTarget(this);
}
void YoshiEgg::fn_80416720() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
        MR::startAction(this, "NoSound");
        mShell->makeActorAppeared();
        MR::startAction(mShell, "Break");
        MR::fn_80422790(getBaseMtx(), this);
        MR::fn_80422440(this);
        if (MR::isOnGameEventFlagYoshiFirst() || !MR::isDemoCast(this, nullptr)) MR::startActionSound(mShell, "YoshiAppear", -1, -1, -1);
    }
    if (!MR::isOnGameEventFlagYoshiFirst() && MR::isDemoCast(this, nullptr) && MR::isStep(this, 5)) {
        MR::requestStartDemoRegisteredMarioPuppetable(this, &NrvYoshiEgg::State11::sInstance, &NrvYoshiEgg::State11::sInstance, nullptr);
        return;
    }
    if (MR::isActionEnd(mShell)) {
        kill();
        mShell->makeActorDead();
    }
}
void YoshiEgg::fn_80416880() {
    if (MR::isFirstStep(this)) {
        mShell->makeActorAppeared();
        if (isNerve(&NrvYoshiEgg::State5::sInstance)) MR::startAction(mShell, "DisappearDamage");
        else MR::startAction(mShell, "Disappear");
    }
    fn_80417370();
    calcAnmMtx();
    if (MR::isActionEnd(mShell)) {
        MR::showModel(this);
        if (isNerve(&NrvYoshiEgg::State5::sInstance)) MR::startAction(this, "DisappearDamage");
        else MR::startAction(this, "Disappear");
        setNerve(&NrvYoshiEgg::State6::sInstance);
    }
}
void YoshiEgg::fn_80416970() {
    if (MR::isFirstStep(this)) mShell->makeActorDead();
    fn_80417370();
    if (MR::isActionEnd(this)) {
        MR::fn_80422480(this);
        setNerve(&NrvYoshiEgg::State0::sInstance);
        MR::fn_80008560(this, "Disappear");
    }
}
void YoshiEgg::fn_80416A00() {
    if (MR::isFirstStep(this)) {
        MR::hideModelAndOnCalcAnim(this);
        mShell->makeActorAppeared();
        MR::startAction(mShell, "DisappearFire");
    }
    fn_80417370();
    f32 end = MR::getBckFrameMax(mShell);
    f32 rate = MR::getBckRate(mShell);
    f32 frame = MR::getBckFrame(mShell);
    if (frame + rate >= end) MR::showModel(this);
    if (MR::isActionEnd(mShell)) {
        mShell->makeActorDead();
        MR::copyJointPos(mShell, "YoshiEggBreakRoot", &mPosition);
        MR::startAction(this, "DisappearFire");
        setNerve(&NrvYoshiEgg::State8::sInstance);
        MR::fn_80008560(this, "DisappearFire");
    }
}
void YoshiEgg::fn_80416B30() {
    if (MR::isFirstStep(this)) {
        MR::hideModelAndOnCalcAnim(this);
        mShell->makeActorAppeared();
        MR::startAction(mShell, "DisappearElectric");
    }
    fn_80417370();
    f32 end = MR::getBckFrameMax(mShell);
    f32 rate = MR::getBckRate(mShell);
    f32 frame = MR::getBckFrame(mShell);
    if (frame + rate >= end) MR::showModel(this);
    if (MR::isActionEnd(mShell)) {
        mShell->makeActorDead();
        MR::startAction(this, "DisappearDamage");
        setNerve(&NrvYoshiEgg::State8::sInstance);
    }
}
void YoshiEgg::fn_80416C30() {
    if (MR::isStep(this, 0)) MR::startSubBGM("BGM_SMG2_JG_MEETYOSHI", false);
    if (MR::isDemoActive("ヨッシー出会いデモ") && (MR::isDemoPartActive("たまご割れる") || MR::isDemoPartActive("インフォメーション"))) MR::requestStageBGMVolumeDown(1, false);
}
void YoshiEgg::makeActorAppeared() {
    mShouldAppear = true;
    if (mSwitchEnabled) {
        mNest->makeActorAppeared();
        if (MR::fn_804224C0()) LiveActor::makeActorAppeared();
    }
}
void YoshiEgg::fn_80416D30() {
    if (!mShouldAppear || !mSwitchEnabled) return;
    LiveActor::makeActorAppeared();
    MR::invalidateClipping(this);
    MR::invalidateClipping(mShell);
    fn_80417370();
    calcAnmMtx();
    setNerve(&NrvYoshiEgg::State4::sInstance);
}
void YoshiEgg::fn_80416DB0() {
    if (!mSwitchEnabled && MR::isExistActorCamera(mCameraInfo)) fn_804173F0();
    mSwitchEnabled = true;
    if (mShouldAppear) {
        mNest->makeActorAppeared();
        if (MR::fn_804224C0()) {
            LiveActor::makeActorAppeared();
            MR::hideModel(this);
            MR::invalidateClipping(this);
            setNerve(&NrvYoshiEgg::State0::sInstance);
        }
    }
}
void YoshiEgg::fn_80416E60() {
    if (!mShouldAppear || !mSwitchEnabled) return;
    LiveActor::makeActorAppeared();
    MR::invalidateClipping(this);
    MR::invalidateClipping(mShell);
    fn_80417370();
    calcAnmMtx();
    setNerve(&NrvYoshiEgg::State7::sInstance);
}
void YoshiEgg::fn_80416EE0() {
    if (!mShouldAppear || !mSwitchEnabled) return;
    LiveActor::makeActorAppeared();
    MR::invalidateClipping(this);
    MR::invalidateClipping(mShell);
    fn_80417370();
    calcAnmMtx();
    setNerve(&NrvYoshiEgg::State9::sInstance);
}
void YoshiEgg::fn_80416F60() {
    if (mSwitchEnabled && mShouldAppear) {
        LiveActor::makeActorAppeared();
        MR::hideModel(this);
        MR::invalidateClipping(this);
        setNerve(&NrvYoshiEgg::State0::sInstance);
    }
}
void YoshiEgg::fn_80416FC0() { LiveActor::makeActorDead(); }
void YoshiEgg::makeActorDead() {
    mShouldAppear = false;
    mNest->makeActorDead();
    LiveActor::makeActorDead();
}
void YoshiEgg::kill() { LiveActor::makeActorDead(); }
void YoshiEgg::appear() {}
void YoshiEgg::fn_80417040() { MR::enablePlayerResourceYoshi(); }
void YoshiEgg::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (MR::isSensorPlayerOrRide(receiver) && !MR::isHiddenModel(this) && !fn_804172C0()) MR::sendMsgPush(receiver, sender);
}
bool YoshiEgg::receiveMsgPlayerAttack(u32 message, HitSensor*, HitSensor*) {
    if ((isNerve(&NrvYoshiEgg::State2::sInstance) || isNerve(&NrvYoshiEgg::State1::sInstance)) &&
        (!MR::isValidSwitchA(this) || MR::isOnSwitchA(this)) &&
        (MR::isMsgPlayerSpinAttackOrSupportTico(message) || MR::isMsgPlayerTrample(message) || MR::isMsgPlayerHipDrop(message))) {
        setNerve(&NrvYoshiEgg::State3::sInstance);
        return true;
    }
    if (MR::isMsgRockAttack(message) && (isNerve(&NrvYoshiEgg::State1::sInstance) || isNerve(&NrvYoshiEgg::State2::sInstance))) {
        setNerve(&NrvYoshiEgg::State3::sInstance);
        return true;
    }
    if (MR::isMsgStarPieceReflect(message)) return true;
    return false;
}
void YoshiEgg::calcAndSetBaseMtx() {
    bool follow = false;
    if (isNerve(&NrvYoshiEgg::State0::sInstance) || isNerve(&NrvYoshiEgg::State1::sInstance) || isNerve(&NrvYoshiEgg::State2::sInstance) || isNerve(&NrvYoshiEgg::State3::sInstance)) follow = true;
    if (follow) {
        if (mNest->mConnected) mNest->mConnector->connect(this);
        MR::setRotation(this, mNest->mRotation);
    }
    LiveActor::calcAndSetBaseMtx();
}
bool YoshiEgg::fn_804172C0() const {
    bool inactive = false;
    if (isNerve(&NrvYoshiEgg::State0::sInstance) || isNerve(&NrvYoshiEgg::State3::sInstance) || isNerve(&NrvYoshiEgg::State4::sInstance) || isNerve(&NrvYoshiEgg::State6::sInstance) || isNerve(&NrvYoshiEgg::State7::sInstance) || isNerve(&NrvYoshiEgg::State9::sInstance)) inactive = true;
    return inactive;
}
void YoshiEgg::fn_80417370() {
    MR::setPosition(this, MR::getYoshiPosition());
    TVec3f rotation(MR::getYoshiRotation());
    rotation.scale(57.2957763671875f);
    MR::setRotation(this, rotation);
}
void YoshiEgg::fn_804173F0() {
    if (MR::tryStartDemoWithoutCinemaFrame(this, "出現")) {
        MR::startActorCameraTargetSelf(this, mCameraInfo, -1);
        MR::requestMovementOn(this);
        MR::pauseOffCameraDirector();
        mCameraActive = true;
    }
}
bool YoshiEgg::fn_80417450() const {
    if (mCameraActive) return MR::isGreaterStep(this, mCameraFrames);
    return true;
}
void YoshiEgg::fn_80417470() {
    if (mCameraActive) {
        MR::endActorCamera(this, mCameraInfo, false, -1);
        MR::endDemoWaitCameraInterpolating(this, "出現");
        mCameraActive = false;
    }
}
void YoshiEgg::meetYoshiDemo() { MR::explainYoshiIfAtFirst(this); }
