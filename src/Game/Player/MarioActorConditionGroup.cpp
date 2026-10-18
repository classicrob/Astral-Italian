#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/StringUtil.hpp"

MarioActorCondition1::MarioActorCondition1(MarioActor* actor) : MarioActorCondition(actor) {}
bool MarioActorCondition1::canGet() { return mActor->fn_803CD000(); }
bool MarioActorCondition1::result() { mActor->_38C = 2; return false; }

MarioActorCondition2::MarioActorCondition2(MarioActor* actor) : MarioActorCondition(actor) {}
bool MarioActorCondition2::canGet() { return mActor->fn_803CD0A0(); }
void MarioActorCondition2::execute() { mActor->fn_803CD450(mSensor); }
bool MarioActorCondition2::result() { mActor->fn_803CCE60(mSensor, 40); return true; }

MarioActorCondition3::MarioActorCondition3(MarioActor* actor) : MarioActorCondition(actor) {}
bool MarioActorCondition3::canGet() { return mActor->fn_803CD0A0(); }
void MarioActorCondition3::execute() { mActor->fn_803CD450(mSensor); }
bool MarioActorCondition3::result() { mActor->fn_803CCE60(mSensor, 25); return true; }

MarioActorCondition4::MarioActorCondition4(MarioActor* actor) : MarioActorCondition(actor) {}
bool MarioActorCondition4::canGet() { return mActor->fn_803CD2C0(); }
void MarioActorCondition4::execute() { mActor->fn_803CD450(mSensor); }
bool MarioActorCondition4::result() { mActor->fn_803CCE60(mSensor, 25); return true; }

MarioActorCondition5::MarioActorCondition5(MarioActor* actor) : MarioActorCondition(actor) {}
bool MarioActorCondition5::canGet() { return mActor->fn_803CD380(); }
bool MarioActorCondition5::result() { mActor->fn_803CCF20(mSensor); return true; }

MarioActorCondition6::MarioActorCondition6(MarioActor* actor) : MarioActorCondition(actor) {}
bool MarioActorCondition6::canGet() { return mActor->fn_803CD380(); }
bool MarioActorCondition6::result() { mActor->fn_803CCF90(mSensor); return true; }

MarioActorCondition7::MarioActorCondition7(MarioActor* actor) : MarioActorCondition(actor) {}
bool MarioActorCondition7::canGet() { return mActor->fn_803CD3C0(); }
void MarioActorCondition7::execute() { mActor->fn_803CD450(mSensor); }
bool MarioActorCondition7::result() { mActor->fn_803CCE60(mSensor, 40); return true; }

MarioActorConditionGroup::MarioActorConditionGroup(MarioActor* actor) {
    mActor = actor;
    mConditions[0] = new MarioActorCondition(actor);
    mConditions[1] = new MarioActorCondition1(actor);
    mConditions[2] = new MarioActorCondition2(actor);
    mConditions[3] = new MarioActorCondition3(actor);
    mConditions[4] = new MarioActorCondition4(actor);
    mConditions[5] = new MarioActorCondition5(actor);
    mConditions[6] = new MarioActorCondition6(actor);
    mConditions[7] = new MarioActorCondition7(actor);
}

bool MarioActorConditionGroup::tryGetItem(HitSensor* sensor) {
    MarioActorCondition* condition = getCondition(sensor);
    condition->mSensor = sensor;
    if (!condition->canGet()) return false;
    if (!sensor->receiveMessage(0x83, mActor->getSensor("dummy"))) return false;
    condition->execute();
    return condition->result();
}
void MarioActorCondition::execute() {}
bool MarioActorCondition::result() { return true; }
MarioActorCondition* MarioActorConditionGroup::getCondition(HitSensor* sensor) {
    if (mActor->fn_803CEEE0(sensor)) return mConditions[7];
    if (MR::getSensorHost(sensor) && MR::isEqualString(MR::getSensorHost(sensor)->mName, "スピン雲アイテム")) return mConditions[6];
    u32 type = sensor->mType;
    switch (type) {
    case 0x26: return mConditions[1];
    case 0x1D: return mConditions[2];
    case 0x12:
    case 0x13: return mConditions[3];
    case 0x48: return mConditions[4];
    case 0x4A: return mConditions[5];
    default: return mConditions[0];
    }
}
bool MarioActorCondition::canGet() { return true; }
