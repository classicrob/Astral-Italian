#include "Game/MapObj/ItemRoomDoor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "math_types.hpp"
#include <revolution/types.h>


namespace {
    const char* sEnterDemoName = "アイテム部屋への入室";
    const char* sLeaveDemoName = "アイテム部屋からの退室";
    const char* sPlanetChangeDemoName = "マリオ顔惑星変化";
}


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
    MR::tryRegisterDemoCast(this, sEnterDemoName, rIter);
    if (MR::tryRegisterDemoCast(this, sLeaveDemoName, rIter)) {
        MR::tryRegisterDemoActionNerveDirect(this, &NrvItemRoomDoor::ItemRoomDoorNrvOpen::sInstance, sLeaveDemoName, nullptr);
    }
    if (MR::tryRegisterDemoCast(this, sPlanetChangeDemoName, rIter)) {
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
    // Shift-JIS "アイテム部屋への扉開" block. The +N offsets below are not names inside this literal: they
    // select the strings that the target pools right after it, so the substrings must stay one literal.
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
        if (MR::tryStartTimeKeepDemoMarioPuppetableWithoutCinemaFrame(this, sEnterDemoName)) {
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

    if (!MR::isDemoActive(sEnterDemoName)) {
        MR::setPlayerStateWait();
        MR::validateClipping(this);
        setNerve(&NrvItemRoomDoor::ItemRoomDoorNrvOpenWait::sInstance);
    }
}
