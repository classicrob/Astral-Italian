#define JGEOMETRY_OUTLINE_MATRIX_ZERO_TRANS
#include "Game/Player/Yoshi.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/Util/TalkUtil.hpp"
#include "Game/Map/WaterInfo.hpp"
#include "Game/Effect/MultiEmitter.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/Animation/BvaPlayer.hpp"
#include "Game/Animation/BtkPlayer.hpp"
#include "Game/Animation/BrkPlayer.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/YoshiUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/XanimeResource.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/LiveActor/EffectKeeper.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JMath/JMath.hpp>
#include <JSystem/J3DGraphAnimator/J3DModel.hpp>
#include <cstring>

extern XanimeGroupInfo yoshiAnimationGroups[];
extern XanimeOfsInfo yoshiAnimationAux[];
extern XanimeBckTable1 yoshiAnimationOffsets[];
extern XanimeBckTable2 yoshiAnimationBckTable[];
extern XanimeBckTable3 yoshiAnimationBckTable2[];
extern XanimeBckTable4 yoshiAnimationBckTable3[];

namespace NrvYoshi {
    class YoshiNrv80413840 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        virtual void executeOnEnd(Spine*) const;
        static YoshiNrv80413840 sInstance;
    };
    YoshiNrv80413840 YoshiNrv80413840::sInstance;

    class YoshiNrv80413850 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413850 sInstance;
    };
    YoshiNrv80413850 YoshiNrv80413850::sInstance;

    class YoshiNrv80413860 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413860 sInstance;
    };
    YoshiNrv80413860 YoshiNrv80413860::sInstance;

    class YoshiNrv80413870 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413870 sInstance;
    };
    YoshiNrv80413870 YoshiNrv80413870::sInstance;

    class YoshiNrv80413880 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413880 sInstance;
    };
    YoshiNrv80413880 YoshiNrv80413880::sInstance;

    class YoshiNrv80413890 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413890 sInstance;
    };
    YoshiNrv80413890 YoshiNrv80413890::sInstance;

    class YoshiNrv804138A0 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv804138A0 sInstance;
    };
    YoshiNrv804138A0 YoshiNrv804138A0::sInstance;

    class YoshiNrv804138B0 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv804138B0 sInstance;
    };
    YoshiNrv804138B0 YoshiNrv804138B0::sInstance;

    class YoshiNrv804138C0 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv804138C0 sInstance;
    };
    YoshiNrv804138C0 YoshiNrv804138C0::sInstance;

    class YoshiNrv804138D0 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv804138D0 sInstance;
    };
    YoshiNrv804138D0 YoshiNrv804138D0::sInstance;

    class YoshiNrv804138E0 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv804138E0 sInstance;
    };
    YoshiNrv804138E0 YoshiNrv804138E0::sInstance;

    class YoshiNrv804138F0 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv804138F0 sInstance;
    };
    YoshiNrv804138F0 YoshiNrv804138F0::sInstance;

    class YoshiNrv80413900 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413900 sInstance;
    };
    YoshiNrv80413900 YoshiNrv80413900::sInstance;

    class YoshiNrv80413910 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413910 sInstance;
    };
    YoshiNrv80413910 YoshiNrv80413910::sInstance;

    class YoshiNrv80413920 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413920 sInstance;
    };
    YoshiNrv80413920 YoshiNrv80413920::sInstance;

    class YoshiNrv80413930 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413930 sInstance;
    };
    YoshiNrv80413930 YoshiNrv80413930::sInstance;

    class YoshiNrv80413940 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413940 sInstance;
    };
    YoshiNrv80413940 YoshiNrv80413940::sInstance;

    class YoshiNrv80413950 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413950 sInstance;
    };
    YoshiNrv80413950 YoshiNrv80413950::sInstance;

    class YoshiNrv80413960 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413960 sInstance;
    };
    YoshiNrv80413960 YoshiNrv80413960::sInstance;

    class YoshiNrv80413970 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413970 sInstance;
    };
    YoshiNrv80413970 YoshiNrv80413970::sInstance;

    class YoshiNrv80413980 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413980 sInstance;
    };
    YoshiNrv80413980 YoshiNrv80413980::sInstance;

    class YoshiNrv80413990 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413990 sInstance;
    };
    YoshiNrv80413990 YoshiNrv80413990::sInstance;

    class YoshiNrv804139A0 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv804139A0 sInstance;
    };
    YoshiNrv804139A0 YoshiNrv804139A0::sInstance;

    class YoshiNrv804139B0 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv804139B0 sInstance;
    };
    YoshiNrv804139B0 YoshiNrv804139B0::sInstance;

    class YoshiNrv804139C0 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv804139C0 sInstance;
    };
    YoshiNrv804139C0 YoshiNrv804139C0::sInstance;

    class YoshiNrv804139D0 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv804139D0 sInstance;
    };
    YoshiNrv804139D0 YoshiNrv804139D0::sInstance;

    class YoshiNrv804139E0 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv804139E0 sInstance;
    };
    YoshiNrv804139E0 YoshiNrv804139E0::sInstance;

    class YoshiNrv804139F0 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv804139F0 sInstance;
    };
    YoshiNrv804139F0 YoshiNrv804139F0::sInstance;

    class YoshiNrv80413A00 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413A00 sInstance;
    };
    YoshiNrv80413A00 YoshiNrv80413A00::sInstance;

    class YoshiNrv80413A10 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static YoshiNrv80413A10 sInstance;
    };
    YoshiNrv80413A10 YoshiNrv80413A10::sInstance;

}

namespace {template<class T> inline const Nerve* getYoshiNerve(){return &T::sInstance;}}


// NONMATCHING: string pool base, local stack layout and register allocation differ.
void Yoshi::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, "Yoshi", false);
    MR::connectToScene(this, 37, 6, 22, -1);
    initHitSensor(6);
    MR::addHitSensorEye(this, "AutoEat", 32, 200.0f, TVec3f(0.0f, 60.0f, 0.0f));
    MR::addHitSensorRide(this, "Ride", 32, 80.0f, TVec3f(0.0f, 10.0f, 0.0f));
    MR::addHitSensorRide(this, "SpinRide", 32, 80.0f, TVec3f(0.0f, 50.0f, 0.0f));
    MR::addHitSensorEye(this, "NpcBodyAttack", 32, 60.0f, TVec3f(0.0f, 60.0f, 0.0f));
    MR::addHitSensor(this, "NpcBodyReceive", 5, 32, 120.0f, TVec3f(0.0f, 60.0f, 0.0f));
    MR::addHitSensorNpc(this, "NpcBodyReceive2", 32, 60.0f, TVec3f(0.0f, 60.0f, 0.0f));
    initBinder(50.0f, 50.0f, 256);
    MR::setBinderOffsetVec(this, &mBinderOffset, false);
    MR::offBind(this);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
    MR::invalidateClipping(this);
    MR::initLightCtrl(this);
    mDashEffectActive = false;
    initNerve(&NrvYoshi::YoshiNrv80413860::sInstance, 0);
    MR::initStarPointerTarget(this, 100.0f, TVec3f(0.0f, 100.0f, 0.0f));
    makeActorDead();
    mAnimationResources = new XanimeResourceTable(MR::getResourceHolder(this), yoshiAnimationGroups, yoshiAnimationAux, yoshiAnimationOffsets,
        yoshiAnimationBckTable, yoshiAnimationBckTable2, yoshiAnimationBckTable3, NULL);
    fn_8040FCD0();
    fn_8040FD80();
    fn_8040FE30();
    mTongue = new YoshiTongue(this, "ヨッシーの舌");
    mTongue->initWithoutIter();
    mYoshiModelCtrl = new YoshiModelCtrl(this);
    mHeadCtrl = new YoshiHeadCtrl(mYoshiModelCtrl);
    mAnimationModelCtrl = new YoshiAnimationModelCtrl(this, mXanimePlayer, "YoshiHand");
    mRideModel = new YoshiSilhouette(this);
    mRideModel->initWithoutIter();
    mBreakModel = new BreakModel(this, "水柱", "WaterColumn", mBreakModelMtx.fn_vf0()->toMtxPtr(), "Splash");
    mBreakModel->initWithoutIter();
    mFruitCtrl = new YoshiFruitCtrl(this);
    mModelVisibility = new YoshiModelVisibility(this, mAnimationModelCtrl);
    initSound(10, NULL, NULL, TVec3f(0.0f));
    MR::createCenterScreenBlur();
}

void Yoshi::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    YoshiModelVisibility* visibility = mModelVisibility;
    if (visibility != NULL) {
        visibility->mAppeared = true;
        visibility->mModelCtrl->appear();
    }
    mFruitCtrl->fn_804157E0();
    fn_8040EE70(NULL);
    mHeadCtrl->fn_8041A9F0();
    mYoshiModelCtrl->fn_80418120();
    MR::onBind(this);
    MR::onCalcGravity(this);
    mTongue->makeActorAppeared();
    mVelocity.zero();
    mMovementVelocity.zero();
    mPreviousVelocity.zero();
    mAutoDisappearTask->mTimer = 600;
    YoshiGroundChecker* ground = mGroundChecker;
    ground->mAirborneFrames = 4;
    ground->mUpdated = false;
    _144 = false;
    _145 = false;
    setNerve(&NrvYoshi::YoshiNrv80413860::sInstance);
}

void Yoshi::makeActorDead() {
    LiveActor::makeActorDead();
    if (mTongue != NULL) mTongue->makeActorDead();
    YoshiModelVisibility* visibility = mModelVisibility;
    if (visibility != NULL) {
        visibility->mAppeared = false;
        visibility->mModelCtrl->kill();
    }
}

void Yoshi::movement() {
    TVec3f previousPosition(mPosition);
    LiveActor::movement();
    mVelocityAdjustment->remove(this);
    YoshiVelocityAdjustment* adjustment = mVelocityAdjustment;
    adjustment->mMinimum.zero();
    adjustment->mMaximum.zero();
    fn_804116B0();
}

// NONMATCHING: matrix translation is inlined; FPR allocation differs.
void Yoshi::calcAnim() {
    if (isNerve(&NrvYoshi::YoshiNrv80413840::sInstance)) {
        mAnimationSync->update();
        return;
    }
    mAnimationSync->mAnimation = NULL;
    mBaseMtx.setRotate(mRotation.x, mRotation.y, mRotation.z);
    mBaseMtx.setTrans(mPosition);
    fn_8040C960();
    mXanimePlayer->calcAnm(0);
    if (mUpperAnimationActive) {
        mXanimePlayerUpper->calcAnm(MR::getJointIndex(this, "Head"));
    }
    fn_8040CB30();
    mXanimePlayer->clearAnm(0);
    if (mUpperAnimationActive) {
        mXanimePlayerUpper->clearAnm(MR::getJointIndex(this, "Head"));
    }
    mXanimePlayer->mHasUpdatedFrame = false;
    mXanimePlayer->mAnimationChanged = false;
    mXanimePlayerUpper->mHasUpdatedFrame = false;
    mXanimePlayerUpper->mAnimationChanged = false;
}

void Yoshi::fn_8040C960() {
    if (mModelManager != NULL) {
        mFruitCtrl->fn_80415630();
        calcAndSetBaseMtx();
        if (mModelManager->mBvaPlayer != NULL) {
            mModelManager->mBvaPlayer->calc();
            mModelManager->mBvaPlayer->clearUpdateSkipFlag();
        }
        mModelManager->updateDL(true);
        mModelManager->mBrkPlayer->clearUpdateSkipFlag();
        mModelManager->mBtkPlayer->clearUpdateSkipFlag();
        mAnimationModelCtrl->calcAnim();
    }
}

#pragma pool_data off
void Yoshi::fn_8040CA10() {
    fn_804103F0();
    fn_80411A30();
    mYoshiModelCtrl->fn_80417FE0();
    if (mDashTimer > 0) {
        MR::setStageBGMStateBit(0x10);
        if (--mDashTimer == 0) {
            MR::startCenterScreenBlur(35, 15.0f, 80, 5, 30);
        }
        if (!mDashEffectActive) {
            mDashEffectActive = true;
            MR::emitEffect(this, "Dash");
        }
    }
    else if (mDashEffectActive) {
        mDashEffectActive = false;
        MR::deleteEffect(this, "Dash");
        MR::emitEffect(this, "DashEnd");
        MR::startActionSound(this, "DashEnd", -1, -1, -1);
    }
    if (mDashTopWaterEffectActive) {
        MR::startActionSound(this, "DashWater", -1, -1, 6);
    }
}
#pragma pool_data reset


