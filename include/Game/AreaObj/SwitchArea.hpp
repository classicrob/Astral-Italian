#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class SwitchArea : public AreaObj {
public:
    SwitchArea(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void movement();

    virtual const char* getManagerName() const;

    bool isUpdate() const;
    bool checkPlayerDrillRush() const;
    void onSwitchA();

    /* 0x48 */ s32 mSoundType;
    /* 0x4C */ bool mIsSoundPlayed;
};
