#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class ActorCameraInfo;

class StrayTico;

class CollectTico : public LiveActor {
public:
    CollectTico(const char*);

    virtual void init(const JMapInfoIter&);

    void launch(const TPos3f&);
    void exeWait();
    void exeCompleteDemo();
    void exeFlash();
    void exeAppearPowerStar();
    inline void exeTryStartDemo();
    s32 calcNoRescuedCount() const;
    void startAppearPowerStar();

    /* 0x90 */ ActorCameraInfo* mCameraInfo;
    /* 0x94 */ StrayTico** mStrayTicos;
    /* 0x98 */ s32 mTicoNum;
    /* 0x9C */ TVec3f mDemoCenter;
    /* 0xA8 */ u8 mHasAppearDemo;
    bool mHasCamera;
    bool mUsePlacedPosition;
};