void Yoshi::fn_8040CB30() {
    mXanimePlayer->getCore()->mCalcMode = 1;
    MR::calcJ3DModel(this);
    mYoshiModelCtrl->fn_80418020();
    mXanimePlayer->getCore()->mCalcMode = 2;
    MR::calcJ3DModel(this);
}

void Yoshi::kill() {
    LiveActor::kill();
    mDashTimer = 0;
}

void Yoshi::fn_8040CBD0() {
    _1A1 = false;
    if (!_1A2) mModelVisibility->fn_804132C0();
}

void Yoshi::fn_8040CBF0() {
    _1A1 = true;
    mModelVisibility->fn_80413330();
}

void Yoshi::fn_8040CC00() {
    _1A2 = false;
    if (!_1A1) mModelVisibility->fn_804132C0();
}

void Yoshi::fn_8040CC20() {
    _1A2 = true;
    mModelVisibility->fn_80413330();
}

void Yoshi::fn_8040CC30(const char* name) {
    mAnimationModelCtrl->fn_8040B6D0(name);
}

void Yoshi::fn_8040CC40() {
    mRideCountdown = 1;
    MR::offBind(this);
    MR::invalidateHitSensors(this);
    MR::validateHitSensor(this, "AutoEat");
    fn_8040EE70(NULL);
    setNerve(&NrvYoshi::YoshiNrv80413840::sInstance);
    MR::startActionSound(this, "Ride", -1, -1, -1);
    MR::changePlayerModeYoshi();
}

void Yoshi::fn_8040CCD0(const TVec3f& velocity) {
    fn_80410350(velocity);fn_804107D0();*mJumpRequested=false;setNerve(&NrvYoshi::YoshiNrv80413920::sInstance);
}

void Yoshi::fn_8040CD20(const TVec3f& velocity) {
    fn_80410350(velocity);TVec3f front;mBaseMtx.getZDir(front);
    if(front.dot(velocity)>0.0f){fn_8040EDA0("空中後方ダメージ（Ｎｐｃ）");_130=true;}
    else{fn_8040EDA0("空中前方ダメージ（Ｎｐｃ）");_130=false;}
    fn_804107D0();setNerve(&NrvYoshi::YoshiNrv80413930::sInstance);
}

void Yoshi::fn_8040CDD0(const TVec3f& velocity) {
    fn_80410350(velocity);TVec3f front;mBaseMtx.getZDir(front);
    if(front.dot(velocity)>0.0f){fn_8040EDA0("後方ダメージ（Ｎｐｃ）");_130=true;}
    else{fn_8040EDA0("前方ダメージ（Ｎｐｃ）");_130=false;}
    fn_804107D0();setNerve(&NrvYoshi::YoshiNrv80413950::sInstance);
}

void Yoshi::fn_8040CE80(){fn_80410350(TVec3f(0.0f));MR::fn_80422520();setNerve(&NrvYoshi::YoshiNrv80413960::sInstance);}

void Yoshi::fn_8040CED0(){fn_80410350(TVec3f(0.0f));MR::fn_80422550();setNerve(&NrvYoshi::YoshiNrv80413970::sInstance);}

void Yoshi::fn_8040CF20() {
    if (mRideCountdown > 0) {
        mRideCountdown--;
    }
}

bool Yoshi::fn_8040CF40() const {
    return mRideCountdown == 0;
}

void Yoshi::fn_8040CF50() {
    if (MR::isFirstStep(this)) {
        fn_8040EDA0("出現（Ｎｐｃ）");
        *mJumpRequested = false;
    }
    if (fn_8040F590(NULL)) fn_80410280();
}

void Yoshi::fn_8040CFC0() {
    if (MR::isFirstStep(this)) {
        if (*mJumpRequested) fn_8040EDA0("キョロキョロ（Ｎｐｃ）");
        else fn_8040EDA0("待機（Ｎｐｃ）");
    }
    if (*mJumpRequested) MR::startActionSound(this, "Wander", -1, -1, -1);
    if (!mAutoDisappearTask->mTimer) {
        setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);
        return;
    }
    if (_144) {
        setNerve(&NrvYoshi::YoshiNrv80413910::sInstance);
        return;
    }
    if (*mJumpRequested && fn_8040F590(nullptr)) {
        setNerve(&NrvYoshi::YoshiNrv804138A0::sInstance);
        return;
    }
    YoshiGroundChecker* ground = mGroundChecker;
    bool slipping = false;
    if (ground->isGrounded() && ground->mSlipping) slipping = true;
    if (slipping) {
        setNerve(&NrvYoshi::YoshiNrv804138F0::sInstance);
        return;
    }
    if (mGroundChecker->isAirborne()) setNerve(&NrvYoshi::YoshiNrv804138D0::sInstance);
}

void Yoshi::fn_8040D130() {
    if (MR::isFirstStep(this)) fn_8040EDA0("待機（Ｎｐｃ）");
    if (!mAutoDisappearTask->mTimer) {
        setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);
        return;
    }
    if (_144) {
        setNerve(&NrvYoshi::YoshiNrv80413910::sInstance);
        return;
    }
    YoshiGroundChecker* ground = mGroundChecker;
    bool slipping = false;
    if (ground->isGrounded() && ground->mSlipping) slipping = true;
    if (slipping) {
        setNerve(&NrvYoshi::YoshiNrv804138F0::sInstance);
        return;
    }
    if (mGroundChecker->isAirborne()) {
        setNerve(&NrvYoshi::YoshiNrv804138D0::sInstance);
        return;
    }
    if (MR::isInAreaObj("YoshiSendOffArea", mPosition)) {
        setNerve(&NrvYoshi::YoshiNrv80413A10::sInstance);
        return;
    }
    if (mPlayerFacingCtrl->fn_80413040()) setNerve(&NrvYoshi::YoshiNrv80413890::sInstance);
}

void Yoshi::fn_8040D260() {
    if (MR::isFirstStep(this)) {
        fn_8040EDA0("待機ターン（Ｎｐｃ）");
        mPlayerFacingCtrl->mTurnTimer = 0;
    }
    if (!mAutoDisappearTask->mTimer) {
        setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);
        return;
    }
    if (_144) {
        setNerve(&NrvYoshi::YoshiNrv80413910::sInstance);
        return;
    }
    YoshiGroundChecker* ground = mGroundChecker;
    bool slipping = false;
    if (ground->isGrounded() && ground->mSlipping) slipping = true;
    if (slipping) {
        setNerve(&NrvYoshi::YoshiNrv804138F0::sInstance);
        return;
    }
    if (mGroundChecker->isAirborne()) {
        setNerve(&NrvYoshi::YoshiNrv804138D0::sInstance);
        return;
    }
    mPlayerFacingCtrl->fn_80413110();
    YoshiPlayerFacingCtrl* facing = mPlayerFacingCtrl;
    if (facing->mTurnTimer > 30) {
        facing->mTimer = 0;
        fn_80410280();
    }
}

void Yoshi::fn_8040D380() {
    if (MR::isFirstStep(this)) {
        fn_8040EDA0("ラン（Ｎｐｃ）");
        fn_804124D0(8.0f);
        mRandomTurn->reset();
    }
    MR::startActionSound(this, "WanderRun", -1, -1, -1);
    mRandomTurn->update(mBaseMtx, &mMovementVelocity);
    fn_80410570(1.0f);
    YoshiGroundChecker* ground = mGroundChecker;
    bool slipping = false;
    if (ground->isGrounded() && ground->mSlipping) slipping = true;
    if (slipping) {
        TVec3f slope;
        fn_80410810(&slope);
        if (mMovementVelocity.dot(slope) < 0.0f) {
            mMovementVelocity.x *= 0.95f;
            mMovementVelocity.y *= 0.95f;
            mMovementVelocity.z *= 0.95f;
        }
    } else {
        TVec3f planar;
        MR::vecKillElement(mMovementVelocity, mGravity, &planar);
        planar.setLength(8.0f);
        mMovementVelocity = planar;
    }
    if (!mAutoDisappearTask->mTimer) {
        setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);
        return;
    }
    if (_144) {
        setNerve(&NrvYoshi::YoshiNrv80413910::sInstance);
        return;
    }
    ground = mGroundChecker;
    bool stoppedOnSlope = false;
    if (ground->isGrounded() && ground->mSlipping) stoppedOnSlope = true;
    if (stoppedOnSlope && mMovementVelocity.squared() < 1.0f) {
        setNerve(&NrvYoshi::YoshiNrv804138F0::sInstance);
        return;
    }
    if (mGroundChecker->isAirborne()) {
        setNerve(&NrvYoshi::YoshiNrv804138D0::sInstance);
        return;
    }
    if (mRandomTurn->mTimer <= 0) {
        setNerve(&NrvYoshi::YoshiNrv80413870::sInstance);
        return;
    }
    if (fn_804112A0()) setNerve(&NrvYoshi::YoshiNrv804138B0::sInstance);
}

void Yoshi::fn_8040D5B0() {
    if (MR::isFirstStep(this)) {
        fn_8040EDA0("おっとっと（Ｎｐｃ）");
        mEscapeDirection->reset();
    }
    mEscapeDirection->update();
    if (!mAutoDisappearTask->mTimer) {
        setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);
        return;
    }
    if (_144) {
        setNerve(&NrvYoshi::YoshiNrv80413910::sInstance);
        return;
    }
    YoshiGroundChecker* ground = mGroundChecker;
    bool slipping = false;
    if (ground->isGrounded() && ground->mSlipping) slipping = true;
    if (slipping) {
        setNerve(&NrvYoshi::YoshiNrv804138F0::sInstance);
        return;
    }
    if (mGroundChecker->isAirborne()) {
        setNerve(&NrvYoshi::YoshiNrv804138D0::sInstance);
        return;
    }
    if (mEscapeDirection->mStep >= 16 && fn_8040F590(nullptr)) setNerve(&NrvYoshi::YoshiNrv804138C0::sInstance);
}

void Yoshi::fn_8040D6E0(){
    if(MR::isFirstStep(this)){fn_8040EDA0("ターン（Ｎｐｃ）");mMovementVelocity.zero();}
    f32 rate;if(getNerveStep()<30)rate=1.0f/(30-getNerveStep());else rate=1.0f;
    fn_8040BD00(&mBaseMtx,mEscapeDirection->mDirection,rate,3.1415927410125732f);
    if(!mAutoDisappearTask->mTimer){setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);return;}
    if(_144){setNerve(&NrvYoshi::YoshiNrv80413910::sInstance);return;}
    if(MR::isGreaterEqualStep(this,30))setNerve(&NrvYoshi::YoshiNrv80413870::sInstance);
}

void Yoshi::fn_8040D7F0() {
    if (MR::isFirstStep(this)) fn_8040EDA0("落下（Ｎｐｃ）");
    if (!mAutoDisappearTask->mTimer) {
        setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);
        return;
    }
    if (_144) {
        setNerve(&NrvYoshi::YoshiNrv80413910::sInstance);
        return;
    }
    YoshiGroundChecker* ground = mGroundChecker;
    bool slipping = false;
    if (ground->isGrounded() && ground->mSlipping) slipping = true;
    if (slipping) {
        setNerve(&NrvYoshi::YoshiNrv804138F0::sInstance);
        return;
    }
    if (mGroundChecker->isGrounded()) setNerve(&NrvYoshi::YoshiNrv804138E0::sInstance);
}

void Yoshi::fn_8040D8E0() {
    if (MR::isFirstStep(this)) fn_8040EDA0("着地（Ｎｐｃ）");
    if (_144) {
        setNerve(&NrvYoshi::YoshiNrv80413910::sInstance);
        return;
    }
    if (mGroundChecker->isAirborne()) {
        setNerve(&NrvYoshi::YoshiNrv804138D0::sInstance);
        return;
    }
    if (fn_8040F590(nullptr)) {
        YoshiGroundChecker* ground = mGroundChecker;
        bool slipping = false;
        if (ground->isGrounded() && ground->mSlipping) slipping = true;
        if (slipping) {
            setNerve(&NrvYoshi::YoshiNrv804138F0::sInstance);
            return;
        }
        if (*mJumpRequested) {
            setNerve(&NrvYoshi::YoshiNrv804138A0::sInstance);
            return;
        }
        setNerve(&NrvYoshi::YoshiNrv80413870::sInstance);
    }
}

