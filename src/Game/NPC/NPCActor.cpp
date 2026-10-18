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
            _10 = speed;
            _14 = magnitude;
            _20 = damping;
        };
    };

    static NPCParam sParam(15.0f, 1.4f, 0.3f);
    static NPCParam sParam2(10.0f, 1.1f, 0.05f);
};  // namespace

NPCActorCaps::NPCActorCaps(const char* pName) {
    _0 = pName;
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
    _67 = false;
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
    _F = false;
    mMessageOffset.x = 0.0f;
    mMessageOffset.y = 150.0f;
    mMessageOffset.z = 0.0f;
    mSensorOffset.set< f32 >(0.0f, 50.0f, 0.0f);
    _10 = pName;
    mTalkMtx = nullptr;
    mTalkJointName = nullptr;
    mObjectName = pName;
    mSensorJoint = nullptr;
    mSensorMax = 1;
    mWaitNerve = GET_NERVE(NPCActor, NPCActorNrvWait);
    mTalkNerve = GET_NERVE(NPCActor, NPCActorNrvTalk);
    mReactionNerve = GET_NERVE(NPCActor, NPCActorNrvReaction);
    _70 = nullptr;
    _7C = false;
    _6C = nullptr;
    _94 = false;
    _95 = false;
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
    _95 = true;
}

void NPCActorCaps::setIndirect() {
    mSceneConnectionType = 2;
}

NPCActor::NPCActor(const char* pName) : LiveActor(pName) {
    mLodCtrl = nullptr;
    mMsgCtrl = nullptr;
    _94 = nullptr;
    _98 = nullptr;
    mLodCtrl = nullptr;
    mMsgCtrl = nullptr;
    _94 = nullptr;
    _98 = nullptr;
    _9C = 0;
    _D8 = 0;
    _D9 = 0;
    _DA = 0;
    _DB = 0;
    _DC = 0;
    _DD = 0;
    _DE = 0;
    _DF = 0;
    _E0 = 0;
    _E1 = 0;
    _E2 = 0;
    _E3 = 0;
    _E4 = 0;
    _E5 = 0;
    _E6 = 0;
    _E7 = 0;
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
    _11C = nullptr;
    _120 = nullptr;
    _10C = 2.0f;
    _110 = 0.1f;
    _114 = 0.08f;
    _118 = 1.0f;
    _124 = 0;
    _128 = 1;
    _12C = 0.0f;
    _130 = 0;
    _134 = 0;
    _138 = 0;
    _13C = 0;
    mScaleController = nullptr;
    mDelegator = nullptr;
    mYoshiTarget = nullptr;
    mCurNerve = nullptr;
    _158 = 0x400;
    _A0.set< f32 >(0.0f, 0.0f, 0.0f, 1.0f);
    _B0.set< f32 >(0.0f, 0.0f, 0.0f, 1.0f);
    _C0.zero();
    _CC.zero();
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
    MR::makeQuatRotateRadian(&_A0, eulerXYZ);
    _A0.normalize();

    mPosition.set(trans);
    eulerXYZ = eulerXYZ * 57.29578f;
    mRotation.set(eulerXYZ);
    _CC.set(eulerXYZ);

    MR::setBaseTRMtx(this, _A0);
    MR::resetPosition(this);
}

void NPCActor::setBaseMtx(MtxPtr pMtx) {
    TPos3f pos;
    pos.setInline(pMtx);
    setBaseMtx(pos);
}

