#include "Game/Map/HitInfo.hpp"
#include "Game/Screen/StarPointerTarget.hpp"
#include "Game/Util/TriangleFilter.hpp"
#include "Game/Util/YoshiUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#pragma auto_inline off

#include "Game/NPC/NPCActor.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/NameObj/NameObjArchiveListCollector.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/NPCUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JGeometry/TUtil.hpp>
#include <JSystem/JMath/JMATrigonometric.hpp>

#include "Game/LiveActor/ActorItemGenerator.hpp"

namespace NrvNPCActor {
    NEW_NERVE(NPCActorNrvReaction, NPCActor, Reaction);
    NEW_NERVE(NPCActorNrvWait, NPCActor, Wait);
    NEW_NERVE(NPCActorNrvTalk, NPCActor, Talk);
    NEW_NERVE(NPCActorNrvNull, NPCActor, Null);
};  // namespace NrvNPCActor

namespace {
    class NPCParam : public AnimScaleParam {
    public:
        NPCParam(f32 speed, f32 magnitude, f32 damping) : AnimScaleParam() {
            mDpdVibrationTime = speed;
            mDpdVibrationBase = magnitude;
            mHitReactionVelocityY = damping;
        };
    };

    static NPCParam sParam(15.0f, 1.4f, 0.3f);
    static NPCParam sParam2(10.0f, 1.1f, 0.05f);
};  // namespace

NPCActorCaps::NPCActorCaps(const char* pName) {
    mName = pName;
    mMakeActor = false;
    mHostIO = false;
    mInterpole = false;
    mConnectTo = false;
    mLightCtrl = false;
    mEffect = false;
    mSound = false;
    mAttribute = false;
    mPosition = false;
    mLodCtrl = false;
    mNerve = false;
    mNerveStep = 0;
    mSensor = false;
    mBinder = false;
    mShadow = false;
    mUseShadow = false;
    mRailRider = false;
    mSwitchDead = true;
    mSwitchAppear = false;
    mStartDead = false;
    mBinderSize = 50.0f;
    mSensorSize = 50.0f;
    mShadowSize = 50.0f;
    mPointerSize = 80.0f;
    mSoundName = nullptr;
    mSoundSize = 4;
    mSceneConnectionType = 0;
    mPointer = false;
    mStarPointerOffs.x = 0.0f;
    mStarPointerOffs.y = 0.0f;
    mStarPointerOffs.z = 0.0f;
    mModel = false;
    mMessage = false;
    mUseDirectMessage = false;
    mMessageOffset.x = 0.0f;
    mMessageOffset.y = 150.0f;
    mMessageOffset.z = 0.0f;
    mSensorOffset.set< f32 >(0.0f, 50.0f, 0.0f);
    mMessageId = pName;
    mTalkMtx = nullptr;
    mTalkJointName = nullptr;
    mObjectName = pName;
    mSensorJoint = nullptr;
    mSensorMax = 1;
    mWaitNerve = GET_NERVE(NPCActor, NPCActorNrvWait);
    mTalkNerve = GET_NERVE(NPCActor, NPCActorNrvTalk);
    mReactionNerve = GET_NERVE(NPCActor, NPCActorNrvReaction);
    mScaleJointName = nullptr;
    mUseAlternateScaleParam = false;
    mStarPointerJointName = nullptr;
    mDropItemOnSupportTicoSpin = false;
    mYoshiInteraction = false;
}

void NPCActorCaps::setDefault() {
    mMakeActor = true;
    mHostIO = true;
    mInterpole = true;
    mConnectTo = true;
    mLightCtrl = true;
    mEffect = true;
    mSound = true;
    mAttribute = true;
    mPosition = true;
    mLodCtrl = true;
    mNerve = true;
    mSensor = true;
    mShadow = true;
    mRailRider = true;
    mSwitchDead = true;
    mSwitchAppear = true;
    mPointer = true;
    mModel = true;
    mMessage = true;
    mYoshiInteraction = true;
}

