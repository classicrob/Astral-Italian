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

extern "C" const char* lbl_807CFCC0;
extern "C" const char* lbl_807CFCC4;
extern "C" const char* lbl_807CFCC8;

extern "C" const char lbl_806A3668[];
extern "C" const char lbl_806A36AC[];
extern "C" const char lbl_806A36B4[];
extern "C" const char lbl_806A36C8[];
extern "C" const char lbl_806A36E4[];

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
    MR::tryRegisterDemoCast(this, lbl_807CFCC0, rIter);
    if (MR::tryRegisterDemoCast(this, lbl_807CFCC4, rIter)) {
        MR::tryRegisterDemoActionFunctorDirect(this, reinterpret_cast<const MR::FunctorBase&>(NrvItemRoomDoor::ItemRoomDoorNrvOpen::sInstance), lbl_807CFCC4, nullptr);
    }
    if (MR::tryRegisterDemoCast(this, lbl_807CFCC8, rIter)) {
        makeActorDead();
    }
}

void ItemRoomDoor::exeCloseWait() {
    if (mIsAutoOpen) {
        f32 radius = MR::getSensorRadius(this, lbl_806A36AC);
        if (MR::isPlayerInRange(radius, *MR::getSensorPos(this, lbl_806A36AC))) {
            setNerve(&NrvItemRoomDoor::ItemRoomDoorNrvOpenWait::sInstance);
        }
    }
}

void ItemRoomDoor::exeOpen() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, lbl_806A36B4);
        MR::setAllAnimFrameAtEnd(this, lbl_806A36B4);
        MR::invalidateCollisionParts(this);
    }

    if (MR::isDemoLastStep()) {
        setNerve(&NrvItemRoomDoor::ItemRoomDoorNrvClose::sInstance);
    } else {
        MR::startSoundInPlayerDistance(this, lbl_806A36C8, -1, -1, 10000.0f);
    }
}

void ItemRoomDoor::exeOpenWait() {
    const char* pBase = lbl_806A3668;
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
        if (fn_8001FDD0(this, lbl_807CFCC0)) {
            setNerve(&NrvItemRoomDoor::ItemRoomDoorNrvDemoPlayerWaitOpen::sInstance);
        }
    } else {
        MR::startSoundInPlayerDistance(this, pBase + 0x60, -1, -1, 10000.0f);
    }
}

void ItemRoomDoor::exeClose() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, lbl_806A36E4);
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

    if (!MR::isDemoActive(lbl_807CFCC0)) {
        MR::setPlayerStateWait();
        MR::validateClipping(this);
        setNerve(&NrvItemRoomDoor::ItemRoomDoorNrvOpenWait::sInstance);
    }
}
