#include "Game/MapObj/Shellfish.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/BenefitItemOneUp.hpp"
#include "Game/MapObj/Coin.hpp"
#include "Game/MapObj/YellowChip.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"

namespace {
    const Vec sBodyHitOffset = {0.0f, 100.0f, 0.0f};
    const Vec sSandHitOffset = {0.0f, -100.0f, 200.0f};
    const Vec sCoinOffset = {0.0f, 50.0f, 30.0f};
    const Vec sChipOffset = {0.0f, 100.0f, 50.0f};
    const Vec sKinokoOneUpOffset = {0.0f, 70.0f, 30.0f};
    const s32 sStepToOpen = 150;
    const s32 sStepToSetPosItem = 100;
    const s32 sStepToCloseSignal = 170;
    const s32 sStepToClose = 150;
    const s32 sStepToHitSensorOn = 0;
    const s32 sStepToHitSensorOff = 15;
    const s32 sStepToApperItem = 40;
    const s32 sStepToDeadItem = 5;
    const s32 sStepToRecover = 540;
    const s32 sShockToInterpole = 2;
    const f32 sHitPushPower = 40.0f;
};  // namespace

namespace NrvShellfish {
    NEW_NERVE(ShellfishNrvWait, Shellfish, Wait);
    NEW_NERVE(ShellfishNrvOpen, Shellfish, Open);
    NEW_NERVE(ShellfishNrvOpenWait, Shellfish, OpenWait);
    NEW_NERVE(ShellfishNrvCloseSignal, Shellfish, CloseSignal);
    NEW_NERVE(ShellfishNrvClose, Shellfish, Close);
    NEW_NERVE(ShellfishNrvShock, Shellfish, Shock);
    NEW_NERVE(ShellfishNrvDPDOpen, Shellfish, DPDOpen);
};  // namespace NrvShellfish

Shellfish::Shellfish(const char* pName)
    : LiveActor(pName), mScaleController(), mStarPointerState(), mItem(), mItemType(), mIsItemBound(), mLowCollision() {
}

void Shellfish::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Shellfish", false);
    initHitSensor(2);
    MR::addHitSensorEnemy(this, "Body", 8, 400.0f, TVec3f(::sBodyHitOffset));
    MR::addHitSensorAtJointEnemyAttack(this, "core", "Lid", 8, 120.0f, TVec3f(::sSandHitOffset));
    MR::invalidateHitSensor(this, "core");
    MR::initCollisionPartsAutoEqualScaleOne(this, "Hi", getSensor("Body"), MR::getJointMtx(this, "Lid"));
    mLowCollision = MR::createCollisionPartsFromLiveActor(this, "Low", getSensor("Body"), MR::CollisionScaleType_NotUsingScale);
    MR::validateCollisionParts(mLowCollision);
    TVec3f front;
    MR::calcFrontVec(&front, this);
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f shadowPosition;
    shadowPosition.scaleAdd(30.0f, front, mPosition);
    shadowPosition.scaleAdd(5.0f, up, shadowPosition);
    MR::setShadowDropPosition(this, nullptr, shadowPosition);
    MR::setShadowDropDirection(this, nullptr, -up);
    MR::offCalcShadow(this, nullptr);
    MR::addEffectHitNormal(this, nullptr);
    mScaleController = new AnimScaleController(nullptr);
    mScaleController->setParamTight();
    mStarPointerState = new WalkerStateBindStarPointer(this, mScaleController);
    initNerve(GET_NERVE(Shellfish, ShellfishNrvWait), 1);
    MR::initActorState(this, mStarPointerState, GET_NERVE(Shellfish, ShellfishNrvDPDOpen), "SupportFreeze");
    initItem(rIter);
    MR::addToAttributeGroupSearchTurtle(this);
    makeActorAppeared();
}

void Shellfish::initAfterPlacement() {
    MR::onCalcShadowOneTime(this, nullptr);
}

void Shellfish::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    MR::updateBaseScale(this, mScaleController);
}

