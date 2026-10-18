#include "Game/Player/Yoshi.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/LiveActor/EffectKeeper.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JMath/JMath.hpp>
#include <JSystem/J3DGraphAnimator/J3DModel.hpp>
#include <cstring>

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

// NONMATCHING: cached player/core loads are scheduled differently.
void Yoshi::fn_8040EE70(const char* animation) {
    if (mUpperAnimationActive && (animation == NULL || mXanimePlayerUpper->isRun(animation))) {
        XanimePlayer* player = mXanimePlayer;
        u16 joint = MR::getJointIndex(this, "Head");
        XanimeCore* core = player->getCore();
        mXanimePlayerUpper->getCore()->freezeCopy(MR::getJ3DModel(this)->getModelData(), core, joint, 8);
        mXanimePlayerUpper->stopAnimation();
        mUpperAnimationActive = false;
    }
}

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

f32 Yoshi::fn_8040F620() const {
    return mXanimePlayer->getFrameCtrl()->getFrame();
}

void Yoshi::kill() {
    LiveActor::kill();
    mDashTimer = 0;
}

void Yoshi::fn_8040CF20() {
    if (mRideCountdown > 0) {
        mRideCountdown--;
    }
}

bool Yoshi::fn_8040CF40() const {
    return mRideCountdown == 0;
}

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

u16 Yoshi::fn_8040F190() const {
    return getSensor("AutoEat")->mSensorCount;
}

HitSensor* Yoshi::fn_8040F1C0(s32 index) const {
    return getSensor("AutoEat")->mSensors[index];
}

const TVec3f& Yoshi::fn_8040F200() const {
    return getSensor("AutoEat")->mPosition;
}

// NONMATCHING: Euler extraction is inlined; matrix copy scheduling differs.
void Yoshi::fn_8040F630(const TPos3f& matrix) {
    mBaseMtx.set(matrix);
    MR::setPosition(this, matrix.mMtx[0][3], matrix.mMtx[1][3], matrix.mMtx[2][3]);
    TVec3f rotation;
    mBaseMtx.getEulerXYZ(rotation);
    MR::setRotation(this, rotation);
}


void Yoshi::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mBaseMtx);
    MR::setBaseScale(this, mScale);
}

YoshiVelocityAdjustment::YoshiVelocityAdjustment() : mMinimum(0.0f) {
    mMaximum.x = 0.0f;
    mMaximum.y = 0.0f;
    mMaximum.z = 0.0f;
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

void Yoshi::movement() {
    TVec3f previousPosition(mPosition);
    LiveActor::movement();
    mVelocityAdjustment->remove(this);
    YoshiVelocityAdjustment* adjustment = mVelocityAdjustment;
    adjustment->mMinimum.zero();
    adjustment->mMaximum.zero();
    fn_804116B0();
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

// NONMATCHING: vector copy/set inlining and local stack ordering differ.
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
    TVec3f previous(mPreviousVelocity);
    previous.scale(0.5f);
    TVec3f current(mMovementVelocity);
    current.scale(0.5f);
    TVec3f average(current);
    average.add(previous);
    mPreviousVelocity = mMovementVelocity;
    TVec3f velocity(average);
    velocity.add(mAdditionalVelocity);
    mVelocity.set<f32>(velocity);
    _198 = fn_804113C0();
    fn_804117F0();
    fn_80411BF0();
}

YoshiAnimationSync::YoshiAnimationSync(Yoshi* actor) : mActor(actor), mAnimation(NULL), _8(false), _9(false), _A(false) {
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
    mXanimePlayer->_88 = false;
    mXanimePlayer->_89 = false;
    mXanimePlayerUpper->_88 = false;
    mXanimePlayerUpper->_89 = false;
}

void Yoshi::fn_8040CB30() {
    mXanimePlayer->getCore()->_6 = 1;
    MR::calcJ3DModel(this);
    mYoshiModelCtrl->fn_80418020();
    mXanimePlayer->getCore()->_6 = 2;
    MR::calcJ3DModel(this);
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

bool Yoshi::fn_80411960() const {
    bool result;

    result = false;
    if ((isNerve(&NrvYoshi::YoshiNrv80413870::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413880::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413890::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138A0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138B0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138C0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413A10::sInstance) != 0)) {
        result = true;
    }
    return result;
}

bool Yoshi::fn_80411D10() const {
    bool result;

    result = false;
    if ((isNerve(&NrvYoshi::YoshiNrv80413870::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413880::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413890::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138A0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138B0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138C0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138D0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138E0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804138F0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413900::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413910::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413990::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139A0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139B0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139C0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139D0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv804139E0::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413A00::sInstance) != 0) || (isNerve(&NrvYoshi::YoshiNrv80413A10::sInstance) != 0)) {
        result = true;
    }
    return result;
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

// NONMATCHING: string pool base and register allocation differ.
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

bool Yoshi::fn_8040F0E0() const {
    return !isNerve(&NrvYoshi::YoshiNrv80413840::sInstance);
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

// NONMATCHING: pointer equality uses direct compare instead of boolean normalization.
void Yoshi::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (MR::isSensorPlayerOrRide(receiver) && getSensor("NpcBodyAttack") == sender && fn_804100E0()) {
        MR::sendMsgPush(receiver, sender);
    }
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

// NONMATCHING: string pool base, register allocation and equality lowering differ.
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
    if (getSensor("Ride") == receiver || getSensor("NpcBodyReceive") == receiver) {
        if (MR::isMsgPlayerTrample(message) || MR::isMsgPlayerHipDrop(message)) {
            mMountedBySpin = false;
            fn_8040CC40();
            return false;
        }
    }
    else if (getSensor("SpinRide") == receiver && MR::isMsgPlayerSpinAttack(message)) {
        mMountedBySpin = true;
        fn_8040CC40();
        return false;
    }
    return false;
}

// NONMATCHING: getZDir is inlined; pointer comparison lowering differs.
bool Yoshi::receiveMsgPush(HitSensor* sender, HitSensor* receiver) {
    if (isNerve(&NrvYoshi::YoshiNrv80413840::sInstance)) return false;
    if (MR::isSensorPlayerOrRide(sender)) {
        if (!fn_80411D10()) return false;
        if (getSensor("NpcBodyReceive2") != receiver) return false;
    }
    else {
        if (fn_804101D0()) return false;
        if (getSensor("NpcBodyReceive") != receiver) return false;
    }
    f32 senderRadius = MR::getSensorRadius(sender);
    f32 combinedRadius = MR::getSensorRadius(receiver) + senderRadius;
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
