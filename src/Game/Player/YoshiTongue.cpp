#include "Game/Player/YoshiTongue.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/FixedPosition.hpp"
#include <cstdio>

#pragma auto_inline off

namespace {
    const char sTonguePos[] = "TonguePos";
    const f32 sJointLengths[] = {37.16f, 47.83f, 47.35f, 39.48f};
}

namespace NrvYoshiTongue {
    NEW_NERVE(State0, YoshiTongue, State0);
    NEW_NERVE(State1, YoshiTongue, State1);
    class State2 : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static State2 sInstance;
    };
    State2 State2::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(State3, YoshiTongue, State3);
    class State4 : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static State4 sInstance;
    };
    State4 State4::sInstance ATTRIBUTE_WEAK;
    class State5 : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static State5 sInstance;
    };
    State5 State5::sInstance ATTRIBUTE_WEAK;
    class State6 : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static State6 sInstance;
    };
    State6 State6::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(State7, YoshiTongue, State7);
}

YoshiTongue::YoshiTongue(Yoshi* yoshi, const char* name) : LiveActor(name) {
    mYoshi = yoshi;
    mFixedPosition = nullptr;
    mMouthMtx = nullptr;
    mTipPosition.x = 0.0f;
    mTipPosition.y = 0.0f;
    mTipPosition.z = 0.0f;
    mTipDirection.x = 0.0f;
    mTipDirection.y = 0.0f;
    mTipDirection.z = 0.0f;
    mProgress = 0.0f;
    mUseMouthDirection = true;
    mHitMap = false;
    mCollisionProgress = 0.0f;
    mCollisionPosition.x = 0.0f;
    mCollisionPosition.y = 0.0f;
    mCollisionPosition.z = 0.0f;
    mCurve = new YoshiTongueCurve;
    mExtendFrames = 0;
    mRetractFrames = 0;
    mRetractOffset.x = 0.0f;
    mRetractOffset.y = 0.0f;
    mRetractOffset.z = 0.0f;
    mRetractProgress = 0.0f;
}

YoshiTongue::~YoshiTongue() {}

void YoshiTongue::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, "YoshiTongue", false);
    MR::invalidateClipping(this);
    initEffectKeeper(0, nullptr, false);
    MR::setEffectHostSRT(this, "TongueCollisionHit", &mCollisionPosition, nullptr, nullptr);
    initSound(2, nullptr, &mTipPosition, TVec3f(0.0f));
    MR::connectToScene(this, 40, 11, 17, -1);
    MR::startAction(this, "YoshiTongueWait");
    mFixedPosition = new FixedPosition(mYoshi, sTonguePos, TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    mMouthMtx = mFixedPosition->mMtx.toMtxPtr();
    for (s32 index = 0; index < 4; ++index) {
        char name[64];
        snprintf(name, sizeof(name), "Tongue%d", index + 1);
        mJoints[index] = new YoshiTongueJoint(this, name, sJointLengths[index]);
    }
    initNerve(&NrvYoshiTongue::State0::sInstance, 0);
    fn_8041D180(mCurve);
    makeActorDead();
}

void YoshiTongue::control() { fn_8041D2B0(); }
void YoshiTongue::makeActorAppeared() { fn_8041D460(); LiveActor::makeActorAppeared(); }
void YoshiTongue::makeActorDead() { LiveActor::makeActorDead(); setNerve(&NrvYoshiTongue::State0::sInstance); }

