#pragma once

#include "Game/NameObj/NameObj.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/Array.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class CollisionPartsFilterBase;
class HitSensor;
class LiveActor;
class ShadowController;
class ShadowDrawer;

class ShadowControllerList {
public:
    ShadowControllerList(LiveActor*, u32);

    void addController(ShadowController*);
    u32 getControllerCount() const;
    ShadowController* getController(u32) const;
    ShadowController* getController(const char*) const;
    void resetCalcCount();
    void requestCalc();

    /* 0x00 */ MR::Vector< MR::AssignableArray< ShadowController* > > mShadowList;
    /* 0x0C */ LiveActor* mHost;
};

class ShadowControllerHolder : public NameObj {
public:
    ShadowControllerHolder();

    virtual ~ShadowControllerHolder();
    virtual void initAfterPlacement() override;
    virtual void movement() override;

    void updateFarClipParam();
    void updateController();

    /* 0x14 */ MR::Vector< MR::AssignableArray< ShadowController* > > _C;
    /* 0x20 */ MR::Vector< MR::AssignableArray< ShadowController* > > _18;
    /* 0x2C */ f32 mFarClip;
    /* 0x30 */ f32 mDefaultFarClip;
    /* 0x34 */ bool mIsFarClipEnabled;
    /* 0x35 */ bool mIsDefaultFarClipEnabled;
};

class ShadowController {
public:
    ShadowController(LiveActor*, const char*);

    void requestCalc();
    void update();
    void updateFarClipping(f32);
    void updateDirection();
    ShadowDrawer* getShadowDrawer();
    LiveActor* getHost() const;
    void setShadowDrawer(ShadowDrawer*);
    void getDropPos(TVec3f*) const;
    void getDropDir(TVec3f*) const;
    f32 getDropLength() const;
    void getProjectionPos(TVec3f*) const;
    void getProjectionNormal(TVec3f*) const;
    f32 getProjectionLength() const;
    bool isProjected() const;
    bool isDraw() const;
    bool isCalcCollision() const;
    bool isCalcGravity() const;
    bool isCalcShadowGravity() const NO_INLINE;
    void setGroupName(const char*);
    void setDropPosPtr(const TVec3f*);
    void setDropPosMtxPtr(MtxPtr, const TVec3f&);
    void setLocalRotate(const TVec3f&);
    void setDropPosFix(const TVec3f&);
    void setDropDirPtr(const TVec3f*);
    void setProjectionPtr(const TVec3f*, const TVec3f*);
    void setDropDirFix(const TVec3f&);
    void setDropLength(f32);
    void setDropStartOffset(f32);
    void setDropTypeNormal();
    void setDropTypeSurface();
    void setProjectionFix(const TVec3f&, const TVec3f&, bool);
    void onCalcCollision();
    void offCalcCollision();
    void onCalcCollisionOneTime();
    void onCalcDropGravity();
    void onCalcDropGravityOneTime();
    void offCalcDropGravity();
    void onCalcDropPrivateGravity();
    void onCalcDropPrivateGravityOneTime();
    void offCalcDropPrivateGravity();
    void setCollisionPartsFilter(CollisionPartsFilterBase*);
    void onFollowHostScale();
    void offFollowHostScale();
    bool isFollowHostScale() const;
    void onVisibleSyncHost();
    void offVisibleSyncHost();
    void validate();
    void invalidate();

    void calcBaseMtx(TPos3f*) const;
    void updateProjection();

    inline void appendToHolder() {
        MR::getSceneObj< ShadowControllerHolder >(SceneObj_ShadowControllerHolder)->_18.push_back(this);
    }

    /* 0x00 */ LiveActor* mActor;
    /* 0x04 */ const char* mName;
    /* 0x08 */ const char* mGroupName;
    /* 0x0C */ ShadowDrawer* mDrawer;
    /* 0x10 */ HitSensor* mProjectedSensor;
    /* 0x14 */ CollisionPartsFilterBase* mCollisionPartsFilter;
    /* 0x18 */ MtxPtr _18;
    /* 0x1C */ MtxPtr _1C;
    /* 0x20 */ const TVec3f* mDropPos;
    /* 0x24 */ const TVec3f* mDropDir;
    /* 0x28 */ const TVec3f* mProjPos;
    /* 0x2C */ const TVec3f* mProjNorm;
    /* 0x30 */ TVec3f _30;
    /* 0x3C */ TVec3f _3C;
    /* 0x48 */ TVec3f _48;
    /* 0x54 */ TVec3f _54;
    /* 0x60 */ TVec3f _60Vec;  // SMG2
    /* 0x6C */ u8 _60;
    /* 0x6D */ u8 _61;
    /* 0x6E */ u8 _62;
    /* 0x6F */ u8 _63;
    /* 0x70 */ u8 _64;
    /* 0x71 */ u8 _65;
    /* 0x72 */ u8 _66;
    /* 0x73 */ u8 _67;
    /* 0x74 */ f32 mStartOffset;
    /* 0x78 */ f32 mDropLength;
    /* 0x7C */ u8 _70;
    /* 0x7D */ u8 _71;
    /* 0x7E */ u8 _72;
};

namespace MR {
    void addShadowController(ShadowController*) NO_INLINE;
    void requestCalcActorShadowAppear(LiveActor*);
    void requestCalcActorShadow(LiveActor*);
};  // namespace MR
