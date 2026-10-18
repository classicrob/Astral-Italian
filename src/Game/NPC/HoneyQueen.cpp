// NONMATCHING: independent init stores and literal aliases.
#include "Game/NPC/HoneyQueen.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/TalkUtil.hpp"

namespace {
    static const f32 sTalkBalloonOffset = 950.0f;
};  // namespace

namespace NrvHoneyQueen {
    NEW_NERVE(HoneyQueenNrvWait, HoneyQueen, Wait);
    NEW_NERVE(HoneyQueenNrvReady, HoneyQueen, Ready);
};  // namespace NrvHoneyQueen

HoneyQueen::HoneyQueen(const char* pName) : NPCActor(pName) {
    mWing = nullptr;
    for (s32 i = 0; i < 10; i++) {
        mCollisionParts[i] = nullptr;
    }
}

void HoneyQueen::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps = "HoneyQueen";
    caps.setDefault();
    caps.mWaitNerve = GET_NERVE(HoneyQueen, HoneyQueenNrvWait);
    caps.mUseShadow = true;
    caps.mMessageOffset.x = 0.0f;
    caps.mMessageOffset.y = ::sTalkBalloonOffset;
    caps.mMessageOffset.z = 0.0f;
    caps.mSensorJoint = "Center";
    caps.mMakeActor = 0;
    caps.mSensorMax = 2;
    caps._95=false;
    MR::initUseStageSwitchWriteA(this, rIter);
    initialize(rIter, caps, nullptr, nullptr, nullptr);

    mCollisionParts[0] =
        MR::createCollisionPartsFromLiveActor(this, "Center", getSensor("Body"), MR::getJointMtx(this, "Center"), MR::CollisionScaleType_Unk2);
    mCollisionParts[1] =
        MR::createCollisionPartsFromLiveActor(this, "CenterFur", getSensor("Body"), MR::getJointMtx(this, "Center"), MR::CollisionScaleType_Unk2);
    mCollisionParts[2] =
        MR::createCollisionPartsFromLiveActor(this, "Face", getSensor("Body"), MR::getJointMtx(this, "Face"), MR::CollisionScaleType_Unk2);
    mCollisionParts[3] =
        MR::createCollisionPartsFromLiveActor(this, "LArm01", getSensor("Body"), MR::getJointMtx(this, "LArm01"), MR::CollisionScaleType_Unk2);
    mCollisionParts[4] =
        MR::createCollisionPartsFromLiveActor(this, "LArm02", getSensor("Body"), MR::getJointMtx(this, "LArm02"), MR::CollisionScaleType_Unk2);
    mCollisionParts[5] =
        MR::createCollisionPartsFromLiveActor(this, "LFoot001", getSensor("Body"), MR::getJointMtx(this, "LFoot001"), MR::CollisionScaleType_Unk2);
    mCollisionParts[6] =
        MR::createCollisionPartsFromLiveActor(this, "RArm01", getSensor("Body"), MR::getJointMtx(this, "RArm01"), MR::CollisionScaleType_Unk2);
    mCollisionParts[7] =
        MR::createCollisionPartsFromLiveActor(this, "RArm02", getSensor("Body"), MR::getJointMtx(this, "RArm02"), MR::CollisionScaleType_Unk2);
    mCollisionParts[8] =
        MR::createCollisionPartsFromLiveActor(this, "RFoot001", getSensor("Body"), MR::getJointMtx(this, "RFoot001"), MR::CollisionScaleType_Unk2);
    mCollisionParts[9] =
        MR::createCollisionPartsFromLiveActor(this, "Tactile", getSensor("Body"), MR::getJointMtx(this, "Tactile"), MR::CollisionScaleType_Unk2);

    MR::excludeCalcShadowToActorAll(this, this);

    mWing = MR::createModelObjNpc("羽", "HoneyQueenWing", MR::getJointMtx(this, "Center"));
    mWing->makeActorAppeared();
    MR::startAction(mWing, "HoneyQueenWing");

    for (s32 i = 0; i < ARRAY_SIZE(mCollisionParts); i++) {
        MR::validateCollisionParts(mCollisionParts[i]);
    }

    mLodCtrl->setDistanceToLow(10000.0f);
    mLodCtrl->setDistanceToMiddle(5000.0f);
    mLodCtrl->setFarClipping(-1.0f);
    mLodCtrl->setClippingTypeSphereContainsModelBoundingBox(100.0f);

    MR::setDistanceToTalk(mMsgCtrl, 0.0f);

    MR::initFur(this);
    MR::initFur(mLodCtrl->mMiddleModel);
    MR::initFur(mLodCtrl->mLowModel);

}

void HoneyQueen::control() {
    NPCActor::control();
    MR::startActionSound(this, "Fly");
}

void HoneyQueen::calcAnim() {
    LiveActor::calcAnim();

    for (s32 i = 0; i < ARRAY_SIZE(mCollisionParts); i++) {
        if(mCollisionParts[i]) mCollisionParts[i]->setMtx();
    }
}

void HoneyQueen::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        if (MR::isEqualSensor(pSender, this, "TouchJump")) {
            MR::sendMsgTouchJump(pReceiver, pSender);
        }
    }
}

bool HoneyQueen::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        return false;
    }

    return NPCActor::receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

void HoneyQueen::exeWait() {
    if (MR::tryTalkNearPlayerAtEnd(mMsgCtrl)) {
        setNerve(GET_NERVE(HoneyQueen, HoneyQueenNrvReady));
    }

    if (MR::isTalkTalking(mMsgCtrl)) {
        MR::tryStartBckAndBtp(this, "Talk", nullptr);
    } else {
        MR::tryStartBckAndBtp(this, "Wait", nullptr);
    }
}

void HoneyQueen::exeReady() {
    if (!MR::isNearPlayer(mMsgCtrl, -1.0f)) {
        setNerve(GET_NERVE(HoneyQueen, HoneyQueenNrvWait));
    }
}

