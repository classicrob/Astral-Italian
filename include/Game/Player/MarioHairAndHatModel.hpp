#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class BckCtrlData;

/// @brief Mario's hair and cap model.
class MarioHairAndHatModel : public LiveActor {
public:
    MarioHairAndHatModel();
    virtual ~MarioHairAndHatModel();
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    void exeNoCap();
    void exeAction();
    void exeWait();

    // SMG2-only (original names unknown; named by address)
    void fn_803EC760(MtxPtr);
    void fn_803EC770(MtxPtr);
    void fn_803EC780();
    bool fn_803EC7F0(const char*);
    bool fn_803EC8A0(const char*, const BckCtrlData&);

    /* 0x90 */ MtxPtr _90;
    /* 0x94 */ MtxPtr _94;
    /* 0x98 */ MtxPtr _98;
    /* 0x9C */ bool _9C;
    /* 0x9D */ bool _9D;
};
