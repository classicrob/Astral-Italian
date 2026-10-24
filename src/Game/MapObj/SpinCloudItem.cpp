#pragma auto_inline off
#include "Game/MapObj/SpinCloudItem.hpp"
#include "Game/MapObj/SpinCloudBlock.hpp"
#include "Game/MapObj/ActorStateDemoAppearance.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/NPCUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
namespace NrvSpinCloudItem {
    class SpinCloudItemNrvAppearDemo : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static SpinCloudItemNrvAppearDemo sInstance;
    };
    SpinCloudItemNrvAppearDemo SpinCloudItemNrvAppearDemo::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(SpinCloudItemNrvAppear, SpinCloudItem, Appear);
    NEW_NERVE(SpinCloudItemNrvWait, SpinCloudItem, Wait);
    NEW_NERVE(SpinCloudItemNrvHide, SpinCloudItem, Hide);
}
SpinCloudItem::SpinCloudItem(const char* name) : LiveActor(name) { MR::createSpinCloudBlockGroup(); }
SpinCloudItem::~SpinCloudItem() {}
void SpinCloudItem::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, "PowerUpCloud", false);
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f gravity(-up);
    mGravity.x = gravity.x; mGravity.y = gravity.y; mGravity.z = gravity.z;
    initNerve(&NrvSpinCloudItem::SpinCloudItemNrvWait::sInstance, 4);
    ActorStateDemoAppearance* state = new ActorStateDemoAppearance(this, iter, "Appear");
    state->mIsWaitActionEnd = true;
    MR::initActorState(this, state, &NrvSpinCloudItem::SpinCloudItemNrvAppearDemo::sInstance, "AppearFirst");
    if (MR::useStageSwitchSyncAppear(this, iter)) {
        setNerve(&NrvSpinCloudItem::SpinCloudItemNrvAppearDemo::sInstance);
        MR::invalidateClipping(this);
    }
    MR::registerMarioFaceShipNpc(this, iter);
}
void NrvSpinCloudItem::SpinCloudItemNrvAppearDemo::execute(Spine* spine) const {
    SpinCloudItem* actor = static_cast<SpinCloudItem*>(spine->mExecutor);
    if (MR::isFirstStep(actor)) MR::startActionSound(actor, "OjMorphFlowerAppear");
    MR::updateActorStateAndNextNerve(actor, &NrvSpinCloudItem::SpinCloudItemNrvWait::sInstance);
}
void SpinCloudItem::control() {}
bool SpinCloudItem::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgItemGet(msg)) return updatePlayerPowerUp();
    return false;
}
bool SpinCloudItem::updatePlayerPowerUp() {
    if (isNerve(&NrvSpinCloudItem::SpinCloudItemNrvWait::sInstance)) {
        MR::hideModelAndOnCalcAnim(this);
        MR::invalidateClipping(this);
        MR::changePlayerModeCloud();
        setNerve(&NrvSpinCloudItem::SpinCloudItemNrvHide::sInstance);
        return true;
    }
    return false;
}
void SpinCloudItem::enablePlayerResourceCloud() { MR::enablePlayerResourceCloud(); }
void SpinCloudItem::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
        MR::showModel(this);
    }
    if (MR::isActionEnd(this)) setNerve(&NrvSpinCloudItem::SpinCloudItemNrvWait::sInstance);
}
void SpinCloudItem::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::showModel(this);
        MR::validateClipping(this);
    }
}
void SpinCloudItem::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        if (MR::isPlayerElementModeCloud()) {
            if (MR::spawnSpinCloudBlocks()) MR::startActionSound(this, "GetFilled");
            else MR::startActionSound(this, "GetInvalid");
        }
    }
    if (MR::isGreaterStep(this, 180)) setNerve(&NrvSpinCloudItem::SpinCloudItemNrvAppear::sInstance);
}
