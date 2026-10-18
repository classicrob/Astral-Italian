#pragma once

#include "Game/NameObj/NameObj.hpp"

class ActorLightCtrl;
class AreaLightInfo;
class LightDataHolder;
class LightPointCtrl;
class LightZoneDataHolder;
class ResourceHolder;
class LightAreaHolder;

class LightDirector : public NameObj {
public:
    /// @brief Creates a new `LightDirector`.
    LightDirector();

    virtual void init(const JMapInfoIter&);
    virtual void movement();

    void initData();
    void loadLightPlayer() const;
    void loadPointLightSub() const;
    ResourceHolder* getZoneLightResourceHolder(const char* pZoneName) const;

    /* 0x14 */ LightAreaHolder* _C;
    /* 0x18 */ LightDataHolder* mDataHolder;
    /* 0x1C */ LightZoneDataHolder* mZoneDataHolder;
    /* 0x20 */ AreaLightInfo* mDefaultAreaLight;
    /* 0x24 */ const ActorLightCtrl* _1C;
    /* 0x28 */ LightPointCtrl* mPointCtrl;
    /* 0x2C */ LightPointCtrl* mPointCtrlSub;  // SMG2; MR::requestPointLight feeds both
    /* 0x30 */ ResourceHolder* mResourceHolder;
    /* 0x34 */ ResourceHolder** mZoneResourceHolders;  // SMG2; one per zone (LightFunction::loadZoneLightArchive)
};
