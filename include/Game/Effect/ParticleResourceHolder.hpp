#pragma once

#include <revolution/types.h>

class JMapInfo;
class JPAResourceManager;
struct ResTIMG;

class ParticleResourceHolder {
public:
    ParticleResourceHolder(const char* pArchiveName);

    u16 getUserIndex(const char* pName) const;
    void swapTexture(const ResTIMG* pImage, const char* pName);
    bool isExistInResource(const char* pName, u16* pIndex) const;
    JMapInfo* getAutoEffectListBinary() const;

    /* 0x00 */ JPAResourceManager* mResourceMgr;
    /* 0x04 */ JMapInfo* mAutoEffectList;
    /* 0x08 */ JMapInfo* mParticleNames;
};