void Yoshi::fn_8040D9F0() {
    if (MR::isFirstStep(this)) fn_8040EDA0("坂すべり仰向け（Ｎｐｃ）");
    fn_80410570(0.05f);
    if (!mAutoDisappearTask->mTimer) {
        setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);
        return;
    }
    if (_144) {
        setNerve(&NrvYoshi::YoshiNrv80413910::sInstance);
        return;
    }
    if (mGroundChecker->isAirborne()) {
        setNerve(&NrvYoshi::YoshiNrv804138D0::sInstance);
        return;
    }
    if (mGroundChecker->isGrounded()) {
        YoshiGroundChecker* ground = mGroundChecker;
        bool slipping = false;
        if (ground->isGrounded() && ground->mSlipping) slipping = true;
        if (!slipping && MR::isGreaterEqualStep(this, 30)) setNerve(&NrvYoshi::YoshiNrv80413900::sInstance);
    }
}

void Yoshi::fn_8040DB10() {
    if (MR::isFirstStep(this)) {
        fn_8040EDA0("坂すべり仰向け終了（Ｎｐｃ）");
        mMovementVelocity.zero();
    }
    if (!mAutoDisappearTask->mTimer) {
        setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);
        return;
    }
    if (_144) {
        setNerve(&NrvYoshi::YoshiNrv80413910::sInstance);
        return;
    }
    if (mGroundChecker->isAirborne()) {
        setNerve(&NrvYoshi::YoshiNrv804138D0::sInstance);
        return;
    }
    YoshiGroundChecker* ground = mGroundChecker;
    bool slipping = false;
    if (ground->isGrounded() && ground->mSlipping) slipping = true;
    if (slipping) {
        setNerve(&NrvYoshi::YoshiNrv804138F0::sInstance);
        return;
    }
    if (fn_8040F590(nullptr)) fn_80410280();
}

#pragma pool_data off
void Yoshi::fn_8040DC20() {
    if (MR::isFirstStep(this)) fn_8040EDA0("水中泳ぎ（Ｎｐｃ）");
    if (!fn_8040F490("水面泳ぎ（Ｎｐｃ）") && !fn_8040F490("水中泳ぎ（Ｎｐｃ）")) {
        if (_145) fn_8040EDA0("水面泳ぎ（Ｎｐｃ）");
        else fn_8040EDA0("水中泳ぎ（Ｎｐｃ）");
    }
    if (!mAutoDisappearTask->mTimer) {
        fn_80411740();
        setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);
        return;
    }
    if (!_144) {
        if (mGroundChecker->isGrounded()) {
            setNerve(&NrvYoshi::YoshiNrv80413880::sInstance);
            TVec3f center;
            MR::copyJointPos(this, "Center", &center);
            MtxPtr matrix = getBaseMtx();
            TVec3f up(matrix[0][1], matrix[1][1], matrix[2][1]);
            MR::setPosition(this, center - up.scaleInline(50.0f));
            return;
        }
        setNerve(&NrvYoshi::YoshiNrv804138D0::sInstance);
    }
}
#pragma pool_data reset


void Yoshi::fn_8040DDA0(){
    if(MR::isFirstStep(this)){fn_8040EDA0("乗り捨て（Ｎｐｃ）");YoshiGroundChecker* ground=mGroundChecker;ground->mAirborneFrames=4;ground->mUpdated=false;}
    if(fn_8040F590(nullptr))fn_80410280();
}

#pragma pool_data off
void Yoshi::fn_8040DE20(){
    if(MR::isFirstStep(this)){YoshiGroundChecker* ground=mGroundChecker;ground->mAirborneFrames=4;ground->mUpdated=false;MR::startActionSound(this,"YoshiDamageRunAway",-1,-1,-1);}
    if(fn_8040F590(nullptr)){if(_130)fn_8040EDA0("空中後方ダメージ落下（Ｎｐｃ）");else fn_8040EDA0("空中前方ダメージ落下（Ｎｐｃ）");}
    if(mGroundChecker->isGrounded()){setNerve(&NrvYoshi::YoshiNrv80413940::sInstance);return;}
    if(MR::isGreaterEqualStep(this,120))setNerve(&NrvYoshi::YoshiNrv804138D0::sInstance);
}
#pragma pool_data reset


void Yoshi::fn_8040DF10(){
    if(MR::isFirstStep(this)){if(_130)fn_8040EDA0("空中後方ダメージ着地（Ｎｐｃ）");else fn_8040EDA0("空中前方ダメージ着地（Ｎｐｃ）");}
    if(mGroundChecker->isAirborne()){setNerve(&NrvYoshi::YoshiNrv804138D0::sInstance);return;}
    if(fn_8040F590(nullptr))fn_80410280();
}

// NONMATCHING: GPR/FPR allocation and scalar-load scheduling differ.
void Yoshi::fn_8040DFB0() {
    if (MR::isFirstStep(this)) MR::startActionSound(this, "YoshiDamageRunAway", -1, -1, -1);
    if (mGroundChecker->isGrounded()) {
        const TVec3f* gravity = &mGravity;
        JMAVECScaleAdd(gravity, &mMovementVelocity, &mMovementVelocity, -gravity->dot(mMovementVelocity));
        mMovementVelocity.x *= .9f;
        mMovementVelocity.y *= .9f;
        mMovementVelocity.z *= .9f;
    }
    if (fn_8040F590(nullptr)) fn_80410280();
}

void Yoshi::fn_8040E080(){
    if(MR::isFirstStep(this)){_13C=0;MR::emitEffect(this,"FireDamage");}
    if(MR::isStep(this,1))fn_8040EDA0("ファイア死亡（Ｎｐｃ）");
    if(fn_8040F590(nullptr)&&MR::isGreaterEqualStep(this,1))++_13C;
    if(_13C==2){MR::deleteEffect(this,"FireDamage");makeActorDead();}
}

void Yoshi::fn_8040E160(){
    if(MR::isFirstStep(this)){mFruitCtrl->fn_80415810();_13C=0;}
    if(MR::isStep(this,1))fn_8040EDA0("電気ダメージ死亡（Ｎｐｃ）");
    if(fn_8040F590(nullptr)&&MR::isGreaterEqualStep(this,1))++_13C;
    if(_13C==2){mFruitCtrl->fn_80415820();makeActorDead();}
}

void Yoshi::fn_8040E230(){
    if(MR::isFirstStep(this)){mMovementVelocity.zero();fn_8040EDA0("消滅（Ｎｐｃ）");MR::fn_804224F0();MR::offBind(this);MR::startActionSound(this,"TurnToEgg",-1,-1,-1);}
    if(fn_8040F590(nullptr))makeActorDead();
}

void Yoshi::fn_8040E2D0(){
    if(MR::isFirstStep(this)){mMovementVelocity.zero();MR::hideModel(mTongue);}
    if(!MR::isDemoActive()){setNerve(&NrvYoshi::YoshiNrv80413870::sInstance);MR::showModel(mTongue);MR::startBtk(this,"Normal");}
}

void Yoshi::fn_8040E350() {
    if (MR::isFirstStep(this)) {
        fn_8040EDA0("ラン（Ｎｐｃ）");
        _188 = MR::getRandom(120L, 180L);
        TVec3f front;
        mBaseMtx.getZDir(front);
        TVec3f direction;
        JMAVECScaleAdd(&_18C, &front, &direction, -_18C.dot(front));
        const TVec3f* gravity = &mGravity;
        JMAVECScaleAdd(gravity, &direction, &direction, -gravity->dot(direction));
        if (MR::normalizeOrZero(&direction)) {
            PSVECCrossProduct(gravity, &_18C, &direction);
            MR::normalizeOrZero(&direction);
        }
        mMovementVelocity.set(direction);
        mMovementVelocity.x *= 8.0f;
        mMovementVelocity.y *= 8.0f;
        mMovementVelocity.z *= 8.0f;
    }
    MR::startActionSound(this, "WanderRun", -1, -1, -1);
    if (!_198) {
        setNerve(&NrvYoshi::YoshiNrv804138A0::sInstance);
        return;
    }
    if (MR::isBindedWall(this) && MR::getWallNormal(this).dot(mMovementVelocity) < 0.0f) fn_80411630();
    const TVec3f* gravity = &mGravity;
    JMAVECScaleAdd(gravity, &mMovementVelocity, &mMovementVelocity, -gravity->dot(mMovementVelocity));
    JMAVECScaleAdd(&_18C, &mMovementVelocity, &mMovementVelocity, -_18C.dot(mMovementVelocity));
    mMovementVelocity.setLength(8.0f);
    fn_80410570(1.0f);
    if (mGroundChecker->isAirborne()) {
        setNerve(&NrvYoshi::YoshiNrv804139B0::sInstance);
        return;
    }
    YoshiGroundChecker* ground = mGroundChecker;
    bool slipping = false;
    if (ground->isGrounded() && ground->mSlipping) slipping = true;
    if (slipping) {
        TVec3f slope;
        fn_80410810(&slope);
        if (mMovementVelocity.dot(slope) < 0.0f) {
            mMovementVelocity.x *= 0.95f;
            mMovementVelocity.y *= 0.95f;
            mMovementVelocity.z *= 0.95f;
        }
    } else mMovementVelocity.setLength(8.0f);
    if (!mAutoDisappearTask->mTimer) {
        setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);
        return;
    }
    ground = mGroundChecker;
    bool stoppedOnSlope = false;
    if (ground->isGrounded() && ground->mSlipping) stoppedOnSlope = true;
    if (stoppedOnSlope && mMovementVelocity.squared() < 1.0f) {
        setNerve(&NrvYoshi::YoshiNrv804139D0::sInstance);
        return;
    }
    if (MR::isGreaterEqualStep(this, _188)) setNerve(&NrvYoshi::YoshiNrv804139A0::sInstance);
}

void Yoshi::fn_8040E660() {
    if (MR::isFirstStep(this)) fn_8040EDA0("キョロキョロ（Ｎｐｃ）");
    if (fn_8040F590(nullptr)) {
        setNerve(&NrvYoshi::YoshiNrv80413990::sInstance);
        return;
    }
    if (!mAutoDisappearTask->mTimer) {
        setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);
        return;
    }
    YoshiGroundChecker* ground = mGroundChecker;
    bool slipping = false;
    if (ground->isGrounded() && ground->mSlipping) slipping = true;
    if (slipping) {
        setNerve(&NrvYoshi::YoshiNrv804138F0::sInstance);
        return;
    }
    if (mGroundChecker->isAirborne()) setNerve(&NrvYoshi::YoshiNrv804138D0::sInstance);
}

void Yoshi::fn_8040E750() {
    if (MR::isFirstStep(this)) fn_8040EDA0("落下（Ｎｐｃ）");
    if (!mAutoDisappearTask->mTimer) {
        setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);
        return;
    }
    YoshiGroundChecker* ground = mGroundChecker;
    bool slipping = false;
    if (ground->isGrounded() && ground->mSlipping) slipping = true;
    if (slipping) {
        setNerve(&NrvYoshi::YoshiNrv804139D0::sInstance);
        return;
    }
    if (mGroundChecker->isGrounded()) setNerve(&NrvYoshi::YoshiNrv804139C0::sInstance);
}

void Yoshi::fn_8040E820() {
    if (MR::isFirstStep(this)) fn_8040EDA0("着地（Ｎｐｃ）");
    if (mGroundChecker->isAirborne()) {
        setNerve(&NrvYoshi::YoshiNrv804139B0::sInstance);
        return;
    }
    if (fn_8040F590(nullptr)) {
        YoshiGroundChecker* ground = mGroundChecker;
        bool slipping = false;
        if (ground->isGrounded() && ground->mSlipping) slipping = true;
        if (slipping) {
            setNerve(&NrvYoshi::YoshiNrv804139D0::sInstance);
            return;
        }
        setNerve(&NrvYoshi::YoshiNrv80413990::sInstance);
    }
}

void Yoshi::fn_8040E8F0() {
    if (MR::isFirstStep(this)) fn_8040EDA0("坂すべり仰向け（Ｎｐｃ）");
    fn_80410570(0.05f);
    if (!mAutoDisappearTask->mTimer) {
        setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);
        return;
    }
    if (mGroundChecker->isAirborne()) {
        setNerve(&NrvYoshi::YoshiNrv804139B0::sInstance);
        return;
    }
    if (mGroundChecker->isGrounded()) {
        YoshiGroundChecker* ground = mGroundChecker;
        bool slipping = false;
        if (ground->isGrounded() && ground->mSlipping) slipping = true;
        if (!slipping) setNerve(&NrvYoshi::YoshiNrv804139E0::sInstance);
    }
}

