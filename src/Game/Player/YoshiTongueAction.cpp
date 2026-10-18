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
        action->mTarget->fn_8041C050();
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
    if (mTarget->fn_8041C0E0() && fn_804201A0()) mTarget->fn_8041C240(&mTargetPosition);
    if (!isNerve(&NrvYoshiTongueAction::State8::sInstance) && !isNerve(&NrvYoshiTongueAction::State9::sInstance)) MR::tryDeleteEffect(mYoshi, "KeepMouthKiller");
}

void YoshiLockOnManager::fn_8041E120(LiveActor* actor) {
    mContextActive = true;
    _1D = false;
    mContextActor = actor;
}
void YoshiLockOnManager::fn_8041E140() {
    mContextActive = false;
    _1E = false;
    fn_8041A0B0();
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
void YoshiTongueAction::fn_8041FB40(TVec3f* output) const { mTarget->fn_8041C240(output); }
f32 YoshiTongueAction::fn_8041FB50() const { return 4.0f; }

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
    static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->fn_8041E140();
    mYoshi->fn_8040EE70(nullptr);
    mYoshi->mHeadCtrl->fn_8041AB50(1);
    mYoshi->fn_8040EF30();
    MR::tryDeleteEffect(mYoshi, "KeepMouthKiller");
    setNerve(&NrvYoshiTongueAction::State0::sInstance);
    return true;
}
bool YoshiTongueAction::fn_8041FD80() {
    fn_804214C0();
    static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->fn_8041E140();
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
    static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->fn_8041E120(mYoshi);
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
    static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->fn_8041E140();
    setNerve(&NrvYoshiTongueAction::State18::sInstance);
}
void YoshiTongueAction::fn_80420000() {
    static_cast<YoshiLockOnManager*>(MR::getMarioHolder()->getMarioActor()->fn_803B9820())->fn_8041E120(mYoshi);
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

#pragma push
#pragma dont_inline on
void YoshiTongueAction::fn_80420C30(TVec3f* position, const TVec3f& target, const TVec3f& gravity, f32 angle) const {
    TVec3f center;
    mTarget->fn_8041C240(&center);
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
#pragma pop

void YoshiTongueAction::fn_80420D20(const TVec3f& position, const TVec3f& target, const TVec3f& gravity, f32 progress) {
    TPos3f original;
    original.set(mYoshi->getBaseMtx());
    TVec3f up(target);
    JMathInlineVEC::PSVECSubtract(&up, &mStartPosition, &up);
    if (MR::normalizeOrZero(&up)) original.getYDir(up);
    TVec3f side;
    PSVECCrossProduct(&up, &gravity, &side);
    if (MR::normalizeOrZero(&side)) side.set<f32>(original.mMtx[0][0], original.mMtx[1][0], original.mMtx[2][0]);
    TVec3f delta(target);
    JMathInlineVEC::PSVECSubtract(&delta, &position, &delta);
    up = delta;
    if (MR::normalizeOrZero(&up)) original.getYDir(up);
    TVec3f front;
    PSVECCrossProduct(&side, &up, &front);
    if (MR::normalizeOrZero(&front)) front.set<f32>(original.mMtx[0][2], original.mMtx[1][2], original.mMtx[2][2]);
    PSVECCrossProduct(&up, &front, &side);
    TPos3f oriented;
    oriented.identity();
    oriented.setXYZDir(side, up, front);
    TQuat4f originalRotation;
    TQuat4f targetRotation;
    original.getQuat(originalRotation);
    oriented.getQuat(targetRotation);
    TQuat4f rotation(originalRotation);
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
    TVec3f velocity(position);
    JMathInlineVEC::PSVECSubtract(&velocity, &mYoshi->mPosition, &velocity);
    mPlayer->addVelocity(velocity);
}

void YoshiTongueAction::fn_80421830(TVec3f* output) const {
    TPos3f matrix;
    mTongue->fn_8041CD10(&matrix);
    output->set<f32>(matrix.mMtx[0][3], matrix.mMtx[1][3], matrix.mMtx[2][3]);
}
