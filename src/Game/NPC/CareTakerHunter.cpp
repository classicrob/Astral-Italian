#include "Game/NPC/CareTakerHunter.hpp"
#include "Game/NameObj/BringEnemy.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"





namespace NrvCareTakerHunter {
    class CareTakerHunterNrvTakeOutStar : public Nerve { public: virtual void execute(Spine*) const; static CareTakerHunterNrvTakeOutStar sInstance; };
    CareTakerHunterNrvTakeOutStar CareTakerHunterNrvTakeOutStar::sInstance;
    NEW_NERVE(CareTakerHunterNrvReaction, CareTakerHunter, Reaction);
    class CareTakerHunterNrvWaitDefault : public Nerve { public: virtual void execute(Spine*) const; static CareTakerHunterNrvWaitDefault sInstance; };
    CareTakerHunterNrvWaitDefault CareTakerHunterNrvWaitDefault::sInstance;
    NEW_NERVE(CareTakerHunterNrvTalk, CareTakerHunter, Talk);
    NEW_NERVE(CareTakerHunterNrvTalkBegin, CareTakerHunter, TalkBegin);
    NEW_NERVE(CareTakerHunterNrvTalkEnd, CareTakerHunter, TalkEnd);
    NEW_NERVE(CareTakerHunterNrvWait, CareTakerHunter, Wait);
    NEW_NERVE(CareTakerHunterNrvGrab, CareTakerHunter, Grab);
    NEW_NERVE(CareTakerHunterNrvGrabWait, CareTakerHunter, GrabWait);
    NEW_NERVE(CareTakerHunterNrvHide, CareTakerHunter, Hide);
    NEW_NERVE(CareTakerHunterNrvHideWait, CareTakerHunter, HideWait);
    NEW_NERVE(CareTakerHunterNrvAppear, CareTakerHunter, Appear);
    NEW_NERVE(CareTakerHunterNrvAppeared, CareTakerHunter, Appeared);
    NEW_NERVE(CareTakerHunterNrvWipeIn, CareTakerHunter, WipeIn);
    NEW_NERVE(CareTakerHunterNrvLeave, CareTakerHunter, Leave);
    NEW_NERVE(CareTakerHunterNrvEnd, CareTakerHunter, End);
    class CareTakerHunterNrvWaitEnd : public Nerve { public: virtual void execute(Spine*) const; static CareTakerHunterNrvWaitEnd sInstance; };
    CareTakerHunterNrvWaitEnd CareTakerHunterNrvWaitEnd::sInstance;
}

TVec3f sHunterSingleEnemyOffset(60.0f, 250.0f, 0.0f);
TVec3f sHunterPairEnemyOffset(60.0f, 250.0f, 0.0f);
TVec3f sHunterEnemyModelRotation(180.0f, 0.0f, 90.0f);

CareTakerHunter::CareTakerHunter(const char* pName) : NPCActor(pName) {
    mTakeOutStar = NULL;
    mObjArg0 = -1;
    mObjArg4 = 0;
    mBodyColorFrame = 0;
    mCameraInfo0 = NULL;
    mCameraInfo1 = NULL;
    mCameraInfo2 = NULL;
    mCameraInfo3 = NULL;
    mBringEnemy = NULL;
    mChildObjs = NULL;
    mChildCount = 0;
    mActorGroup = NULL;
    mRotAngle = 0.0f;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    mTimer = 0.0f;
}

