#include "Game/Enemy/MogucchiHill.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    static const char* sPieceModelTable[] = {
        "MogucchiHillA",
        "MogucchiHillB",
        "MogucchiHillC",
    };
    static const f32 sPieceSensorRadius = 120.0f;
    // static const f32 sPieceLargeSensorRadius = _;
    static const f32 sMinScale = 0.6f;
    static const s32 sCrumbleFrame = 40;
    // static const s32 sPieceInterval = _;
    static const f32 sAppearDist = 300.0f;
    // static const s32 sNaturallyNum = _;
};  // namespace

namespace NrvMogucchiHillPiece {
    NEW_NERVE(MogucchiHillPieceNrvAppear, MogucchiHillPiece, Appear);
    NEW_NERVE(MogucchiHillPieceNrvWait, MogucchiHillPiece, Wait);
    NEW_NERVE(MogucchiHillPieceNrvCrumble, MogucchiHillPiece, Crumble);
};  // namespace NrvMogucchiHillPiece

namespace NrvMogucchiHill {
    NEW_NERVE(MogucchiHillNrvMove, MogucchiHill, Move);
    NEW_NERVE(MogucchiHillNrvWait, MogucchiHill, Wait);
};  // namespace NrvMogucchiHill

MogucchiHillPiece::MogucchiHillPiece(const char* pName) : LiveActor(pName), mHost(), mModelName(), mJointName(), mAutoEffectName(), mCanBeDestroyed(), mWasAttackedThisFrame() {
    mBaseMtx.identity();
}

// NONMATCHING: sensor and sound vector temporary stack slots are exchanged.
void MogucchiHillPiece::init(const JMapInfoIter& rIter) {
    TVec3f soundOffset;
    MR::initActor(this, mModelName, false);
    MR::connectToSceneMapObjDecorationStrongLight(this);
    initHitSensor(1);
    MR::addHitSensorAtJointMapObj(this, "Body", mJointName, 8, mScale.x * ::sPieceSensorRadius, TVec3f(0.0f, 0.0f, 0.0f));

    if (mAutoEffectName != nullptr) {
        initEffectKeeper(0, mAutoEffectName, false);
    }

    if (mHasLightCtrl) {
        MR::initLightCtrl(this);
    }

    soundOffset.setAll<f32>(0.0f);
    initSound(1, "MogucchiHill", nullptr, soundOffset);
    initNerve(GET_NERVE(MogucchiHillPiece, MogucchiHillPieceNrvWait));
    makeActorDead();
}

void MogucchiHillPiece::control() {
    f32 sqrDist = (mPosition - *MR::getPlayerCenterPos()).squared();

    if (MR::isPlayerHipDropLand() && sqrDist < ::sAppearDist * ::sAppearDist) {
        if (MR::isExistEffectKeeper(this)) {
            MR::emitEffect(this, "Break");
        }

        kill();
    }
}

void MogucchiHillPiece::appear() {
    LiveActor::appear();
    setNerve(GET_NERVE(MogucchiHillPiece, MogucchiHillPieceNrvAppear));
    MR::hideModel(this);

    mCanBeDestroyed = false;

    MR::validateClipping(this);
}

void MogucchiHillPiece::initWithModelName(const char* pModelName, const char* pJointName, const char* pAutoEffectName, bool hasLightCtrl) {
    mModelName = pModelName;
    mJointName = pJointName;
    mAutoEffectName = pAutoEffectName;
    mHasLightCtrl = hasLightCtrl;

    initWithoutIter();
}

void MogucchiHillPiece::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::startAction(this, "Lead");
        MR::startBrk(this, "Normal");
        MR::startActionSound(this, "EmMoghillAppear");
    }

    MR::setNerveAtBckStopped(this, GET_NERVE(MogucchiHillPiece, MogucchiHillPieceNrvWait));
}

void MogucchiHillPiece::exeWait() {
    if (mHost->isNerve(GET_NERVE(MogucchiHill, MogucchiHillNrvWait))) {
        return;
    }

    if (!mWasAttackedThisFrame) {
        mCanBeDestroyed = true;
    }

    mWasAttackedThisFrame = false;
}

void MogucchiHillPiece::exeCrumble() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Fade");
        MR::startBrk(this, "Fade");
    }

    if (MR::isActionEnd(this)) {
        kill();
    }
}

void MogucchiHillPiece::crumble() {
    if (MR::isDead(this)) {
        return;
    }

    if (MR::isClipped(this)) {
        kill();
        return;
    }

    if (isNerve(GET_NERVE(MogucchiHillPiece, MogucchiHillPieceNrvCrumble))) {
        return;
    }

    MR::invalidateClipping(this);
    setNerve(GET_NERVE(MogucchiHillPiece, MogucchiHillPieceNrvCrumble));
}

