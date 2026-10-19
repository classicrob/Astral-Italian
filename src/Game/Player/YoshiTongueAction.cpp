#include "Game/Player/YoshiTongueAction.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Player/YoshiTongue.hpp"
#include "Game/Player/YoshiTongueTarget.hpp"
#include "Game/Player/YoshiLockOnManager.hpp"
#include "Game/Player/YoshiArrow.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioHolder.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/Popo.hpp"
#include "Game/LiveActor/HitSensor.hpp"

#pragma auto_inline off

namespace NrvYoshiTongueAction {
    class State0 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041DFD0(); }
        static State0 sInstance;
    };
    State0 State0::sInstance ATTRIBUTE_WEAK;
    class State1 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041E150(); }
        virtual void executeOnEnd(Spine*) const;
        static State1 sInstance;
    };
    State1 State1::sInstance ATTRIBUTE_WEAK;
    class State2 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041E210(); }
        static State2 sInstance;
    };
    State2 State2::sInstance ATTRIBUTE_WEAK;
    class State3 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041E470(); }
        static State3 sInstance;
    };
    State3 State3::sInstance ATTRIBUTE_WEAK;
    class State4 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041E680(); }
        static State4 sInstance;
    };
    State4 State4::sInstance ATTRIBUTE_WEAK;
    class State5 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041E8F0(); }
        static State5 sInstance;
    };
    State5 State5::sInstance ATTRIBUTE_WEAK;
    class State6 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041E9E0(); }
        static State6 sInstance;
    };
    State6 State6::sInstance ATTRIBUTE_WEAK;
    class State7 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041EAA0(); }
        static State7 sInstance;
    };
    State7 State7::sInstance ATTRIBUTE_WEAK;
    class State8 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041EBE0(); }
        virtual void executeOnEnd(Spine*) const;
        static State8 sInstance;
    };
    State8 State8::sInstance ATTRIBUTE_WEAK;
    class State9 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041EDE0(); }
        static State9 sInstance;
    };
    State9 State9::sInstance ATTRIBUTE_WEAK;
    class State10 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041EE00(); }
        static State10 sInstance;
    };
    State10 State10::sInstance ATTRIBUTE_WEAK;
    class State11 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041EEB0(); }
        static State11 sInstance;
    };
    State11 State11::sInstance ATTRIBUTE_WEAK;
    class State12 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041F000(); }
        static State12 sInstance;
    };
    State12 State12::sInstance ATTRIBUTE_WEAK;
    class State13 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041F0E0(); }
        virtual void executeOnEnd(Spine*) const;
        static State13 sInstance;
    };
    State13 State13::sInstance ATTRIBUTE_WEAK;
    class State14 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041F3D0(); }
        static State14 sInstance;
    };
    State14 State14::sInstance ATTRIBUTE_WEAK;
    class State15 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041F520(); }
        static State15 sInstance;
    };
    State15 State15::sInstance ATTRIBUTE_WEAK;
    class State16 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041F590(); }
        static State16 sInstance;
    };
    State16 State16::sInstance ATTRIBUTE_WEAK;
    class State17 : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<YoshiTongueAction*>(spine->mExecutor)->fn_8041F6F0(); }
        static State17 sInstance;
    };
    State17 State17::sInstance ATTRIBUTE_WEAK;
    class State18 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        virtual void executeOnEnd(Spine*) const;
        static State18 sInstance;
    };
    State18 State18::sInstance ATTRIBUTE_WEAK;

    void State1::executeOnEnd(Spine* spine) const {
        YoshiTongueAction* action = static_cast<YoshiTongueAction*>(spine->mExecutor);
        action->_A4 = 100.0f;
        MR::endStarPointerMode(action);
    }
    void State8::executeOnEnd(Spine* spine) const {
        static_cast<YoshiTongueAction*>(spine->mExecutor)->mArrow->kill();
    }
    void State13::executeOnEnd(Spine* spine) const {
        YoshiTongueAction* action = static_cast<YoshiTongueAction*>(spine->mExecutor);
        action->fn_80421790();
        action->mTarget->sendReleaseEatenMessage();
        action->fn_804217B0();
    }
    void State18::executeOnEnd(Spine* spine) const {
        MR::endStarPointerMode(static_cast<YoshiTongueAction*>(spine->mExecutor));
    }
}

YoshiTongueAction::YoshiTongueAction(Yoshi* yoshi, YoshiTongue* tongue, Mario* player, YoshiTongueTarget* target, YoshiArrow* arrow) : NerveExecutor("ヨッシー舌アクション管理") {
    mYoshi = yoshi;
    mTongue = tongue;
    mPlayer = player;
    mTarget = target;
    _18 = false;
    mArrow = arrow;
    _2C = false;
    _2D = true;
    _2E = true;
    _60 = false;
    mStartPosition.x = 0.0f;
    mStartPosition.y = 0.0f;
    mStartPosition.z = 0.0f;
    mDirection.x = 0.0f;
    mDirection.y = 0.0f;
    mDirection.z = 1.0f;
    _7C.x = 0.0f;
    _7C.y = 0.0f;
    _7C.z = 0.0f;
    _88 = 0.0f;
    _8C = false;
    _8D = false;
    _90.x = 0.0f;
    _90.y = 0.0f;
    _90.z = 0.0f;
    _9C = false;
    _A0 = nullptr;
    _A4 = 100.0f;
    _A8 = false;
    mTargetMtx.identity();
    mTarget->mSensor = yoshi->getSensor("AutoEat");
    initNerve(&NrvYoshiTongueAction::State0::sInstance, 0);
}

YoshiTongueAction::~YoshiTongueAction() {}

void YoshiTongueAction::fn_8041DF40() {
    updateNerve();
    if (mTarget->hasCurrentTarget() && fn_804201A0()) mTarget->calcCurrentTargetPosition(&mTargetPosition);
    if (!isNerve(&NrvYoshiTongueAction::State8::sInstance) && !isNerve(&NrvYoshiTongueAction::State9::sInstance)) MR::tryDeleteEffect(mYoshi, "KeepMouthKiller");
}

void YoshiLockOnManager::startLockOn(LiveActor* actor) {
    mContextActive = true;
    mSuspended = false;
    mContextActor = actor;
}
void YoshiLockOnManager::endLockOn() {
    mContextActive = false;
    mHasUpdated = false;
    clearActiveTargets();
}

bool YoshiTongueAction::fn_8041F8A0() const { return isNerve(&NrvYoshiTongueAction::State13::sInstance); }

bool YoshiTongueAction::fn_8041F8B0() const { return isNerve(&NrvYoshiTongueAction::State13::sInstance); }