void NPCActorCaps::setIndirect() {
    mSceneConnectionType = 2;
}

NPCActor::NPCActor(const char* pName) : LiveActor(pName) {
    mLodCtrl = nullptr;
    mMsgCtrl = nullptr;
    mGoods0 = nullptr;
    mGoods1 = nullptr;
    mLodCtrl = nullptr;
    mMsgCtrl = nullptr;
    mGoods0 = nullptr;
    mGoods1 = nullptr;
    mTrampleCooldown = 0;
    mTrampledStart = 0;
    mSpinAttackedStart = 0;
    mPointingStart = 0;
    mReactionStart = 0;
    mJetTurtleHitStart = 0;
    mWasTrampled = 0;
    mWasSpinAttacked = 0;
    mWasPointed = 0;
    mWasReacting = 0;
    mWasHitByJetTurtle = 0;
    mTrampled = 0;
    mSpinAttacked = 0;
    mPointed = 0;
    mReacting = 0;
    mHitByJetTurtle = 0;
    mDropItemOnSupportTicoSpin = 0;
    mParam._0 = 1;
    mParam._1 = 1;
    mParam._4 = 2000.0f;
    mParam._8 = 4.0f;
    mParam._C = 0.0f;
    mParam._10 = 0.0f;
    mParam._14 = nullptr;
    mParam._18 = nullptr;
    mParam._1C = nullptr;
    mParam._20 = nullptr;
    mRailMoveAction = nullptr;
    mRailTalkAction = nullptr;
    mRailMoveSpeed = 2.0f;
    mRailMoveAccel = 0.1f;
    mRailTurnRate = 0.08f;
    mRailTalkAnimRate = 1.0f;
    mFollowRailOnGround = 0;
    mUseReactionActions = 1;
    mSpinAttackRange = 0.0f;
    mSpinAction = 0;
    mTrampledAction = 0;
    mPointingAction = 0;
    mReactionAction = 0;
    mScaleController = nullptr;
    mDelegator = nullptr;
    mYoshiTarget = nullptr;
    mCurNerve = nullptr;
    mFramesSincePointingSound = 0x400;
    mOrientationQuat.set< f32 >(0.0f, 0.0f, 0.0f, 1.0f);
    mInitOrientationQuat.set< f32 >(0.0f, 0.0f, 0.0f, 1.0f);
    mInitPosition.zero();
    mPreviousRotation.zero();
    mWaitNerve = GET_NERVE(NPCActor, NPCActorNrvWait);
    mTalkNerve = GET_NERVE(NPCActor, NPCActorNrvTalk);
    mReactionNerve = GET_NERVE(NPCActor, NPCActorNrvReaction);
}

void NPCActor::makeArchiveList(NameObjArchiveListCollector* pCollector, const JMapInfoIter& rIter) {
    const char* name;
    MR::getObjectName(&name, rIter);
    NPCActorItem item(name);

    s32 item_type = -1;
    MR::getJMapInfoArg7NoInit(rIter, &item_type);

    if (MR::getNPCItemData(&item, item_type)) {
        NPCActor::addArchive(pCollector, item);
    }
}

void NPCActor::addArchive(NameObjArchiveListCollector* pCollector, const NPCActorItem& rItem) {
    if (!MR::isNullOrEmptyString(rItem.mGoods0) && MR::isNPCItemFileExist(rItem.mGoods0)) {
        pCollector->addArchive(rItem.mGoods0);
    }

    if (!MR::isNullOrEmptyString(rItem.mGoods1) && MR::isNPCItemFileExist(rItem.mGoods1)) {
        pCollector->addArchive(rItem.mGoods1);
    }
}

void NPCActor::makeArchiveListDefault(NameObjArchiveListCollector* pCollector, const JMapInfoIter& rIter) {
    const char* objName;
    MR::getObjectName(&objName, rIter);
    NPCActorItem item(objName);

    if (MR::getNPCItemData(&item, 0)) {
        NPCActor::addArchive(pCollector, item);
    }
}