// NONMATCHING: callback member-pointer copies and local stack allocation differ.
void CareTakerHunter::init(const JMapInfoIter& rIter) {
    const char* pName;
    s32 arg0 = -1;
    MR::getJMapInfoArg0NoInit(rIter, &arg0);
    mObjArg0 = arg0;

    NPCActorCaps caps("CareTakerHunter");
    caps.setDefault();
    caps.mWaitNerve = GET_NERVE(CareTakerHunter, CareTakerHunterNrvWaitDefault);
    caps.mReactionNerve = GET_NERVE(CareTakerHunter, CareTakerHunterNrvReaction);
    initialize(rIter, caps, "CareTakerHunter", NULL, nullptr);

    s32 arg3 = 0;
    MR::getJMapInfoArg3NoInit(rIter, &arg3);
    mBodyColorFrame = arg3;

    MR::startBrk(this, "BodyColor");
    MR::setBrkFrameAndStop(this, (f32)mBodyColorFrame);

    mTrampledAction = "BTrampled";
    mSpinAction = "BSpinHit";
    mReactionAction = "BSpinHit";
    mPointingAction = "BTalkHelp";
    mParam._14 = "BWaitStand";
    mParam._18 = "BWaitStand";
    mParam._0 = 0;
    mParam._1 = 1;
    mParam._8 = 3.0f;
    mRailMoveSpeed = 2.0f;
    mRailMoveAccel = 0.1f;
    mRailTurnRate = 0.05f;
    mFollowRailOnGround = 1;

    MR::startBckNoInterpole(this, "BWaitStand");
    MR::calcAnimDirect(this);

    s32 arg4 = 0;
    MR::getJMapInfoArg4NoInit(rIter, &arg4);
    mObjArg4 = arg4;
    setTalkStrings(mObjArg4);

    if (getMsgCtrl() != NULL) {
        MR::registerBranchFunc(getMsgCtrl(), TalkMessageFunc_Inline(this, &CareTakerHunter::branchFunc));
        MR::registerEventFunc(getMsgCtrl(), TalkMessageFunc_Inline(this, &CareTakerHunter::eventFunc));
    }

    if (mObjArg0 == 0) {
        if (!MR::isScenarioOpenInCurrentStageWithDeclarer("パマタリアンハンター")) {
            makeActorDead();
        }
        MR::initMultiActorCamera(this, rIter, &mCameraInfo0, "敵集め開始");
        MR::initMultiActorCamera(this, rIter, &mCameraInfo1, "敵集め終了");
        MR::initMultiActorCamera(this, rIter, &mCameraInfo2, "敵集めスター登場");
        MR::initMultiActorCamera(this, rIter, &mCameraInfo3, "スター登場");
        MR::declarePowerStar(this);

        TakeOutStar* pTakeOut = new TakeOutStar(this, "TakeOutStarCaretakerHunter", "TakeOutStarCaretakerHunter", GET_NERVE(CareTakerHunter, CareTakerHunterNrvTakeOutStar), nullptr);
        mTakeOutStar = pTakeOut;

        s32 numChild = MR::getChildObjNum(rIter);
        mChildCount = numChild - 1;
        mChildObjs = new LiveActor*[mChildCount];

        s32 idx = 0;
        for (s32 i = 0; i < numChild; i++) {
            pName = NULL;
            MR::getChildObjName(&pName, rIter, i);
            if (MR::isEqualString(pName, "BringEnemy")) {
                NameObj* pObj = NameObjFactory::initChildObj(rIter, i);
                mBringEnemy = static_cast< BringEnemy* >(pObj);
                MR::setPosition(mBringEnemy, mPosition);
                mBringEnemy->makeActorDead();
            } else {
                mChildObjs[idx] = static_cast< LiveActor* >(NameObjFactory::initChildObj(rIter, i));
                mChildObjs[idx]->makeActorDead();
                idx++;
            }
        }

        switch (mBringEnemy->getEnemyType()) {
        case 1: {
            mActorGroup = new LiveActorGroup("デモ用アクター", 1);
            LiveActor* pModel = static_cast< LiveActor* >(MR::createModelObjEnemy("デモ用クリボー", "Kuribo", NULL));
            MR::invalidateClipping(pModel);
            mActorGroup->registerActor(pModel);
            break;
        }
        case 2: {
            mActorGroup = new LiveActorGroup("デモ用アクター", 2);
            LiveActor* pModel1 = static_cast< LiveActor* >(MR::createModelObjEnemy("デモ用ベーゴマン", "BegomanSpike", NULL));
            LiveActor* pModel2 = static_cast< LiveActor* >(MR::createModelObjEnemy("デモ用ベーゴ頭", "BegomanSpikeHead", MR::getJointMtx(pModel1, "Center")));
            mActorGroup->registerActor(pModel1);
            mActorGroup->registerActor(pModel2);
            MR::initLightCtrl(pModel1);
            MR::initLightCtrl(pModel2);
            MR::invalidateClipping(pModel1);
            MR::invalidateClipping(pModel2);
            break;
        }
        }

        MR::setEffectHostMtx(this, "GetLight", mActorGroup->getActor(0)->getBaseMtx());

        for (s32 i = 0; i < mActorGroup->getObjNum(); i++) {
            LiveActor* pActor = mActorGroup->getActor(i);
            MR::registerDemoSimpleCastAll(pActor);
            MR::setScale(mActorGroup->getActor(i), mScale);
            mActorGroup->getActor(i)->makeActorDead();
        }

        setNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvTalk));
    }
}