void Shellfish::control() {
    mScaleController->updateNerve();

    if ((isNerve(GET_NERVE(Shellfish, ShellfishNrvOpenWait)) || isNerve(GET_NERVE(Shellfish, ShellfishNrvCloseSignal)) ||
         (isNerve(GET_NERVE(Shellfish, ShellfishNrvShock)) && MR::isGreaterStep(this, 60))) &&
        mStarPointerState->tryStartPointBind()) {
        setNerve(GET_NERVE(Shellfish, ShellfishNrvDPDOpen));
    }
}

void Shellfish::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    if (MR::isStep(this, ::sStepToOpen)) {
        setNerve(GET_NERVE(Shellfish, ShellfishNrvOpen));
    }
}

void Shellfish::exeOpen() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Open");
        MR::startActionSound(this, "OjShellfishOpen", -1, -1, -1);
        putItem();
    }

    if (MR::isStep(this, ::sStepToApperItem)) {
        MR::sendArbitraryMsg(0x86 /* ACTMES_ITEM_SHOW */, mItem->getSensor(nullptr), getSensor("Body"));
        startBindItem();
    }

    if (mIsItemBound && MR::isLessStep(this, ::sStepToSetPosItem) && MR::isGreaterStep(this, ::sStepToApperItem)) {
        f32 speed = 0.5f;
        TVec3f up;
        MR::calcUpVec(&up, this);
        mItem->mPosition.scaleAdd(speed, up, mItem->mPosition);
    }

    if (MR::isStep(this, ::sStepToSetPosItem)) {
        if (mIsItemBound) {
            endBindItem();
        }
        setNerve(GET_NERVE(Shellfish, ShellfishNrvOpenWait));
    }
}

void Shellfish::exeOpenWait() {
    if (MR::isStep(this, ::sStepToCloseSignal)) {
        setNerve(GET_NERVE(Shellfish, ShellfishNrvCloseSignal));
    }
}

void Shellfish::exeCloseSignal() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "CloseSignal");
    }

    MR::startActionSound(this, "OjLvShellfishSignal");

    if (MR::isStep(this, ::sStepToClose)) {
        setNerve(GET_NERVE(Shellfish, ShellfishNrvClose));
    }
}

void Shellfish::exeClose() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Close");
        MR::startActionSound(this, "OjShellfishClose", -1, -1, -1);
    }

    if (MR::isStep(this, ::sStepToHitSensorOn)) {
        MR::validateHitSensor(this, "core");
    }

    if (MR::isStep(this, ::sStepToHitSensorOff)) {
        MR::invalidateHitSensor(this, "core");
    }

    if (MR::isStep(this, ::sStepToDeadItem)) {
        MR::sendArbitraryMsg(0x87 /* ACTMES_ITEM_HIDE */, mItem->getSensor(nullptr), getSensor("Body"));
    }

    if (MR::isActionEnd(this)) {
        MR::validateCollisionParts(this);
        setNerve(GET_NERVE(Shellfish, ShellfishNrvWait));
    }
}

void Shellfish::exeShock() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensor(this, "core");
        MR::sendArbitraryMsg(0x86 /* ACTMES_ITEM_SHOW */, mItem->getSensor(nullptr), getSensor("Body"));
    }

    if (MR::isStep(this, ::sStepToRecover)) {
        setNerve(GET_NERVE(Shellfish, ShellfishNrvCloseSignal));
    }
}

void Shellfish::exeDPDOpen() {
    MR::updateActorStateAndNextNerve(this, GET_NERVE(Shellfish, ShellfishNrvOpenWait));
}

// NONMATCHING: target inlines the TVec3<f>(f,f,f) ctor for `direction`; ours is written member-wise (instruction order differs).
void Shellfish::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pSender) && MR::isSensorPlayer(pReceiver)) {
        MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, nullptr);
        MtxPtr mtx = getBaseMtx();
        TVec3f direction;
        direction.z = mtx[2][2];
        direction.y = mtx[1][2];
        direction.x = mtx[0][2];

        if (MR::sendMsgEnemyAttackStrongToDir(pReceiver, pSender, direction)) {
            MR::invalidateCollisionParts(this);
        }

        TVec3f push;
        push.scale(::sHitPushPower, direction);
        MR::pushPlayer(push);
    }
}

