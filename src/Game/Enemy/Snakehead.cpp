#include "Game/Enemy/Snakehead.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/StringUtil.hpp"

class ScoreAttackFunction {
public:
    static void score300Points(const LiveActor*);
};

namespace {
    struct SnakeheadData {
        /* 0x00 */ const s32* mWaitFrames;
        /* 0x04 */ const s32* mRestFrames;
        /* 0x08 */ const f32* mDefeatReturnSpeed;
        /* 0x0C */ const char* mAppearBck;
        /* 0x10 */ const char* mWaitAction;
        /* 0x14 */ const char* mForwardAction;
        /* 0x18 */ const char* mForwardSmokeEffect;
        /* 0x1C */ const char* mBackAction;
    };

    static const f32 sDefaultForwardSpeed = 15.0f;
    static const f32 sDefaultBackSpeed = 10.0f;
    static const f32 sGoHomeSpeedNormal = 20.0f;
    static const f32 sGoHomeSpeedRace = 50.0f;
    static const s32 sStepForWaitBig = 30;
    static const s32 sStepForRestBig = 100;
    static const s32 sStepForWaitSmall = 60;
    static const s32 sStepForRestSmall = 120;
    static const s32 sStepForWaitBigRace = 30;
    static const s32 sStepForRestBigRace = 100;
    static const s32 sStepForWaitSmallRace = 60;
    static const s32 sStepForRestSmallRace = 120;
    static const f32 sBodySensorRadiusBig = 0.0f;
    static const f32 sBodySensorRadiusSmall = 0.0f;
    static const f32 sDefaultMoveStartRadius = 0.0f;
    static const f32 sStartCoord = 300.0f;
    // static const f32 sStepForShockStay = 0.0f;
    // static const f32 sCoordToGoalForSlowMove = 0.0f;

    static const SnakeheadData sSnakeheadDataTable[] = {
        {&::sStepForWaitBig, &::sStepForRestBig, &::sGoHomeSpeedNormal, "StraightAppear", "StraightWait", "StraightForward", "StraightForwardSmoke",
         "StraightBack"},
        {&::sStepForWaitSmall, &::sStepForRestSmall, &::sGoHomeSpeedNormal, "StraightAppear", "Wait", "StraightForward", nullptr, "StraightBack"},
        {&::sStepForWaitBigRace, &::sStepForRestBigRace, &::sGoHomeSpeedRace, nullptr, "Wait", "Forward", "ForwardSmoke", "Back"},
        {&::sStepForWaitSmallRace, &::sStepForRestSmallRace, &::sGoHomeSpeedRace, nullptr, "Wait", "Forward", nullptr, "Back"}};

    const SnakeheadData* getParam(s32 index) {
        return &::sSnakeheadDataTable[index];
    }
};  // namespace

namespace NrvSnakehead {
    NEW_NERVE(SnakeheadNrvWaylay, Snakehead, Waylay);
    NEW_NERVE(SnakeheadNrvWait, Snakehead, Wait);
    NEW_NERVE(SnakeheadNrvMoveForward, Snakehead, MoveForward);
    NEW_NERVE(SnakeheadNrvRest, Snakehead, Rest);
    NEW_NERVE(SnakeheadNrvMoveBack, Snakehead, MoveBack);
    NEW_NERVE(SnakeheadNrvTurtleDown, Snakehead, TurtleDown);
    class SnakeheadNrvDPDSwoon : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            Snakehead* actor = reinterpret_cast< Snakehead* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(actor, GET_NERVE(Snakehead, SnakeheadNrvMoveBack));
        };
        static SnakeheadNrvDPDSwoon sInstance;
    };
    SnakeheadNrvDPDSwoon SnakeheadNrvDPDSwoon::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvSnakehead

Snakehead::Snakehead(const char* pName)
    : LiveActor(pName), mController(), mStarPointer(), mEffectPos(0.0f, 0.0f, 0.0f), mForwardSpeed(::sDefaultForwardSpeed),
      mBackwardSpeed(::sDefaultBackSpeed), mActivationRadius(1000.0f), mRailCenter(0.0f, 0.0f, 0.0f), mType(), mItem(), mIsValidateShadow(true) {
    mBodyMtx.identity();
}