void Yoshi::fn_8040E9E0() {
    if (MR::isFirstStep(this)) {
        fn_8040EDA0("坂すべり仰向け終了（Ｎｐｃ）");
        mMovementVelocity.zero();
    }
    if (!mAutoDisappearTask->mTimer) {
        setNerve(&NrvYoshi::YoshiNrv80413980::sInstance);
        return;
    }
    if (mGroundChecker->isAirborne()) {
        setNerve(&NrvYoshi::YoshiNrv804139B0::sInstance);
        return;
    }
    YoshiGroundChecker* ground = mGroundChecker;
    bool slipping = false;
    if (ground->isGrounded() && ground->mSlipping) slipping = true;
    if (slipping) {
        setNerve(&NrvYoshi::YoshiNrv804139D0::sInstance);
        return;
    }
    if (fn_8040F590(nullptr)) setNerve(&NrvYoshi::YoshiNrv80413990::sInstance);
}

void Yoshi::fn_8040EAE0() {
    if (MR::isFirstStep(this)) {
        fn_8040EDA0("フリーズ（Ｎｐｃ）");
        mMovementVelocity.zero();
    }
    if (!MR::isStarPointerPointing2PAttachTarget(this)) {
        mAutoDisappearTask->mTimer = 600;
        *mJumpRequested = true;
        fn_80410280();
    }
}

void Yoshi::fn_8040EB60() {
    if (MR::isFirstStep(this)) {
        fn_8040EDA0("スターピースヒット（Ｎｐｃ）");
        mMovementVelocity.set(mGravity);
    }
    if (fn_8040F590(nullptr)) {
        mAutoDisappearTask->mTimer = 600;
        *mJumpRequested = true;
        fn_80410280();
    }
}

void Yoshi::fn_8040EBF0() {
    if (MR::isFirstStep(this)) {
        fn_8040EDA0("見送り（Ｎｐｃ）");
        mPlayerFacingCtrl->mTurnTimer = 0;
    }
    if (_144) {
        setNerve(&NrvYoshi::YoshiNrv80413910::sInstance);
        return;
    }
    YoshiGroundChecker* ground = mGroundChecker;
    bool slipping = false;
    if (ground->isGrounded() && ground->mSlipping) slipping = true;
    if (slipping) {
        setNerve(&NrvYoshi::YoshiNrv804138F0::sInstance);
        return;
    }
    if (mGroundChecker->isAirborne()) {
        setNerve(&NrvYoshi::YoshiNrv804138D0::sInstance);
        return;
    }
    if (MR::getBckFrame(this) == 5.0f || MR::getBckFrame(this) == 20.0f || MR::getBckFrame(this) == 36.0f || MR::getBckFrame(this) == 50.0f || MR::getBckFrame(this) == 65.0f || MR::getBckFrame(this) == 79.0f || MR::getBckFrame(this) == 95.0f || MR::getBckFrame(this) == 110.0f) {
        MR::startSoundInPlayerDistance(this, "SendOffOnce", -1, -1, 1800.0f);
    }
    mPlayerFacingCtrl->fn_80413110();
}

void Yoshi::fn_8040ED90(const char* animation) {
    mXanimePlayer->setDefaultAnimation(animation);
}

void Yoshi::fn_8040EDA0(const char* animation) {
    mXanimePlayer->changeAnimationBck(animation);
    if (mEffectKeeper != NULL) {
        mEffectKeeper->changeBck();
    }
}

void Yoshi::fn_8040EDE0(const char* animation) {
    if (animation != NULL) {
        mXanimePlayer->stopAnimation(animation);
    }
    else {
        mXanimePlayer->stopAnimation();
    }
}

void Yoshi::fn_8040EE00(const char* animation) {
    if (animation != NULL) {
        mUpperDefaultAnimationEnabled = true;
        mXanimePlayerUpper->setDefaultAnimation(animation);
    }
    else {
        mUpperDefaultAnimationEnabled = false;
    }
}

void Yoshi::fn_8040EE30(const char* animation) {
    mXanimePlayerUpper->changeAnimationBck(animation);
    mUpperAnimationActive = true;
}

void Yoshi::fn_8040EE70(const char* animation) {
    if (mUpperAnimationActive && (animation == NULL || mXanimePlayerUpper->isRun(animation))) {
        u16 joint = MR::getJointIndex(this, "Head");
        XanimeCore* core = mXanimePlayer->getCore();
        mXanimePlayerUpper->getCore()->freezeCopy(MR::getJ3DModel(this)->getModelData(), core, joint, 8);
        mXanimePlayerUpper->stopAnimation();
        mUpperAnimationActive = false;
    }
}

void Yoshi::fn_8040EF20() {
    mYoshiModelCtrl->fn_80418130();
}

void Yoshi::fn_8040EF30() {
    mYoshiModelCtrl->fn_80418120();
}

void Yoshi::fn_8040EF40(){mDashTimer=0;MR::tryForceDeleteEffect(this,"Dash");mFruitCtrl->_5=true;YoshiModelVisibility* visibility=mModelVisibility;visibility->mAppeared=false;visibility->mModelCtrl->kill();}

void Yoshi::fn_8040EFB0(){mFruitCtrl->_5=false;YoshiModelVisibility* visibility=mModelVisibility;visibility->mAppeared=true;visibility->mModelCtrl->appear();MR::emitEffect(this,"BalloonEnd");}

void Yoshi::fn_8040F020(){mFruitCtrl->fn_80415800();}

void Yoshi::fn_8040F030(){mFruitCtrl->mLightFruitActive=true;MR::emitEffect(this,"Glow");}

void Yoshi::fn_8040F050(){mFruitCtrl->mLightFruitActive=false;MR::deleteEffect(this,"Glow");}

bool Yoshi::fn_8040F070()const{return mFruitCtrl->mLightFruitActive;}

void Yoshi::fn_8040F080(){YoshiFruitCtrl* fruit=mFruitCtrl;fruit->_10=true;fruit->_11=false;}

void Yoshi::fn_8040F0A0(){mFruitCtrl->_10=false;}

void Yoshi::fn_8040F0B0(){YoshiFruitCtrl* fruit=mFruitCtrl;fruit->_11=true;fruit->_10=false;}

void Yoshi::fn_8040F0D0(){mFruitCtrl->_11=false;}

bool Yoshi::fn_8040F0E0() const {
    return !isNerve(&NrvYoshi::YoshiNrv80413840::sInstance);
}

void Yoshi::fn_8040F110(){mDashTimer=480;MR::startCenterScreenBlur(35,15.0f,80,5,30);MR::startActionSound(this,"Dash",-1,-1,-1);}

f32 Yoshi::fn_8040F180(){return 1.8f;}

s32 Yoshi::getAutoEatSensorCount() const {
    return getSensor("AutoEat")->mSensorCount;
}

HitSensor* Yoshi::getAutoEatSensor(s32 index) const {
    return getSensor("AutoEat")->mSensors[index];
}

const TVec3f& Yoshi::getAutoEatSensorPosition() const {
    return getSensor("AutoEat")->mPosition;
}

bool Yoshi::fn_8040F230(const JMapInfoIter& iter){
    if(MR::isDemoExist("ヨッシー出会いデモ")){
        TalkMessageCtrl* talk=MR::createTalkCtrlDirectOnRootNodeAutomatic(this,iter,"DemoMeetYoshi",TVec3f(0.0f,0.0f,0.0f),nullptr);
        MR::initDemoSheetTalkAnim(this,iter,"ヨッシー出会いデモ","DemoMeetYoshi",talk,TVec3f(0.0f,0.0f,0.0f),nullptr);
        return MR::tryRegisterDemoActionNerve(this,&NrvYoshi::YoshiNrv80413850::sInstance,nullptr);
    }
    return false;
}

MtxPtr Yoshi::fn_8040F300(){return MR::getJointMtx(this,"Mouth");}

void Yoshi::fn_8040F310(){MR::tryEmitEffect(this,"Get");}

void Yoshi::fn_8040F320() {
    if (!mDashTopWaterEffectActive) {
        MR::emitEffect(this, "DashTopWater");
        mDashTopWaterEffectActive = true;
    }
}

void Yoshi::fn_8040F370() {
    if (mDashTopWaterEffectActive) {
        MR::deleteEffect(this, "DashTopWater");
        mDashTopWaterEffectActive = false;
    }
}

void Yoshi::fn_8040F3C0(){fn_8040EE70(nullptr);mDashTimer=0;MR::tryForceDeleteEffect(this,"Dash");if(mFruitCtrl->mLightFruitActive)MR::endYoshiLightFruit();fn_8040F050();mFruitCtrl->_10=false;mFruitCtrl->_11=false;}

u16 Yoshi::fn_8040F440() {
    return MR::getJointIndex(this, "Head");
}

// NONMATCHING: player load is scheduled after the prologue stores.
bool Yoshi::fn_8040F450(const char* animation) {
    return strcmp(mXanimePlayer->getDefaultAnimationName(), animation) == 0;
}

bool Yoshi::fn_8040F490(const char* animation) {
    if (animation == NULL) {
        return mXanimePlayer->mCurrentAnimation != mXanimePlayer->mDefaultAnimation;
    }
    if (mXanimePlayer->isRun(animation)) {
        return true;
    }
    if (mUpperAnimationActive && mXanimePlayerUpper->isRun(animation)) {
        return true;
    }
    return false;
}

bool Yoshi::fn_8040F530() const {
    return mXanimePlayer->isAnimationRunSimple();
}

bool Yoshi::fn_8040F540(const char* animation) {
    if (animation == NULL) {
        return mXanimePlayer->mCurrentAnimation != mXanimePlayer->mDefaultAnimation;
    }
    return mXanimePlayer->isRun(animation);
}

bool Yoshi::fn_8040F570() {
    return mXanimePlayer->mCurrentAnimation == mXanimePlayer->mDefaultAnimation;
}

bool Yoshi::fn_8040F590(const char* animation) {
    if (animation != NULL) {
        return mXanimePlayer->isTerminate(animation);
    }
    return mXanimePlayer->isTerminate();
}

// NONMATCHING: player load is scheduled after the prologue stores.
bool Yoshi::fn_8040F5B0(const char* animation) {
    return strcmp(mXanimePlayerUpper->getCurrentAnimationName(), animation) == 0;
}

bool Yoshi::fn_8040F5F0() const {
    return mXanimePlayerUpper->fn_8040F600();
}

f32 Yoshi::fn_8040F620() const {
    return mXanimePlayer->getFrameCtrl()->getFrame();
}

// NONMATCHING: Euler extraction is inlined; matrix copy scheduling differs.
void Yoshi::setTransformFromMtx(const f32 (*matrix)[4]) {
    mBaseMtx.set(matrix);
    MR::setPosition(this, matrix[0][3], matrix[1][3], matrix[2][3]);
    TVec3f rotation;
    mBaseMtx.getEulerXYZ(rotation);
    MR::setRotation(this, rotation);
}

void Yoshi::control() {
    fn_80411ED0();
    if (MR::isPlayerDead()) {
        mTongue->fn_8041CBA0();
    }
    if (isNerve(&NrvYoshi::YoshiNrv80413840::sInstance)) {
        return;
    }
    MR::validateHitSensors(this);
    if (isNerve(&NrvYoshi::YoshiNrv80413910::sInstance)) {
        MR::copyJointPos(this, "Center", &mBinderOffset);
        mBinderOffset.sub(mPosition);
    }
    else {
        MtxPtr matrix = getBaseMtx();
        mBinderOffset.set<f32>(50.0f * matrix[0][1], 50.0f * matrix[1][1], 50.0f * matrix[2][1]);
    }
    fn_8040CA10();
    fn_80410890();
    mUpdateTasks->update();
    mAdditionalVelocity.zero();
    if (_144) {
        fn_80410D90();
    }
    else if (fn_80412270()) {
        fn_804109E0();
        fn_80410AC0();
        fn_80410BA0();
    }
    mVelocityAdjustment->apply(this);
    TVec3f average(mMovementVelocity * 0.5f + mPreviousVelocity * 0.5f);
    mPreviousVelocity = mMovementVelocity;
    TVec3f velocity(average + mAdditionalVelocity);
    mVelocity.set<f32>(velocity);
    _198 = fn_804113C0();
    fn_804117F0();
    fn_80411BF0();
}

void Yoshi::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mBaseMtx);
    MR::setBaseScale(this, mScale);
}

