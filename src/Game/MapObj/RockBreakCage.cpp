#include "Game/MapObj/RockBreakCage.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvRockBreakCage {
    class RockBreakCageNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static RockBreakCageNrvWait sInstance;
    };
    RockBreakCageNrvWait RockBreakCageNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(RockBreakCageNrvDamage, RockBreakCage, Damage);
    class RockBreakCageNrvBreak : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            RockBreakCage* actor = static_cast<RockBreakCage*>(spine->mExecutor);
            if (MR::isGreaterEqualStep(actor,30)) actor->setNerve(&RockBreakCageNrvWait::sInstance);
        }
        static RockBreakCageNrvBreak sInstance;
    };
    RockBreakCageNrvBreak RockBreakCageNrvBreak::sInstance ATTRIBUTE_WEAK;
}
RockBreakCage::RockBreakCage(const char* name) : LiveActor(name), mHitPoints(2), mBreakModel(nullptr), mObjectName(nullptr) {}
RockBreakCage::~RockBreakCage() {}
void RockBreakCage::init(const JMapInfoIter& iter) {
    MR::initActor(this,iter,false);
    initNerve(&NrvRockBreakCage::RockBreakCageNrvWait::sInstance);
    MR::startBva(this,"RockBreakCage");
    MR::setBvaFrameAndStop(this,0.0f);
    MR::initUseStageSwitchWriteDead(this,iter);
    mBreakModel = MR::tryCreateBreakModel(this,nullptr);
    MR::getObjectName(&mObjectName,iter);
    makeActorAppeared();
}
bool RockBreakCage::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    if (!isNerve(&NrvRockBreakCage::RockBreakCageNrvWait::sInstance)) return false;
    if (MR::isMsgRockAttack(msg)) {
        if (--mHitPoints <= 0) {
            mBreakModel->appear();
            if (MR::isEqualString(mObjectName,"RockBreakCageL")) MR::setScale(mBreakModel,4.0f);
            else MR::setScale(mBreakModel,2.0f);
            MR::startActionSound(this,"Break",-1,-1,-1);
            setNerve(&NrvRockBreakCage::RockBreakCageNrvDamage::sInstance);
        } else {
            MR::startActionSound(this,"Crack",-1,-1,-1);
            MR::setBvaFrameAndStop(this,1.0f);
            setNerve(&NrvRockBreakCage::RockBreakCageNrvBreak::sInstance);
        }
        return true;
    }
    return false;
}
void RockBreakCage::kill() {
    if (MR::isValidSwitchDead(this)) MR::onSwitchDead(this);
    LiveActor::kill();
}
void RockBreakCage::exeDamage() {
    if (MR::isFirstStep(this)) {
        if (MR::isValidSwitchDead(this)) MR::onSwitchDead(this);
        MR::setBvaFrameAndStop(this,2.0f);
        MR::invalidateCollisionParts(this);
    }
}