bool YoshiTongueAction::fn_8041FA30() const { return isNerve(&NrvYoshiTongueAction::State0::sInstance); }

bool YoshiTongueAction::fn_8041F8C0() const {
    if (isNerve(&NrvYoshiTongueAction::State11::sInstance)) return _8C;
    return false;
}

bool YoshiTongueAction::fn_8041F910() const {
    bool result = false;
    if (isNerve(&NrvYoshiTongueAction::State0::sInstance) || isNerve(&NrvYoshiTongueAction::State8::sInstance)) result = true;
    return result;
}

bool YoshiTongueAction::fn_8041FA40() const {
    bool result = false;
    if (isNerve(&NrvYoshiTongueAction::State8::sInstance) || isNerve(&NrvYoshiTongueAction::State9::sInstance)) result = true;
    return result;
}

bool YoshiTongueAction::fn_8041FAA0() const {
    bool result = false;
    if (isNerve(&NrvYoshiTongueAction::State0::sInstance) || isNerve(&NrvYoshiTongueAction::State8::sInstance) || isNerve(&NrvYoshiTongueAction::State5::sInstance) || isNerve(&NrvYoshiTongueAction::State6::sInstance)) result = true;
    return result;
}

f32 YoshiTongueAction::fn_8041FB30() const { return 1.0f; }
void YoshiTongueAction::fn_8041FB40(TVec3f* output) const { mTarget->calcCurrentTargetPosition(output); }
f32 YoshiTongueAction::fn_8041FB50() { return 4.0f; }

bool YoshiTongueAction::fn_8041FB60() {
    if (_2C) _8D = true;
    else {
        setNerve(&NrvYoshiTongueAction::State15::sInstance);
        _8D = true;
        mTongue->fn_8041CBA0();
    }
    MR::tryDeleteEffect(mYoshi, "KeepMouthKiller");
    return true;
}
bool YoshiTongueAction::fn_8041FBD0() {
    if (isNerve(&NrvYoshiTongueAction::State11::sInstance)) {
        mPlayer->fn_803B1780();
        mTongue->fn_8041CBA0();
        setNerve(&NrvYoshiTongueAction::State12::sInstance);
        return true;
    }
    setNerve(&NrvYoshiTongueAction::State15::sInstance);
    _8D = false;
    mTongue->fn_8041CBA0();
    MR::tryDeleteEffect(mYoshi, "KeepMouthKiller");
    return true;
}
bool YoshiTongueAction::fn_8041FC60() {
    if (!isNerve(&NrvYoshiTongueAction::State11::sInstance)) return false;
    _A8 = false;
    setNerve(&NrvYoshiTongueAction::State17::sInstance);
    mTongue->fn_8041CB70(false);
    mPlayer->fn_803B1780();
    return true;
}
bool YoshiTongueAction::fn_8041FCD0() { setNerve(&NrvYoshiTongueAction::State0::sInstance); return true; }
bool YoshiTongueAction::fn_8041FD00() {
    fn_804214C0();
    static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->endLockOn();
    mYoshi->fn_8040EE70(nullptr);
    mYoshi->mHeadCtrl->fn_8041AB50(1);
    mYoshi->fn_8040EF30();
    MR::tryDeleteEffect(mYoshi, "KeepMouthKiller");
    setNerve(&NrvYoshiTongueAction::State0::sInstance);
    return true;
}
bool YoshiTongueAction::fn_8041FD80() {
    fn_804214C0();
    static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->endLockOn();
    mYoshi->fn_8040EE70(nullptr);
    mYoshi->mHeadCtrl->fn_8041AB50(1);
    mYoshi->fn_8040EF30();
    MR::tryDeleteEffect(mYoshi, "KeepMouthKiller");
    setNerve(&NrvYoshiTongueAction::State0::sInstance);
    return true;
}
bool YoshiTongueAction::fn_8041FE00() {
    mPlayer->fn_803B1800();
    mYoshi->fn_8040EE70(nullptr);
    mYoshi->mHeadCtrl->fn_8041AB50(1);
    mYoshi->fn_8040EF30();
    MR::tryDeleteEffect(mYoshi, "KeepMouthKiller");
    setNerve(&NrvYoshiTongueAction::State1::sInstance);
    return true;
}
bool YoshiTongueAction::fn_8041FE80() { setNerve(&NrvYoshiTongueAction::State0::sInstance); return true; }
void YoshiTongueAction::fn_8041FEB0() {
    fn_804214C0();
    static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->startLockOn(mYoshi);
    mYoshi->fn_8040EE70(nullptr);
    mYoshi->mHeadCtrl->fn_8041AB50(1);
    mYoshi->fn_8040EF30();
    MR::tryDeleteEffect(mYoshi, "KeepMouthKiller");
    mTongue->fn_8041CC60();
    if (mPlayer->fn_803B1B70()) {
        mPlayer->fn_803B1B20();
        mPlayer->changeAnimation(nullptr, static_cast<const char*>(nullptr));
    }
    setNerve(&NrvYoshiTongueAction::State0::sInstance);
}
bool YoshiTongueAction::fn_8041FF60() const { return mPlayer->fn_803B1850(); }
void YoshiTongueAction::fn_8041FF70() { mArrow->fn_80414040(); }
void YoshiTongueAction::fn_8041FF80() {
    mYoshi->fn_8040EE70(nullptr);
    mYoshi->mHeadCtrl->fn_8041AB50(1);
    mYoshi->fn_8040EF30();
    MR::tryDeleteEffect(mYoshi, "KeepMouthKiller");
    static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->endLockOn();
    setNerve(&NrvYoshiTongueAction::State18::sInstance);
}
void YoshiTongueAction::fn_80420000() {
    static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->startLockOn(mYoshi);
    setNerve(&NrvYoshiTongueAction::State0::sInstance);
}
void YoshiTongueAction::fn_80420050() {
    if (fn_80421410()) { setNerve(&NrvYoshiTongueAction::State13::sInstance); return; }
    bool flag = _8D;
    setNerve(&NrvYoshiTongueAction::State15::sInstance);
    _8D = flag;
    mTongue->fn_8041CBA0();
}
void YoshiTongueAction::fn_804200C0() {
    if (fn_80421410()) { setNerve(&NrvYoshiTongueAction::State11::sInstance); return; }
    bool flag = _8D;
    setNerve(&NrvYoshiTongueAction::State15::sInstance);
    _8D = flag;
    mTongue->fn_8041CBA0();
}
#pragma push
#pragma dont_inline on
void YoshiTongueAction::fn_80420130() {
    MtxPtr matrix = mYoshi->getBaseMtx();
    TVec3f direction(-matrix[0][2], -matrix[1][2], -matrix[2][2]);
    mPlayer->faint(direction);
}
#pragma pop
bool YoshiTongueAction::fn_804201A0() const {
    bool result = false;
    if (!isNerve(&NrvYoshiTongueAction::State3::sInstance) && !isNerve(&NrvYoshiTongueAction::State4::sInstance) && !isNerve(&NrvYoshiTongueAction::State7::sInstance) && !isNerve(&NrvYoshiTongueAction::State8::sInstance)) result = true;
    return result;
}