void Yoshi::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (!MR::isSensorPlayerOrRide(receiver)) return;
    bool isBody = getSensor("NpcBodyAttack") == sender;
    if (isBody && fn_804100E0()) {
        MR::sendMsgPush(receiver, sender);
    }
}

#pragma pool_data off
bool Yoshi::receiveMsgPlayerAttack(u32 message, HitSensor* sender, HitSensor* receiver) {
    if (MR::isMsgStarPieceReflect(message)) {
        if (fn_80411960()) {
            setNerve(&NrvYoshi::YoshiNrv80413A00::sInstance);
        }
        return true;
    }
    if (MR::isMsgSupportTicoSpin(message)) {
        if (fn_80411960() || isNerve(&NrvYoshi::YoshiNrv804139F0::sInstance)) {
            mSupportSpinTimer = 60;
            setNerve(&NrvYoshi::YoshiNrv80413A00::sInstance);
            return true;
        }
        return false;
    }
    if (!fn_8040FEE0()) return false;
    bool isBody = getSensor("Ride") == receiver;
    if (!isBody) isBody = getSensor("NpcBodyReceive") == receiver;
    if (isBody) {
        if (MR::isMsgPlayerTrample(message) || MR::isMsgPlayerHipDrop(message)) {
            mMountedBySpin = false;
            fn_8040CC40();
            return false;
        }
    }
    else {
        bool isSpinBody = getSensor("SpinRide") == receiver;
        if (isSpinBody && MR::isMsgPlayerSpinAttack(message)) {
            mMountedBySpin = true;
            fn_8040CC40();
            return false;
        }
    }
    return false;
}
#pragma pool_data reset


// NONMATCHING: getZDir is inlined; pointer comparison lowering differs.
bool Yoshi::receiveMsgPush(HitSensor* sender, HitSensor* receiver) {
    if (isNerve(&NrvYoshi::YoshiNrv80413840::sInstance)) return false;
    if (MR::isSensorPlayerOrRide(sender)) {
        if (!fn_80411D10()) return false;
        bool isBody = getSensor("NpcBodyReceive2") == receiver;
        if (isBody == false) return false;
    }
    else {
        if (fn_804101D0()) return false;
        bool isBody = getSensor("NpcBodyReceive") == receiver;
        if (isBody == false) return false;
    }
    f32 senderRadius = MR::getSensorRadius(sender);
    f32 combinedRadius = MR::getSensorRadius(receiver);
    combinedRadius += senderRadius;
    const TVec3f* senderPosition = MR::getSensorPos(sender);
    TVec3f displacement(*MR::getSensorPos(receiver));
    displacement.sub(*senderPosition);
    if (MR::isNearZero(displacement, 0.001f)) {
        if (MR::isNearZero(mMovementVelocity, 0.001f)) {
            mBaseMtx.getZDir(displacement);
            displacement.negate();
        }
        else {
            TVec3f opposite(-mMovementVelocity);
            displacement.set(opposite);
        }
        displacement.setLength(combinedRadius);
    }
    else {
        f32 penetration = combinedRadius - PSVECMag(&displacement);
        if (penetration > 0.0f) displacement.setLength(penetration);
        else displacement.zero();
    }
    mVelocityAdjustment->include(displacement);
    return true;
}

void Yoshi::fn_8040FCD0() {
    mXanimePlayer = new XanimePlayer(MR::getJ3DModel(this), mAnimationResources);
    mXanimePlayer->setDefaultAnimation("基本");
    fn_8040EDA0("基本");
    XanimeCore* core = mXanimePlayer->getCore();
    core->enableJointTransform(MR::getJ3DModelData(this));
    mModelManager->mXanimePlayer = mXanimePlayer;
}

void Yoshi::fn_8040FD80() {
    mXanimePlayerUpper = new XanimePlayer(MR::getJ3DModel(this), mAnimationResources, mXanimePlayer);
    fn_8040EE00("基本");
    mXanimePlayerUpper->changeAnimationBck("基本");
    mXanimePlayerUpper->getCore()->shareJointTransform(mXanimePlayer->getCore());
    mXanimePlayerUpper->mAutoRunDefaultAnimation = false;
}

// NONMATCHING: string pool base and register allocation differ.
void Yoshi::fn_8040FE30() {
    initEffectKeeper(0, NULL, false);
    MR::setEffectName(this, "WaterSplashDash", "水はね");
    MR::setEffectName(this, "WaterSplashWalk", "水はね弱");
    MR::setEffectName(this, "WaterRipple", "水波紋");
    MR::setEffectName(this, "MudMove", "ヨッシー泥移動");
    MR::setEffectHostSRT(this, "水波紋", NULL, NULL, NULL);
}

bool Yoshi::fn_8040FEE0() const {
    bool result;

    result = false;
    if (((isNerve(&NrvYoshi::YoshiNrv80413860::sInstance) != 0) && (getNerveStep() > 0x1E)) || (isNerve(&NrvYoshi::YoshiNrv80413870::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413880::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413890::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138A0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138B0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138C0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138D0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138E0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138F0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413900::sInstance) != 0) || ((isNerve(&NrvYoshi::YoshiNrv80413910::sInstance) != 0) && _145) || (isNerve(&NrvYoshi::YoshiNrv80413990::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139A0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139B0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139C0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139D0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139E0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139F0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413A00::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413A10::sInstance) != 0)) {
        result = true;
    }
    return result;
}

bool Yoshi::fn_804100E0() const {
    bool result;

    result = false;
    if ((isNerve(&NrvYoshi::YoshiNrv80413840::sInstance) == 0) && (isNerve(&NrvYoshi::YoshiNrv80413920::sInstance) == 0) && (isNerve(&NrvYoshi::YoshiNrv80413910::sInstance) == 0) && (isNerve(&NrvYoshi::YoshiNrv80413990::sInstance) == 0) && (isNerve(&NrvYoshi::YoshiNrv804139A0::sInstance) == 0) && (isNerve(&NrvYoshi::YoshiNrv804139B0::sInstance) == 0) && (isNerve(&NrvYoshi::YoshiNrv804139C0::sInstance) == 0) && (isNerve(&NrvYoshi::YoshiNrv804139D0::sInstance) == 0) && (isNerve(&NrvYoshi::YoshiNrv804139E0::sInstance) == 0)) {
        result = true;
    }
    return result;
}

bool Yoshi::fn_804101D0() const {
    bool result;

    result = false;
    if ((isNerve(&NrvYoshi::YoshiNrv80413990::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139B0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139C0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139A0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139D0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139E0::sInstance) != 0)) {
        result = true;
    }
    return result;
}

void Yoshi::fn_80410280() {
    if (mGroundChecker->isAirborne()) {
        setNerve(&NrvYoshi::YoshiNrv804138D0::sInstance);
        return;
    }
    if (mGroundChecker->isGrounded()) {
        YoshiGroundChecker* ground = mGroundChecker;
        s32 slipping = 0;
        if (ground->isGrounded() && ground->mSlipping) slipping = 1;
        if (slipping) {
            setNerve(&NrvYoshi::YoshiNrv804138F0::sInstance);
            return;
        }
        if (*mJumpRequested) {
            setNerve(&NrvYoshi::YoshiNrv804138A0::sInstance);
            return;
        }
        setNerve(&NrvYoshi::YoshiNrv80413880::sInstance);
    }
}

void Yoshi::fn_80410350(const TVec3f& velocity) {
    MR::onBind(this);
    mMovementVelocity.set(velocity);
    mAutoDisappearTask->mTimer = 600;
    YoshiVelocityAdjustment* adjustment = mVelocityAdjustment;
    adjustment->mMinimum.zero();
    adjustment->mMaximum.zero();
    mDashTimer = 0;
    if (mFruitCtrl->mLightFruitActive) MR::endYoshiLightFruit();
    mTongue->fn_8041CBA0();
    _144 = false;
    _145 = false;
}

// NONMATCHING: named quaternion components replace the outlined vector alias; stack and FPR allocation differ.
void Yoshi::fn_804103F0() {
    MR::calcGravity(this);
    TVec3f up;
    mBaseMtx.getYDir(up);
    TVec3f front;
    mBaseMtx.getZDir(front);
    TVec3f side;
    mBaseMtx.getXDir(side);
    TQuat4f rotation;
    TVec3f angles;
    TVec3f gravityUp;
    JGeometry::negateInternal(&mGravity.x, &gravityUp.x);
    TVec3f axis;
    PSVECCrossProduct(&up, &gravityUp, &axis);
    f32 angle;
    f32 length = axis.length();
    if (length <= 0.000003814697265625f) rotation.set<f32>(0.0f, 0.0f, 0.0f, 1.0f);
    else {
        angle = 0.1f * (0.5f * JMAATan2(length, up.dot(gravityUp)));
        f32 factor = static_cast<f32>(::sin(angle)) / length;
        rotation.toTvec()->scale(factor, axis);
        rotation.w = static_cast<f32>(::cos(angle));
    }
    rotation.transform(up);
    rotation.transform(front);
    rotation.transform(side);
    mBaseMtx.setXDir(side);
    mBaseMtx.setYDir(up);
    mBaseMtx.setZDir(front);
    mBaseMtx.getEulerXYZ(angles);
    MR::setRotation(this, angles);
}

// NONMATCHING: quaternion arithmetic and argument scheduling differ.
void Yoshi::fn_80410570(f32 amount) {
    TVec3f direction(mMovementVelocity);
    YoshiGroundChecker* ground = mGroundChecker;
    bool slipping = false;
    if (ground->isGrounded() && ground->mSlipping) slipping = true;
    if (slipping) {
        const TVec3f* normal = mGroundChecker->mGroundHit.mParentTriangle.getNormal(0);
        JMAVECScaleAdd(normal, &direction, &direction, -normal->dot(direction));
    }
    const TVec3f* gravity = &mGravity;
    JMAVECScaleAdd(gravity, &direction, &direction, -gravity->dot(direction));
    if (!MR::normalizeOrZero(&direction)) {
        TVec3f front;
        mBaseMtx.getZDir(front);
        JMAVECScaleAdd(gravity, &front, &front, -gravity->dot(front));
        if (!MR::normalizeOrZero(&front)) {
            TVec3f position;
            mBaseMtx.getTrans(position);
            TVec3f cross;
            PSVECCrossProduct(&front, &direction, &cross);
            f32 angle = front.angle(direction);
            if (cross.dot(*gravity) < 0.0f) angle = -angle;
            TQuat4f rotation;
            rotation.setRotate(mGravity, angle * amount);
            TPos3f matrix;
            matrix.makeQuat(rotation);
            mBaseMtx.concat(matrix, mBaseMtx);
            mBaseMtx.setTrans(position);
            mBaseMtx.getEulerXYZ(mRotation);
        }
    }
}

void Yoshi::fn_804107D0(){mRandomTurn->reset();*mJumpRequested=true;}

bool Yoshi::fn_80410810(TVec3f* slope) const {
    const TVec3f* gravity = &mGravity;
    const TVec3f* normal = mGroundChecker->mGroundHit.mParentTriangle.getNormal(0);
    JMAVECScaleAdd(normal, gravity, slope, -normal->dot(*gravity));
    return !MR::normalizeOrZero(slope);
}

void Yoshi::fn_80410890() {
    TVec3f center;
    MR::copyJointPos(this, "Center", &center);
    MR::getWaterAreaObj(mWaterInfo, center);
    if (mWaterInfo->isInWater()) {
        _14C = center;
        MR::getWaterAreaInfo(mWaterInfo, _14C, mGravity, false);
        if (!_144 && !fn_80411CB0()) {
            _144 = true;
            fn_80411740();
        }
    } else if (!mGroundChecker->isGrounded()) {
        TVec3f displacement(_14C);
        displacement.sub(center);
        TVec3f planar;
        f32 vertical = MR::vecKillElement(displacement, mGravity, &planar);
        TVec3f position(center + mGravity.scaleInline(vertical));
        WaterInfo unused;
        MR::getWaterAreaObj(mWaterInfo, center);
        if (!mWaterInfo->isInWater()) _144 = false;
    } else _144 = false;
}

void Yoshi::fn_804109E0() {
    if (fn_80412310() && mGroundChecker->isGrounded()) {
        YoshiGroundChecker* ground = mGroundChecker;
        bool slipping = false;
        if (ground->isGrounded() && ground->mSlipping) slipping = true;
        if (slipping) {
            TVec3f slope;
            if (fn_80410810(&slope) && slope.dot(mMovementVelocity) < 0.0f) {
                JMAVECScaleAdd(&slope, &mMovementVelocity, &mMovementVelocity, -slope.dot(mMovementVelocity));
            }
        } else mMovementVelocity.zero();
    }
}