void NPCActor::setBaseMtx(const TPos3f& rPos) {
    TVec3f trans;
    rPos.getTrans(trans);

    TVec3f eulerXYZ;
    rPos.getEulerXYZ(eulerXYZ);
    MR::makeQuatRotateRadian(&mOrientationQuat, eulerXYZ);
    mOrientationQuat.normalize();

    mPosition.set(trans);
    eulerXYZ = eulerXYZ * 57.29578f;
    mRotation.set(eulerXYZ);
    mPreviousRotation.set(eulerXYZ);

    MR::setBaseTRMtx(this, mOrientationQuat);
    MR::resetPosition(this);
}

void NPCActor::setBaseMtx(MtxPtr pMtx) {
    TPos3f pos;
    pos.setInline(pMtx);
    setBaseMtx(pos);
}

void NPCActor::setInitPose() {
    mInitOrientationQuat.x = mOrientationQuat.x;
    mInitOrientationQuat.y = mOrientationQuat.y;
    mInitOrientationQuat.z = mOrientationQuat.z;
    mInitOrientationQuat.w = mOrientationQuat.w;

    mInitPosition.set(mPosition);
}

void NPCActor::init(const JMapInfoIter& rIter) {
    LiveActor::init(rIter);
}

void NPCActor::initAfterPlacement() {
    MR::calcGravity(this);

    if (!MR::isNullOrEmptyString(mParam._14)) {
        MR::startAction(this, mParam._14);

        if (MR::isExistBck(this, mParam._14)) {
            MR::startBckNoInterpole(this, mParam._14);
        }

        MR::setBckFrameAtRandom(this);
        MR::calcAnimDirect(this);
    }
}