void CareTakerHunter::control() {
    MR::startActionSound(this, "SmLvCaretakerWait", -1, -1, -1);
    NPCActor::control();
}

bool CareTakerHunter::branchFunc(u32 msg) {
    if (msg == 0) {
        if (mBringEnemy->getNearestEnemy() != NULL) {
            if (isNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvWait))) {
                setNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvGrab));
            }
            return true;
        }
        return false;
    }
    return false;
}

bool CareTakerHunter::eventFunc(u32 msg) {
    if (mObjArg0 == -1) {
        if (msg == 0xA) {
            MR::addPlayerLeft(1);
            return true;
        }
        return true;
    }
    if (msg == 0) {
        return mTakeOutStar->takeOut();
    }
    if (msg == 1) {
        if (isNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvTalk))) {
            setNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvTalkBegin));
            return false;
        }
        return isNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvTalkEnd));
    } else if (msg == 2) {
        return isNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvAppeared));
    }
    return true;
}

bool CareTakerHunter::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        mSpinAttacked = 1;
        return false;
    }
    return NPCActor::receiveMsgPlayerAttack(msg, pSender, pReceiver);
}




// NONMATCHING: floating temporary lifetimes, load scheduling, and register allocation differ.
void CareTakerHunter::updateRotation() {
    if (mActorGroup != nullptr) {
        LiveActor* actor = mActorGroup->getActor(0);
        mRotAngle += mTimer;
        f32 minimum = 0.0f;
        f32 period = 360.0f;
        mRotAngle = minimum + (f32)fmod(period + (mRotAngle - minimum), 360.0);
        TPos3f mtx;
        PSMTXCopy(getBaseMtx(), mtx.toMtxPtr());
        MR::addTransMtxLocal(mtx.toMtxPtr(), mBringEnemy->getEnemyType() == 1 ? sHunterSingleEnemyOffset : sHunterPairEnemyOffset);
        MR::extractMtxTrans(mtx.toMtxPtr(), &actor->mPosition);
        MR::rotateMtxLocalYDegree(mtx.toMtxPtr(), mRotAngle);
        TVec3f direction;
        MR::extractMtxZDir(mtx.toMtxPtr(), &direction);
        mtx.getEuler(actor->mRotation);
        actor->mRotation = TVec3f(57.295780181884766f * actor->mRotation.x,
                                  57.295780181884766f * actor->mRotation.y,
                                  57.295780181884766f * actor->mRotation.z);
    }
}

void CareTakerHunter::setTalkStrings(s32 arg) {
    if (arg == 0) {
        mParam._1C = "BTalkNormal";
        mParam._20 = "BTalkNormal";
    }
}

void CareTakerHunter::exeReaction() {
    if (mTrampledStart != 0) {
        MR::startActionSound(this, "SmNpcTrampledMetal", -1, -1, -1);
        MR::startActionSound(this, "SvCareTakerTrample", -1, -1, -1);
    }
    if (mSpinAttackedStart != 0) {
        MR::startActionSound(this, "SmCaretakerSpinHit", -1, -1, -1);
        MR::startActionSound(this, "SvCareTakerSpinHit", -1, -1, -1);
    }
    if (isPointingSe()) {
        MR::startActionSound(this, "SvCareTakerPointed2p", -1, -1, -1);
        MR::startDPDHitSound();
    }
    if (mReactionStart != 0) {
        MR::startActionSound(this, "SmCaretakerSpinHit", -1, -1, -1);
        MR::startActionSound(this, "SvCareTakerStarPiece", -1, -1, -1);
    }
    if (MR::tryStartReactionAndPopNerve(this)) {
        return;
    }
}