// NONMATCHING: 97.3%; this/rIter and string-base register allocation swapped (r30/r31/r28 vs r31/r28/r29), and the smoke-name getParam reload (lwz r4,0x18(r4)) is CSE'd away in ours
void Snakehead::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);

    const char* objectName;
    MR::getObjectName(&objectName, rIter);
    MR::initActor(this, rIter, objectName, false);

    MR::getJMapInfoArg0NoInit(rIter, &mForwardSpeed);
    MR::getJMapInfoArg1NoInit(rIter, &mBackwardSpeed);
    MR::getJMapInfoArg2NoInit(rIter, &mActivationRadius);

    s32 arg3 = 0;
    MR::getJMapInfoArg3NoInit(rIter, &arg3);

    mType = static_cast< SnakeheadType >(MR::isEqualString(objectName, "SnakeheadSmall") + arg3 * 2);

    bool arg4 = false;
    MR::getJMapInfoArg4NoInit(rIter, &arg4);
    if (arg4) {
        mIsValidateShadow = false;
    }

    MR::connectToSceneEnemy(this);

    if (isSmall()) {
        initHitSensor(3);
        MR::addHitSensorAtJointEnemy(this, "upper_jaw", "UpperJaw", 8, 150.0f, TVec3f(50.0f, 0.0f, 0.0f));
        MR::addHitSensorAtJointEnemy(this, "head", "Head", 8, 100.0f, TVec3f(140.0f, -70.0f, 0.0f));
        MR::addHitSensorAtJointEnemy(this, "Body", "Body03", 8, 300.0f, TVec3f(0.0f, 0.0f, 0.0f));
    } else {
        initHitSensor(3);
        MR::addHitSensorAtJointEnemy(this, "upper_jaw", "UpperJaw", 8, 250.0f, TVec3f(100.0f, 0.0f, 0.0f));
        MR::addHitSensorAtJointEnemy(this, "lower_jaw", "LowerJaw", 8, 200.0f, TVec3f(250.0f, 0.0f, 0.0f));
        MR::addHitSensorAtJointEnemy(this, "Body", "Body03", 8, 600.0f, TVec3f(0.0f, 0.0f, 0.0f));
    }

    initEffectKeeper(1, nullptr, false);

    if (::getParam(mType)->mForwardSmokeEffect != nullptr) {
        mEffectPos = mPosition;
        MR::setEffectHostSRT(this, ::getParam(mType)->mForwardSmokeEffect, &mEffectPos, nullptr, nullptr);
    }

    initSound(4, "Snakehead", nullptr, TVec3f(0.0f));

    if (isSmall()) {
        MR::initStarPointerTarget(this, 250.0f, TVec3f(0.0f, 0.0f, 150.0f));
    } else {
        MR::initStarPointerTarget(this, 500.0f, TVec3f(0.0f, 0.0f, 200.0f));
    }

    mController = new AnimScaleController(nullptr);
    mController->setParamTight();

    mStarPointer = new WalkerStateBindStarPointer(this, mController);

    mBodyMtx.set(MR::getJointMtx(this, "Body04"));

    if (MR::isEqualString(objectName, "Snakehead")) {
        MR::initShadowControllerList(this, 2);
        MR::addShadowVolumeFlatModel(this, "Head", "SnakeheadShadow", mBodyMtx);
        MR::addShadowVolumeBox(this, "Body", TVec3f(480.0f, 300.0f, 680.0f));
    } else {
        MR::initShadowVolumeFlatModel(this, "SnakeheadSmallShadow", mBodyMtx);
        MR::setShadowDropLength(this, "SnakeheadSmallShadow", 500.0f);
    }

    if (arg4) {
        MR::invalidateShadowAll(this);
    }

    MR::initLightCtrl(this);

    initRailRider(rIter);
    MR::initAndSetRailClipping(&mRailCenter, this, 100.0f, 500.0f);
    MR::setGroupClipping(this, rIter, 16);
    MR::setRailCoord(this, ::sStartCoord);
    MR::moveTransToCurrentRailPos(this);

    MR::addToAttributeGroupSearchTurtle(this);

    if (isSmall()) {
        MR::declareStarPiece(this, 9);
    } else {
        mItem = MR::createKinokoOneUp();
    }

    initNerve(GET_NERVE(Snakehead, SnakeheadNrvWait), 1);
    MR::initActorState(this, mStarPointer, GET_NERVE(Snakehead, SnakeheadNrvDPDSwoon), "SupportFreeze");

    if (isMove()) {
        setNerve(GET_NERVE(Snakehead, SnakeheadNrvWait));
    } else {
        setNerve(GET_NERVE(Snakehead, SnakeheadNrvWaylay));

        MR::hideModel(this);

        MR::invalidateShadow(this, nullptr);
    }

    makeActorAppeared();
}

void Snakehead::initAfterPlacement() {
    TVec3f headPos;
    MR::copyJointPos(this, "Head", &headPos);

    headPos -= MR::getRailPointPosStart(this);
    headPos += (MR::getRailPointPosEnd(this));

    TVec3f v7;
    v7.scaleAdd(-50.0f, mGravity, headPos);
    MR::getFirstPolyOnLineToMapExceptSensor(&mEffectPos, nullptr, v7, mGravity * 1000.0f, getSensor("Body"));

    mBodyMtx.set(MR::getJointMtx(this, "Body04"));
}