void MogucchiHillPiece::destroy() {
    if (MR::isExistEffectKeeper(this)) {
        MR::emitEffect(this, "Break");
    }

    kill();
}

void MogucchiHillPiece::setSize(f32 size) {
    mScale.setAll< f32 >(size);
    MR::setSensorRadius(this, "Body", ::sPieceSensorRadius * size);
}

void MogucchiHillPiece::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mBaseMtx);
}

void MogucchiHillPiece::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    bool isCrumbleEnd = isNerve(GET_NERVE(MogucchiHillPiece, MogucchiHillPieceNrvCrumble)) && MR::isGreaterEqualStep(this, ::sCrumbleFrame);

    if (isCrumbleEnd) {
        return;
    }

    if (MR::isSensorPlayerOrRide(pReceiver)) {
        if (MR::isPlayerDamaging()) {
            destroy();
            return;
        }

        if (isNerve(GET_NERVE(MogucchiHillPiece, MogucchiHillPieceNrvAppear)) && mHost->notifyAppearAttackToPlayer(pSender, pReceiver)) {
            return;
        }

        if (isNerve(GET_NERVE(MogucchiHillPiece, MogucchiHillPieceNrvWait)) && mHost->notifyWaitAttackToPlayer(pSender, pReceiver)) {
            return;
        }

        if (MR::isPlayerHipDropFalling()) {
            return;
        }

        if (isTargetGoingAway(pSender, pReceiver)) {
            return;
        }
    }

    MR::sendMsgPush(pReceiver, pSender);
}

bool MogucchiHillPiece::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isDead(this)) {
        return false;
    }

    bool isCrumbleEnd = isNerve(GET_NERVE(MogucchiHillPiece, MogucchiHillPieceNrvCrumble)) && MR::isGreaterEqualStep(this, ::sCrumbleFrame);

    if (isCrumbleEnd) {
        return false;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        MR::startActionSound(this, "EmMoghillBreak");
        destroy();

        return true;
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
        MR::startActionSound(this, "EmMoghillBreak");
        destroy();

        return false;
    }

    if (MR::isMsgPlayerTrample(msg)) {
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
        MR::startActionSound(this, "EmMoghillBreak");
        destroy();

        return true;
    }

    bool isHitAll = false;

    if (MR::isMsgPlayerHitAll(msg)) {
        isHitAll = true;
    }

    if (isHitAll) {
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        MR::startActionSound(this, "EmMoghillBreak");
        mHost->killPieces(this);

        return true;
    }

    return false;
}

bool MogucchiHillPiece::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    bool isCrumbleEnd = isNerve(GET_NERVE(MogucchiHillPiece, MogucchiHillPieceNrvCrumble)) && MR::isGreaterEqualStep(this, ::sCrumbleFrame);

    if (isCrumbleEnd) {
        return false;
    }

    mWasAttackedThisFrame = true;

    if (!mCanBeDestroyed) {
        return false;
    }

    MR::tryRumblePadWeak(this, WPAD_CHAN0);
    MR::startActionSound(this, "EmMoghillBreakSoft");
    destroy();

    return true;
}

bool MogucchiHillPiece::isTargetGoingAway(HitSensor* pSender, HitSensor* pReceiver) const NO_INLINE {
    const TVec3f* senderPos = MR::getSensorPos(pSender);
    TVec3f toReceiver = *MR::getSensorPos(pReceiver) - *senderPos;

    return toReceiver.dot(MR::getSensorHost(pReceiver)->mVelocity) >= 0.0f;
}

MogucchiHill::MogucchiHill(LiveActor* pHost, s32 pieceCount, const char* pName)
    : LiveActor(pName), mHost(pHost), mPieces(), mPieceCount(pieceCount), mNextPieceIndex(), mCanAppearPiece(), mAppearDistance(100.0f), mReserveDistance(150.0f), mModelCount(3), mModelNameTable(::sPieceModelTable),
      mJointName("MogucchiHill"), mAutoEffectName("MogucchiHill"), mHasLightCtrl(), mNaturalMode(), mGrowStep(), mAppearNum() {
    if (pieceCount > 100) {
        mPieceCount = 100;
    }

    mAppearNum = mPieceCount;

    mReserveMtx.identity();
}

void MogucchiHill::init(const JMapInfoIter& rIter) {
    MR::connectToSceneEnemyMovement(this);
    initNerve(GET_NERVE(MogucchiHill, MogucchiHillNrvMove));
    createPieces();
    makeActorDead();
}

void MogucchiHill::start() {
    if (MR::isDead(this)) {
        MR::invalidateClipping(this);
        appear();
    }

    setNerve(GET_NERVE(MogucchiHill, MogucchiHillNrvMove));

    mGrowStep = 3;
}

