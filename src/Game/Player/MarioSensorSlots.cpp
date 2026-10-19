#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

MarioSensorSlots::MarioSensorSlots() {
    Slot* slot = mSlots;
    do {
        slot->mTimer = 0;
        slot->mActor = nullptr;
        ++slot;
    } while (slot < mSlots + 16);
    mCount = 0;
}

void MarioSensorSlots::fn_803DAB10(LiveActor* actor) {
    if (mCount == 16)
        return;
    for (s32 i = 0; i < 16; ++i) {
        Slot& slot = mSlots[i];
        if (!slot.mActor) {
            slot.mTimer = YoshiParam::fn_8041BDF0();
            slot.mActor = actor;
            ++mCount;
            break;
        }
    }
}

// NONMATCHING: loop index and slot pointer exchange r28/r29.
void MarioSensorSlots::fn_803DAB90() {
    for (s32 i = 0; i < 16; ++i) {
        Slot& slot = mSlots[i];
        if (slot.mActor && --slot.mTimer <= 0) {
            MR::appearItemYoshiEatToPlayer(slot.mActor);
            slot.mActor = nullptr;
            --mCount;
        }
    }
}
