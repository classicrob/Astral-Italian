#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ItemRoomDoor : public LiveActor {
public:
    ItemRoomDoor(const char*);

    virtual ~ItemRoomDoor();
    virtual void init(const JMapInfoIter&) override;
    void exeCloseWait();
    void exeOpen();
    void exeOpenWait();
    void exeClose();
    void exeDemoPlayerWait();

    /* 0x90 */ bool mIsAutoOpen;  // !JMap arg0 (see init)
};