void NPCActor::initialize(const JMapInfoIter& rIter, const NPCActorCaps& rCaps, const char* pArcName, const char* pAnimName, const char* pAnimArcName) {
    if (pArcName != nullptr) {
        if (pAnimArcName != nullptr) {
            MR::initActor(this, rIter, pArcName, pAnimArcName, pAnimName, false);
        } else if (pAnimName != nullptr) {
            MR::initActor(this, rIter, pArcName, pAnimName, false);
        } else {
            MR::initActor(this, rIter, pArcName, false);
        }

        MR::makeQuatRotateDegree(&mOrientationQuat, mRotation);
        setInitPose();

        if (MR::isExistRail(this)) {
            MR::moveCoordToStartPos(this);

            if (MR::isExistShadows(this)) {
                MR::onCalcShadow(this, nullptr);
            }
        }
    } else {
        if (rCaps.mPosition) {
            MR::initDefaultPosAndQuat(this, rIter);
        }

        if (rCaps.mModel) {
            MR::initActor(this, rIter, rCaps.mObjectName, false);
        }

        if (rCaps.mConnectTo) {
            if (rCaps.mSceneConnectionType == 0) {
                MR::connectToSceneNpc(this);
            } else if (rCaps.mSceneConnectionType == 1) {
                MR::connectToSceneNpcMovement(this);
            } else if (rCaps.mSceneConnectionType == 2) {
                MR::connectToSceneIndirectNpc(this);
            } else if (rCaps.mSceneConnectionType == 3) {
                MR::connectToSceneRide(this);
            } else if (rCaps.mSceneConnectionType == 4) {
                MR::connectToSceneSuperDreamer(this);
            }
        }

        if (rCaps.mLightCtrl) {
            MR::initLightCtrl(this);
        }

        if (rCaps.mSound) {
            initSound(rCaps.mSoundSize, rCaps.mSoundName, nullptr, TVec3f(0.0f));
        }

        if (rCaps.mAttribute) {
            MR::addToAttributeGroupSearchTurtle(this);
        }

        if (rCaps.mSensor) {
            initHitSensor(rCaps.mSensorMax);
            if (rCaps.mSensorJoint != nullptr) {
                MR::addHitSensorAtJointNpc(this, "Body", rCaps.mSensorJoint, 8, rCaps.mSensorSize, rCaps.mSensorOffset);
            } else {
                MR::addHitSensorNpc(this, "Body", 8, rCaps.mSensorSize, rCaps.mSensorOffset);
            }
        }

        if (rCaps.mBinder) {
            initBinder(rCaps.mBinderSize, rCaps.mBinderSize, 0);
            MR::onCalcGravity(this);
        }

        if (rCaps.mEffect) {
            initEffectKeeper(0, nullptr, false);
        }

        if (rCaps.mUseShadow) {
            MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
        } else if (rCaps.mShadow) {
            MR::initShadowVolumeSphere(this, rCaps.mShadowSize);
            MR::onCalcShadowOneTime(this, nullptr);
        }

        if (rCaps.mRailRider && MR::isConnectedWithRail(rIter)) {
            initRailRider(rIter);
            MR::moveCoordToStartPos(this);
            if (MR::isExistShadows(this)) {
                MR::onCalcShadow(this, nullptr);
            }
        }

        if (rCaps.mPointer) {
            if (rCaps.mStarPointerJointName != nullptr) {
                MR::initStarPointerTargetAtJoint(this, rCaps.mStarPointerJointName, rCaps.mPointerSize, rCaps.mStarPointerOffs);
            } else {
                MR::initStarPointerTarget(this, rCaps.mPointerSize, rCaps.mStarPointerOffs);
            }
        }
    }

    MR::useStageSwitchAwake(this, rIter);

    if (rCaps.mSwitchDead) {
        MR::initUseStageSwitchWriteDead(this, rIter);
    }

    if (rCaps.mNerve) {
        mWaitNerve = rCaps.mWaitNerve;
        mTalkNerve = rCaps.mTalkNerve;
        mReactionNerve = rCaps.mReactionNerve;
        if (mWaitNerve != nullptr) {
            initNerve(mWaitNerve, rCaps.mNerveStep);
        }
    }

    if (rCaps.mYoshiInteraction) {
        if (MR::isValidStarPointerTarget(this)) {
            TVec3f offset(rCaps.mStarPointerOffs);
            MtxPtr targetMtx = getBaseMtx();
            if (mStarPointerTarget != nullptr) {
                offset = MR::getStarPointerTargetOffset(this);
                targetMtx = MR::getStarPointerTargetMtx(this);
            }
            if (rCaps.mStarPointerJointName != nullptr) {
                mYoshiTarget = MR::createYoshiLockOnTarget(this, rCaps.mStarPointerJointName, offset);
            } else {
                mYoshiTarget = MR::createYoshiLockOnTarget(this, targetMtx, offset);
            }
        }
        MR::addEffect(this, "YoshiTongueHit");
    }

    if (rCaps.mMessage) {
        MtxPtr talkMtx;
        if (rCaps.mTalkJointName != nullptr) {
            talkMtx = MR::getJointMtx(this, rCaps.mTalkJointName);
        } else {
            talkMtx = rCaps.mTalkMtx;
        }

        if (rCaps.mUseDirectMessage) {
            initTalkCtrlDirect(rIter, rCaps.mMessageId, rCaps.mMessageOffset, talkMtx);
        } else {
            initTalkCtrl(rIter, rCaps.mMessageId, rCaps.mMessageOffset, talkMtx);
        }
    }

    if (rCaps.mLodCtrl) {
        mLodCtrl = MR::createLodCtrlNPC(this, rIter);
    }

    if (rCaps.mScaleJointName != nullptr) {
        mScaleController = new AnimScaleController(rCaps.mUseAlternateScaleParam ? &::sParam2 : &::sParam);
        mDelegator = MR::createJointDelegatorWithNullChildFunc(this, &calcJointScale, rCaps.mScaleJointName);
    }

    if (rCaps.mDropItemOnSupportTicoSpin) {
        mDropItemOnSupportTicoSpin = 1;
    }

    if (rCaps.mStartDead) {
        makeActorDead();
    } else if (rCaps.mSwitchAppear && MR::useStageSwitchSyncAppear(this, rIter)) {
        if (rCaps.mMakeActor) {
            makeActorDead();
        }
    } else if (rCaps.mMakeActor) {
        makeActorAppeared();
    }

    MR::registerMarioFaceShipNpc(this, rIter);
    if (MR::isExistBinder(this)) {
        MR::setBindTriangleFilter(this, MR::createTriangleFilterDelegator(this, &NPCActor::isSensorSpinCloudBlock));
    }
}