bool Shellfish::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }

    if (MR::isMsgJetTurtleAttack(msg) || MR::isMsgStarPieceReflect(msg)) {
        MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, nullptr);

        if (isNerve(GET_NERVE(Shellfish, ShellfishNrvWait))) {
            MR::startBckWithInterpole(this, "CloseDamage", ::sShockToInterpole);
        } else {
            MR::startBckWithInterpole(this, "OpenDamage", ::sShockToInterpole);
        }

        if (mIsItemBound) {
            endBindItem();
        }

        setNerve(GET_NERVE(Shellfish, ShellfishNrvShock));
        return true;
    }

    return false;
}

void Shellfish::initItem(const JMapInfoIter& rIter) {
    const char* pObjectName;
    MR::getObjectName(&pObjectName, rIter);

    if (MR::isEqualStringCase(pObjectName, "ShellfishCoin")) {
        mItemType = 0;
    } else if (MR::isEqualStringCase(pObjectName, "ShellfishYellowChip")) {
        mItemType = 1;
    } else if (MR::isEqualStringCase(pObjectName, "ShellfishBlueChip")) {
        mItemType = 2;
    } else if (MR::isEqualStringCase(pObjectName, "ShellfishKinokoOneUp")) {
        mItemType = 3;
    }

    switch (mItemType) {
    case 0:
        initCoin(rIter);
        break;
    case 1:
        initYellowChip(rIter);
        break;
    case 3:
        initKinokoOneUp(rIter);
        break;
    }

    putItem();
    mItem->makeActorAppeared();
    MR::sendArbitraryMsg(0x87 /* ACTMES_ITEM_HIDE */, mItem->getSensor(nullptr), getSensor("Body"));
}

void Shellfish::initCoin(const JMapInfoIter&) {
    mItem = static_cast< Coin* >(MR::createCoin(this, "コイン"));
    mItem->initWithoutIter();
}

void Shellfish::initYellowChip(const JMapInfoIter& rIter) {
    s32 groupId;
    MR::getJMapInfoArg0WithInit(rIter, &groupId);
    YellowChip* chip = new YellowChip("イエローチップ");
    chip->setHost(this);
    chip->setGroupID(groupId);
    mItem = chip;
    chip->initWithoutIter();
}

void Shellfish::initKinokoOneUp(const JMapInfoIter&) {
    mItem = new BenefitItemOneUp("１ＵＰキノコ");
    mItem->initWithoutIter();
}

void Shellfish::startBindItem() {
    mIsItemBound = MR::sendArbitraryMsg(0x8B /* ACTMES_ITEM_START_MOVE */, mItem->getSensor(nullptr), getSensor("Body"));
}

void Shellfish::endBindItem() {
    putItem();
    TVec3f up;
    MR::calcUpVec(&up, this);
    mItem->mPosition.scaleAdd(30.0f, up, mItem->mPosition);
    MR::sendArbitraryMsg(0x8C /* ACTMES_ITEM_END_MOVE */, mItem->getSensor(nullptr), getSensor("Body"));
    mIsItemBound = false;
}

void Shellfish::putItem() {
    const Vec* pOffset;

    if (mItemType == 0) {
        pOffset = &::sCoinOffset;
    } else if (mItemType == 1) {
        pOffset = &::sChipOffset;
    } else if (mItemType == 2) {
        pOffset = &::sChipOffset;
    } else if (mItemType == 3) {
        pOffset = &::sKinokoOneUpOffset;
    }

    PSMTXMultVec(getBaseMtx(), pOffset, &mItem->mPosition);
}

Shellfish::~Shellfish() {
}
