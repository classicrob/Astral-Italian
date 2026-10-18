#include "Game/MapObj/ItemRoomDoor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Functor.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace MR {
    bool getJMapInfoArg0NoInit(const JMapInfoIter&, bool*);
    bool isPlayerInRange(f32, const TVec3f&);
    void startSoundInPlayerDistance(LiveActor*, const char*, s32, s32, f32);
};  // namespace MR

extern "C" bool fn_8001FDD0(const LiveActor*, const char*);



namespace NrvItemRoomDoor {
    NEW_NERVE(ItemRoomDoorNrvCloseWait, ItemRoomDoor, CloseWait);
    NEW_NERVE(ItemRoomDoorNrvOpen, ItemRoomDoor, Open);
    NEW_NERVE(ItemRoomDoorNrvOpenWait, ItemRoomDoor, OpenWait);
    NEW_NERVE(ItemRoomDoorNrvClose, ItemRoomDoor, Close);
    NEW_NERVE(ItemRoomDoorNrvDemoPlayerWaitOpen, ItemRoomDoor, DemoPlayerWait);
    NEW_NERVE(ItemRoomDoorNrvDemoPlayerWaitClose, ItemRoomDoor, DemoPlayerWait);
};  // namespace NrvItemRoomDoor

ItemRoomDoor::ItemRoomDoor(const char* pName) : LiveActor(pName) {
    mIsAutoOpen = true;
}

ItemRoomDoor::~ItemRoomDoor() {}

void ItemRoomDoor::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    initNerve(&NrvItemRoomDoor::ItemRoomDoorNrvCloseWait::sInstance, 0);

    bool arg0 = false;
    MR::getJMapInfoArg0NoInit(rIter, &arg0);
    mIsAutoOpen = !arg0;

    MR::useStageSwitchSyncAppear(this, rIter);
    MR::tryRegisterDemoCast(this, "\200j6h\200j6\200\200j6\230", rIter);
    if (MR::tryRegisterDemoCast(this, "\200j6\200\200j6\230", rIter)) {
        MR::tryRegisterDemoActionFunctorDirect(this, reinterpret_cast<const MR::FunctorBase&>(NrvItemRoomDoor::ItemRoomDoorNrvOpen::sInstance), "\200j6\200\200j6\230", nullptr);
    }
    if (MR::tryRegisterDemoCast(this, "\200j6\230", rIter)) {
        makeActorDead();
    }
}

void ItemRoomDoor::exeCloseWait() {
    if (mIsAutoOpen) {
        f32 radius = MR::getSensorRadius(this, "Open");
        if (MR::isPlayerInRange(radius, *MR::getSensorPos(this, "Open"))) {
            setNerve(&NrvItemRoomDoor::ItemRoomDoorNrvOpenWait::sInstance);
        }
    }
}

void ItemRoomDoor::exeOpen() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "ItemRoomDoorOpen");
        MR::setAllAnimFrameAtEnd(this, "ItemRoomDoorOpen");
        MR::invalidateCollisionParts(this);
    }

    if (MR::isDemoLastStep()) {
        setNerve(&NrvItemRoomDoor::ItemRoomDoorNrvClose::sInstance);
    } else {
        MR::startSoundInPlayerDistance(this, "InnerSoundEmitting", -1, -1, 10000.0f);
    }
}

void ItemRoomDoor::exeOpenWait() {
    const char* pBase = "\203A\203C\203e\203\200\225\224\211\256\202\326\202\314\223\374\216\272";
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, pBase + 0x4c);
        MR::invalidateCollisionParts(this);
    }

    f32 dist = MR::calcDistanceToPlayer(*MR::getSensorPos(this, pBase + 0x44));
    bool isFar = dist >= 450.0f;
    if (isFar) {
        setNerve(&NrvItemRoomDoor::ItemRoomDoorNrvClose::sInstance);
        return;
    }

    f32 radius = MR::getSensorRadius(this, pBase + 0x74);
    if (MR::isPlayerInRange(radius, *MR::getSensorPos(this, pBase + 0x74))) {
        if (fn_8001FDD0(this, "\200j6h\200j6\200\200j6\230")) {
            setNerve(&NrvItemRoomDoor::ItemRoomDoorNrvDemoPlayerWaitOpen::sInstance);
        }
    } else {
        MR::startSoundInPlayerDistance(this, pBase + 0x60, -1, -1, 10000.0f);
    }
}

void ItemRoomDoor::exeClose() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "ItemRoomDoorClose");
    }

    if (MR::isActionEnd(this)) {
        setNerve(&NrvItemRoomDoor::ItemRoomDoorNrvCloseWait::sInstance);
        MR::validateCollisionParts(this);
    }
}

void ItemRoomDoor::exeDemoPlayerWait() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        return;
    }

    if (!MR::isDemoActive("\200j6h\200j6\200\200j6\230")) {
        MR::setPlayerStateWait();
        MR::validateClipping(this);
        setNerve(&NrvItemRoomDoor::ItemRoomDoorNrvOpenWait::sInstance);
    }
}