void YoshiTongueAction::fn_80420AC0(TVec3f* output, const TVec3f& target, const TVec3f& gravity, f32 progress) const {
    TVec3f delta(target);
    JMathInlineVEC::PSVECSubtract(&delta, &mStartPosition, &delta);
    TVec3f planar;
    JMAVECScaleAdd(&gravity, &delta, &planar, -gravity.dot(delta));
    TVec3f vertical(delta);
    JMathInlineVEC::PSVECSubtract(&vertical, &planar, &vertical);
    f32 inverse = progress - 1.0f;
    f32 square = inverse * inverse;
    f32 arc = 50.0f * (6.7f * (inverse * square + square));
    vertical.scale(progress);
    TVec3f negativeGravity;
    JGeometry::negateInternal(&gravity.x, &negativeGravity.x);
    TVec3f offset(negativeGravity);
    offset.scale(arc);
    vertical += offset;
    f32 remaining = progress - 1.0f;
    planar.scale(1.0f + -remaining * remaining);
    TVec3f base(mStartPosition);
    base += vertical;
    TVec3f position(base);
    position += planar;
    output->set(position);
}

void YoshiTongueAction::fn_80420C30(TVec3f* position, const TVec3f& target, const TVec3f& gravity, f32 angle) const {
    TVec3f center;
    mTarget->calcCurrentTargetPosition(&center);
    TVec3f direction(target);
    JMathInlineVEC::PSVECSubtract(&direction, &center, &direction);
    MR::normalize(&direction);
    TVec3f axis;
    PSVECCrossProduct(&direction, &gravity, &axis);
    MR::normalize(&axis);
    TQuat4f rotation;
    rotation.setRotate(axis, angle);
    TVec3f offset(*position);
    JMathInlineVEC::PSVECSubtract(&offset, &center, &offset);
    rotation.transform(offset);
    TVec3f result(center);
    result += offset;
    position->set(result);
}

// NONMATCHING: temporary stack slots, quaternion copy instruction widths and argument scheduling differ.
void YoshiTongueAction::fn_80420D20(const TVec3f& position, const TVec3f& target, const TVec3f& gravity, f32 progress) {
    TPos3f original;
    original.set(mYoshi->getBaseMtx());
    TVec3f up(target);
    JMathInlineVEC::PSVECSubtract(&up, &mStartPosition, &up);
    if (MR::normalizeOrZero(&up)) original.getYDir(up);
    TVec3f side;
    PSVECCrossProduct(&up, &gravity, &side);
    if (MR::normalizeOrZero(&side)) {
        f32 z = original.mMtx[2][0];
        f32 y = original.mMtx[1][0];
        f32 x = original.mMtx[0][0];
        side.set<f32>(x, y, z);
    }
    up = target - position;
    if (MR::normalizeOrZero(&up)) original.getYDir(up);
    TVec3f front;
    PSVECCrossProduct(&side, &up, &front);
    if (MR::normalizeOrZero(&front)) {
        f32 z = original.mMtx[2][2];
        f32 y = original.mMtx[1][2];
        f32 x = original.mMtx[0][2];
        front.set<f32>(x, y, z);
    }
    PSVECCrossProduct(&up, &front, &side);
    TPos3f oriented;
    oriented.identity();
    oriented.setXYZDir(side, up, front);
    TQuat4f originalRotation;
    TQuat4f targetRotation;
    original.getQuat(originalRotation);
    oriented.getQuat(targetRotation);
    TQuat4f rotation(static_cast<const Quaternion&>(originalRotation));
    rotation.slerp(targetRotation, progress);
    side.set<f32>((1.0f - 2.0f * (rotation.y * rotation.y)) - 2.0f * (rotation.z * rotation.z),
                  2.0f * (rotation.x * rotation.y) + 2.0f * (rotation.w * rotation.z),
                  2.0f * (rotation.x * rotation.z) - 2.0f * (rotation.w * rotation.y));
    up.set<f32>(2.0f * (rotation.x * rotation.y) - 2.0f * (rotation.w * rotation.z),
                (1.0f - 2.0f * (rotation.x * rotation.x)) - 2.0f * (rotation.z * rotation.z),
                2.0f * (rotation.y * rotation.z) + 2.0f * (rotation.w * rotation.x));
    front.set<f32>(2.0f * (rotation.x * rotation.z) + 2.0f * (rotation.w * rotation.y),
                   2.0f * (rotation.y * rotation.z) - 2.0f * (rotation.w * rotation.x),
                   (1.0f - 2.0f * (rotation.x * rotation.x)) - 2.0f * (rotation.y * rotation.y));
    mPlayer->setSideVec(side);
    mPlayer->setHeadVec(up);
    mPlayer->setFrontVec(front);
    mPlayer->clearVelocity();
    const TVec3f& yoshiPosition = mYoshi->mPosition;
    mPlayer->addVelocity(position - yoshiPosition);
}

void YoshiTongueAction::fn_80421830(TVec3f* output) const {
    TPos3f matrix;
    mTongue->fn_8041CD10(&matrix);
    output->set<f32>(matrix.mMtx[0][3], matrix.mMtx[1][3], matrix.mMtx[2][3]);
}