// NONMATCHING: verified scalar relocation labels only.
void Yoshi::fn_80410AC0() {
    YoshiGroundChecker* ground = mGroundChecker;
    const TVec3f& position = mPosition;
    TVec3f movement;
    if (!ground->isGrounded()) movement.zero();
    else ground->mGroundHit.mParentTriangle.calcForceMovePower(&movement, position);
    f32 vertical = mGravity.dot(movement);
    if (vertical < 0.0f) {
        TVec3f offset(mGravity);
        offset.scale(vertical);
        movement.sub(offset);
    }
    TVec3f adjustment(movement);
    adjustment.scale(0.5f);
    mAdditionalVelocity.add(adjustment);
}

void Yoshi::fn_80410BA0() {
    TVec3f planar;
    f32 fallSpeed = MR::vecKillElement(mMovementVelocity, mGravity, &planar);
    if (isNerve(&NrvYoshi::YoshiNrv804138A0::sInstance) || isNerve(&NrvYoshi::YoshiNrv80413990::sInstance)) {
        if (mMovementVelocity.dot(mGravity) >= 0.0f) {
            Triangle triangle;
            if (mGroundChecker->isGrounded() && fallSpeed < 20.0f) {
                TVec3f hit;
                const TVec3f& origin = mPosition + planar - mGravity * 10.0f;
                if (MR::getFirstPolyOnLineToMap(&hit, &triangle, origin, mGravity * 30.0f)) {
                    mAdditionalVelocity = mGravity * mGravity.dot(hit - mPosition);
                }
            }
        }
    }
    mMovementVelocity.add(mGravity);
    if (mMovementVelocity.dot(mGravity) > 50.0f) {
        const TVec3f* gravity = &mGravity;
        JMAVECScaleAdd(gravity, &mMovementVelocity, &mMovementVelocity, -gravity->dot(mMovementVelocity));
        mMovementVelocity.add(*gravity * 50.0f);
    }
}

// NONMATCHING: temporary stack layout, FPR allocation and argument scheduling differ.
void Yoshi::fn_80410D90() {
    if (mWaterInfo->isInWater()) {
        f32 depth = mWaterInfo->mCamWaterDepth;
        depth = depth < 200.0f ? depth : 200.0f;
        mMovementVelocity.x *= 0.98f;
        mMovementVelocity.y *= 0.98f;
        mMovementVelocity.z *= 0.98f;
        mMovementVelocity.add(mGravity * 0.3f);
        mMovementVelocity.add(-mGravity * depth * 0.007f);
        f32 speed = mMovementVelocity.dot(-mGravity);
        f32 maximum = mWaterInfo->mCamWaterDepth - 60.0f;
        if (maximum < speed) {
            f32 correctionSpeed = speed - maximum;
            mMovementVelocity.add(mGravity * correctionSpeed);
        }
    } else {
        if (mMovementVelocity.dot(mGravity) < 0.0f) mMovementVelocity.zero();
        mMovementVelocity.add(mGravity * 0.3f);
    }
    if (!mWaterInfo->isInWater() || (mWaterInfo->isInWater() && mWaterInfo->mCamWaterDepth < 200.0f)) {
        _145 = true;
        TQuat4f orientation;
        orientation.setRotate(TVec3f(0.0f, 1.0f, 0.0f), -mGravity);
        TVec3f side(1.0f, 0.0f, 0.0f);
        orientation.transform(side);
        TVec3f front(0.0f, 0.0f, 1.0f);
        orientation.transform(front);
        f32 strength;
        if (_158 < 10) strength = static_cast<f32>(_158) / 10.0f;
        else {
            f32 progress = 1.0f - static_cast<f32>(180 - _158) / 120.0f;
            strength = 1.0f - progress * progress;
            if (strength > 1.0f) strength = 1.0f;
            if (strength < 0.1f) strength = 0.1f;
        }
        f32 sway = MR::sin(0.1f + 0.09f * static_cast<f32>(getNerveStep()) + 0.01f * mPosition.x);
        side.add(mGravity * 0.2f * sway * strength);
        f32 bob = MR::sin(0.2f + 0.09f * (1.2f * static_cast<f32>(getNerveStep())) + 0.01f * mPosition.y);
        front.add(mGravity * 0.36f * bob * strength);
        TVec3f normal;
        PSVECCrossProduct(&front, &side, &normal);
        if (!MR::normalizeOrZero(&normal)) {
            TVec3f oldSide;
            mBaseMtx.getXDir(oldSide);
            TVec3f oldUp;
            mBaseMtx.getYDir(oldUp);
            TVec3f oldFront;
            TQuat4f tilt;
            tilt.setRotate(oldUp, normal);
            tilt.transform(oldSide);
            tilt.transform(oldUp);
            MR::normalize(&oldUp);
            PSVECCrossProduct(&oldSide, &oldUp, &oldFront);
            MR::normalize(&oldFront);
            PSVECCrossProduct(&oldUp, &oldFront, &oldSide);
            MR::normalize(&oldSide);
            mBaseMtx.setXDir(oldSide);
            mBaseMtx.setYDir(oldUp);
            mBaseMtx.setZDir(oldFront);
            mBaseMtx.getEulerXYZ(mRotation);
            ++_158;
        }
    } else {
        _158 = 0;
        _145 = false;
    }
}

bool Yoshi::fn_804112A0() const {
    TVec3f unusedPosition;
    mBaseMtx.getTrans(unusedPosition);
    TVec3f up;
    mBaseMtx.getYDir(up);
    TVec3f front;
    mBaseMtx.getZDir(front);
    TVec3f origin = mPosition + up.scaleInline(100.0f);
    TVec3f ray(front);
    ray.scale(150.0f);
    if (Collision::checkStrikeLineToMap(origin, ray, 0, nullptr, nullptr)) return false;
    TVec3f end(origin);
    end.add(ray);
    TVec3f drop(TVec3f(-up));
    drop.scale(300.0f);
    return !Collision::checkStrikeLineToMap(end, drop, 0, nullptr, nullptr);
}

bool Yoshi::fn_804113C0() {
    if (fn_80411410()) return true;
    return fn_80411550();
}

bool Yoshi::fn_80411410() {
    bool running;
    AreaObj* area = MR::getAreaObj("PlaneModeCube", mPosition);
    if (!area) return false;
    if (!MR::isAreaFormCube(area)) return false;
    running = false;
    if (isNerve(&NrvYoshi::YoshiNrv804138A0::sInstance) || isNerve(&NrvYoshi::YoshiNrv804138B0::sInstance)) running = true;
    if (running) setNerve(&NrvYoshi::YoshiNrv80413990::sInstance);
    TVec3f origin;
    MR::calcCubePos(area, &origin);
    TVec3f axis;
    MR::calcCubeAxisZ(area, &axis);
    TVec3f displacement(mPosition);
    displacement.sub(origin);
    f32 distance = axis.dot(displacement);
    TVec3f position = mPosition - axis.scaleInline(distance);
    MR::setPosition(this, position);
    MR::calcCubeAxisZ(area, &_18C);
    return true;
}

bool Yoshi::fn_80411550(){
    TVec3f center,axis;f32 radius;if(!MR::isInTowerModeCylinder(mPosition,&center,&axis,&radius))return false;
    bool inAir=false;if(isNerve(&NrvYoshi::YoshiNrv804138A0::sInstance)||isNerve(&NrvYoshi::YoshiNrv804138B0::sInstance))inAir=true;
    if(inAir)setNerve(&NrvYoshi::YoshiNrv80413990::sInstance);
    TVec3f position;MR::calcPlayerScaledVec(&position,mPosition,center,axis,radius);MR::setPosition(this,position);MR::calcPlayerKillDiffVec(&_18C,mPosition,center,axis);return true;
}

// NONMATCHING: verified scalar relocation labels only.
void Yoshi::fn_80411630() {
    const TVec3f* gravity = &mGravity;
    TVec3f velocity;
    JMAVECScaleAdd(gravity, &mMovementVelocity, &velocity, -gravity->dot(mMovementVelocity));
    JGeometry::negateInternal(&velocity.x, &velocity.x);
    velocity.scale(2.0f);
    mMovementVelocity.add(velocity);
}

void Yoshi::fn_804116B0() {
    if (isNerve(&NrvYoshi::YoshiNrv80413840::sInstance)) {
        if (MR::isHiddenModel(mRideModel)) {
            if (mRideModelVisible) MR::showModel(mRideModel);
        }
        else if (!mRideModelVisible) {
            MR::hideModel(mRideModel);
        }
    }
    else if (MR::isHiddenModel(mRideModel)) {
        MR::showModel(mRideModel);
    }
}

void Yoshi::fn_80411740() {
    mBreakModelMtx.fn_80409700(getBaseMtx());
    if (mWaterInfo->isInWater()) {
        TVec3f up;
        mBreakModelMtx.getYAxis(&up);
        MarioMtxPos surface;
        surface.fn_80409710(up, mWaterInfo->mSurfaceNormal);
        mBreakModelMtx.preConcat(surface);
        mBreakModelMtx.setTranslation(mWaterInfo->mSurfacePos);
    }
    mBreakModel->appear();
}

void Yoshi::fn_804117F0(){if(mSupportSpinTimer>0){--mSupportSpinTimer;return;}if(fn_80411860()&&MR::isStarPointerPointing2PAttachTarget(this))setNerve(&NrvYoshi::YoshiNrv804139F0::sInstance);}

bool Yoshi::fn_80411860()const{
    static const Nerve* nerves[]={getYoshiNerve<NrvYoshi::YoshiNrv80413840>(),getYoshiNerve<NrvYoshi::YoshiNrv80413860>(),getYoshiNerve<NrvYoshi::YoshiNrv80413910>(),getYoshiNerve<NrvYoshi::YoshiNrv80413920>(),getYoshiNerve<NrvYoshi::YoshiNrv80413930>(),getYoshiNerve<NrvYoshi::YoshiNrv80413940>(),getYoshiNerve<NrvYoshi::YoshiNrv80413950>(),getYoshiNerve<NrvYoshi::YoshiNrv80413960>(),getYoshiNerve<NrvYoshi::YoshiNrv80413970>(),getYoshiNerve<NrvYoshi::YoshiNrv80413980>(),getYoshiNerve<NrvYoshi::YoshiNrv80413850>(),getYoshiNerve<NrvYoshi::YoshiNrv804139F0>(),getYoshiNerve<NrvYoshi::YoshiNrv80413A00>(),getYoshiNerve<NrvYoshi::YoshiNrv80413A10>()};
    for(u32 i=0;i<14;++i)if(isNerve(nerves[i]))return false;return true;
}

bool Yoshi::fn_80411960() const {
    bool result;

    result = false;
    if ((isNerve(&NrvYoshi::YoshiNrv80413870::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413880::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413890::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138A0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138B0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138C0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413A10::sInstance) != 0)) {
        result = true;
    }
    return result;
}

void Yoshi::fn_80411A30() {
    if (mUpperAnimationActive) {
        XanimePlayer* upper = mXanimePlayerUpper;
        upper->updateBeforeMovement();
        upper->updateAfterMovement();
        if (mXanimePlayerUpper->fn_8040F600() && mXanimePlayerUpper->mFrameCtrl->mAttribute != 1 && !mUpperDefaultAnimationEnabled) fn_8040EE70(nullptr);
    }
}

bool Yoshi::fn_80411AC0() const {
    bool result = false;
    if (isNerve(&NrvYoshi::YoshiNrv80413870::sInstance) || isNerve(&NrvYoshi::YoshiNrv80413880::sInstance) || isNerve(&NrvYoshi::YoshiNrv80413890::sInstance) || isNerve(&NrvYoshi::YoshiNrv804138A0::sInstance) || isNerve(&NrvYoshi::YoshiNrv804138B0::sInstance) || isNerve(&NrvYoshi::YoshiNrv804138C0::sInstance) || isNerve(&NrvYoshi::YoshiNrv804138D0::sInstance) || isNerve(&NrvYoshi::YoshiNrv804138E0::sInstance) || isNerve(&NrvYoshi::YoshiNrv804138F0::sInstance) || isNerve(&NrvYoshi::YoshiNrv80413900::sInstance) || isNerve(&NrvYoshi::YoshiNrv80413A00::sInstance) || isNerve(&NrvYoshi::YoshiNrv80413920::sInstance)) result = true;
    return result;
}

