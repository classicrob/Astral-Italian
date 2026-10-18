#include "Game/MapObj/MemoryRoadDoor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/RhythmLib/AudRhythmTiming.hpp"
#include "Game/Util.hpp"
namespace NrvMemoryRoadDoor {
NEW_NERVE(MemoryRoadDoorNrvOpen, MemoryRoadDoor, Open);
NEW_NERVE(MemoryRoadDoorNrvClose, MemoryRoadDoor, Close);
}
MemoryRoadDoor::MemoryRoadDoor(const char* name) : LiveActor(name) {}
void MemoryRoadDoor::init(const JMapInfoIter& iter) { MR::initActor(this, iter, false); initNerve(&NrvMemoryRoadDoor::MemoryRoadDoorNrvOpen::sInstance); makeActorAppeared(); MR::invalidateCollisionParts(this); }
void MemoryRoadDoor::exeOpen() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Appear");
    tryFlashAnim();
    if (MR::isValidSwitchA(this) && MR::isOnSwitchA(this)) setNerve(&NrvMemoryRoadDoor::MemoryRoadDoorNrvClose::sInstance);
}
void MemoryRoadDoor::exeClose() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Move");
    tryFlashAnim();
    if (MR::isBckStopped(this)) MR::validateCollisionParts(this);
}
bool MemoryRoadDoor::tryFlashAnim() { if (!AudRhythmWrap::isBeatTiming(8, 7.94f)) return false; MR::startAction(this, "Appear"); return true; }
MemoryRoadDoor::~MemoryRoadDoor() {}