void YoshiTongueAction::fn_8041E210() {
    if (MR::isFirstStep(this)) {
        mYoshi->fn_8040EE30("口を開く");
        mYoshi->mHeadCtrl->fn_8041AA70(1, &mTargetPosition);
        mYoshi->fn_8040EF20();
        _18 = true;
        MR::startSound(mYoshi, "SE_SM_YOSHI_TONGUE", -1, -1);
    }
    if (!MR::testCorePadButtonB(0)) _18 = false;
    TVec3f target;
    mTarget->calcCurrentTargetPosition(&target);
    mTongue->mTipPosition.set(target);
    if (mTarget->fn_8041C1B0()) mTongue->fn_8041CCF0();
    else {
        TVec3f direction;
        mTarget->fn_8041C440(&direction);
        TVec3f tangent(direction);
        tangent.scale(2.0f);
        mTongue->fn_8041CCB0(tangent);
    }
    if (mTongue->fn_8041CD00()) {
        MR::startActionSound(mTongue, "TongueHitWall", -1, -1, -1);
        mTongue->fn_8041CEE0();
        setNerve(&NrvYoshiTongueAction::State15::sInstance);
        _8D = false;
        mTongue->fn_8041CBA0();
        return;
    }
    if (mTongue->fn_8041CB50()) {
        if (!mTarget->fn_8041C170() && !mTarget->fn_8041C120()) MR::startActionSound(mTongue, "TongueTouch", -1, -1, -1);
        if (mTarget->fn_8041C170()) { fn_80420050(); return; }
        if (mTarget->fn_8041C1E0()) { fn_804200C0(); return; }
        if (mTarget->fn_8041C210() || mTarget->fn_8041C120() || mTarget->fn_8041C140()) {
            if (fn_80421410()) {
                setNerve(&NrvYoshiTongueAction::State3::sInstance);
                mTongue->fn_8041CB70(mTarget->hasMultipleRemainingTargets());
                return;
            }
            bool flag = _8D;
            setNerve(&NrvYoshiTongueAction::State15::sInstance);
            _8D = flag;
            mTongue->fn_8041CBA0();
        }
    }
}

void YoshiTongueAction::fn_8041E9E0() {
    bool complete = false;
    if (mYoshi->fn_8040F5F0() || MR::isGreaterEqualStep(this, 50)) complete = true;
    if (complete || mYoshi->mDashTimer > 0) {
        mYoshi->fn_8040EE70(nullptr);
        if (mPlayer->fn_803B1B70()) {
            mPlayer->changeAnimation("基本", static_cast<const char*>(nullptr));
            mPlayer->fn_803B1B20();
        }
        setNerve(&NrvYoshiTongueAction::State0::sInstance);
    }
}

void YoshiTongueAction::fn_8041EAA0() {
    if (MR::isFirstStep(this)) {
        mYoshi->fn_8040EE30("口を閉じる");
        MR::tryRumblePadMiddle(this, 0);
        MR::startActionSoundPlayer("Swallow", -1, -1);
        _60 = false;
    }
    if (!_60) {
        fn_80420480(false);
        if (mTongue->fn_8041CB80()) {
            fn_80421790();
            mTarget->sendReleaseEatenMessage();
            fn_804217B0();
            fn_80420230(nullptr);
            _60 = true;
        }
    }
    bool complete = false;
    if (mYoshi->fn_8040F5F0() || MR::isGreaterEqualStep(this, 50)) complete = true;
    if (complete && _60) {
        mYoshi->mHeadCtrl->fn_8041AB50(1);
        mTongue->fn_8041CC60();
        mYoshi->fn_8040EE70(nullptr);
        setNerve(&NrvYoshiTongueAction::State0::sInstance);
    }
}

void YoshiTongueAction::fn_8041EDE0() {
    if (mPlayer->mActor->_934) return;
    setNerve(&NrvYoshiTongueAction::State8::sInstance);
}

void YoshiTongueAction::fn_8041EE00() {
    if (MR::isFirstStep(this)) {
        mYoshi->fn_8040EE70(nullptr);
        mYoshi->mHeadCtrl->fn_8041AC40(1);
        fn_80421790();
        mTarget->sendReleaseEatenMessage();
        fn_804217B0();
        _9C = false;
        MR::tryDeleteEffect(mYoshi, "KeepMouthKiller");
    }
    if (_9C == true) { setNerve(&NrvYoshiTongueAction::State0::sInstance); return; }
    mPlayer->fn_803B1880();
}

void YoshiTongueAction::fn_8041F000() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(mYoshi, "PullSuccess", -1, -1, -1);
        mYoshi->fn_8040EE70(nullptr);
        mYoshi->mHeadCtrl->fn_8041AB50(1);
        mPlayer->changeAnimation("ヨッシー引っ張り終了", static_cast<const char*>(nullptr));
    }
    if (mTongue->fn_8041CC00() && mPlayer->isAnimationTerminate("ヨッシー引っ張り終了")) {
        mPlayer->changeAnimation("基本", static_cast<const char*>(nullptr));
        mPlayer->fn_803B17C0();
        setNerve(&NrvYoshiTongueAction::State0::sInstance);
        mTongue->fn_8041CC60();
    }
}

void YoshiTongueAction::fn_8041F0E0() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(mYoshi, "GrabTarget", -1, -1, -1);
        mPlayer->changeAnimation("フック接近", static_cast<const char*>(nullptr));
        fn_80420780();
        mStartPosition = mYoshi->mPosition;
    }
    mPlayer->mMovementStates.mLowWord &= 0xBFFFFFFF;
    mPlayer->mMovementStates.mLowWord &= 0xFFDFFFFF;
    mPlayer->mMovementStates.mLowWord |= 0x80000000;
    TVec3f position(mYoshi->mPosition);
    TVec3f target;
    mTarget->calcCurrentTargetPosition(&target);
    TVec3f approach(target - mDirection.scaleInline(170.0f));
    TVec3f gravity(mPlayer->getAirGravityVec());
    fn_80420AC0(&position, approach, gravity, fn_80420A20());
    s32 turnStep = fn_80420980() - 4;
    if (turnStep < 0) turnStep = 1;
    if (MR::isStep(this, turnStep)) {
        mYoshi->mHeadCtrl->fn_8041AB50(1);
        mPlayer->changeAnimation("フックぶら下がり", static_cast<const char*>(nullptr));
    }
    if (MR::isGreaterEqualStep(this, turnStep)) {
        f32 elapsed = static_cast<f32>(getNerveStep() - turnStep);
        f32 radians = 3.1415927410125732f * elapsed;
        fn_80420C30(&position, approach, gravity, 210.0f * radians / 3420.0f);
    }
    if (MR::isStep(this, turnStep)) {
        MR::startActionSound(mYoshi, "JumpObj", -1, -1, -1);
        MR::startActionSound(mYoshi, "CaptureJump", -1, -1, -1);
    }
    f32 progress = 0.1f;
    if (MR::isGreaterEqualStep(this, turnStep)) {
        f32 remaining = static_cast<f32>(turnStep + 19 - getNerveStep()) / 19.0f;
        progress = (1.0f - remaining) + 0.1f * remaining;
    }
    fn_80420D20(position, target, gravity, progress);
    if (MR::isGreaterEqualStep(this, turnStep + 19)) {
        fn_804210B0();
        setNerve(&NrvYoshiTongueAction::State15::sInstance);
        _8D = false;
        mTongue->fn_8041CBA0();
    }
}

