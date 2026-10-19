#include "Game/MapObj/DrumTrampoline.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjActor.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace NrvDrumTrampoline {
    class DrumTrampolineNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }

        static DrumTrampolineNrvWait sInstance;
    };
    DrumTrampolineNrvWait DrumTrampolineNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(DrumTrampolineNrvBounce, DrumTrampoline, Bound);
};  // namespace NrvDrumTrampoline

DrumTrampoline::DrumTrampoline(const char* pName) : MapObjActor(pName) {
    mIsBounce = false;
}

void DrumTrampoline::init(const JMapInfoIter& rIter) {
    MapObjActor::init(rIter);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoSimpleMapObj(&info);
    MapObjActorUtil::setupInitInfoTypical(&info, mObjectName);
    MapObjActorUtil::setupInitInfoColorChangeArg0(&info, rIter);
    MapObjActorUtil::setupInitInfoTextureChangeArg1(&info, rIter);
    MapObjActorUtil::setupInitInfoShadowLengthArg2(&info, rIter);
    info.setupNerve(&NrvDrumTrampoline::DrumTrampolineNrvWait::sInstance);
    initialize(rIter, info);
}

void DrumTrampoline::control() {
    TVec3f up;
    MR::calcActorAxisY(&up, this);
    TVec3f diff(*MR::getPlayerPos());
    diff -= mPosition;
    up.dot(diff);
    f32 vDist = MR::clamp(MR::calcDistanceToPlayerVGravity(this) - 1000.0f, 0.0f, 100000.0f);
    f32 hDist = MR::calcDistanceToPlayerH(this);
    if (mIsBounce) {
        if (hDist <= 700.0f * mScale.x && vDist <= 1700.0f) {
            s32 pitch = (s32)MR::clamp(1000.0f * (1.0f - vDist / 1700.0f), 0.0f, 1000.0f);
            s32 vol = (s32)MR::clamp(1000.0f * (1.0f / mScale.x) - 80.0f, 0.0f, 1000.0f);
            MR::startActionSound(this, "JumpLv", pitch, vol, -1);
        } else if (hDist > 700.0f) {
            mIsBounce = false;
        }
    }
    MapObjActor::control();
}

void DrumTrampoline::exeBound() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Bound");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvDrumTrampoline::DrumTrampolineNrvWait::sInstance);
    }
}

bool DrumTrampoline::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xD5) {
        if (isNerve(&NrvDrumTrampoline::DrumTrampolineNrvBounce::sInstance) && MR::isGreaterStep(this, 3)) {
            return true;
        }
        return false;
    }
    if (isNerve(&NrvDrumTrampoline::DrumTrampolineNrvBounce::sInstance)) {
        return false;
    }
    if (MR::isMsgFloorTouch(msg) || MR::isMsgPlayerHipDropFloor(msg)) {
        MR::tryStartAllAnim(this, "Move");
        if (MR::isPlayerHipDropLand()) {
            MR::startActionSound(this, "JumpHDrop", (s32)(100.0f * (1.0f / mScale.x)), -1, -1);
        } else {
            MR::startActionSound(this, "Jump", (s32)(100.0f * (1.0f / mScale.x)), -1, -1);
        }
        mIsBounce = true;
        setNerve(&NrvDrumTrampoline::DrumTrampolineNrvBounce::sInstance);
        return true;
    }
    return false;
}

bool DrumTrampoline::tryGetGroundNormal(TVec3f* pNormal, LiveActor* pActor, HitSensor* pSensor) {
    if (MR::sendMsgToGroundSensor(0xD5, pSensor)) {
        pNormal->set(MR::getGroundNormal(pActor));
        return true;
    }
    return false;
}

DrumTrampoline::~DrumTrampoline() {
}
