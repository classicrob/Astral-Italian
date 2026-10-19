#include "Game/Enemy/OtaRock.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/CocoNutBall.hpp"
#include "Game/Enemy/FireBall.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/CocoNut.hpp"
#include "Game/NameObj/NameObjArchiveListCollector.hpp"
#include "Game/Util.hpp"

namespace {
    const Vec cSensorOffset = {0.0f, 180.0f, 0.0f};
    const f32 cSensorRadius = 300.0f;
    const s32 cWaitFrameMin = 30;
    const s32 cWaitFrameMax = 90;
    const f32 cAppearDistance = 3800.0f;
    const f32 cHideDistance = 4000.0f;
    const s32 cThrowBallStep = 40;
    const f32 cThrowAngleMin = 5.0f;
    const f32 cThrowAngleMax = 10.0f;
    const f32 cThrowFireBallSpeed = 15.0f;
    const s32 cWaitFrameMinNoThrowCocoNut = 120;
    const s32 cWaitFrameMaxNoThrowCocoNut = 180;
};  // namespace

namespace NrvOtaRock {
    NEW_NERVE(OtaRockNrvWait, OtaRock, Wait);
    NEW_NERVE(OtaRockNrvThrowCocoNut, OtaRock, ThrowCocoNut);
    NEW_NERVE(OtaRockNrvThrowFireBall, OtaRock, ThrowFireBall);
    NEW_NERVE(OtaRockNrvDown, OtaRock, Down);
    NEW_NERVE(OtaRockNrvHide, OtaRock, Hide);
    NEW_NERVE(OtaRockNrvHideWait, OtaRock, HideWait);
    NEW_NERVE(OtaRockNrvShow, OtaRock, Show);
    class OtaRockNrvOpeningDemo : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<OtaRock*>(spine->mExecutor)->exeOtaRockOpeningDemo(); }
        static OtaRockNrvOpeningDemo sInstance;
    };
    OtaRockNrvOpeningDemo OtaRockNrvOpeningDemo::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvOtaRock

void OtaRock::initOtaRockPlacement(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initUseStageSwitchReadA(this, rIter);
    MR::initUseStageSwitchWriteDead(this, rIter);
    MR::useStageSwitchAwake(this, rIter);
    MR::getJMapInfoArg0NoInit(rIter, &this->mNoThrowCocoNut);
    bool flag = false;
    MR::getJMapInfoArg1NoInit(rIter, &flag);
    if (flag) {
        this->mThrowCocoNutCounter = 0;
    }
}

void OtaRock::chooseWaitNerve() {
    if (this->mNoThrowCocoNut) {
        this->setNerve(GET_NERVE(OtaRock, OtaRockNrvWait));
    } else {
        this->setNerve(GET_NERVE(OtaRock, OtaRockNrvHideWait));
    }
}


void OtaRock::exeOtaRockOpeningDemo() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    this->updateBaseMtx();

    if (!MR::isStageStateScenarioOpeningCamera()) {
        this->chooseWaitNerve();
    }
}

bool OtaRock::tryStartOpeningDemo() {
    if (MR::isStageStateScenarioOpeningCamera()) {
        this->setNerve(GET_NERVE(OtaRock, OtaRockNrvOpeningDemo));
        return true;
    }
    return false;
}

OtaRock::OtaRock(const char* pName)
    : LiveActor(pName), mNoThrowCocoNut(false), mCocoNutArray(nullptr), mFireBallArray(nullptr), mFixedPosition(nullptr),
      mThrowCocoNutCounter(1), mWaitFrame(0), mBaseMtx(), mFront(0.0f, 0.0f, 1.0f), mAnimScaleController(nullptr) {
    mBaseMtx.identity();
}

void OtaRock::init(const JMapInfoIter& rIter) {
    this->initOtaRockPlacement(rIter);
    MR::calcGravity(this);
    initModel();
    MR::connectToSceneEnemy(this);
    MR::initLightCtrl(this);
    initSensor();
    initEffectKeeper(0, nullptr, false);
    initSound(8, "OtaRock", nullptr, TVec3f(0.0f));
    MR::addToAttributeGroupSearchTurtle(this);
    mAnimScaleController = new AnimScaleController(nullptr);
    initNerve(GET_NERVE(OtaRock, OtaRockNrvWait), 0);
    this->chooseWaitNerve();

    if (MR::isExistStageSwitchAwake(rIter)) {
        makeActorDead();
    } else {
        makeActorAppeared();
    }
}

void OtaRock::appear() {
    LiveActor::appear();
    if (mNoThrowCocoNut) {
        setNerve(GET_NERVE(OtaRock, OtaRockNrvWait));
    } else {
        setNerve(GET_NERVE(OtaRock, OtaRockNrvHideWait));
    }
}