void NPCActor::equipment(const NPCActorItem& rItem, bool indirect) {
    if (indirect) {
        mGoods0 = MR::createIndirectNPCGoods(this, rItem.mGoods0, rItem.mGoodsJoint0);
        mGoods1 = MR::createIndirectNPCGoods(this, rItem.mGoods1, rItem.mGoodsJoint1);
    } else {
        mGoods0 = MR::createNPCGoods(this, rItem.mGoods0, rItem.mGoodsJoint0);
        mGoods1 = MR::createNPCGoods(this, rItem.mGoods1, rItem.mGoodsJoint1);
    }

    if (mGoods0 != nullptr) {
        MR::registerDemoSimpleCastAll(mGoods0);
    }

    if (mGoods1 != nullptr) {
        MR::registerDemoSimpleCastAll(mGoods1);
    }
}

void NPCActor::calcAndSetBaseMtx() {
    mOrientationQuat.normalize();
    bool sameRotation = false;
    if (JGeometry::TUtil<f32>::epsilonEquals(mPreviousRotation.x, mRotation.x, 3.8146973e-06f) &&
        JGeometry::TUtil<f32>::epsilonEquals(mPreviousRotation.y, mRotation.y, 3.8146973e-06f) &&
        JGeometry::TUtil<f32>::epsilonEquals(mPreviousRotation.z, mRotation.z, 3.8146973e-06f)) {
        sameRotation = true;
    }
    if (!sameRotation) {
        MR::makeQuatRotateDegree(&mOrientationQuat, mRotation);
    } else {
        mOrientationQuat.getEuler(mRotation);
        mRotation = mRotation * 57.29578f;
    }

    mPreviousRotation = mRotation;
    MR::setBaseTRMtx(this, mOrientationQuat);
    if (mDelegator != nullptr) {
        mDelegator->registerCallBack();
    }
}

void NPCActor::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    if (mLodCtrl != nullptr) {
        mLodCtrl->appear();
    }

    if (mGoods0 != nullptr) {
        mGoods0->appear();
    }

    if (mGoods1 != nullptr) {
        mGoods1->appear();
    }
}

void NPCActor::makeActorDead() {
    if (mLodCtrl != nullptr) {
        mLodCtrl->kill();
    }

    if (mGoods0 != nullptr) {
        mGoods0->kill();
    }

    if (mGoods1 != nullptr) {
        mGoods1->kill();
    }

    LiveActor::makeActorDead();
}

void NPCActor::kill() {
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }

    LiveActor::kill();
}

void NPCActor::control() {
    if (isPointingSe()) {
        mFramesSincePointingSound = 0;
    } else {
        mFramesSincePointingSound++;
    }

    const s32 count = mFramesSincePointingSound;
    mFramesSincePointingSound = count < 0x400 ? count : 0x400;

    updateReaction();

    if (mTrampleCooldown > 0) {
        mTrampleCooldown--;
    }

    if (mLodCtrl != nullptr) {
        mLodCtrl->update();
    }

    if (mScaleController != nullptr) {
        mScaleController->update();
    }

    if (mModelManager != nullptr && MR::checkPlayerSwingTrigger() && MR::isNearPlayer(this, mSpinAttackRange)) {
        mSpinAttacked = true;
    }

    if (mStarPointerTarget != nullptr && MR::isStarPointerPointing2PAttachTarget(this)) {
        mPointed = true;
    }

    if (mScaleController != nullptr && mDelegator != nullptr) {
        updateScaleCtrl();
    }
    if (mYoshiTarget != nullptr && isReactionNerve()) {
        YoshiFunction::tryYoshiEat(mYoshiTarget);
    }
}