void YoshiTongueAction::fn_8041F3D0() {
    if (MR::isFirstStep(this)) {
        mYoshi->fn_8040EE30("口を開く");
        mTongue->fn_8041CB30(false);
        MR::startSound(mYoshi, "SE_SM_YOSHI_TONGUE_SHORT", -1, -1);
    }
    MtxPtr mouthMtx = MR::getJointMtx(mYoshi, "TonguePos");
    TVec3f mouth(mouthMtx[0][3], mouthMtx[1][3], mouthMtx[2][3]);
    MtxPtr matrix = mYoshi->getBaseMtx();
    TVec3f direction(matrix[0][2], matrix[1][2], matrix[2][2]);
    MR::normalizeOrZero(&direction);
    TVec3f offset(direction);
    offset.scale(40.0f);
    TVec3f target(mouth);
    target += offset;
    mTargetPosition.set(target);
    mTongue->mTipPosition.set(mTargetPosition);
    mTongue->fn_8041CCF0();
    if (mTongue->fn_8041CB50()) {
        setNerve(&NrvYoshiTongueAction::State15::sInstance);
        _8D = false;
        mTongue->fn_8041CBA0();
    }
}

void YoshiTongueAction::fn_8041F520() {
    if (mTongue->fn_8041CC00()) {
        mYoshi->fn_8040EE30("口を閉じる");
        setNerve(&NrvYoshiTongueAction::State0::sInstance);
        mTongue->fn_8041CC60();
        if (_8D) fn_80420130();
    }
}

void YoshiTongueAction::fn_8041F6F0() {
    if (MR::isFirstStep(this)) {
        mYoshi->fn_8040EE70(nullptr);
        mYoshi->mHeadCtrl->fn_8041AB50(1);
        mPlayer->changeAnimation("ヨッシー引っ張り食べ終了", static_cast<const char*>(nullptr));
        MR::tryRumblePadMiddle(this, 0);
        _A8 = false;
    }
    if (MR::isStep(this, 3)) MR::startActionSound(mYoshi, "PullToMouth", -1, -1, -1);
    if (MR::isStep(this, 8)) {
        MR::startActionSound(mYoshi, "yoshieatstart", -1, -1, -1);
        MR::startCSSound1P("CS_YOSHI_EAT", 2, 1.0f);
    }
    fn_80420480(MR::isFirstStep(this));
    if (!_A8 && mTongue->fn_8041CB80()) {
        fn_80421790();
        mTarget->sendReleaseEatenMessage();
        fn_804217B0();
        fn_80420230(nullptr);
        _A8 = true;
        MR::tryRumblePadMiddle(this, 0);
    }
    if (_A8 && mPlayer->isAnimationTerminate("ヨッシー引っ張り食べ終了")) {
        mPlayer->changeAnimation("基本", static_cast<const char*>(nullptr));
        mPlayer->fn_803B17C0();
        mYoshi->fn_8040EE30("噛む");
        setNerve(&NrvYoshiTongueAction::State5::sInstance);
    }
}

void YoshiTongueAction::fn_8041E8F0() {
    if (fn_80421880()) MR::startActionSoundPlayer("Eating", -1, -1);
    if (MR::isStep(this, 0) && fn_80421880()) {
        MR::startActionSoundPlayer("Swallow", -1, -1);
        MR::startCSSound1P("CS_YOSHI_SWALLOW", 4, 1.0f);
    }
    if (MR::isGreaterEqualStep(this, 1)) {
        if (mYoshi->mDashTimer <= 0) {
            mYoshi->fn_8040EE30("飲み下す");
            if (mPlayer->fn_803B1B70()) mPlayer->changeAnimation("飲み下す", static_cast<const char*>(nullptr));
        }
        setNerve(&NrvYoshiTongueAction::State6::sInstance);
    }
}

void YoshiTongueAction::fn_8041F590() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(mYoshi, "yoshieatstart", -1, -1, -1);
        MR::startCSSound1P("CS_YOSHI_EAT", 2, 1.0f);
        mYoshi->fn_8040EE30("自動で食べる");
        mYoshi->fn_8040EF20();
        sendAutoEatSensorMessage(0x100, _A0, MR::getMessageSensor());
        LiveActor* actor = MR::getSensorHost(_A0);
        if (actor) fn_80420230(actor);
        if (_2D) MR::tryRumblePadMiddle(this, 0);
        MR::stopScene(8);
    }
    bool complete = false;
    if (mYoshi->fn_8040F5F0() || MR::isGreaterEqualStep(this, 50)) complete = true;
    if (complete) {
        if (_2E) {
            mYoshi->fn_8040EE70(nullptr);
            setNerve(&NrvYoshiTongueAction::State0::sInstance);
            return;
        }
        if (mYoshi->mDashTimer > 0) mYoshi->fn_8040EE70(nullptr);
        else mYoshi->fn_8040EE30("噛む");
        setNerve(&NrvYoshiTongueAction::State5::sInstance);
    }
}

void YoshiTongueAction::fn_80420230(LiveActor* actor) {
    LiveActor* target = actor == nullptr ? mTarget->getCurrentTargetActor() : actor;
    if (!MR::tryEmitEffect(target, "Get")) MR::tryEmitEffect(mYoshi, "Bite");
    mPlayer->mActor->fn_803CEF50(target);
}

void YoshiTongueAction::fn_80420780() {
    if (mTarget->fn_8041C1B0()) { fn_804207D0(); return; }
    fn_80420890();
}

void YoshiTongueAction::fn_804207D0() {
    TVec3f gravity(mPlayer->getAirGravityVec());
    TVec3f delta;
    mTarget->calcCurrentTargetPosition(&delta);
    JMathInlineVEC::PSVECSubtract(&delta, &mYoshi->mPosition, &delta);
    TVec3f side;
    if (MR::isNearZero(delta, 0.001f)) side.set<f32>(1.0f, 0.0f, 0.0f);
    else {
        PSVECCrossProduct(&delta, &gravity, &side);
        MR::normalize(&side);
    }
    PSVECCrossProduct(&gravity, &side, &mDirection);
    MR::normalize(&mDirection);
}

void YoshiTongueAction::fn_80420890() {
    MtxPtr matrix = mTarget->getCurrentTargetActor()->getBaseMtx();
    TVec3f direction(matrix[0][2], matrix[1][2], matrix[2][2]);
    TVec3f delta;
    mTarget->calcCurrentTargetPosition(&delta);
    JMathInlineVEC::PSVECSubtract(&delta, &mYoshi->mPosition, &delta);
    if (delta.dot(direction) < 0.0f) JGeometry::negateInternal(&direction.x, &direction.x);
    TVec3f gravity;
    MR::calcGravityVector(mTarget->getCurrentTargetActor(), &gravity, nullptr, 0);
    JMAVECScaleAdd(&gravity, &direction, &direction, -gravity.dot(direction));
    MR::normalize(&direction);
    mDirection.set(direction);
}