void OtaRock::kill() {
    MR::emitEffect(this, "OtaRockDeath");
    MR::startActionSound(this, "EmOtarockDie");
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
    LiveActor::kill();
}

void OtaRock::makeArchiveList(NameObjArchiveListCollector* pCollector, const JMapInfoIter& rIter) {
    bool noCocoNut = false;
    MR::getJMapInfoArg0NoInit(rIter, &noCocoNut);

    if (!noCocoNut) {
        pCollector->addArchive(CocoNut::getModelName());
    }
}

void OtaRock::control() {
    mAnimScaleController->updateNerve();
}

void OtaRock::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mBaseMtx);
    TVec3f scale = mAnimScaleController->_C * mScale;
    MR::setBaseScale(this, scale);
}

void OtaRock::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
            mAnimScaleController->startHitReaction();
        } else {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }

    if (MR::isSensorYoshi(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool OtaRock::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        mAnimScaleController->startHitReaction();
        return true;
    }

    if (MR::isMsgJetTurtleAttack(msg)) {
        setNerve(GET_NERVE(OtaRock, OtaRockNrvDown));
        return true;
    }

    return true;
}

bool OtaRock::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    setNerve(GET_NERVE(OtaRock, OtaRockNrvDown));
    return true;
}

void OtaRock::initModel() {
    MR::initActor(this, "OtaRock", false);

    if (!mNoThrowCocoNut) {
        mCocoNutArray = new CocoNutBall[4];
        for (int i = 0; i < 4; i++) {
            mCocoNutArray[i].initWithoutIter();
            mCocoNutArray[i]._8C = this;
        }
    }

    mFireBallArray = new FireBall[3];
    for (int i = 0; i < 3; i++) {
        mFireBallArray[i].initWithoutIter();
        mFireBallArray[i].mHost = this;
    }

    mFixedPosition = new FixedPosition(this, "Mouth", TVec3f(0.0f, 0.0f, 50.0f), TVec3f(0.0f, 0.0f, -90.0f));

    TPos3f mtxTR;
    MR::makeMtxTR(mtxTR.toMtxPtr(), this);
    mtxTR.getZDir(mFront);
}

void OtaRock::initSensor() {
    initHitSensor(1);
    MR::addHitSensorMtxEnemy(this, "Body", 8, ::cSensorRadius, getBaseMtx(), static_cast< TVec3f >(::cSensorOffset));
}

void OtaRock::updateBaseMtx() {
    TVec3f up = -mGravity;
    TVec3f front;
    if (MR::isStageStateScenarioOpeningCamera()) {
        front.set(mFront);
    } else {
        front.sub(*MR::getPlayerPos(), mPosition);
        MR::normalize(&front);
    }
    MR::makeMtxUpFrontPos(&mBaseMtx, up, front, mPosition);
}

CocoNutBall* OtaRock::getDisappearedCocoNut() {
    for (s32 i = 0; i < 4; i++) {
        if (MR::isDead(&mCocoNutArray[i])) {
            return &mCocoNutArray[i];
        }
    }

    return nullptr;
}

FireBall* OtaRock::getDisappearedFireBall() {
    for (s32 i = 0; i < 3; i++) {
        if (MR::isDead(&mFireBallArray[i])) {
            return &mFireBallArray[i];
        }
    }
    return nullptr;
}

s32 OtaRock::getDisappearedCocoNutNum() const {
    s32 num = 0;
    for (s32 i = 0; i < 4; i++) {
        if (MR::isDead(&mCocoNutArray[i])) {
            num++;
        }
    }
    return num;
}

s32 OtaRock::getDisappearedFireBallNum() const {
    s32 num = 0;
    for (s32 i = 0; i < 3; i++) {
        if (MR::isDead(&mFireBallArray[i])) {
            num++;
        }
    }
    return num;
}

bool OtaRock::isValidThrowCocoNut() const {
    return getDisappearedCocoNutNum() > 0;
}

bool OtaRock::isValidThrowFireBall() const {
    return getDisappearedFireBallNum() > 0;
}

void OtaRock::throwCocoNut() {
    CocoNutBall* pCocoNut = OtaRock::getDisappearedCocoNut();
    TVec3f fixedTrans;
    mFixedPosition->calc();
    mFixedPosition->copyTrans(&fixedTrans);
    f32 f = MR::getRandom(::cThrowAngleMin, ::cThrowAngleMax);
    pCocoNut->appearAndThrow(fixedTrans, MR::isHalfProbability() ? f : -f);
    mThrowCocoNutCounter++;
}

