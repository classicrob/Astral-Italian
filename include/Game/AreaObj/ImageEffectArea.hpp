#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class ImageEffectArea : public AreaObj {
public:
    enum EImageEffectType {
        /* 0x00 */ IMAGE_EFFECT_TYPE_BLOOM,
        /* 0x01 */ IMAGE_EFFECT_TYPE_BLUR,
        /* 0x02 */ IMAGE_EFFECT_TYPE_DOF,
        /* 0x03 */ IMAGE_EFFECT_TYPE_POST_FOG,
        /* 0x04 */ IMAGE_EFFECT_TYPE_POST_OUTLINE,
    };

    ImageEffectArea(int, const char*);

    virtual void init(const JMapInfoIter&);

    virtual bool isSyncWithPlayer() const {
        return false;
    }

    /* 0x3C */ s32 mEffectType;
};

class ImageEffectAreaMgr : public AreaObjMgr {
public:
    ImageEffectAreaMgr(s32, const char*);

    virtual void initAfterPlacement();

    void sort();
};
