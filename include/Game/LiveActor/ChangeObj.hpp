#pragma once

#include "Game/NameObj/NameObj.hpp"

class LiveActor;

class ChangeObj {
public:
    virtual ~ChangeObj();

    void changeTRSVGFromOther(const ChangeObj*);

    /* 0x04 */ LiveActor* mActor;
};
