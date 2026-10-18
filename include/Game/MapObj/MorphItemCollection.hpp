#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class MorphItemCollection : public LiveActor {
public:
    MorphItemCollection(const char*);
    virtual ~MorphItemCollection();
    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();
    void exeWait();
    void initPowerUpModel(const JMapInfoIter&);
    s32 mPowerUp;
    f32 mAngle;
    bool mIsDrill;
};