bool Yoshi::fn_80411BF0() {
    if (!fn_80411AC0()) return false;
    if (MR::isBindedGroundDamageFire(this)) {
        fn_80410350(TVec3f(0.0f));
        MR::fn_80422520();
        setNerve(&NrvYoshi::YoshiNrv80413960::sInstance);
        return true;
    }
    if (MR::isBindedGroundDamageElectric(this)) {
        fn_80410350(TVec3f(0.0f));
        MR::fn_80422550();
        setNerve(&NrvYoshi::YoshiNrv80413970::sInstance);
        return true;
    }
    return false;
}

bool Yoshi::fn_80411CB0() const {
    TVec3f hit;
    Triangle triangle;
    TVec3f ray(mGravity);
    ray.scale(70.0f);
    return MR::getFirstPolyOnLineToMap(&hit, &triangle, _14C, ray);
}

bool Yoshi::fn_80411D10() const {
    bool result;

    result = false;
    if ((isNerve(&NrvYoshi::YoshiNrv80413870::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413880::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413890::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138A0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138B0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138C0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138D0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138E0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138F0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413900::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413910::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413990::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139A0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139B0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139C0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139D0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139E0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413A00::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413A10::sInstance) != 0)) {
        result = true;
    }
    return result;
}

void Yoshi::fn_80411ED0() {
    if (!isNerve(&NrvYoshi::YoshiNrv80413840::sInstance)) {
        WaterInfo info;
        if (MR::getWaterAreaObj(&info, mPosition)) MR::getWaterAreaInfo(&info, mPosition, mGravity, false);
        if (isNerve(&NrvYoshi::YoshiNrv804138A0::sInstance) || isNerve(&NrvYoshi::YoshiNrv80413990::sInstance)) {
            if (MR::isFirstStep(this)) _15C = 0.0f;
            f32 frame = mXanimePlayer->getAnimationFrame();
            if (info.isInWater()) {
                if (fn_80412590(frame)) {
                    MR::emitEffect(this, "WaterSplashWalkAlone");
                    fn_804121A0(info.mSurfacePos, info.mSurfaceNormal);
                }
            } else {
                bool waterBottom = false;
                if (MR::isBindedGroundWaterBottomH(this) || MR::isBindedGroundWaterBottomM(this) || MR::isBindedGroundWaterBottomL(this)) waterBottom = true;
                if (waterBottom && fn_80412590(frame)) {
                    MR::emitEffect(this, "WaterSplashWalkAlone");
                    TVec3f up(MR::getGroundNormal(this));
                    up.scale(10.0f);
                    TVec3f position(mPosition);
                    position.add(up);
                    fn_804121A0(position, MR::getGroundNormal(this));
                }
            }
            _15C = frame;
            return;
        }
        if (!isNerve(&NrvYoshi::YoshiNrv80413910::sInstance)) {
            if (info.isInWater()) {
                ++_160;
                if (_160 == 60) {
                    fn_804121A0(info.mSurfacePos, info.mSurfaceNormal);
                    _160 = 0;
                }
            } else {
                bool waterBottom = false;
                if (MR::isBindedGroundWaterBottomH(this) || MR::isBindedGroundWaterBottomM(this) || MR::isBindedGroundWaterBottomL(this)) waterBottom = true;
                if (waterBottom) {
                    ++_160;
                    if (_160 == 60) {
                        TVec3f position = mPosition + MR::getGroundNormal(this).scaleInline(10.0f);
                        fn_804121A0(position, MR::getGroundNormal(this));
                        _160 = 0;
                    }
                }
            }
        }
    }
}

void Yoshi::fn_804121A0(const TVec3f& position, const TVec3f& normal) {
    TPos3f matrix;
    MR::makeMtxUpNoSupportPos(&matrix, normal, TVec3f(0.0f));
    TVec3f rotation;
    MR::makeRTFromMtxPtr(nullptr, &rotation, matrix.toMtxPtr(), true);
    MultiEmitter* emitter = MR::tryEmitEffect(this, "水波紋");
    if (emitter) {
        TVec3f scale;
        scale.x = 0.2f;
        scale.y = 0.2f;
        scale.z = 0.2f;
        emitter->setGlobalScale(scale, -1);
    }
    if (emitter) emitter->setGlobalTranslation(position, -1);
    if (emitter) emitter->setGlobalRotationDegree(rotation, -1);
}

bool Yoshi::fn_80412270() const {
    bool result;

    result = false;
    if ((isNerve(&NrvYoshi::YoshiNrv80413980::sInstance) == 0) && (isNerve(&NrvYoshi::YoshiNrv80413850::sInstance) == 0) && (isNerve(&NrvYoshi::YoshiNrv80413910::sInstance) == 0) && (isNerve(&NrvYoshi::YoshiNrv804139F0::sInstance) == 0) && (isNerve(&NrvYoshi::YoshiNrv80413A00::sInstance) == 0)) {
        result = true;
    }
    return result;
}

bool Yoshi::fn_80412310() const {
    bool result;

    result = false;
    if ((isNerve(&NrvYoshi::YoshiNrv80413860::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413870::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413880::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413890::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138B0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138C0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138E0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138F0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413900::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413920::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413930::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413940::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413960::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413970::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139A0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139C0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139D0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139E0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413A10::sInstance) != 0)) {
        result = true;
    }
    return result;
}

void Yoshi::fn_804124D0(f32 speed) {
    TVec3f up;
    mBaseMtx.getYDir(up);
    TVec3f front;
    mBaseMtx.getZDir(front);
    TQuat4f rotation;
    TVec3f gravityUp;
    JGeometry::negateInternal(&mGravity.x, &gravityUp.x);
    rotation.setRotate(up, gravityUp);
    rotation.transform(front);
    front.setLength(speed);
    f32 vertical = mGravity.dot(mMovementVelocity);
    TVec3f velocity(mGravity);
    velocity.scale(vertical);
    mMovementVelocity = velocity;
    mMovementVelocity.add(front);
}

bool Yoshi::fn_80412590(f32 frame) const {
    bool crossing = false;
    if ((_15C < 3.0f && frame >= 3.0f) || (_15C < 12.0f && frame >= 12.0f) || (_15C < 20.0f && frame >= 20.0f) || (_15C < 29.0f && frame >= 29.0f)) crossing = true;
    return crossing;
}

void YoshiUpdateTaskList::add(YoshiUpdateTask* task) {
    s32 index = mCount;
    mCount = index + 1;
    mTasks[index] = task;
}

void YoshiUpdateTaskList::update() {
    for (s32 i = 0; i < mCount; i++) {
        mTasks[i]->update();
    }
}

void YoshiGroundChecker::update() {
    if (MR::isOnGround(mActor)) {
        mAirborneFrames = 0;
        const HitInfo& ground = mActor->mBinder->mGroundInfo;
        mGroundHit.mParentTriangle.mParts = ground.mParentTriangle.mParts;
        mGroundHit.mParentTriangle.mIdx = ground.mParentTriangle.mIdx;
        mGroundHit.mParentTriangle.mSensor = ground.mParentTriangle.mSensor;
        mGroundHit.mParentTriangle.mNormals[0] = ground.mParentTriangle.mNormals[0];
        mGroundHit.mParentTriangle.mNormals[1] = ground.mParentTriangle.mNormals[1];
        mGroundHit.mParentTriangle.mNormals[2] = ground.mParentTriangle.mNormals[2];
        mGroundHit.mParentTriangle.mNormals[3] = ground.mParentTriangle.mNormals[3];
        mGroundHit.mParentTriangle.mPos[0] = ground.mParentTriangle.mPos[0];
        mGroundHit.mParentTriangle.mPos[1] = ground.mParentTriangle.mPos[1];
        mGroundHit.mParentTriangle.mPos[2] = ground.mParentTriangle.mPos[2];
        mGroundHit.mDistance = ground.mDistance;
        mGroundHit.mHitPos = ground.mHitPos;
        mGroundHit.mQueryPos = ground.mQueryPos;
        mGroundHit.mNormalMovement = ground.mNormalMovement;
        mGroundHit.mCollisionType = ground.mCollisionType;
        if (MR::isBindedGroundSlip(mActor)) {
            mSlipping = true;
        }
        else {
            TVec3f up(-mActor->mGravity);
            f32 slope = MR::getGroundNormal(mActor).dot(up);
            if (slope <= MR::cos(0.753982246f)) {
                mSlipping = true;
            }
            else if (slope >= MR::cos(2.98451304f)) {
                mSlipping = false;
            }
        }
    }
    else {
        mAirborneFrames++;
    }
    bool airborne = false;
    if (mUpdated == true && mAirborneFrames > 4) airborne = true;
    if (airborne) {
        mSlipping = false;
    }
    mUpdated = true;
}

bool YoshiGroundChecker::isGrounded() const {
    bool result = false;
    if (mUpdated == true) {
        bool airborne = false;
        if (mUpdated == true && mAirborneFrames > 4) airborne = true;
        if (!airborne) result = true;
    }
    return result;
}

bool YoshiGroundChecker::isAirborne() const {
    bool result = false;
    if (mUpdated == true && mAirborneFrames > 4) result = true;
    return result;
}

void YoshiAutoDisappearTask::update() {
    if (!MR::isDemoActive()) {
        if (mTimer > 0) mTimer--;
        if (MR::isInAreaObj("DeathArea", mActor->mPosition)) mTimer = 0;
    }
}

void YoshiRandomTurn::reset() {
    mTimer = MR::getRandom((s32)120, (s32)180);
    mTurnTimer = MR::getRandom((s32)5, (s32)90);
    f32 angle;
    if (MR::getRandom() < 0.5f) angle = -0.0349065848f;
    else angle = 0.0349065848f;
    mTurnAngle = angle;
}

void YoshiRandomTurn::update(const TPos3f& matrix, TVec3f* direction) {
    if (mTimer > 0) {
        mTimer--;
        if (mTurnTimer > 0) {
            mTurnTimer--;
            TVec3f up;
            matrix.getYDir(up);
            TQuat4f rotation;
            rotation.setRotate(up, mTurnAngle);
            rotation.transform(*direction);
        }
    }
}

void YoshiEscapeDirection::reset() {
    mStep = 0;
    mHitCount = 0;
    mDirection.zero();
}

// NONMATCHING: vector temporary inlining, stack slots and argument scheduling differ.
void YoshiEscapeDirection::update() {
    if (mStep < 16) {
        mStep++;
        TVec3f direction(0.0f, 0.0f, 150.0f);
        TVec3f axis(0.0f, 1.0f, 0.0f);
        TQuat4f rotation;
        rotation.setRotate(axis, 6.28318548f * mStep * 0.0625f);
        rotation.transform(direction);
        TVec3f start(0.0f, 100.0f, 0.0f);
        TVec3f ray(direction);
        mBaseMtx->mult(start, start);
        mBaseMtx->mult33(ray);
        if (Collision::checkStrikeLineToMap(start, ray, 0, NULL, NULL)) {
            mHitCount++;
            mDirection.add(direction);
        }
        else {
            TVec3f end(TVec3f(0.0f, 100.0f, 0.0f) + direction);
            TVec3f down(0.0f, -300.0f, 0.0f);
            mBaseMtx->mult(end, end);
            mBaseMtx->mult33(down);
            if (Collision::checkStrikeLineToMap(end, down, 0, NULL, NULL)) {
                mHitCount++;
                mDirection.add(direction);
            }
        }
        if (mStep == 16) finish();
    }
}

void YoshiEscapeDirection::finish() {
    if (mHitCount > 0) mDirection.scale(1.0f / mHitCount);
    if (MR::normalizeOrZero(&mDirection)) {
        mBaseMtx->getZDir(mDirection);
        mDirection.negate();
    }
    else {
        mBaseMtx->mult33(mDirection);
    }
}

void YoshiVelocityAdjustment::include(const TVec3f& vector) {
    if (mMinimum.x > vector.x) mMinimum.x = vector.x;
    if (mMinimum.y > vector.y) mMinimum.y = vector.y;
    if (mMinimum.z > vector.z) mMinimum.z = vector.z;
    if (mMaximum.x < vector.x) mMaximum.x = vector.x;
    if (mMaximum.y < vector.y) mMaximum.y = vector.y;
    if (mMaximum.z < vector.z) mMaximum.z = vector.z;
}

