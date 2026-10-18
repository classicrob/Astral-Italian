#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JointController.hpp"

class DemoAstroPlanet : public LiveActor {
public:
    /// @brief Creates a new `DemoAstroPlanet`.
    /// @param pName A pointer to the null-terminated name of the object.
    /// @param pMtx The base matrix whose translation places the planet, or `NULL`.
    DemoAstroPlanet(const char* pName, MtxPtr pMtx);

    virtual ~DemoAstroPlanet();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();

    bool calcJointMatrix(TPos3f*, const JointControllerInfo&);

    /* 0x90 */ JointController* mJointController;
    /* 0x94 */ MtxPtr mBaseMtx;
    /* 0x98 */ f32 mEaseTimer;
    /* 0x9C */ f32 mRotationY;
    /* 0xA0 */ bool mIsRotating;
};
