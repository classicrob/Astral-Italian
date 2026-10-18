#include "Game/Util/BellyDragon.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

// Nerve execute bodies are inlined into their NEW_NERVE execute (defined before the classes); the others are thunks.
inline void BellyDragonWeakPoint::exeWaitC() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
}

inline void BellyDragonWeakPoint::exeChance() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Chance");
    }
}

namespace NrvBellyDragonWeakPoint {
    NEW_NERVE(BellyDragonWeakPointNrvWait, BellyDragonWeakPoint, Wait);
    NEW_NERVE(BellyDragonWeakPointNrvBurn, BellyDragonWeakPoint, Burn);
    NEW_NERVE(BellyDragonWeakPointNrvChance, BellyDragonWeakPoint, Chance);
    NEW_NERVE(BellyDragonWeakPointNrvWaitC, BellyDragonWeakPoint, WaitC);

    class BellyDragonWeakPointNrvDamage : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }
        static BellyDragonWeakPointNrvDamage sInstance;
    };
    BellyDragonWeakPointNrvDamage BellyDragonWeakPointNrvDamage::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvBellyDragonWeakPoint

// PLACEHOLDER(core): weak-point nerve instances are named by their vtable/setNerve use (lbl_807D0F60..70, lbl_8066D618..658); resolve when the BellyDragon nerve names are confirmed in symbols.txt
BellyDragonWeakPoint::BellyDragonWeakPoint(HitSensor* pSensor, BellyDragonBodyPart* pPart) : LiveActor("胴体（ドラゴン）") {
    mSensor = pSensor;
    mPart = pPart;
    mMtx.identity();
    updateMtx();
}

void BellyDragonWeakPoint::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "BellyDragonWeakPoint", false);
    initNerve(&NrvBellyDragonWeakPoint::BellyDragonWeakPointNrvWait::sInstance, 0);
    makeActorDead();
}

void BellyDragonWeakPoint::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    mPart->mWeakPoint = this;
    setNerve(&NrvBellyDragonWeakPoint::BellyDragonWeakPointNrvWait::sInstance);
}

void BellyDragonWeakPoint::setNerveChance() {
    setNerve(&NrvBellyDragonWeakPoint::BellyDragonWeakPointNrvChance::sInstance);
}

void BellyDragonWeakPoint::setNerveWaitA() {
    setNerve(&NrvBellyDragonWeakPoint::BellyDragonWeakPointNrvWait::sInstance);
}

void BellyDragonWeakPoint::setNerveBurn() {
    setNerve(&NrvBellyDragonWeakPoint::BellyDragonWeakPointNrvBurn::sInstance);
}

void BellyDragonWeakPoint::setNerveWaitB() {
    setNerve(&NrvBellyDragonWeakPoint::BellyDragonWeakPointNrvWait::sInstance);
}

void BellyDragonWeakPoint::setNerveWaitC() {
    setNerve(&NrvBellyDragonWeakPoint::BellyDragonWeakPointNrvWaitC::sInstance);
}

bool BellyDragonWeakPoint::isAttackable() {
    if (MR::isDead(this) || isNerve(&NrvBellyDragonWeakPoint::BellyDragonWeakPointNrvDamage::sInstance)) {
        return false;
    }
    return true;
}

void BellyDragonWeakPoint::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
    }
    MR::startActionSound(this, "Fly", -1, -1, -1);
    if (mPart->mIsLv2) {
        MR::startActionSound(this, "Burn", -1, -1, -1);
    }
}

void BellyDragonWeakPoint::exeBurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
    MR::startActionSound(this, "Fly", -1, -1, -1);
    if (mPart->mIsLv2) {
        MR::startActionSound(this, "Burn", -1, -1, -1);
    }
}

// NONMATCHING: target copies the base matrix with inline paired-single loads and calls an out-of-line TVec3::set; ours uses gekko_ps_copy12 loop and inlines set
void BellyDragonWeakPoint::updateMtx() {
    TPos3f* pBaseMtx = (TPos3f*)mPart->getBaseMtx();
    mMtx = *pBaseMtx;
    mPosition.set(mMtx.mMtx[0][3], mMtx.mMtx[1][3], mMtx.mMtx[2][3]);
}

void BellyDragonWeakPoint::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mMtx);
}

void BellyDragonWeakPoint::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    MR::sendMsgPush(pReceiver, pSender);
}

bool BellyDragonWeakPoint::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    return mPart->receiveMsgPush(pSender, pReceiver);
}

bool BellyDragonWeakPoint::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && isAttackable() && MR::sendArbitraryMsg(msg, mSensor, pReceiver)) {
        MR::tryHitReactionBetweenSensors(this, "胴体スピンヒット", pSender, pReceiver);
        mPart->mWeakPoint = NULL;
        MR::hideModel(this);
        MR::invalidateHitSensors(this);
        setNerve(&NrvBellyDragonWeakPoint::BellyDragonWeakPointNrvDamage::sInstance);
        return true;
    }
    return false;
}

BellyDragonWeakPoint::~BellyDragonWeakPoint() {
}
