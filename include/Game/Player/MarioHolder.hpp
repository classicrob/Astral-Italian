#pragma once

#include "Game/NameObj/NameObj.hpp"

class MarioActor;

class MarioHolder : public NameObj {
public:
    MarioHolder();

    virtual ~MarioHolder();

    void setMarioActor(MarioActor* actor);
    MarioActor* getMarioActor() const;

    MarioActor* mActor;
    bool mBeeResourceEnabled;
    bool mHopperResourceEnabled;
    bool mFireResourceEnabled;
    bool mIceResourceEnabled;
    bool mInvincibleResourceEnabled;
    bool mTeresaResourceEnabled;
    bool mTornadoResourceEnabled;
    bool mUnknownResourceEnabled;
    bool mYoshiResourceEnabled;
    bool mYoshiBalloonResourceEnabled;
    bool mCloudResourceEnabled;
    bool mRockResourceEnabled;
    bool mHairAndHeadResourceEnabled;
    bool mFreezeIceResourceEnabled;

};

namespace MR {
    MarioHolder* getMarioHolder();
    void enablePlayerResourceBee() NO_INLINE;
    bool isCreatePlayerResourceBee();
    void enablePlayerResourceHopper() NO_INLINE;
    bool isCreatePlayerResourceHopper();
    void enablePlayerResourceFire() NO_INLINE;
    bool isCreatePlayerResourceFire();
    void enablePlayerResourceIce() NO_INLINE;
    bool isCreatePlayerResourceIce();
    void enablePlayerResourceInvincible() NO_INLINE;
    bool isCreatePlayerResourceInvincible();
    void enablePlayerResourceTeresa() NO_INLINE;
    bool isCreatePlayerResourceTeresa();
    void enablePlayerResourceTornado() NO_INLINE;
    bool isCreatePlayerResourceTornado();
    void enablePlayerResourceYoshi() NO_INLINE;
    bool isCreatePlayerResourceYoshi();
    void enablePlayerResourceYoshiBalloon() NO_INLINE;
    bool isCreatePlayerResourceYoshiBalloon();
    void enablePlayerResourceCloud() NO_INLINE;
    bool isCreatePlayerResourceCloud();
    void enablePlayerResourceRock() NO_INLINE;
    bool isCreatePlayerResourceRock();
    void enablePlayerResourceHairAndHead() NO_INLINE;
    bool isCreatePlayerResourceHairAndHead();
    void enablePlayerResourceFreezeIce() NO_INLINE;
    bool isCreatePlayerResourceFreezeIce();
    void enablePlayerResourceHopperComet();
    void enablePlayerResourceInvincibleComet();
    void enablePlayerResourceNoHat();

};  // namespace MR
