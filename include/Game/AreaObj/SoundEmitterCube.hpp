#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class ActionSoundKeeper;

class SoundEmitterCube : public AreaObj {
public:
    SoundEmitterCube(int formType, const char* pName);

    virtual ~SoundEmitterCube();

    virtual void init(const JMapInfoIter& rIter);
    virtual void movement();

    /* 0x48 */ ActionSoundKeeper* mSoundObj;
    /* 0x4C */ TVec3f mSoundPos;
};

class SoundEmitterCubeWorldMapSync : public SoundEmitterCube {
public:
    SoundEmitterCubeWorldMapSync(int formType, const char* pName);

    virtual ~SoundEmitterCubeWorldMapSync();
    virtual void init(const JMapInfoIter& rIter);
};