void MogucchiHill::end() {
    setNerve(GET_NERVE(MogucchiHill, MogucchiHillNrvWait));
}

void MogucchiHill::startNaturally() {
    mNaturalMode = 1;

    start();
}

void MogucchiHill::endNaturally() {
    mNaturalMode = 2;
}

void MogucchiHill::killPieces(MogucchiHillPiece* pIter) {
    s32 index = pIter - mPieces - 2;

    for (s32 i = 0; i < 3; i++) {
        index = (index + mPieceCount + 1) % mPieceCount;

        if (MR::isDead(&mPieces[index])) {
            continue;
        }

        mPieces[index].destroy();
    }
}

void MogucchiHill::pauseOff() {
    MR::requestMovementOn(this);

    for (u32 i = 0; i < mPieceCount; i++) {
        MR::requestMovementOn(&mPieces[i]);
    }
}

void MogucchiHill::killAllPieces() {
    for (u32 i = 0; i < mPieceCount; i++) {
        mPieces[i].makeActorDead();
    }
}

void MogucchiHill::setAppearNum(s32 appearNum) {
    mAppearNum = appearNum;
}

void MogucchiHill::exeMove() {
    if (MR::isFirstStep(this)) {
        reserveCurrentMtx();
    }

    if (mCanAppearPiece && getDistanceFromReserveMtx() >= mAppearDistance) {
        appearPiece();
    }

    if (getDistanceFromReserveMtx() >= mReserveDistance) {
        reserveCurrentMtx();
    }
}

void MogucchiHill::exeWait() {
    mGrowStep = 0;
}

void MogucchiHill::createPieces() {
    mPieces = new MogucchiHillPiece[mPieceCount]();

    s32 modelCount = mModelCount;
    s32 previousModel = MR::getRandom(0, modelCount);
    s32 secondPreviousModel = -2;
    s32 model;
    s32 thirdPreviousModel = -3;

    for (u32 i = 0; i < mPieceCount; i++) {
        s32 offset = MR::getRandom(0, modelCount - 1);
        model = previousModel + offset + 1;
        model = (model + modelCount) % modelCount;

        if (previousModel == thirdPreviousModel && secondPreviousModel == model) {
            s32 alternativeCount = modelCount - 1;
            offset = (offset + alternativeCount + 1) % alternativeCount;
            model = previousModel + offset + 1;
            model = (model + modelCount) % modelCount;
        }

        thirdPreviousModel = secondPreviousModel;
        secondPreviousModel = previousModel;
        previousModel = model;

        mPieces[i].mHost = this;
        mPieces[i].mScale.setAll< f32 >(mScale.x);
        mPieces[i].initWithModelName(mModelNameTable[model], mJointName, mAutoEffectName, mHasLightCtrl);
    }
}

void MogucchiHill::appearPiece() {
    killPieceOverAppear();
    killPieceIfAlive();

    switch (mNaturalMode) {
    case 0:
        break;
    case 1:
        mGrowStep++;

        if (mGrowStep >= 3) {
            mGrowStep = 3;
            mNaturalMode = 0;
        }

        break;
    case 2:
        mGrowStep--;

        if (mGrowStep <= 0) {
            mGrowStep = 0;
            setNerve(GET_NERVE(MogucchiHill, MogucchiHillNrvWait));
            mNaturalMode = 0;
            return;
        }

        break;
    }

    if (mGrowStep != 0) {
        mPieces[mNextPieceIndex].setSize(mScale.x * (::sMinScale + (1.0f - ::sMinScale) * (mGrowStep - 1) / 2.0f));
        MogucchiHillPiece* pPiece = &mPieces[mNextPieceIndex];
        pPiece->mBaseMtx.setInline(mReserveMtx);
        pPiece->mBaseMtx.getTrans(pPiece->mPosition);
        pPiece->appear();
        s32 count = mPieceCount;
        s32 index = mNextPieceIndex;
        mCanAppearPiece = false;
        mNextPieceIndex = (index + count + 1) % count;
    }
}

void MogucchiHill::killPieceOverAppear() {
    s32 index = (mPieceCount + (mNextPieceIndex - mAppearNum)) % mPieceCount;

    mPieces[index].crumble();
}

void MogucchiHill::killPieceIfAlive() {
    if (MR::isDead(&mPieces[mNextPieceIndex])) {
        return;
    }

    mPieces[mNextPieceIndex].kill();
}

f32 MogucchiHill::getDistanceFromReserveMtx() const NO_INLINE {
    TVec3f trans;
    mReserveMtx.getTrans(trans);

    return mHost->mPosition.distance(trans);
}

void MogucchiHill::reserveCurrentMtx() {
    mReserveMtx.setInline(mHost->getBaseMtx());
    mCanAppearPiece = true;
}
