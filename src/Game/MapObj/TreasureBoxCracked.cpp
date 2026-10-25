#include "Game/MapObj/TreasureBoxCracked.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/BenefitItemObj.hpp"
#include "Game/MapObj/BenefitItemOneUp.hpp"
#include "Game/Util.hpp"

namespace NrvTreasureBoxCracked {
    NEW_NERVE(TreasureBoxCrackedNrvWait, TreasureBoxCracked, Wait);
    NEW_NERVE(TreasureBoxCrackedNrvOpenWait, TreasureBoxCracked, OpenWait);
    NEW_NERVE(TreasureBoxCrackedNrvAlwaysOpen, TreasureBoxCracked, AlwaysOpen);
};  // namespace NrvTreasureBoxCracked

namespace {
    Color8 sPointLightColor(0xFF, 0xFF, 0x96, 0xFF);
    const Vec sTopOpenStartOffset = {0.0f, -100.0f, -250.0f};
    const Vec sTopOpenEndOffset = {0.0f, 23.0f, -250.0f};
    const Vec sCoinOffset = {0.0f, 130.0f, 0.0f};
    const Vec sChipOffset = {0.0f, 170.0f, 0.0f};
    const Vec sKinokoOneUpOffset = {0.0f, 130.0f, 0.0f};
    const Vec sKinokoLifeUpOffset = {0.0f, 130.0f, 0.0f};
    const Vec sAirBubbleOffset = {0.0f, 220.0f, 0.0f};
    const f32 sFirstSpeed = 15.0f;
    const f32 sAccelRate = 1.2f;
    const f32 sBoundSpeedMax = 15.0f;
    const s32 sBoundTime = 20;
    const s32 sDefaultWaitForSwitchOn = 5;
    const f32 sPointLightBrightness = 0.99f;
    const f32 sStepForCollisionMove = 15.0f;
};  // namespace

TreasureBoxCracked::TreasureBoxCracked(const char* pName) : LiveActor(pName) {
    mBoxCollider = nullptr;
    mBoxType = 0;
    mOpenCondition = 0;
    mItem = nullptr;
    mItemType = 0;
    mWaitForSwitchOn = ::sDefaultWaitForSwitchOn;
    mTopMtx.identity();
}

void TreasureBoxCracked::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::validateCollisionParts(mBoxCollider);
}

void TreasureBoxCracked::makeActorDead() {
    LiveActor::makeActorDead();
    MR::invalidateCollisionParts(mBoxCollider);
}

void TreasureBoxCracked::init(const JMapInfoIter& rIter) {
    const char* pObjectName;
    MR::getObjectName(&pObjectName, rIter);

    const char* pName;
    if (MR::isEqualSubString(pObjectName, "TreasureBoxCracked")) {
        mBoxType = 1;
        pName = "TreasureBoxCracked";
    } else if (MR::isEqualSubString(pObjectName, "TreasureBoxGold")) {
        mBoxType = 2;
        pName = "TreasureBoxGold";
    } else {
        mBoxType = 0;
        pName = "TreasureBox";
    }

    MR::initActor(this, rIter, pName, false);
    MR::getJMapInfoArg2NoInit(rIter, &mOpenCondition);
    mTopMtx.set(MR::getJointMtx(this, "Top"));
    MR::initCollisionParts(this, "Top", getSensor("Body"), mTopMtx.toMtxPtr());
    mBoxCollider = MR::createCollisionPartsFromLiveActor(this, "Box", getSensor("Body"), MR::CollisionScaleType_Unk2);
    MR::validateCollisionParts(mBoxCollider);

    if (mOpenCondition == 2) {
        initNerve(GET_NERVE(TreasureBoxCracked, TreasureBoxCrackedNrvAlwaysOpen));
    } else {
        initNerve(GET_NERVE(TreasureBoxCracked, TreasureBoxCrackedNrvWait));
    }

    initItem(rIter);
    MR::addToAttributeGroupSearchTurtle(this);
    makeActorAppeared();
}

void TreasureBoxCracked::exeWait() {
    if (MR::isFirstStep(this)) {
        if (mBoxType == 1) {
            MR::startBrk(this, "Wait");
            MR::emitEffect(this, "Light");
        } else if (mBoxType == 2) {
            MR::emitEffect(this, "Glow");
        }
    }

    if (mBoxType == 2) {
        MR::requestPointLight(this, mPosition, ::sPointLightColor, ::sPointLightBrightness, -1);
    }
}

void TreasureBoxCracked::exeOpenWait() {
    if (mItemType == 0 && MR::isStep(this, mWaitForSwitchOn)) {
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
    }

    if (MR::isActionEnd(this)) {
        MR::invalidateHitSensors(this);

        if (MR::isValidSwitchDead(this)) {
            MR::onSwitchDead(this);
        }
    }
}