s32 YoshiTongueAction::fn_80420980() const {
    TVec3f target;
    mTarget->calcCurrentTargetPosition(&target);
    f32 distance = PSVECDistance(&target, &mStartPosition);
    if (distance < 20.0f) distance = 20.0f;
    if (distance > 2000.0f) distance = 2000.0f;
    f32 weight = (distance - 20.0f) / 1980.0f;
    return 5.0f * ((1.0f - weight) + 6.0f * weight);
}

f32 YoshiTongueAction::fn_80420A20() const {
    f32 frames = static_cast<f32>(fn_80420980());
    f32 progress = static_cast<f32>(getNerveStep()) / frames;
    if (progress > 1.0f) progress = 1.0f;
    return progress;
}

bool YoshiTongueAction::sendAutoEatSensorMessage(u32 message, HitSensor* sender, HitSensor* receiver) {
    mPlayer->mActor->mFlag1052 = true;
    bool result = MR::sendArbitraryMsg(message, sender, receiver);
    mPlayer->mActor->mFlag1052 = false;
    return result;
}
void YoshiTongueAction::fn_80421790() { mPlayer->mActor->mFlag1052 = true; }
void YoshiTongueAction::fn_804217B0() { mPlayer->mActor->mFlag1052 = false; }
void YoshiTongueAction::fn_804217D0() {
    if (MR::isEqualString(mTarget->getCurrentTargetActor()->mName, "ホーミングキラー")) MR::emitEffect(mYoshi, "KeepMouthKiller");
}
// NONMATCHING: positive countdown is tested directly instead of materializing a boolean.
bool YoshiTongueAction::fn_80421880() const {
    bool canEat = false;
    if (mYoshi->mDashTimer <= 0 && !mYoshi->fn_8040F070()) canEat = true;
    return canEat;
}

void YoshiTongueAction::fn_8041EEB0() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(mYoshi, "GrabTarget", -1, -1, -1);
        _8C = false;
        mYoshi->fn_8040EE70(nullptr);
    }
    if (!MR::testCorePadButtonB(0)) _18 = false;
    if (fn_8041F8C0()) MR::startActionSound(mYoshi, "YoshiTonguePull", -1, -1, -1);
    fn_80421790();
    _8C = mTarget->sendLoopTongueMessage();
    fn_804217B0();
    TVec3f target;
    mTarget->calcCurrentTargetPosition(&target);
    mTongue->mTipPosition.set(target);
    if (!_18 || MR::calcDistanceToPlayer(target) > 2000.0f) {
        fn_80421790();
        mTarget->sendReleaseEatenMessage();
        fn_804217B0();
        mPlayer->fn_803B17C0();
        setNerve(&NrvYoshiTongueAction::State15::sInstance);
        _8D = false;
        mTongue->fn_8041CBA0();
        return;
    }
    mPlayer->fn_803B1720();
}

void YoshiTongueAction::fn_8041E680() {
    if (MR::isFirstStep(this)) {
        mYoshi->fn_8040EE30("食べる");
        if (mPlayer->fn_803B18C0()) mPlayer->changeAnimation("食べる", static_cast<const char*>(nullptr));
        MR::tryRumblePadMiddle(this, 0);
        _60 = false;
    }
    if (MR::isStep(this, 3)) MR::startActionSound(mYoshi, "PullToMouth", -1, -1, -1);
    if (MR::isStep(this, 8)) {
        MR::startActionSound(mYoshi, "yoshieatstart", -1, -1, -1);
        MR::startCSSound1P("CS_YOSHI_EAT", 2, 1.0f);
        if (mTarget->fn_8041C210()) MR::startActionSound(mYoshi, "MouthPreShoot", -1, -1, -1);
    }
    if (!_60) {
        fn_80420480(false);
        if (mTongue->fn_8041CB80()) {
            fn_804206E0();
            _60 = true;
        }
    }
    bool complete = false;
    if (mYoshi->fn_8040F5F0() || MR::isGreaterEqualStep(this, 50)) complete = true;
    if (complete && _60) {
        mYoshi->mHeadCtrl->fn_8041AB50(1);
        mTongue->fn_8041CC60();
        if (mTarget->fn_8041C210()) {
            mYoshi->fn_8040EE30("口に含む");
            if (mPlayer->fn_803B1B70()) {
                mPlayer->fn_803B1B20();
                mPlayer->changeAnimation(nullptr, static_cast<const char*>(nullptr));
            }
            mYoshi->mDashTimer = 0;
            fn_804217D0();
            setNerve(&NrvYoshiTongueAction::State8::sInstance);
            return;
        }
        if (mYoshi->mDashTimer > 0) {
            mYoshi->fn_8040EE70(nullptr);
            setNerve(&NrvYoshiTongueAction::State5::sInstance);
            return;
        }
        mYoshi->fn_8040EE30("噛む");
        if (mPlayer->fn_803B1B70()) mPlayer->changeAnimation("噛む", static_cast<const char*>(nullptr));
        setNerve(&NrvYoshiTongueAction::State5::sInstance);
    }
}

void YoshiTongueAction::fn_80420480(bool first) {
    fn_80421790();
    bool attached = mTarget->sendLoopTongueMessage();
    fn_804217B0();
    if (attached) {
        TPos3f tongueMtx;
        mTongue->fn_8041CD10(&tongueMtx);
        if (first) {
            TPos3f inverse(tongueMtx);
            inverse.invert(inverse);
            mTargetMtx.set(mTarget->getCurrentTargetActor()->getBaseMtx());
            mTargetMtx.concat(inverse, mTargetMtx);
        }
        f32 scale = mTongue->fn_8041CEC0();
        TPos3f local(mTargetMtx);
        local.mMtx[0][3] *= scale;
        local.mMtx[1][3] *= scale;
        local.mMtx[2][3] *= scale;
        TPos3f result;
        result.concat(tongueMtx, local);
        LiveActor* actor = mTarget->getCurrentTargetActor();
        MR::setPosition(actor, result.mMtx[0][3], result.mMtx[1][3], result.mMtx[2][3]);
        result.getEulerXYZ(actor->mRotation);
        actor->mRotation.x *= 57.2957763671875f;
        actor->mRotation.y *= 57.2957763671875f;
        actor->mRotation.z *= 57.2957763671875f;
        actor->mScale.setAll<f32>(scale);
    }
}