void NPCActor::setInitPose() {
    _B0.x = _A0.x;
    _B0.y = _A0.y;
    _B0.z = _A0.z;
    _B0.w = _A0.w;

    _C0.set(mPosition);
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

        MR::makeQuatRotateDegree(&_A0, mRotation);
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
            if (rCaps._6C != nullptr) {
                MR::initStarPointerTargetAtJoint(this, rCaps._6C, rCaps.mPointerSize, rCaps.mStarPointerOffs);
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

    if (rCaps._95) {
        if (MR::isValidStarPointerTarget(this)) {
            TVec3f offset(rCaps.mStarPointerOffs);
            MtxPtr targetMtx = getBaseMtx();
            if (mStarPointerTarget != nullptr) {
                offset = MR::getStarPointerTargetOffset(this);
                targetMtx = MR::getStarPointerTargetMtx(this);
            }
            if (rCaps._6C != nullptr) {
                mYoshiTarget = MR::createYoshiLockOnTarget(this, rCaps._6C, offset);
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

        if (rCaps._F) {
            initTalkCtrlDirect(rIter, rCaps._10, rCaps.mMessageOffset, talkMtx);
        } else {
            initTalkCtrl(rIter, rCaps._10, rCaps.mMessageOffset, talkMtx);
        }
    }

    if (rCaps.mLodCtrl) {
        mLodCtrl = MR::createLodCtrlNPC(this, rIter);
    }

    if (rCaps._70 != nullptr) {
        mScaleController = new AnimScaleController(rCaps._7C ? &::sParam2 : &::sParam);
        mDelegator = MR::createJointDelegatorWithNullChildFunc(this, &calcJointScale, rCaps._70);
    }

    if (rCaps._94) {
        _E7 = 1;
    }

    if (rCaps._67) {
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
        _94 = MR::createIndirectNPCGoods(this, rItem.mGoods0, rItem.mGoodsJoint0);
        _98 = MR::createIndirectNPCGoods(this, rItem.mGoods1, rItem.mGoodsJoint1);
    } else {
        _94 = MR::createNPCGoods(this, rItem.mGoods0, rItem.mGoodsJoint0);
        _98 = MR::createNPCGoods(this, rItem.mGoods1, rItem.mGoodsJoint1);
    }

    if (_94 != nullptr) {
        MR::registerDemoSimpleCastAll(_94);
    }

    if (_98 != nullptr) {
        MR::registerDemoSimpleCastAll(_98);
    }
}

void NPCActor::calcAndSetBaseMtx() {
    _A0.normalize();
    bool sameRotation = false;
    if (JGeometry::TUtil<f32>::epsilonEquals(_CC.x, mRotation.x, 3.8146973e-06f) &&
        JGeometry::TUtil<f32>::epsilonEquals(_CC.y, mRotation.y, 3.8146973e-06f) &&
        JGeometry::TUtil<f32>::epsilonEquals(_CC.z, mRotation.z, 3.8146973e-06f)) {
        sameRotation = true;
    }
    if (!sameRotation) {
        MR::makeQuatRotateDegree(&_A0, mRotation);
    } else {
        _A0.getEuler(mRotation);
        mRotation = mRotation * 57.29578f;
    }

    _CC = mRotation;
    MR::setBaseTRMtx(this, _A0);
    if (mDelegator != nullptr) {
        mDelegator->registerCallBack();
    }
}

void NPCActor::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    if (mLodCtrl != nullptr) {
        mLodCtrl->appear();
    }

    if (_94 != nullptr) {
        _94->appear();
    }

    if (_98 != nullptr) {
        _98->appear();
    }
}

void NPCActor::makeActorDead() {
    if (mLodCtrl != nullptr) {
        mLodCtrl->kill();
    }

    if (_94 != nullptr) {
        _94->kill();
    }

    if (_98 != nullptr) {
        _98->kill();
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
        _158 = 0;
    } else {
        _158++;
    }

    const s32 count = _158;
    _158 = count < 0x400 ? count : 0x400;

    updateReaction();

    if (_9C > 0) {
        _9C--;
    }

    if (mLodCtrl != nullptr) {
        mLodCtrl->update();
    }

    if (mScaleController != nullptr) {
        mScaleController->update();
    }

    if (mModelManager != nullptr && MR::checkPlayerSwingTrigger() && MR::isNearPlayer(this, _12C)) {
        _E3 = true;
    }

    if (mStarPointerTarget != nullptr && MR::isStarPointerPointing2PAttachTarget(this)) {
        _E4 = true;
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
        _E5 = 1;
        return true;
    }

    return false;
}

bool NPCActor::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgRushEnd(msg)) {
        _E5 = true;
        return true;
    }
    if (MR::isMsgYoshiStartTongue(msg)) {
        MR::tryRumblePadWeak(this, 0);
        MR::stopSceneForDefaultHit(4);
        TVec3f position;
        mStarPointerTarget->calcPosition(&position);
        MR::emitEffectHit(this, position, "YoshiTongueHit");
        MR::startActionSoundPlayer("TongueHitNpc", -1, -1);
        _E3 = true;
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
    MR::preScaleMtx(pPos->toMtxPtr(), mScaleController->_C);
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
    _A0.getZDir(zDir);

    if (MR::isOppositeDirection(-zDir, toPlayer)) {
        return true;
    }

    return MR::faceToVector(&_A0, toPlayer, f1);
}

bool NPCActor::turnToPlayer(f32 f1, f32 f2, f32 f3) {
    bool turned = turnToPlayer(f1);
    if (f3 == 0.0f) {
        return turned;
    }

    TVec3f toPlayer;
    TVec3f yDir;
    _B0.getYDir(yDir);
    ::calcVecToPlayer(&toPlayer, this);

    if (MR::isSameDirection(toPlayer, yDir)) {
        return false;
    }

    TVec3f vec1;
    TVec3f vec2;
    MR::makeAxisFrontUp(&vec1, &vec2, toPlayer, yDir);
    MR::clampVecAngleDeg(&vec2, yDir, f3);
    return MR::turnQuatYDirRad(&_A0, _A0, vec2, f2 * 0.017453292f) & turned;
}

bool NPCActor::turnToDefault(f32 f1) {
    TVec3f zDir;
    TVec3f zDir2;
    TVec3f yDir;
    _B0.getZDir(zDir);
    _B0.getYDir(yDir);
    _A0.getZDir(zDir2);

    if (MR::isOppositeDirection(-zDir, zDir2)) {
        return true;
    }

    f32 dot = zDir.dot(zDir2);
    dot = MR::clamp(dot, -1.0f, 1.0f);

    f32 flt = MR::abs((f1 * 0.017453292f) / MR::acos(dot));
    flt = MR::clamp(flt, 0.0f, 1.0f);

    MR::blendQuatUpFront(&_A0, yDir, zDir, flt, flt);
    return MR::isSameDirection(zDir2, zDir);
}

void NPCActor::setToDefault() {
    mPosition = _C0;
    _A0 = _B0;
    _A0.normalize();

    TVec3f euler;
    _A0.getEuler(euler);
    euler = euler * 57.29578f;
    mRotation = euler;
    _CC = euler;

    MR::setBaseTRMtx(this, _A0);
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
        if (_9C == 0) {
            _9C = 30;
            _E2 = true;
            return true;
        }

        return false;
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        _E2 = true;
        MR::sendMsgAwayJump(pSender, pReceiver);
        return true;
    }

    if (MR::isMsgSupportTicoSpin(msg)) {
        if (_E7) {
            MR::emitItemHit(this);
        }
        _E3 = true;
        return false;
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && MR::isNearZero(_12C)) {
        _E3 = true;
        return false;
    }

    if (MR::isMsgStarPieceReflect(msg)) {
        _E5 = true;
        return true;
    }

    if (MR::isMsgFireBallAttack(msg)) {
        _E5 = true;
        return true;
    }

    if (MR::isMsgYoshiProjectileOrEat(msg)) {
        _E5 = true;
        return true;
    }

    if (MR::isMsgJetTurtleAttack(msg)) {
        _E5 = true;
        _E6 = true;
        return true;
    }

    if (MR::isMsgRockAttack(msg)) {
        _E5 = true;
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

    bool isDefaultScale = (MR::isNearZero(1.0f - pScaleController->_C.x, 0.2f) && MR::isNearZero(1.0f - pScaleController->_C.y, 0.2f) &&
                           MR::isNearZero(1.0f - pScaleController->_C.z, 0.2f));
    if (isDefaultScale) {
        return false;
    }

    return true;
}

bool NPCActor::isPointingSe() const {
    if (_DA && _158 >= 60) {
        return true;
    }

    return false;
}

void NPCActor::updateReaction() {
    _D8 = 0;
    _D9 = 0;
    _DA = 0;
    _DB = 0;
    _DC = 0;

    if (!_DD && _E2) {
        _D8 = true;
    } else if (!_E0 && _E5) {
        _DB = true;
        _DC = !_E1 && _E6;
    } else if (!_DE && _E3) {
        _D9 = true;
    } else if (!_DF && _E4) {
        _DA = true;
    }

    _DD = _E2;
    _DE = _E3;
    _DF = _E4;
    _E0 = _E5;
    _E1 = _E6;
    _E2 = false;
    _E3 = false;
    _E4 = false;
    _E5 = false;
    _E6 = false;
}

void NPCActor::updateScaleCtrl() {
    if (_128) {
        if (_D9 && MR::isNullOrEmptyString(_130)) {
            mScaleController->startHitReaction();
        }

        if (_D8 && MR::isNullOrEmptyString(_134)) {
            mScaleController->startHitReaction();
        }

        if (_DB && MR::isNullOrEmptyString(_13C)) {
            mScaleController->startHitReaction();
        }

        if (!_DF && _E4) {
            if (MR::isNullOrEmptyString(_138)) {
                mScaleController->startDpdHitVibration();
            }
        } else if (_DF && !_E4 && MR::isNullOrEmptyString(_138)) {
            mScaleController->startAnim();
        }
    } else {
        if (_D9 || _D8 || _DB) {
            mScaleController->startHitReaction();
        }

        if (!_DF && _E4) {
            mScaleController->startDpdHitVibration();
        } else if (_DF && !_E4) {
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
