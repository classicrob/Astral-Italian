#define JGEOMETRY_VEC3_WMR_OUTLINE
#define JGEOMETRY_VEC3_MI_NOINLINE
#define JGEOMETRY_VEC3_MUL_NOINLINE
#include "Game/System/MiniMeister.hpp"
#include "Game/System/WorldMapCameraHolder.hpp"

#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/System/MiniPlayerRocket.hpp"

namespace {
    extern const TVec2f sTalkScreenPos;
    extern const TVec3f sTalkOffset;
}  // namespace

namespace NrvMiniMeister {
    NEW_NERVE(MiniMeisterNrvTalkStart, MiniMeister, TalkStart);
    NEW_NERVE(MiniMeisterNrvTalkStartWait, MiniMeister, TalkStartWait);
    NEW_NERVE(MiniMeisterNrvWait, MiniMeister, Wait);
    NEW_NERVE(MiniMeisterNrvTalk, MiniMeister, Talk);
    NEW_NERVE(MiniMeisterNrvTalkEndWait, MiniMeister, TalkEndWait);
    NEW_NERVE(MiniMeisterNrvTalkEnd, MiniMeister, TalkEnd);
    NEW_NERVE(MiniMeisterNrvDisappear, MiniMeister, Disappear);
};  // namespace NrvMiniMeister

// NONMATCHING: regalloc; front-vector stores scheduled after screen-position member construction.
MiniMeister::MiniMeister(WorldMapCameraHolder* pOwner) : LiveActor("ミニマイスター"), mFront(), mScreenPos(sTalkScreenPos) {
    mFront.x = 0.0f;
    mFront.y = 0.0f;
    mFront.z = 1.0f;
    mOwner = pOwner;
}

MiniMeister::~MiniMeister() {}

void MiniMeister::init(const JMapInfoIter& rIter) {
    if (!MR::initActor(this, rIter, "Meister", "MeisterAnim", "Mini", false)) {
        MR::connectToSceneNpc(this);
        initEffectKeeper(8, NULL, false);
    }
    initNerve(&NrvMiniMeister::MiniMeisterNrvWait::sInstance, 0);
    MR::invalidateClipping(this);
    MR::registerDemoSimpleCastAll(this);
    makeActorDead();
}

void MiniMeister::appear() {
    LiveActor::appear();
    mFront.set(0.0f, 0.0f, 1.0f);
    mScreenPos.set(sTalkScreenPos);
    setNerve(&NrvMiniMeister::MiniMeisterNrvTalkStart::sInstance);
}

void MiniMeister::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::emitEffect(this, "HeadLight");
    MR::showModel(this);
}

void MiniMeister::makeActorDead() {
    MR::forceDeleteEffect(this, "HeadLight");
    LiveActor::makeActorDead();
}

void MiniMeister::calcAndSetBaseMtx() {
    MR::calcAndSetBaseMtxFromFrontAndGravity(this, &mFront);
}

void MiniMeister::control() {
    if (!isNerve(&NrvMiniMeister::MiniMeisterNrvTalkStart::sInstance) && !isNerve(&NrvMiniMeister::MiniMeisterNrvTalkEnd::sInstance)) {
        if (isNerve(&NrvMiniMeister::MiniMeisterNrvDisappear::sInstance)) {
            updateTalkScreenPos(0.0f);
            return;
        }
        updateTalkScreenPos(1.0f);
    }
}

// fn_804E3B20: appears and moves to the given screen position.
void MiniMeister::appearAtScreenPos(const TVec2f& rPos) {
    appear();
    mScreenPos.set(rPos);
}

// fn_804E3B70
void MiniMeister::setNerveTalkEndWait() {
    setNerve(&NrvMiniMeister::MiniMeisterNrvTalkEndWait::sInstance);
}

// fn_804E3B80
void MiniMeister::setNerveTalk() {
    setNerve(&NrvMiniMeister::MiniMeisterNrvTalk::sInstance);
}

// fn_804E3B90
s32 MiniMeister::isTalkStarting() const {
    s32 result = 0;
    if (isNerve(&NrvMiniMeister::MiniMeisterNrvTalkStart::sInstance) || isNerve(&NrvMiniMeister::MiniMeisterNrvTalkStartWait::sInstance)) {
        result = 1;
    }
    return result;
}

// fn_804E3BF0
s32 MiniMeister::isTalkEnding() const {
    s32 result = 0;
    if (!MR::isDead(this) && (isNerve(&NrvMiniMeister::MiniMeisterNrvTalkEnd::sInstance) || isNerve(&NrvMiniMeister::MiniMeisterNrvTalkEndWait::sInstance) || isNerve(&NrvMiniMeister::MiniMeisterNrvDisappear::sInstance))) {
        result = 1;
    }
    return result;
}