void CareTakerHunter::exeTalk() {
    if (!MR::tryStartReactionAndPushNerve(this, GET_NERVE(CareTakerHunter, CareTakerHunterNrvReaction))) {
        if (MR::isShortTalk(getMsgCtrl())) {
            mParam._1C = "BTalkHelp";
        } else {
            mParam._1C = "BTalkNormal";
        }
        if (MR::tryTalkNearPlayerAndStartMoveTalkAction(this)) {
            mParam._1C = "BTalkNormal";
        }
    }
}

// NONMATCHING: pooled camera string keeps an extra saved register.
void CareTakerHunter::exeTalkBegin() {
    if (MR::isFirstStep(this)) {
        mBringEnemy->appear();
        for (s32 i = 0; i < mChildCount; i++) {
            if (mChildObjs[i] != NULL) {
                mChildObjs[i]->appear();
            }
        }
        MR::startMultiActorCameraTargetSelf(this, mCameraInfo0, "敵集め開始", -1);
    }
    if (MR::isGreaterStep(this, 120)) {
        setNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvTalkEnd));
    }
}

void CareTakerHunter::exeTalkEnd() {
    if (MR::tryTalkForceWithoutDemoAtEnd(getMsgCtrl())) {
        MR::endMultiActorCamera(this, mCameraInfo0, "敵集め開始", false, -1);
        setNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvWait));
    }
}

void CareTakerHunter::exeWait() {
    if (MR::isFirstStep(this)) {
        mParam._8 = 0.0f;
    }
    TVec3f dir(0.0f);
    f32 dist = mBringEnemy->calcNearestEnemy(&dir, this);
    if (!MR::isNearZero(dist, 0.001f)) {
        if (dist < 1500.0f) {
            mParam._8 = 0.0f;
            TVec3f front(0.0f, 0.0f, 1.0f);
            mOrientationQuat.getZDir(front);
            if (!MR::isOppositeDirection(-front, dir, 0.01f)) {
                MR::faceToVector(&mOrientationQuat, dir, 4.0f);
            }
        } else {
            mParam._8 = 4.0f;
        }
    } else {
        mParam._8 = 4.0f;
    }
    MR::tryTalkNearPlayerAtEndAndStartTalkAction(this);
    if (MR::tryStartReactionAndPushNerve(this, GET_NERVE(CareTakerHunter, CareTakerHunterNrvReaction))) {
        mParam._8 = 4.0f;
    }
}

void CareTakerHunter::exeGrab() {
    if (MR::tryStartDemoMarioPuppetable(this, "敵集め成功")) {
        if (MR::isPlayerCarryAny()) {
            LiveActor* carried = MR::getPlayerCarrySensorActor();
            MR::tryPlayerKillTakingActor();
            carried->makeActorDead();
        }
        setNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvGrabWait));
    }
}

void CareTakerHunter::exeGrabWait() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "GetEnemySuccess", -1, -1, -1);
    }
    MR::requestStageBGMVolumeDown(0, 0);
    if (MR::isGreaterStep(this, 90)) {
        setNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvHide));
    }
}

void CareTakerHunter::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::endDemo(this, "敵集め成功");
    }
    MR::requestStageBGMVolumeDown(0, false);
    if (MR::tryStartDemoMarioPuppetable(this, "敵集め終了")) {
        setNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvHideWait));
    }
}

void CareTakerHunter::exeHideWait() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeFade(-1);
        MR::preparePlayerForPositionReset();
    }
    MR::requestStageBGMVolumeDown(0, false);
    if (!MR::isWipeActive()) {
        MR::preparePlayerForPositionReset();
        setNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvAppear));
    }
}