void YoshiTongue::exeState0() {
    mProgress = 0.0f;
    mTipPosition.set<f32>(mMouthMtx[0][3], mMouthMtx[1][3], mMouthMtx[2][3]);
    mTipDirection.zero();
}
void YoshiTongue::exeState1() {
    if (MR::isFirstStep(this)) mCollisionProgress = 0.0f;
    if (MR::isLessStep(this, mExtendFrames)) {
        f32 t = static_cast<f32>(getNerveStep()) / static_cast<f32>(mExtendFrames);
        mProgress = t * t;
        return;
    }
    mProgress = 1.0f;
}
void YoshiTongue::exeState3() {
    if (MR::isFirstStep(this)) mRetractProgress = 1.0f;
    if (MR::isLessStep(this, mRetractFrames)) {
        f32 t = 1.0f - static_cast<f32>(getNerveStep()) / static_cast<f32>(mRetractFrames);
        mProgress = t * t;
        return;
    }
    mProgress = 0.0f;
}
void YoshiTongue::exeState7() {
    if (MR::isFirstStep(this)) {
        mRetractProgress = mProgress;
        TVec3f mouth(mMouthMtx[0][3], mMouthMtx[1][3], mMouthMtx[2][3]);
        JMathInlineVEC::PSVECSubtract(&mTipPosition, &mouth, &mRetractOffset);
    }
    mTipPosition.set<f32>(mMouthMtx[0][3], mMouthMtx[1][3], mMouthMtx[2][3]);
    mTipPosition += mRetractOffset;
    if (MR::isLessStep(this, 12)) {
        f32 t = static_cast<f32>(getNerveStep()) / 12.0f;
        f32 progress = 1.0f - t * t;
        progress *= mRetractProgress;
        mProgress = progress;
        return;
    }
    mProgress = 0.0f;
}
void YoshiTongue::fn_8041CB30(bool quick) {
    mExtendFrames = quick ? 1 : 5;
    setNerve(&NrvYoshiTongue::State1::sInstance);
}
bool YoshiTongue::fn_8041CB50() const { return 1.0f == mProgress; }
void YoshiTongue::fn_8041CB70(bool) { mRetractFrames = 12; setNerve(&NrvYoshiTongue::State3::sInstance); }
bool YoshiTongue::fn_8041CB80() const { return MR::isGreaterEqualStep(this, mRetractFrames); }
bool YoshiTongue::fn_8041CB90() const { return MR::isGreaterEqualStep(this, mRetractFrames - 12); }
void YoshiTongue::fn_8041CBA0() {
    if (!isNerve(&NrvYoshiTongue::State7::sInstance)) {
        if (isNerve(&NrvYoshiTongue::State0::sInstance)) return;
        setNerve(&NrvYoshiTongue::State7::sInstance);
    }
}
bool YoshiTongue::fn_8041CC00() const {
    bool complete = false;
    if (!isNerve(&NrvYoshiTongue::State7::sInstance) || MR::isGreaterEqualStep(this, 12)) complete = true;
    return complete;
}
void YoshiTongue::fn_8041CC60() { if (!isNerve(&NrvYoshiTongue::State0::sInstance)) setNerve(&NrvYoshiTongue::State0::sInstance); }
void YoshiTongue::fn_8041CCB0(const TVec3f& direction) { mTipDirection.set(direction); mUseMouthDirection = false; }
void YoshiTongue::fn_8041CCF0() { mUseMouthDirection = true; }
bool YoshiTongue::fn_8041CD00() const { return mHitMap; }
f32 YoshiTongue::fn_8041CEC0() const { return 0.8f + 0.19999998807907104f * mProgress; }
void YoshiTongue::fn_8041CEE0() { MR::emitEffect(this, "TongueCollisionHit"); }
void YoshiTongue::fn_8041D460() {
    mFixedPosition->calc();
    mTipPosition.set<f32>(mMouthMtx[0][3], mMouthMtx[1][3], mMouthMtx[2][3]);
    mTipDirection.zero();
    mProgress = 0.0f;
    mExtendFrames = 0;
}

YoshiTongueCurve::YoshiTongueCurve() { mInitialized = false; }

namespace {
    void fn_8041D910(TVec3f* output, const TVec3f& start, const TVec3f& startTangent, const TVec3f& endTangent, const TVec3f& end, f32 t) {
        f32 square = t * t;
        f32 cube = square * t;
        f32 tripleSquare = 3.0f * square;
        f32 endWeight = -2.0f * cube + tripleSquare;
        f32 startTangentWeight = t + (cube - 2.0f * square);
        f32 startWeight = 1.0f + (2.0f * cube - tripleSquare);
        f32 endTangentWeight = cube - square;
        output->x = endTangentWeight * endTangent.x + (startTangentWeight * startTangent.x + (startWeight * start.x + endWeight * end.x));
        output->y = endTangentWeight * endTangent.y + (startTangentWeight * startTangent.y + (startWeight * start.y + endWeight * end.y));
        output->z = endTangentWeight * endTangent.z + (startTangentWeight * startTangent.z + (startWeight * start.z + endWeight * end.z));
    }
}

