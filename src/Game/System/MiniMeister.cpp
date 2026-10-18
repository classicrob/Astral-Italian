#include "Game/System/MiniMeister.hpp"

#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace {
    const TVec2f sTalkScreenPos(140.0f, 0.0f);
    const TVec3f sTalkOffset(0.0f, 2000.0f, -500.0f);
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

MiniMeister::MiniMeister() : LiveActor("\x83\x7E\x83\x6A\x83\x7D\x83\x43\x83\x58\x83\x5E\x81\x5B"), mScreenPos(sTalkScreenPos) {
    mUnk90 = 0.0f;
    mUnk94 = 0.0f;
    mUnk98 = 1.0f;
    // NONMATCHING: the target stores an uninitialized register (r4) into mOwner.
    mOwner = NULL;
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
    mUnk90 = 0.0f;
    mUnk94 = 0.0f;
    mUnk98 = 1.0f;
    mScreenPos.set(sTalkScreenPos);
    setNerve(&NrvMiniMeister::MiniMeisterNrvTalkStart::sInstance);
}

void MiniMeister::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::showModel((LiveActor*)MR::emitEffect(this, "HeadLight"));
}

void MiniMeister::makeActorDead() {
    MR::forceDeleteEffect(this, "HeadLight");
    LiveActor::makeActorDead();
}

void MiniMeister::calcAndSetBaseMtx() {
    MR::calcTalkBaseMtx(&mUnk90);
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

// NONMATCHING: the vector arithmetic is approximated from the draft.
void MiniMeister::calcPositionFromCamera(const TVec3f& rPos) {
    MiniMeister* pActor = this;
    TVec3f root;
    TVec3f center;
    MR::extractMtxTrans(MR::getJointMtx(pActor, "All_Root"), &root);
    MR::extractMtxTrans(MR::getJointMtx(pActor, "Center"), &center);
    TVec3f diff = root - center;
    TVec3f camPos = MR::getCamPos();
    TVec3f toCam = camPos - (diff + rPos);
    TVec3f dir;
    MR::normalizeOrZero(&dir, toCam);
    if (!MR::isSameDirection(dir, pActor->mGravity, 0.01f)) {
        TVec3f negDir;
        negDir.set(-dir.x, -dir.y, -dir.z);
        if (!MR::isSameDirection(negDir, pActor->mGravity, 0.01f)) {
            pActor->mPosition.set(dir);
        }
    }
}

// NONMATCHING: the screen/world math is approximated from the draft.
void MiniMeister::updateTalkScreenPos(f32 ratio) {
    MiniMeister* pActor = this;
    f32 fovy = MR::getFovy();
    f32 halfTan = MR::tan(0.017453292f * (fovy * 0.5f));
    TVec3f worldPos;
    MR::calcWorldPositionFromCenterScreen(&worldPos, pActor->mScreenPos, (875.0f * MR::tan(0.3926991f)) / halfTan);

    TVec3f anchor = *pActor->mOwner->mTalkActor->getPosition();
    TVec3f anchorScreen;
    MR::calcScreenPosition(&anchorScreen, anchor);
    TVec2f anchorScreen2(anchorScreen.x, anchorScreen.y);
    TVec2f worldScreen2(worldPos.x, worldPos.y);

    TVec3f camPos = MR::getCamPos();
    TVec3f d1 = anchorScreen - worldPos;
    f32 distA = PSVECMag(&d1);
    TVec3f d2 = worldPos - camPos;
    f32 distB = PSVECMag(&d2);
    f32 scale = distB / distA;
    f32 blend = scale + (ratio * (1.0f - scale));

    TVec2f sp30 = worldScreen2 * blend;
    TVec2f sp38 = anchorScreen2 * (1.0f - blend);
    TVec2f mix = sp30 + sp38;
    TVec3f dir;
    MR::calcWorldRayDirectionFromScreen(&dir, mix);
    MR::normalize(&dir);
    TVec3f pos = camPos + dir * (distB);
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
        MR::tryRumblePadMiddle(mOwner->mTalkActor, 0);
        MR::stopTalkEffect(mOwner->mTalkActor);
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

