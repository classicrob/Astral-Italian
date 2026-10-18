#pragma auto_inline off
#include "Game/MapObj/MemoryRoadStep.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/RhythmLib/AudRhythmTiming.hpp"
#include "Game/Util.hpp"
namespace NrvMemoryRoadStep {
NEW_NERVE(MemoryRoadStepNrvWait, MemoryRoadStep, Wait);
}
MemoryRoadStep::MemoryRoadStep(const char* name) : LiveActor(name) {}
void MemoryRoadStep::init(const JMapInfoIter& iter) {
    char name[256];
    MR::getMapPartsObjectNameIfExistShapeID(name, sizeof(name), iter);
    MR::initActor(this, iter, name, false);
    MR::connectToSceneMapObj(this);
    initSound(4, "MemoryRoadStep", 0, TVec3f(0.0f));
    MR::initDefaultPos(this, iter);
    MR::initMapPartsClipping(this, iter, 0, false);
    initHitSensor(1);
    MR::addBodyMessageSensorMapObj(this);
    MR::initCollisionParts(this, name, getSensor(0), 0);
    initNerve(&NrvMemoryRoadStep::MemoryRoadStepNrvWait::sInstance);
    makeActorAppeared();
}
void MemoryRoadStep::exeWait() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Appear");
    tryFlashAnim();
    if (AudRhythmWrap::isBeatTiming(8, 7.818f)) MR::startActionSound(this, "SyMemoryroadPrethunder");
}
bool MemoryRoadStep::tryFlashAnim() {
    if (!AudRhythmWrap::isBeatTiming(8, 7.94f)) return false;
    if (!MR::isActionEnd(this)) return false;
    MR::startAction(this, "Appear");
    MR::startActionSound(this, "SyMemoryroadThunder");
    return true;
}
MemoryRoadStep::~MemoryRoadStep() {}