void YoshiTongueAction::fn_804210B0() {
    TVec3f center;
    mTarget->calcCurrentTargetPosition(&center);
    TVec3f gravity(mPlayer->getAirGravityVec());
    TVec3f offset(mDirection);
    offset.scale(170.0f);
    TVec3f position(center);
    JMathInlineVEC::PSVECSubtract(&position, &offset, &position);
    TVec3f direction(position);
    JMathInlineVEC::PSVECSubtract(&direction, &center, &direction);
    MR::normalize(&direction);
    TVec3f axis;
    PSVECCrossProduct(&direction, &gravity, &axis);
    MR::normalize(&axis);
    TQuat4f rotation;
    rotation.setRotate(axis, 2.757620334625244f);
    TVec3f jump(gravity);
    rotation.transform(jump);
    jump.scale(23.0f);
    TVec3f vertical;
    JMAVECScaleAdd(&gravity, &jump, &vertical, -gravity.dot(jump));
    TVec3f planar;
    JMathInlineVEC::PSVECSubtract(&jump, &vertical, &planar);
    mPlayer->clearVelocity();
    mPlayer->mMovementStates.mHighWord |= 0x40000000;
    mPlayer->tryForceJumpDelay(jump);
    mPlayer->changeAnimation("フックジャンプ", static_cast<const char*>(nullptr));
}

bool YoshiTongueAction::tryStartNearestAutoEat(const TVec3f& position) {
    if (isPopoSticking()) return false;
    _A0 = nullptr;
    s32 count = mYoshi->getAutoEatSensorCount();
    if (!count) return false;
    f32 nearest = 1000.0f;
    bool alternate = false;
    for (s32 i = 0; i < count; ++i) {
        if (!MR::isSensorPlayerOrRide(mYoshi->getAutoEatSensor(i))) {
            HitSensor* messageSensor = MR::getMessageSensor();
            if (sendAutoEatSensorMessage(0xFD, mYoshi->getAutoEatSensor(i), messageSensor)) {
                f32 distance = PSVECDistance(&mYoshi->getAutoEatSensor(i)->mPosition, &position);
                if (nearest > distance) {
                    _A0 = mYoshi->getAutoEatSensor(i);
                    nearest = distance;
                    alternate = false;
                }
            }
            messageSensor = MR::getMessageSensor();
            if (sendAutoEatSensorMessage(0xFE, mYoshi->getAutoEatSensor(i), messageSensor)) {
                f32 distance = PSVECDistance(&mYoshi->getAutoEatSensor(i)->mPosition, &position);
                if (nearest > distance) {
                    _A0 = mYoshi->getAutoEatSensor(i);
                    nearest = distance;
                    alternate = true;
                }
            }
        }
    }
    if (nearest > _A4) return false;
    _2D = true;
    _2E = alternate;
    return sendAutoEatSensorMessage(0xFF, _A0, MR::getMessageSensor());
}

bool YoshiTongueAction::fn_8041F970() const {
    if (isPopoSticking()) return false;
    if (isNerve(&NrvYoshiTongueAction::State0::sInstance)) return true;
    return isNerve(&NrvYoshiTongueAction::State1::sInstance);
}
bool YoshiTongueAction::fn_8041F9E0() const {
    bool result = false;
    if (isNerve(&NrvYoshiTongueAction::State0::sInstance) && !isPopoSticking()) result = true;
    return result;
}
bool YoshiTongueAction::fn_804206E0() {
    if (!mTongue->fn_8041CB80()) return false;
    fn_80421790();
    if (mTarget->fn_8041C210()) mTarget->sendStartEatenMessage();
    else {
        mTarget->sendReleaseEatenMessage();
        fn_80420230(nullptr);
    }
    fn_804217B0();
    if (_2D) MR::tryRumblePadMiddle(this, 0);
    return true;
}
bool YoshiTongueAction::fn_80421410() {
    _2C = true;
    _2D = true;
    fn_80421790();
    bool hit = mTarget->sendStartTongueMessage();
    fn_804217B0();
    _2C = false;
    if (hit) {
        if (fn_804216C0()) MR::stopScene(8);
        MR::tryRumblePadMiddle(this, 0);
        TVec3f position;
        fn_80421830(&position);
        MR::emitEffectHit(mYoshi, position, "TongueHit");
    }
    return hit;
}
void YoshiTongueAction::fn_804214C0() {
    if (fn_80421510()) {
        fn_80421790();
        mTarget->cancelTargetAndAdvance();
        fn_804217B0();
    }
}
bool YoshiTongueAction::fn_80421510() const {
    if (mTarget->fn_8041C210()) {
        if (isNerve(&NrvYoshiTongueAction::State3::sInstance)) return true;
        if (isNerve(&NrvYoshiTongueAction::State4::sInstance)) return true;
        if (isNerve(&NrvYoshiTongueAction::State8::sInstance)) return true;
        return isNerve(&NrvYoshiTongueAction::State9::sInstance);
    }
    if (mTarget->fn_8041C120()) {
        if (isNerve(&NrvYoshiTongueAction::State3::sInstance)) return true;
        if (isNerve(&NrvYoshiTongueAction::State4::sInstance) && !_60) return true;
        return false;
    }
    if (mTarget->fn_8041C140()) {
        if (isNerve(&NrvYoshiTongueAction::State3::sInstance)) return true;
        if (isNerve(&NrvYoshiTongueAction::State7::sInstance) && !_60) return true;
        return false;
    }
    if (mTarget->fn_8041C1E0()) {
        if (isNerve(&NrvYoshiTongueAction::State11::sInstance)) return true;
        if (isNerve(&NrvYoshiTongueAction::State17::sInstance) && !_A8) return true;
    }
    return false;
}
bool YoshiTongueAction::fn_804216C0() const {
    bool result = false;
    if (mTarget->fn_8041C140() || mTarget->fn_8041C120() || mTarget->fn_8041C210()) result = true;
    return result;
}

