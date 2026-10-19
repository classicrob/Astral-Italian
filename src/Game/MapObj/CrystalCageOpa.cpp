#include "Game/MapObj/CrystalCage.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/MapObj/CrystalCageOpa.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "math_types.hpp"




namespace NrvCrystalCageOpa {
    NEW_NERVE(CrystalCageOpaNrvWait, CrystalCageOpa, Wait);
    NEW_NERVE(CrystalCageOpaNrvBreak, CrystalCageOpa, Break);
    NEW_NERVE(CrystalCageOpaNrvBreakAfter, CrystalCageOpa, BreakAfter);
};  // namespace NrvCrystalCageOpa

namespace NrvCrystalCage {
    class CrystalCageNrvFall : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
    };
};  // namespace NrvCrystalCage

void NrvCrystalCage::CrystalCageNrvFall::execute(Spine* pSpine) const {
    static_cast<CrystalCage*>(pSpine->mExecutor)->exeFall();
}

CrystalCageOpa::CrystalCageOpa(const char* pName)
    : LiveActor(pName), mWaitTime(1000), mIsStartAnim(false), mIsHideModel(false), mObjName(nullptr), mItemType(-1), mStarPieceNum(5) {}

void CrystalCageOpa::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    initNerve(&NrvCrystalCageOpa::CrystalCageOpaNrvWait::sInstance, 0);
    MR::getObjectName(&mObjName, rIter);

    if (MR::startBvaIfExist(this, "Break")) {
        MR::setBvaFrameAndStop(this, 0.0f);
    }

    MR::initUseStageSwitchWriteDead(this, rIter);
    mBreakModel = MR::tryCreateBreakModel(this, nullptr);
    MR::registerDemoSimpleCastAll(mBreakModel);
    MR::registerDemoSimpleCastAll(this);

    MR::getJMapInfoArg0NoInit(rIter, &mIsStartAnim);
    if (mIsStartAnim) {
        MR::getJMapInfoArg1NoInit(rIter, &mWaitTime);
    }
    MR::getJMapInfoArg2NoInit(rIter, &mItemType);
    MR::getJMapInfoArg3NoInit(rIter, &mStarPieceNum);

    if (mItemType == 0) {
    } else if (mItemType == 1) {
        MR::declareCoin(this, 1);
    } else {
        MR::declareStarPiece(this, mStarPieceNum);
    }

    MR::getJMapInfoArg4NoInit(rIter, &mIsHideModel);
    makeActorAppeared();
}

bool CrystalCageOpa::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(&NrvCrystalCageOpa::CrystalCageOpaNrvWait::sInstance)) {
        return false;
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) || MR::isMsgJetTurtleAttack(msg) || MR::isMsgRockAttack(msg) || MR::isMsgYoshiProjectile(msg)) {
        MR::invalidateClipping(this);
        MR::invalidateCollisionParts(this);
        MR::invalidateHitSensors(this);
        setNerve(&NrvCrystalCageOpa::CrystalCageOpaNrvBreak::sInstance);
        return true;
    }

    return false;
}

bool CrystalCageOpa::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(&NrvCrystalCageOpa::CrystalCageOpaNrvWait::sInstance)) {
        return false;
    }

    MR::invalidateClipping(this);
    MR::invalidateCollisionParts(this);
    MR::invalidateHitSensors(this);
    setNerve(&NrvCrystalCageOpa::CrystalCageOpaNrvBreak::sInstance);
    return true;
}

void CrystalCageOpa::exeWait() {
    if (MR::isFirstStep(this) && mIsStartAnim) {
        MR::tryStartAllAnim(this, "Move");
    }

    MR::attachSupportTicoToTarget(this);
}

// NONMATCHING: target keeps the "Break" string base in a separate addi (addi r4,r31,0 with reloc); we emit mr r4,r31 (one instruction, 98.2%)
void CrystalCageOpa::exeBreak() {
    if (MR::isFirstStep(this)) {
        mBreakModel->appear();

        if (mIsStartAnim) {
            if (MR::fn_80008F00(this, "Move")) {
                MR::setBckFrameAndStop(this, 0.0f);
            }
        }

        MR::startAction(mBreakModel, "Break");

        if (mIsHideModel || !MR::isExistBva(this, "Break")) {
            MR::hideModel(this);
        } else {
            MR::setBvaFrameAndStop(this, 1.0f);
        }

        appearItem();

        if (MR::isValidSwitchDead(this)) {
            MR::onSwitchDead(this);
        }
    }

    if (!MR::isEqualString(mObjName, "CrystalCageOpaLarge")) {
        if (MR::isStep(this, 2)) {
            MR::stopScene(3);
        }
    }

    if (MR::isActionEnd(mBreakModel)) {
        setNerve(&NrvCrystalCageOpa::CrystalCageOpaNrvBreakAfter::sInstance);
    }
}

void CrystalCageOpa::exeBreakAfter() {
    if (MR::isFirstStep(this)) {
        mBreakModel->kill();
        MR::validateClipping(this);
    }
}

void CrystalCageOpa::appearItem() {
    if (mItemType == 0) {
        return;
    }

    if (mItemType == 1) {
        MR::appearCoinPop(this, mPosition, 1);
    } else {
        MR::appearStarPiece(this, mPosition, mStarPieceNum, 10.0f, 40.0f, false);
    }
}

CrystalCageOpa::~CrystalCageOpa() {}