void YoshiTongueCurve::fn_8041D750(const TVec3f& start, const TVec3f& startTangent, const TVec3f& endTangent, const TVec3f& end) {
    mControl[0].set(start);
    mControl[1].set(startTangent);
    mControl[2].set(endTangent);
    mControl[3].set(end);
    for (u32 index = 1; index < 6; ++index) fn_8041D910(&mSamples[index], start, startTangent, endTangent, end, static_cast<f32>(index) / 7.0f);
    mSamples[0].set(start);
    mSamples[6].set(end);
    f32 length = 0.0f;
    for (u32 index = 1; index <= 6; ++index) {
        f32 distance = PSVECDistance(&mSamples[index], &mSamples[index - 1]);
        mDistances[index] = distance + length;
        length += distance;
    }
    mDistances[0] = 0.0f;
    if (length > 0.0f) {
        for (s32 index = 1; index < 6; ++index) mDistances[index] /= length;
        mDistances[6] = 1.0f;
    } else {
        for (s32 index = 1; index <= 6; ++index) mDistances[index] = 0.0f;
    }
    mInitialized = true;
}

void YoshiTongueCurve::fn_8041DA20(TVec3f* output, f32 progress) const {
    u32 index;
    for (index = 1; index < 7; ++index) if (mDistances[index] >= progress) break;
    if (index == 7) index = 6;
    f32 previous = mDistances[index - 1];
    f32 span = mDistances[index] - previous;
    f32 t;
    if (span == 0.0f) t = static_cast<f32>(index - 1) / 7.0f;
    else {
        f32 weight = (progress - previous) / span;
        t = ((1.0f - weight) * static_cast<f32>(index - 1) + weight * static_cast<f32>(index)) / 7.0f;
    }
    fn_8041D910(output, mControl[0], mControl[1], mControl[2], mControl[3], t);
}

YoshiTongueJoint::YoshiTongueJoint(LiveActor* actor, const char* name, f32 weight) {
    mController = MR::createJointController<YoshiTongueJoint>(this, actor, name, &YoshiTongueJoint::fn_8041D5B0, 0);
    mWeight = weight;
}

bool YoshiTongueJoint::fn_8041D5B0(TPos3f* matrix, const JointControllerInfo&) {
    TVec3f up;
    TVec3f front;
    TVec3f side;
    matrix->getYDir(up);
    matrix->getZDir(front);
    PSVECCrossProduct(&up, &front, &side);
    if (MR::normalizeOrZero(&side)) return false;
    TQuat4f rotation;
    rotation.setRotate(side, mDirection);
    rotation.transform(side);
    rotation.transform(up);
    rotation.transform(front);
    if (mWeight == 0.0f) side.zero();
    else side.setLength(mLength / mWeight);
    matrix->setXYZDir(side, up, front);
    matrix->setTrans(mPosition);
    return true;
}

void YoshiTongueJoint::fn_8041D6E0(const TVec3f& position, const TVec3f& direction, f32 length) {
    mPosition.set(position);
    mDirection.set(direction);
    mLength = length;
}

void YoshiTongue::fn_8041D180(YoshiTongueCurve* curve) {
    TVec3f start(mMouthMtx[0][3], mMouthMtx[1][3], mMouthMtx[2][3]);
    TVec3f difference(mTipPosition);
    difference.sub(start);
    TVec3f direction(difference);
    if (MR::normalizeOrZero(&direction)) mFixedPosition->mMtx.getZDir(direction);
    TVec3f front;
    TVec3f up;
    mFixedPosition->mMtx.getZDir(front);
    mFixedPosition->mMtx.getYDir(up);
    TQuat4f rotation;
    rotation.setRotate(front, direction);
    rotation.transform(up);
    TVec3f tangent(front);
    tangent.setLength(PSVECMag(&difference));
    curve->fn_8041D750(start, tangent, mTipDirection, mTipPosition);
}

f32 YoshiTongue::fn_8041D290(f32 progress, f32 t) const { return ((0.5f + t) * (progress * t)) / 1.5f; }