void OtaRock::throwFireBall() {
    FireBall* fireBall = getDisappearedFireBall();
    TVec3f fixedTrans;
    mFixedPosition->calc();
    mFixedPosition->copyTrans(&fixedTrans);
    fireBall->appearAndThrow(fixedTrans, ::cThrowFireBallSpeed, 0.0f);
}

bool OtaRock::tryToHide() {
    bool canHide;

    if (MR::isValidSwitchA(this)) {
        canHide = !MR::isOnSwitchA(this);
    } else {
        canHide = !MR::isNearPlayer(this, ::cHideDistance);
    }

    if (!mNoThrowCocoNut && getDisappearedCocoNutNum() == 4 && canHide) {
        setNerve(GET_NERVE(OtaRock, OtaRockNrvHide));
        return true;
    }
    return false;
}

void OtaRock::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        if (mNoThrowCocoNut) {
            mWaitFrame = MR::getRandom(::cWaitFrameMinNoThrowCocoNut, ::cWaitFrameMaxNoThrowCocoNut);
        } else {
            mWaitFrame = MR::getRandom(::cWaitFrameMin, ::cWaitFrameMax);
        }
    }
    updateBaseMtx();
    if (!this->tryStartOpeningDemo() && !tryToHide() && MR::isGreaterStep(this, mWaitFrame)) {
        if (!mNoThrowCocoNut && mThrowCocoNutCounter < 1) {
            if (isValidThrowCocoNut()) {
                setNerve(GET_NERVE(OtaRock, OtaRockNrvThrowCocoNut));
            }
        } else {
            if (isValidThrowFireBall()) {
                mThrowCocoNutCounter = 0;
                setNerve(GET_NERVE(OtaRock, OtaRockNrvThrowFireBall));
            }
        }
    }
}

void OtaRock::exeThrowCocoNut() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::startActionSound(this, "EvOtarockPreRallyball");
    }
    OtaRock::updateBaseMtx();
    if (MR::isStep(this, ::cThrowBallStep)) {
        throwCocoNut();
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(OtaRock, OtaRockNrvWait));
    }
}

void OtaRock::exeThrowFireBall() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::startActionSound(this, "EvOtarockPreHotball");
    }
    OtaRock::updateBaseMtx();
    if (MR::isStep(this, ::cThrowBallStep)) {
        MR::startActionSound(this, "EmOtarockFireOut");
        throwFireBall();
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(OtaRock, OtaRockNrvWait));
    }
}

void OtaRock::exeDown() {
    if (MR::isFirstStep(this)) {
        MR::startBckNoInterpole(this, "Death");
        MR::startActionSound(this, "EmOtarockLastDamage");
        MR::startActionSound(this, "EvOtarockDie");
        MR::invalidateHitSensors(this);
        MR::tryRumblePadVeryStrong(this, WPAD_CHAN0);

        if (!mNoThrowCocoNut) {
            for (s32 i = 0; i < 4; i++) {
                CocoNutBall* cocoNut = &mCocoNutArray[i];
                if (!MR::isDead(&mCocoNutArray[i]))
                    mCocoNutArray[i].kill();
            }
        }
        for (s32 i = 0; i < 3; i++) {
            FireBall* fireBall = &mFireBallArray[i];
            if (!MR::isDead(&mFireBallArray[i]))
                mFireBallArray[i].kill();
        }
    }

    if (MR::isStep(this, 2)) {
        MR::stopScene(16);
    }

    if (MR::isStep(this, 3)) {
        MR::shakeCameraNormal();
    }

    if (MR::isActionEnd(this)) {
        kill();
    }
}

void OtaRock::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        MR::startActionSound(this, "EmOtarockHide");
    }
    updateBaseMtx();
    MR::setNerveAtBckStopped(this, GET_NERVE(OtaRock, OtaRockNrvHideWait));
}

void OtaRock::exeHideWait() {
    if (MR::isFirstStep(this)) {
        MR::startBckNoInterpole(this, "HideWait");
    }
    updateBaseMtx();

    if (this->tryStartOpeningDemo()) {
        return;
    }

    bool show;

    if (MR::isValidSwitchA(this)) {
        show = MR::isOnSwitchA(this);
    } else {
        show = MR::isNearPlayer(this, ::cAppearDistance);
    }

    if (show) {
        setNerve(GET_NERVE(OtaRock, OtaRockNrvShow));
    }
}

void OtaRock::exeShow() {
    if (MR::isFirstStep(this)) {
        MR::startBckNoInterpole(this, "Show");
        MR::startActionSound(this, "EmOtarockAppear");
    }
    updateBaseMtx();
    MR::setNerveAtBckStopped(this, GET_NERVE(OtaRock, OtaRockNrvWait));
}

OtaRock::~OtaRock() {
}