void MiniMeister::calcPositionFromCamera(const TVec3f& rPos) {
    TVec3f root;
    TVec3f center;
    MR::extractMtxTrans(MR::getJointMtx(this, "All_Root"), &root);
    MR::extractMtxTrans(MR::getJointMtx(this, "Center"), &center);
    TVec3f offset = root - center;
    TVec3f pos = offset + rPos;
    MR::setPosition(this, pos);
    TVec3f direction = MR::getCamPos() - mPosition;
    MR::normalizeOrZero(&direction);
    if (!MR::isSameDirection(direction, mGravity, 0.01f)) {
        TVec3f negative;
        negative.negate(direction);
        if (!MR::isSameDirection(negative, mGravity, 0.01f)) {
            mFront.set<f32>(direction);
        }
    }
}

// NONMATCHING: register/stack scheduling or equivalent branch lowering; calls, values and fields audited.
void MiniMeister::updateTalkScreenPos(f32 ratio) {
    TVec2f screen;
    screen.x = mScreenPos.x;
    screen.y = mScreenPos.y;
    f32 halfTan = MR::tan(0.017453292f * (0.5f * MR::getFovy()));
    TVec3f worldPos;
    MR::calcWorldPositionFromCenterScreen(&worldPos, screen, (875.0f * MR::tan(0.3926991f)) / halfTan);
    TVec3f anchor;
    anchor.set<f32>(mOwner->mRocket->mPosition + sTalkOffset);
    TVec2f anchorScreen;
    MR::calcScreenPosition(&anchorScreen, anchor);
    TVec2f worldScreen;
    MR::calcScreenPosition(&worldScreen, worldPos);
    f32 distA = (anchor - MR::getCamPos()).length();
    f32 distB = (worldPos - MR::getCamPos()).length();
    f32 scale = distB / distA;
    f32 blend = scale + (ratio * (1.0f - scale));
    TVec2f a = worldScreen * blend;
    TVec2f b = anchorScreen * (1.0f - blend);
    TVec2f mix = a + b;
    TVec3f dir;
    MR::calcWorldRayDirectionFromScreen(&dir, mix);
    MR::normalize(&dir);
    TVec3f scaled = dir * (1.0f / blend);
    TVec3f delta = scaled * distB;
    TVec3f pos = MR::getCamPos() + delta;
    calcPositionFromCamera(pos);
}

// fn_804E3F80: starts one of the talk actions by type; always returns 1.
bool MiniMeister::startTalkAction(u32 type) {
    switch (type) {
    case 0:
        MR::startAction(this, "Talk");
        return 1;
    case 1:
        MR::startAction(this, "TalkCool");
        return 1;
    case 2:
        MR::startAction(this, "TalkAngry");
        return 1;
    default:
        return 1;
    }
}

// fn_804E4000
void MiniMeister::exeTalkStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "WorldMapTalkStart");
    }
    f32 rate = MR::calcNerveRate(this, 30);
    f32 ease = MR::getEaseOutValue(rate, 0.0f, 1.0f, 1.0f);
    updateTalkScreenPos(ease);
    if (MR::isGreaterEqualStep(this, 30)) {
        setNerve(&NrvMiniMeister::MiniMeisterNrvTalkStartWait::sInstance);
    }
}

// fn_804E4090
void MiniMeister::exeTalkStartWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "WorldMapTalkStartWait");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvMiniMeister::MiniMeisterNrvWait::sInstance);
    }
}

// fn_804E44C0
void MiniMeister::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
}

// fn_804E40F0: turns the meister toward the camera.
// NONMATCHING: register/stack scheduling or equivalent branch lowering; calls, values and fields audited.
void MiniMeister::exeTalk() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Talk");
    }
    TQuat4f quat;
    quat.setEuler(mRotation.x, mRotation.y, mRotation.z);
    TVec3f camDiff = MR::getCamPos() - mPosition;
    TVec3f toCam;
    MR::normalizeOrZero(&toCam, camDiff);
    TQuat4f turned;
    MR::turnQuatZDirRad(&turned, quat, toCam, 0.017453292f);
    TVec3f euler;
    turned.getEuler(euler);
    MR::setRotation(this, euler.x * 57.29578f, euler.y * 57.29578f, euler.z * 57.29578f);
}

// fn_41B0
void MiniMeister::exeTalkEndWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "WorldMapTalkEndWait");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvMiniMeister::MiniMeisterNrvTalkEnd::sInstance);
    }
}

// fn_804E4210
void MiniMeister::exeTalkEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "WorldMapTalkEnd");
    }
    f32 rate = MR::calcNerveRate(this, 30);
    f32 ease = MR::getEaseInValue(rate, 0.0f, 1.0f, 1.0f);
    updateTalkScreenPos(1.0f - ease);
    if (MR::isGreaterEqualStep(this, 30)) {
        MR::tryRumblePadMiddle(mOwner->mRocket, 0);
        static_cast<MiniPlayerRocket*>(mOwner->mRocket)->startBounce();
        setNerve(&NrvMiniMeister::MiniMeisterNrvDisappear::sInstance);
    }
}

// fn_804E42C0
void MiniMeister::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
    }
    if (MR::isGreaterEqualStep(this, 30)) {
        kill();
    }
}


namespace {
    const TVec2f sTalkScreenPos(140.0f, 0.0f);
    const TVec3f sTalkOffset(0.0f, 2000.0f, -500.0f);
}
