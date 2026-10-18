#pragma auto_inline off
#include "Game/LiveActor/ActorParam.hpp"
#include "Game/Ride/GliBird.hpp"
#include "Game/Camera/CameraTargetMtx.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Map/RaceManager.hpp"
#include "Game/NPC/GliBirdNpc.hpp"
#include "Game/NPC/Pichan.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/GliBirdGuidance.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include <revolution/wpad.h>

namespace {
    static TVec3f sFootOffsetL(-15.0f, 10.0f, 20.0f);
    static TVec3f sFootOffsetR(-15.0f, -10.0f, 20.0f);
};  // namespace

GliBirdFootJointCtrl::GliBirdFootJointCtrl(GliBird* pHost) : LiveActor("足ジョイント制御"), mHost(pHost) {
}

void GliBirdFootJointCtrl::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, -1, 0xB, -1, -1);
    MR::invalidateClipping(this);
    makeActorAppeared();
}

static void calcFootJoint(GliBird* pHost, const char* pJointName, const char* pPlayerJointName, const TVec3f& rOffset, f32 rate, f32 scale) {
    TPos3f playerMtx;
    playerMtx.identity();
    TPos3f jointMtx;
    jointMtx.identity();

    MtxPtr pJointMtx = MR::getJointMtx(pHost, pJointName);
    TVec3f playerPos(0.0f);
    TVec3f birdPos(0.0f);
    MR::calcPlayerJointMtx(&playerMtx, pPlayerJointName);
    playerMtx.getTrans(playerPos);
    playerMtx.mult(rOffset, playerPos);
    jointMtx.set(pJointMtx);
    jointMtx.getTrans(birdPos);

    TVec3f dir(birdPos);
    dir.sub(playerPos);
    MR::normalizeOrZero(&dir);
    TVec3f dirScaled = dir * scale;
    playerPos.add(dirScaled);

    TVec3f birdPart = birdPos * (1.0f - rate);
    TVec3f playerPart = playerPos * rate;
    TVec3f result(playerPart);
    result.add(birdPart);
    jointMtx[0][3] = result.x;
    jointMtx[1][3] = result.y;
    jointMtx[2][3] = result.z;
    PSMTXCopy(jointMtx, pJointMtx);
}

void GliBirdFootJointCtrl::calcAnim() {
    if (mHost->isEnableFootJointCtrl()) {
        calcFootJoint(mHost, "FingerL", "HandL0", sFootOffsetL, 0.5f, 5.0f);
        calcFootJoint(mHost, "FingerR", "HandR0", sFootOffsetR, 0.5f, 5.0f);
        calcFootJoint(mHost, "LegL3", "HandL0", sFootOffsetL, 0.5f, 5.0f);
        calcFootJoint(mHost, "LegR3", "HandR0", sFootOffsetR, 0.5f, 5.0f);
        calcFootJoint(mHost, "LegL2", "HandL0", sFootOffsetL, 0.1f, 5.0f);
        calcFootJoint(mHost, "LegR2", "HandR0", sFootOffsetR, 0.1f, 5.0f);
        calcFootJoint(mHost, "LegL1", "HandL0", sFootOffsetL, 0.05f, 5.0f);
        calcFootJoint(mHost, "LegR1", "HandR0", sFootOffsetR, 0.05f, 5.0f);
    }
}