void YoshiTongue::calcAndSetBaseMtx() {
    mFixedPosition->calc();
    mPosition.set<f32>(mMouthMtx[0][3], mMouthMtx[1][3], mMouthMtx[2][3]);
    MR::setBaseTRMtx(this, mMouthMtx);
    if (mUseMouthDirection) {
        TVec3f difference(mTipPosition);
        difference.sub(mPosition);
        mTipDirection.set(difference);
        TVec3f mouthGravity;
        TVec3f tipGravity;
        MR::calcGravityVector(this, mPosition, &mouthGravity, nullptr, 0);
        MR::calcGravityVector(this, mTipPosition, &tipGravity, nullptr, 0);
        if (mouthGravity.dot(tipGravity) < 0.999f) {
            f32 length = PSVECMag(&mTipDirection);
            JMAVECScaleAdd(&tipGravity, &mTipDirection, &mTipDirection, -tipGravity.dot(mTipDirection));
            mTipDirection.setLength(0.3f * length);
        } else mTipDirection.setLength(1.0f);
    }
    fn_8041D180(mCurve);
    TVec3f direction;
    mFixedPosition->mMtx.getZDir(direction);
    for (u32 index = 0; index < 4; ++index) {
        TVec3f start;
        TVec3f end;
        mCurve->fn_8041DA20(&start, ((0.5f + static_cast<f32>(index) * 0.25f) * (mProgress * (static_cast<f32>(index) * 0.25f))) / 1.5f);
        mCurve->fn_8041DA20(&end, fn_8041D290(mProgress, static_cast<f32>(index + 1) * 0.25f));
        TVec3f tangent;
        JMathInlineVEC::PSVECSubtract(&end, &start, &tangent);
        if (!MR::normalizeOrZero(&tangent)) direction = tangent;
        f32 length = PSVECDistance(&start, &end);
        mJoints[index]->fn_8041D6E0(start, direction, length);
        mJoints[index]->mController->registerCallBack();
    }
}

bool YoshiTongue::fn_8041D3C0() const {
    bool inactive = false;
    if (isNerve(&NrvYoshiTongue::State0::sInstance) || isNerve(&NrvYoshiTongue::State3::sInstance) || isNerve(&NrvYoshiTongue::State4::sInstance) || isNerve(&NrvYoshiTongue::State5::sInstance) || isNerve(&NrvYoshiTongue::State7::sInstance)) inactive = true;
    return inactive;
}

// NONMATCHING: inline quaternion-to-matrix scheduling and temporary FPR allocation.
void YoshiTongue::fn_8041CD10(TPos3f* output) const {
    TVec3f position;
    TVec3f ahead;
    mCurve->fn_8041DA20(&position, mProgress);
    mCurve->fn_8041DA20(&ahead, 0.01f + mProgress);
    TVec3f direction(ahead);
    direction.sub(position);
    if (MR::normalizeOrZero(&direction)) {
        output->set(getBaseMtx());
        output->setTrans(position);
        return;
    }
    TPos3f base;
    base.set(getBaseMtx());
    TVec3f front;
    base.getZDir(front);
    TQuat4f rotation;
    rotation.setRotate(front, direction);
    TPos3f turn;
    turn.setQuat(rotation);
    output->concat(turn, base);
    output->setTrans(position);
}

void YoshiTongue::fn_8041D2B0() {
    mHitMap = false;
    if (fn_8041D3C0()) return;
    f32 endProgress;
    if (isNerve(&NrvYoshiTongue::State1::sInstance)) endProgress = mProgress;
    else {
        endProgress = 0.2f + mCollisionProgress;
        if (endProgress > mProgress) endProgress = mProgress;
    }
    TVec3f start;
    TVec3f segment;
    mCurve->fn_8041DA20(&start, mCollisionProgress);
    mCurve->fn_8041DA20(&segment, endProgress);
    segment.sub(start);
    Triangle triangle;
    if (MR::getFirstPolyOnLineToMap(&mCollisionPosition, &triangle, start, segment)) mHitMap = true;
    if (!isNerve(&NrvYoshiTongue::State1::sInstance) && endProgress == mProgress) {
        mCollisionProgress = 0.0f;
        return;
    }
    mCollisionProgress = endProgress;
}