void TreasureBoxCracked::exeAlwaysOpen() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
        MR::startAction(this, "Open");
        MR::setBckFrameAndStop(this, MR::getBckCtrl(this)->getEnd());
        TVec3f position;
        PSMTXMultVec(getBaseMtx(), &::sTopOpenEndOffset, &position);
        mTopMtx.setTrans(position);
    }
}

void TreasureBoxCracked::control() {
    if (MR::isBckPlaying(this, "Open")) {
        f32 frame = MR::getBckFrame(this);

        if (!(frame >= ::sStepForCollisionMove)) {
            f32 rate = frame / ::sStepForCollisionMove;
            TVec3f offset;
            TVec3f position;
            MR::vecBlend(&offset, ::sTopOpenStartOffset, ::sTopOpenEndOffset, rate);
            PSMTXMultVec(getBaseMtx(), &offset, &position);
            mTopMtx.setTrans(position);

            if (frame == 0.0f) {
                MR::resetAllCollisionMtx(this);
            }
        }
    }
}

void TreasureBoxCracked::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isHalfOpen() && MR::isSensorPlayerOrRide(pReceiver)) {
        TVec3f direction;
        direction.sub(*MR::getSensorPos(pReceiver), *MR::getSensorPos(pSender));
        MR::normalize(&direction);
        TVec3f front;
        MR::calcFrontVec(&front, this);

        if (front.dot(direction) < 0.0f && !MR::isPlayerDamaging()) {
            MR::sendMsgEnemyAttackFlipWeak(pReceiver, pSender);
        }
    }
}

bool TreasureBoxCracked::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(TreasureBoxCracked, TreasureBoxCrackedNrvOpenWait))) {
        return false;
    }

    if (isNerve(GET_NERVE(TreasureBoxCracked, TreasureBoxCrackedNrvAlwaysOpen))) {
        return false;
    }

    if (MR::isMsgJetTurtleAttack(msg) || MR::isMsgYoshiProjectile(msg)) {
        if (mOpenCondition == 1 && !MR::isSensorType(pSender, 0x14)) {
            MR::startAction(this, "Shock");
            MR::startActionSound(this, "OjTerasBoxGoldRefuse", -1, -1, -1);
            MR::startActionSound(this, "SyFailure1", -1, -1, -1);
            return false;
        }

        makeOpen();
        return true;
    }

    return false;
}

bool TreasureBoxCracked::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg) && isNerve(GET_NERVE(TreasureBoxCracked, TreasureBoxCrackedNrvWait))) {
        if (mOpenCondition == 1) {
            MR::startActionSound(this, "OjTerasBoxGoldRefuse", -1, -1, -1);
            return false;
        }

        makeOpen();
        return true;
    }

    if (MR::isMsgToEnemyAttackBlow(msg) && mOpenCondition == 3) {
        MR::startBlowHitReaction(this);
        makeOpen();
        return true;
    }

    return false;
}

void TreasureBoxCracked::initItem(const JMapInfoIter& rIter) {
    const char* pObjectName;
    MR::getObjectName(&pObjectName, rIter);

    if (MR::isEqualSubString(pObjectName, "Empty")) {
        mItemType = 0;
        MR::getJMapInfoArg0NoInit(rIter, &mWaitForSwitchOn);
        return;
    }

    mItemType = 1;
    mItem = new BenefitItemOneUp("１ＵＰキノコ");
    MR::setPosition(mItem, mPosition);
    MR::setRotation(mItem, mRotation);
    mItem->initWithoutIter();
    mItem->makeActorDead();
}

void TreasureBoxCracked::makeOpen() {
    MR::startAction(this, "Open");
    MR::emitEffect(this, "Open");

    if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        MR::startActionSound(this, "OjTreasureBoxOpenW", -1, -1, -1);
    } else {
        MR::startActionSound(this, "OjTreasureBoxOpen", -1, -1, -1);
    }

    if (mBoxType == 1) {
        MR::startBrk(this, "Open");
        MR::deleteEffect(this, "Light");
    } else if (mBoxType == 2) {
        MR::deleteEffect(this, "Glow");
    }

    if (mItemType == 0) {
        setNerve(GET_NERVE(TreasureBoxCracked, TreasureBoxCrackedNrvOpenWait));
        return;
    }

    mItem->_DE = 1;
    TVec3f front(0.0f, 0.0f, 1.0f);
    MR::calcFrontVec(&front, this);
    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::calcUpVec(&up, this);
    JMAVECScaleAdd(&up, &mPosition, &mItem->mPosition, 120.0f);
    mItem->appearPop();
    setNerve(GET_NERVE(TreasureBoxCracked, TreasureBoxCrackedNrvOpenWait));
}

bool TreasureBoxCracked::isHalfOpen() const {
    if (isNerve(GET_NERVE(TreasureBoxCracked, TreasureBoxCrackedNrvWait))) {
        return false;
    }

    if (isNerve(GET_NERVE(TreasureBoxCracked, TreasureBoxCrackedNrvAlwaysOpen))) {
        return false;
    }

    return !MR::isActionEnd(this);
}

TreasureBoxCracked::~TreasureBoxCracked() {
}