// NONMATCHING: vector scale is inlined; argument scheduling differs.
void YoshiVelocityAdjustment::apply(Yoshi* actor) {
    TVec3f adjustment(mMinimum);
    adjustment.add(mMaximum);
    TVec3f direction(adjustment);
    MR::normalizeOrZero(&direction);
    f32 speed = direction.dot(actor->mMovementVelocity);
    if (speed < 0.0f) {
        TVec3f* velocity = &actor->mMovementVelocity;
        JMAVECScaleAdd(&direction, velocity, velocity, -direction.dot(*velocity));
        f32 rebound = speed * 0.95f;
        TVec3f impulse(direction);
        impulse.scale(rebound);
        velocity->add(impulse);
    }
    actor->mMovementVelocity.add(adjustment);
}

void YoshiVelocityAdjustment::remove(Yoshi* actor) {
    TVec3f adjustment(mMinimum);
    adjustment.add(mMaximum);
    actor->mMovementVelocity.sub(adjustment);
}

// NONMATCHING: verified scalar relocation labels only.
bool YoshiPlayerFacingCtrl::fn_80413040() {
    TVec3f position;
    mBaseMtx->getTrans(position);
    TVec3f front;
    mBaseMtx->getZDir(front);
    TVec3f toPlayer(*MR::getPlayerPos());
    toPlayer.sub(position);
    if (MR::normalizeOrZero(&toPlayer)) {
        mTimer = 0;
        return false;
    }
    if (front.angle(toPlayer) > 0.785398185f) {
        mTimer++;
    } else {
        mTimer = 0;
        return false;
    }
    return mTimer > 15;
}

void YoshiPlayerFacingCtrl::fn_80413110(){TVec3f position;mBaseMtx->getTrans(position);TVec3f direction(*MR::getPlayerPos());direction.sub(position);if(MR::normalizeOrZero(&direction))mBaseMtx->getZDir(direction);fn_8040BD00(mBaseMtx,direction,1.0f,0.05235987901687622f);if(fn_804131C0(direction))++mTurnTimer;else mTurnTimer=0;}

bool YoshiPlayerFacingCtrl::fn_804131C0(const TVec3f& direction) {
    TVec3f up;
    mBaseMtx->getYDir(up);
    TVec3f front;
    mBaseMtx->getZDir(front);
    JMAVECScaleAdd(&up, &front, &front, -up.dot(front));
    if (MR::normalizeOrZero(&front)) return true;
    TVec3f projected;
    JMAVECScaleAdd(&up, &direction, &projected, -up.dot(direction));
    if (MR::normalizeOrZero(&projected)) return true;
    return projected.angle(front) < 0.0349065848f;
}

void YoshiModelVisibility::fn_804132C0() {
    mEnabled = true;
    MR::showModel(mActor);
    MR::showModel(mActor->mTongue);
    if (mAppeared) mModelCtrl->appear();
}

void YoshiModelVisibility::fn_80413330() {
    mEnabled = false;
    MR::hideModel(mActor);
    MR::hideModel(mActor->mTongue);
    mModelCtrl->kill();
}

void YoshiAnimationSync::update() {
    const char* animation;
    if (mActor->mUpperAnimationActive) {
        animation = mActor->mXanimePlayerUpper->getCurrentBckName();
    }
    else {
        animation = mActor->mXanimePlayer->getCurrentBckName();
    }
    if (mAnimation != NULL && MR::isEqualString(mAnimation, animation)) {
        fn_80413440();
    }
    else {
        fn_804135A0(animation);
    }
}

void YoshiAnimationSync::fn_80413440() {
    if (_8) {
        if (mActor->mUpperAnimationActive) {
            MR::setBvaRate(mActor, mActor->mXanimePlayerUpper->mFrameCtrl->mRate);
            MR::setBvaFrame(mActor, mActor->mXanimePlayerUpper->mFrameCtrl->mFrame);
        } else {
            MR::setBvaRate(mActor, mActor->mXanimePlayer->mFrameCtrl->mRate);
            MR::setBvaFrame(mActor, mActor->mXanimePlayer->mFrameCtrl->mFrame);
        }
    }
    if (_9) {
        if (mActor->mUpperAnimationActive) {
            MR::setBtpRate(mActor, mActor->mXanimePlayerUpper->mFrameCtrl->mRate);
            MR::setBtpFrame(mActor, mActor->mXanimePlayerUpper->mFrameCtrl->mFrame);
        } else {
            MR::setBtpRate(mActor, mActor->mXanimePlayer->mFrameCtrl->mRate);
            MR::setBtpFrame(mActor, mActor->mXanimePlayer->mFrameCtrl->mFrame);
        }
    }
    if (_A) {
        if (mActor->mUpperAnimationActive) {
            MR::setBtkRate(mActor, mActor->mXanimePlayerUpper->mFrameCtrl->mRate);
            MR::setBtkFrame(mActor, mActor->mXanimePlayerUpper->mFrameCtrl->mFrame);
        } else {
            MR::setBtkRate(mActor, mActor->mXanimePlayer->mFrameCtrl->mRate);
            MR::setBtkFrame(mActor, mActor->mXanimePlayer->mFrameCtrl->mFrame);
        }
    }
}

void YoshiAnimationSync::fn_804135A0(const char* animation) {
    if (MR::isExistBva(mActor, animation)) {
        MR::startBva(mActor, animation);
        _8 = true;
    } else {
        MR::startBva(mActor, "Blink");
        _8 = false;
    }
    if (MR::isExistBtp(mActor, animation)) {
        MR::startBtp(mActor, animation);
        _9 = true;
    } else {
        if (MR::isExistBtp(mActor, "Blink")) MR::startBtp(mActor, "Blink");
        _9 = false;
    }
    if (MR::isExistBtk(mActor, animation)) {
        MR::startBtk(mActor, animation);
        _A = true;
    } else {
        if (MR::isExistBtk(mActor, "Normal")) MR::startBtk(mActor, "Normal");
        _A = false;
    }
    mAnimation = animation;
}


YoshiVelocityAdjustment::YoshiVelocityAdjustment() : mMinimum(0.0f) {
    mMaximum.x = 0.0f;
    mMaximum.y = 0.0f;
    mMaximum.z = 0.0f;
}


void NrvYoshi::YoshiNrv80413A10::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040EBF0();
}

void NrvYoshi::YoshiNrv80413A00::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040EB60();
}

void NrvYoshi::YoshiNrv804139F0::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040EAE0();
}

void NrvYoshi::YoshiNrv804139E0::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040E9E0();
}

void NrvYoshi::YoshiNrv804139D0::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040E8F0();
}

void NrvYoshi::YoshiNrv804139C0::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040E820();
}

void NrvYoshi::YoshiNrv804139B0::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040E750();
}

void NrvYoshi::YoshiNrv804139A0::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040E660();
}

void NrvYoshi::YoshiNrv80413990::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040E350();
}

void NrvYoshi::YoshiNrv80413980::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040E230();
}

void NrvYoshi::YoshiNrv80413970::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040E160();
}

void NrvYoshi::YoshiNrv80413960::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040E080();
}

void NrvYoshi::YoshiNrv80413950::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040DFB0();
}

void NrvYoshi::YoshiNrv80413940::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040DF10();
}

void NrvYoshi::YoshiNrv80413930::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040DE20();
}

void NrvYoshi::YoshiNrv80413920::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040DDA0();
}

void NrvYoshi::YoshiNrv80413910::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040DC20();
}

void NrvYoshi::YoshiNrv80413900::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040DB10();
}

void NrvYoshi::YoshiNrv804138F0::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040D9F0();
}

void NrvYoshi::YoshiNrv804138E0::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040D8E0();
}

void NrvYoshi::YoshiNrv804138D0::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040D7F0();
}

void NrvYoshi::YoshiNrv804138C0::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040D6E0();
}

void NrvYoshi::YoshiNrv804138B0::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040D5B0();
}

void NrvYoshi::YoshiNrv804138A0::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040D380();
}

void NrvYoshi::YoshiNrv80413890::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040D260();
}

void NrvYoshi::YoshiNrv80413880::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040D130();
}

void NrvYoshi::YoshiNrv80413870::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040CFC0();
}

void NrvYoshi::YoshiNrv80413860::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040CF50();
}

void NrvYoshi::YoshiNrv80413850::execute(Spine* spine) const {
    static_cast<Yoshi*>(spine->mExecutor)->fn_8040E2D0();
}

void NrvYoshi::YoshiNrv80413840::execute(Spine* spine) const {
    MR::setStageBGMStateBit(8);
}

void NrvYoshi::YoshiNrv80413840::executeOnEnd(Spine* spine) const {
    Yoshi* actor = static_cast<Yoshi*>(spine->mExecutor);
    actor->fn_8040F370();
    MR::validateHitSensors(actor);
}

YoshiUpdateTaskList::YoshiUpdateTaskList(s32 capacity) : mTasks(NULL), mCapacity(0), mCount(0) {
    mTasks = new YoshiUpdateTask*[capacity];
    mCapacity = capacity;
}


YoshiAnimationSync::YoshiAnimationSync(Yoshi* actor) : mActor(actor), mAnimation(NULL), _8(false), _9(false), _A(false) {
}


YoshiGroundChecker::YoshiGroundChecker(Yoshi* actor) : mActor(actor), mAirborneFrames(0), mUpdated(false), mSlipping(false) {
}

YoshiAutoDisappearTask::YoshiAutoDisappearTask(Yoshi* actor) : mActor(actor), mTimer(0) {
}

YoshiRandomTurn::YoshiRandomTurn() : mTimer(0), mTurnTimer(0), mTurnAngle(0.0f) {
}

YoshiEscapeDirection::YoshiEscapeDirection(TPos3f* matrix) : mBaseMtx(matrix), mStep(0), mHitCount(0), mDirection(0.0f, 0.0f, 0.0f) {
}

YoshiPlayerFacingCtrl::YoshiPlayerFacingCtrl(TPos3f* matrix) : mBaseMtx(matrix), mTimer(0), mTurnTimer(0) {
}

Yoshi::Yoshi(const char* name) : LiveActor(name), mAnimationResources(NULL), mXanimePlayer(NULL), mXanimePlayerUpper(NULL), _A0(NULL),
    mUpperDefaultAnimationEnabled(false), mUpperAnimationActive(false), mBinderOffset(0.0f, 0.0f, 0.0f), mTongue(NULL), mYoshiModelCtrl(NULL),
    mHeadCtrl(NULL), mAnimationModelCtrl(NULL), mFruitCtrl(NULL), mUpdateTasks(NULL), mGroundChecker(NULL), mAutoDisappearTask(NULL),
    mRandomTurn(NULL), mJumpRequested(NULL), _130(false), mDashTimer(0), mDashEffectActive(false), mDashTopWaterEffectActive(false),
    _13C(0), mSupportSpinTimer(0), _144(false), _145(false), mWaterInfo(new WaterInfo()), _14C(0.0f, 0.0f, 0.0f), _158(0), _15C(0.0f), _160(0),
    mMovementVelocity(0.0f, 0.0f, 0.0f), mPreviousVelocity(0.0f, 0.0f, 0.0f), _188(0), _18C(0.0f, 0.0f, 0.0f), _198(false), mRideModel(NULL), mRideModelVisible(false),
    _1A1(false), _1A2(false), mBreakModel(NULL) {
    mBaseMtx.identity();
    mUpdateTasks = new YoshiUpdateTaskList(16);
    mGroundChecker = new YoshiGroundChecker(this);
    mUpdateTasks->add(mGroundChecker);
    mAutoDisappearTask = new YoshiAutoDisappearTask(this);
    mUpdateTasks->add(mAutoDisappearTask);
    mRandomTurn = new YoshiRandomTurn();
    mJumpRequested = new bool(false);
    mEscapeDirection = new YoshiEscapeDirection(&mBaseMtx);
    mVelocityAdjustment = new YoshiVelocityAdjustment();
    mPlayerFacingCtrl = new YoshiPlayerFacingCtrl(&mBaseMtx);
    mModelVisibility = NULL;
    mAnimationSync = new YoshiAnimationSync(this);
    mBreakModelMtx.identity();
}

Yoshi::~Yoshi() {
}


YoshiModelVisibility::YoshiModelVisibility(Yoshi* actor, YoshiAnimationModelCtrl* model) : mActor(actor), mModelCtrl(model), mEnabled(true), mAppeared(false) {
}


bool XanimePlayer::fn_8040F600() const {
    return (mFrameCtrls[mCurrentFrameCtrlIndex].mState & 1) != 0;
}