bool NPCActor::receiveMsgEnemyAttack(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgExplosionAttack(msg)) {
        mReacting = 1;
        return true;
    }

    return false;
}

bool NPCActor::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgRushEnd(msg)) {
        mReacting = true;
        return true;
    }
    if (MR::isMsgYoshiStartTongue(msg)) {
        MR::tryRumblePadWeak(this, 0);
        MR::stopSceneForDefaultHit(4);
        TVec3f position;
        mStarPointerTarget->calcPosition(&position);
        MR::emitEffectHit(this, position, "YoshiTongueHit");
        MR::startActionSoundPlayer("TongueHitNpc", -1, -1);
        mSpinAttacked = true;
        return false;
    }
    return false;
}

bool NPCActor::initTalkCtrl(const JMapInfoIter& rIter, const char* pChar, const TVec3f& rVec, MtxPtr pMtx) {
    mMsgCtrl = MR::createTalkCtrl(this, rIter, pChar, rVec, pMtx);

    if (mMsgCtrl == nullptr) {
        return false;
    }

    MR::onRootNodeAutomatic(mMsgCtrl);
    return true;
}

bool NPCActor::initTalkCtrlDirect(const JMapInfoIter& rIter, const char* pChar, const TVec3f& rVec, MtxPtr pMtx) {
    mMsgCtrl = MR::createTalkCtrlDirect(this, rIter, pChar, rVec, pMtx);

    if (mMsgCtrl == nullptr) {
        return false;
    }

    MR::onRootNodeAutomatic(mMsgCtrl);
    return true;
}

bool NPCActor::calcJointScale(TPos3f* pPos, const JointControllerInfo&) {
    MR::preScaleMtx(pPos->toMtxPtr(), mScaleController->mScale);
    return true;
}

bool NPCActor::turnToPlayer() {
    return turnToPlayer(5.0f);
}

namespace {
    void calcVecToPlayer(TVec3f* pOut, const LiveActor* pActor) {
        TVec3f playerTrans;
        MR::extractMtxTrans(MR::getPlayerBaseMtx(), &playerTrans);
        MR::normalizeOrZero(pOut, playerTrans - pActor->mPosition);
    }
};  // namespace

// NONMATCHING: player-joint vector temporaries use different stack slots.
bool NPCActor::turnToPlayer(f32 f1) {
    TPos3f playerJoint;
    TVec3f zDir;
    TVec3f toPlayer;
    if (mMsgCtrl != nullptr && MR::isTalkTalking(mMsgCtrl)) {
        TVec3f position;
        TVec3f front;
        MR::calcPlayerJointMtx(&playerJoint, "Face0");
        MR::extractMtxTrans(playerJoint.toMtxPtr(), &position);
        MR::extractMtxZDir(playerJoint.toMtxPtr(), &front);
        MR::normalizeOrZero(&toPlayer, position - front * 10.0f - mPosition);
    } else {
        ::calcVecToPlayer(&toPlayer, this);
    }
    mOrientationQuat.getZDir(zDir);

    if (MR::isOppositeDirection(-zDir, toPlayer)) {
        return true;
    }

    return MR::faceToVector(&mOrientationQuat, toPlayer, f1);
}

bool NPCActor::turnToPlayer(f32 f1, f32 f2, f32 f3) {
    bool turned = turnToPlayer(f1);
    if (f3 == 0.0f) {
        return turned;
    }

    TVec3f toPlayer;
    TVec3f yDir;
    mInitOrientationQuat.getYDir(yDir);
    ::calcVecToPlayer(&toPlayer, this);

    if (MR::isSameDirection(toPlayer, yDir)) {
        return false;
    }

    TVec3f vec1;
    TVec3f vec2;
    MR::makeAxisFrontUp(&vec1, &vec2, toPlayer, yDir);
    MR::clampVecAngleDeg(&vec2, yDir, f3);
    return MR::turnQuatYDirRad(&mOrientationQuat, mOrientationQuat, vec2, f2 * 0.017453292f) & turned;
}