void Snakehead::kill() {
    MR::startActionSound(this, "EmExplodeUnderWater");

    MR::emitEffect(this, "Death");

    if (isSmall()) {
        TVec3f headPos;
        MR::copyJointPos(this, "Head", &headPos);
        MR::appearStarPiece(this, headPos, 9, 10.0f, 40.0f, 0);
    } else {
        TVec3f attPos;
        MR::copyJointPos(this, "Antenna", &attPos);

        TPos3f itemMtx;
        itemMtx.set(getBaseMtx());
        itemMtx.setTrans(attPos);
        MR::appearKinokoOneUpPop(reinterpret_cast< BenefitItemObj* >(mItem), itemMtx);
    }

    LiveActor::kill();
}

void Snakehead::control() {
    mController->updateNerve();

    tryDPDSwoon();

    TVec3f jointPos;
    MR::copyJointPos(this, "Body04", &jointPos);
    mBodyMtx.setTrans(jointPos);

    if (!isSmall()) {
        TVec3f body01Pos;
        MR::copyJointPos(this, "Body01", &body01Pos);

        TVec3f v6;
        f32 dist = jointPos.distance(body01Pos);

        v6.add(jointPos, body01Pos);
        v6.scale(0.5f);
        MR::setShadowDropPosition(this, "Body", v6);
        MR::setShadowVolumeBoxSize(this, "Body", TVec3f(480.0f, 300.0f, dist));
    }
}

void Snakehead::calcAndSetBaseMtx() {
    TVec3f endPntDir;
    MR::calcRailEndPointDirection(&endPntDir, this);

    TPos3f frontUp;
    MR::makeMtxFrontUpPos(&frontUp, endPntDir, -mGravity, mPosition);
    MR::setBaseTRMtx(this, frontUp);

    MR::updateBaseScale(this, mController);
}

void Snakehead::exeWaylay() {
    if (MR::isFirstStep(this)) {
        choiceAndStartBck("Waylay");
        MR::setBckFrameAndStop(this, 0.0f);

        mBodyMtx.set(MR::getJointMtx(this, "Body04"));
    }

    if (isNearPlayerFromRail() && MR::getBckFrame(this) == 0.0f) {
        choiceAndStartBck("Waylay");
        MR::setBckFrame(this, 1.0f);

        MR::showModel(this);
    }

    if (MR::getBckFrame(this) == 2.0f && mIsValidateShadow) {
        MR::validateShadow(this, nullptr);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Snakehead, SnakeheadNrvMoveForward));
    }
}

void Snakehead::exeWait() {
    if (MR::isFirstStep(this)) {
        choiceAndStartBck("Wait");
    }

    if (MR::isGreaterEqualStep(this, *::getParam(mType)->mWaitFrames) && (isMove() || isNearPlayerFromRail())) {
        setNerve(GET_NERVE(Snakehead, SnakeheadNrvMoveForward));
    }
}

void Snakehead::exeMoveForward() {
    if (MR::isFirstStep(this)) {
        choiceAndStartBck("Forward");

        MR::setRailDirectionToEnd(this);

        MR::startActionSound(this, "EmSnakeheadForward");
    }

    if (!isMove() && MR::isRailReachedNearGoal(this, ::sStartCoord)) {
        MR::moveCoordAndFollowTrans(this, mForwardSpeed / 1.5f);
    } else {
        MR::moveCoordAndFollowTrans(this, mForwardSpeed);
    }

    if (MR::isRailReachedGoal(this)) {
        setNerve(GET_NERVE(Snakehead, SnakeheadNrvRest));
    }
}

void Snakehead::exeRest() {
    if (MR::isStep(this, *::getParam(mType)->mRestFrames)) {
        setNerve(GET_NERVE(Snakehead, SnakeheadNrvMoveBack));
    }
}

void Snakehead::exeMoveBack() {
    if (MR::isFirstStep(this)) {
        choiceAndStartBck("Back");

        MR::setRailDirectionToStart(this);

        MR::startActionSound(this, "EmSnakeheadBackward");
    }

    MR::moveCoordAndFollowTrans(this, mBackwardSpeed);

    if (MR::isRailReachedNearGoal(this, ::sStartCoord)) {
        MR::setRailCoord(this, ::sStartCoord);
        MR::moveTransToCurrentRailPos(this);

        setNerve(GET_NERVE(Snakehead, SnakeheadNrvWait));
    }
}