void YoshiTongueAction::fn_8041DFD0() {
    if (MR::isFirstStep(this)) {
        static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->startLockOn(mYoshi);
        mYoshi->mHeadCtrl->fn_8041AB50(1);
        mYoshi->fn_8040EF30();
    }
    if (mYoshi->fn_8040F5B0("口を閉じる") && mYoshi->fn_8040F5F0()) mYoshi->fn_8040EE70(nullptr);
    MR::requestStarPointerModeCommandStream(this);
    if (MR::testCorePadTriggerB(0)) {
        if (isPopoSticking()) {
            tryReactStickingPopo();
            return;
        }
        mTarget->takeLockOnTargets();
        static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->endLockOn();
        if (mTarget->hasCurrentTarget()) {
            setNerve(&NrvYoshiTongueAction::State2::sInstance);
            mTongue->fn_8041CB30(false);
            return;
        }
        setNerve(&NrvYoshiTongueAction::State14::sInstance);
        return;
    }
    TVec3f position;
    fn_80421830(&position);
    if (tryStartNearestAutoEat(position)) {
        static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->endLockOn();
        setNerve(&NrvYoshiTongueAction::State16::sInstance);
    }
}
void YoshiTongueAction::fn_8041E150() {
    if (MR::isFirstStep(this)) {
        _A4 = 200.0f;
        MR::startStarPointerModePictureBook(this);
    }
    if (tryStartNearestAutoEat(mYoshi->getAutoEatSensorPosition())) {
        sendAutoEatSensorMessage(0x100, _A0, MR::getMessageSensor());
        LiveActor* actor = MR::getSensorHost(_A0);
        if (actor) fn_80420230(actor);
        if (_2D) MR::tryRumblePadMiddle(this, 0);
        MR::stopScene(8);
        mPlayer->_A72 = true;
    }
}
void YoshiTongueAction::fn_8041E470() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(mYoshi, "GrabTarget", -1, -1, -1);
        mYoshi->mHeadCtrl->fn_8041AD20(1);
    }
    if (MR::isStep(this, 3)) MR::startActionSound(mYoshi, "PullToMouth", -1, -1, -1);
    fn_80420480(MR::isFirstStep(this));
    if (mTongue->fn_8041CB90()) {
        if (mTarget->fn_8041C1E0() || mTarget->fn_8041C210()) {
            _60 = false;
            setNerve(&NrvYoshiTongueAction::State4::sInstance);
            return;
        }
        if (!mTarget->hasNextTarget()) {
            if (mTarget->fn_8041C140()) {
                _60 = false;
                setNerve(&NrvYoshiTongueAction::State7::sInstance);
                return;
            }
            if (mTarget->fn_8041C120()) {
                _60 = false;
                setNerve(&NrvYoshiTongueAction::State4::sInstance);
                return;
            }
        }
    }
    if (mTongue->fn_8041CB80()) {
        fn_80421790();
        mTarget->sendReleaseEatenMessage();
        fn_804217B0();
        fn_80420230(nullptr);
        if (!isNerve(&NrvYoshiTongueAction::State1::sInstance)) {
            if (isPopoSticking() || !isNerve(&NrvYoshiTongueAction::State3::sInstance)) {
                _60 = false;
                setNerve(&NrvYoshiTongueAction::State7::sInstance);
                return;
            }
            mTarget->advanceTargetIfMore();
            if (_2D) MR::tryRumblePadMiddle(this, 0);
            setNerve(&NrvYoshiTongueAction::State2::sInstance);
            mTongue->fn_8041CB30(true);
        }
    }
}
void NrvYoshiTongueAction::State18::execute(Spine* spine) const {
    YoshiTongueAction* action = static_cast<YoshiTongueAction*>(spine->mExecutor);
    if (MR::isFirstStep(action)) MR::startStarPointerModePictureBook(action);
}

// NONMATCHING: redundant ori r3,r3,0 before boolean normalization is omitted.
bool YoshiTongueAction::fn_80421210() const {
    return mPlayer->fn_803EF000() != 0;
}
bool YoshiTongueAction::fn_80421240() {
    TVec3f ground;
    mPlayer->mActor->getGroundPos(&ground);
    TVec3f offset(mPlayer->mActor->getGravityVec());
    offset.scale(-75.0f);
    ground += offset;
    if (!MR::isStarPointerInScreen(0)) {
        mPlayer->mActor->getFrontVec(&_90);
        return false;
    }
    TVec3f negativeGravity;
    JGeometry::negateInternal(&mPlayer->mActor->getGravityVec().x, &negativeGravity.x);
    TVec3f target;
    MR::calcStarPointerPosOnPlane(&target, ground, negativeGravity, 0, false);
    TVec3f start;
    fn_80421830(&start);
    TVec3f direction(target);
    JMathInlineVEC::PSVECSubtract(&direction, &start, &direction);
    if ((mPlayer->mMovementStates.mLowWord >> 30) & 1) {
        const TVec3f& gravity = mPlayer->mActor->getGravityVec();
        JMAVECScaleAdd(&gravity, &direction, &direction, -gravity.dot(direction));
    }
    if (MR::normalizeOrZero(&direction)) {
        mPlayer->mActor->getFrontVec(&_90);
        return false;
    }
    _90.set(direction);
    if (!MR::isDead(mArrow)) {
        MtxPtr mtx = mYoshi->getBaseMtx();
        TVec3f up(mtx[0][1], mtx[1][1], mtx[2][1]);
        mArrow->fn_80413DC0(start, target, up);
        mTargetPosition.set(target);
    }
    return true;
}
void YoshiTongueAction::fn_8041EBE0() {
    if (MR::isFirstStep(this)) {
        mYoshi->mHeadCtrl->fn_8041AA70(1, &mTargetPosition);
        mArrow->appear();
        mArrow->fn_80413FB0();
    }
    mPlayer->mWalkSpeed = 0.9789999723434448f * mPlayer->mWalkSpeed;
    if (!fn_80421240()) {
        TVec3f start;
        fn_80421830(&start);
        MtxPtr mtx = mYoshi->getBaseMtx();
        TVec3f up(mtx[0][1], mtx[1][1], mtx[2][1]);
        if (!MR::isDead(mArrow)) {
            TVec3f offset(_90);
            offset.scale(300.0f);
            TVec3f target(start);
            target += offset;
            mArrow->fn_80413DC0(start, target, up);
            mTargetPosition.set(target);
        }
    }
    if (mTarget->getCurrentTargetActor()) {
        if (MR::isDead(mTarget->getCurrentTargetActor())) {
            setNerve(&NrvYoshiTongueAction::State0::sInstance);
            mYoshi->fn_8040EE70(nullptr);
            mYoshi->mHeadCtrl->fn_8041AC40(1);
            fn_80421790();
            fn_804217B0();
            MR::tryDeleteEffect(mYoshi, "KeepMouthKiller");
            mYoshi->fn_8040F020();
            mYoshi->fn_8040F0A0();
            mYoshi->fn_8040F0D0();
            return;
        }
    }
    if (mPlayer->mActor->_934) {
        mYoshi->mHeadCtrl->fn_8041AB50(1);
        setNerve(&NrvYoshiTongueAction::State9::sInstance);
        return;
    }
    if (fn_80421210()) setNerve(&NrvYoshiTongueAction::State10::sInstance);
}

bool MR::fn_80421D10() {
    return MR::getMarioHolder()->getMarioActor()->fn_803B9840();
}