bool NPCActor::turnToDefault(f32 f1) {
    TVec3f zDir;
    TVec3f zDir2;
    TVec3f yDir;
    mInitOrientationQuat.getZDir(zDir);
    mInitOrientationQuat.getYDir(yDir);
    mOrientationQuat.getZDir(zDir2);

    if (MR::isOppositeDirection(-zDir, zDir2)) {
        return true;
    }

    f32 dot = zDir.dot(zDir2);
    dot = MR::clamp(dot, -1.0f, 1.0f);

    f32 flt = MR::abs((f1 * 0.017453292f) / MR::acos(dot));
    flt = MR::clamp(flt, 0.0f, 1.0f);

    MR::blendQuatUpFront(&mOrientationQuat, yDir, zDir, flt, flt);
    return MR::isSameDirection(zDir2, zDir);
}

void NPCActor::setToDefault() {
    mPosition = mInitPosition;
    mOrientationQuat = mInitOrientationQuat;
    mOrientationQuat.normalize();

    TVec3f euler;
    mOrientationQuat.getEuler(euler);
    euler = euler * 57.29578f;
    mRotation = euler;
    mPreviousRotation = euler;

    MR::setBaseTRMtx(this, mOrientationQuat);
    MR::resetPosition(this);
}

void NPCActor::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrYoshiOrRide(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
    } else if (MR::isSensorEnemy(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool NPCActor::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerTrample(msg)) {
        if (mTrampleCooldown == 0) {
            mTrampleCooldown = 30;
            mTrampled = true;
            return true;
        }

        return false;
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        mTrampled = true;
        MR::sendMsgAwayJump(pSender, pReceiver);
        return true;
    }

    if (MR::isMsgSupportTicoSpin(msg)) {
        if (mDropItemOnSupportTicoSpin) {
            MR::emitItemHit(this);
        }
        mSpinAttacked = true;
        return false;
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && MR::isNearZero(mSpinAttackRange)) {
        mSpinAttacked = true;
        return false;
    }

    if (MR::isMsgStarPieceReflect(msg)) {
        mReacting = true;
        return true;
    }

    if (MR::isMsgFireBallAttack(msg)) {
        mReacting = true;
        return true;
    }

    if (MR::isMsgYoshiProjectileOrEat(msg)) {
        mReacting = true;
        return true;
    }

    if (MR::isMsgJetTurtleAttack(msg)) {
        mReacting = true;
        mHitByJetTurtle = true;
        return true;
    }

    if (MR::isMsgRockAttack(msg)) {
        mReacting = true;
        return true;
    }

    return false;
}

void NPCActor::pushNerve(const Nerve* pNerve) {
    mCurNerve = mSpine->getCurrentNerve();
    setNerve(pNerve);
}

const Nerve* NPCActor::popAndPushNerve(const Nerve* pNerve) {
    const Nerve* poppedNerve = popNerve();
    pushNerve(pNerve);
    return poppedNerve;
}

const Nerve* NPCActor::popNerve() {
    const Nerve* currentNerve = mSpine->getCurrentNerve();
    setNerve(mCurNerve);
    mCurNerve = nullptr;
    return currentNerve;
}

bool NPCActor::tryPushNullNerve() {
    if (isEmptyNerve()) {
        pushNerve(GET_NERVE(NPCActor, NPCActorNrvNull));
        return true;
    }

    return false;
}

bool NPCActor::isReactionNerve() const {
    return !isNerve(mReactionNerve);
}

bool NPCActor::isEmptyNerve() const {
    return mCurNerve == nullptr;
}