void Snakehead::exeTurtleDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Damage");

        MR::setRailDirectionToStart(this);

        MR::startActionSound(this, "EvSnakeheadDamage");
    }

    f32 coord = *::getParam(mType)->mDefeatReturnSpeed;
    if (MR::isRailReachedNearGoal(this, ::sStartCoord + coord)) {
        MR::setRailCoord(this, ::sStartCoord);
        MR::moveTransToCurrentRailPos(this);
    } else {
        MR::moveCoordAndFollowTrans(this, coord);
    }

    if (MR::isActionEnd(this)) {
        ScoreAttackFunction::score300Points(this);
        kill();
    }
}

void Snakehead::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(Snakehead, SnakeheadNrvTurtleDown))) {
        return;
    }

    if (!MR::isSensorEnemy(pSender)) {
        return;
    }

    if (!MR::isSensorPlayerOrRide(pReceiver) && !MR::isSensorEnemy(pReceiver) && !MR::isSensorYoshi(pReceiver)) {
        return;
    }

    if (pSender != getSensor("Body")) {
        if (isNerve(GET_NERVE(Snakehead, SnakeheadNrvDPDSwoon)) || !MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
            MR::sendMsgPush(pReceiver, pSender);
        }

        return;
    }

    if (pSender != getSensor("Body")) {
        return;
    }

    TVec3f vec40;
    MR::calcPerpendicFootToLineInside(&vec40, *MR::getSensorPos(pReceiver), MR::getRailPointPosStart(this), MR::getRailPointPosEnd(this));

    TVec3f vec4C;
    vec4C.sub(*MR::getSensorPos(pReceiver), vec40);

    f32 pushRadius = isSmall() ? 100.0f : ::sStartCoord;
    if (vec4C.length() > pushRadius) {
        return;
    }

    if (!isNerve(GET_NERVE(Snakehead, SnakeheadNrvDPDSwoon)) && MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
        return;
    }

    TVec3f pos((*MR::getSensorPos(this, "Body")));
    f32 radius = MR::getSensorRadius(this, "Body");

    MR::setSensorPos(this, "Body", vec40);
    MR::setSensorRadius(this, "Body", pushRadius);

    MR::sendMsgPush(pReceiver, pSender);

    MR::setSensorPos(this, "Body", pos);
    MR::setSensorRadius(this, "Body", radius);
}

bool Snakehead::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(Snakehead, SnakeheadNrvWaylay))) {
        return false;
    }

    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }

    if (isNerve(GET_NERVE(Snakehead, SnakeheadNrvTurtleDown))) {
        return false;
    }

    if (MR::isMsgJetTurtleAttack(msg)) {
        setNerve(GET_NERVE(Snakehead, SnakeheadNrvTurtleDown));
        return true;
    }

    return false;
}

void Snakehead::choiceAndStartBck(const char* pBck) {
    const char* v1 = nullptr;
    if (MR::isEqualString(pBck, "Forward")) {
        v1 = ::getParam(mType)->mForwardAction;
    } else if (MR::isEqualString(pBck, "Back")) {
        v1 = ::getParam(mType)->mBackAction;
    } else if (MR::isEqualString(pBck, "Waylay")) {
        v1 = ::getParam(mType)->mAppearBck;
    } else if (MR::isEqualString(pBck, "Wait")) {
        v1 = ::getParam(mType)->mWaitAction;
    }

    if (v1 != nullptr) {
        if (!MR::isEqualString(pBck, "Waylay")) {
            MR::startAction(this, v1);
        } else {
            MR::startBckNoInterpole(this, v1);
        }

        if (MR::isExistBtk(this, v1)) {
            MR::startBtk(this, v1);
        } else if (MR::isExistBtk(this, "Wait")) {
            MR::startBtk(this, "Wait");
            MR::setBtkFrameAndStop(this, 0.0f);
        }
    }
}

bool Snakehead::tryDPDSwoon() {
    if (isNerve(GET_NERVE(Snakehead, SnakeheadNrvDPDSwoon))) {
        return false;
    }

    if (isNerve(GET_NERVE(Snakehead, SnakeheadNrvWaylay)) && MR::getBckFrame(this) <= 2.0f) {
        return false;
    }

    if (isNerve(GET_NERVE(Snakehead, SnakeheadNrvTurtleDown))) {
        return false;
    }

    if (!mStarPointer->tryStartPointBind()) {
        return false;
    }

    setNerve(GET_NERVE(Snakehead, SnakeheadNrvDPDSwoon));

    return true;
}

bool Snakehead::isNearPlayerFromRail() const {
    TVec3f nearestPos;
    MR::calcNearestRailPos(&nearestPos, this, *MR::getPlayerPos());

    return nearestPos.distance(*MR::getPlayerPos()) <= mActivationRadius;
}