void CareTakerHunter::exeAppear() {
    if (MR::isFirstStep(this)) {
        mBringEnemy->getNearestEnemy()->makeActorDead();
        MR::preparePlayerForPositionReset();
        MR::startSubBGM("BGM_SMG2_JG_PAMATA", false);
        MR::openWipeFade(-1);
        MR::startMultiActorCameraTargetSelf(this, mCameraInfo1, "敵集め終了", -1);
        MR::setPlayerPosOnGroundAndWait("パマタリアンハンターデモ用", true);
        MR::startBckPlayer("Watch", nullptr);
        TVec3f toPlayer(0.0f);
        MR::calcVecToPlayerH(&toPlayer, this, nullptr);
        MR::setNPCActorPose(this, toPlayer, -mGravity, mPosition);
        mActorGroup->appearAll();
        MR::startAction(this, "GetEnemy");
        switch (mBringEnemy->getEnemyType()) {
        case 1:
            for (s32 i = 0; i < mActorGroup->getObjNum(); i++)
                MR::startAction(mActorGroup->getActor(i), "Wait");
            break;
        case 2:
            MR::startAction(mActorGroup->getActor(0), "SignRotate");
            break;
        }
        mTimer = 0.0f;
    }
    mScale = TVec3f(1.0f);
    for (s32 i = 0; i < mActorGroup->getObjNum(); i++)
        MR::setScale(mActorGroup->getActor(i), mScale);
    if (MR::isGreaterStep(this, 120))
        mTimer = -3.0f;
    MR::requestStageBGMVolumeDown(0, false);
    updateRotation();
    if (!MR::isWipeActive() && MR::isActionEnd(this))
        setNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvAppeared));
}

void CareTakerHunter::exeAppeared() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "GetEnemyWait");
    }
    MR::requestStageBGMVolumeDown(2, false);
    updateRotation();
    if (MR::tryTalkForceWithoutDemoAtEnd(getMsgCtrl())) {
        setNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvWipeIn));
    }
}

void CareTakerHunter::exeWipeIn() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeFade(-1);
    }
    MR::requestStageBGMVolumeDown(2, false);
    updateRotation();
    if (!MR::isWipeActive()) {
        setNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvLeave));
    }
}

void CareTakerHunter::exeLeave() {
    if (MR::isFirstStep(this)) {
        MR::openWipeFade(-1);
        MR::startBckPlayer("Watch");
        MR::endMultiActorCamera(this, mCameraInfo1, "敵集め終了", false, -1);
        mActorGroup->makeAllDead();
        MR::startMultiActorCameraTargetSelf(this, mCameraInfo2, "敵集めスター登場", -1);
        MR::startAction(this, "BTalkNormal");
        MR::forceDeleteEffect(this, "GetLight");
    }
    MR::requestStageBGMVolumeDown(2, 0);
    if (!MR::isWipeActive()) {
        setNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvEnd));
    }
}

void CareTakerHunter::exeEnd() {
    MR::requestStageBGMVolumeDown(0, false);
    if (MR::tryTalkForceWithoutDemoAtEnd(getMsgCtrl())) {
        MR::endDemo(this, "敵集め終了");
        setNerve(GET_NERVE(CareTakerHunter, CareTakerHunterNrvWaitEnd));
    }
}

CareTakerHunter::~CareTakerHunter() {}

inline void NrvCareTakerHunter::CareTakerHunterNrvTakeOutStar::execute(Spine* spine) const {
    CareTakerHunter* actor = static_cast<CareTakerHunter*>(spine->mExecutor);
    MR::isFirstStep(actor);
    actor->mTakeOutStar->takeOut();
}
inline void NrvCareTakerHunter::CareTakerHunterNrvWaitDefault::execute(Spine* spine) const {
    CareTakerHunter* actor = static_cast<CareTakerHunter*>(spine->mExecutor);
    MR::tryTalkNearPlayerAndStartMoveTalkAction(actor);
    MR::tryStartReactionAndPushNerve(actor, GET_NERVE(CareTakerHunter, CareTakerHunterNrvReaction));
}
inline void NrvCareTakerHunter::CareTakerHunterNrvWaitEnd::execute(Spine* spine) const {
    CareTakerHunter* actor = static_cast<CareTakerHunter*>(spine->mExecutor);
    MR::requestStageBGMVolumeDown(0, false);
    MR::tryTalkNearPlayerAndStartTalkAction(actor);
    MR::tryStartReactionAndPushNerve(actor, GET_NERVE(CareTakerHunter, CareTakerHunterNrvReaction));
}