bool NPCActor::isScaleAnim() const {
    AnimScaleController* pScaleController = mScaleController;
    if (mScaleController == nullptr) {
        return false;
    }

    bool isDefaultScale = (MR::isNearZero(1.0f - pScaleController->mScale.x, 0.2f) && MR::isNearZero(1.0f - pScaleController->mScale.y, 0.2f) &&
                           MR::isNearZero(1.0f - pScaleController->mScale.z, 0.2f));
    if (isDefaultScale) {
        return false;
    }

    return true;
}

bool NPCActor::isPointingSe() const {
    if (mPointingStart && mFramesSincePointingSound >= 60) {
        return true;
    }

    return false;
}

void NPCActor::updateReaction() {
    mTrampledStart = 0;
    mSpinAttackedStart = 0;
    mPointingStart = 0;
    mReactionStart = 0;
    mJetTurtleHitStart = 0;

    if (!mWasTrampled && mTrampled) {
        mTrampledStart = true;
    } else if (!mWasReacting && mReacting) {
        mReactionStart = true;
        mJetTurtleHitStart = !mWasHitByJetTurtle && mHitByJetTurtle;
    } else if (!mWasSpinAttacked && mSpinAttacked) {
        mSpinAttackedStart = true;
    } else if (!mWasPointed && mPointed) {
        mPointingStart = true;
    }

    mWasTrampled = mTrampled;
    mWasSpinAttacked = mSpinAttacked;
    mWasPointed = mPointed;
    mWasReacting = mReacting;
    mWasHitByJetTurtle = mHitByJetTurtle;
    mTrampled = false;
    mSpinAttacked = false;
    mPointed = false;
    mReacting = false;
    mHitByJetTurtle = false;
}

void NPCActor::updateScaleCtrl() {
    if (mUseReactionActions) {
        if (mSpinAttackedStart && MR::isNullOrEmptyString(mSpinAction)) {
            mScaleController->startHitReaction();
        }

        if (mTrampledStart && MR::isNullOrEmptyString(mTrampledAction)) {
            mScaleController->startHitReaction();
        }

        if (mReactionStart && MR::isNullOrEmptyString(mReactionAction)) {
            mScaleController->startHitReaction();
        }

        if (!mWasPointed && mPointed) {
            if (MR::isNullOrEmptyString(mPointingAction)) {
                mScaleController->startDpdHitVibration();
            }
        } else if (mWasPointed && !mPointed && MR::isNullOrEmptyString(mPointingAction)) {
            mScaleController->startAnim();
        }
    } else {
        if (mSpinAttackedStart || mTrampledStart || mReactionStart) {
            mScaleController->startHitReaction();
        }

        if (!mWasPointed && mPointed) {
            mScaleController->startDpdHitVibration();
        } else if (mWasPointed && !mPointed) {
            mScaleController->startAnim();
        }
    }
}

void NPCActor::exeReaction() {
    MR::tryStartReactionAndPopNerve(this);
}

void NPCActor::exeWait() {
    if (!MR::tryStartReactionAndPushNerve(this, mReactionNerve)) {
        if (mMsgCtrl != nullptr) {
            if (MR::tryTalkNearPlayerAndStartTalkAction(this)) {
                setNerve(mTalkNerve);
            }
        } else {
            MR::tryStartTurnAction(this);
        }
    }
}

void NPCActor::exeTalk() {
    if (!MR::tryStartReactionAndPushNerve(this, mReactionNerve) && (MR::tryTalkNearPlayerAtEndAndStartMoveTalkAction(this))) {
        setNerve(mWaitNerve);
    }
}

void NPCActor::exeNull() {
}

void NPCActor_FORCE_MATCH(TQuat4f* pDest, const TQuat4f& rSrc) {
    *pDest = rSrc;
}

bool NPCActor::isSensorSpinCloudBlock(const Triangle* triangle) {
    return MR::isSensorType(triangle->getSensor(), 0x56);
}

bool NPCActor::tryPullNullNerve() {
    if (!isNerve(mReactionNerve)) return false;
    popNerve();
    return true;
}
