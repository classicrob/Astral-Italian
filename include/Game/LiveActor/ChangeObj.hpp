#pragma once

#include "Game/NameObj/NameObj.hpp"

class LiveActor;

class ChangeObj {
public:
    ChangeObj(LiveActor* pActor) : mActor(pActor) {}

    virtual void doChange(ChangeObj*) = 0;

    void changeTRSVGFromOther(const ChangeObj*);

    /* 0x04 */ LiveActor* mActor;
};
