// NONMATCHING: target data-symbol boundaries differ for matching literal offsets.
#include "Game/NPC/HoneyBee.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/NameObj/NameObjArchiveListCollector.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/NPCUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/TalkUtil.hpp"

extern const char cNPCTalkDefault[], cNPCTalkTurnDefault[];
extern const char lbl_807CFE90[4], lbl_807CFE94[4], lbl_807CFE98[4], lbl_807CFE9C[4];

namespace {
    static const s32 sFlyLectureWaitStep = 120;
    static const s32 sHipLectureWaitStep = 120;
    static const f32 sFlyMoveSpeed = 5.0f;
    // static const f32 sMessageOffsetY = _;
};  // namespace

namespace NrvHoneyBee {
    NEW_NERVE(HoneyBeeNrvWait, HoneyBee, Wait);
    NEW_NERVE(HoneyBeeNrvReaction, HoneyBee, Reaction);
    NEW_NERVE(HoneyBeeNrvFly, HoneyBee, Fly);
    NEW_NERVE(HoneyBeeNrvJumpLecture, HoneyBee, JumpLecture);
};  // namespace NrvHoneyBee

HoneyBee::HoneyBee(const char* pName) : NPCActor(pName), mCurrentRailPoint(-1), mArg0(), mFloatOffset() {
    mSmoothedCenterMtx.identity();
    mCenterPosition.zero();
}

void HoneyBee::makeArchiveList(NameObjArchiveListCollector* pCollector, const JMapInfoIter& rIter) {
    const char* objName;
    MR::getObjectName(&objName, rIter);
    NPCActorItem npcItem = objName;

    s32 arg0 = -1;
    MR::getJMapInfoArg0NoInit(rIter, &arg0);

    if (MR::getNPCItemData(&npcItem, arg0)) {
        NPCActor::addArchive(pCollector, npcItem);
    }
}

void HoneyBee::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps("HoneyBee");
    caps.setDefault();
    caps._70="Center";
    caps.mWaitNerve=GET_NERVE(HoneyBee,HoneyBeeNrvWait);
    caps.mReactionNerve=GET_NERVE(HoneyBee,HoneyBeeNrvReaction);
    s32 action=0;
    MR::getJMapInfoArg0NoInit(rIter,&action);
    mArg0=action;
    mParam._14=cNPCWaitDefault; mParam._18=cNPCTurnDefault;
    mParam._1C=cNPCTalkDefault; mParam._20=cNPCTalkTurnDefault;
    _130=cNPCSpinDefault; _134=cNPCTrampledDefault;
    _138=cNPCPointingDefault; _13C=cNPCReactionDefault;
    _128=false;
    _12C=450.0f;
    switch(mArg0) {
    case 0: case 6: case 7: case 8: case 9: _128=true; break;
    case 1: caps.mWaitNerve=GET_NERVE(HoneyBee,HoneyBeeNrvJumpLecture); break;
    case 2: caps.mWaitNerve=GET_NERVE(HoneyBee,HoneyBeeNrvFly); break;
    case 3: mParam.setNoTurnAction("SleepWait"); break;
    case 4: mParam.setNoTurnAction("StickWait"); break;
    case 5: mParam.setSingleAction("GatekeeperWait"); _128=true; break;
    }
    initialize(rIter,caps,"HoneyBee",nullptr,nullptr);
    NPCActorItem item;
    item.mActor="HoneyBee"; item.mGoods0=lbl_807CFE9C; item.mGoodsJoint0=lbl_807CFE98;
    item.mGoods1=lbl_807CFE94; item.mGoodsJoint1=lbl_807CFE90;
    MR::getNPCItemData(&item,mArg0);
    equipment(item,false);
    MR::initFur(this);
    MR::initFur(mLodCtrl->mMiddleModel);
    MR::initFur(mLodCtrl->mLowModel);
}

void HoneyBee::calcAndSetBaseMtx() {
    MR::calcAndSetFloatBaseMtx(this, mFloatOffset);
}

void HoneyBee::control() {
    if (_D8) {
        MR::startActionSound(this, "SmNpcTrampled");
        MR::startActionSound(this, "SvHoneybeeTrampled");
    }

    if (NPCActor::isPointingSe()) {
        MR::startDPDHitSound();
        MR::startActionSound(this, "SvHoneybeePoint");
    }

    if (_D9) {
        MR::startActionSound(this, "SvHoneybeeSpin");
    }

    if (_DB) {
        MR::startActionSound(this, "SvHoneybeeStarPieceHit");
    }

    NPCActor::control();
    f32 a = mSmoothedCenterMtx[1][3] - 80.0f;
    mSmoothedCenterMtx.set(MR::getJointMtx(this, "Center"));
    MR::extractMtxTrans(mSmoothedCenterMtx, &mCenterPosition);
    mSmoothedCenterMtx[1][3] = 80.0f + (((9.0f * a) + mSmoothedCenterMtx[1][3]) / 10.0f);

    if (mArg0 != InitAction_Sleep && mArg0 != InitAction_Stick) {
        mFloatOffset = MR::calcFloatOffset(this, mFloatOffset, 120.0f);
    }
}

void HoneyBee::exeWait() {
    if (!MR::tryStartReactionAndPushNerve(this, GET_NERVE(HoneyBee, HoneyBeeNrvReaction))) {
        MR::tryTalkNearPlayerAndStartMoveTalkAction(this);
    }
}

void HoneyBee::exeReaction() {
    MR::tryStartReactionAndPopNerve(this);
}

void HoneyBee::exeFly() {
    if (MR::isFirstStep(this)) {
        MR::onCalcShadow(this, nullptr);
    }

    MR::startActionSound(this, "SmLvHoneybeeFly");

    if (mMsgCtrl != nullptr) {
        MR::tryTalkNearPlayer(mMsgCtrl);
    }

    MR::moveCoordAndFollowTrans(this, ::sFlyMoveSpeed);

    if (mCurrentRailPoint != MR::getCurrentRailPointNo(this)) {
        s32 arg0 = 0;
        mCurrentRailPoint = MR::getCurrentRailPointNo(this);
        MR::getRailPointArg0WithInit(this, mCurrentRailPoint, &arg0);

        if (arg0 == 0) {
            if (!MR::isBckPlaying(this, "WalkWait")) {
                MR::startAction(this, "Wait");
            }
        } else {
            if (!MR::isBckPlaying(this, "FlyWait")) {
                MR::startAction(this, "FlyWait");
            }
        }
    }
}



void HoneyBee::exeJumpLecture() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    if (MR::isBckPlaying(this, "Wait")) {
        if (mMsgCtrl && MR::tryTalkNearPlayer(mMsgCtrl)) {
            NPCActor::turnToPlayer();
        }

        if (MR::isGreaterStep(this, ::sFlyLectureWaitStep)) {
            MR::invalidateClipping(this);
            MR::startAction(this, "FlyLectureWait");
        }
    } else if (MR::isBckLooped(this)) {
        MR::validateClipping(this);
        setNerve(GET_NERVE(HoneyBee, HoneyBeeNrvJumpLecture));
    }
}




